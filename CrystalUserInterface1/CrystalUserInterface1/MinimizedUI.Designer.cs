// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
namespace CrystalAEPUI
{
    partial class MinimizedUI
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }

            if (disposing)
            {
                trayIcon.Dispose();
            }

            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MinimizedUI));
            this.picTopBar = new System.Windows.Forms.PictureBox();
            this.picLogo = new System.Windows.Forms.PictureBox();
            this.picDisabledLogo = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.picTopBar)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.picLogo)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.picDisabledLogo)).BeginInit();
            this.SuspendLayout();
            // 
            // picTopBar
            // 
            this.picTopBar.Image = ((System.Drawing.Image)(resources.GetObject("picTopBar.Image")));
            this.picTopBar.InitialImage = ((System.Drawing.Image)(resources.GetObject("picTopBar.InitialImage")));
            this.picTopBar.Location = new System.Drawing.Point(0, 0);
            this.picTopBar.Name = "picTopBar";
            this.picTopBar.Size = new System.Drawing.Size(50, 16);
            this.picTopBar.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.picTopBar.TabIndex = 1;
            this.picTopBar.TabStop = false;
            this.picTopBar.MouseDown += new System.Windows.Forms.MouseEventHandler(this.picTopBar_MouseDown);
            this.picTopBar.MouseMove += new System.Windows.Forms.MouseEventHandler(this.picTopBar_MouseMove);
            this.picTopBar.MouseUp += new System.Windows.Forms.MouseEventHandler(this.picTopBar_MouseUp);
            // 
            // picLogo
            // 
            this.picLogo.BackColor = System.Drawing.SystemColors.Control;
            this.picLogo.Cursor = System.Windows.Forms.Cursors.Hand;
            this.picLogo.Image = global::CrystalAEPUI.Properties.Resources.BlueShield1;
            this.picLogo.Location = new System.Drawing.Point(0, 14);
            this.picLogo.Name = "picLogo";
            this.picLogo.Size = new System.Drawing.Size(50, 60);
            this.picLogo.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.picLogo.TabIndex = 0;
            this.picLogo.TabStop = false;
            this.picLogo.Click += new System.EventHandler(this.picLogo_Click);
            // 
            // picDisabledLogo
            // 
            this.picDisabledLogo.BackColor = System.Drawing.SystemColors.Control;
            this.picDisabledLogo.Cursor = System.Windows.Forms.Cursors.Hand;
            this.picDisabledLogo.Image = global::CrystalAEPUI.Properties.Resources.RedSh;
            this.picDisabledLogo.Location = new System.Drawing.Point(0, 15);
            this.picDisabledLogo.Name = "picDisabledLogo";
            this.picDisabledLogo.Size = new System.Drawing.Size(50, 60);
            this.picDisabledLogo.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.picDisabledLogo.TabIndex = 2;
            this.picDisabledLogo.TabStop = false;
            this.picDisabledLogo.Click += new System.EventHandler(this.picDisabledLogo_Click);
            // 
            // MinimizedUI
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Control;
            this.ClientSize = new System.Drawing.Size(52, 77);
            this.Controls.Add(this.picDisabledLogo);
            this.Controls.Add(this.picTopBar);
            this.Controls.Add(this.picLogo);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MinimizedUI";
            this.ShowInTaskbar = false;
            this.Text = "CrystalAEP 2012";
            this.TopMost = true;
            this.TransparencyKey = System.Drawing.SystemColors.Control;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MinimizedUI_FormClosing);
            this.Load += new System.EventHandler(this.MinimizedUI_Load);
            ((System.ComponentModel.ISupportInitialize)(this.picTopBar)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.picLogo)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.picDisabledLogo)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.PictureBox picLogo;
        private System.Windows.Forms.PictureBox picTopBar;
        private System.Windows.Forms.PictureBox picDisabledLogo;
    }
}