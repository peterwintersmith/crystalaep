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
    public partial class Tutorial : Form
    {
        public Tutorial()
        {
            InitializeComponent();
        }

        private void Tutorial_FormClosing(object sender, FormClosingEventArgs e)
        {
            Hide();

            if (!MainUI.ShuttingDownProcess)
                e.Cancel = true;
        }

        private int viewStage = 0;

        private void button2_Click(object sender, EventArgs e)
        {
            switch (viewStage)
            {
                case 0:
                    break;
                case 1:
                    panel0.Visible = true;
                    panel1.Visible = false;
                    panel2.Visible = false;
                    panel3.Visible = false;
                    panel4.Visible = false;
                    panel5.Visible = false;
                    viewStage--;
                    break;
                case 2:
                    panel0.Visible = false;
                    panel1.Visible = true;
                    panel2.Visible = false;
                    panel3.Visible = false;
                    panel4.Visible = false;
                    panel5.Visible = false;
                    viewStage--;
                    break;
                case 3:
                    panel0.Visible = false;
                    panel1.Visible = false;
                    panel2.Visible = true;
                    panel3.Visible = false;
                    panel4.Visible = false;
                    panel5.Visible = false;
                    viewStage--;
                    break;
                case 4:
                    panel0.Visible = false;
                    panel1.Visible = false;
                    panel2.Visible = false;
                    panel3.Visible = true;
                    panel4.Visible = false;
                    panel5.Visible = false;
                    viewStage--;
                    break;
                case 5:
                    panel0.Visible = false;
                    panel1.Visible = false;
                    panel2.Visible = false;
                    panel3.Visible = false;
                    panel4.Visible = true;
                    panel5.Visible = false;
                    viewStage--;
                    button1.Text = "Next";
                    break;
            }

            textBox8.Text = (viewStage + 1).ToString();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            switch (viewStage)
            {
                case 0:
                    panel0.Visible = false;
                    panel1.Visible = true;
                    panel2.Visible = false;
                    panel3.Visible = false;
                    panel4.Visible = false;
                    panel5.Visible = false;
                    viewStage++;
                    break;
                case 1:
                    panel0.Visible = false;
                    panel1.Visible = false;
                    panel2.Visible = true;
                    panel3.Visible = false;
                    panel4.Visible = false;
                    panel5.Visible = false;
                    viewStage++;
                    break;
                case 2:
                    panel0.Visible = false;
                    panel1.Visible = false;
                    panel2.Visible = false;
                    panel3.Visible = true;
                    panel4.Visible = false;
                    panel5.Visible = false;
                    viewStage++;
                    break;
                case 3:
                    panel0.Visible = false;
                    panel1.Visible = false;
                    panel2.Visible = false;
                    panel3.Visible = false;
                    panel4.Visible = true;
                    panel5.Visible = false;
                    viewStage++;
                    break;
                case 4:
                    panel0.Visible = false;
                    panel1.Visible = false;
                    panel2.Visible = false;
                    panel3.Visible = false;
                    panel4.Visible = false;
                    panel5.Visible = true;
                    button1.Text = "Exit";
                    viewStage++;
                    break;
                case 5:
                    Close();
                    return;
            }

            textBox8.Text = (viewStage + 1).ToString();
        }

        private void textBox4_TextChanged(object sender, EventArgs e)
        {

        }

        void ResetTutorial()
        {
            viewStage = 0;
            button1.Text = "Next";

            panel0.Visible = true;
            panel0.Parent = this;
            panel0.Location = new Point(2, 34);

            panel1.Visible = false;
            panel1.Parent = this;
            panel1.Location = new Point(2, 34);

            panel2.Visible = false;
            panel2.Parent = this;
            panel2.Location = new Point(2, 34);

            panel3.Visible = false;
            panel3.Parent = this;
            panel3.Location = new Point(2, 34);

            panel4.Visible = false;
            panel4.Parent = this;
            panel4.Location = new Point(2, 34);

            panel5.Visible = false;
            panel5.Parent = this;
            panel5.Location = new Point(2, 34);

            textBox8.Text = "1";
            textBox9.Text = "6";
        }

        private void Tutorial_Load(object sender, EventArgs e)
        {
            ResetTutorial();
        }

        private void panel4_Paint(object sender, PaintEventArgs e)
        {

        }

        private void panel0_Paint(object sender, PaintEventArgs e)
        {

        }

        private void Tutorial_VisibleChanged(object sender, EventArgs e)
        {
            ResetTutorial();
        }
    }
}
