// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Security.Cryptography;
using System.Runtime.CompilerServices;

namespace CrystalAEPUI
{
    public partial class HelpUI : Form
    {
        public HelpUI()
        {
            InitializeComponent();
        }

        private Dictionary<string, string> helpFileHashes = new Dictionary<string, string>()
        {
            {"Main User Interface", "4dvve+1iBeui5NZFYTdekSm/XdDDT5msPTnbU8ijR94="},
            {"Alerts Browser", "mnDXiObPgJTzH7E5MDrFW4LWOGD/nfXA9jeBSaGAk5I="},
            {"Basic Options", "NVyxx3MchnCg7hDFAF3uBUPM6dzr+lYOYB8YHaPnDuo="},
            {"Expert Options", "b17C0bDI/jJeuRSnlHrnVvGiwL+bWS01SUob+nPM9oE="},
            {"Managing Content Filters", "Xdh5u99e4VSejCAPE4pw1rCwGWPP+K10C9oT+/KjkcE="},
            {"Updating Crystal", "67r0xA5uBEAT2W51RIlhMWZJsf/NoRWXm4VcIHH70gw="}
        };

        private void cmbTopic_SelectedIndexChanged(object sender, EventArgs e)
        {
            try
            {
                Uri helpUri = new Uri("file://" + EventManager.InstallPath + "Help\\" + cmbTopic.Text.Replace(" ", "_") + ".htm");

                byte[] data = File.ReadAllBytes(helpUri.LocalPath);
                SHA256 sha256 = SHA256.Create();
                byte[] hash = sha256.ComputeHash(data), expectedHash = helpFileHashes[cmbTopic.Text] == null ? null : Convert.FromBase64String(helpFileHashes[cmbTopic.Text]);
                
                if (expectedHash == null || ByteUtils.ValuesEqual(hash, expectedHash))
                {
                    // otherwise we provide a great avenue for cross-zone exploitation
                    wbHelp.Navigate(helpUri);
                }
                else
                {
                    throw new Exception();
                }
            }
            catch
            {
                MessageBox.Show("The help file for section \"" + cmbTopic.Text + "\" has been tampered with and will not be displayed.");
            }
        }

        private void HelpUI_Load(object sender, EventArgs e)
        {
            cmbTopic.Items.Add("Main User Interface");
            cmbTopic.Items.Add("Alerts Browser");
            cmbTopic.Items.Add("Basic Options");
            cmbTopic.Items.Add("Expert Options");
            cmbTopic.Items.Add("Managing Content Filters");
            cmbTopic.Items.Add("Updating Crystal");

            cmbTopic.SelectedIndex = 0;
        }
    }

    public static class ByteUtils
    {
        public static bool ValuesEqual(byte[] array1, byte[] array2)
        {
            if (array1.Length != array2.Length)
                return false;

            for (int i = 0; i < array1.Length; i++)
            {
                if (array1[i] != array2[i])
                    return false;
            }

            return true;
        }
    }
}
