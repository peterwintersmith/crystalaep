// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.Text;

namespace CrystalAEPUI
{
    static class ProtectionLevels
    {
        // build presets - path/value - for various defense levels

        public enum EProtectionLevel
        {
            ProtLevel_Minimum = 0,
            ProtLevel_Moderate,
            ProtLevel_High,
            ProtLevel_Maximum
        }

        private static Dictionary<string, string>[] protLevelPresets = new Dictionary<string, string>[4]
        {
            // Minimal
            new Dictionary<string, string>(){
                {"configuration/defenseInDepth/property/uiProtLevel", "0"},
                {"configuration/browserProtection/property/enabled", "false"},
                {"configuration/memoryMonitor/property/enabled", "false"},
                {"configuration/apiMonitor/property/enabled", "false"},
                {"configuration/activeXMonitor/property/enabled", "false"}
            },
            
            // Moderate
            new Dictionary<string, string>(){
                {"configuration/defenseInDepth/property/uiProtLevel", "1"},
                //{"configuration/browserProtection/property/enabled", "false"},
                {"configuration/memoryMonitor/property/enabled", "true"},
                {"configuration/memoryMonitor/property/varyAllocSize", "true"},
                {"configuration/memoryMonitor/property/terminateCorruptHeaps", "true"},
                {"configuration/memoryMonitor/property/winValidateAlloc", "false"},
                {"configuration/memoryMonitor/property/customValidateAlloc", "false"},
                {"configuration/memoryMonitor/property/antiUseAfterFree", "false"},
                {"configuration/memoryMonitor/property/antiDoubleFree", "false"},
                {"configuration/memoryMonitor/property/processDepEnabled", "false"},
                {"configuration/memoryMonitor/property/wipeAllocsOnFree", "true"},
                {"configuration/memoryMonitor/property/removeExecuteHeaps", "false"},
                {"configuration/apiMonitor/property/enabled", "true"},
                {"configuration/apiMonitor/property/allowCodeFromTemp", "false"},
                {"configuration/apiMonitor/property/allowCodeFromNetwork", "false"},
                {"configuration/apiMonitor/property/allowCodeLongPath", "true"},
                {"configuration/apiMonitor/property/allowCodeFromInetTemp", "true"},
                {"configuration/apiMonitor/property/allowCodeFromDownloads", "true"},
                {"configuration/apiMonitor/property/blockDllPlanting", "false"},
                {"configuration/apiMonitor/property/enableAntiROPStack", "false"},
                {"configuration/apiMonitor/property/disableRWXVAMemory", "false"},
                {"configuration/apiMonitor/property/disableRWXStackMemory", "true"},
                {"configuration/apiMonitor/property/monitorAPICallOrigins", "false"},
                {"configuration/apiMonitor/property/injectIntoChildProc", "false"},
                {"configuration/apiMonitor/property/randomizeAllocBase", "false"},
                {"configuration/apiMonitor/property/exceptHandlerChecks", "true"},
                {"configuration/apiMonitor/property/emulateSEHHandler", "false"},
                {"configuration/apiMonitor/property/promptElevateLowInteg", "false"},
                {"configuration/apiMonitor/property/elevateLowIntegApps", "false"},
                {"configuration/apiMonitor/property/promptUserExeBlocked", "true"},
                {"configuration/apiMonitor/property/checkWhitelist", "true"},
                {"configuration/apiMonitor/property/allowSpawnProtectedProcs", "true"},
                {"configuration/apiMonitor/property/promptRelaxDotnet", "false"},
                {"configuration/apiMonitor/property/checkROPReturn", "false"},
                {"configuration/apiMonitor/property/autoAddHookDllRWX", "false"},
                {"configuration/activeXMonitor/property/enabled", "false"},
                {"configuration/activeXMonitor/property/clsidBlacklistEnabled", "true"},
                {"configuration/activeXMonitor/property/clsidWhitelistEnabled", "false"}
            },
            
            // High
            new Dictionary<string, string>(){
                {"configuration/defenseInDepth/property/uiProtLevel", "2"},
                //{"configuration/browserProtection/property/enabled", "true"},
                {"configuration/memoryMonitor/property/enabled", "true"},
                {"configuration/memoryMonitor/property/varyAllocSize", "true"},
                {"configuration/memoryMonitor/property/terminateCorruptHeaps", "true"},
                {"configuration/memoryMonitor/property/winValidateAlloc", "false"},
                {"configuration/memoryMonitor/property/customValidateAlloc", "true"},
                {"configuration/memoryMonitor/property/antiUseAfterFree", "false"},
                {"configuration/memoryMonitor/property/antiDoubleFree", "false"},
                {"configuration/memoryMonitor/property/processDepEnabled", "true"},
                {"configuration/memoryMonitor/property/wipeAllocsOnFree", "true"},
                {"configuration/memoryMonitor/property/removeExecuteHeaps", "true"},
                {"configuration/apiMonitor/property/enabled", "true"},
                {"configuration/apiMonitor/property/allowCodeFromTemp", "false"},
                {"configuration/apiMonitor/property/allowCodeFromNetwork", "false"},
                {"configuration/apiMonitor/property/allowCodeLongPath", "false"},
                {"configuration/apiMonitor/property/allowCodeFromInetTemp", "true"},
                {"configuration/apiMonitor/property/allowCodeFromDownloads", "true"},
                {"configuration/apiMonitor/property/blockDllPlanting", "true"},
                {"configuration/apiMonitor/property/enableAntiROPStack", "true"},
                {"configuration/apiMonitor/property/disableRWXVAMemory", "false"},
                {"configuration/apiMonitor/property/disableRWXStackMemory", "true"},
                {"configuration/apiMonitor/property/monitorAPICallOrigins", "true"},
                {"configuration/apiMonitor/property/injectIntoChildProc", "true"},
                {"configuration/apiMonitor/property/randomizeAllocBase", "false"},
                {"configuration/apiMonitor/property/exceptHandlerChecks", "true"},
                {"configuration/apiMonitor/property/emulateSEHHandler", "false"},
                {"configuration/apiMonitor/property/promptElevateLowInteg", "false"},
                {"configuration/apiMonitor/property/elevateLowIntegApps", "false"},
                {"configuration/apiMonitor/property/promptUserExeBlocked", "true"},
                {"configuration/apiMonitor/property/checkWhitelist", "true"},
                {"configuration/apiMonitor/property/allowSpawnProtectedProcs", "true"},
                {"configuration/apiMonitor/property/promptRelaxDotnet", "false"},
                {"configuration/apiMonitor/property/checkROPReturn", "true"},
                {"configuration/apiMonitor/property/autoAddHookDllRWX", "false"},
                {"configuration/activeXMonitor/property/enabled", "false"},
                {"configuration/activeXMonitor/property/clsidBlacklistEnabled", "true"},
                {"configuration/activeXMonitor/property/clsidWhitelistEnabled", "false"}
            },
            
            // Maximum
            new Dictionary<string, string>(){
                {"configuration/defenseInDepth/property/uiProtLevel", "3"},
                //{"configuration/browserProtection/property/enabled", "true"},
                {"configuration/memoryMonitor/property/enabled", "true"},
                {"configuration/memoryMonitor/property/varyAllocSize", "true"},
                {"configuration/memoryMonitor/property/terminateCorruptHeaps", "true"},
                {"configuration/memoryMonitor/property/winValidateAlloc", "false"},
                {"configuration/memoryMonitor/property/customValidateAlloc", "true"},
                {"configuration/memoryMonitor/property/antiUseAfterFree", "true"},
                {"configuration/memoryMonitor/property/antiDoubleFree", "true"},
                {"configuration/memoryMonitor/property/processDepEnabled", "true"},
                {"configuration/memoryMonitor/property/wipeAllocsOnFree", "true"},
                {"configuration/memoryMonitor/property/removeExecuteHeaps", "true"},
                {"configuration/apiMonitor/property/enabled", "true"},
                {"configuration/apiMonitor/property/allowCodeFromTemp", "false"},
                {"configuration/apiMonitor/property/allowCodeFromNetwork", "false"},
                {"configuration/apiMonitor/property/allowCodeLongPath", "false"},
                {"configuration/apiMonitor/property/allowCodeFromInetTemp", "false"},
                {"configuration/apiMonitor/property/allowCodeFromDownloads", "false"},
                {"configuration/apiMonitor/property/blockDllPlanting", "true"},
                {"configuration/apiMonitor/property/enableAntiROPStack", "true"},
                {"configuration/apiMonitor/property/disableRWXVAMemory", "true"},
                {"configuration/apiMonitor/property/disableRWXStackMemory", "true"},
                {"configuration/apiMonitor/property/monitorAPICallOrigins", "true"},
                {"configuration/apiMonitor/property/injectIntoChildProc", "true"},
                {"configuration/apiMonitor/property/randomizeAllocBase", "true"},
                {"configuration/apiMonitor/property/exceptHandlerChecks", "true"},
                {"configuration/apiMonitor/property/emulateSEHHandler", "true"},
                {"configuration/apiMonitor/property/promptElevateLowInteg", "false"},
                {"configuration/apiMonitor/property/elevateLowIntegApps", "false"},
                {"configuration/apiMonitor/property/promptUserExeBlocked", "true"},
                {"configuration/apiMonitor/property/checkWhitelist", "true"},
                {"configuration/apiMonitor/property/allowSpawnProtectedProcs", "true"},
                {"configuration/apiMonitor/property/promptRelaxDotnet", "true"},
                {"configuration/apiMonitor/property/checkROPReturn", "true"},
                {"configuration/apiMonitor/property/autoAddHookDllRWX", "true"},
                {"configuration/activeXMonitor/property/enabled", "true"},
                {"configuration/activeXMonitor/property/clsidBlacklistEnabled", "false"},
                {"configuration/activeXMonitor/property/clsidWhitelistEnabled", "true"}
            }
        };

        public static void SetProtectionLevelInConfig(EProtectionLevel protLevel)
        {
            if (XmlConfig.Path == null)
            {
                throw new Exception("XmlConfig needs to have been initialised before calling");
            }

            Dictionary<string, string> settings = protLevelPresets[(int)protLevel];

            foreach (KeyValuePair<string, string> kvp in settings)
            {
                XmlConfig.SetConfigString(kvp.Key, kvp.Value);
            }
        }
    }
}
