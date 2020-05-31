// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Microsoft.Win32;
using System.IO;
using System.Diagnostics;

namespace CrystalAEPUI
{
    public partial class ExpertOptions : Form
    {
        public ExpertOptions()
        {
            InitializeComponent();
        }

        void ShowConfigLoadError(string configXmlName)
        {
            DialogResult dlgResult = MessageBox.Show(
                "There has been an error processing \"" + configXmlName + "\". Please press OK to restore configuration from default.",
                "Error",
                MessageBoxButtons.OKCancel,
                MessageBoxIcon.Error);

            if (dlgResult == System.Windows.Forms.DialogResult.OK)
            {
                try
                {
                    File.Copy(EventManager.InstallPath + "config.default", EventManager.InstallPath + configXmlName);
                }
                catch
                {
                    MessageBox.Show(
                        "Unable to restore config.",
                        "Error",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Error);
                }
            }
        }

        private bool undergoingUIUpdate = false;

        private bool UpdateUIFromConfig()
        {
            if (!XmlConfig.RefreshConfig())
                return false;

            lbComBlacklistClsidProgId.Items.Clear();
            lbComJITlistClsidProgId.Items.Clear();
            lbComWhitelistClsidProgId.Items.Clear();
            lbEnabledConnFilters.Items.Clear();
            lbEnabledIncomingFilters.Items.Clear();
            lbEnabledOutgoingFilters.Items.Clear();
            lbExeWhitelist.Items.Clear();
            lbModuleRWX.Items.Clear();
            lbProtProc.Items.Clear();
            
            string[] stringArray = XmlConfig.GetConfigStringArray("configuration/protectedProcessList/property/processList");
            if (stringArray != null)
            {
                lbProtProc.BeginUpdate();

                foreach (string name in stringArray)
                {
                    lbProtProc.Items.Add(name);
                }

                lbProtProc.EndUpdate();
            }

            // defense method checkboxes
            bool checkedState = XmlConfig.GetConfigBool("configuration/defenseInDepth/property/enabled");
            cbEnableDiDCore.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/browserProtection/property/enabled");
            cbEnableBpCore.Checked = checkedState;

            // major feature aspect checkboxes
            checkedState = XmlConfig.GetConfigBool("configuration/connectionMonitor/property/enabled");
            cbEnableConnMon.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/activeXMonitor/property/enabled");
            cbEnableComMon.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/enabled");
            cbEnableAPIMon.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/enabled");
            cbEnableMemMon.Checked = checkedState;

            // conn mon checkboxes

            checkedState = XmlConfig.GetConfigBool("configuration/connectionMonitor/property/validateConnect");
            cbConnMonValConnect.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/connectionMonitor/property/validateRequest");
            cbConnMonValOutgoing.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/connectionMonitor/property/validateResponse");
            cbConnMonValIncoming.Checked = checkedState;

            // conn mon listboxes

            XmlConfig.NameValuePair[] nvpArray = XmlConfig.GetNameValuePairArray("configuration/connectFilters/property/");
            if (nvpArray != null)
            {
                lbEnabledConnFilters.BeginUpdate();
                lbEnabledConnFilters.HorizontalScrollbar = true;

                foreach (XmlConfig.NameValuePair nvpair in nvpArray)
                {
                    string name = XmlConfig.GetNameFromPath(nvpair.Name);

                    if (name.StartsWith("Enabled:", true, null))
                    {
                        lbEnabledConnFilters.Items.Add(name.Substring(8) + " => " + nvpair.Value);
                    }
                }

                lbEnabledConnFilters.EndUpdate();
            }

            nvpArray = XmlConfig.GetNameValuePairArray("configuration/requestFilters/property/");
            if (nvpArray != null)
            {
                lbEnabledOutgoingFilters.BeginUpdate();
                lbEnabledOutgoingFilters.HorizontalScrollbar = true;

                foreach (XmlConfig.NameValuePair nvpair in nvpArray)
                {
                    string name = XmlConfig.GetNameFromPath(nvpair.Name);
                    
                    if (name.StartsWith("Enabled:", true, null))
                    {
                        lbEnabledOutgoingFilters.Items.Add(name.Substring(8) + " => " + nvpair.Value);
                    }
                }

                lbEnabledOutgoingFilters.EndUpdate();
            }

            nvpArray = XmlConfig.GetNameValuePairArray("configuration/responseFilters/property/");
            if (nvpArray != null)
            {
                lbEnabledIncomingFilters.BeginUpdate();
                lbEnabledIncomingFilters.HorizontalScrollbar = true;

                foreach (XmlConfig.NameValuePair nvpair in nvpArray)
                {
                    string name = XmlConfig.GetNameFromPath(nvpair.Name);

                    if (name.StartsWith("Enabled:", true, null))
                    {
                        lbEnabledIncomingFilters.Items.Add(name.Substring(8) + " => " + nvpair.Value);
                    }
                }

                lbEnabledIncomingFilters.EndUpdate();
            }

            // mem monitor checkboxes

            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/varyAllocSize");
            cbMemVaryAllocSizes.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/terminateCorruptHeaps");
            cbMemTermCorrupt.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/winValidateAlloc");
            cbMemValidateAllocsWindows.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/customValidateAlloc");
            cbMemValidateAllocsCrystal.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/antiUseAfterFree");
            cbMemUseAfterFree.Checked = checkedState;
            
            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/antiDoubleFree");
            cbMemDoubleFree.Checked = checkedState;
            
            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/maxMonSensitivity");
            cbMemMaxSens.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/processDepEnabled");
            cbMemEnableProcDEP.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/wipeAllocsOnFree");
            cbMemWipeAllocs.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/memoryMonitor/property/removeExecuteHeaps");
            cbMemDisableRWXHeap.Checked = checkedState;

            // api checkbox

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/allowCodeFromTemp");
            cbAPICodeFromTemp.Checked = !checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/allowCodeFromNetwork");
            cbAPICodeFromNet.Checked = !checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/allowCodeLongPath");
            cbAPICodeFromLongPath.Checked = !checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/allowCodeFromInetTemp");
            cbAPICodeFromInetTemp.Checked = !checkedState;
            
            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/allowCodeFromDownloads");
            cbAPICodeFromDownloads.Checked = !checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/blockDllPlanting");
            cbAPIAntiDllPlanting.Checked = checkedState;
            
            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/enableAntiROPStack");
            cbAPIAntiROPStack.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/checkROPReturn");
            cbAPIMonAddnAntiROP.Checked = checkedState;
            
            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/disableRWXVAMemory");
            cbAPIDisableRWXVA.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/disableRWXStackMemory");
            cbAPIDisableRWXStack.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/autoAddHookDllRWX");
            cbAPIAutoAllowDetours.Checked = checkedState;
            
            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/monitorAPICallOrigins");
            cbAPIMonCallOrigins.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/checkWhitelist");
            cbAPIEnableExeWhitelist.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/allowSpawnProtectedProcs");
            cbAPISpawnProtProc.Checked = checkedState;
            
            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/injectIntoChildProc");
            cbAPIInjectChild.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/promptUserExeBlocked");
            cbAPIQueryUsrExeBlocked.Checked = checkedState;
            
            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/randomizeAllocBase");
            cbAPIMonRndAllocBase.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/exceptHandlerChecks");
            cbAPIMonCheckExcept.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/emulateSEHHandler");
            cbAPIMonEmuExcept.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/promptRelaxDotnet");
            cbAPIPromptOnDotNet.Checked = checkedState;

            stringArray = XmlConfig.GetConfigStringArray("configuration/apiMonitor/property/executableWhitelist");
            if (stringArray != null)
            {
                lbExeWhitelist.BeginUpdate();

                foreach (string name in stringArray)
                {
                    lbExeWhitelist.Items.Add(name);
                }

                lbExeWhitelist.EndUpdate();
            }

            stringArray = XmlConfig.GetConfigStringArray("configuration/apiMonitor/property/allowRWXVAModules");
            if (stringArray != null)
            {
                lbModuleRWX.BeginUpdate();

                foreach (string name in stringArray)
                {
                    lbModuleRWX.Items.Add(name);
                }

                lbModuleRWX.EndUpdate();
            }
            
            // com monitor checkboxes

            checkedState = XmlConfig.GetConfigBool("configuration/activeXMonitor/property/clsidWhitelistEnabled");
            cbComEnableWhitelist.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/activeXMonitor/property/clsidBlacklistEnabled");
            cbComEnableBlacklist.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/activeXMonitor/property/persistJitDecision");
            cbComJITPersistDecision.Checked = checkedState;

            // com monitor listboxes

            stringArray = XmlConfig.GetConfigStringArray("configuration/activeXMonitor/property/clsidWhitelist");
            if (stringArray != null)
            {
                lbComWhitelistClsidProgId.BeginUpdate();

                foreach (string name in stringArray)
                {
                    lbComWhitelistClsidProgId.Items.Add(name);
                }

                lbComWhitelistClsidProgId.EndUpdate();
            }

            stringArray = XmlConfig.GetConfigStringArray("configuration/activeXMonitor/property/clsidBlacklist");
            if (stringArray != null)
            {
                lbComBlacklistClsidProgId.BeginUpdate();

                foreach (string name in stringArray)
                {
                    lbComBlacklistClsidProgId.Items.Add(name);
                }

                lbComBlacklistClsidProgId.EndUpdate();
            }

            stringArray = XmlConfig.GetConfigStringArray("configuration/activeXMonitor/property/clsidRequiresJit");
            if (stringArray != null)
            {
                lbComJITlistClsidProgId.BeginUpdate();

                foreach (string name in stringArray)
                {
                    lbComJITlistClsidProgId.Items.Add(name);
                }

                lbComJITlistClsidProgId.EndUpdate();
            }

            return true;
        }

        private bool LoadConfigOrShowError(string configFileName)
        {
            if (!XmlConfig.OpenConfig(EventManager.InstallPath + configFileName))
            {
                ShowConfigLoadError(configFileName);
                return false;
            }

            if (!XmlConfig.ReadConfig())
            {
                ShowConfigLoadError(configFileName);
                return false;
            }

            XmlConfig.Close();
            return true;
        }

        private void OptionsDialog_Load(object sender, EventArgs e)
        {
            RefEnableDiDCore = cbEnableDiDCore;
            RefEnableBpCore = cbEnableBpCore;
            
            LoadConfigOrShowError("config.xml");
        }

        private bool SavePendingSettings()
        {
            bool allSaved = true;

            foreach (KeyValuePair<string, string> kvp in pendingSettings)
            {
                if (!XmlConfig.SetConfigString(kvp.Key, kvp.Value))
                {
                    allSaved = false;
                }
            }

            pendingSettings.Clear();

            return allSaved;
        }

        private void AskUserSavePendingSettings()
        {
            if (pendingSettings.Count != 0)
            {
                DialogResult dlgResult = MessageBox.Show(
                    "There appear to have been changes to the protection settings. Would you like to save these changes?",
                    "Confirm Changes",
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Question);

                if (dlgResult == System.Windows.Forms.DialogResult.Yes)
                {
                    if (!SavePendingSettings())
                    {
                        MessageBox.Show(
                            "Not all of the changes saved successfully - please reset the config. and try again",
                            "Error Saving Changes",
                            MessageBoxButtons.OK);
                    }
                }
                else
                {
                    ResetFormChanges();
                }
            }
        }

        private void OptionsDialog_FormClosing(object sender, FormClosingEventArgs e)
        {
            AskUserSavePendingSettings();

            if (XmlConfig.Path.EndsWith("_config.xml", true, System.Globalization.CultureInfo.InvariantCulture))
            {
                if (!LoadConfigOrShowError("config.xml"))
                    return;

                ToggleProtProcEditControls(true);
            }

            Hide();

            if (!MainUI.ShuttingDownProcess)
                e.Cancel = true;
        }

        private bool UpdateUIFromConfigOrShowError()
        {
            undergoingUIUpdate = true;

            if (!UpdateUIFromConfig())
            {
                ShowConfigLoadError(XmlConfig.Path);

                undergoingUIUpdate = false;
                return false;
            }

            undergoingUIUpdate = false;
            return true;
        }

        private void OptionsDialog_Shown(object sender, EventArgs e)
        {
            if (!UpdateUIFromConfigOrShowError())
                return;
        }

        private bool CheckListboxContainsItem(ListBox lb, string item)
        {
            foreach (string s in lb.Items)
            {
                if (String.Compare(s, item, true) == 0)
                    return true;
            }

            return false;
        }

        private void btnAddProc_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(txtAddProc.Text) || !txtAddProc.Text.EndsWith(".exe"))
            {
                MessageBox.Show("Please enter a valid process name (*.exe)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (CheckListboxContainsItem(lbProtProc, txtAddProc.Text))
            {
                MessageBox.Show("This process already exists - please remove first or edit existing entry.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string newList = txtAddProc.Text;

            foreach (string s in lbProtProc.Items)
            {
                newList += "|" + s;
            }

            newList = newList.Trim('|');

            pendingSettings["configuration/protectedProcessList/property/processList"] = newList;
            lbProtProc.Items.Insert(0, txtAddProc.Text);
        }

        private void btnRemoveProc_Click(object sender, EventArgs e)
        {
            string newList = "";

            if (lbProtProc.SelectedIndex == -1)
                return;

            for (int i = 0; i < lbProtProc.Items.Count; i++)
            {
                if (lbProtProc.SelectedIndex != i)
                {
                    newList += lbProtProc.Items[i];

                    if (i + 1 < lbProtProc.Items.Count)
                    {
                        newList += "|";
                    }
                }
            }

            newList = newList.Trim('|');

            string procName = (string)lbProtProc.SelectedItem;

            lbProtProc.Items.RemoveAt(lbProtProc.SelectedIndex);
            pendingSettings["configuration/protectedProcessList/property/processList"] = newList;

            if (procName != null)
            {
                string configXml = procName.Replace(".exe", "_config.xml");
                
                if (File.Exists(EventManager.InstallPath + configXml))
                {
                    DialogResult dlgResult = MessageBox.Show(
                        "Delete custom configuration for \"" + procName + "\"?",
                        "Delete Configuration",
                        MessageBoxButtons.YesNo,
                        MessageBoxIcon.Question);
                    
                    if (dlgResult == System.Windows.Forms.DialogResult.Yes)
                    {
                        File.Delete(EventManager.InstallPath + configXml);
                    }
                }
            }
        }

        private Dictionary<string, string> pendingSettings = new Dictionary<string, string>();

        private void ResetFormChanges()
        {
            pendingSettings.Clear();
            
            if (!UpdateUIFromConfigOrShowError())
                return;
        }

        private void btnResetConfig_Click(object sender, EventArgs e)
        {
            ResetFormChanges();
        }

        private void btnUninstallAll_Click(object sender, EventArgs e)
        {
            DialogResult dlgResult = MessageBox.Show(
                "Please confirm that you wish to fully remove the Crystal Anti-Exploit Protection product.\n\n" +
                "This will eliminate all traces of the product from your system, and should only be used if Crystal " +
                "is causing your software to crash or exit unexpectedly. This cannot be undone; a reinstall will be " + 
                "required if you wish to use the product in the future.\n\n" +
                "Choose Yes to remove, and No to return without making any changes.",
                "Please Confirm",
                MessageBoxButtons.YesNo,
                MessageBoxIcon.Warning);

            if (dlgResult == System.Windows.Forms.DialogResult.Yes)
            {
                RemoveProductFully();
            }
        }

        enum InstallFailureCodes
        {
            Failure_Base = 0x30000000,
            Failure_RegInstallPath,
            Failure_FilesNotFound,
            Failure_UserAbortBeforeRemove,
        };

        bool RemoveProductFully()
        {
            // if for any reason the operation could not be completed in full return false
            // shell: reset.exe -remove
            
            Process resetProc = Process.Start(EventManager.InstallPath + "reset.exe", "-remove");
            resetProc.WaitForExit();

            return resetProc.ExitCode == 0;
        }

        private void cbEnableConnMon_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/connectionMonitor/property/enabled"] = cbEnableConnMon.Checked ? "true" : "false";
        }

        private void cbEnableMemMon_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/enabled"] = cbEnableMemMon.Checked ? "true" : "false";
        }

        private void cbEnableAPIMon_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/enabled"] = cbEnableAPIMon.Checked ? "true" : "false";
        }

        private void cbEnableComMon_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/activeXMonitor/property/enabled"] = cbEnableComMon.Checked ? "true" : "false";
        }

        private void cbComEnableWhitelist_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            if (pendingSettings.ContainsKey("configuration/activeXMonitor/property/clsidBlacklistEnabled") && cbComEnableWhitelist.Checked)
            {
                pendingSettings.Remove("configuration/activeXMonitor/property/clsidBlacklistEnabled");
            }

            undergoingUIUpdate = true;
            cbComEnableBlacklist.Checked = false;
            undergoingUIUpdate = false;

            pendingSettings["configuration/activeXMonitor/property/clsidWhitelistEnabled"] = cbComEnableWhitelist.Checked ? "true" : "false";
            pendingSettings["configuration/activeXMonitor/property/clsidBlacklistEnabled"] = cbComEnableWhitelist.Checked ? "false" : "true";
        }

        private void cbComEnableBlacklist_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            if (pendingSettings.ContainsKey("configuration/activeXMonitor/property/clsidWhitelistEnabled") && cbComEnableBlacklist.Checked)
            {
                pendingSettings.Remove("configuration/activeXMonitor/property/clsidWhitelistEnabled");
            }

            undergoingUIUpdate = true;
            cbComEnableWhitelist.Checked = false;
            undergoingUIUpdate = false;

            pendingSettings["configuration/activeXMonitor/property/clsidBlacklistEnabled"] = cbComEnableBlacklist.Checked ? "true" : "false";
            pendingSettings["configuration/activeXMonitor/property/clsidWhitelistEnabled"] = cbComEnableBlacklist.Checked ? "false" : "true";
        }

        private void btnComWhitelistClsidProgIdAdd_Click(object sender, EventArgs e)
        {
            if(String.IsNullOrEmpty(txtComWhitelistClsidProgId.Text))
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (CheckListboxContainsItem(lbComWhitelistClsidProgId, txtComWhitelistClsidProgId.Text))
            {
                MessageBox.Show("This CLSID already exists.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string[] nameClsidSplit = txtComWhitelistClsidProgId.Text.Split(':');
            if (nameClsidSplit == null || nameClsidSplit.Length == 0)
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string name = null, clsid = null;

            if (nameClsidSplit.Length == 1)
            {
                clsid = nameClsidSplit[0];
            }
            else if (nameClsidSplit.Length == 2)
            {
                name = nameClsidSplit[0];
                clsid = nameClsidSplit[1];
            }
            else
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            try
            {
                Guid guid = new Guid(clsid);

                if (guid == null)
                {
                    MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                clsid = "{" + guid.ToString() + "}";
            }
            catch
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (name != null && name.IndexOf('|') != -1)
            {
                MessageBox.Show("Please input name part of string in alpha-numeric form only.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string newList = !String.IsNullOrEmpty(name) ? name + ":" + clsid : clsid;

            foreach (string s in lbComWhitelistClsidProgId.Items)
            {
                newList += "|" + s;
            }
            
            newList = newList.Trim('|');

            pendingSettings["configuration/activeXMonitor/property/clsidWhitelist"] = newList;
            lbComWhitelistClsidProgId.Items.Insert(0, !String.IsNullOrEmpty(name) ? name + ":" + clsid : clsid);
        }

        private void btnComWhitelistClsidProgIdRemove_Click(object sender, EventArgs e)
        {
            string newList = "";

            if (lbComWhitelistClsidProgId.SelectedIndex == -1)
                return;

            for (int i = 0; i < lbComWhitelistClsidProgId.Items.Count; i++)
            {
                if (lbComWhitelistClsidProgId.SelectedIndex != i)
                {
                    newList += lbComWhitelistClsidProgId.Items[i];

                    if (i + 1 < lbComWhitelistClsidProgId.Items.Count)
                    {
                        newList += "|";
                    }
                }
            }

            lbComWhitelistClsidProgId.Items.RemoveAt(lbComWhitelistClsidProgId.SelectedIndex);
            pendingSettings["configuration/activeXMonitor/property/clsidWhitelist"] = newList;
        }

        private void btnComBlacklistClsidProgIdAdd_Click(object sender, EventArgs e)
        {
            if(String.IsNullOrEmpty(txtComBlacklistClsidProgId.Text))
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (CheckListboxContainsItem(lbComBlacklistClsidProgId, txtComBlacklistClsidProgId.Text))
            {
                MessageBox.Show("This CLSID already exists.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string[] nameClsidSplit = txtComBlacklistClsidProgId.Text.Split(':');
            if (nameClsidSplit == null || nameClsidSplit.Length == 0)
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string name = null, clsid = null;

            if (nameClsidSplit.Length == 1)
            {
                clsid = nameClsidSplit[0];
            }
            else if (nameClsidSplit.Length == 2)
            {
                name = nameClsidSplit[0];
                clsid = nameClsidSplit[1];
            }
            else
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            try
            {
                Guid guid = new Guid(clsid);

                if (guid == null)
                {
                    MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                clsid = "{" + guid.ToString() + "}";
            }
            catch
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (name != null && name.IndexOf('|') != -1)
            {
                MessageBox.Show("Please input name part of string in alpha-numeric form only.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string newList = !String.IsNullOrEmpty(name) ? name + ":" + clsid : clsid;

            foreach (string s in lbComBlacklistClsidProgId.Items)
            {
                newList += "|" + s;
            }
            
            newList = newList.Trim('|');

            pendingSettings["configuration/activeXMonitor/property/clsidBlacklist"] = newList;
            lbComBlacklistClsidProgId.Items.Insert(0, !String.IsNullOrEmpty(name) ? name + ":" + clsid : clsid);
        }

        private void btnComBlacklistClsidProgIdRemove_Click(object sender, EventArgs e)
        {
            string newList = "";

            if (lbComBlacklistClsidProgId.SelectedIndex == -1)
                return;

            for (int i = 0; i < lbComBlacklistClsidProgId.Items.Count; i++)
            {
                if (lbComBlacklistClsidProgId.SelectedIndex != i)
                {
                    newList += lbComBlacklistClsidProgId.Items[i];

                    if (i + 1 < lbComBlacklistClsidProgId.Items.Count)
                    {
                        newList += "|";
                    }
                }
            }

            newList = newList.Trim('|');

            lbComBlacklistClsidProgId.Items.RemoveAt(lbComBlacklistClsidProgId.SelectedIndex);
            pendingSettings["configuration/activeXMonitor/property/clsidBlacklist"] = newList;
        }

        private void btnComJITlistClsidProgIdAdd_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(txtComJITlistClsidProgId.Text))
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (CheckListboxContainsItem(lbComJITlistClsidProgId, txtComJITlistClsidProgId.Text))
            {
                MessageBox.Show("This CLSID already exists.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string[] nameClsidSplit = txtComJITlistClsidProgId.Text.Split(':');
            if (nameClsidSplit == null || nameClsidSplit.Length == 0)
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string name = null, clsid = null;

            if (nameClsidSplit.Length == 1)
            {
                clsid = nameClsidSplit[0];
            }
            else if (nameClsidSplit.Length == 2)
            {
                name = nameClsidSplit[0];
                clsid = nameClsidSplit[1];
            }
            else
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            try
            {
                Guid guid = new Guid(clsid);

                if (guid == null)
                {
                    MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                clsid = "{" + guid.ToString() + "}";
            }
            catch
            {
                MessageBox.Show("Please input CLSID in form: Name:{clsid} or just {clsid} (include braces)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (name != null && name.IndexOf('|') != -1)
            {
                MessageBox.Show("Please input name part of string in alpha-numeric form only.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string newList = !String.IsNullOrEmpty(name) ? name + ":" + clsid : clsid;

            foreach (string s in lbComJITlistClsidProgId.Items)
            {
                newList += "|" + s;
            }

            newList = newList.Trim('|');

            pendingSettings["configuration/activeXMonitor/property/clsidRequiresJit"] = newList;
            lbComJITlistClsidProgId.Items.Insert(0, !String.IsNullOrEmpty(name) ? name + ":" + clsid : clsid);
        }

        private void btnComJITlistClsidProgIdRemove_Click(object sender, EventArgs e)
        {
            string newList = "";

            if (lbComJITlistClsidProgId.SelectedIndex == -1)
                return;

            for (int i = 0; i < lbComJITlistClsidProgId.Items.Count; i++)
            {
                if (lbComJITlistClsidProgId.SelectedIndex != i)
                {
                    newList += lbComJITlistClsidProgId.Items[i];

                    if (i + 1 < lbComJITlistClsidProgId.Items.Count)
                    {
                        newList += "|";
                    }
                }
            }

            newList = newList.Trim('|');

            lbComJITlistClsidProgId.Items.RemoveAt(lbComJITlistClsidProgId.SelectedIndex);
            pendingSettings["configuration/activeXMonitor/property/clsidRequiresJit"] = newList;
        }

        private void cbConnMonValConnect_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/connectionMonitor/property/validateConnect"] = cbConnMonValConnect.Checked ? "true" : "false";
        }

        private void cbConnMonValOutgoing_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/connectionMonitor/property/validateRequest"] = cbConnMonValOutgoing.Checked ? "true" : "false";
        }

        private void cbConnMonValIncoming_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/connectionMonitor/property/validateResponse"] = cbConnMonValIncoming.Checked ? "true" : "false";
        }

        private void cbMemVaryAllocSizes_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/varyAllocSize"] = cbMemVaryAllocSizes.Checked ? "true" : "false";
        }

        private void cbMemTermCorrupt_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/terminateCorruptHeaps"] = cbMemTermCorrupt.Checked ? "true" : "false";
        }

        private void cbMemValidateAllocsWindows_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/winValidateAlloc"] = cbMemValidateAllocsWindows.Checked ? "true" : "false";
        }

        private void cbMemValidateAllocsCrystal_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/customValidateAlloc"] = cbMemValidateAllocsCrystal.Checked ? "true" : "false";
        }

        private void cbMemEnableProcDEP_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/processDepEnabled"] = cbMemEnableProcDEP.Checked ? "true" : "false";
        }

        private void cbMemUseAfterFree_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/antiUseAfterFree"] = cbMemUseAfterFree.Checked ? "true" : "false";
        }

        private void cbMemDoubleFree_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/antiDoubleFree"] = cbMemDoubleFree.Checked ? "true" : "false";
        }

        private void btnExeWlAdd_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(txtExecWlAdd.Text) || !txtExecWlAdd.Text.EndsWith(".exe"))
            {
                MessageBox.Show("Please enter a valid process name (*.exe)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (CheckListboxContainsItem(lbExeWhitelist, txtExecWlAdd.Text))
            {
                MessageBox.Show("This executable already exists on the whitelist.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string newList = txtExecWlAdd.Text;

            foreach (string s in lbExeWhitelist.Items)
            {
                newList += "|" + s;
            }

            newList = newList.Trim('|');

            pendingSettings["configuration/apiMonitor/property/executableWhitelist"] = newList;
            lbExeWhitelist.Items.Insert(0, txtExecWlAdd.Text);
        }

        private void btnExeWlRemove_Click(object sender, EventArgs e)
        {
            string newList = "";

            if (lbExeWhitelist.SelectedIndex == -1)
                return;

            for (int i = 0; i < lbExeWhitelist.Items.Count; i++)
            {
                if (lbExeWhitelist.SelectedIndex != i)
                {
                    newList += lbExeWhitelist.Items[i];

                    if (i + 1 < lbExeWhitelist.Items.Count)
                    {
                        newList += "|";
                    }
                }
            }

            newList = newList.Trim('|');

            lbExeWhitelist.Items.RemoveAt(lbExeWhitelist.SelectedIndex);
            pendingSettings["configuration/apiMonitor/property/executableWhitelist"] = newList;
        }

        private void cbAPIInjectChild_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/injectIntoChildProc"] = cbAPIInjectChild.Checked ? "true" : "false";
        }

        private void cbAPICodeFromLongPath_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/allowCodeLongPath"] = cbAPICodeFromLongPath.Checked ? "false" : "true";
        }

        private void cbAPICodeFromTemp_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/allowCodeFromTemp"] = cbAPICodeFromTemp.Checked ? "false" : "true";
        }

        private void cbAPICodeFromNet_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/allowCodeFromNetwork"] = cbAPICodeFromNet.Checked ? "false" : "true";
        }

        private void cbAPICodeFromInetTemp_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/allowCodeFromInetTemp"] = cbAPICodeFromInetTemp.Checked ? "false" : "true";
        }

        private void cbAPIEnableExeWhitelist_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/checkWhitelist"] = cbAPIEnableExeWhitelist.Checked ? "true" : "false";
        }

        private void cbAPISpawnProtProc_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/allowSpawnProtectedProcs"] = cbAPISpawnProtProc.Checked ? "true" : "false";
        }

        private void cbAPIDisableRWX_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/disableRWXVAMemory"] = cbAPIDisableRWXVA.Checked ? "true" : "false";
        }

        private void cbAPIMonCallOrigins_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/monitorAPICallOrigins"] = cbAPIMonCallOrigins.Checked ? "true" : "false";
        }

        private void cbAPIAntiROPStack_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/enableAntiROPStack"] = cbAPIAntiROPStack.Checked ? "true" : "false";
        }

        Filters filtersForm = new Filters();

        private void btnImportFilters_Click(object sender, EventArgs e)
        {
            //filtersForm.Show();
            filtersForm.ShowDialog();
            // when this returns, the filter listboxes must be updated. how best to check?
            if (filtersForm.ConfigChanged)
            {
                MessageBox.Show(
                    "To save any changes made to the protection options, and view any made to filters, please close/open the Protection Options form.",
                    "Information",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Information);
            }
        }

        private void OptionsDialog_VisibleChanged(object sender, EventArgs e)
        {
            undergoingUIUpdate = true;

            if (!UpdateUIFromConfigOrShowError())
                return;
        }

        bool ReadSFSAndSFIActiveXFromRegistry(out string[] sfsClsidArray, out string[] sfiClsidArray)
        {
            sfsClsidArray = null;
            sfiClsidArray = null;

            try
            {
                Guid sfsGuid = new Guid("{7DD95801-9882-11CF-9FA9-00AA006C42C4}"), // CATID_SafeForScripting
                    sfiGuid = new Guid("{7DD95802-9882-11CF-9FA9-00AA006C42C4}"); // CATID_SafeForInitialization

                List<string> clsidListSFS = new List<string>(), clsidListSFI = new List<string>();

                RegistryKey rkClassesRoot = Registry.ClassesRoot;
                RegistryKey rkClsid = rkClassesRoot.OpenSubKey("CLSID");

                string[] clsidKeyNames = rkClsid.GetSubKeyNames();
                if (clsidKeyNames == null)
                    return false;

                foreach (string clsidName in clsidKeyNames)
                {
                    try
                    {
                        Guid testGuid = new Guid(clsidName); // throws if not guid format

                        bool sfsFound = false, sfiFound = false;

                        RegistryKey rkClsidCurrent = rkClsid.OpenSubKey(clsidName);
                        if (rkClsidCurrent == null)
                            continue;

                        RegistryKey rkImplementedCat = rkClsidCurrent.OpenSubKey("Implemented Categories");
                        if (rkImplementedCat == null)
                            continue;

                        string[] implCatNames = rkImplementedCat.GetSubKeyNames();
                        if (implCatNames == null)
                            continue;

                        foreach (string catName in implCatNames)
                        {
                            if (new Guid(catName).Equals(sfiGuid))
                                sfiFound = true;

                            if (new Guid(catName).Equals(sfsGuid))
                                sfsFound = true;
                        }

                        string displayName = (string)rkClsidCurrent.GetValue(null).ToString();

                        if (displayName == null)
                        {
                            string progID = null;

                            RegistryKey rkProgId = rkClsidCurrent.OpenSubKey("ProgID");
                            if (rkProgId != null)
                            {
                                progID = rkProgId.GetValue(null).ToString();
                            }

                            displayName = progID;
                        }

                        if(displayName != null)
                            displayName = displayName.Replace("|", "_");

                        if (sfiFound)
                        {
                            clsidListSFI.Add((displayName == null ? "Unnamed-SFI" : displayName) + ":" + clsidName);
                        }

                        if (sfsFound)
                        {
                            clsidListSFS.Add((displayName == null ? "Unnamed-SFS" : displayName) + ":" + clsidName);
                        }
                    }
                    catch
                    {
                        continue;
                    }
                }

                sfsClsidArray = clsidListSFS.ToArray();
                sfiClsidArray = clsidListSFI.ToArray();

                return (clsidListSFI.Count != 0 || clsidListSFS.Count != 0);
            }
            catch
            {
                return false;
            }
        }

        private void btnAutoPopBlacklist_Click(object sender, EventArgs e)
        {
            string[] sfsClsidArray = null, sfiClsidArray = null;

            DialogResult dlgResult = MessageBox.Show("This will automatically block all COM/ActiveX modules marked Safe for Scripting and Safe for Initialisation" +
                " and will override your current settings. Do you wish to proceed?\r\n\r\nIt is advisable not to proceed unless you understand that this" +
                " will probably add some components that you rely on frequently (for example, Adobe Flash for YouTube) which will need to be manually removed to" +
                " allow use of reliant sites in the future.\r\n\r\nManual removal can be done via the Remove Button on the CLSID Blacklist tab.",
                "Override CLSID Blacklist?",
                MessageBoxButtons.YesNo,
                MessageBoxIcon.Information
                );

            if (dlgResult != System.Windows.Forms.DialogResult.Yes)
                return;

            if (ReadSFSAndSFIActiveXFromRegistry(out sfsClsidArray, out sfiClsidArray))
            {
                string newList = "";

                lbComBlacklistClsidProgId.Items.Clear();

                lbComBlacklistClsidProgId.BeginUpdate();

                if (sfsClsidArray != null && sfsClsidArray.Length != 0)
                {
                    foreach (string clsid in sfsClsidArray)
                    {
                        lbComBlacklistClsidProgId.Items.Add(clsid);
                        newList += clsid + "|";
                    }
                }

                if (sfiClsidArray != null && sfiClsidArray.Length != 0)
                {
                    List<string> sfsClsidList = new List<string>(sfsClsidArray);

                    foreach (string clsid in sfiClsidArray)
                    {
                        if (sfsClsidList.Contains(clsid))
                            continue;

                        lbComBlacklistClsidProgId.Items.Add(clsid);
                        newList += clsid + "|";
                    }
                }

                newList = newList.Trim('|');

                lbComBlacklistClsidProgId.EndUpdate();

                if (newList.Length != 0)
                {
                    pendingSettings["configuration/activeXMonitor/property/clsidBlacklist"] = newList;
                }
            }
        }

        public static CheckBox RefEnableDiDCore = null;

        private void cbEnableDiDCore_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            XmlConfig.SetConfigString("configuration/defenseInDepth/property/enabled", cbEnableDiDCore.Checked ? "true" : "false");
            MainUI.RefEnableDiD.Checked = cbEnableDiDCore.Checked;
        }

        public static CheckBox RefEnableBpCore = null;

        private void cbEnableBpCore_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            XmlConfig.SetConfigString("configuration/browserProtection/property/enabled", cbEnableBpCore.Checked ? "true" : "false");
            MainUI.RefEnableCF.Checked = cbEnableBpCore.Checked;
        }

        private void cbAPIDisableRWXStack_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/disableRWXStackMemory"] = cbAPIDisableRWXStack.Checked ? "true" : "false";
        }

        private void btnModuleRWXAdd_Click(object sender, EventArgs e)
        {
            if (String.IsNullOrEmpty(txtModuleRWXAdd.Text) || !txtModuleRWXAdd.Text.EndsWith(".dll"))
            {
                MessageBox.Show("Please enter a valid module name (*.dll)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (CheckListboxContainsItem(lbModuleRWX, txtModuleRWXAdd.Text))
            {
                MessageBox.Show("This module already exists on the whitelist.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string newList = txtModuleRWXAdd.Text;

            foreach (string s in lbModuleRWX.Items)
            {
                newList += "|" + s;
            }

            newList = newList.Trim('|');

            pendingSettings["configuration/apiMonitor/property/allowRWXVAModules"] = newList;
            lbModuleRWX.Items.Insert(0, txtModuleRWXAdd.Text);
        }

        private void btnModuleRWXRemove_Click(object sender, EventArgs e)
        {
            string newList = "";

            if (lbModuleRWX.SelectedIndex == -1)
                return;

            for (int i = 0; i < lbModuleRWX.Items.Count; i++)
            {
                if (lbModuleRWX.SelectedIndex != i)
                {
                    newList += lbModuleRWX.Items[i];

                    if (i + 1 < lbModuleRWX.Items.Count)
                    {
                        newList += "|";
                    }
                }
            }

            newList = newList.Trim('|');

            lbModuleRWX.Items.RemoveAt(lbModuleRWX.SelectedIndex);
            pendingSettings["configuration/apiMonitor/property/allowRWXVAModules"] = newList;
        }

        private void cbMemMonSerialized_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/maxMonSensitivity"] = cbMemMaxSens.Checked ? "true" : "false";
        }

        private void cbComJITPersistDecision_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/activeXMonitor/property/persistJitDecision"] = cbComJITPersistDecision.Checked ? "true" : "false";
        }

        private void cbAPIMonRndAllocBase_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/randomizeAllocBase"] = cbAPIMonRndAllocBase.Checked ? "true" : "false";
        }

        private void cbAPICodeFromDownloads_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/allowCodeFromDownloads"] = cbAPICodeFromDownloads.Checked ? "false" : "true";
        }

        private void cbAPIMonCheckExcept_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/exceptHandlerChecks"] = cbAPIMonCheckExcept.Checked ? "true" : "false";
        }

        private void cbAPIMonEmuExcept_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/emulateSEHHandler"] = cbAPIMonEmuExcept.Checked ? "true" : "false";
        }

        private void cbAPIQueryUsrExeBlocked_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/promptUserExeBlocked"] = cbAPIQueryUsrExeBlocked.Checked ? "true" : "false";
        }

        private void cbAPIAntiDllPlanting_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/blockDllPlanting"] = cbAPIAntiDllPlanting.Checked ? "true" : "false";
        }

        private void cbMemWipeAllocs_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/wipeAllocsOnFree"] = cbMemWipeAllocs.Checked ? "true" : "false";
        }

        private void cbMemDisableRWXHeap_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/memoryMonitor/property/removeExecuteHeaps"] = cbMemDisableRWXHeap.Checked ? "true" : "false";
        }

        void ToggleProtProcEditControls(bool enabled)
        {
            if (enabled == false)
            {
                // show prog specific settings
                if (lbProtProc.SelectedItem == null)
                    return;

                string procName = (string)lbProtProc.SelectedItem;

                btnProcEditConf.Tag = (string)lbProtProc.SelectedItem;
                btnProcEditConf.Text = "Finished (Revert)";
                grpProtMethods.Text = "Protection Methods (" + procName + ")";
                grpProtMethods.ForeColor = Color.Red;
                lbProtProc.Enabled = false;
                btnAddProc.Enabled = false;
                btnRemoveProc.Enabled = false;
                txtAddProc.Enabled = false;
            }
            else
            {
                // show default settings
                btnProcEditConf.Tag = null;
                btnProcEditConf.Text = "Edit Configuration";
                grpProtMethods.Text = "Protection Methods (Defaults)";
                grpProtMethods.ForeColor = Color.Black;
                lbProtProc.Enabled = true;
                btnAddProc.Enabled = true;
                btnRemoveProc.Enabled = true;
                txtAddProc.Enabled = true;
            }
        }

        private void btnProcEditConf_Click(object sender, EventArgs e)
        {
            if (btnProcEditConf.Tag == null)
            {
                // show prog specific settings
                if (lbProtProc.SelectedItem == null)
                    return;

                ToggleProtProcEditControls(false);

                string procName = (string)lbProtProc.SelectedItem;
                string confName = procName.Replace(".exe", "_config.xml");

                if (!File.Exists(EventManager.InstallPath + confName))
                {
                    // this is first time config is being made, copy default

                    try
                    {
                        File.Copy(EventManager.InstallPath + "config.xml", EventManager.InstallPath + confName);
                    }
                    catch
                    {
                        MessageBox.Show(String.Format("Unable to create new configuration XML file at path {0}.", EventManager.InstallPath + confName), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        ToggleProtProcEditControls(true);
                        return;
                    }
                }
                //else: we have a config XML for this process already

                if (!File.Exists(EventManager.InstallPath + confName))
                {
                    MessageBox.Show(String.Format("Unable to open new configuration XML file at path {0}.", EventManager.InstallPath + confName), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    ToggleProtProcEditControls(true);
                    return;
                }

                if (!LoadConfigOrShowError(confName))
                    return;

                if (!UpdateUIFromConfigOrShowError())
                    return;
            }
            else
            {
                // save settings
                AskUserSavePendingSettings();

                // show default settings
                ToggleProtProcEditControls(true);

                if (!LoadConfigOrShowError("config.xml"))
                    return;

                if (!UpdateUIFromConfigOrShowError())
                    return;
            }
        }

        private void cbAPIPromptOnDotNet_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/promptRelaxDotnet"] = cbAPIPromptOnDotNet.Checked ? "true" : "false";
        }

        private void cbAPIMonAddnAntiROP_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/checkROPReturn"] = cbAPIMonAddnAntiROP.Checked ? "true" : "false";
        }

        private void cbAPIAutoAllowDetours_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            pendingSettings["configuration/apiMonitor/property/autoAddHookDllRWX"] = cbAPIAutoAllowDetours.Checked ? "true" : "false";
        }
    }
}
