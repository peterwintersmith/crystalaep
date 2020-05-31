// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
namespace CrystalAEPUI
{
    partial class MainUI
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainUI));
            this.lblMonProc = new System.Windows.Forms.Label();
            this.lblLatestAlerts = new System.Windows.Forms.Label();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.alertsBrowserToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.configurationToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.basicOptionsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.expertOptionsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
            this.updateCrystalToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.informationToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.usersGuideToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.lvLatestAlerts = new System.Windows.Forms.ListView();
            this.colDate = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colTime = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colTitle = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.tvMonProc = new System.Windows.Forms.TreeView();
            this.cbRisk = new System.Windows.Forms.CheckBox();
            this.lvRealtime = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.lblEnabledFeatures = new System.Windows.Forms.Label();
            this.cbContentFilter = new System.Windows.Forms.CheckBox();
            this.cbApiMonitor = new System.Windows.Forms.CheckBox();
            this.cbRopMonitor = new System.Windows.Forms.CheckBox();
            this.cbStackMonitor = new System.Windows.Forms.CheckBox();
            this.cbHeapMonitor = new System.Windows.Forms.CheckBox();
            this.cbAntiSpray = new System.Windows.Forms.CheckBox();
            this.cbComMonitor = new System.Windows.Forms.CheckBox();
            this.pbxLogo = new System.Windows.Forms.PictureBox();
            this.lblRealtimeAlerts = new System.Windows.Forms.Label();
            this.lblAEPLevel = new System.Windows.Forms.Label();
            this.tbProtLevel = new System.Windows.Forms.TrackBar();
            this.cbEnableCF = new System.Windows.Forms.CheckBox();
            this.cbEnableDiD = new System.Windows.Forms.CheckBox();
            this.btnDisableAll = new System.Windows.Forms.Button();
            this.btnShowMore = new System.Windows.Forms.Button();
            this.lblProtMin = new System.Windows.Forms.Label();
            this.lblProtMod = new System.Windows.Forms.Label();
            this.lblProtHigh = new System.Windows.Forms.Label();
            this.lblProtMax = new System.Windows.Forms.Label();
            this.tipUI = new System.Windows.Forms.ToolTip(this.components);
            this.lblProtState = new System.Windows.Forms.Label();
            this.tutorialToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbxLogo)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbProtLevel)).BeginInit();
            this.SuspendLayout();
            // 
            // lblMonProc
            // 
            this.lblMonProc.AutoSize = true;
            this.lblMonProc.Cursor = System.Windows.Forms.Cursors.Default;
            this.lblMonProc.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblMonProc.Location = new System.Drawing.Point(6, 31);
            this.lblMonProc.Name = "lblMonProc";
            this.lblMonProc.Size = new System.Drawing.Size(126, 15);
            this.lblMonProc.TabIndex = 0;
            this.lblMonProc.Text = "Monitored Processes:";
            // 
            // lblLatestAlerts
            // 
            this.lblLatestAlerts.AutoSize = true;
            this.lblLatestAlerts.Cursor = System.Windows.Forms.Cursors.Default;
            this.lblLatestAlerts.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblLatestAlerts.Location = new System.Drawing.Point(6, 204);
            this.lblLatestAlerts.Name = "lblLatestAlerts";
            this.lblLatestAlerts.Size = new System.Drawing.Size(77, 15);
            this.lblLatestAlerts.TabIndex = 2;
            this.lblLatestAlerts.Text = "Latest Alerts:";
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItem1,
            this.configurationToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Padding = new System.Windows.Forms.Padding(7, 2, 0, 2);
            this.menuStrip1.Size = new System.Drawing.Size(580, 24);
            this.menuStrip1.TabIndex = 4;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.alertsBrowserToolStripMenuItem,
            this.toolStripSeparator1,
            this.exitToolStripMenuItem});
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(79, 20);
            this.toolStripMenuItem1.Text = "Monitoring";
            // 
            // alertsBrowserToolStripMenuItem
            // 
            this.alertsBrowserToolStripMenuItem.Name = "alertsBrowserToolStripMenuItem";
            this.alertsBrowserToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
            this.alertsBrowserToolStripMenuItem.Text = "Alerts Browser";
            this.alertsBrowserToolStripMenuItem.Click += new System.EventHandler(this.alertsBrowserToolStripMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(146, 6);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(149, 22);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // configurationToolStripMenuItem
            // 
            this.configurationToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.basicOptionsToolStripMenuItem,
            this.expertOptionsToolStripMenuItem,
            this.toolStripMenuItem3,
            this.updateCrystalToolStripMenuItem});
            this.configurationToolStripMenuItem.Name = "configurationToolStripMenuItem";
            this.configurationToolStripMenuItem.Size = new System.Drawing.Size(93, 20);
            this.configurationToolStripMenuItem.Text = "Configuration";
            // 
            // basicOptionsToolStripMenuItem
            // 
            this.basicOptionsToolStripMenuItem.Name = "basicOptionsToolStripMenuItem";
            this.basicOptionsToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.basicOptionsToolStripMenuItem.Text = "Basic Options";
            this.basicOptionsToolStripMenuItem.Click += new System.EventHandler(this.basicOptionsToolStripMenuItem_Click);
            // 
            // expertOptionsToolStripMenuItem
            // 
            this.expertOptionsToolStripMenuItem.Name = "expertOptionsToolStripMenuItem";
            this.expertOptionsToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.expertOptionsToolStripMenuItem.Text = "Expert Options";
            this.expertOptionsToolStripMenuItem.Click += new System.EventHandler(this.expertOptionsToolStripMenuItem_Click);
            // 
            // toolStripMenuItem3
            // 
            this.toolStripMenuItem3.Name = "toolStripMenuItem3";
            this.toolStripMenuItem3.Size = new System.Drawing.Size(149, 6);
            // 
            // updateCrystalToolStripMenuItem
            // 
            this.updateCrystalToolStripMenuItem.Name = "updateCrystalToolStripMenuItem";
            this.updateCrystalToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.updateCrystalToolStripMenuItem.Text = "Update Crystal";
            this.updateCrystalToolStripMenuItem.Click += new System.EventHandler(this.updateCrystalToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.informationToolStripMenuItem,
            this.usersGuideToolStripMenuItem,
            this.tutorialToolStripMenuItem,
            this.toolStripMenuItem2,
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // informationToolStripMenuItem
            // 
            this.informationToolStripMenuItem.Name = "informationToolStripMenuItem";
            this.informationToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.informationToolStripMenuItem.Text = "Help";
            this.informationToolStripMenuItem.Click += new System.EventHandler(this.informationToolStripMenuItem_Click);
            // 
            // usersGuideToolStripMenuItem
            // 
            this.usersGuideToolStripMenuItem.Name = "usersGuideToolStripMenuItem";
            this.usersGuideToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.usersGuideToolStripMenuItem.Text = "User\'s Guide";
            this.usersGuideToolStripMenuItem.Click += new System.EventHandler(this.usersGuideToolStripMenuItem_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(149, 6);
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.aboutToolStripMenuItem.Text = "About";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
            // 
            // lvLatestAlerts
            // 
            this.lvLatestAlerts.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colDate,
            this.colTime,
            this.colTitle});
            this.lvLatestAlerts.Location = new System.Drawing.Point(9, 222);
            this.lvLatestAlerts.Name = "lvLatestAlerts";
            this.lvLatestAlerts.Size = new System.Drawing.Size(562, 104);
            this.lvLatestAlerts.TabIndex = 5;
            this.lvLatestAlerts.UseCompatibleStateImageBehavior = false;
            this.lvLatestAlerts.DoubleClick += new System.EventHandler(this.DblClick);
            // 
            // colDate
            // 
            this.colDate.Text = "Date";
            this.colDate.Width = 75;
            // 
            // colTime
            // 
            this.colTime.Text = "Time";
            this.colTime.Width = 75;
            // 
            // colTitle
            // 
            this.colTitle.Text = "Title";
            this.colTitle.Width = 220;
            // 
            // tvMonProc
            // 
            this.tvMonProc.Location = new System.Drawing.Point(9, 49);
            this.tvMonProc.Name = "tvMonProc";
            this.tvMonProc.Size = new System.Drawing.Size(445, 123);
            this.tvMonProc.TabIndex = 6;
            this.tvMonProc.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.tvMonProc_AfterSelect);
            // 
            // cbRisk
            // 
            this.cbRisk.AutoSize = true;
            this.cbRisk.Checked = true;
            this.cbRisk.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbRisk.Location = new System.Drawing.Point(462, 181);
            this.cbRisk.Name = "cbRisk";
            this.cbRisk.Size = new System.Drawing.Size(110, 19);
            this.cbRisk.TabIndex = 7;
            this.cbRisk.Text = "Important Only";
            this.cbRisk.UseVisualStyleBackColor = true;
            this.cbRisk.CheckedChanged += new System.EventHandler(this.cbRisk_CheckedChanged);
            // 
            // lvRealtime
            // 
            this.lvRealtime.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.lvRealtime.Location = new System.Drawing.Point(9, 465);
            this.lvRealtime.Name = "lvRealtime";
            this.lvRealtime.Size = new System.Drawing.Size(445, 167);
            this.lvRealtime.TabIndex = 10;
            this.lvRealtime.UseCompatibleStateImageBehavior = false;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Realtime Events";
            this.columnHeader1.Width = 478;
            // 
            // lblEnabledFeatures
            // 
            this.lblEnabledFeatures.AutoSize = true;
            this.lblEnabledFeatures.Cursor = System.Windows.Forms.Cursors.Default;
            this.lblEnabledFeatures.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblEnabledFeatures.Location = new System.Drawing.Point(459, 443);
            this.lblEnabledFeatures.Name = "lblEnabledFeatures";
            this.lblEnabledFeatures.Size = new System.Drawing.Size(104, 15);
            this.lblEnabledFeatures.TabIndex = 12;
            this.lblEnabledFeatures.Text = "Enabled Features:";
            // 
            // cbContentFilter
            // 
            this.cbContentFilter.AutoSize = true;
            this.cbContentFilter.Location = new System.Drawing.Point(462, 461);
            this.cbContentFilter.Name = "cbContentFilter";
            this.cbContentFilter.Size = new System.Drawing.Size(100, 19);
            this.cbContentFilter.TabIndex = 13;
            this.cbContentFilter.Text = "Content Filter";
            this.cbContentFilter.UseVisualStyleBackColor = true;
            // 
            // cbApiMonitor
            // 
            this.cbApiMonitor.AutoSize = true;
            this.cbApiMonitor.Location = new System.Drawing.Point(462, 611);
            this.cbApiMonitor.Name = "cbApiMonitor";
            this.cbApiMonitor.Size = new System.Drawing.Size(92, 19);
            this.cbApiMonitor.TabIndex = 14;
            this.cbApiMonitor.Text = "API Monitor";
            this.cbApiMonitor.UseVisualStyleBackColor = true;
            // 
            // cbRopMonitor
            // 
            this.cbRopMonitor.AutoSize = true;
            this.cbRopMonitor.Location = new System.Drawing.Point(462, 486);
            this.cbRopMonitor.Name = "cbRopMonitor";
            this.cbRopMonitor.Size = new System.Drawing.Size(97, 19);
            this.cbRopMonitor.TabIndex = 15;
            this.cbRopMonitor.Text = "ROP Monitor";
            this.cbRopMonitor.UseVisualStyleBackColor = true;
            // 
            // cbStackMonitor
            // 
            this.cbStackMonitor.AutoSize = true;
            this.cbStackMonitor.Location = new System.Drawing.Point(462, 511);
            this.cbStackMonitor.Name = "cbStackMonitor";
            this.cbStackMonitor.Size = new System.Drawing.Size(103, 19);
            this.cbStackMonitor.TabIndex = 16;
            this.cbStackMonitor.Text = "Stack Monitor";
            this.cbStackMonitor.UseVisualStyleBackColor = true;
            // 
            // cbHeapMonitor
            // 
            this.cbHeapMonitor.AutoSize = true;
            this.cbHeapMonitor.Location = new System.Drawing.Point(462, 561);
            this.cbHeapMonitor.Name = "cbHeapMonitor";
            this.cbHeapMonitor.Size = new System.Drawing.Size(102, 19);
            this.cbHeapMonitor.TabIndex = 17;
            this.cbHeapMonitor.Text = "Heap Monitor";
            this.cbHeapMonitor.UseVisualStyleBackColor = true;
            // 
            // cbAntiSpray
            // 
            this.cbAntiSpray.AutoSize = true;
            this.cbAntiSpray.Location = new System.Drawing.Point(462, 536);
            this.cbAntiSpray.Name = "cbAntiSpray";
            this.cbAntiSpray.Size = new System.Drawing.Size(81, 19);
            this.cbAntiSpray.TabIndex = 18;
            this.cbAntiSpray.Text = "Anti Spray";
            this.cbAntiSpray.UseVisualStyleBackColor = true;
            // 
            // cbComMonitor
            // 
            this.cbComMonitor.AutoSize = true;
            this.cbComMonitor.Location = new System.Drawing.Point(462, 586);
            this.cbComMonitor.Name = "cbComMonitor";
            this.cbComMonitor.Size = new System.Drawing.Size(101, 19);
            this.cbComMonitor.TabIndex = 19;
            this.cbComMonitor.Text = "COM Monitor";
            this.cbComMonitor.UseVisualStyleBackColor = true;
            // 
            // pbxLogo
            // 
            this.pbxLogo.Image = global::CrystalAEPUI.Properties.Resources.BlueShield1;
            this.pbxLogo.Location = new System.Drawing.Point(460, 49);
            this.pbxLogo.Name = "pbxLogo";
            this.pbxLogo.Size = new System.Drawing.Size(111, 124);
            this.pbxLogo.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pbxLogo.TabIndex = 8;
            this.pbxLogo.TabStop = false;
            // 
            // lblRealtimeAlerts
            // 
            this.lblRealtimeAlerts.AutoSize = true;
            this.lblRealtimeAlerts.Cursor = System.Windows.Forms.Cursors.Default;
            this.lblRealtimeAlerts.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblRealtimeAlerts.Location = new System.Drawing.Point(6, 443);
            this.lblRealtimeAlerts.Name = "lblRealtimeAlerts";
            this.lblRealtimeAlerts.Size = new System.Drawing.Size(92, 15);
            this.lblRealtimeAlerts.TabIndex = 20;
            this.lblRealtimeAlerts.Text = "Realtime Alerts:";
            // 
            // lblAEPLevel
            // 
            this.lblAEPLevel.AutoSize = true;
            this.lblAEPLevel.Cursor = System.Windows.Forms.Cursors.Default;
            this.lblAEPLevel.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblAEPLevel.Location = new System.Drawing.Point(6, 331);
            this.lblAEPLevel.Name = "lblAEPLevel";
            this.lblAEPLevel.Size = new System.Drawing.Size(162, 15);
            this.lblAEPLevel.TabIndex = 21;
            this.lblAEPLevel.Text = "Anti-Exploit Protection Level:";
            // 
            // tbProtLevel
            // 
            this.tbProtLevel.LargeChange = 1;
            this.tbProtLevel.Location = new System.Drawing.Point(9, 349);
            this.tbProtLevel.Maximum = 3;
            this.tbProtLevel.Name = "tbProtLevel";
            this.tbProtLevel.Size = new System.Drawing.Size(562, 45);
            this.tbProtLevel.TabIndex = 22;
            this.tbProtLevel.Scroll += new System.EventHandler(this.tbProtLevel_Scroll);
            // 
            // cbEnableCF
            // 
            this.cbEnableCF.AutoSize = true;
            this.cbEnableCF.Location = new System.Drawing.Point(9, 182);
            this.cbEnableCF.Name = "cbEnableCF";
            this.cbEnableCF.Size = new System.Drawing.Size(157, 19);
            this.cbEnableCF.TabIndex = 23;
            this.cbEnableCF.Text = "Enable Content Filtering";
            this.cbEnableCF.UseVisualStyleBackColor = true;
            this.cbEnableCF.CheckedChanged += new System.EventHandler(this.cbEnableCF_CheckedChanged);
            // 
            // cbEnableDiD
            // 
            this.cbEnableDiD.AutoSize = true;
            this.cbEnableDiD.Location = new System.Drawing.Point(172, 182);
            this.cbEnableDiD.Name = "cbEnableDiD";
            this.cbEnableDiD.Size = new System.Drawing.Size(129, 19);
            this.cbEnableDiD.TabIndex = 24;
            this.cbEnableDiD.Text = "Enable Anti-Exploit";
            this.cbEnableDiD.UseVisualStyleBackColor = true;
            this.cbEnableDiD.CheckedChanged += new System.EventHandler(this.cbEnableDiD_CheckedChanged);
            // 
            // btnDisableAll
            // 
            this.btnDisableAll.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.btnDisableAll.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btnDisableAll.ForeColor = System.Drawing.Color.Red;
            this.btnDisableAll.Location = new System.Drawing.Point(335, 177);
            this.btnDisableAll.Name = "btnDisableAll";
            this.btnDisableAll.Size = new System.Drawing.Size(119, 23);
            this.btnDisableAll.TabIndex = 25;
            this.btnDisableAll.Text = "Disable All";
            this.btnDisableAll.UseVisualStyleBackColor = true;
            this.btnDisableAll.Click += new System.EventHandler(this.btnDisableAll_Click);
            // 
            // btnShowMore
            // 
            this.btnShowMore.Location = new System.Drawing.Point(496, 402);
            this.btnShowMore.Name = "btnShowMore";
            this.btnShowMore.Size = new System.Drawing.Size(75, 23);
            this.btnShowMore.TabIndex = 26;
            this.btnShowMore.Text = "More ...";
            this.btnShowMore.UseVisualStyleBackColor = true;
            this.btnShowMore.Click += new System.EventHandler(this.btnShowMore_Click);
            // 
            // lblProtMin
            // 
            this.lblProtMin.AutoSize = true;
            this.lblProtMin.Cursor = System.Windows.Forms.Cursors.Help;
            this.lblProtMin.Location = new System.Drawing.Point(6, 384);
            this.lblProtMin.Name = "lblProtMin";
            this.lblProtMin.Size = new System.Drawing.Size(60, 15);
            this.lblProtMin.TabIndex = 27;
            this.lblProtMin.Text = "Minimum";
            this.lblProtMin.MouseHover += new System.EventHandler(this.lblProtMin_MouseHover);
            // 
            // lblProtMod
            // 
            this.lblProtMod.AutoSize = true;
            this.lblProtMod.Cursor = System.Windows.Forms.Cursors.Help;
            this.lblProtMod.Location = new System.Drawing.Point(169, 384);
            this.lblProtMod.Name = "lblProtMod";
            this.lblProtMod.Size = new System.Drawing.Size(60, 15);
            this.lblProtMod.TabIndex = 28;
            this.lblProtMod.Text = "Moderate";
            this.lblProtMod.MouseHover += new System.EventHandler(this.lblProtMod_MouseHover);
            // 
            // lblProtHigh
            // 
            this.lblProtHigh.AutoSize = true;
            this.lblProtHigh.Cursor = System.Windows.Forms.Cursors.Help;
            this.lblProtHigh.Location = new System.Drawing.Point(365, 384);
            this.lblProtHigh.Name = "lblProtHigh";
            this.lblProtHigh.Size = new System.Drawing.Size(32, 15);
            this.lblProtHigh.TabIndex = 29;
            this.lblProtHigh.Text = "High";
            this.lblProtHigh.MouseHover += new System.EventHandler(this.lblProtHigh_MouseHover);
            // 
            // lblProtMax
            // 
            this.lblProtMax.AutoSize = true;
            this.lblProtMax.Cursor = System.Windows.Forms.Cursors.Help;
            this.lblProtMax.Location = new System.Drawing.Point(510, 384);
            this.lblProtMax.Name = "lblProtMax";
            this.lblProtMax.Size = new System.Drawing.Size(62, 15);
            this.lblProtMax.TabIndex = 30;
            this.lblProtMax.Text = "Maximum";
            this.lblProtMax.MouseHover += new System.EventHandler(this.lblProtMax_MouseHover);
            // 
            // lblProtState
            // 
            this.lblProtState.AutoSize = true;
            this.lblProtState.Cursor = System.Windows.Forms.Cursors.Default;
            this.lblProtState.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Underline, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblProtState.Location = new System.Drawing.Point(332, 204);
            this.lblProtState.Name = "lblProtState";
            this.lblProtState.Size = new System.Drawing.Size(0, 15);
            this.lblProtState.TabIndex = 31;
            // 
            // tutorialToolStripMenuItem
            // 
            this.tutorialToolStripMenuItem.Name = "tutorialToolStripMenuItem";
            this.tutorialToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.tutorialToolStripMenuItem.Text = "Tutorial";
            this.tutorialToolStripMenuItem.Click += new System.EventHandler(this.tutorialToolStripMenuItem_Click);
            // 
            // MainUI
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(580, 642);
            this.Controls.Add(this.lblProtState);
            this.Controls.Add(this.lblProtMax);
            this.Controls.Add(this.lblProtHigh);
            this.Controls.Add(this.lblProtMod);
            this.Controls.Add(this.lblProtMin);
            this.Controls.Add(this.btnShowMore);
            this.Controls.Add(this.btnDisableAll);
            this.Controls.Add(this.cbEnableDiD);
            this.Controls.Add(this.cbEnableCF);
            this.Controls.Add(this.tbProtLevel);
            this.Controls.Add(this.lblAEPLevel);
            this.Controls.Add(this.lblRealtimeAlerts);
            this.Controls.Add(this.cbComMonitor);
            this.Controls.Add(this.cbAntiSpray);
            this.Controls.Add(this.cbHeapMonitor);
            this.Controls.Add(this.cbStackMonitor);
            this.Controls.Add(this.cbRopMonitor);
            this.Controls.Add(this.cbApiMonitor);
            this.Controls.Add(this.cbContentFilter);
            this.Controls.Add(this.lblEnabledFeatures);
            this.Controls.Add(this.lvRealtime);
            this.Controls.Add(this.pbxLogo);
            this.Controls.Add(this.cbRisk);
            this.Controls.Add(this.tvMonProc);
            this.Controls.Add(this.lvLatestAlerts);
            this.Controls.Add(this.lblLatestAlerts);
            this.Controls.Add(this.lblMonProc);
            this.Controls.Add(this.menuStrip1);
            this.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.MaximizeBox = false;
            this.Name = "MainUI";
            this.Text = "Crystal Anti-Exploit Protection 2012";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainUI_FormClosing);
            this.Load += new System.EventHandler(this.MainUI_Load);
            this.Shown += new System.EventHandler(this.MainUI_Shown);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbxLogo)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbProtLevel)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblMonProc;
        private System.Windows.Forms.Label lblLatestAlerts;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem alertsBrowserToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem configurationToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem updateCrystalToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ListView lvLatestAlerts;
        private System.Windows.Forms.ColumnHeader colDate;
        private System.Windows.Forms.ColumnHeader colTime;
        private System.Windows.Forms.ColumnHeader colTitle;
        private System.Windows.Forms.TreeView tvMonProc;
        private System.Windows.Forms.CheckBox cbRisk;
        private System.Windows.Forms.ListView lvRealtime;
        private System.Windows.Forms.ToolStripMenuItem informationToolStripMenuItem;
        private System.Windows.Forms.Label lblEnabledFeatures;
        private System.Windows.Forms.CheckBox cbContentFilter;
        private System.Windows.Forms.CheckBox cbApiMonitor;
        private System.Windows.Forms.CheckBox cbRopMonitor;
        private System.Windows.Forms.CheckBox cbStackMonitor;
        private System.Windows.Forms.CheckBox cbHeapMonitor;
        private System.Windows.Forms.CheckBox cbAntiSpray;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.CheckBox cbComMonitor;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem expertOptionsToolStripMenuItem;
        private System.Windows.Forms.PictureBox pbxLogo;
        private System.Windows.Forms.Label lblRealtimeAlerts;
        private System.Windows.Forms.Label lblAEPLevel;
        private System.Windows.Forms.TrackBar tbProtLevel;
        private System.Windows.Forms.CheckBox cbEnableCF;
        private System.Windows.Forms.CheckBox cbEnableDiD;
        private System.Windows.Forms.Button btnDisableAll;
        private System.Windows.Forms.Button btnShowMore;
        private System.Windows.Forms.Label lblProtMin;
        private System.Windows.Forms.Label lblProtMod;
        private System.Windows.Forms.Label lblProtHigh;
        private System.Windows.Forms.Label lblProtMax;
        private System.Windows.Forms.ToolStripMenuItem basicOptionsToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
        private System.Windows.Forms.ToolTip tipUI;
        private System.Windows.Forms.Label lblProtState;
        private System.Windows.Forms.ToolStripMenuItem usersGuideToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem tutorialToolStripMenuItem;
    }
}

