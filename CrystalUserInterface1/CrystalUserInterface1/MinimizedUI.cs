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
    public partial class MinimizedUI : Form
    {
        public MinimizedUI()
        {
            InitializeComponent();
        }

        private void picLogo_Click(object sender, EventArgs e)
        {
            if (MainUIInst != null)
            {
                MainUIInst.Show();
                Hide();
            }
        }

        private NotifyIcon trayIcon = null;
        private ContextMenu trayMenu = null;
        private bool createdTrayIcon = false;

        private void OnRestore(object sender, EventArgs e)
        {
            Show();
        }

        private void OnExit(object sender, EventArgs e)
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

        private void CreateSystrayIcon()
        {
            if (createdTrayIcon)
                return;

            trayMenu = new ContextMenu();
            trayMenu.MenuItems.Add("Restore", OnRestore);
            trayMenu.MenuItems.Add("-", (object o, EventArgs e) => { return; });
            trayMenu.MenuItems.Add("Exit", OnExit);

            trayIcon = new NotifyIcon();
            trayIcon.Text = "CrystalAEP";
            trayIcon.Icon = new Icon(this.Icon, 40, 40);

            trayIcon.Click += OnRestore;

            trayIcon.ContextMenu = trayMenu;
            trayIcon.Visible = true;

            createdTrayIcon = true;
        }
        
        public MainUI MainUIInst = null;
        
        private bool isMouseDown = false;
        private int offsX = 0, offsY = 0;
        
        private void picTopBar_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.X >= 39)
            {
                if (!MainUI.MinimizeToTray)
                {
                    OnExit(null, null);
                }
                else
                {
                    Hide();
                }
            }
            else
            {
                isMouseDown = true;
                offsX = e.X;
                offsY = e.Y;
            }
        }

        public void SetEnableStatePicture(bool enabled)
        {
            picDisabledLogo.Visible = !enabled;
            picLogo.Visible = enabled;
        }

        private void picTopBar_MouseUp(object sender, MouseEventArgs e)
        {
            isMouseDown = false;
        }

        private void picTopBar_MouseMove(object sender, MouseEventArgs e)
        {
            if (isMouseDown)
            {
                Location = new Point(Location.X + e.X - offsX, Location.Y + e.Y - offsY);
            }
        }

        private void MinimizedUI_Load(object sender, EventArgs e)
        {
            Location = new Point(Screen.PrimaryScreen.WorkingArea.Width - (2 * Width), 20);
            CreateSystrayIcon();
        }

        private void picDisabledLogo_Click(object sender, EventArgs e)
        {
            if (MainUIInst != null)
            {
                MainUIInst.Show();
                Hide();
            }
        }

        private void MinimizedUI_FormClosing(object sender, FormClosingEventArgs e)
        {
            MainUI.ShuttingDownProcess = true;
        }
    }
}
