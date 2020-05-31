// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace CrystalAEPUI
{
    public partial class IndividualAlert : Form
    {
        public IndividualAlert()
        {
            InitializeComponent();

            rtbInfo.AppendText(new string('a', 100));

            int i;

            for (i = 0; i < 100; i++)
            {
                if (rtbInfo.GetLineFromCharIndex(i) > 0)
                    break;
            }

            rtbInfo.Clear();
            rtbInfo.ReadOnly = true;

            CharsPerLine = i;
        }

        private int CharsPerLine = 0;

        private void WriteTitle(string titleText)
        {
            rtbInfo.SelectionBackColor = Color.SteelBlue;
            rtbInfo.SelectionFont = new Font("Courier New", 10f, FontStyle.Bold);
            rtbInfo.AppendText(titleText + new string(' ', (CharsPerLine - (titleText.Length % CharsPerLine))) + "\r\n");
        }

        private void WriteBody(string body)
        {
            rtbInfo.SelectionFont = new Font("Calibri", 9f, FontStyle.Regular);
            rtbInfo.BackColor = Color.LightSteelBlue;
            rtbInfo.AppendText(body + "\r\n");
        }

        public void ShowEvent(EventItem evt)
        {
            WriteTitle("Date");
            WriteBody(evt.EventDate.ToString());

            WriteTitle("Title");
            WriteBody(evt.EventTitle);

            WriteTitle("Risk Rating");
            WriteBody(evt.EventRisk);

            WriteTitle("Message");
            WriteBody(evt.EventMessage);

            rtbInfo.SelectionStart = 0;

            this.Text = evt.EventTitle;
        }

        private void IndividualAlert_Load(object sender, EventArgs e)
        {
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
