// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace StackEmuHelper
{
    class Program
    {
        static void DoIdcLogProcess(string idaLog)
        {
            string[] logLines = File.ReadAllLines(idaLog);
            Dictionary<string, int> mnemCounts = new Dictionary<string, int>();
            bool processing = false;
            int lineCounter = 0;

            foreach (string line in logLines)
            {
                lineCounter++;

                if (line.StartsWith("--CODE START--"))
                {
                    processing = true;
                    continue;
                }
                else if (line.StartsWith("--CODE END--"))
                {
                    processing = false;
                    continue;
                }

                if (processing)
                {
                    string disasm = line.Trim();

                    int indexMnemEnd = disasm.IndexOfAny(new char[] { ' ', '\t', '\r', '\n' });
                    if (indexMnemEnd == -1)
                        indexMnemEnd = disasm.Length;

                    string mnemonic = disasm.Substring(0, indexMnemEnd).Trim();

                    if (!mnemCounts.ContainsKey(mnemonic))
                    {
                        mnemCounts[mnemonic] = 0;
                    }

                    mnemCounts[mnemonic]++;
                }
            }

            IOrderedEnumerable<KeyValuePair<string, int>> query = mnemCounts.OrderByDescending(item => item.Value);

            foreach (KeyValuePair<string, int> kvp in query)
            {
                Console.WriteLine("{0}\t{1}", kvp.Key, kvp.Value);
            }
        }

        static void DoCompareDis(string ollyFile, string mydisFile)
        {
            string[] ollyLines = File.ReadAllLines(ollyFile), mydisLines = File.ReadAllLines(mydisFile);

            if (mydisLines.Length != ollyLines.Length)
            {
                Console.WriteLine("Files are of different line count");
            }

            for (int i = 0; i < ollyLines.Length; i++)
            {
                Console.WriteLine("{0}\t{1}", i + 1, ollyLines[i]);
                Console.WriteLine("{0}\t{1}", i + 1, mydisLines[i]);
                Console.WriteLine();

                bool alert = false;

                if (((ollyLines[i].IndexOf("BYTE") != -1 && mydisLines[i].IndexOf("BYTE") == -1) || (ollyLines[i].IndexOf("BYTE") == -1 && mydisLines[i].IndexOf("BYTE") != -1)) ||
                    ((ollyLines[i].IndexOf("WORD") != -1 && mydisLines[i].IndexOf("WORD") == -1) || (ollyLines[i].IndexOf("WORD") == -1 && mydisLines[i].IndexOf("WORD") != -1)) ||
                    ((ollyLines[i].IndexOf("DWORD") != -1 && mydisLines[i].IndexOf("DWORD") == -1) || (ollyLines[i].IndexOf("DWORD") == -1 && mydisLines[i].IndexOf("DWORD") != -1)))
                {
                    alert = true;
                }

                if (alert)
                {
                    ConsoleKeyInfo cki;

                    do
                    {
                        cki = Console.ReadKey(true);
                    }
                    while (cki.Key != ConsoleKey.Enter);
                }
            }
        }

        static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("StackEmuHelper. usage: StackEmuHelper.exe <-idalog|-mydis2olly> <ida-generated-log|disasm-file> <disasm-file-2>");
                return;
            }

            if (args[0] == "-idalog")
            {
                DoIdcLogProcess(args[1]);
            }
            else if (args[0] == "-comparedis")
            {
                if (args.Length != 3)
                    return;

                DoCompareDis(args[1], args[2]);
            }
        }
    }
}
