// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace CrystalAEPUI
{
    public partial class Filters : Form
    {
        public Filters()
        {
            InitializeComponent();
        }

        private void lbConnEnabledFilters_Click(object sender, EventArgs e)
        {
            if (lbConnEnabledFilters.SelectedIndex != -1)
            {
                lbConnDisabledFilters.SelectedIndex = -1;
                btnConnEnableDisable.Text = "Disable";
            }
        }

        private void lbConnDisabledFilters_Click(object sender, EventArgs e)
        {
            if (lbConnDisabledFilters.SelectedIndex != -1)
            {
                lbConnEnabledFilters.SelectedIndex = -1;
                btnConnEnableDisable.Text = "Enable";
            }
        }

        private void lbOutEnabledFilters_Click(object sender, EventArgs e)
        {
            if (lbOutEnabledFilters.SelectedIndex != -1)
            {
                lbOutDisabledFilters.SelectedIndex = -1;
                btnOutEnableDisable.Text = "Disable";
            }
        }

        private void lbOutDisabledFilters_Click(object sender, EventArgs e)
        {
            if (lbOutDisabledFilters.SelectedIndex != -1)
            {
                lbOutEnabledFilters.SelectedIndex = -1;
                btnOutEnableDisable.Text = "Enable";
            }
        }

        private void lbInEnabledFilters_Click(object sender, EventArgs e)
        {
            if (lbInEnabledFilters.SelectedIndex != -1)
            {
                lbInDisabledFilters.SelectedIndex = -1;
                btnInEnableDisable.Text = "Disable";
            }
        }

        private void lbInDisabledFilters_Click(object sender, EventArgs e)
        {
            if (lbInDisabledFilters.SelectedIndex != -1)
            {
                lbInEnabledFilters.SelectedIndex = -1;
                btnInEnableDisable.Text = "Enable";
            }
        }

        private bool SavePendingSettings()
        {
            bool allSaved = true;

            foreach (KeyValuePair<string, string> kvp in pendingSettings)
            {
                if (XmlConfig.GetConfigString(kvp.Key) == null)
                {
                    string keyAlternate = null;

                    if (kvp.Key.Contains("/Enabled:"))
                    {
                        // the entry may exist in the xml config as Disabled ...
                        keyAlternate = kvp.Key.Replace("/Enabled:", "/Disabled:");
                    }
                    else if (kvp.Key.Contains("/Disabled:"))
                    {
                        // as may the opposite occur ...
                        keyAlternate = kvp.Key.Replace("/Disabled:", "/Enabled:");
                    }

                    NaiveConfigWriter writer = new NaiveConfigWriter(XmlConfig.Path);

                    if (keyAlternate != null)
                    {
                        if (XmlConfig.GetConfigString(keyAlternate) != null)
                        {
                            // remove keyAlternate and continue
                            writer.RemoveXmlPropertyWithPath(keyAlternate);
                        }
                    }

                    // this config entry doesn't exist (i.e. a new imported filter)
                    // add new entry to config file before proceeding.

                    int idx = kvp.Key.LastIndexOf('/');

                    // this code should be tested later
                    string xmlPropertyPath = kvp.Key.Substring(0, idx);
                    string xmlPropertyName = kvp.Key.Substring(idx + 1);

                    writer.AddXmlPropertyAtPath(xmlPropertyPath, xmlPropertyName, kvp.Value);

                    // then update config
                    if (!XmlConfig.RefreshConfig())
                    {
                        ShowConfigLoadError();
                        return false;
                    }
                }
                else
                {
                    if (!XmlConfig.SetConfigString(kvp.Key, kvp.Value))
                    {
                        allSaved = false;
                    }
                }
            }

            pendingSettings.Clear();

            return allSaved;
        }

        private void ResetFormChanges()
        {
            lbConnDisabledFilters.Items.Clear();
            lbConnEnabledFilters.Items.Clear();

            lbOutDisabledFilters.Items.Clear();
            lbOutEnabledFilters.Items.Clear();

            lbInDisabledFilters.Items.Clear();
            lbInEnabledFilters.Items.Clear();

            pendingSettings.Clear();

            if (!UpdateUIFromConfig())
            {
                ShowConfigLoadError();
                return;
            }
        }

        public bool ConfigChanged
        {
            get;
            set;
        }

        private void Filters_FormClosing(object sender, FormClosingEventArgs e)
        {
            ConfigChanged = false;

            if (pendingSettings.Count != 0)
            {
                DialogResult dlgResult = MessageBox.Show(
                    "There appear to have been changes to the filter settings. Would you like to save these changes?",
                    "Confirm Changes",
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Question);

                if (dlgResult == System.Windows.Forms.DialogResult.Yes)
                {
                    try
                    {
                        ConfigChanged = true;

                        if (!SavePendingSettings())
                        {
                            MessageBox.Show(
                                "Not all of the changes saved successfully - please reset the config. and try again",
                                "Error Saving Changes",
                                MessageBoxButtons.OK);
                        }
                    }
                    catch
                    {
                        // not much to do here
                        ResetFormChanges();
                    }
                }
                else
                {
                    ResetFormChanges();
                }
            }

            Hide();

            if (!MainUI.ShuttingDownProcess)
                e.Cancel = true;
        }

        void ShowConfigLoadError()
        {
            string configXmlName = "config.xml";

            DialogResult dlgResult = MessageBox.Show(
                "There has been an error processing \"" + configXmlName + "\". Please press OK to restore configuration.",
                "Error",
                MessageBoxButtons.OKCancel,
                MessageBoxIcon.Error);

            if (dlgResult == System.Windows.Forms.DialogResult.OK)
            {
                //restore config from config.last
            }
        }

        private void Filters_Shown(object sender, EventArgs e)
        {
            ResetFormChanges();
        }

        private bool AddFilterModule(string filterFile)
        {
            // ensure DLL file
            //
            // pinvoke: BOOL filter!QueryFilters(
		    //     char ***pppszConnectExports, int *pnConnectExports,
		    //     char ***pppszRequestExports, int *pnRequestExports,
		    //     char ***pppszResponseExports, int *pnResponseExports
	        // );
            //
            // if(BOOL result == true) add exports to XML config, update UI (and update options dialog UI)
            //
            // return BOOL result

            string[] connectFilters = null, requestFilters = null, responseFilters = null;

            bool result = FilterNativeMethods.QueryFilters(filterFile,
                out connectFilters, out requestFilters, out responseFilters
                );

            if (!result)
                return false;

            // we can add the filters

            NaiveConfigWriter writer = new NaiveConfigWriter(XmlConfig.Path);

            if (connectFilters != null)
            {
                foreach (string filter in connectFilters)
                {
                    if (XmlConfig.GetConfigString("configuration/connectFilters/property/Enabled:" + filter) == null)
                    {
                        writer.AddXmlPropertyAtPath("configuration/connectFilters/property", "Enabled:" + filter, filterFile);
                    }
                    else
                    {
                        XmlConfig.SetConfigString("configuration/connectFilters/property/Enabled:" + filter, filterFile);
                    }

                    if (XmlConfig.GetConfigString("configuration/connectFilters/property/Disabled:" + filter) != null)
                    {
                        writer.RemoveXmlPropertyWithPath("configuration/connectFilters/property/Disabled:" + filter);
                    }
                }
            }

            if (requestFilters != null)
            {
                foreach (string filter in requestFilters)
                {
                    if (XmlConfig.GetConfigString("configuration/requestFilters/property/Enabled:" + filter) == null)
                    {
                        writer.AddXmlPropertyAtPath("configuration/requestFilters/property", "Enabled:" + filter, filterFile);
                    }
                    else
                    {
                        XmlConfig.SetConfigString("configuration/requestFilters/property/Enabled:" + filter, filterFile);
                    }

                    if (XmlConfig.GetConfigString("configuration/requestFilters/property/Disabled:" + filter) != null)
                    {
                        writer.RemoveXmlPropertyWithPath("configuration/requestFilters/property/Disabled:" + filter);
                    }
                }
            }

            if (responseFilters != null)
            {
                foreach (string filter in responseFilters)
                {
                    if (XmlConfig.GetConfigString("configuration/responseFilters/property/Enabled:" + filter) == null)
                    {
                        writer.AddXmlPropertyAtPath("configuration/responseFilters/property", "Enabled:" + filter, filterFile);
                    }
                    else
                    {
                        XmlConfig.SetConfigString("configuration/responseFilters/property/Enabled:" + filter, filterFile);
                    }

                    if (XmlConfig.GetConfigString("configuration/responseFilters/property/Disabled:" + filter) != null)
                    {
                        writer.RemoveXmlPropertyWithPath("configuration/responseFilters/property/Disabled:" + filter);
                    }
                }
            }

            return true;
        }

        private void btnImport_Click(object sender, EventArgs e)
        {
            opendlgFilters.Filter = "Filter Modules (*.dll)|*.dll";
            
            DialogResult dlgResult = opendlgFilters.ShowDialog();
            
            if (dlgResult == System.Windows.Forms.DialogResult.OK && opendlgFilters.FileName.Length > 0)
            {
                if (!AddFilterModule(opendlgFilters.FileName))
                {
                    MessageBox.Show(
                        "There was an error importing the filter module - please ensure that you have chosen the correct module and try again.",
                        "Error Importing Filters",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Error);
                }

                if (!UpdateUIFromConfig())
                {
                    ShowConfigLoadError();
                    return;
                }
            }
        }

        bool UpdateUIFromConfig()
        {
            if (!XmlConfig.RefreshConfig())
                return false;

            XmlConfig.NameValuePair[] nvpArray = XmlConfig.GetNameValuePairArray("configuration/connectFilters/property/");
            if (nvpArray != null)
            {
                lbConnEnabledFilters.Items.Clear();
                lbConnEnabledFilters.BeginUpdate();
                lbConnEnabledFilters.HorizontalScrollbar = true;

                foreach (XmlConfig.NameValuePair nvpair in nvpArray)
                {
                    string name = XmlConfig.GetNameFromPath(nvpair.Name);

                    if (name.StartsWith("Enabled:", true, null))
                    {
                        lbConnEnabledFilters.Items.Add(name.Substring(8) + " => " + nvpair.Value);
                    }
                    else if (name.StartsWith("Disabled:", true, null))
                    {
                        lbConnDisabledFilters.Items.Add(name.Substring(9) + " => " + nvpair.Value);
                    }
                }

                lbConnEnabledFilters.EndUpdate();
            }
            
            nvpArray = XmlConfig.GetNameValuePairArray("configuration/requestFilters/property/");
            if (nvpArray != null)
            {
                lbOutEnabledFilters.Items.Clear();
                lbOutEnabledFilters.BeginUpdate();
                lbOutEnabledFilters.HorizontalScrollbar = true;

                foreach (XmlConfig.NameValuePair nvpair in nvpArray)
                {
                    string name = XmlConfig.GetNameFromPath(nvpair.Name);

                    if (name.StartsWith("Enabled:", true, null))
                    {
                        lbOutEnabledFilters.Items.Add(name.Substring(8) + " => " + nvpair.Value);
                    }
                    else if (name.StartsWith("Disabled:", true, null))
                    {
                        lbOutDisabledFilters.Items.Add(name.Substring(9) + " => " + nvpair.Value);
                    }
                }

                lbOutEnabledFilters.EndUpdate();
            }

            nvpArray = XmlConfig.GetNameValuePairArray("configuration/responseFilters/property/");
            if (nvpArray != null)
            {
                lbInEnabledFilters.Items.Clear();
                lbInEnabledFilters.BeginUpdate();
                lbInEnabledFilters.HorizontalScrollbar = true;

                foreach (XmlConfig.NameValuePair nvpair in nvpArray)
                {
                    string name = XmlConfig.GetNameFromPath(nvpair.Name);

                    if (name.StartsWith("Enabled:", true, null))
                    {
                        lbInEnabledFilters.Items.Add(name.Substring(8) + " => " + nvpair.Value);
                    }
                    else if (name.StartsWith("Disabled:", true, null))
                    {
                        lbInDisabledFilters.Items.Add(name.Substring(9) + " => " + nvpair.Value);
                    }
                }

                lbInEnabledFilters.EndUpdate();
            }

            return true;
        }

        private void Filters_Load(object sender, EventArgs e)
        {
            // anything to do here?
        }

        private void btnConnEnableDisable_Click(object sender, EventArgs e)
        {
            if (btnConnEnableDisable.Text == "Disable")
            {
                if (lbConnEnabledFilters.SelectedIndex == -1)
                    return;

                string[] sep = { " => " };
                string[] filterInfo = ((string)lbConnEnabledFilters.Items[lbConnEnabledFilters.SelectedIndex]).Split(sep, 2, StringSplitOptions.None);
                
                if (filterInfo.Length != 2)
                    return;

                lbConnDisabledFilters.Items.Insert(0, (string)lbConnEnabledFilters.Items[lbConnEnabledFilters.SelectedIndex]);
                lbConnEnabledFilters.Items.RemoveAt(lbConnEnabledFilters.SelectedIndex);

                if (pendingSettings.ContainsKey("configuration/connectFilters/property/Enabled:" + filterInfo[0]))
                {
                    pendingSettings.Remove("configuration/connectFilters/property/Enabled:" + filterInfo[0]);
                }

                pendingSettings["configuration/connectFilters/property/Disabled:" + filterInfo[0]] = filterInfo[1];
            }
            else if (btnConnEnableDisable.Text == "Enable")
            {
                if (lbConnDisabledFilters.SelectedIndex == -1)
                    return;

                string[] sep = { " => " };
                string[] filterInfo = ((string)lbConnDisabledFilters.Items[lbConnDisabledFilters.SelectedIndex]).Split(sep, 2, StringSplitOptions.None);

                if (filterInfo.Length != 2)
                    return;

                lbConnEnabledFilters.Items.Insert(0, (string)lbConnDisabledFilters.Items[lbConnDisabledFilters.SelectedIndex]);
                lbConnDisabledFilters.Items.RemoveAt(lbConnDisabledFilters.SelectedIndex);

                if (pendingSettings.ContainsKey("configuration/connectFilters/property/Disabled:" + filterInfo[0]))
                {
                    pendingSettings.Remove("configuration/connectFilters/property/Disabled:" + filterInfo[0]);
                }

                pendingSettings["configuration/connectFilters/property/Enabled:" + filterInfo[0]] = filterInfo[1];
            }
        }

        Dictionary<string, string> pendingSettings = new Dictionary<string, string>();
        
        private void btnOutEnableDisable_Click(object sender, EventArgs e)
        {
            if (btnOutEnableDisable.Text == "Disable")
            {
                if (lbOutEnabledFilters.SelectedIndex == -1)
                    return;

                string[] sep = { " => " };
                string[] filterInfo = ((string)lbOutEnabledFilters.Items[lbOutEnabledFilters.SelectedIndex]).Split(sep, 2, StringSplitOptions.None);

                if (filterInfo.Length != 2)
                    return;

                lbOutDisabledFilters.Items.Insert(0, (string)lbOutEnabledFilters.Items[lbOutEnabledFilters.SelectedIndex]);
                lbOutEnabledFilters.Items.RemoveAt(lbOutEnabledFilters.SelectedIndex);

                if (pendingSettings.ContainsKey("configuration/requestFilters/property/Enabled:" + filterInfo[0]))
                {
                    pendingSettings.Remove("configuration/requestFilters/property/Enabled:" + filterInfo[0]);
                }

                pendingSettings["configuration/requestFilters/property/Disabled:" + filterInfo[0]] = filterInfo[1];
            }
            else if (btnOutEnableDisable.Text == "Enable")
            {
                if (lbOutDisabledFilters.SelectedIndex == -1)
                    return;

                string[] sep = { " => " };
                string[] filterInfo = ((string)lbOutDisabledFilters.Items[lbOutDisabledFilters.SelectedIndex]).Split(sep, 2, StringSplitOptions.None);

                if (filterInfo.Length != 2)
                    return;

                lbOutEnabledFilters.Items.Insert(0, (string)lbOutDisabledFilters.Items[lbOutDisabledFilters.SelectedIndex]);
                lbOutDisabledFilters.Items.RemoveAt(lbOutDisabledFilters.SelectedIndex);

                if (pendingSettings.ContainsKey("configuration/requestFilters/property/Disabled:" + filterInfo[0]))
                {
                    pendingSettings.Remove("configuration/requestFilters/property/Disabled:" + filterInfo[0]);
                }

                pendingSettings["configuration/requestFilters/property/Enabled:" + filterInfo[0]] = filterInfo[1];
            }
        }

        private void btnInEnableDisable_Click(object sender, EventArgs e)
        {
            if (btnInEnableDisable.Text == "Disable")
            {
                if (lbInEnabledFilters.SelectedIndex == -1)
                    return;

                string[] sep = { " => " };
                string[] filterInfo = ((string)lbInEnabledFilters.Items[lbInEnabledFilters.SelectedIndex]).Split(sep, 2, StringSplitOptions.None);

                if (filterInfo.Length != 2)
                    return;

                lbInDisabledFilters.Items.Insert(0, (string)lbInEnabledFilters.Items[lbInEnabledFilters.SelectedIndex]);
                lbInEnabledFilters.Items.RemoveAt(lbInEnabledFilters.SelectedIndex);

                if (pendingSettings.ContainsKey("configuration/responseFilters/property/Enabled:" + filterInfo[0]))
                {
                    pendingSettings.Remove("configuration/responseFilters/property/Enabled:" + filterInfo[0]);
                }

                pendingSettings["configuration/responseFilters/property/Disabled:" + filterInfo[0]] = filterInfo[1];
            }
            else if (btnInEnableDisable.Text == "Enable")
            {
                if (lbInDisabledFilters.SelectedIndex == -1)
                    return;

                string[] sep = { " => " };
                string[] filterInfo = ((string)lbInDisabledFilters.Items[lbInDisabledFilters.SelectedIndex]).Split(sep, 2, StringSplitOptions.None);

                if (filterInfo.Length != 2)
                    return;

                lbInEnabledFilters.Items.Insert(0, (string)lbInDisabledFilters.Items[lbInDisabledFilters.SelectedIndex]);
                lbInDisabledFilters.Items.RemoveAt(lbInDisabledFilters.SelectedIndex);

                if (pendingSettings.ContainsKey("configuration/responseFilters/property/Disabled:" + filterInfo[0]))
                {
                    pendingSettings.Remove("configuration/responseFilters/property/Disabled:" + filterInfo[0]);
                }

                pendingSettings["configuration/responseFilters/property/Enabled:" + filterInfo[0]] = filterInfo[1];
            }
        }
    }

    public static class FilterNativeMethods
    {
        const uint DONT_RESOLVE_DLL_REFERENCES = 1;

        [DllImport("kernel32.dll")]
        private static extern IntPtr LoadLibraryEx(String dllname, IntPtr handle, uint flags);
        
        [DllImport("kernel32.dll")]
        private static extern IntPtr GetProcAddress(IntPtr hModule, String procname);

        // pinvoke: BOOL filter!QueryFilters(
        //     char ***pppszConnectExports, int *pnConnectExports,
        //     char ***pppszRequestExports, int *pnRequestExports,
        //     char ***pppszResponseExports, int *pnResponseExports
        // );

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        delegate bool QueryFiltersDelegate(
            out IntPtr pppszConnectExports, out int pnConnectExports,
            out IntPtr pppszRequestExports, out int pnRequestExports,
            out IntPtr pppszResponseExports, out int pnResponseExports
            );

        public static bool QueryFilters(string dllPath, out string[] connectFilters, out string[] requestFilters, out string[] responseFilters)
        {
            connectFilters = null;
            requestFilters = null;
            responseFilters = null;

            try
            {
                // DONT_RESOLVE_DLL_REFERENCES is super important, otherwise static initializers are executed
                // for e.g. GetConnectionPool which leads to protection of the UI process (and JIT MSIL is
                // denied execution => crash)

                IntPtr filterModule = LoadLibraryEx(dllPath, IntPtr.Zero, DONT_RESOLVE_DLL_REFERENCES);
                if (filterModule == IntPtr.Zero)
                    return false;

                IntPtr funcQueryFilters = IntPtr.Zero;

                funcQueryFilters = GetProcAddress(filterModule, "QueryFilters");
                if (funcQueryFilters == IntPtr.Zero)
                    return false;

                QueryFiltersDelegate QueryFiltersFunction = (QueryFiltersDelegate)Marshal.GetDelegateForFunctionPointer(funcQueryFilters, typeof(QueryFiltersDelegate));
                if (QueryFiltersFunction == null)
                    return false;

                IntPtr ptrConnect = IntPtr.Zero, ptrRequest = IntPtr.Zero, ptrResponse = IntPtr.Zero;
                int cConnect = 0, cRequest = 0, cResponse = 0;

                if (!QueryFiltersFunction(out ptrConnect, out cConnect, out ptrRequest, out cRequest, out ptrResponse, out cResponse))
                    return false;

                List<string> connectExports = new List<string>();

                for (int i = 0; i < cConnect; i++)
                {
                    var stringPtr = (IntPtr)Marshal.PtrToStructure(ptrConnect, typeof(IntPtr));
                    connectExports.Add(Marshal.PtrToStringAnsi(stringPtr));
                    ptrConnect = new IntPtr(ptrConnect.ToInt32() + IntPtr.Size);
                }

                List<string> requestExports = new List<string>();

                for (int i = 0; i < cRequest; i++)
                {
                    var stringPtr = (IntPtr)Marshal.PtrToStructure(ptrConnect, typeof(IntPtr));
                    requestExports.Add(Marshal.PtrToStringAnsi(stringPtr));
                    ptrConnect = new IntPtr(ptrConnect.ToInt32() + IntPtr.Size);
                }

                List<string> responseExports = new List<string>();

                for (int i = 0; i < cResponse; i++)
                {
                    var stringPtr = (IntPtr)Marshal.PtrToStructure(ptrConnect, typeof(IntPtr));
                    responseExports.Add(Marshal.PtrToStringAnsi(stringPtr));
                    ptrConnect = new IntPtr(ptrConnect.ToInt32() + IntPtr.Size);
                }

                connectFilters = connectExports.Count == 0 ? null : connectExports.ToArray();
                requestFilters = requestExports.Count == 0 ? null : requestExports.ToArray();
                responseFilters = responseExports.Count == 0 ? null : responseExports.ToArray();
                
                return true;
            }
            catch
            {
                // nothing to do
            }

            return false;
        }
    }
}
