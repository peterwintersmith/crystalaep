// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Drawing;

namespace CrystalAEPUI
{
    public class WatchedProcess
    {
        public WatchedProcess(int id, string name)
        {
            m_pid = id;
            m_name = name;
        }

        public int PID
        {
            get
            {
                return m_pid;
            }
        }

        public string ExecutableName
        {
            get
            {
                return m_name;
            }
        }

        public string MainWindowTitle
        {
            get
            {
                string title = null;
                Process proc = null;

                try
                {
                    proc = Process.GetProcessById(m_pid);
                    title = proc.MainWindowTitle;
                    proc.Close();
                }
                catch (Exception)
                {
                    // process cannot be opened
                }

                return title;
            }
        }

        public Icon AssociatedIcon
        {
            get
            {
                Icon icon = null;
                Process proc = null;

                try
                {
                    proc = Process.GetProcessById(m_pid);
                    icon = Icon.ExtractAssociatedIcon(proc.MainModule.FileName);
                    proc.Close();
                }
                catch(Exception)
                {
                    // process cannot be opened
                }

                return icon;
            }
        }

        public FileVersionInfo FileVersion
        {
            get
            {
                FileVersionInfo fvi = null;
                Process proc = null;

                try
                {
                    proc = Process.GetProcessById(m_pid);
                    fvi = FileVersionInfo.GetVersionInfo(proc.MainModule.FileName);
                    proc.Close();
                }
                catch(Exception)
                {
                    // process cannot be opened
                }

                return fvi;
            }
        }

        private int m_pid = 0;
        private string m_name = null;
    }

    public static class MonitoredProcesses
    {
        public static void Initialize()
        {
            if (init)
                return;

            refreshThread.Start();

            init = true;
        }

        private static Thread refreshThread = new Thread(new ParameterizedThreadStart(RefreshProcesses));

        public static string ProcTrackerExeName
        {
            get
            {
                return "ProcTracker.exe";
            }
        }

        public static string ProcTrackerExePath
        {
            get
            {
                string dir = EventManager.InstallPath; // read from registry
                return dir + ProcTrackerExeName;
            }
        }

        public static Dictionary<string, WatchedProcess> Processes
        {
            get
            {
                lock (listlock)
                {
                    pauseUpdate = true;
                }

                return monitoredProcs;
            }
        }

        public static void Resume()
        {
            lock (listlock)
            {
                pauseUpdate = false;
            }
        }

        private static bool pauseUpdate = false;
        private static bool terminateThread = false;

        public static void TerminateThread()
        {
            terminateThread = true;
        }

        private static void RefreshProcesses(object param)
        {
            while (!terminateThread)
            {
                Thread.Sleep(1000);

                try
                {
                    lock (listlock)
                    {
                        if (pauseUpdate)
                            continue;
                    }

                    string[] procIDs = NativeMethods.GetProcessIDs();
                    if (procIDs == null || procIDs.Length == 0)
                    {
                        monitoredProcs.Clear();
                        continue;
                    }

                    List<string> removeKeys = new List<string>();

                    foreach (KeyValuePair<string, WatchedProcess> kvp in monitoredProcs)
                    {
                        bool found = false;

                        foreach (string procID in procIDs)
                        {
                            if (String.CompareOrdinal(procID, kvp.Key) == 0)
                            {
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            removeKeys.Add(kvp.Key);
                        }
                    }

                    if (removeKeys.Count != 0)
                    {
                        foreach (string s in removeKeys)
                        {
                            RemoveProcessByProcessIDString(s);
                        }
                    }

                    WatchedProcess wp = null;

                    foreach (string procID in procIDs)
                    {
                        if (!monitoredProcs.TryGetValue(procID, out wp))
                        {
                            AddProcessByProcessIDString(procID);
                        }
                    }
                }
                catch
                {
                    // blah
                }
            }
        }

        private static void RemoveProcessByProcessIDString(string procIDString)
        {
            WatchedProcess wp = null;

            lock (listlock)
            {
                if (!monitoredProcs.TryGetValue(procIDString, out wp))
                    return;

                monitoredProcs.Remove(procIDString);
            }
        }

        private static void AddProcessByProcessIDString(string procIDString)
        {
            string name = null;
            int pid = 0;

            if (procIDString[0] != '[')
                return;

            int idx = procIDString.IndexOf(']');
            if (idx == -1)
                return;

            name = procIDString.Substring(1, idx - 1);

            idx = procIDString.IndexOf('[', idx + 1);
            if (idx == -1)
                return;

            int eidx = procIDString.IndexOf(']', idx);
            if (eidx == -1)
                return;

            string tmp = procIDString.Substring(idx + 1, eidx - idx - 1);
            if (!int.TryParse(tmp, out pid))
                return;

            try
            {
                lock (listlock)
                {
                    monitoredProcs.Add(procIDString, new WatchedProcess(pid, name));
                }
            }
            catch
            {
                return;
            }
        }
        
        private static object listlock = new object();
        private static Dictionary<string, WatchedProcess> monitoredProcs = new Dictionary<string, WatchedProcess>();
        private static bool init = false;
    }
}
