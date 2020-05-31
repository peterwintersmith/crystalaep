// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
namespace CrystalAEPUI
{
    partial class BasicOptions
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
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(BasicOptions));
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label3 = new System.Windows.Forms.Label();
            this.btnAddByName = new System.Windows.Forms.Button();
            this.txtProcName = new System.Windows.Forms.TextBox();
            this.btnRemoveProc = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.btnSwitchProgView = new System.Windows.Forms.Button();
            this.btnRefresh = new System.Windows.Forms.Button();
            this.btnBrowse = new System.Windows.Forms.Button();
            this.btnAddProc = new System.Windows.Forms.Button();
            this.lvInstalledProc = new System.Windows.Forms.ListView();
            this.colProgName = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colDetails = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.lblInstalledRunning = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.lvProtProc = new System.Windows.Forms.ListView();
            this.colProtProc = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.radUseCustom = new System.Windows.Forms.RadioButton();
            this.radUseDefault = new System.Windows.Forms.RadioButton();
            this.lblProtMax = new System.Windows.Forms.Label();
            this.lblProtHigh = new System.Windows.Forms.Label();
            this.lblProtMod = new System.Windows.Forms.Label();
            this.lblProtMin = new System.Windows.Forms.Label();
            this.tbProtLevel = new System.Windows.Forms.TrackBar();
            this.lblAEPLevel = new System.Windows.Forms.Label();
            this.openProgramDlg = new System.Windows.Forms.OpenFileDialog();
            this.tipUI = new System.Windows.Forms.ToolTip(this.components);
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.cbPromptUserExe = new System.Windows.Forms.CheckBox();
            this.cbPromptOnDotNet = new System.Windows.Forms.CheckBox();
            this.cbMinimizeToTray = new System.Windows.Forms.CheckBox();
            this.cbExeWhitelist = new System.Windows.Forms.CheckBox();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.tbProtLevel)).BeginInit();
            this.groupBox3.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.btnAddByName);
            this.groupBox1.Controls.Add(this.txtProcName);
            this.groupBox1.Controls.Add(this.btnRemoveProc);
            this.groupBox1.Controls.Add(this.groupBox2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.lvProtProc);
            this.groupBox1.Controls.Add(this.radUseCustom);
            this.groupBox1.Controls.Add(this.radUseDefault);
            this.groupBox1.Controls.Add(this.lblProtMax);
            this.groupBox1.Controls.Add(this.lblProtHigh);
            this.groupBox1.Controls.Add(this.lblProtMod);
            this.groupBox1.Controls.Add(this.lblProtMin);
            this.groupBox1.Controls.Add(this.tbProtLevel);
            this.groupBox1.Controls.Add(this.lblAEPLevel);
            this.groupBox1.Location = new System.Drawing.Point(4, 3);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(675, 417);
            this.groupBox1.TabIndex = 37;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Protected Programs";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 42);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(87, 15);
            this.label3.TabIndex = 59;
            this.label3.Text = "Process Name:";
            // 
            // btnAddByName
            // 
            this.btnAddByName.Location = new System.Drawing.Point(202, 60);
            this.btnAddByName.Name = "btnAddByName";
            this.btnAddByName.Size = new System.Drawing.Size(57, 23);
            this.btnAddByName.TabIndex = 58;
            this.btnAddByName.Text = "Add";
            this.btnAddByName.UseVisualStyleBackColor = true;
            this.btnAddByName.Click += new System.EventHandler(this.btnAddByName_Click);
            // 
            // txtProcName
            // 
            this.txtProcName.Location = new System.Drawing.Point(12, 60);
            this.txtProcName.Name = "txtProcName";
            this.txtProcName.Size = new System.Drawing.Size(184, 23);
            this.txtProcName.TabIndex = 57;
            // 
            // btnRemoveProc
            // 
            this.btnRemoveProc.Location = new System.Drawing.Point(12, 295);
            this.btnRemoveProc.Name = "btnRemoveProc";
            this.btnRemoveProc.Size = new System.Drawing.Size(247, 23);
            this.btnRemoveProc.TabIndex = 56;
            this.btnRemoveProc.Text = "Remove Program";
            this.btnRemoveProc.UseVisualStyleBackColor = true;
            this.btnRemoveProc.Click += new System.EventHandler(this.btnRemoveProc_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.btnSwitchProgView);
            this.groupBox2.Controls.Add(this.btnRefresh);
            this.groupBox2.Controls.Add(this.btnBrowse);
            this.groupBox2.Controls.Add(this.btnAddProc);
            this.groupBox2.Controls.Add(this.lvInstalledProc);
            this.groupBox2.Controls.Add(this.lblInstalledRunning);
            this.groupBox2.Location = new System.Drawing.Point(265, 19);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(403, 305);
            this.groupBox2.TabIndex = 53;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Add Programs to Protect";
            // 
            // btnSwitchProgView
            // 
            this.btnSwitchProgView.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.btnSwitchProgView.Location = new System.Drawing.Point(147, 15);
            this.btnSwitchProgView.Name = "btnSwitchProgView";
            this.btnSwitchProgView.Size = new System.Drawing.Size(131, 23);
            this.btnSwitchProgView.TabIndex = 58;
            this.btnSwitchProgView.Text = "Show Running ...";
            this.btnSwitchProgView.UseVisualStyleBackColor = true;
            this.btnSwitchProgView.Click += new System.EventHandler(this.btnSwitchProgView_Click);
            // 
            // btnRefresh
            // 
            this.btnRefresh.Location = new System.Drawing.Point(284, 15);
            this.btnRefresh.Name = "btnRefresh";
            this.btnRefresh.Size = new System.Drawing.Size(112, 23);
            this.btnRefresh.TabIndex = 57;
            this.btnRefresh.Text = "Refresh List";
            this.btnRefresh.UseVisualStyleBackColor = true;
            this.btnRefresh.Click += new System.EventHandler(this.btnRefresh_Click);
            // 
            // btnBrowse
            // 
            this.btnBrowse.Location = new System.Drawing.Point(284, 276);
            this.btnBrowse.Name = "btnBrowse";
            this.btnBrowse.Size = new System.Drawing.Size(112, 23);
            this.btnBrowse.TabIndex = 56;
            this.btnBrowse.Text = "Browse ...";
            this.btnBrowse.UseVisualStyleBackColor = true;
            this.btnBrowse.Click += new System.EventHandler(this.btnBrowse_Click);
            // 
            // btnAddProc
            // 
            this.btnAddProc.Location = new System.Drawing.Point(11, 276);
            this.btnAddProc.Name = "btnAddProc";
            this.btnAddProc.Size = new System.Drawing.Size(112, 23);
            this.btnAddProc.TabIndex = 55;
            this.btnAddProc.Text = "Add Program";
            this.btnAddProc.UseVisualStyleBackColor = true;
            this.btnAddProc.Click += new System.EventHandler(this.btnAddProc_Click);
            // 
            // lvInstalledProc
            // 
            this.lvInstalledProc.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colProgName,
            this.colDetails});
            this.lvInstalledProc.Location = new System.Drawing.Point(11, 41);
            this.lvInstalledProc.Name = "lvInstalledProc";
            this.lvInstalledProc.Size = new System.Drawing.Size(385, 229);
            this.lvInstalledProc.TabIndex = 54;
            this.lvInstalledProc.UseCompatibleStateImageBehavior = false;
            this.lvInstalledProc.View = System.Windows.Forms.View.Details;
            // 
            // colProgName
            // 
            this.colProgName.Text = "Program Name";
            this.colProgName.Width = 117;
            // 
            // colDetails
            // 
            this.colDetails.Text = "Details";
            this.colDetails.Width = 234;
            // 
            // lblInstalledRunning
            // 
            this.lblInstalledRunning.AutoSize = true;
            this.lblInstalledRunning.Cursor = System.Windows.Forms.Cursors.Default;
            this.lblInstalledRunning.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblInstalledRunning.Location = new System.Drawing.Point(8, 23);
            this.lblInstalledRunning.Name = "lblInstalledRunning";
            this.lblInstalledRunning.Size = new System.Drawing.Size(115, 15);
            this.lblInstalledRunning.TabIndex = 54;
            this.lblInstalledRunning.Text = "Installed Programs:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Cursor = System.Windows.Forms.Cursors.Default;
            this.label1.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(9, 19);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(118, 15);
            this.label1.TabIndex = 51;
            this.label1.Text = "Protected Programs:";
            // 
            // lvProtProc
            // 
            this.lvProtProc.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colProtProc});
            this.lvProtProc.Location = new System.Drawing.Point(12, 89);
            this.lvProtProc.Name = "lvProtProc";
            this.lvProtProc.Size = new System.Drawing.Size(247, 200);
            this.lvProtProc.TabIndex = 38;
            this.lvProtProc.UseCompatibleStateImageBehavior = false;
            this.lvProtProc.View = System.Windows.Forms.View.Details;
            this.lvProtProc.SelectedIndexChanged += new System.EventHandler(this.lvProtProc_SelectedIndexChanged);
            // 
            // colProtProc
            // 
            this.colProtProc.Text = "Programs Currently Protected";
            this.colProtProc.Width = 214;
            // 
            // radUseCustom
            // 
            this.radUseCustom.AutoSize = true;
            this.radUseCustom.Location = new System.Drawing.Point(478, 330);
            this.radUseCustom.Name = "radUseCustom";
            this.radUseCustom.Size = new System.Drawing.Size(89, 17);
            this.radUseCustom.TabIndex = 50;
            this.radUseCustom.Text = "Custom Level";
            this.radUseCustom.UseVisualStyleBackColor = true;
            this.radUseCustom.CheckedChanged += new System.EventHandler(this.radUseCustom_CheckedChanged);
            // 
            // radUseDefault
            // 
            this.radUseDefault.AutoSize = true;
            this.radUseDefault.Location = new System.Drawing.Point(372, 330);
            this.radUseDefault.Name = "radUseDefault";
            this.radUseDefault.Size = new System.Drawing.Size(81, 17);
            this.radUseDefault.TabIndex = 49;
            this.radUseDefault.Text = "Use Default";
            this.radUseDefault.UseVisualStyleBackColor = true;
            this.radUseDefault.CheckedChanged += new System.EventHandler(this.radUseDefault_CheckedChanged);
            // 
            // lblProtMax
            // 
            this.lblProtMax.AutoSize = true;
            this.lblProtMax.Cursor = System.Windows.Forms.Cursors.Help;
            this.lblProtMax.Location = new System.Drawing.Point(597, 391);
            this.lblProtMax.Name = "lblProtMax";
            this.lblProtMax.Size = new System.Drawing.Size(62, 15);
            this.lblProtMax.TabIndex = 48;
            this.lblProtMax.Text = "Maximum";
            this.lblProtMax.MouseHover += new System.EventHandler(this.lblProtMax_MouseHover);
            // 
            // lblProtHigh
            // 
            this.lblProtHigh.AutoSize = true;
            this.lblProtHigh.Cursor = System.Windows.Forms.Cursors.Help;
            this.lblProtHigh.Location = new System.Drawing.Point(428, 391);
            this.lblProtHigh.Name = "lblProtHigh";
            this.lblProtHigh.Size = new System.Drawing.Size(32, 15);
            this.lblProtHigh.TabIndex = 47;
            this.lblProtHigh.Text = "High";
            this.lblProtHigh.MouseHover += new System.EventHandler(this.lblProtHigh_MouseHover);
            // 
            // lblProtMod
            // 
            this.lblProtMod.AutoSize = true;
            this.lblProtMod.Cursor = System.Windows.Forms.Cursors.Help;
            this.lblProtMod.Location = new System.Drawing.Point(199, 391);
            this.lblProtMod.Name = "lblProtMod";
            this.lblProtMod.Size = new System.Drawing.Size(60, 15);
            this.lblProtMod.TabIndex = 46;
            this.lblProtMod.Text = "Moderate";
            this.lblProtMod.MouseHover += new System.EventHandler(this.lblProtMod_MouseHover);
            // 
            // lblProtMin
            // 
            this.lblProtMin.AutoSize = true;
            this.lblProtMin.Cursor = System.Windows.Forms.Cursors.Help;
            this.lblProtMin.Location = new System.Drawing.Point(9, 391);
            this.lblProtMin.Name = "lblProtMin";
            this.lblProtMin.Size = new System.Drawing.Size(60, 15);
            this.lblProtMin.TabIndex = 45;
            this.lblProtMin.Text = "Minimum";
            this.lblProtMin.MouseHover += new System.EventHandler(this.lblProtMin_MouseHover);
            // 
            // tbProtLevel
            // 
            this.tbProtLevel.LargeChange = 1;
            this.tbProtLevel.Location = new System.Drawing.Point(12, 351);
            this.tbProtLevel.Maximum = 3;
            this.tbProtLevel.Name = "tbProtLevel";
            this.tbProtLevel.Size = new System.Drawing.Size(656, 45);
            this.tbProtLevel.TabIndex = 44;
            this.tbProtLevel.Scroll += new System.EventHandler(this.tbProtLevel_Scroll);
            // 
            // lblAEPLevel
            // 
            this.lblAEPLevel.AutoSize = true;
            this.lblAEPLevel.Cursor = System.Windows.Forms.Cursors.Default;
            this.lblAEPLevel.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblAEPLevel.Location = new System.Drawing.Point(9, 330);
            this.lblAEPLevel.Name = "lblAEPLevel";
            this.lblAEPLevel.Size = new System.Drawing.Size(273, 15);
            this.lblAEPLevel.TabIndex = 43;
            this.lblAEPLevel.Text = "Anti-Exploit Protection Level (<select a process>):";
            // 
            // openProgramDlg
            // 
            this.openProgramDlg.Filter = "Executable Files|*.exe";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.cbPromptUserExe);
            this.groupBox3.Controls.Add(this.cbPromptOnDotNet);
            this.groupBox3.Controls.Add(this.cbMinimizeToTray);
            this.groupBox3.Controls.Add(this.cbExeWhitelist);
            this.groupBox3.Location = new System.Drawing.Point(4, 426);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(675, 70);
            this.groupBox3.TabIndex = 60;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Miscellaneous";
            // 
            // cbPromptUserExe
            // 
            this.cbPromptUserExe.AutoSize = true;
            this.cbPromptUserExe.Font = new System.Drawing.Font("Calibri", 9F);
            this.cbPromptUserExe.Location = new System.Drawing.Point(314, 47);
            this.cbPromptUserExe.Name = "cbPromptUserExe";
            this.cbPromptUserExe.Size = new System.Drawing.Size(272, 18);
            this.cbPromptUserExe.TabIndex = 64;
            this.cbPromptUserExe.Text = "Prompt User to Override Blocked Applications";
            this.cbPromptUserExe.UseVisualStyleBackColor = true;
            this.cbPromptUserExe.CheckedChanged += new System.EventHandler(this.cbPromptUserExe_CheckedChanged);
            // 
            // cbPromptOnDotNet
            // 
            this.cbPromptOnDotNet.AutoSize = true;
            this.cbPromptOnDotNet.Font = new System.Drawing.Font("Calibri", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.cbPromptOnDotNet.Location = new System.Drawing.Point(12, 22);
            this.cbPromptOnDotNet.Name = "cbPromptOnDotNet";
            this.cbPromptOnDotNet.Size = new System.Drawing.Size(283, 18);
            this.cbPromptOnDotNet.TabIndex = 63;
            this.cbPromptOnDotNet.Text = "Prompt to Relax Protection for .NET Applications";
            this.cbPromptOnDotNet.UseVisualStyleBackColor = true;
            this.cbPromptOnDotNet.CheckedChanged += new System.EventHandler(this.cbPromptOnDotNet_CheckedChanged);
            // 
            // cbMinimizeToTray
            // 
            this.cbMinimizeToTray.AutoSize = true;
            this.cbMinimizeToTray.Font = new System.Drawing.Font("Calibri", 9F);
            this.cbMinimizeToTray.Location = new System.Drawing.Point(12, 47);
            this.cbMinimizeToTray.Name = "cbMinimizeToTray";
            this.cbMinimizeToTray.Size = new System.Drawing.Size(195, 18);
            this.cbMinimizeToTray.TabIndex = 62;
            this.cbMinimizeToTray.Text = "Minimize Crystal to System Tray";
            this.cbMinimizeToTray.UseVisualStyleBackColor = true;
            this.cbMinimizeToTray.CheckedChanged += new System.EventHandler(this.cbMinimizeToTray_CheckedChanged);
            // 
            // cbExeWhitelist
            // 
            this.cbExeWhitelist.AutoSize = true;
            this.cbExeWhitelist.Font = new System.Drawing.Font("Calibri", 9F);
            this.cbExeWhitelist.Location = new System.Drawing.Point(314, 22);
            this.cbExeWhitelist.Name = "cbExeWhitelist";
            this.cbExeWhitelist.Size = new System.Drawing.Size(343, 18);
            this.cbExeWhitelist.TabIndex = 61;
            this.cbExeWhitelist.Text = "Protected Programs Do Not Run Unrecognised Applications";
            this.cbExeWhitelist.UseVisualStyleBackColor = true;
            this.cbExeWhitelist.CheckedChanged += new System.EventHandler(this.cbExeWhitelist_CheckedChanged);
            // 
            // BasicOptions
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(683, 499);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox1);
            this.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(699, 537);
            this.MinimumSize = new System.Drawing.Size(699, 537);
            this.Name = "BasicOptions";
            this.Text = "Basic Options";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.BasicOptions_FormClosing);
            this.Load += new System.EventHandler(this.BasicOptions_Load);
            this.VisibleChanged += new System.EventHandler(this.BasicOptions_VisibleChanged);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.tbProtLevel)).EndInit();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnRemoveProc;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button btnRefresh;
        private System.Windows.Forms.Button btnBrowse;
        private System.Windows.Forms.Button btnAddProc;
        private System.Windows.Forms.ListView lvInstalledProc;
        private System.Windows.Forms.Label lblInstalledRunning;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ListView lvProtProc;
        private System.Windows.Forms.RadioButton radUseCustom;
        private System.Windows.Forms.RadioButton radUseDefault;
        private System.Windows.Forms.Label lblProtMax;
        private System.Windows.Forms.Label lblProtHigh;
        private System.Windows.Forms.Label lblProtMod;
        private System.Windows.Forms.Label lblProtMin;
        private System.Windows.Forms.TrackBar tbProtLevel;
        private System.Windows.Forms.Label lblAEPLevel;
        private System.Windows.Forms.OpenFileDialog openProgramDlg;
        private System.Windows.Forms.ColumnHeader colProgName;
        private System.Windows.Forms.ColumnHeader colDetails;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button btnAddByName;
        private System.Windows.Forms.TextBox txtProcName;
        private System.Windows.Forms.ColumnHeader colProtProc;
        private System.Windows.Forms.Button btnSwitchProgView;
        private System.Windows.Forms.ToolTip tipUI;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.CheckBox cbExeWhitelist;
        private System.Windows.Forms.CheckBox cbMinimizeToTray;
        private System.Windows.Forms.CheckBox cbPromptOnDotNet;
        private System.Windows.Forms.CheckBox cbPromptUserExe;

    }
}