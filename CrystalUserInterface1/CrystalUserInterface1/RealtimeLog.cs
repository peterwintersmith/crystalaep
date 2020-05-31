// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Drawing;

namespace CrystalAEPUI
{
    public static class RealtimeLog
    {
        public delegate void NewLogEntryDelegate(RealtimeLogEntry rle);

        public static event NewLogEntryDelegate NewLogEntryMethod = null;
        public static uint SelectedPid { get; set; }

        public static void Initialize()
        {
            if (init)
                return;

            refreshThread.Start();

            init = true;
        }

        private static Thread refreshThread = new Thread(new ParameterizedThreadStart(RefreshRealtimeLog));

        private static void RefreshRealtimeLog(object param)
        {
            while (!terminateThread)
            {
                Thread.Sleep(1000);

                if (NewLogEntryMethod == null || SelectedPid == 0 || pauseUpdate == true)
                    continue;

                lock (updateLock)
                {
                    RealtimeLogEntry[] rleArray = NativeMethods.GetProcessLogs(SelectedPid);

                    if (rleArray != null)
                    {
                        foreach (RealtimeLogEntry rle in rleArray)
                        {
                            if (pauseUpdate)
                                break;

                            NewLogEntryMethod(rle);
                        }
                    }
                }
            }
        }

        public static void Pause()
        {
            pauseUpdate = true;
        }

        public static void Resume()
        {
            pauseUpdate = false;
        }

        private static bool pauseUpdate = false;

        public static void TerminateThread()
        {
            terminateThread = true;
        }

        private static bool terminateThread = false;
        private static bool init = false;
        private static object updateLock = new object();
    }
}
