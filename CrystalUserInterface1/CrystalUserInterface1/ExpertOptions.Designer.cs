// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
namespace CrystalAEPUI
{
    partial class ExpertOptions
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ExpertOptions));
            this.grpProtProc = new System.Windows.Forms.GroupBox();
            this.btnProcEditConf = new System.Windows.Forms.Button();
            this.lbProtProc = new System.Windows.Forms.ListBox();
            this.btnRemoveProc = new System.Windows.Forms.Button();
            this.btnAddProc = new System.Windows.Forms.Button();
            this.txtAddProc = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.grpProtMethods = new System.Windows.Forms.GroupBox();
            this.tabFeatures = new System.Windows.Forms.TabControl();
            this.tabpgConnMon = new System.Windows.Forms.TabPage();
            this.btnImportFilters = new System.Windows.Forms.Button();
            this.grpConnMonFeatures = new System.Windows.Forms.GroupBox();
            this.tabConnFilters = new System.Windows.Forms.TabControl();
            this.tabPage8 = new System.Windows.Forms.TabPage();
            this.lbEnabledConnFilters = new System.Windows.Forms.ListBox();
            this.lblEnabledConnFilters = new System.Windows.Forms.Label();
            this.tabPage9 = new System.Windows.Forms.TabPage();
            this.lbEnabledOutgoingFilters = new System.Windows.Forms.ListBox();
            this.label9 = new System.Windows.Forms.Label();
            this.tabPage10 = new System.Windows.Forms.TabPage();
            this.lbEnabledIncomingFilters = new System.Windows.Forms.ListBox();
            this.label11 = new System.Windows.Forms.Label();
            this.cbConnMonValIncoming = new System.Windows.Forms.CheckBox();
            this.cbConnMonValOutgoing = new System.Windows.Forms.CheckBox();
            this.cbConnMonValConnect = new System.Windows.Forms.CheckBox();
            this.cbEnableConnMon = new System.Windows.Forms.CheckBox();
            this.tabpgMemMon = new System.Windows.Forms.TabPage();
            this.grpMemMonFeatures = new System.Windows.Forms.GroupBox();
            this.cbMemDisableRWXHeap = new System.Windows.Forms.CheckBox();
            this.cbMemWipeAllocs = new System.Windows.Forms.CheckBox();
            this.cbMemMaxSens = new System.Windows.Forms.CheckBox();
            this.cbMemDoubleFree = new System.Windows.Forms.CheckBox();
            this.cbMemUseAfterFree = new System.Windows.Forms.CheckBox();
            this.cbMemEnableProcDEP = new System.Windows.Forms.CheckBox();
            this.cbMemValidateAllocsCrystal = new System.Windows.Forms.CheckBox();
            this.cbMemValidateAllocsWindows = new System.Windows.Forms.CheckBox();
            this.cbMemTermCorrupt = new System.Windows.Forms.CheckBox();
            this.cbMemVaryAllocSizes = new System.Windows.Forms.CheckBox();
            this.cbEnableMemMon = new System.Windows.Forms.CheckBox();
            this.tabpgApiMon1 = new System.Windows.Forms.TabPage();
            this.grpAPIMonFeatures1 = new System.Windows.Forms.GroupBox();
            this.cbAPIPromptOnDotNet = new System.Windows.Forms.CheckBox();
            this.cbAPISpawnProtProc = new System.Windows.Forms.CheckBox();
            this.cbAPIAntiDllPlanting = new System.Windows.Forms.CheckBox();
            this.cbAPIQueryUsrExeBlocked = new System.Windows.Forms.CheckBox();
            this.cbAPIMonEmuExcept = new System.Windows.Forms.CheckBox();
            this.cbAPIMonCheckExcept = new System.Windows.Forms.CheckBox();
            this.cbAPIDisableRWXStack = new System.Windows.Forms.CheckBox();
            this.cbAPICodeFromDownloads = new System.Windows.Forms.CheckBox();
            this.cbAPIMonRndAllocBase = new System.Windows.Forms.CheckBox();
            this.cbAPIInjectChild = new System.Windows.Forms.CheckBox();
            this.cbAPIMonCallOrigins = new System.Windows.Forms.CheckBox();
            this.cbAPIDisableRWXVA = new System.Windows.Forms.CheckBox();
            this.cbAPIAntiROPStack = new System.Windows.Forms.CheckBox();
            this.cbAPICodeFromInetTemp = new System.Windows.Forms.CheckBox();
            this.cbAPICodeFromLongPath = new System.Windows.Forms.CheckBox();
            this.cbAPICodeFromNet = new System.Windows.Forms.CheckBox();
            this.cbAPICodeFromTemp = new System.Windows.Forms.CheckBox();
            this.cbEnableAPIMon = new System.Windows.Forms.CheckBox();
            this.tabpgApiMon2 = new System.Windows.Forms.TabPage();
            this.grpApi = new System.Windows.Forms.GroupBox();
            this.btnModuleRWXAdd = new System.Windows.Forms.Button();
            this.btnModuleRWXRemove = new System.Windows.Forms.Button();
            this.txtModuleRWXAdd = new System.Windows.Forms.TextBox();
            this.lbModuleRWX = new System.Windows.Forms.ListBox();
            this.cbAPIEnableExeWhitelist = new System.Windows.Forms.CheckBox();
            this.lblExeWhitelist = new System.Windows.Forms.Label();
            this.lbExeWhitelist = new System.Windows.Forms.ListBox();
            this.txtExecWlAdd = new System.Windows.Forms.TextBox();
            this.btnExeWlRemove = new System.Windows.Forms.Button();
            this.lblModuleRWXList = new System.Windows.Forms.Label();
            this.btnExeWlAdd = new System.Windows.Forms.Button();
            this.tabpgComMon = new System.Windows.Forms.TabPage();
            this.grpComFeatures = new System.Windows.Forms.GroupBox();
            this.btnAutoPopBlacklist = new System.Windows.Forms.Button();
            this.tabComWhiteBlackList = new System.Windows.Forms.TabControl();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.btnComWhitelistClsidProgIdRemove = new System.Windows.Forms.Button();
            this.btnComWhitelistClsidProgIdAdd = new System.Windows.Forms.Button();
            this.lblComWhitelistClsidProgId = new System.Windows.Forms.Label();
            this.txtComWhitelistClsidProgId = new System.Windows.Forms.TextBox();
            this.lbComWhitelistClsidProgId = new System.Windows.Forms.ListBox();
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.btnComBlacklistClsidProgIdRemove = new System.Windows.Forms.Button();
            this.btnComBlacklistClsidProgIdAdd = new System.Windows.Forms.Button();
            this.lblComBlacklistClsidProgId = new System.Windows.Forms.Label();
            this.txtComBlacklistClsidProgId = new System.Windows.Forms.TextBox();
            this.lbComBlacklistClsidProgId = new System.Windows.Forms.ListBox();
            this.tabPage7 = new System.Windows.Forms.TabPage();
            this.cbComJITPersistDecision = new System.Windows.Forms.CheckBox();
            this.btnComJITlistClsidProgIdRemove = new System.Windows.Forms.Button();
            this.btnComJITlistClsidProgIdAdd = new System.Windows.Forms.Button();
            this.lblComJITlistClsidProgId = new System.Windows.Forms.Label();
            this.txtComJITlistClsidProgId = new System.Windows.Forms.TextBox();
            this.lbComJITlistClsidProgId = new System.Windows.Forms.ListBox();
            this.cbComEnableBlacklist = new System.Windows.Forms.CheckBox();
            this.cbComEnableWhitelist = new System.Windows.Forms.CheckBox();
            this.cbEnableComMon = new System.Windows.Forms.CheckBox();
            this.grpInstSettings = new System.Windows.Forms.GroupBox();
            this.btnUninstallAll = new System.Windows.Forms.Button();
            this.grpMisc = new System.Windows.Forms.GroupBox();
            this.btnResetConfig = new System.Windows.Forms.Button();
            this.cbEnableDiDCore = new System.Windows.Forms.CheckBox();
            this.cbEnableBpCore = new System.Windows.Forms.CheckBox();
            this.lblWarning = new System.Windows.Forms.Label();
            this.cbAPIMonAddnAntiROP = new System.Windows.Forms.CheckBox();
            this.cbAPIAutoAllowDetours = new System.Windows.Forms.CheckBox();
            this.grpProtProc.SuspendLayout();
            this.grpProtMethods.SuspendLayout();
            this.tabFeatures.SuspendLayout();
            this.tabpgConnMon.SuspendLayout();
            this.grpConnMonFeatures.SuspendLayout();
            this.tabConnFilters.SuspendLayout();
            this.tabPage8.SuspendLayout();
            this.tabPage9.SuspendLayout();
            this.tabPage10.SuspendLayout();
            this.tabpgMemMon.SuspendLayout();
            this.grpMemMonFeatures.SuspendLayout();
            this.tabpgApiMon1.SuspendLayout();
            this.grpAPIMonFeatures1.SuspendLayout();
            this.tabpgApiMon2.SuspendLayout();
            this.grpApi.SuspendLayout();
            this.tabpgComMon.SuspendLayout();
            this.grpComFeatures.SuspendLayout();
            this.tabComWhiteBlackList.SuspendLayout();
            this.tabPage5.SuspendLayout();
            this.tabPage6.SuspendLayout();
            this.tabPage7.SuspendLayout();
            this.grpInstSettings.SuspendLayout();
            this.grpMisc.SuspendLayout();
            this.SuspendLayout();
            // 
            // grpProtProc
            // 
            this.grpProtProc.Controls.Add(this.btnProcEditConf);
            this.grpProtProc.Controls.Add(this.lbProtProc);
            this.grpProtProc.Controls.Add(this.btnRemoveProc);
            this.grpProtProc.Controls.Add(this.btnAddProc);
            this.grpProtProc.Controls.Add(this.txtAddProc);
            this.grpProtProc.Controls.Add(this.label1);
            this.grpProtProc.Location = new System.Drawing.Point(6, 16);
            this.grpProtProc.Name = "grpProtProc";
            this.grpProtProc.Size = new System.Drawing.Size(212, 235);
            this.grpProtProc.TabIndex = 0;
            this.grpProtProc.TabStop = false;
            this.grpProtProc.Text = "Protected Processes";
            // 
            // btnProcEditConf
            // 
            this.btnProcEditConf.Location = new System.Drawing.Point(9, 206);
            this.btnProcEditConf.Name = "btnProcEditConf";
            this.btnProcEditConf.Size = new System.Drawing.Size(128, 23);
            this.btnProcEditConf.TabIndex = 6;
            this.btnProcEditConf.Text = "Edit Configuration";
            this.btnProcEditConf.UseVisualStyleBackColor = true;
            this.btnProcEditConf.Click += new System.EventHandler(this.btnProcEditConf_Click);
            // 
            // lbProtProc
            // 
            this.lbProtProc.FormattingEnabled = true;
            this.lbProtProc.Location = new System.Drawing.Point(9, 63);
            this.lbProtProc.Name = "lbProtProc";
            this.lbProtProc.Size = new System.Drawing.Size(197, 134);
            this.lbProtProc.TabIndex = 5;
            // 
            // btnRemoveProc
            // 
            this.btnRemoveProc.Location = new System.Drawing.Point(144, 206);
            this.btnRemoveProc.Name = "btnRemoveProc";
            this.btnRemoveProc.Size = new System.Drawing.Size(62, 23);
            this.btnRemoveProc.TabIndex = 4;
            this.btnRemoveProc.Text = "Remove";
            this.btnRemoveProc.UseVisualStyleBackColor = true;
            this.btnRemoveProc.Click += new System.EventHandler(this.btnRemoveProc_Click);
            // 
            // btnAddProc
            // 
            this.btnAddProc.Location = new System.Drawing.Point(146, 34);
            this.btnAddProc.Name = "btnAddProc";
            this.btnAddProc.Size = new System.Drawing.Size(60, 23);
            this.btnAddProc.TabIndex = 3;
            this.btnAddProc.Text = "Add";
            this.btnAddProc.UseVisualStyleBackColor = true;
            this.btnAddProc.Click += new System.EventHandler(this.btnAddProc_Click);
            // 
            // txtAddProc
            // 
            this.txtAddProc.Location = new System.Drawing.Point(9, 37);
            this.txtAddProc.Name = "txtAddProc";
            this.txtAddProc.Size = new System.Drawing.Size(131, 20);
            this.txtAddProc.TabIndex = 2;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 16);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(79, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Process Name:";
            // 
            // grpProtMethods
            // 
            this.grpProtMethods.Controls.Add(this.tabFeatures);
            this.grpProtMethods.Location = new System.Drawing.Point(224, 16);
            this.grpProtMethods.Name = "grpProtMethods";
            this.grpProtMethods.Size = new System.Drawing.Size(423, 345);
            this.grpProtMethods.TabIndex = 1;
            this.grpProtMethods.TabStop = false;
            this.grpProtMethods.Text = "Protection Methods (Defaults)";
            // 
            // tabFeatures
            // 
            this.tabFeatures.Controls.Add(this.tabpgConnMon);
            this.tabFeatures.Controls.Add(this.tabpgMemMon);
            this.tabFeatures.Controls.Add(this.tabpgApiMon1);
            this.tabFeatures.Controls.Add(this.tabpgApiMon2);
            this.tabFeatures.Controls.Add(this.tabpgComMon);
            this.tabFeatures.Location = new System.Drawing.Point(6, 16);
            this.tabFeatures.Name = "tabFeatures";
            this.tabFeatures.SelectedIndex = 0;
            this.tabFeatures.Size = new System.Drawing.Size(411, 322);
            this.tabFeatures.TabIndex = 0;
            // 
            // tabpgConnMon
            // 
            this.tabpgConnMon.Controls.Add(this.btnImportFilters);
            this.tabpgConnMon.Controls.Add(this.grpConnMonFeatures);
            this.tabpgConnMon.Controls.Add(this.cbEnableConnMon);
            this.tabpgConnMon.Location = new System.Drawing.Point(4, 22);
            this.tabpgConnMon.Name = "tabpgConnMon";
            this.tabpgConnMon.Padding = new System.Windows.Forms.Padding(3);
            this.tabpgConnMon.Size = new System.Drawing.Size(403, 296);
            this.tabpgConnMon.TabIndex = 0;
            this.tabpgConnMon.Text = "Connection Monitor";
            this.tabpgConnMon.UseVisualStyleBackColor = true;
            // 
            // btnImportFilters
            // 
            this.btnImportFilters.Location = new System.Drawing.Point(272, 1);
            this.btnImportFilters.Name = "btnImportFilters";
            this.btnImportFilters.Size = new System.Drawing.Size(96, 23);
            this.btnImportFilters.TabIndex = 4;
            this.btnImportFilters.Text = "Manage Filters";
            this.btnImportFilters.UseVisualStyleBackColor = true;
            this.btnImportFilters.Click += new System.EventHandler(this.btnImportFilters_Click);
            // 
            // grpConnMonFeatures
            // 
            this.grpConnMonFeatures.Controls.Add(this.tabConnFilters);
            this.grpConnMonFeatures.Controls.Add(this.cbConnMonValIncoming);
            this.grpConnMonFeatures.Controls.Add(this.cbConnMonValOutgoing);
            this.grpConnMonFeatures.Controls.Add(this.cbConnMonValConnect);
            this.grpConnMonFeatures.Location = new System.Drawing.Point(9, 20);
            this.grpConnMonFeatures.Name = "grpConnMonFeatures";
            this.grpConnMonFeatures.Size = new System.Drawing.Size(388, 270);
            this.grpConnMonFeatures.TabIndex = 1;
            this.grpConnMonFeatures.TabStop = false;
            this.grpConnMonFeatures.Text = "Feature Settings";
            // 
            // tabConnFilters
            // 
            this.tabConnFilters.Controls.Add(this.tabPage8);
            this.tabConnFilters.Controls.Add(this.tabPage9);
            this.tabConnFilters.Controls.Add(this.tabPage10);
            this.tabConnFilters.Location = new System.Drawing.Point(6, 62);
            this.tabConnFilters.Name = "tabConnFilters";
            this.tabConnFilters.SelectedIndex = 0;
            this.tabConnFilters.Size = new System.Drawing.Size(376, 202);
            this.tabConnFilters.TabIndex = 3;
            // 
            // tabPage8
            // 
            this.tabPage8.Controls.Add(this.lbEnabledConnFilters);
            this.tabPage8.Controls.Add(this.lblEnabledConnFilters);
            this.tabPage8.Location = new System.Drawing.Point(4, 22);
            this.tabPage8.Name = "tabPage8";
            this.tabPage8.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage8.Size = new System.Drawing.Size(368, 176);
            this.tabPage8.TabIndex = 0;
            this.tabPage8.Text = "Connect Filters";
            this.tabPage8.UseVisualStyleBackColor = true;
            // 
            // lbEnabledConnFilters
            // 
            this.lbEnabledConnFilters.FormattingEnabled = true;
            this.lbEnabledConnFilters.Location = new System.Drawing.Point(11, 21);
            this.lbEnabledConnFilters.Name = "lbEnabledConnFilters";
            this.lbEnabledConnFilters.Size = new System.Drawing.Size(351, 147);
            this.lbEnabledConnFilters.TabIndex = 6;
            // 
            // lblEnabledConnFilters
            // 
            this.lblEnabledConnFilters.AutoSize = true;
            this.lblEnabledConnFilters.Location = new System.Drawing.Point(8, 3);
            this.lblEnabledConnFilters.Name = "lblEnabledConnFilters";
            this.lblEnabledConnFilters.Size = new System.Drawing.Size(37, 13);
            this.lblEnabledConnFilters.TabIndex = 4;
            this.lblEnabledConnFilters.Text = "Filters:";
            // 
            // tabPage9
            // 
            this.tabPage9.Controls.Add(this.lbEnabledOutgoingFilters);
            this.tabPage9.Controls.Add(this.label9);
            this.tabPage9.Location = new System.Drawing.Point(4, 22);
            this.tabPage9.Name = "tabPage9";
            this.tabPage9.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage9.Size = new System.Drawing.Size(368, 176);
            this.tabPage9.TabIndex = 1;
            this.tabPage9.Text = "Outgoing Data Filters";
            this.tabPage9.UseVisualStyleBackColor = true;
            // 
            // lbEnabledOutgoingFilters
            // 
            this.lbEnabledOutgoingFilters.FormattingEnabled = true;
            this.lbEnabledOutgoingFilters.Location = new System.Drawing.Point(11, 21);
            this.lbEnabledOutgoingFilters.Name = "lbEnabledOutgoingFilters";
            this.lbEnabledOutgoingFilters.Size = new System.Drawing.Size(351, 147);
            this.lbEnabledOutgoingFilters.TabIndex = 6;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(8, 3);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(37, 13);
            this.label9.TabIndex = 4;
            this.label9.Text = "Filters:";
            // 
            // tabPage10
            // 
            this.tabPage10.Controls.Add(this.lbEnabledIncomingFilters);
            this.tabPage10.Controls.Add(this.label11);
            this.tabPage10.Location = new System.Drawing.Point(4, 22);
            this.tabPage10.Name = "tabPage10";
            this.tabPage10.Size = new System.Drawing.Size(368, 176);
            this.tabPage10.TabIndex = 2;
            this.tabPage10.Text = "Incoming Data Filters";
            this.tabPage10.UseVisualStyleBackColor = true;
            // 
            // lbEnabledIncomingFilters
            // 
            this.lbEnabledIncomingFilters.FormattingEnabled = true;
            this.lbEnabledIncomingFilters.Location = new System.Drawing.Point(11, 21);
            this.lbEnabledIncomingFilters.Name = "lbEnabledIncomingFilters";
            this.lbEnabledIncomingFilters.Size = new System.Drawing.Size(351, 147);
            this.lbEnabledIncomingFilters.TabIndex = 6;
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(8, 3);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(37, 13);
            this.label11.TabIndex = 4;
            this.label11.Text = "Filters:";
            // 
            // cbConnMonValIncoming
            // 
            this.cbConnMonValIncoming.AutoSize = true;
            this.cbConnMonValIncoming.Location = new System.Drawing.Point(6, 39);
            this.cbConnMonValIncoming.Name = "cbConnMonValIncoming";
            this.cbConnMonValIncoming.Size = new System.Drawing.Size(136, 17);
            this.cbConnMonValIncoming.TabIndex = 2;
            this.cbConnMonValIncoming.Text = "Validate Incoming Data";
            this.cbConnMonValIncoming.UseVisualStyleBackColor = true;
            this.cbConnMonValIncoming.CheckedChanged += new System.EventHandler(this.cbConnMonValIncoming_CheckedChanged);
            // 
            // cbConnMonValOutgoing
            // 
            this.cbConnMonValOutgoing.AutoSize = true;
            this.cbConnMonValOutgoing.Location = new System.Drawing.Point(179, 19);
            this.cbConnMonValOutgoing.Name = "cbConnMonValOutgoing";
            this.cbConnMonValOutgoing.Size = new System.Drawing.Size(136, 17);
            this.cbConnMonValOutgoing.TabIndex = 1;
            this.cbConnMonValOutgoing.Text = "Validate Outgoing Data";
            this.cbConnMonValOutgoing.UseVisualStyleBackColor = true;
            this.cbConnMonValOutgoing.CheckedChanged += new System.EventHandler(this.cbConnMonValOutgoing_CheckedChanged);
            // 
            // cbConnMonValConnect
            // 
            this.cbConnMonValConnect.AutoSize = true;
            this.cbConnMonValConnect.Location = new System.Drawing.Point(6, 16);
            this.cbConnMonValConnect.Name = "cbConnMonValConnect";
            this.cbConnMonValConnect.Size = new System.Drawing.Size(167, 17);
            this.cbConnMonValConnect.TabIndex = 0;
            this.cbConnMonValConnect.Text = "Validate Outgoing Connection";
            this.cbConnMonValConnect.UseVisualStyleBackColor = true;
            this.cbConnMonValConnect.CheckedChanged += new System.EventHandler(this.cbConnMonValConnect_CheckedChanged);
            // 
            // cbEnableConnMon
            // 
            this.cbEnableConnMon.AutoSize = true;
            this.cbEnableConnMon.Location = new System.Drawing.Point(9, 3);
            this.cbEnableConnMon.Name = "cbEnableConnMon";
            this.cbEnableConnMon.Size = new System.Drawing.Size(98, 17);
            this.cbEnableConnMon.TabIndex = 0;
            this.cbEnableConnMon.Text = "Enable Feature";
            this.cbEnableConnMon.UseVisualStyleBackColor = true;
            this.cbEnableConnMon.CheckedChanged += new System.EventHandler(this.cbEnableConnMon_CheckedChanged);
            // 
            // tabpgMemMon
            // 
            this.tabpgMemMon.Controls.Add(this.grpMemMonFeatures);
            this.tabpgMemMon.Controls.Add(this.cbEnableMemMon);
            this.tabpgMemMon.Location = new System.Drawing.Point(4, 22);
            this.tabpgMemMon.Name = "tabpgMemMon";
            this.tabpgMemMon.Padding = new System.Windows.Forms.Padding(3);
            this.tabpgMemMon.Size = new System.Drawing.Size(403, 296);
            this.tabpgMemMon.TabIndex = 1;
            this.tabpgMemMon.Text = "Memory Monitor";
            this.tabpgMemMon.UseVisualStyleBackColor = true;
            // 
            // grpMemMonFeatures
            // 
            this.grpMemMonFeatures.Controls.Add(this.cbMemDisableRWXHeap);
            this.grpMemMonFeatures.Controls.Add(this.cbMemWipeAllocs);
            this.grpMemMonFeatures.Controls.Add(this.cbMemMaxSens);
            this.grpMemMonFeatures.Controls.Add(this.cbMemDoubleFree);
            this.grpMemMonFeatures.Controls.Add(this.cbMemUseAfterFree);
            this.grpMemMonFeatures.Controls.Add(this.cbMemEnableProcDEP);
            this.grpMemMonFeatures.Controls.Add(this.cbMemValidateAllocsCrystal);
            this.grpMemMonFeatures.Controls.Add(this.cbMemValidateAllocsWindows);
            this.grpMemMonFeatures.Controls.Add(this.cbMemTermCorrupt);
            this.grpMemMonFeatures.Controls.Add(this.cbMemVaryAllocSizes);
            this.grpMemMonFeatures.Location = new System.Drawing.Point(9, 20);
            this.grpMemMonFeatures.Name = "grpMemMonFeatures";
            this.grpMemMonFeatures.Size = new System.Drawing.Size(175, 270);
            this.grpMemMonFeatures.TabIndex = 2;
            this.grpMemMonFeatures.TabStop = false;
            this.grpMemMonFeatures.Text = "Feature Settings";
            // 
            // cbMemDisableRWXHeap
            // 
            this.cbMemDisableRWXHeap.AutoSize = true;
            this.cbMemDisableRWXHeap.Location = new System.Drawing.Point(6, 154);
            this.cbMemDisableRWXHeap.Name = "cbMemDisableRWXHeap";
            this.cbMemDisableRWXHeap.Size = new System.Drawing.Size(147, 17);
            this.cbMemDisableRWXHeap.TabIndex = 8;
            this.cbMemDisableRWXHeap.Text = "Disable RWX Heap (Soft)";
            this.cbMemDisableRWXHeap.UseVisualStyleBackColor = true;
            this.cbMemDisableRWXHeap.CheckedChanged += new System.EventHandler(this.cbMemDisableRWXHeap_CheckedChanged);
            // 
            // cbMemWipeAllocs
            // 
            this.cbMemWipeAllocs.AutoSize = true;
            this.cbMemWipeAllocs.Location = new System.Drawing.Point(6, 131);
            this.cbMemWipeAllocs.Name = "cbMemWipeAllocs";
            this.cbMemWipeAllocs.Size = new System.Drawing.Size(144, 17);
            this.cbMemWipeAllocs.TabIndex = 7;
            this.cbMemWipeAllocs.Text = "Wipe Allocations on Free";
            this.cbMemWipeAllocs.UseVisualStyleBackColor = true;
            this.cbMemWipeAllocs.CheckedChanged += new System.EventHandler(this.cbMemWipeAllocs_CheckedChanged);
            // 
            // cbMemMaxSens
            // 
            this.cbMemMaxSens.AutoSize = true;
            this.cbMemMaxSens.Location = new System.Drawing.Point(5, 225);
            this.cbMemMaxSens.Name = "cbMemMaxSens";
            this.cbMemMaxSens.Size = new System.Drawing.Size(151, 17);
            this.cbMemMaxSens.TabIndex = 3;
            this.cbMemMaxSens.Text = "Max. Monitoring Sensitivity";
            this.cbMemMaxSens.UseVisualStyleBackColor = true;
            this.cbMemMaxSens.CheckedChanged += new System.EventHandler(this.cbMemMonSerialized_CheckedChanged);
            // 
            // cbMemDoubleFree
            // 
            this.cbMemDoubleFree.AutoSize = true;
            this.cbMemDoubleFree.Location = new System.Drawing.Point(5, 202);
            this.cbMemDoubleFree.Name = "cbMemDoubleFree";
            this.cbMemDoubleFree.Size = new System.Drawing.Size(135, 17);
            this.cbMemDoubleFree.TabIndex = 6;
            this.cbMemDoubleFree.Text = "Double-Free Protection";
            this.cbMemDoubleFree.UseVisualStyleBackColor = true;
            this.cbMemDoubleFree.CheckedChanged += new System.EventHandler(this.cbMemDoubleFree_CheckedChanged);
            // 
            // cbMemUseAfterFree
            // 
            this.cbMemUseAfterFree.AutoSize = true;
            this.cbMemUseAfterFree.Location = new System.Drawing.Point(5, 177);
            this.cbMemUseAfterFree.Name = "cbMemUseAfterFree";
            this.cbMemUseAfterFree.Size = new System.Drawing.Size(145, 17);
            this.cbMemUseAfterFree.TabIndex = 5;
            this.cbMemUseAfterFree.Text = "Use-After-Free Protection";
            this.cbMemUseAfterFree.UseVisualStyleBackColor = true;
            this.cbMemUseAfterFree.CheckedChanged += new System.EventHandler(this.cbMemUseAfterFree_CheckedChanged);
            // 
            // cbMemEnableProcDEP
            // 
            this.cbMemEnableProcDEP.AutoSize = true;
            this.cbMemEnableProcDEP.Location = new System.Drawing.Point(6, 108);
            this.cbMemEnableProcDEP.Name = "cbMemEnableProcDEP";
            this.cbMemEnableProcDEP.Size = new System.Drawing.Size(125, 17);
            this.cbMemEnableProcDEP.TabIndex = 4;
            this.cbMemEnableProcDEP.Text = "Enable Process DEP";
            this.cbMemEnableProcDEP.UseVisualStyleBackColor = true;
            this.cbMemEnableProcDEP.CheckedChanged += new System.EventHandler(this.cbMemEnableProcDEP_CheckedChanged);
            // 
            // cbMemValidateAllocsCrystal
            // 
            this.cbMemValidateAllocsCrystal.AutoSize = true;
            this.cbMemValidateAllocsCrystal.Location = new System.Drawing.Point(6, 85);
            this.cbMemValidateAllocsCrystal.Name = "cbMemValidateAllocsCrystal";
            this.cbMemValidateAllocsCrystal.Size = new System.Drawing.Size(152, 17);
            this.cbMemValidateAllocsCrystal.TabIndex = 3;
            this.cbMemValidateAllocsCrystal.Text = "Crystal Validate Allocations";
            this.cbMemValidateAllocsCrystal.UseVisualStyleBackColor = true;
            this.cbMemValidateAllocsCrystal.CheckedChanged += new System.EventHandler(this.cbMemValidateAllocsCrystal_CheckedChanged);
            // 
            // cbMemValidateAllocsWindows
            // 
            this.cbMemValidateAllocsWindows.AutoSize = true;
            this.cbMemValidateAllocsWindows.Location = new System.Drawing.Point(6, 62);
            this.cbMemValidateAllocsWindows.Name = "cbMemValidateAllocsWindows";
            this.cbMemValidateAllocsWindows.Size = new System.Drawing.Size(165, 17);
            this.cbMemValidateAllocsWindows.TabIndex = 2;
            this.cbMemValidateAllocsWindows.Text = "Windows Validate Allocations";
            this.cbMemValidateAllocsWindows.UseVisualStyleBackColor = true;
            this.cbMemValidateAllocsWindows.CheckedChanged += new System.EventHandler(this.cbMemValidateAllocsWindows_CheckedChanged);
            // 
            // cbMemTermCorrupt
            // 
            this.cbMemTermCorrupt.AutoSize = true;
            this.cbMemTermCorrupt.Location = new System.Drawing.Point(6, 39);
            this.cbMemTermCorrupt.Name = "cbMemTermCorrupt";
            this.cbMemTermCorrupt.Size = new System.Drawing.Size(158, 17);
            this.cbMemTermCorrupt.TabIndex = 1;
            this.cbMemTermCorrupt.Text = "Set Terminate on Corruption";
            this.cbMemTermCorrupt.UseVisualStyleBackColor = true;
            this.cbMemTermCorrupt.CheckedChanged += new System.EventHandler(this.cbMemTermCorrupt_CheckedChanged);
            // 
            // cbMemVaryAllocSizes
            // 
            this.cbMemVaryAllocSizes.AutoSize = true;
            this.cbMemVaryAllocSizes.Location = new System.Drawing.Point(6, 16);
            this.cbMemVaryAllocSizes.Name = "cbMemVaryAllocSizes";
            this.cbMemVaryAllocSizes.Size = new System.Drawing.Size(124, 17);
            this.cbMemVaryAllocSizes.TabIndex = 0;
            this.cbMemVaryAllocSizes.Text = "Vary Allocation Sizes";
            this.cbMemVaryAllocSizes.UseVisualStyleBackColor = true;
            this.cbMemVaryAllocSizes.CheckedChanged += new System.EventHandler(this.cbMemVaryAllocSizes_CheckedChanged);
            // 
            // cbEnableMemMon
            // 
            this.cbEnableMemMon.AutoSize = true;
            this.cbEnableMemMon.Location = new System.Drawing.Point(9, 3);
            this.cbEnableMemMon.Name = "cbEnableMemMon";
            this.cbEnableMemMon.Size = new System.Drawing.Size(98, 17);
            this.cbEnableMemMon.TabIndex = 1;
            this.cbEnableMemMon.Text = "Enable Feature";
            this.cbEnableMemMon.UseVisualStyleBackColor = true;
            this.cbEnableMemMon.CheckedChanged += new System.EventHandler(this.cbEnableMemMon_CheckedChanged);
            // 
            // tabpgApiMon1
            // 
            this.tabpgApiMon1.Controls.Add(this.grpAPIMonFeatures1);
            this.tabpgApiMon1.Controls.Add(this.cbEnableAPIMon);
            this.tabpgApiMon1.Location = new System.Drawing.Point(4, 22);
            this.tabpgApiMon1.Name = "tabpgApiMon1";
            this.tabpgApiMon1.Size = new System.Drawing.Size(403, 296);
            this.tabpgApiMon1.TabIndex = 2;
            this.tabpgApiMon1.Text = "API Monitor [1]";
            this.tabpgApiMon1.UseVisualStyleBackColor = true;
            // 
            // grpAPIMonFeatures1
            // 
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIMonAddnAntiROP);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIPromptOnDotNet);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPISpawnProtProc);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIAntiDllPlanting);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIQueryUsrExeBlocked);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIMonEmuExcept);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIMonCheckExcept);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIDisableRWXStack);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPICodeFromDownloads);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIMonRndAllocBase);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIInjectChild);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIMonCallOrigins);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIDisableRWXVA);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPIAntiROPStack);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPICodeFromInetTemp);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPICodeFromLongPath);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPICodeFromNet);
            this.grpAPIMonFeatures1.Controls.Add(this.cbAPICodeFromTemp);
            this.grpAPIMonFeatures1.Location = new System.Drawing.Point(9, 20);
            this.grpAPIMonFeatures1.Name = "grpAPIMonFeatures1";
            this.grpAPIMonFeatures1.Size = new System.Drawing.Size(391, 273);
            this.grpAPIMonFeatures1.TabIndex = 2;
            this.grpAPIMonFeatures1.TabStop = false;
            this.grpAPIMonFeatures1.Text = "Feature Settings";
            // 
            // cbAPIPromptOnDotNet
            // 
            this.cbAPIPromptOnDotNet.AutoSize = true;
            this.cbAPIPromptOnDotNet.Location = new System.Drawing.Point(9, 246);
            this.cbAPIPromptOnDotNet.Name = "cbAPIPromptOnDotNet";
            this.cbAPIPromptOnDotNet.Size = new System.Drawing.Size(255, 17);
            this.cbAPIPromptOnDotNet.TabIndex = 64;
            this.cbAPIPromptOnDotNet.Text = "Prompt to Relax Protection for .NET Applications";
            this.cbAPIPromptOnDotNet.UseVisualStyleBackColor = true;
            this.cbAPIPromptOnDotNet.CheckedChanged += new System.EventHandler(this.cbAPIPromptOnDotNet_CheckedChanged);
            // 
            // cbAPISpawnProtProc
            // 
            this.cbAPISpawnProtProc.AutoSize = true;
            this.cbAPISpawnProtProc.Location = new System.Drawing.Point(188, 17);
            this.cbAPISpawnProtProc.Name = "cbAPISpawnProtProc";
            this.cbAPISpawnProtProc.Size = new System.Drawing.Size(205, 17);
            this.cbAPISpawnProtProc.TabIndex = 15;
            this.cbAPISpawnProtProc.Text = "Run Protected Apps Without Warning";
            this.cbAPISpawnProtProc.UseVisualStyleBackColor = true;
            this.cbAPISpawnProtProc.CheckedChanged += new System.EventHandler(this.cbAPISpawnProtProc_CheckedChanged);
            // 
            // cbAPIAntiDllPlanting
            // 
            this.cbAPIAntiDllPlanting.AutoSize = true;
            this.cbAPIAntiDllPlanting.Location = new System.Drawing.Point(9, 133);
            this.cbAPIAntiDllPlanting.Name = "cbAPIAntiDllPlanting";
            this.cbAPIAntiDllPlanting.Size = new System.Drawing.Size(147, 17);
            this.cbAPIAntiDllPlanting.TabIndex = 14;
            this.cbAPIAntiDllPlanting.Text = "Anti-DLL Planting Checks";
            this.cbAPIAntiDllPlanting.UseVisualStyleBackColor = true;
            this.cbAPIAntiDllPlanting.CheckedChanged += new System.EventHandler(this.cbAPIAntiDllPlanting_CheckedChanged);
            // 
            // cbAPIQueryUsrExeBlocked
            // 
            this.cbAPIQueryUsrExeBlocked.AutoSize = true;
            this.cbAPIQueryUsrExeBlocked.Location = new System.Drawing.Point(9, 177);
            this.cbAPIQueryUsrExeBlocked.Name = "cbAPIQueryUsrExeBlocked";
            this.cbAPIQueryUsrExeBlocked.Size = new System.Drawing.Size(162, 17);
            this.cbAPIQueryUsrExeBlocked.TabIndex = 13;
            this.cbAPIQueryUsrExeBlocked.Text = "Prompt User on Exe Blocked";
            this.cbAPIQueryUsrExeBlocked.UseVisualStyleBackColor = true;
            this.cbAPIQueryUsrExeBlocked.CheckedChanged += new System.EventHandler(this.cbAPIQueryUsrExeBlocked_CheckedChanged);
            // 
            // cbAPIMonEmuExcept
            // 
            this.cbAPIMonEmuExcept.AutoSize = true;
            this.cbAPIMonEmuExcept.Location = new System.Drawing.Point(188, 157);
            this.cbAPIMonEmuExcept.Name = "cbAPIMonEmuExcept";
            this.cbAPIMonEmuExcept.Size = new System.Drawing.Size(159, 17);
            this.cbAPIMonEmuExcept.TabIndex = 12;
            this.cbAPIMonEmuExcept.Text = "Emulate Exception Handlers";
            this.cbAPIMonEmuExcept.UseVisualStyleBackColor = true;
            this.cbAPIMonEmuExcept.CheckedChanged += new System.EventHandler(this.cbAPIMonEmuExcept_CheckedChanged);
            // 
            // cbAPIMonCheckExcept
            // 
            this.cbAPIMonCheckExcept.AutoSize = true;
            this.cbAPIMonCheckExcept.Location = new System.Drawing.Point(188, 134);
            this.cbAPIMonCheckExcept.Name = "cbAPIMonCheckExcept";
            this.cbAPIMonCheckExcept.Size = new System.Drawing.Size(162, 17);
            this.cbAPIMonCheckExcept.TabIndex = 11;
            this.cbAPIMonCheckExcept.Text = "Watch Exception Dispatcher";
            this.cbAPIMonCheckExcept.UseVisualStyleBackColor = true;
            this.cbAPIMonCheckExcept.CheckedChanged += new System.EventHandler(this.cbAPIMonCheckExcept_CheckedChanged);
            // 
            // cbAPIDisableRWXStack
            // 
            this.cbAPIDisableRWXStack.AutoSize = true;
            this.cbAPIDisableRWXStack.Location = new System.Drawing.Point(188, 65);
            this.cbAPIDisableRWXStack.Name = "cbAPIDisableRWXStack";
            this.cbAPIDisableRWXStack.Size = new System.Drawing.Size(149, 17);
            this.cbAPIDisableRWXStack.TabIndex = 10;
            this.cbAPIDisableRWXStack.Text = "Disable RWX Stack (Soft)";
            this.cbAPIDisableRWXStack.UseVisualStyleBackColor = true;
            this.cbAPIDisableRWXStack.CheckedChanged += new System.EventHandler(this.cbAPIDisableRWXStack_CheckedChanged);
            // 
            // cbAPICodeFromDownloads
            // 
            this.cbAPICodeFromDownloads.AutoSize = true;
            this.cbAPICodeFromDownloads.Location = new System.Drawing.Point(9, 110);
            this.cbAPICodeFromDownloads.Name = "cbAPICodeFromDownloads";
            this.cbAPICodeFromDownloads.Size = new System.Drawing.Size(170, 17);
            this.cbAPICodeFromDownloads.TabIndex = 9;
            this.cbAPICodeFromDownloads.Text = "Block Code from \\Downloads\\";
            this.cbAPICodeFromDownloads.UseVisualStyleBackColor = true;
            this.cbAPICodeFromDownloads.CheckedChanged += new System.EventHandler(this.cbAPICodeFromDownloads_CheckedChanged);
            // 
            // cbAPIMonRndAllocBase
            // 
            this.cbAPIMonRndAllocBase.AutoSize = true;
            this.cbAPIMonRndAllocBase.Location = new System.Drawing.Point(187, 111);
            this.cbAPIMonRndAllocBase.Name = "cbAPIMonRndAllocBase";
            this.cbAPIMonRndAllocBase.Size = new System.Drawing.Size(160, 17);
            this.cbAPIMonRndAllocBase.TabIndex = 6;
            this.cbAPIMonRndAllocBase.Text = "Randomize Allocation Bases";
            this.cbAPIMonRndAllocBase.UseVisualStyleBackColor = true;
            this.cbAPIMonRndAllocBase.CheckedChanged += new System.EventHandler(this.cbAPIMonRndAllocBase_CheckedChanged);
            // 
            // cbAPIInjectChild
            // 
            this.cbAPIInjectChild.AutoSize = true;
            this.cbAPIInjectChild.Location = new System.Drawing.Point(9, 156);
            this.cbAPIInjectChild.Name = "cbAPIInjectChild";
            this.cbAPIInjectChild.Size = new System.Drawing.Size(135, 17);
            this.cbAPIInjectChild.TabIndex = 8;
            this.cbAPIInjectChild.Text = "Run in Child Processes";
            this.cbAPIInjectChild.UseVisualStyleBackColor = true;
            this.cbAPIInjectChild.CheckedChanged += new System.EventHandler(this.cbAPIInjectChild_CheckedChanged);
            // 
            // cbAPIMonCallOrigins
            // 
            this.cbAPIMonCallOrigins.AutoSize = true;
            this.cbAPIMonCallOrigins.Location = new System.Drawing.Point(188, 88);
            this.cbAPIMonCallOrigins.Name = "cbAPIMonCallOrigins";
            this.cbAPIMonCallOrigins.Size = new System.Drawing.Size(116, 17);
            this.cbAPIMonCallOrigins.TabIndex = 7;
            this.cbAPIMonCallOrigins.Text = "Monitor Call Origins";
            this.cbAPIMonCallOrigins.UseVisualStyleBackColor = true;
            this.cbAPIMonCallOrigins.CheckedChanged += new System.EventHandler(this.cbAPIMonCallOrigins_CheckedChanged);
            // 
            // cbAPIDisableRWXVA
            // 
            this.cbAPIDisableRWXVA.AutoSize = true;
            this.cbAPIDisableRWXVA.Location = new System.Drawing.Point(188, 42);
            this.cbAPIDisableRWXVA.Name = "cbAPIDisableRWXVA";
            this.cbAPIDisableRWXVA.Size = new System.Drawing.Size(158, 17);
            this.cbAPIDisableRWXVA.TabIndex = 6;
            this.cbAPIDisableRWXVA.Text = "Disable RWX Memory (Soft)";
            this.cbAPIDisableRWXVA.UseVisualStyleBackColor = true;
            this.cbAPIDisableRWXVA.CheckedChanged += new System.EventHandler(this.cbAPIDisableRWX_CheckedChanged);
            // 
            // cbAPIAntiROPStack
            // 
            this.cbAPIAntiROPStack.AutoSize = true;
            this.cbAPIAntiROPStack.Location = new System.Drawing.Point(9, 200);
            this.cbAPIAntiROPStack.Name = "cbAPIAntiROPStack";
            this.cbAPIAntiROPStack.Size = new System.Drawing.Size(101, 17);
            this.cbAPIAntiROPStack.TabIndex = 5;
            this.cbAPIAntiROPStack.Text = "Anti-ROP Stack";
            this.cbAPIAntiROPStack.UseVisualStyleBackColor = true;
            this.cbAPIAntiROPStack.CheckedChanged += new System.EventHandler(this.cbAPIAntiROPStack_CheckedChanged);
            // 
            // cbAPICodeFromInetTemp
            // 
            this.cbAPICodeFromInetTemp.AutoSize = true;
            this.cbAPICodeFromInetTemp.Location = new System.Drawing.Point(9, 88);
            this.cbAPICodeFromInetTemp.Name = "cbAPICodeFromInetTemp";
            this.cbAPICodeFromInetTemp.Size = new System.Drawing.Size(161, 17);
            this.cbAPICodeFromInetTemp.TabIndex = 3;
            this.cbAPICodeFromInetTemp.Text = "Block Code from Inet. Temp.";
            this.cbAPICodeFromInetTemp.UseVisualStyleBackColor = true;
            this.cbAPICodeFromInetTemp.CheckedChanged += new System.EventHandler(this.cbAPICodeFromInetTemp_CheckedChanged);
            // 
            // cbAPICodeFromLongPath
            // 
            this.cbAPICodeFromLongPath.AutoSize = true;
            this.cbAPICodeFromLongPath.Location = new System.Drawing.Point(9, 65);
            this.cbAPICodeFromLongPath.Name = "cbAPICodeFromLongPath";
            this.cbAPICodeFromLongPath.Size = new System.Drawing.Size(155, 17);
            this.cbAPICodeFromLongPath.TabIndex = 2;
            this.cbAPICodeFromLongPath.Text = "Block Code with Long Path";
            this.cbAPICodeFromLongPath.UseVisualStyleBackColor = true;
            this.cbAPICodeFromLongPath.CheckedChanged += new System.EventHandler(this.cbAPICodeFromLongPath_CheckedChanged);
            // 
            // cbAPICodeFromNet
            // 
            this.cbAPICodeFromNet.AutoSize = true;
            this.cbAPICodeFromNet.Location = new System.Drawing.Point(9, 42);
            this.cbAPICodeFromNet.Name = "cbAPICodeFromNet";
            this.cbAPICodeFromNet.Size = new System.Drawing.Size(158, 17);
            this.cbAPICodeFromNet.TabIndex = 1;
            this.cbAPICodeFromNet.Text = "Block Code from Net. Share";
            this.cbAPICodeFromNet.UseVisualStyleBackColor = true;
            this.cbAPICodeFromNet.CheckedChanged += new System.EventHandler(this.cbAPICodeFromNet_CheckedChanged);
            // 
            // cbAPICodeFromTemp
            // 
            this.cbAPICodeFromTemp.AutoSize = true;
            this.cbAPICodeFromTemp.Location = new System.Drawing.Point(9, 19);
            this.cbAPICodeFromTemp.Name = "cbAPICodeFromTemp";
            this.cbAPICodeFromTemp.Size = new System.Drawing.Size(156, 17);
            this.cbAPICodeFromTemp.TabIndex = 0;
            this.cbAPICodeFromTemp.Text = "Block Code from Temp. Dir.";
            this.cbAPICodeFromTemp.UseVisualStyleBackColor = true;
            this.cbAPICodeFromTemp.CheckedChanged += new System.EventHandler(this.cbAPICodeFromTemp_CheckedChanged);
            // 
            // cbEnableAPIMon
            // 
            this.cbEnableAPIMon.AutoSize = true;
            this.cbEnableAPIMon.Location = new System.Drawing.Point(9, 3);
            this.cbEnableAPIMon.Name = "cbEnableAPIMon";
            this.cbEnableAPIMon.Size = new System.Drawing.Size(98, 17);
            this.cbEnableAPIMon.TabIndex = 1;
            this.cbEnableAPIMon.Text = "Enable Feature";
            this.cbEnableAPIMon.UseVisualStyleBackColor = true;
            this.cbEnableAPIMon.CheckedChanged += new System.EventHandler(this.cbEnableAPIMon_CheckedChanged);
            // 
            // tabpgApiMon2
            // 
            this.tabpgApiMon2.Controls.Add(this.grpApi);
            this.tabpgApiMon2.Location = new System.Drawing.Point(4, 22);
            this.tabpgApiMon2.Name = "tabpgApiMon2";
            this.tabpgApiMon2.Size = new System.Drawing.Size(403, 296);
            this.tabpgApiMon2.TabIndex = 4;
            this.tabpgApiMon2.Text = "API Monitor [2]";
            this.tabpgApiMon2.UseVisualStyleBackColor = true;
            // 
            // grpApi
            // 
            this.grpApi.Controls.Add(this.cbAPIAutoAllowDetours);
            this.grpApi.Controls.Add(this.btnModuleRWXAdd);
            this.grpApi.Controls.Add(this.btnModuleRWXRemove);
            this.grpApi.Controls.Add(this.txtModuleRWXAdd);
            this.grpApi.Controls.Add(this.lbModuleRWX);
            this.grpApi.Controls.Add(this.cbAPIEnableExeWhitelist);
            this.grpApi.Controls.Add(this.lblExeWhitelist);
            this.grpApi.Controls.Add(this.lbExeWhitelist);
            this.grpApi.Controls.Add(this.txtExecWlAdd);
            this.grpApi.Controls.Add(this.btnExeWlRemove);
            this.grpApi.Controls.Add(this.lblModuleRWXList);
            this.grpApi.Controls.Add(this.btnExeWlAdd);
            this.grpApi.Location = new System.Drawing.Point(7, 11);
            this.grpApi.Name = "grpApi";
            this.grpApi.Size = new System.Drawing.Size(393, 282);
            this.grpApi.TabIndex = 24;
            this.grpApi.TabStop = false;
            this.grpApi.Text = "Feature Settings";
            // 
            // btnModuleRWXAdd
            // 
            this.btnModuleRWXAdd.Location = new System.Drawing.Point(347, 52);
            this.btnModuleRWXAdd.Name = "btnModuleRWXAdd";
            this.btnModuleRWXAdd.Size = new System.Drawing.Size(36, 23);
            this.btnModuleRWXAdd.TabIndex = 27;
            this.btnModuleRWXAdd.Text = "Add";
            this.btnModuleRWXAdd.UseVisualStyleBackColor = true;
            this.btnModuleRWXAdd.Click += new System.EventHandler(this.btnModuleRWXAdd_Click);
            // 
            // btnModuleRWXRemove
            // 
            this.btnModuleRWXRemove.Location = new System.Drawing.Point(209, 247);
            this.btnModuleRWXRemove.Name = "btnModuleRWXRemove";
            this.btnModuleRWXRemove.Size = new System.Drawing.Size(174, 23);
            this.btnModuleRWXRemove.TabIndex = 26;
            this.btnModuleRWXRemove.Text = "Remove";
            this.btnModuleRWXRemove.UseVisualStyleBackColor = true;
            this.btnModuleRWXRemove.Click += new System.EventHandler(this.btnModuleRWXRemove_Click);
            // 
            // txtModuleRWXAdd
            // 
            this.txtModuleRWXAdd.Location = new System.Drawing.Point(209, 54);
            this.txtModuleRWXAdd.Name = "txtModuleRWXAdd";
            this.txtModuleRWXAdd.Size = new System.Drawing.Size(132, 20);
            this.txtModuleRWXAdd.TabIndex = 25;
            // 
            // lbModuleRWX
            // 
            this.lbModuleRWX.FormattingEnabled = true;
            this.lbModuleRWX.Location = new System.Drawing.Point(209, 86);
            this.lbModuleRWX.Name = "lbModuleRWX";
            this.lbModuleRWX.Size = new System.Drawing.Size(174, 147);
            this.lbModuleRWX.TabIndex = 24;
            // 
            // cbAPIEnableExeWhitelist
            // 
            this.cbAPIEnableExeWhitelist.AutoSize = true;
            this.cbAPIEnableExeWhitelist.Location = new System.Drawing.Point(9, 19);
            this.cbAPIEnableExeWhitelist.Name = "cbAPIEnableExeWhitelist";
            this.cbAPIEnableExeWhitelist.Size = new System.Drawing.Size(122, 17);
            this.cbAPIEnableExeWhitelist.TabIndex = 14;
            this.cbAPIEnableExeWhitelist.Text = "Executable Whitelist";
            this.cbAPIEnableExeWhitelist.UseVisualStyleBackColor = true;
            this.cbAPIEnableExeWhitelist.CheckedChanged += new System.EventHandler(this.cbAPIEnableExeWhitelist_CheckedChanged);
            // 
            // lblExeWhitelist
            // 
            this.lblExeWhitelist.AutoSize = true;
            this.lblExeWhitelist.Location = new System.Drawing.Point(6, 38);
            this.lblExeWhitelist.Name = "lblExeWhitelist";
            this.lblExeWhitelist.Size = new System.Drawing.Size(184, 13);
            this.lblExeWhitelist.TabIndex = 13;
            this.lblExeWhitelist.Text = "Executable Whitelist (Process Name):";
            // 
            // lbExeWhitelist
            // 
            this.lbExeWhitelist.FormattingEnabled = true;
            this.lbExeWhitelist.Location = new System.Drawing.Point(7, 86);
            this.lbExeWhitelist.Name = "lbExeWhitelist";
            this.lbExeWhitelist.Size = new System.Drawing.Size(174, 147);
            this.lbExeWhitelist.TabIndex = 15;
            // 
            // txtExecWlAdd
            // 
            this.txtExecWlAdd.Location = new System.Drawing.Point(9, 54);
            this.txtExecWlAdd.Name = "txtExecWlAdd";
            this.txtExecWlAdd.Size = new System.Drawing.Size(130, 20);
            this.txtExecWlAdd.TabIndex = 17;
            // 
            // btnExeWlRemove
            // 
            this.btnExeWlRemove.Location = new System.Drawing.Point(5, 247);
            this.btnExeWlRemove.Name = "btnExeWlRemove";
            this.btnExeWlRemove.Size = new System.Drawing.Size(176, 23);
            this.btnExeWlRemove.TabIndex = 16;
            this.btnExeWlRemove.Text = "Remove";
            this.btnExeWlRemove.UseVisualStyleBackColor = true;
            this.btnExeWlRemove.Click += new System.EventHandler(this.btnExeWlRemove_Click);
            // 
            // lblModuleRWXList
            // 
            this.lblModuleRWXList.AutoSize = true;
            this.lblModuleRWXList.Location = new System.Drawing.Point(206, 38);
            this.lblModuleRWXList.Name = "lblModuleRWXList";
            this.lblModuleRWXList.Size = new System.Drawing.Size(177, 13);
            this.lblModuleRWXList.TabIndex = 19;
            this.lblModuleRWXList.Text = "Module RWX Whitelist (DLL Name):";
            // 
            // btnExeWlAdd
            // 
            this.btnExeWlAdd.Location = new System.Drawing.Point(145, 54);
            this.btnExeWlAdd.Name = "btnExeWlAdd";
            this.btnExeWlAdd.Size = new System.Drawing.Size(36, 23);
            this.btnExeWlAdd.TabIndex = 18;
            this.btnExeWlAdd.Text = "Add";
            this.btnExeWlAdd.UseVisualStyleBackColor = true;
            this.btnExeWlAdd.Click += new System.EventHandler(this.btnExeWlAdd_Click);
            // 
            // tabpgComMon
            // 
            this.tabpgComMon.Controls.Add(this.grpComFeatures);
            this.tabpgComMon.Controls.Add(this.cbEnableComMon);
            this.tabpgComMon.Location = new System.Drawing.Point(4, 22);
            this.tabpgComMon.Name = "tabpgComMon";
            this.tabpgComMon.Size = new System.Drawing.Size(403, 296);
            this.tabpgComMon.TabIndex = 3;
            this.tabpgComMon.Text = "COM/ActiveX Monitor";
            this.tabpgComMon.UseVisualStyleBackColor = true;
            // 
            // grpComFeatures
            // 
            this.grpComFeatures.Controls.Add(this.btnAutoPopBlacklist);
            this.grpComFeatures.Controls.Add(this.tabComWhiteBlackList);
            this.grpComFeatures.Controls.Add(this.cbComEnableBlacklist);
            this.grpComFeatures.Controls.Add(this.cbComEnableWhitelist);
            this.grpComFeatures.Location = new System.Drawing.Point(9, 20);
            this.grpComFeatures.Name = "grpComFeatures";
            this.grpComFeatures.Size = new System.Drawing.Size(391, 273);
            this.grpComFeatures.TabIndex = 2;
            this.grpComFeatures.TabStop = false;
            this.grpComFeatures.Text = "Feature Settings";
            // 
            // btnAutoPopBlacklist
            // 
            this.btnAutoPopBlacklist.Location = new System.Drawing.Point(258, 12);
            this.btnAutoPopBlacklist.Name = "btnAutoPopBlacklist";
            this.btnAutoPopBlacklist.Size = new System.Drawing.Size(127, 23);
            this.btnAutoPopBlacklist.TabIndex = 3;
            this.btnAutoPopBlacklist.Text = "Auto Populate Blacklist";
            this.btnAutoPopBlacklist.UseVisualStyleBackColor = true;
            this.btnAutoPopBlacklist.Click += new System.EventHandler(this.btnAutoPopBlacklist_Click);
            // 
            // tabComWhiteBlackList
            // 
            this.tabComWhiteBlackList.Controls.Add(this.tabPage5);
            this.tabComWhiteBlackList.Controls.Add(this.tabPage6);
            this.tabComWhiteBlackList.Controls.Add(this.tabPage7);
            this.tabComWhiteBlackList.Location = new System.Drawing.Point(9, 39);
            this.tabComWhiteBlackList.Name = "tabComWhiteBlackList";
            this.tabComWhiteBlackList.SelectedIndex = 0;
            this.tabComWhiteBlackList.Size = new System.Drawing.Size(376, 228);
            this.tabComWhiteBlackList.TabIndex = 2;
            // 
            // tabPage5
            // 
            this.tabPage5.Controls.Add(this.btnComWhitelistClsidProgIdRemove);
            this.tabPage5.Controls.Add(this.btnComWhitelistClsidProgIdAdd);
            this.tabPage5.Controls.Add(this.lblComWhitelistClsidProgId);
            this.tabPage5.Controls.Add(this.txtComWhitelistClsidProgId);
            this.tabPage5.Controls.Add(this.lbComWhitelistClsidProgId);
            this.tabPage5.Location = new System.Drawing.Point(4, 22);
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage5.Size = new System.Drawing.Size(368, 202);
            this.tabPage5.TabIndex = 0;
            this.tabPage5.Text = "CLSID Whitelist";
            this.tabPage5.UseVisualStyleBackColor = true;
            // 
            // btnComWhitelistClsidProgIdRemove
            // 
            this.btnComWhitelistClsidProgIdRemove.Location = new System.Drawing.Point(6, 174);
            this.btnComWhitelistClsidProgIdRemove.Name = "btnComWhitelistClsidProgIdRemove";
            this.btnComWhitelistClsidProgIdRemove.Size = new System.Drawing.Size(356, 22);
            this.btnComWhitelistClsidProgIdRemove.TabIndex = 8;
            this.btnComWhitelistClsidProgIdRemove.Text = "Remove";
            this.btnComWhitelistClsidProgIdRemove.UseVisualStyleBackColor = true;
            this.btnComWhitelistClsidProgIdRemove.Click += new System.EventHandler(this.btnComWhitelistClsidProgIdRemove_Click);
            // 
            // btnComWhitelistClsidProgIdAdd
            // 
            this.btnComWhitelistClsidProgIdAdd.Location = new System.Drawing.Point(326, 20);
            this.btnComWhitelistClsidProgIdAdd.Name = "btnComWhitelistClsidProgIdAdd";
            this.btnComWhitelistClsidProgIdAdd.Size = new System.Drawing.Size(36, 23);
            this.btnComWhitelistClsidProgIdAdd.TabIndex = 7;
            this.btnComWhitelistClsidProgIdAdd.Text = "Add";
            this.btnComWhitelistClsidProgIdAdd.UseVisualStyleBackColor = true;
            this.btnComWhitelistClsidProgIdAdd.Click += new System.EventHandler(this.btnComWhitelistClsidProgIdAdd_Click);
            // 
            // lblComWhitelistClsidProgId
            // 
            this.lblComWhitelistClsidProgId.AutoSize = true;
            this.lblComWhitelistClsidProgId.Location = new System.Drawing.Point(9, 4);
            this.lblComWhitelistClsidProgId.Name = "lblComWhitelistClsidProgId";
            this.lblComWhitelistClsidProgId.Size = new System.Drawing.Size(41, 13);
            this.lblComWhitelistClsidProgId.TabIndex = 6;
            this.lblComWhitelistClsidProgId.Text = "CLSID:";
            // 
            // txtComWhitelistClsidProgId
            // 
            this.txtComWhitelistClsidProgId.Location = new System.Drawing.Point(6, 20);
            this.txtComWhitelistClsidProgId.Name = "txtComWhitelistClsidProgId";
            this.txtComWhitelistClsidProgId.Size = new System.Drawing.Size(314, 20);
            this.txtComWhitelistClsidProgId.TabIndex = 5;
            // 
            // lbComWhitelistClsidProgId
            // 
            this.lbComWhitelistClsidProgId.FormattingEnabled = true;
            this.lbComWhitelistClsidProgId.Location = new System.Drawing.Point(6, 45);
            this.lbComWhitelistClsidProgId.Name = "lbComWhitelistClsidProgId";
            this.lbComWhitelistClsidProgId.Size = new System.Drawing.Size(356, 121);
            this.lbComWhitelistClsidProgId.TabIndex = 4;
            // 
            // tabPage6
            // 
            this.tabPage6.Controls.Add(this.btnComBlacklistClsidProgIdRemove);
            this.tabPage6.Controls.Add(this.btnComBlacklistClsidProgIdAdd);
            this.tabPage6.Controls.Add(this.lblComBlacklistClsidProgId);
            this.tabPage6.Controls.Add(this.txtComBlacklistClsidProgId);
            this.tabPage6.Controls.Add(this.lbComBlacklistClsidProgId);
            this.tabPage6.Location = new System.Drawing.Point(4, 22);
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage6.Size = new System.Drawing.Size(368, 202);
            this.tabPage6.TabIndex = 1;
            this.tabPage6.Text = "CLSID Blacklist";
            this.tabPage6.UseVisualStyleBackColor = true;
            // 
            // btnComBlacklistClsidProgIdRemove
            // 
            this.btnComBlacklistClsidProgIdRemove.Location = new System.Drawing.Point(6, 174);
            this.btnComBlacklistClsidProgIdRemove.Name = "btnComBlacklistClsidProgIdRemove";
            this.btnComBlacklistClsidProgIdRemove.Size = new System.Drawing.Size(356, 22);
            this.btnComBlacklistClsidProgIdRemove.TabIndex = 9;
            this.btnComBlacklistClsidProgIdRemove.Text = "Remove";
            this.btnComBlacklistClsidProgIdRemove.UseVisualStyleBackColor = true;
            this.btnComBlacklistClsidProgIdRemove.Click += new System.EventHandler(this.btnComBlacklistClsidProgIdRemove_Click);
            // 
            // btnComBlacklistClsidProgIdAdd
            // 
            this.btnComBlacklistClsidProgIdAdd.Location = new System.Drawing.Point(326, 20);
            this.btnComBlacklistClsidProgIdAdd.Name = "btnComBlacklistClsidProgIdAdd";
            this.btnComBlacklistClsidProgIdAdd.Size = new System.Drawing.Size(36, 23);
            this.btnComBlacklistClsidProgIdAdd.TabIndex = 7;
            this.btnComBlacklistClsidProgIdAdd.Text = "Add";
            this.btnComBlacklistClsidProgIdAdd.UseVisualStyleBackColor = true;
            this.btnComBlacklistClsidProgIdAdd.Click += new System.EventHandler(this.btnComBlacklistClsidProgIdAdd_Click);
            // 
            // lblComBlacklistClsidProgId
            // 
            this.lblComBlacklistClsidProgId.AutoSize = true;
            this.lblComBlacklistClsidProgId.Location = new System.Drawing.Point(9, 4);
            this.lblComBlacklistClsidProgId.Name = "lblComBlacklistClsidProgId";
            this.lblComBlacklistClsidProgId.Size = new System.Drawing.Size(41, 13);
            this.lblComBlacklistClsidProgId.TabIndex = 6;
            this.lblComBlacklistClsidProgId.Text = "CLSID:";
            // 
            // txtComBlacklistClsidProgId
            // 
            this.txtComBlacklistClsidProgId.Location = new System.Drawing.Point(6, 20);
            this.txtComBlacklistClsidProgId.Name = "txtComBlacklistClsidProgId";
            this.txtComBlacklistClsidProgId.Size = new System.Drawing.Size(314, 20);
            this.txtComBlacklistClsidProgId.TabIndex = 5;
            // 
            // lbComBlacklistClsidProgId
            // 
            this.lbComBlacklistClsidProgId.FormattingEnabled = true;
            this.lbComBlacklistClsidProgId.Location = new System.Drawing.Point(6, 45);
            this.lbComBlacklistClsidProgId.Name = "lbComBlacklistClsidProgId";
            this.lbComBlacklistClsidProgId.Size = new System.Drawing.Size(356, 121);
            this.lbComBlacklistClsidProgId.TabIndex = 4;
            // 
            // tabPage7
            // 
            this.tabPage7.Controls.Add(this.cbComJITPersistDecision);
            this.tabPage7.Controls.Add(this.btnComJITlistClsidProgIdRemove);
            this.tabPage7.Controls.Add(this.btnComJITlistClsidProgIdAdd);
            this.tabPage7.Controls.Add(this.lblComJITlistClsidProgId);
            this.tabPage7.Controls.Add(this.txtComJITlistClsidProgId);
            this.tabPage7.Controls.Add(this.lbComJITlistClsidProgId);
            this.tabPage7.Location = new System.Drawing.Point(4, 22);
            this.tabPage7.Name = "tabPage7";
            this.tabPage7.Size = new System.Drawing.Size(368, 202);
            this.tabPage7.TabIndex = 2;
            this.tabPage7.Text = "CLSID Requiring JIT";
            this.tabPage7.UseVisualStyleBackColor = true;
            // 
            // cbComJITPersistDecision
            // 
            this.cbComJITPersistDecision.AutoSize = true;
            this.cbComJITPersistDecision.Location = new System.Drawing.Point(245, 176);
            this.cbComJITPersistDecision.Name = "cbComJITPersistDecision";
            this.cbComJITPersistDecision.Size = new System.Drawing.Size(119, 17);
            this.cbComJITPersistDecision.TabIndex = 10;
            this.cbComJITPersistDecision.Text = "Persist JIT Decision";
            this.cbComJITPersistDecision.UseVisualStyleBackColor = true;
            this.cbComJITPersistDecision.CheckedChanged += new System.EventHandler(this.cbComJITPersistDecision_CheckedChanged);
            // 
            // btnComJITlistClsidProgIdRemove
            // 
            this.btnComJITlistClsidProgIdRemove.Location = new System.Drawing.Point(6, 172);
            this.btnComJITlistClsidProgIdRemove.Name = "btnComJITlistClsidProgIdRemove";
            this.btnComJITlistClsidProgIdRemove.Size = new System.Drawing.Size(233, 22);
            this.btnComJITlistClsidProgIdRemove.TabIndex = 9;
            this.btnComJITlistClsidProgIdRemove.Text = "Remove";
            this.btnComJITlistClsidProgIdRemove.UseVisualStyleBackColor = true;
            this.btnComJITlistClsidProgIdRemove.Click += new System.EventHandler(this.btnComJITlistClsidProgIdRemove_Click);
            // 
            // btnComJITlistClsidProgIdAdd
            // 
            this.btnComJITlistClsidProgIdAdd.Location = new System.Drawing.Point(329, 20);
            this.btnComJITlistClsidProgIdAdd.Name = "btnComJITlistClsidProgIdAdd";
            this.btnComJITlistClsidProgIdAdd.Size = new System.Drawing.Size(36, 23);
            this.btnComJITlistClsidProgIdAdd.TabIndex = 7;
            this.btnComJITlistClsidProgIdAdd.Text = "Add";
            this.btnComJITlistClsidProgIdAdd.UseVisualStyleBackColor = true;
            this.btnComJITlistClsidProgIdAdd.Click += new System.EventHandler(this.btnComJITlistClsidProgIdAdd_Click);
            // 
            // lblComJITlistClsidProgId
            // 
            this.lblComJITlistClsidProgId.AutoSize = true;
            this.lblComJITlistClsidProgId.Location = new System.Drawing.Point(9, 4);
            this.lblComJITlistClsidProgId.Name = "lblComJITlistClsidProgId";
            this.lblComJITlistClsidProgId.Size = new System.Drawing.Size(41, 13);
            this.lblComJITlistClsidProgId.TabIndex = 6;
            this.lblComJITlistClsidProgId.Text = "CLSID:";
            // 
            // txtComJITlistClsidProgId
            // 
            this.txtComJITlistClsidProgId.Location = new System.Drawing.Point(6, 20);
            this.txtComJITlistClsidProgId.Name = "txtComJITlistClsidProgId";
            this.txtComJITlistClsidProgId.Size = new System.Drawing.Size(317, 20);
            this.txtComJITlistClsidProgId.TabIndex = 5;
            // 
            // lbComJITlistClsidProgId
            // 
            this.lbComJITlistClsidProgId.FormattingEnabled = true;
            this.lbComJITlistClsidProgId.Location = new System.Drawing.Point(6, 45);
            this.lbComJITlistClsidProgId.Name = "lbComJITlistClsidProgId";
            this.lbComJITlistClsidProgId.Size = new System.Drawing.Size(359, 121);
            this.lbComJITlistClsidProgId.TabIndex = 4;
            // 
            // cbComEnableBlacklist
            // 
            this.cbComEnableBlacklist.AutoSize = true;
            this.cbComEnableBlacklist.Location = new System.Drawing.Point(114, 16);
            this.cbComEnableBlacklist.Name = "cbComEnableBlacklist";
            this.cbComEnableBlacklist.Size = new System.Drawing.Size(101, 17);
            this.cbComEnableBlacklist.TabIndex = 1;
            this.cbComEnableBlacklist.Text = "Enable Blacklist";
            this.cbComEnableBlacklist.UseVisualStyleBackColor = true;
            this.cbComEnableBlacklist.CheckedChanged += new System.EventHandler(this.cbComEnableBlacklist_CheckedChanged);
            // 
            // cbComEnableWhitelist
            // 
            this.cbComEnableWhitelist.AutoSize = true;
            this.cbComEnableWhitelist.Location = new System.Drawing.Point(6, 16);
            this.cbComEnableWhitelist.Name = "cbComEnableWhitelist";
            this.cbComEnableWhitelist.Size = new System.Drawing.Size(102, 17);
            this.cbComEnableWhitelist.TabIndex = 0;
            this.cbComEnableWhitelist.Text = "Enable Whitelist";
            this.cbComEnableWhitelist.UseVisualStyleBackColor = true;
            this.cbComEnableWhitelist.CheckedChanged += new System.EventHandler(this.cbComEnableWhitelist_CheckedChanged);
            // 
            // cbEnableComMon
            // 
            this.cbEnableComMon.AutoSize = true;
            this.cbEnableComMon.Location = new System.Drawing.Point(9, 3);
            this.cbEnableComMon.Name = "cbEnableComMon";
            this.cbEnableComMon.Size = new System.Drawing.Size(98, 17);
            this.cbEnableComMon.TabIndex = 1;
            this.cbEnableComMon.Text = "Enable Feature";
            this.cbEnableComMon.UseVisualStyleBackColor = true;
            this.cbEnableComMon.CheckedChanged += new System.EventHandler(this.cbEnableComMon_CheckedChanged);
            // 
            // grpInstSettings
            // 
            this.grpInstSettings.Controls.Add(this.btnUninstallAll);
            this.grpInstSettings.Location = new System.Drawing.Point(6, 312);
            this.grpInstSettings.Name = "grpInstSettings";
            this.grpInstSettings.Size = new System.Drawing.Size(212, 49);
            this.grpInstSettings.TabIndex = 2;
            this.grpInstSettings.TabStop = false;
            this.grpInstSettings.Text = "Installation Settings";
            // 
            // btnUninstallAll
            // 
            this.btnUninstallAll.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btnUninstallAll.ForeColor = System.Drawing.Color.Red;
            this.btnUninstallAll.Location = new System.Drawing.Point(12, 19);
            this.btnUninstallAll.Name = "btnUninstallAll";
            this.btnUninstallAll.Size = new System.Drawing.Size(197, 23);
            this.btnUninstallAll.TabIndex = 0;
            this.btnUninstallAll.Text = "Remove Crystal Fully";
            this.btnUninstallAll.UseVisualStyleBackColor = true;
            this.btnUninstallAll.Click += new System.EventHandler(this.btnUninstallAll_Click);
            // 
            // grpMisc
            // 
            this.grpMisc.Controls.Add(this.btnResetConfig);
            this.grpMisc.Location = new System.Drawing.Point(6, 257);
            this.grpMisc.Name = "grpMisc";
            this.grpMisc.Size = new System.Drawing.Size(212, 49);
            this.grpMisc.TabIndex = 3;
            this.grpMisc.TabStop = false;
            this.grpMisc.Text = "Miscellaneous";
            // 
            // btnResetConfig
            // 
            this.btnResetConfig.Location = new System.Drawing.Point(12, 19);
            this.btnResetConfig.Name = "btnResetConfig";
            this.btnResetConfig.Size = new System.Drawing.Size(196, 23);
            this.btnResetConfig.TabIndex = 0;
            this.btnResetConfig.Text = "Undo Configuration Changes";
            this.btnResetConfig.UseVisualStyleBackColor = true;
            this.btnResetConfig.Click += new System.EventHandler(this.btnResetConfig_Click);
            // 
            // cbEnableDiDCore
            // 
            this.cbEnableDiDCore.AutoSize = true;
            this.cbEnableDiDCore.Location = new System.Drawing.Point(297, 367);
            this.cbEnableDiDCore.Name = "cbEnableDiDCore";
            this.cbEnableDiDCore.Size = new System.Drawing.Size(261, 17);
            this.cbEnableDiDCore.TabIndex = 4;
            this.cbEnableDiDCore.Text = "Enable Anti Malicious Code Execution Behaviours";
            this.cbEnableDiDCore.UseVisualStyleBackColor = true;
            this.cbEnableDiDCore.CheckedChanged += new System.EventHandler(this.cbEnableDiDCore_CheckedChanged);
            // 
            // cbEnableBpCore
            // 
            this.cbEnableBpCore.AutoSize = true;
            this.cbEnableBpCore.Location = new System.Drawing.Point(6, 367);
            this.cbEnableBpCore.Name = "cbEnableBpCore";
            this.cbEnableBpCore.Size = new System.Drawing.Size(280, 17);
            this.cbEnableBpCore.TabIndex = 5;
            this.cbEnableBpCore.Text = "Enable Internet Explorer Content Filtering and Analysis";
            this.cbEnableBpCore.UseVisualStyleBackColor = true;
            this.cbEnableBpCore.CheckedChanged += new System.EventHandler(this.cbEnableBpCore_CheckedChanged);
            // 
            // lblWarning
            // 
            this.lblWarning.AutoSize = true;
            this.lblWarning.ForeColor = System.Drawing.Color.Red;
            this.lblWarning.Location = new System.Drawing.Point(9, 0);
            this.lblWarning.Name = "lblWarning";
            this.lblWarning.Size = new System.Drawing.Size(600, 13);
            this.lblWarning.TabIndex = 6;
            this.lblWarning.Text = "Please do not edit these settings unless you understand what you are doing: Prote" +
                "cted processes may fail to operate correctly.";
            // 
            // cbAPIMonAddnAntiROP
            // 
            this.cbAPIMonAddnAntiROP.AutoSize = true;
            this.cbAPIMonAddnAntiROP.Location = new System.Drawing.Point(9, 223);
            this.cbAPIMonAddnAntiROP.Name = "cbAPIMonAddnAntiROP";
            this.cbAPIMonAddnAntiROP.Size = new System.Drawing.Size(163, 17);
            this.cbAPIMonAddnAntiROP.TabIndex = 65;
            this.cbAPIMonAddnAntiROP.Text = "Additional Anti-ROP Methods";
            this.cbAPIMonAddnAntiROP.UseVisualStyleBackColor = true;
            this.cbAPIMonAddnAntiROP.CheckedChanged += new System.EventHandler(this.cbAPIMonAddnAntiROP_CheckedChanged);
            // 
            // cbAPIAutoAllowDetours
            // 
            this.cbAPIAutoAllowDetours.AutoSize = true;
            this.cbAPIAutoAllowDetours.Location = new System.Drawing.Point(211, 19);
            this.cbAPIAutoAllowDetours.Name = "cbAPIAutoAllowDetours";
            this.cbAPIAutoAllowDetours.Size = new System.Drawing.Size(172, 17);
            this.cbAPIAutoAllowDetours.TabIndex = 28;
            this.cbAPIAutoAllowDetours.Text = "Auto-allow RWX Detours DLLs";
            this.cbAPIAutoAllowDetours.UseVisualStyleBackColor = true;
            this.cbAPIAutoAllowDetours.CheckedChanged += new System.EventHandler(this.cbAPIAutoAllowDetours_CheckedChanged);
            // 
            // ExpertOptions
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(653, 389);
            this.Controls.Add(this.lblWarning);
            this.Controls.Add(this.cbEnableBpCore);
            this.Controls.Add(this.cbEnableDiDCore);
            this.Controls.Add(this.grpMisc);
            this.Controls.Add(this.grpInstSettings);
            this.Controls.Add(this.grpProtMethods);
            this.Controls.Add(this.grpProtProc);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(669, 427);
            this.MinimumSize = new System.Drawing.Size(669, 427);
            this.Name = "ExpertOptions";
            this.Text = "Expert Options";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.OptionsDialog_FormClosing);
            this.Load += new System.EventHandler(this.OptionsDialog_Load);
            this.Shown += new System.EventHandler(this.OptionsDialog_Shown);
            this.VisibleChanged += new System.EventHandler(this.OptionsDialog_VisibleChanged);
            this.grpProtProc.ResumeLayout(false);
            this.grpProtProc.PerformLayout();
            this.grpProtMethods.ResumeLayout(false);
            this.tabFeatures.ResumeLayout(false);
            this.tabpgConnMon.ResumeLayout(false);
            this.tabpgConnMon.PerformLayout();
            this.grpConnMonFeatures.ResumeLayout(false);
            this.grpConnMonFeatures.PerformLayout();
            this.tabConnFilters.ResumeLayout(false);
            this.tabPage8.ResumeLayout(false);
            this.tabPage8.PerformLayout();
            this.tabPage9.ResumeLayout(false);
            this.tabPage9.PerformLayout();
            this.tabPage10.ResumeLayout(false);
            this.tabPage10.PerformLayout();
            this.tabpgMemMon.ResumeLayout(false);
            this.tabpgMemMon.PerformLayout();
            this.grpMemMonFeatures.ResumeLayout(false);
            this.grpMemMonFeatures.PerformLayout();
            this.tabpgApiMon1.ResumeLayout(false);
            this.tabpgApiMon1.PerformLayout();
            this.grpAPIMonFeatures1.ResumeLayout(false);
            this.grpAPIMonFeatures1.PerformLayout();
            this.tabpgApiMon2.ResumeLayout(false);
            this.grpApi.ResumeLayout(false);
            this.grpApi.PerformLayout();
            this.tabpgComMon.ResumeLayout(false);
            this.tabpgComMon.PerformLayout();
            this.grpComFeatures.ResumeLayout(false);
            this.grpComFeatures.PerformLayout();
            this.tabComWhiteBlackList.ResumeLayout(false);
            this.tabPage5.ResumeLayout(false);
            this.tabPage5.PerformLayout();
            this.tabPage6.ResumeLayout(false);
            this.tabPage6.PerformLayout();
            this.tabPage7.ResumeLayout(false);
            this.tabPage7.PerformLayout();
            this.grpInstSettings.ResumeLayout(false);
            this.grpMisc.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox grpProtProc;
        private System.Windows.Forms.Button btnRemoveProc;
        private System.Windows.Forms.Button btnAddProc;
        private System.Windows.Forms.TextBox txtAddProc;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox grpProtMethods;
        private System.Windows.Forms.TabControl tabFeatures;
        private System.Windows.Forms.TabPage tabpgConnMon;
        private System.Windows.Forms.TabPage tabpgMemMon;
        private System.Windows.Forms.GroupBox grpConnMonFeatures;
        private System.Windows.Forms.CheckBox cbConnMonValIncoming;
        private System.Windows.Forms.CheckBox cbConnMonValOutgoing;
        private System.Windows.Forms.CheckBox cbConnMonValConnect;
        private System.Windows.Forms.CheckBox cbEnableConnMon;
        private System.Windows.Forms.GroupBox grpMemMonFeatures;
        private System.Windows.Forms.CheckBox cbMemEnableProcDEP;
        private System.Windows.Forms.CheckBox cbMemValidateAllocsCrystal;
        private System.Windows.Forms.CheckBox cbMemValidateAllocsWindows;
        private System.Windows.Forms.CheckBox cbMemTermCorrupt;
        private System.Windows.Forms.CheckBox cbMemVaryAllocSizes;
        private System.Windows.Forms.CheckBox cbEnableMemMon;
        private System.Windows.Forms.TabPage tabpgApiMon1;
        private System.Windows.Forms.CheckBox cbEnableAPIMon;
        private System.Windows.Forms.TabPage tabpgComMon;
        private System.Windows.Forms.CheckBox cbEnableComMon;
        private System.Windows.Forms.GroupBox grpInstSettings;
        private System.Windows.Forms.Button btnUninstallAll;
        private System.Windows.Forms.CheckBox cbMemDoubleFree;
        private System.Windows.Forms.CheckBox cbMemUseAfterFree;
        private System.Windows.Forms.GroupBox grpComFeatures;
        private System.Windows.Forms.TabControl tabComWhiteBlackList;
        private System.Windows.Forms.TabPage tabPage5;
        private System.Windows.Forms.Button btnComWhitelistClsidProgIdAdd;
        private System.Windows.Forms.Label lblComWhitelistClsidProgId;
        private System.Windows.Forms.TextBox txtComWhitelistClsidProgId;
        private System.Windows.Forms.ListBox lbComWhitelistClsidProgId;
        private System.Windows.Forms.TabPage tabPage6;
        private System.Windows.Forms.Button btnComBlacklistClsidProgIdAdd;
        private System.Windows.Forms.Label lblComBlacklistClsidProgId;
        private System.Windows.Forms.TextBox txtComBlacklistClsidProgId;
        private System.Windows.Forms.ListBox lbComBlacklistClsidProgId;
        private System.Windows.Forms.TabPage tabPage7;
        private System.Windows.Forms.Button btnComJITlistClsidProgIdAdd;
        private System.Windows.Forms.Label lblComJITlistClsidProgId;
        private System.Windows.Forms.TextBox txtComJITlistClsidProgId;
        private System.Windows.Forms.ListBox lbComJITlistClsidProgId;
        private System.Windows.Forms.CheckBox cbComEnableBlacklist;
        private System.Windows.Forms.CheckBox cbComEnableWhitelist;
        private System.Windows.Forms.TabControl tabConnFilters;
        private System.Windows.Forms.TabPage tabPage8;
        private System.Windows.Forms.ListBox lbEnabledConnFilters;
        private System.Windows.Forms.Label lblEnabledConnFilters;
        private System.Windows.Forms.TabPage tabPage9;
        private System.Windows.Forms.ListBox lbEnabledOutgoingFilters;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.TabPage tabPage10;
        private System.Windows.Forms.ListBox lbEnabledIncomingFilters;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.ListBox lbProtProc;
        private System.Windows.Forms.Button btnComWhitelistClsidProgIdRemove;
        private System.Windows.Forms.Button btnComBlacklistClsidProgIdRemove;
        private System.Windows.Forms.Button btnComJITlistClsidProgIdRemove;
        private System.Windows.Forms.GroupBox grpMisc;
        private System.Windows.Forms.Button btnResetConfig;
        private System.Windows.Forms.Button btnImportFilters;
        private System.Windows.Forms.Button btnAutoPopBlacklist;
        private System.Windows.Forms.CheckBox cbEnableDiDCore;
        private System.Windows.Forms.CheckBox cbEnableBpCore;
        private System.Windows.Forms.CheckBox cbMemMaxSens;
        private System.Windows.Forms.CheckBox cbComJITPersistDecision;
        private System.Windows.Forms.GroupBox grpAPIMonFeatures1;
        private System.Windows.Forms.CheckBox cbAPIQueryUsrExeBlocked;
        private System.Windows.Forms.CheckBox cbAPIMonEmuExcept;
        private System.Windows.Forms.CheckBox cbAPIMonCheckExcept;
        private System.Windows.Forms.CheckBox cbAPIDisableRWXStack;
        private System.Windows.Forms.CheckBox cbAPICodeFromDownloads;
        private System.Windows.Forms.CheckBox cbAPIMonRndAllocBase;
        private System.Windows.Forms.CheckBox cbAPIInjectChild;
        private System.Windows.Forms.CheckBox cbAPIMonCallOrigins;
        private System.Windows.Forms.CheckBox cbAPIDisableRWXVA;
        private System.Windows.Forms.CheckBox cbAPIAntiROPStack;
        private System.Windows.Forms.CheckBox cbAPICodeFromInetTemp;
        private System.Windows.Forms.CheckBox cbAPICodeFromLongPath;
        private System.Windows.Forms.CheckBox cbAPICodeFromNet;
        private System.Windows.Forms.CheckBox cbAPICodeFromTemp;
        private System.Windows.Forms.Button btnProcEditConf;
        private System.Windows.Forms.Label lblWarning;
        private System.Windows.Forms.CheckBox cbAPIAntiDllPlanting;
        private System.Windows.Forms.TabPage tabpgApiMon2;
        private System.Windows.Forms.CheckBox cbMemWipeAllocs;
        private System.Windows.Forms.Label lblModuleRWXList;
        private System.Windows.Forms.Button btnExeWlAdd;
        private System.Windows.Forms.Button btnExeWlRemove;
        private System.Windows.Forms.TextBox txtExecWlAdd;
        private System.Windows.Forms.ListBox lbExeWhitelist;
        private System.Windows.Forms.Label lblExeWhitelist;
        private System.Windows.Forms.CheckBox cbAPIEnableExeWhitelist;
        private System.Windows.Forms.GroupBox grpApi;
        private System.Windows.Forms.Button btnModuleRWXAdd;
        private System.Windows.Forms.Button btnModuleRWXRemove;
        private System.Windows.Forms.TextBox txtModuleRWXAdd;
        private System.Windows.Forms.ListBox lbModuleRWX;
        private System.Windows.Forms.CheckBox cbMemDisableRWXHeap;
        private System.Windows.Forms.CheckBox cbAPISpawnProtProc;
        private System.Windows.Forms.CheckBox cbAPIPromptOnDotNet;
        private System.Windows.Forms.CheckBox cbAPIMonAddnAntiROP;
        private System.Windows.Forms.CheckBox cbAPIAutoAllowDetours;

    }
}