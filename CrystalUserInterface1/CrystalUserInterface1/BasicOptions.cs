// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using Microsoft.Win32;
using System.Diagnostics;

namespace CrystalAEPUI
{
    public partial class BasicOptions : Form
    {
        public BasicOptions()
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

        private void PopulateRunningProgramsList()
        {
            Process[] runningProcs = Process.GetProcesses();

            foreach (Process proc in runningProcs)
            {
                string filePath = null, app = null;
                
                try
                {
                    filePath = proc.MainModule.FileName;
                    app = new FileInfo(filePath).Name;
                }
                catch
                {
                    continue;
                }

                app = app.ToLower();
                ListViewItem lviNew = new ListViewItem(new string[] { app, "" });

                if (filePath != null)
                {
                    System.Drawing.Icon icon = System.Drawing.Icon.ExtractAssociatedIcon(filePath);
                    if (icon != null)
                    {
                        if (lvInstalledProc.SmallImageList == null)
                            lvInstalledProc.SmallImageList = new ImageList();

                        lvInstalledProc.SmallImageList.Images.Add(app, icon);
                        lviNew.ImageKey = app;
                    }

                    FileVersionInfo fvi = FileVersionInfo.GetVersionInfo(filePath);
                    if (fvi != null)
                    {
                        lviNew.SubItems[1].Text = fvi.FileDescription;
                    }
                }

                lvInstalledProc.Items.Add(lviNew);
            }
        }

        private void PopulateInstalledProgramsList()
        {
            RegistryKey rk = Registry.ClassesRoot.OpenSubKey("Applications");
            if (rk == null)
                return;

            string[] appNames = rk.GetSubKeyNames();
            string[] paths = { // more ?
                    "shell\\open\\command",
                    "shell\\read\\command",
                    "shell\\edit\\command",
                    "shell\\play\\command"
                };

            foreach (string appName in appNames)
            {
                if (!appName.EndsWith(".exe", StringComparison.InvariantCultureIgnoreCase))
                    continue;

                string app = appName.ToLower();

                ListViewItem lviNew = new ListViewItem(new string[] { app, "" });
                lviNew.Text = app;

                string filePath = null;

                foreach (string path in paths)
                {
                    RegistryKey rkSubKey = rk.OpenSubKey(app + "\\" + path);
                    if (rkSubKey != null)
                    {
                        string value = (string)rkSubKey.GetValue(null, null);
                        if (value == null)
                            continue;

                        if (value.IndexOf(app, StringComparison.InvariantCultureIgnoreCase) == -1)
                            continue;

                        if (value[0] == '\"')
                            value = value.Substring(1, value.IndexOf(app + "\"", StringComparison.InvariantCultureIgnoreCase) + app.Length - 1);
                        else
                            value = value.Substring(0, value.IndexOf(' '));

                        if (!File.Exists(value))
                            continue;

                        filePath = value;
                        break;
                    }
                }

                if (filePath == null)
                {
                    string[] defaultPaths = new string[] {
                            "C:\\Windows\\SysWow64\\",
                            "C:\\Windows\\",
                            "C:\\Windows\\System32\\"
                        };

                    foreach (string path in defaultPaths)
                    {
                        if (File.Exists(path + app))
                        {
                            filePath = path + app;
                            break;
                        }
                    }
                }

                if (filePath != null)
                {
                    System.Drawing.Icon icon = System.Drawing.Icon.ExtractAssociatedIcon(filePath);
                    if (icon != null)
                    {
                        if (lvInstalledProc.SmallImageList == null)
                            lvInstalledProc.SmallImageList = new ImageList();

                        lvInstalledProc.SmallImageList.Images.Add(app, icon);
                        lviNew.ImageKey = app;
                    }

                    FileVersionInfo fvi = FileVersionInfo.GetVersionInfo(filePath);
                    if (fvi != null)
                    {
                        lviNew.SubItems[1].Text = fvi.FileDescription;
                    }
                }

                if (!foundPrograms.Contains(app))
                {
                    foundPrograms.Add(app);
                    lvInstalledProc.Items.Add(lviNew);
                }
            }
        }

        private List<string> EnumExecutablesInDirectory(string directory, int depth)
        {
            List<string> listExes = new List<string>();

            if(String.IsNullOrEmpty(directory))
                return null;

            if(directory[directory.Length - 1] != '\\')
            {
                directory += '\\';
            }

            if (depth > 0)
            {
                string[] dirs = Directory.GetDirectories(directory);

                foreach (string dir in dirs)
                {
                    List<string> newExes = EnumExecutablesInDirectory(dir, depth - 1);
                    listExes.AddRange(newExes);
                }
            }

            string[] files = Directory.GetFiles(directory);
            
            foreach (string file in files)
            {
                if (file.EndsWith(".exe", StringComparison.InvariantCultureIgnoreCase))
                {
                    listExes.Add(file);
                }
            }

            return listExes;
        }

        List<string> foundPrograms = new List<string>();
        
        private void PopulateAddRemoveProgramsList()
        {
            foundPrograms.Clear();

            RegistryKey rk = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
            if (rk == null)
                return;

            string[] subkeys = rk.GetSubKeyNames();
            List<string> installLocations = new List<string>(), displayNames = new List<string>();

            foreach (string clsid in subkeys)
            {
                RegistryKey rkSub = rk.OpenSubKey(clsid);
                if (rkSub == null)
                    continue;

                string path = (string)rkSub.GetValue("InstallLocation");
                if (String.IsNullOrEmpty(path))
                {
                    path = (string)rkSub.GetValue("DisplayIcon");

                    if (String.IsNullOrEmpty(path))
                        continue;

                    path = path.Replace("\"", "");
                    
                    path = new FileInfo(path).Directory.FullName;
                }
                else
                {
                    path = path.Replace("\"", "");
                }

                installLocations.Add(path);

                string display = (string)rkSub.GetValue("DisplayName");

                displayNames.Add(String.IsNullOrEmpty(display) ? "" : display);
            }

            int i = 0;

            foreach (string installDir in installLocations)
            {
                List<string> executables = null;
                try
                {
                    executables = EnumExecutablesInDirectory(installDir, 3);
                }
                catch
                {
                    // bad dir (perms etc)
                    continue;
                }

                string dispName = displayNames[i++];

                foreach (string filePath in executables)
                {
                    FileInfo fi = new FileInfo(filePath);
                    string app = fi.Name;
                    
                    app = app.ToLower();

                    ListViewItem lviNew = new ListViewItem(new string[] { app, "" });
                    
                    lviNew.Text = app;

                    if (filePath != null)
                    {
                        System.Drawing.Icon icon = System.Drawing.Icon.ExtractAssociatedIcon(filePath);
                        if (icon != null)
                        {
                            if (lvInstalledProc.SmallImageList == null)
                                lvInstalledProc.SmallImageList = new ImageList();

                            lvInstalledProc.SmallImageList.Images.Add(app, icon);
                            lviNew.ImageKey = app;
                        }
                        else
                        {
                            continue;
                        }

                        FileVersionInfo fvi = FileVersionInfo.GetVersionInfo(filePath);
                        if (fvi != null)
                        {
                            if (String.IsNullOrEmpty(fvi.FileDescription) && !String.IsNullOrEmpty(dispName))
                            {
                                lviNew.SubItems[1].Text = dispName;
                            }
                            else
                            {
                                lviNew.SubItems[1].Text = fvi.FileDescription;
                            }
                        }
                        else
                        {
                            lviNew.SubItems[1].Text = dispName;
                        }

                        if (fi.Length < (1024 * 128) || // small bins
                            fi.Name.IndexOf("unin", 0, StringComparison.InvariantCultureIgnoreCase) != -1) // uninstallers
                        {
                            // just exclude these
                            continue;
                        }

                    }

                    if (!foundPrograms.Contains(app))
                    {
                        foundPrograms.Add(app);
                        lvInstalledProc.Items.Add(lviNew);
                    }
                }
            }
        }

        private void UpdateUI()
        {
            if (!LoadConfigOrShowError("config.xml"))
                return;

            radUseDefault.Checked = true;
            HighlightProtSliderLevel();

            lvProtProc.Items.Clear();

            string[] stringArray = XmlConfig.GetConfigStringArray("configuration/protectedProcessList/property/processList");
            if (stringArray != null)
            {
                lvProtProc.BeginUpdate();

                foreach (string name in stringArray)
                {
                    lvProtProc.Items.Add(new ListViewItem(new string[] { name }));
                }

                lvProtProc.EndUpdate();
            }

            bool checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/promptUserExeBlocked");
            cbPromptUserExe.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/checkWhitelist");
            cbExeWhitelist.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/defenseInDepth/property/minimizeToTray");
            cbMinimizeToTray.Checked = checkedState;

            checkedState = XmlConfig.GetConfigBool("configuration/apiMonitor/property/promptRelaxDotnet");
            cbPromptOnDotNet.Checked = checkedState;
        }

        private void BasicOptions_Load(object sender, EventArgs e)
        {
            lvInstalledProc.Sorting = SortOrder.Ascending;
            
            PopulateAddRemoveProgramsList();
            PopulateInstalledProgramsList();
            lvInstalledProc.Sort();

            UpdateUI();
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            if (btnSwitchProgView.Tag == null)
            {
                lvInstalledProc.Items.Clear();
                PopulateAddRemoveProgramsList();
                PopulateInstalledProgramsList();
                lvInstalledProc.Sort();
            }
            else
            {
                lvInstalledProc.Items.Clear();
                PopulateRunningProgramsList();
            }
        }

        private void BasicOptions_FormClosing(object sender, FormClosingEventArgs e)
        {
            Hide();

            if (!MainUI.ShuttingDownProcess)
                e.Cancel = true;
        }

        private void btnBrowse_Click(object sender, EventArgs e)
        {
            DialogResult dlgResult = openProgramDlg.ShowDialog();
            
            if (dlgResult == System.Windows.Forms.DialogResult.OK)
            {
                string app = openProgramDlg.FileName;
                if (app != null)
                {
                    app = new FileInfo(app).Name;
                    app = app.ToLower();

                    dlgResult = MessageBox.Show(
                        "Would you like to add program \"" + app + "\" to the list of protected programs?",
                        "Add Protected Progam",
                        MessageBoxButtons.YesNo,
                        MessageBoxIcon.Question);

                    if (dlgResult == System.Windows.Forms.DialogResult.Yes)
                    {
                        if (!LoadConfigOrShowError("config.xml"))
                            return;

                        AddProcByName(app);
                    }
                }
            }
        }

        private void radUseDefault_CheckedChanged(object sender, EventArgs e)
        {
            if (radUseDefault.Checked)
            {
                tbProtLevel.Enabled = false;
                lblProtHigh.Enabled = false;
                lblProtMax.Enabled = false;
                lblProtMin.Enabled = false;
                lblProtMod.Enabled = false;

                if (lvProtProc.SelectedItems.Count == 0)
                    return;
                
                string proc = lvProtProc.SelectedItems[0].Text;
                string xmlFile = proc.Replace(".exe", "_config.xml");

                if (File.Exists(EventManager.InstallPath + xmlFile))
                {
                    DialogResult dlgResult = MessageBox.Show(
                        "A custom configuration has been found for \"" + proc + "\" - delete?",
                        "Replace Configuration",
                        MessageBoxButtons.YesNo,
                        MessageBoxIcon.Question);

                    if (dlgResult == System.Windows.Forms.DialogResult.No)
                    {
                        radUseCustom.Checked = true;
                        return;
                    }

                    File.Delete(EventManager.InstallPath + xmlFile);
                }

                if (!LoadConfigOrShowError("config.xml"))
                    return;
            }
        }

        private void btnSwitchProgView_Click(object sender, EventArgs e)
        {
            if (btnSwitchProgView.Tag == null)
            {
                lvInstalledProc.Items.Clear();
                PopulateRunningProgramsList();
                btnSwitchProgView.Text = "Show Installed ...";
                lblInstalledRunning.Text = "Running Programs:";
                btnSwitchProgView.Tag = "Running ...";
            }
            else
            {
                lvInstalledProc.Items.Clear();
                PopulateAddRemoveProgramsList();
                PopulateInstalledProgramsList();
                lvInstalledProc.Sort();
                btnSwitchProgView.Text = "Show Running ...";
                lblInstalledRunning.Text = "Installed Programs:";
                btnSwitchProgView.Tag = null;
            }
        }

        private bool CheckListboxContainsItem(ListView lv, string item)
        {
            foreach (ListViewItem lvi in lv.Items)
            {
                if (String.Compare(lvi.Text, item, true) == 0)
                    return true;
            }

            return false;
        }

        private void AddProcByName(string proc)
        {
            if (String.IsNullOrEmpty(proc) || !proc.EndsWith(".exe"))
            {
                MessageBox.Show("Please enter a valid process name (*.exe)", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (CheckListboxContainsItem(lvProtProc, proc))
            {
                MessageBox.Show("This process already exists - please remove first or edit existing entry.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string newList = proc;

            foreach (ListViewItem lvi in lvProtProc.Items)
            {
                newList += "|" + lvi.Text;
            }

            newList = newList.Trim('|');

            XmlConfig.SetConfigString("configuration/protectedProcessList/property/processList", newList);
            lvProtProc.Items.Insert(0, proc);
        }

        private void btnAddByName_Click(object sender, EventArgs e)
        {
            if (!LoadConfigOrShowError("config.xml"))
                return;

            AddProcByName(txtProcName.Text);
        }

        private void btnRemoveProc_Click(object sender, EventArgs e)
        {
            if (!LoadConfigOrShowError("config.xml"))
                return;

            string newList = "";

            if (lvProtProc.SelectedItems.Count == 0)
                return;

            for (int i = 0; i < lvProtProc.Items.Count; i++)
            {
                if (lvProtProc.SelectedItems[0].Index != i)
                {
                    newList += lvProtProc.Items[i].Text;

                    if (i + 1 < lvProtProc.Items.Count)
                    {
                        newList += "|";
                    }
                }
            }

            newList = newList.Trim('|');

            string procName = (string)lvProtProc.SelectedItems[0].Text;

            lvProtProc.Items.RemoveAt(lvProtProc.SelectedItems[0].Index);
            XmlConfig.SetConfigString("configuration/protectedProcessList/property/processList", newList);

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

        private void btnAddProc_Click(object sender, EventArgs e)
        {
            if (lvInstalledProc.SelectedItems.Count == 0)
                return;

            if (!LoadConfigOrShowError("config.xml"))
                return;

            AddProcByName(lvInstalledProc.SelectedItems[0].Text);
        }

        private void radUseCustom_CheckedChanged(object sender, EventArgs e)
        {
            if (radUseCustom.Checked)
            {
                if (lvProtProc.SelectedItems.Count == 0)
                {
                    radUseDefault.Checked = true;
                    HighlightProtSliderLevel();
                    return;
                }

                string proc = lvProtProc.SelectedItems[0].Text;
                string xmlFile = proc.Replace(".exe", "_config.xml");

                if (!File.Exists(EventManager.InstallPath + xmlFile))
                {
                    File.Copy(EventManager.InstallPath + "config.xml", EventManager.InstallPath + xmlFile);
                }

                string uiProtLevel = XmlConfig.GetConfigString("configuration/defenseInDepth/property/uiProtLevel");
                int level = int.Parse(uiProtLevel);

                tbProtLevel.Value = level;
                HighlightProtSliderLevel();

                if (!LoadConfigOrShowError(xmlFile))
                {
                    radUseDefault.Checked = true;
                    HighlightProtSliderLevel();
                    return;
                }

                tbProtLevel.Enabled = true;
                lblProtHigh.Enabled = true;
                lblProtMax.Enabled = true;
                lblProtMin.Enabled = true;
                lblProtMod.Enabled = true;
            }
        }


        void HighlightProtSliderLevel()
        {
            lblProtMin.ForeColor = Color.Black;
            lblProtMod.ForeColor = Color.Black;
            lblProtHigh.ForeColor = Color.Black;
            lblProtMax.ForeColor = Color.Black;

            switch ((ProtectionLevels.EProtectionLevel)tbProtLevel.Value)
            {
                case ProtectionLevels.EProtectionLevel.ProtLevel_Minimum:
                    lblProtMin.ForeColor = Color.Red;
                    break;
                case ProtectionLevels.EProtectionLevel.ProtLevel_Moderate:
                    lblProtMod.ForeColor = Color.Red;
                    break;
                case ProtectionLevels.EProtectionLevel.ProtLevel_High:
                    lblProtHigh.ForeColor = Color.Red;
                    break;
                case ProtectionLevels.EProtectionLevel.ProtLevel_Maximum:
                    lblProtMax.ForeColor = Color.Red;
                    break;
            }

            tbProtLevel.Tag = tbProtLevel.Value;
        }

        void UpdateProtLevelFromSlider()
        {
            ProtectionLevels.SetProtectionLevelInConfig((ProtectionLevels.EProtectionLevel)tbProtLevel.Value);
        }

        private void tbProtLevel_Scroll(object sender, EventArgs e)
        {
            HighlightProtSliderLevel();
            UpdateProtLevelFromSlider();
        }

        private void lvProtProc_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lvProtProc.SelectedItems.Count == 0)
            {
                lblAEPLevel.Text = "Anti-Exploit Protection Level (<select a process>):";
                return;
            }

            string proc = lvProtProc.SelectedItems[0].Text;
            string xmlFile = proc.Replace(".exe", "_config.xml");

            lblAEPLevel.Text = "Anti-Exploit Protection Level (" + proc + "):";

            if (File.Exists(EventManager.InstallPath + xmlFile))
            {
                if (!LoadConfigOrShowError(xmlFile))
                    return;

                string uiProtLevel = XmlConfig.GetConfigString("configuration/defenseInDepth/property/uiProtLevel");
                int level = int.Parse(uiProtLevel);

                tbProtLevel.Value = level;
                HighlightProtSliderLevel();
                
                radUseCustom.Checked = true;
            }
            else
            {
                if (XmlConfig.Path == null || XmlConfig.Path.EndsWith("_config.xml"))
                {
                    if (!LoadConfigOrShowError("config.xml"))
                        return;
                }

                radUseDefault.Checked = true;
                HighlightProtSliderLevel();
            }
        }

        private void lblProtMin_MouseHover(object sender, EventArgs e)
        {
            tipUI.Show(
                "Provides bare minimum protection: Use the least invasive and performance intensive settings possible.\r\n" +
                "This setting is not recommended and should be used only if the Moderate level of security and better cause\r\n" +
                "protected applications to not function properly.",
                lblProtMin, 20 * 1000);
        }

        private void lblProtMod_MouseHover(object sender, EventArgs e)
        {
            tipUI.Show(
                "Provides a fair level of protection: The settings configured are adequate to protect against most Internet-based\r\n" +
                "threats however some very effective techniques which are considered slightly more invasive are not employed when\r\n" +
                "operating at this level. It is recommended that the High setting is used instead of Moderate if possible.",
                lblProtMod, 20 * 1000);
        }

        private void lblProtHigh_MouseHover(object sender, EventArgs e)
        {
            tipUI.Show(
                "Provides a strong level of protection: At this level almost all defensive techniques are employed to protect your system.\r\n" +
                "The protection methods which are not enabled are those which are most invasive and most likely to impact the functionality\r\n" +
                "of protected applications. This setting is recommended for most users.",
                lblProtHigh, 20 * 1000);
        }

        private void lblProtMax_MouseHover(object sender, EventArgs e)
        {
            tipUI.Show(
                "Provides the maximum level of protection: This setting ensures that all protection settings are enabled and provides the\r\n" +
                "highest level of defense. This setting may cause some programs to fail to operate correctly, and may interfere with\r\n" +
                "just-in-time compiled applications unless exemptions are added (for example, .NET, Flash and Java applications).",
                lblProtMax, 20 * 1000);
        }

        private void cbExeWhitelist_CheckedChanged(object sender, EventArgs e)
        {
            string xmlPathLast = new FileInfo(XmlConfig.Path).Name;
            if (string.IsNullOrEmpty(xmlPathLast))
                return;

            if (!LoadConfigOrShowError("config.xml"))
                return;

            XmlConfig.SetConfigString("configuration/apiMonitor/property/checkWhitelist", cbExeWhitelist.Checked ? "true" : "false");

            if (!LoadConfigOrShowError(xmlPathLast))
                return;
        }

        private void cbMinimizeToTray_CheckedChanged(object sender, EventArgs e)
        {
            string xmlPathLast = new FileInfo(XmlConfig.Path).Name;
            if (string.IsNullOrEmpty(xmlPathLast))
                return;

            if (!LoadConfigOrShowError("config.xml"))
                return;

            XmlConfig.SetConfigString("configuration/defenseInDepth/property/minimizeToTray", cbMinimizeToTray.Checked ? "true" : "false");
            MainUI.MinimizeToTray = cbMinimizeToTray.Checked;

            if (!LoadConfigOrShowError(xmlPathLast))
                return;
        }

        private void BasicOptions_VisibleChanged(object sender, EventArgs e)
        {
            UpdateUI();
        }

        private void cbPromptOnDotNet_CheckedChanged(object sender, EventArgs e)
        {
            string xmlPathLast = new FileInfo(XmlConfig.Path).Name;
            if (string.IsNullOrEmpty(xmlPathLast))
                return;

            if (!LoadConfigOrShowError("config.xml"))
                return;

            XmlConfig.SetConfigString("configuration/apiMonitor/property/promptRelaxDotnet", cbPromptOnDotNet.Checked ? "true" : "false");
            MainUI.MinimizeToTray = cbMinimizeToTray.Checked;

            if (!LoadConfigOrShowError(xmlPathLast))
                return;
        }

        private void cbPromptUserExe_CheckedChanged(object sender, EventArgs e)
        {
            string xmlPathLast = new FileInfo(XmlConfig.Path).Name;
            if (string.IsNullOrEmpty(xmlPathLast))
                return;

            if (!LoadConfigOrShowError("config.xml"))
                return;

            XmlConfig.SetConfigString("configuration/apiMonitor/property/promptUserExeBlocked", cbPromptUserExe.Checked ? "true" : "false");

            if (!LoadConfigOrShowError(xmlPathLast))
                return;
        }
    }
}
