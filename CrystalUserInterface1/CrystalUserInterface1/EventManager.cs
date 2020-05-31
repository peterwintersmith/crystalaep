// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Threading;
using System.Xml;
using System.Runtime.InteropServices;
using Microsoft.Win32;

namespace CrystalAEPUI
{
    public enum EEventRepository
    {
        AntiSpray = 0,
        ApiFirewall,
        AudioValidator,
        ConnectionMonitor,
        DfnsInDepth,
        HeapMonitor,
        ImageValidator,
        StackMonitor,
        VideoValidator,
        WebAppMonitor
    };
    
    public enum EThreatRiskRating
    {
        Information = 0,
        LowRisk,
        MediumRisk,
        HighRisk
    };

    public enum EEventSortCriteria
    {
        SortByDate = 0,
        SortByDateReverse,
        SortByRisk,
        SortByRiskReverse,
        SortByProcess
    }

    public struct RealtimeLogEntry
    {
        public uint severity;
        public string message;
    }

    public enum EEnabledFeatures : uint
    {
	    InvalidFeature = 0,
	    ContentFilterFeature = 1,
	    APIMonitorFeature = 2,
	    ROPMonitorFeature = 4,
	    AntiSprayFeature  = 8,
	    StackMonitorFeature = 16,
	    HeapMonitorFeature  = 32,
        COMMonitorFeature   = 64
    }

    static class NativeMethods
    {
        // c++ def: DWORD DllGetEnabledFeaturesProcess(DWORD dwProcessID)
#if DEBUG
        [DllImport(@"C:\Work\crystal\BP\Impl\ProcTrackerClient\Debug\ProcTrackerClient.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
        private static extern uint DllGetEnabledFeaturesProcess(uint dwProcessID);
#else
        [DllImport(@"ProcTrackerClient.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
        private static extern uint DllGetEnabledFeaturesProcess(uint dwProcessID);
#endif

        public static EEnabledFeatures[] GetProcessFeatures(uint pid)
        {
            List<EEnabledFeatures> featuresList = new List<EEnabledFeatures>();

            uint features = DllGetEnabledFeaturesProcess(pid);
            if (features == 0)
                return null;

            foreach (uint f in Enum.GetValues(typeof(EEnabledFeatures)))
            {
                if ((f & features) != 0)
                {
                    featuresList.Add((EEnabledFeatures)f);
                }
            }

            if (featuresList.Count == 0)
                return null;

            return featuresList.ToArray();
        }

        // c++ def: DWORD DllGetProcessLogs(DWORD dwProcessID, DWORD cMaxLogEntries, RealtimeLogEntry rg_RtLogEnts[  ])
#if DEBUG
        [DllImport(@"C:\Work\crystal\BP\Impl\ProcTrackerClient\Debug\ProcTrackerClient.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
        private static extern uint DllGetProcessLogs(uint dwProcessID, uint cMaxLogEntries, IntPtr rg_RtLogEnts);
#else
        [DllImport(@"ProcTrackerClient.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.Cdecl)]
        private static extern uint DllGetProcessLogs(uint dwProcessID, uint cMaxLogEntries, IntPtr rg_RtLogEnts);
#endif
        public const int REALTIME_LOG_ENTRY_SIZE_BYTES = (4 + (2 * 256));

        public static RealtimeLogEntry[] GetProcessLogs(uint pid)
        {
            List<RealtimeLogEntry> realtimeLogEntries = new List<RealtimeLogEntry>();

            IntPtr p_rg = Marshal.AllocHGlobal(100 * REALTIME_LOG_ENTRY_SIZE_BYTES);
            if (p_rg == null)
                return null;

            uint count = DllGetProcessLogs(pid, (uint)100, p_rg);
            if (count == 0)
            {
                Marshal.FreeHGlobal(p_rg);
                return null;
            }

            byte[] logEntriesAll = new byte[count * REALTIME_LOG_ENTRY_SIZE_BYTES];

            Marshal.Copy(p_rg, logEntriesAll, 0, (int)count * REALTIME_LOG_ENTRY_SIZE_BYTES);
            Marshal.FreeHGlobal(p_rg);

            for (uint i = 0; i < count; i++)
            {
                uint severity = (uint)BitConverter.ToInt32(logEntriesAll, (int)i * REALTIME_LOG_ENTRY_SIZE_BYTES);

                byte[] buf = new byte[REALTIME_LOG_ENTRY_SIZE_BYTES - 4];
                Array.Copy(logEntriesAll, (int)(i * REALTIME_LOG_ENTRY_SIZE_BYTES) + 4, buf, 0, REALTIME_LOG_ENTRY_SIZE_BYTES - 4);

                string message = ASCIIEncoding.Unicode.GetString(buf);
                message = message.Substring(0, message.IndexOf('\x00'));

                RealtimeLogEntry rle = new RealtimeLogEntry();
                rle.severity = severity;
                rle.message = message;

                realtimeLogEntries.Add(rle);
            }

            return realtimeLogEntries.ToArray();
        }

        // c++ def: DWORD DllGetProcessIDs(DWORD cUniqueProcIDs, char rg_pszUniqueProcID[  ][ 512 ])
#if DEBUG
        [DllImport(@"C:\Work\crystal\BP\Impl\ProcTrackerClient\Debug\ProcTrackerClient.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        private static extern uint DllGetProcessIDs(uint cUniqueProcIDs, IntPtr rg_pszUniqueProcID);
#else
        [DllImport(@"ProcTrackerClient.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        private static extern uint DllGetProcessIDs(uint cUniqueProcIDs, IntPtr rg_pszUniqueProcID);
#endif

        public const int PROCESS_ID_STRING_FIXED_SIZE = 512;
        public static bool errorRaised = false;

        public static string[] GetProcessIDs()
        {
            List<string> procIDsList = new List<string>();
            
            // assume 100 processes max
            byte[] processIdsAll = new byte[100 * PROCESS_ID_STRING_FIXED_SIZE];

            IntPtr p_rg = Marshal.AllocHGlobal(100 * PROCESS_ID_STRING_FIXED_SIZE);
            if(p_rg == null)
                return null;

            uint count = 0;

            try
            {
                count = DllGetProcessIDs(100, p_rg);
                if (count == 0)
                {
                    Marshal.FreeHGlobal(p_rg);
                    return null;
                }
            }
            catch(DllNotFoundException)
            {
                if (!errorRaised)
                    MessageBox.Show("Monitoring of protected processes will not operate correctly at the ProcTrackerClient.dll cannot be found in the Crystal install directory.");
                
                errorRaised = true;

                return null;
            }

            Marshal.Copy(p_rg, processIdsAll, 0, (int)count * PROCESS_ID_STRING_FIXED_SIZE);
            Marshal.FreeHGlobal(p_rg);

            for (uint i = 0; i < count; i++)
            {
                byte[] buf = new byte[512];
                Array.Copy(processIdsAll, (int)i * PROCESS_ID_STRING_FIXED_SIZE, buf, 0, 512);
                
                string procId = ASCIIEncoding.ASCII.GetString(buf);
                procIDsList.Add(procId.Substring(0, procId.IndexOf('\x00')));
            }

            return procIDsList.ToArray();
        }
    }

    static class EventManager
    {
        public static void Initialize()
        {
            if (initialized)
                return;

            try
            {
                foreach (string name in RepositoryNames)
                {
                    EEventRepository repos = (EEventRepository)Enum.Parse(typeof(EEventRepository), name);
                    ReadRepositoryEvents(repos);
                }

                fsw.Path = InstallPath;
                fsw.Filter = "*.xml";
                fsw.NotifyFilter = NotifyFilters.LastWrite;
                fsw.Changed += new FileSystemEventHandler(OnChanged);
                fsw.EnableRaisingEvents = true; // start monitoring
            }
            catch
            {
                MessageBox.Show("There is a problem monitoring for new events - the Crystal registry is likely corrupt. Please reinstall the product.");
            }

            initialized = true;
        }

        public static string InstallPath
        {
            get
            {
                // registry lookup for install path

                string installPath = null;
#if DEBUG
                installPath = "C:\\Work\\crystal\\BP\\Impl\\BpCore\\";
#else
                try
                {
                    RegistryKey rkCrystal = Registry.CurrentUser.OpenSubKey("Software\\CrystalBP");
                    if (rkCrystal != null)
                        installPath = rkCrystal.GetValue("InstallPath").ToString();
                }
                catch
                {
                    // nothing to do here
                    MessageBox.Show("The Crystal registry key does not exist. Please re-install.", "Registry Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return "";
                }
#endif
                return installPath + "\\";
            }
        }

        public static string[] RepositoryNames
        {
            get
            {
                lock (reposNamesList)
                {
                    if (reposNamesList.Count == 0)
                    {
                        foreach (string name in Enum.GetNames(typeof(EEventRepository)))
                        {
                            reposNamesList.Add(name);
                        }
                    }
                }

                return reposNamesList.ToArray();
            }
        }

        public delegate void NotifyNewEventDelegate(EEventRepository r, EventItem e, bool isRealtime);
        public static event NotifyNewEventDelegate NotifyNewEvent = null;
        
        private static Guid? ReadAndAddDifference(EEventRepository repos, XmlReader xml)
        {
            Guid? firstNewEvent = null;
            bool foundLastEvent = true;
            
            List<EventItem> eventItems = null;
            EventItem lastEvent = null, newEvent = null;
            
            if (!events.TryGetValue(repos, out eventItems))
            {
                eventItems = new List<EventItem>();
                events.Add(repos, eventItems);
            }

            if (eventItems.Count != 0)
            {
                lastEvent = eventItems[eventItems.Count - 1];
                foundLastEvent = false;
            }

            while (xml.Read())
            {
                if (String.Compare(xml.Name, "event", true) != 0 && newEvent == null)
                    continue;
                
                try
                {
                    switch (xml.NodeType)
                    {
                        case XmlNodeType.Element:

                            string strId = xml.GetAttribute("eventid");
                            if (strId == null)
                                continue;

                            Guid eventId = new Guid(strId);

                            if (foundLastEvent == false)
                            {
                                if (lastEvent.EventGuid.Equals(eventId))
                                {
                                    foundLastEvent = true;
                                }

                                break;
                            }

                            if (!firstNewEvent.HasValue)
                                firstNewEvent = eventId;

                            newEvent = new EventItem();

                            newEvent.EventRepository = repos;
                            DateTime.TryParse(xml.GetAttribute("date"), null, System.Globalization.DateTimeStyles.AdjustToUniversal, out newEvent.EventDate);
                            
                            //date="12/01/2012 22:27:16" (important to parse exact otherwise US dates will only parse 1/3rd of the month!!
                            DateTime.TryParseExact(xml.GetAttribute("date"), "dd/MM/yyyy HH:mm:ss", new System.Globalization.CultureInfo("en-GB").DateTimeFormat, System.Globalization.DateTimeStyles.None, out newEvent.EventDate);
                            
                            newEvent.EventGuid = eventId;
                            newEvent.EventTitle = xml.GetAttribute("title");
                            newEvent.EventRisk = xml.GetAttribute("risk");
                            
                            newEvent.EventProcess = xml.GetAttribute("procid");
                            if (newEvent.EventProcess != null)
                            {
                                int idx = newEvent.EventProcess.IndexOf(']');
                                
                                if (idx != -1 && newEvent.EventProcess[0] == '[')
                                {
                                    newEvent.EventProcess = newEvent.EventProcess.Substring(1, idx - 1);
                                }
                            }

                            break;
                        case XmlNodeType.Text:

                            if (newEvent != null)
                            {
                                newEvent.EventMessage = xml.Value;

                                if (newEvent.Valid)
                                {
                                    eventItems.Add(newEvent);
                                }

                                newEvent = null;
                            }

                            break;
                    }
                }
                catch
                {
                    continue;
                }
            }

            return firstNewEvent;
        }

        public static EEventSortCriteria InitialSortCriterion = EEventSortCriteria.SortByDate;

        private static int CompareEvents(EventItem e1, EventItem e2)
        {
            int comp = 0;

            switch (InitialSortCriterion)
            {
                case EEventSortCriteria.SortByDate:
                    comp = DateTime.Compare(e1.EventDate, e2.EventDate);
                    break;
                case EEventSortCriteria.SortByDateReverse:
                    comp = DateTime.Compare(e2.EventDate, e1.EventDate);
                    break;
                case EEventSortCriteria.SortByRisk:
                    comp = (int)Enum.Parse(typeof(EThreatRiskRating), e1.EventRisk) - (int)Enum.Parse(typeof(EThreatRiskRating), e2.EventRisk);
                    break;
                case EEventSortCriteria.SortByRiskReverse:
                    comp = (int)Enum.Parse(typeof(EThreatRiskRating), e2.EventRisk) - (int)Enum.Parse(typeof(EThreatRiskRating), e1.EventRisk);
                    break;
                case EEventSortCriteria.SortByProcess:
                    comp = String.Compare(e1.EventProcess, e2.EventProcess, true);
                    break;
            }

            return comp;
        }

        private static List<EventItem> SortEvents(List<EventItem> eventsList)
        {
            List<EventItem> items = new List<EventItem>(eventsList);
            items.Sort(new Comparison<EventItem>(CompareEvents));
            return items;
        }

        private static Guid? ReadRepositoryEvents(EEventRepository repos)
        {
            string xmlFile = InstallPath + repos.ToString() + ".xml";
            Guid? lastEvent = null;

            if (!File.Exists(xmlFile))
                return null;

            bool mutexHeld = false;

            try
            {
                eventWriterMutex.WaitOne();
                mutexHeld = true;

                using (FileStream fs = new FileStream(xmlFile, FileMode.Open))
                {
                    using (XmlReader xml = XmlReader.Create(fs))
                    {
                        lastEvent = ReadAndAddDifference(repos, xml);
                    }
                }
            }
            catch (Exception ex)
            {
                // not a valid repository
                //MessageBox.Show(ex.ToString());
            }
            finally
            {
                if (mutexHeld)
                    eventWriterMutex.ReleaseMutex();
            }

            return lastEvent;
        }
        
        private static void OnChanged(object sender, FileSystemEventArgs e)
        {
            if (e.ChangeType != WatcherChangeTypes.Changed)
                return;

            int idx = e.Name.IndexOf("config.xml", StringComparison.InvariantCultureIgnoreCase);
            if (idx != -1)
                return;

            idx = e.Name.IndexOf(".xml", StringComparison.InvariantCultureIgnoreCase);
            if (idx == -1)
                return;

            string name = e.Name.Substring(0, idx);

            EEventRepository repos;

            try
            {
                repos = (EEventRepository)Enum.Parse(typeof(EEventRepository), name, true);
            }
            catch
            {
                return;
            }

            Guid? lastEvent = ReadRepositoryEvents(repos);
            bool foundLast = false;

            if (lastEvent.HasValue)
            {
                foreach (EventItem evt in events[repos])
                {
                    if (!foundLast && evt.EventGuid != lastEvent)
                        continue;

                    foundLast = true;

                    if (NotifyNewEvent != null)
                        NotifyNewEvent(repos, evt, true);
                }
            }
        }

        public static List<EventItem> AllEvents
        {
            get
            {
                List<EventItem> listAllEvents = new List<EventItem>();

                foreach (List<EventItem> eventsList in events.Values)
                {
                    listAllEvents.AddRange(eventsList);
                }

                listAllEvents = SortEvents(listAllEvents);

                return listAllEvents;
            }
        }

        public static Dictionary<EEventRepository, List<EventItem>> EventByRepository
        {
            get
            {
                if (events.Count == 0)
                {
                    Initialize();
                }

                return events;
            }
        }

        private static FileSystemWatcher fsw = new FileSystemWatcher();
        private static Dictionary<EEventRepository, List<EventItem>> events = new Dictionary<EEventRepository, List<EventItem>>();
        private static List<string> reposNamesList = new List<string>();
        private static bool initialized = false;
        private static Mutex eventWriterMutex = new Mutex(false, "EventWriterSyncMutex");
    }

    public class EventItem
    {
        public EventItem()
        {
        }

        public EventItem(EventItem e)
        {
            EventDate = e.EventDate;
            EventGuid = e.EventGuid;
            EventProcess = e.EventProcess;
            EventRisk = e.EventRisk;
            EventTitle = e.EventTitle;
            EventMessage = e.EventMessage;
        }

        public bool Valid
        {
            get
            {
                if (EventDate == default(DateTime) || EventGuid == default(Guid) ||
                    EventRisk == null || EventProcess == null || EventTitle == null ||
                    EventMessage == null || !EventRepository.HasValue)
                {
                    return false;
                }

                return true;
            }
        }

        public EEventRepository? EventRepository = null;
        public DateTime EventDate = default(DateTime);
        public Guid   EventGuid   = default(Guid);
        public string EventProcess = null;
        public string EventRisk    = null;
        public string EventTitle   = null;
        public string EventMessage = null;
    }
}
