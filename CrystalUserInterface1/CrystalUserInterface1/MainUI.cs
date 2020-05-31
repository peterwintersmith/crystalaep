// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using Microsoft.Win32;
using System.Security.AccessControl;

namespace CrystalAEPUI
{
    public partial class MainUI : Form
    {
        [DllImport(@"kernel32.dll", CharSet = CharSet.Unicode)]
        private static extern bool SetDllDirectoryW(string lpPathName);

        public MainUI()
        {
            InitializeComponent();

            SetDllDirectoryW(EventManager.InstallPath);

            EventManager.NotifyNewEvent += DisplayNewEvent;
            EventManager.Initialize();
            MonitoredProcesses.Initialize();
            RealtimeLog.NewLogEntryMethod += DisplayRealtimeLogEvent;
            RealtimeLog.Initialize();
        }

        AlertsBrowser ab = new AlertsBrowser();

        private void alertsBrowserToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ab.Show();
        }

        private void RefreshProcesses(object sender, EventArgs args)
        {
            try
            {
                WatchedProcess wp = null;
                List<TreeNode> removeTn = new List<TreeNode>();

                foreach (TreeNode tn in tvMonProc.Nodes)
                {
                    if (!MonitoredProcesses.Processes.TryGetValue((string)tn.Tag, out wp))
                    {
                        removeTn.Add(tn);
                        break;
                    }
                }

                if (removeTn.Count != 0)
                {
                    foreach (TreeNode tn in removeTn)
                    {
                        tvMonProc.Nodes.Remove(tn);
                        tvMonProc.ImageList.Images.RemoveByKey((string)tn.Tag);
                        
                        if (tn.Tag != null)
                        {
                            try
                            {
                                string strPid = (string)tn.Tag;
                                int idx = strPid.IndexOf("][");
                                
                                strPid = strPid.Substring(checked(idx + 2));
                                idx = strPid.IndexOf("]");
                                strPid = strPid.Substring(0, idx);

                                int pid = int.Parse(strPid);

                                if (perProcessRtLogs.ContainsKey(pid))
                                {
                                    perProcessRtLogs.Remove(pid);
                                }
                            }
                            catch
                            {
                                continue;
                            }
                        }
                    }
                }

                foreach (KeyValuePair<string, WatchedProcess> kvp in MonitoredProcesses.Processes)
                {
                    bool found = false;
                    TreeNode refTn = null;

                    foreach (TreeNode tn in tvMonProc.Nodes)
                    {
                        if (String.Compare((string)tn.Tag, kvp.Key, true) == 0)
                        {
                            refTn = tn;
                            found = true;
                            break;
                        }
                    }

                    string displayStr = null;
                    string windowTitle = kvp.Value.MainWindowTitle;
                    
                    string strDesc = "";

                    if (kvp.Value.FileVersion == null || kvp.Value.FileVersion.FileDescription == null)
                    {
                        strDesc = "No description";
                    }
                    else
                    {
                        strDesc = kvp.Value.FileVersion.FileDescription;
                    }

                    string strVer = null;

                    if (kvp.Value.FileVersion == null || kvp.Value.FileVersion.FileVersion == null)
                    {
                        strVer = "No version";
                    }
                    else
                    {
                        strVer = kvp.Value.FileVersion.FileVersion;
                    }

                    if (String.IsNullOrEmpty(kvp.Value.MainWindowTitle))
                    {
                        displayStr = string.Format("{0} ({1})", strDesc, kvp.Value.ExecutableName);
                    }
                    else
                    {
                        int fitsChars = tvMonProc.Width / 7;

                        if (windowTitle.Length >= fitsChars)
                            displayStr = windowTitle.Substring(0, fitsChars) + "...";
                        else
                            displayStr = windowTitle;
                    }

                    string tooltipStr = string.Format("Title: {0}\nDescription: {1}\nVersion: {2}\nProcess ID: {3}\n",
                        kvp.Value.MainWindowTitle ?? "(no title)",
                        strDesc,
                        strVer,
                        kvp.Value.PID);
                    
                    if (found)
                    {
                        if (String.Compare(refTn.Text, displayStr, true) != 0)
                        {
                            refTn.Text = displayStr;
                            refTn.ToolTipText = tooltipStr;
                        }
                    }
                    else
                    {
                        TreeNode tn = tvMonProc.Nodes.Add(displayStr);
                        Icon icon = kvp.Value.AssociatedIcon;

                        if (icon != null)
                        {
                            tvMonProc.ImageList.Images.Add(kvp.Key, icon);
                            tn.ImageKey = kvp.Key;
                        }

                        tn.ToolTipText = tooltipStr;
                        tn.Tag = kvp.Key;
                    }
                }

                MonitoredProcesses.Resume();
            }
            catch (Exception ex)
            {
                //MessageBox.Show(ex.ToString());
                //throw;
            }
        }

        void HandleCheckChangeAttempt(object sender, EventArgs e)
        {
            CheckBox checkBoxSelf = (CheckBox)sender;
            
            if (checkBoxSelf != null)
            {
                if ((string)checkBoxSelf.Tag == "ignore")
                    return;

                if ((string)checkBoxSelf.Tag != "internal")
                {
                    checkBoxSelf.Tag = "ignore";
                    checkBoxSelf.Checked = !checkBoxSelf.Checked;
                }

                checkBoxSelf.Tag = null;
            }
        }

        bool EnsureProcTrackerRunning()
        {
            Process[] rgProcTracker = Process.GetProcessesByName(MonitoredProcesses.ProcTrackerExeName);
            if(rgProcTracker == null || rgProcTracker.Length == 0)
            {
                if (!File.Exists(MonitoredProcesses.ProcTrackerExePath))
                {
                    return false;
                }

                Process procTracker = Process.Start(MonitoredProcesses.ProcTrackerExePath);
                if (procTracker == null)
                {
                    return false;
                }

                System.Threading.Thread.Sleep(1000);
            }

            return true;
        }

        private Timer refreshProcTimer = new Timer();

        private bool CheckAndQueryAppInitKey(bool addCrystal, bool removeCrystal)
        {
            RegistryKey rkNtCvWindows = Registry.LocalMachine.OpenSubKey("Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows", false);
            if (rkNtCvWindows == null)
            {
                MessageBox.Show("Crystal is unable to access the registry HKLM hive. Please run the user interface as an administrative user.",
                    "Error Accessing Registry",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);

                return false;
            }

            string strAppInitDlls = null;

            try
            {
                strAppInitDlls = rkNtCvWindows.GetValue("AppInit_DLLs").ToString();
            }
            catch
            {
                // any exception is bad news
            }

            if (strAppInitDlls == null)
            {
                MessageBox.Show("Crystal is unable to access the registry. Please run the user interface as an administrative user.",
                    "Error Accessing Registry",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Error);

                return false;
            }

            bool foundDll = false;

            string[] dllNames = strAppInitDlls.Split(new string[] { " " }, StringSplitOptions.RemoveEmptyEntries);
            foreach (string name in dllNames)
            {
                if (String.Compare(name, "crymodldr.dll", true) == 0)
                {
                    foundDll = true;
                    break;
                }
            }

            if (!addCrystal && !removeCrystal)
            {
                // we're just figuring out whether crymodldr.dll is added or not
                return foundDll;
            }

            if (!foundDll)
            {
                try
                {
                    if (addCrystal && !removeCrystal)
                    {
                        //rkNtCvWindows.SetValue("AppInit_DLLs", strAppInitDlls + (strAppInitDlls.Length == 0 ? "" : " ") + "crymodldr.dll");
                        string appInitValue = strAppInitDlls + (strAppInitDlls.Length == 0 ? "" : " ") + "crymodldr.dll";
                        string resetArgs = "-set-appinit " + string.Format("\"{0}\"", appInitValue);

                        Process proc = Process.Start(EventManager.InstallPath + "reset.exe", resetArgs);
                        proc.WaitForExit();

                        if (proc.ExitCode != 0)
                            return false;
                    }
                }
                catch
                {
                    // can't set value
                    MessageBox.Show("Crystal is unable to set the AppInit registry key (under HKLM). Please run the user interface as an administrative user.",
                        "Error Accessing Registry",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Error);

                    return false;
                }
            }
            else
            {
                try
                {
                    if (removeCrystal && !addCrystal)
                    {
                        // a bit of a naive way of removing the key
                        strAppInitDlls = strAppInitDlls.Replace("crymodldr.dll ", "").Replace(" crymodldr.dll", "").Replace("crymodldr.dll", "");
                        //rkNtCvWindows.SetValue("AppInit_DLLs", strAppInitDlls);

                        string appInitValue = strAppInitDlls;
                        string resetArgs = "-set-appinit " + string.Format("\"{0}\"", appInitValue);

                        Process proc = Process.Start(EventManager.InstallPath + "reset.exe", resetArgs);
                        proc.WaitForExit();

                        if (proc.ExitCode != 0)
                            return false;

                    }
                }
                catch
                {
                    // can't set value
                    MessageBox.Show("Crystal is unable to set the AppInit registry key (under HKLM). Please run the user interface as an administrative user.",
                        "Error Accessing Registry",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Error);

                    return false;
                }
            }
            
            int loadAppInitDLLs = 0;

            try
            {
                loadAppInitDLLs = (int)rkNtCvWindows.GetValue("LoadAppInit_DLLs", 0);
            }
            catch
            {

            }

            if (loadAppInitDLLs == 0)
            {
                try
                {
                    //rkNtCvWindows.SetValue("LoadAppInit_DLLs", 1);
                    
                    Process proc = Process.Start(EventManager.InstallPath + "reset.exe", "-enable-appinit");
                    proc.WaitForExit();

                    if (proc.ExitCode != 0)
                        return false;
                }
                catch
                {
                    // can't set value
                    MessageBox.Show("Crystal is unable to set the Load AppInit registry key (under HKLM). Please run the user interface as an administrative user.",
                        "Error Accessing Registry",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Error);

                    return false;
                }
            }

            return true;
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

        private void CheckUACAndIECompat()
        {
            RegistryKey rkUAC = Registry.LocalMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", false);
            if(rkUAC == null)
                return;

            int uacEnabled = (int)rkUAC.GetValue("EnableLUA", 0);
            if(uacEnabled == 1)
            {
                RegistryKey rkProtMode = Registry.CurrentUser.OpenSubKey("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\3", true);
                if (rkProtMode == null)
                    return;

                int protMode = (int)rkProtMode.GetValue("2500", -1);
                if (protMode == 0)
                {
                    // protected mode and UAC are enabled: warn user
                    DialogResult dr = MessageBox.Show("UAC and IE Protected Mode are both enabled. Crystal cannot protect IE running with this configuration.\r\n\r\n" +
                        "Do you wish to disable IE protected mode (it can be re-enabled via the \"Internet Options/Security\" Control Panel form)?",
                        "Internet Explorer Incompatibility",
                        MessageBoxButtons.YesNo,
                        MessageBoxIcon.Warning);

                    if (dr == System.Windows.Forms.DialogResult.Yes)
                    {
                        rkProtMode.SetValue("2500", 3);
                    }
                }
            }
        }

        private void CheckAndDisplayTutorialFirstLaunch()
        {
            if (File.Exists(EventManager.InstallPath + "tutorial.show"))
            {
                File.Delete(EventManager.InstallPath + "tutorial.show");
                tutForm.Show();
            }
        }

        private System.Threading.Mutex uniqueMutex = null;

        [DllImportAttribute("User32.dll")]
        private static extern IntPtr SetForegroundWindow(IntPtr hWnd);

        private void CheckUniqueInstance()
        {
            bool isNew = false;
            uniqueMutex = new System.Threading.Mutex(true, "CrystalUIMutex", out isNew);

            if (!isNew)
            {
                MessageBox.Show("Crystal is already running. If the software is not being displayed please check the system tray (to the bottom right of your screen) for the grey shield icon.",
                    "Crystal is Already Running",
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Exclamation);

                Process[] procs = Process.GetProcessesByName("CrystalAEPUI");
                
                if (procs != null && procs.Length != 0)
                {
                    Process firstProc = null;
                    DateTime lastStartTime = DateTime.Now;

                    foreach (Process proc in procs)
                    {
                        if (proc.StartTime < lastStartTime)
                        {
                            firstProc = proc;
                            lastStartTime = proc.StartTime;
                        }
                    }

                    SetForegroundWindow(firstProc.MainWindowHandle);

                    MainUI.ShuttingDownProcess = true;
                    //Application.Exit();
                }

                Environment.Exit(0);
            }
        }

        [DllImport("urlmon.dll", CharSet = CharSet.Auto, SetLastError = true)]
        static extern Int32 URLDownloadToFile(
            [MarshalAs(UnmanagedType.IUnknown)] object pCaller,
            [MarshalAs(UnmanagedType.LPWStr)] string szURL,
            [MarshalAs(UnmanagedType.LPWStr)] string szFileName,
            Int32 dwReserved,
            IntPtr lpfnCB);

        private void CheckForUpdatesAlertUser(object param)
        {
            bool alertOnFail = (bool)param;

            lock (updateSync)
            {
                try
                {
                    string storedVersion = null;

                    try
                    {
                        storedVersion = File.ReadAllText(EventManager.InstallPath + "version.info");
                    }
                    catch
                    {
                        storedVersion = "1.0.0.0";
                    }

                    Version storedVer = new Version(storedVersion);

                    Int32 ret = URLDownloadToFile(null, "https://www.crystalaep.com/version.info", EventManager.InstallPath + "version.new", 0, IntPtr.Zero);
                    if(ret != 0)
                        ret = URLDownloadToFile(null, "http://www.crystalaep.com/version.info", EventManager.InstallPath + "version.new", 0, IntPtr.Zero);

                    if (ret == 0) // S_OK
                    {
                        string strVersion = File.ReadAllText(EventManager.InstallPath + "version.new");
                        Version newVer = new Version(strVersion);

                        if (storedVer < newVer)
                        {
                            DialogResult dr = MessageBox.Show("A new version of Crystal AEP is available. Would you like to download and install? " +
                                "This will require a restart. If you choose not to install now you will be prompted next time.",
                                "Update Available", MessageBoxButtons.YesNo, MessageBoxIcon.Question);

                            if (dr == System.Windows.Forms.DialogResult.Yes)
                            {
                                if (File.Exists(EventManager.InstallPath + "update.exe"))
                                {
                                    File.Delete(EventManager.InstallPath + "update.exe");
                                }

                                ret = URLDownloadToFile(null, "https://www.crystalaep.com/update.bin", EventManager.InstallPath + "update.exe", 0, IntPtr.Zero);
                                if (ret != 0)
                                {
                                    dr = MessageBox.Show(
                                        "Crystal AEP was not able to contact the update site over HTTPS (secure HTTP) would you like to try regular HTTP?",
                                        "Try again?",
                                        MessageBoxButtons.YesNo,
                                        MessageBoxIcon.Question);

                                    if (dr == System.Windows.Forms.DialogResult.Yes)
                                    {
                                        ret = URLDownloadToFile(null, "http://www.crystalaep.com/update.bin", EventManager.InstallPath + "update.exe", 0, IntPtr.Zero);
                                    }
                                }

                                if (ret == 0)
                                {
                                    //File.Copy(EventManager.InstallPath + "version.new", EventManager.InstallPath + "version.info", true);
                                    //File.Delete(EventManager.InstallPath + "version.new");
                                    /*
                                    string updateArgs = "-sched-update ";
                                    
                                    if (EventManager.InstallPath.Contains(" "))
                                    {
                                        updateArgs += String.Format("\"{0}\"", EventManager.InstallPath.Substring(0, EventManager.InstallPath.Length - 1));
                                    }
                                    else
                                    {
                                        updateArgs += EventManager.InstallPath.Substring(0, EventManager.InstallPath.Length - 1);
                                    }

                                    Process.Start(EventManager.InstallPath + "reset.exe", updateArgs);
                                    Process.Start(EventManager.InstallPath + "reset.exe", "-remove");
                                    */

                                    Process p = Process.Start(EventManager.InstallPath + "update.exe");
                                    p.WaitForExit();

                                    MessageBox.Show("Update downloaded and run successfully. Please save your work and re-start your computer to complete installation.");
                                }
                                else
                                {
                                    MessageBox.Show("Unfortunately the new version could not be downloaded. Please update from www.crystalaep.com manually.");
                                }
                            }
                        }
                        else
                        {
                            if (alertOnFail)
                            {
                                MessageBox.Show("There are no updates available at present. The current version (v" + storedVer + ") is the latest.",
                                    "No Updates Available", MessageBoxButtons.OK, MessageBoxIcon.Information);
                            }
                        }
                    }
                    else
                    {
                        if (alertOnFail)
                        {
                            MessageBox.Show("Unfortunately Crystal could not check for updates. Please ensure you are connected to the Internet and try again.",
                                "No Updates Available", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        }
                    }
                }
                catch
                {
                    // do nothing
                }
            }
        }

        private object updateSync = new object();

        private void MainUI_Load(object sender, EventArgs e)
        {
            RefEnableCF = cbEnableCF;
            RefEnableDiD = cbEnableDiD;

            CheckUniqueInstance();
            EnsureProcTrackerRunning();
            
            System.Threading.ParameterizedThreadStart param = new System.Threading.ParameterizedThreadStart(CheckForUpdatesAlertUser);
            System.Threading.Thread thdUpdateCheck = new System.Threading.Thread(param);
            thdUpdateCheck.Start(false);
            
            lvRealtime.Font = new Font("Calibri", 9f, FontStyle.Regular);
            lvRealtime.Scrollable = true;
            lvRealtime.View = View.Details;
            lvRealtime.ShowItemToolTips = true;
            lvRealtime.FullRowSelect = true;

            lvLatestAlerts.Font = new Font("Calibri", 9f, FontStyle.Regular);
            lvLatestAlerts.Scrollable = true;
            lvLatestAlerts.View = View.Details;
            lvLatestAlerts.ShowItemToolTips = true;
            lvLatestAlerts.FullRowSelect = true;

            EventManager.InitialSortCriterion = EEventSortCriteria.SortByDate;
            foreach (EventItem evt in EventManager.AllEvents)
            {
                DisplayNewEvent(evt.EventRepository.Value, evt, false);
            }

            tvMonProc.Font = new Font("Calibri", 9f, FontStyle.Regular);
            tvMonProc.ImageList = new ImageList();
            tvMonProc.ShowNodeToolTips = true;

            refreshProcTimer.Tick += new EventHandler(RefreshProcesses);
            refreshProcTimer.Interval = 1000;
            refreshProcTimer.Start();

            lvLatestAlerts.Columns[2].Width = lvLatestAlerts.Width - (lvLatestAlerts.Columns[0].Width + lvLatestAlerts.Columns[1].Width + 20);
            
            cbAntiSpray.CheckedChanged += new EventHandler(HandleCheckChangeAttempt);
            cbApiMonitor.CheckedChanged += new EventHandler(HandleCheckChangeAttempt);
            cbContentFilter.CheckedChanged += new EventHandler(HandleCheckChangeAttempt);
            cbHeapMonitor.CheckedChanged += new EventHandler(HandleCheckChangeAttempt);
            cbRopMonitor.CheckedChanged += new EventHandler(HandleCheckChangeAttempt);
            cbStackMonitor.CheckedChanged += new EventHandler(HandleCheckChangeAttempt);
            cbComMonitor.CheckedChanged += new EventHandler(HandleCheckChangeAttempt);

            if (!XmlConfig.OpenConfig(EventManager.InstallPath + "config.xml"))
            {
                ShowConfigLoadError("config.xml");
                return;
            }
            else
            {
                if (!XmlConfig.ReadConfig())
                {
                    ShowConfigLoadError("config.xml");
                    return;
                }
                else
                {
                    string uiProtLevel = XmlConfig.GetConfigString("configuration/defenseInDepth/property/uiProtLevel");
                    if (uiProtLevel != null)
                    {
                        int tmp = 0;

                        if (!int.TryParse(uiProtLevel, out tmp))
                            tbProtLevel.Value = 2;

                        tbProtLevel.Value = tmp;
                    }
                    else
                    {
                        tbProtLevel.Value = 2;
                    }

                    tbProtLevel.Tag = tbProtLevel.Value;

                    undergoingUIUpdate = true;

                    cbEnableCF.Checked = XmlConfig.GetConfigBool("configuration/browserProtection/property/enabled");
                    cbEnableDiD.Checked = XmlConfig.GetConfigBool("configuration/defenseInDepth/property/enabled");

                    undergoingUIUpdate = false;

                    // IE + ProtectedMode + UAC does not work
                    // nb: fixed in didcore.dll (it elevates IE if elevation is enabled)
                    // CheckUACAndIECompat();
                }
            }

            XmlConfig.Close();

            HighlightProtSliderLevel();

            if (CheckAndQueryAppInitKey(false, false) == true)
            {
                RenderDisableAllButton(false);
                minUi.SetEnableStatePicture(true);
            }
            else
            {
                RenderDisableAllButton(true);
                minUi.SetEnableStatePicture(false);
            }

            this.Size = new Size(0, 0);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;

            MinimizeToTray = XmlConfig.GetConfigBool("configuration/defenseInDepth/property/minimizeToTray");

            CheckAndDisplayTutorialFirstLaunch();
        }

        private delegate void DisplayNewEventDelegate(EEventRepository r, EventItem e, bool isRealtime);
        private object displaylock = new object();
        private bool displayHighRiskOnly = true;

        public static bool MinimizeToTray = true;

        private void DisplayNewEvent(EEventRepository r, EventItem e, bool isRealtime)
        {
            try
            {
                if (this.lvLatestAlerts.InvokeRequired)
                {
                    lvLatestAlerts.Invoke(new DisplayNewEventDelegate(DisplayNewEvent), new object[] { r, e, isRealtime });
                }
                else
                {
                    lock (displaylock)
                    {
                        EThreatRiskRating risk = (EThreatRiskRating)Enum.Parse(typeof(EThreatRiskRating), e.EventRisk, true);
                        Color riskColor = Color.LightGray;

                        if (displayHighRiskOnly)
                        {
                            if (risk != EThreatRiskRating.HighRisk && risk != EThreatRiskRating.MediumRisk)
                                return;
                        }

                        switch (risk)
                        {
                            case EThreatRiskRating.Information:
                                riskColor = Color.LightGray;
                                break;
                            case EThreatRiskRating.LowRisk:
                                riskColor = Color.LightGreen;
                                break;
                            case EThreatRiskRating.MediumRisk:
                                riskColor = Color.Orange;
                                break;
                            case EThreatRiskRating.HighRisk:
                                riskColor = Color.OrangeRed;
                                break;
                        }

                        string date = e.EventDate.ToString().Substring(0, e.EventDate.ToString().IndexOf(' '));
                        string time = e.EventDate.ToString().Substring(e.EventDate.ToString().IndexOf(' ') + 1);

                        ListViewItem lvi = new ListViewItem(new string[] { date, time, e.EventTitle });
                        lvi.BackColor = riskColor;
                        lvi.Tag = e;
                        lvi.ToolTipText = string.Format("Date/Time: {0} - {1}\nRisk: {2}\nTitle: {3}\nMessage: {4}\n", date, time, e.EventRisk, e.EventTitle, e.EventMessage);

                        lvLatestAlerts.Items.Insert(0, lvi);

                        if (lvLatestAlerts.Items.Count > 250)
                            lvLatestAlerts.Items.RemoveAt(250);

                        if (isRealtime && risk == EThreatRiskRating.HighRisk)
                        {
                            IndividualAlert ia = new IndividualAlert();
                            ia.ShowEvent(e);
                            ia.Show();
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private delegate void DisplayRealtimeLogEventDelegate(RealtimeLogEntry rle);
        private object logDisplayLock = new object();

        private void DisplayRealtimeLogEvent(RealtimeLogEntry rle)
        {
            try
            {
                if (this.lvRealtime.InvokeRequired)
                {
                    this.lvRealtime.Invoke(new DisplayRealtimeLogEventDelegate(DisplayRealtimeLogEvent), new object[] { rle });
                }
                else
                {
                    lock (logDisplayLock)
                    {
                        ListViewItem lvi = new ListViewItem(new string[] { rle.message });
                        Color logColor = Color.LightGray;

                        switch (rle.severity)
                        {
                            case 1:
                                logColor = Color.Red;
                                break;
                            default:
                                break;
                        }

                        lvi.BackColor = logColor;
                        
                        lvRealtime.Items.Insert(0, lvi);

                        if (lvRealtime.Items.Count > 250)
                            lvRealtime.Items.RemoveAt(250);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void DblClick(object sender, EventArgs e)
        {
            EventItem evt = (EventItem)lvLatestAlerts.SelectedItems[0].Tag;
            if (evt == null)
                return;

            IndividualAlert ia = new IndividualAlert();
            ia.ShowEvent(evt);
            ia.Show();
        }

        public static bool ShuttingDownProcess = false;

        private MinimizedUI minUi = new MinimizedUI();

        private void MainUI_FormClosing(object sender, FormClosingEventArgs e)
        {
            // moved to minui:
              //MonitoredProcesses.TerminateThread();
              //RealtimeLog.TerminateThread();

            minUi.MainUIInst = this;
            minUi.Show();

            if (!MainUI.ShuttingDownProcess)
                e.Cancel = true;

            Hide();
        }

        private void RefreshEnabledFeatures(EEnabledFeatures[] features)
        {
            if (features == null)
                return;

            foreach (EEnabledFeatures f in features)
            {
                CheckBox featureCb = null;

                switch (f)
                {
                    case EEnabledFeatures.AntiSprayFeature:
                        featureCb = cbAntiSpray;
                        break;
                    case EEnabledFeatures.APIMonitorFeature:
                        featureCb = cbApiMonitor;
                        break;
                    case EEnabledFeatures.ContentFilterFeature:
                        featureCb = cbContentFilter;
                        break;
                    case EEnabledFeatures.HeapMonitorFeature:
                        featureCb = cbHeapMonitor;
                        break;
                    case EEnabledFeatures.ROPMonitorFeature:
                        featureCb = cbRopMonitor;
                        break;
                    case EEnabledFeatures.StackMonitorFeature:
                        featureCb = cbStackMonitor;
                        break;
                    case EEnabledFeatures.COMMonitorFeature:
                        featureCb = cbComMonitor;
                        break;
                }

                featureCb.Tag = "internal";
                featureCb.Checked = true;
                featureCb.Tag = null;
            }
        }

        private Dictionary<int, ListViewItem[]> perProcessRtLogs = new Dictionary<int, ListViewItem[]>();
        private delegate void AddAlertListViewItemArrayDelegate(ListViewItem[] rgLvi);

        private void AddAlertListViewItemArray(ListViewItem[] rgLvi)
        {
            if (lvRealtime.InvokeRequired)
            {
                lvLatestAlerts.Invoke(new AddAlertListViewItemArrayDelegate(AddAlertListViewItemArray), new object[] { rgLvi });
            }
            else
            {
                lvRealtime.Items.AddRange(rgLvi);
            }
        }

        private void tvMonProc_AfterSelect(object sender, TreeViewEventArgs e)
        {
            // preserve selected image icon
            tvMonProc.SelectedImageKey = (string)tvMonProc.Nodes[tvMonProc.SelectedNode.Index].Tag;

            TreeNode tn = tvMonProc.Nodes[tvMonProc.SelectedNode.Index];
            WatchedProcess wp = null;

            if (!MonitoredProcesses.Processes.TryGetValue((string)tn.Tag, out wp))
            {
                return;
            }

            // update enabled features checkboxes
            RefreshEnabledFeatures(NativeMethods.GetProcessFeatures((uint)wp.PID));

            // update realtime log
            RealtimeLog.Pause();

            // save off the currently displayed logs for the previously selected process
            if (lvRealtime.Items.Count != 0)
            {
                ListViewItem[] rgSavedtems = new ListViewItem[lvRealtime.Items.Count];
                lvRealtime.Items.CopyTo(rgSavedtems, 0);
                perProcessRtLogs[(int)RealtimeLog.SelectedPid] = rgSavedtems;
            }

            // reload the previously displayed logs for the currently selected process
            lvRealtime.Items.Clear();

            if (perProcessRtLogs.ContainsKey(wp.PID))
            {
                ListViewItem[] rgSavedtems = perProcessRtLogs[wp.PID];
                AddAlertListViewItemArray(rgSavedtems);
            }
            
            RealtimeLog.SelectedPid = (uint)wp.PID;
            
            RealtimeLog.Resume();
        }

        private void cbRisk_CheckedChanged(object sender, EventArgs e)
        {
            displayHighRiskOnly = !displayHighRiskOnly;

            lvLatestAlerts.Items.Clear();

            EventManager.InitialSortCriterion = EEventSortCriteria.SortByDate;

            foreach (EventItem evt in EventManager.AllEvents)
            {
                if (evt.EventRepository.HasValue)
                {
                    DisplayNewEvent(evt.EventRepository.Value, evt, false);
                }
            }

            lvLatestAlerts.Refresh();
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult dr = MessageBox.Show("Do you wish to exit Crystal AEP? Unless disabled, protection will continue even if the application is closed.", "Exit Program?", MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            if (dr == System.Windows.Forms.DialogResult.Yes)
            {
                MonitoredProcesses.TerminateThread();
                RealtimeLog.TerminateThread();
                MainUI.ShuttingDownProcess = true;
                Application.Exit();
            }
        }

        ExpertOptions optsDialog = new ExpertOptions();

        private void expertOptionsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            optsDialog.Show();
        }

        private void btnShowMore_Click(object sender, EventArgs e)
        {
            if (btnShowMore.Tag == null)
            {
                this.MaximumSize = new Size(596, 680);
                this.MinimumSize = new Size(596, 680);
                this.Size = new Size(588, 682);

                btnShowMore.Text = "Less ...";
                btnShowMore.Tag = "Maximized";
            }
            else
            {
                this.MaximumSize = new Size(596, 470);
                this.MinimumSize = new Size(596, 470);
                this.Size = new Size(596, 470);

                btnShowMore.Text = "More ...";
                btnShowMore.Tag = null;
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
            if (optsDialog.Visible)
            {
                MessageBox.Show("Please close the options dialog before using the protection level slider.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                tbProtLevel.Value = (int)tbProtLevel.Tag;
                return;
            }

            HighlightProtSliderLevel();
            UpdateProtLevelFromSlider();
        }

        private void RenderDisableAllButton(bool enabled)
        {
            if (enabled)
            {
                btnDisableAll.Text = "Enable All";
                btnDisableAll.ForeColor = Color.Green;
                btnDisableAll.Tag = "Disabled";

                lblProtState.Text = "Your System is not Currently Protected";
                lblProtState.ForeColor = Color.Red;
            }
            else
            {
                btnDisableAll.Text = "Disable All";
                btnDisableAll.ForeColor = Color.Red;
                btnDisableAll.Tag = null;

                lblProtState.Text = "Your System is Protected";
                lblProtState.ForeColor = Color.Blue;
            }
        }

        private void btnDisableAll_Click(object sender, EventArgs e)
        {
            if (btnDisableAll.Tag == null)
            {
                // remove crymodldr.dll from registry
                RenderDisableAllButton(true);
                minUi.SetEnableStatePicture(false);
                CheckAndQueryAppInitKey(false, true);
            }
            else
            {
                // add back to registry (use existing fn)
                RenderDisableAllButton(false);
                minUi.SetEnableStatePicture(true);
                CheckAndQueryAppInitKey(true, false);
            }
        }

        private void cbEnableCF_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            XmlConfig.SetConfigString("configuration/browserProtection/property/enabled", cbEnableCF.Checked ? "true" : "false");

            if (ExpertOptions.RefEnableBpCore != null)
                ExpertOptions.RefEnableBpCore.Checked = cbEnableCF.Checked;
        }

        private void cbEnableDiD_CheckedChanged(object sender, EventArgs e)
        {
            if (undergoingUIUpdate)
                return;

            XmlConfig.SetConfigString("configuration/defenseInDepth/property/enabled", cbEnableDiD.Checked ? "true" : "false");

            if (ExpertOptions.RefEnableDiDCore != null)
                ExpertOptions.RefEnableDiDCore.Checked = cbEnableDiD.Checked;
        }

        BasicOptions ba = new BasicOptions();
        private bool warnedSlowBALoad = false;

        private void basicOptionsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!warnedSlowBALoad)
            {
                DialogResult dr = MessageBox.Show("The Basic Options form needs to generate a list of installed software. This may take a minute or so to complete. Do you wish to continue?\n",
                    "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Question);

                if (dr == System.Windows.Forms.DialogResult.Yes)
                {
                    warnedSlowBALoad = true;
                }
            }

            if (warnedSlowBALoad)
            {
                ba.Show();
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

        private void informationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            HelpUI helpUI = new HelpUI();
            helpUI.Show();
        }

        private void MainUI_Shown(object sender, EventArgs e)
        {
            //minUi.MainUIInst = this;
            //minUi.Show();
            //Hide();

            Hide();

            Size = new Size(596, 470);
            MinimumSize = new Size(596, 470);
            MaximumSize = new Size(596, 470);
            FormBorderStyle = System.Windows.Forms.FormBorderStyle.Sizable;

            minUi.MainUIInst = this;
            minUi.Show();
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            About about = new About();
            about.Show();
        }

        private void updateCrystalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            System.Threading.ParameterizedThreadStart param = new System.Threading.ParameterizedThreadStart(CheckForUpdatesAlertUser);
            System.Threading.Thread thdUpdateCheck = new System.Threading.Thread(param);
            thdUpdateCheck.Start(true);
        }

        private void usersGuideToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (File.Exists(EventManager.InstallPath + "CrystalUsersGuide.pdf"))
            {
                ProcessStartInfo psi = new ProcessStartInfo(EventManager.InstallPath + "CrystalUsersGuide.pdf");
                psi.Verb = "open";
                Process.Start(psi);
            }
        }

        Tutorial tutForm = new Tutorial();

        private void tutorialToolStripMenuItem_Click(object sender, EventArgs e)
        {
            tutForm.Show();
        }

        public static CheckBox RefEnableCF = null;
        public static CheckBox RefEnableDiD = null;
    }
}
