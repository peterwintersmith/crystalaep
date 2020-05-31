// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
namespace CrystalAEPUI
{
    partial class Filters
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Filters));
            this.tabFilters = new System.Windows.Forms.TabControl();
            this.tabpgConnectFilters = new System.Windows.Forms.TabPage();
            this.btnConnEnableDisable = new System.Windows.Forms.Button();
            this.lbConnDisabledFilters = new System.Windows.Forms.ListBox();
            this.lblConnDisabledFilters = new System.Windows.Forms.Label();
            this.lbConnEnabledFilters = new System.Windows.Forms.ListBox();
            this.lblConnEnabledFilters = new System.Windows.Forms.Label();
            this.tabpgOutgoingFilters = new System.Windows.Forms.TabPage();
            this.btnOutEnableDisable = new System.Windows.Forms.Button();
            this.lbOutDisabledFilters = new System.Windows.Forms.ListBox();
            this.lblOutDisabledFilters = new System.Windows.Forms.Label();
            this.lbOutEnabledFilters = new System.Windows.Forms.ListBox();
            this.lblOutEnabledFilters = new System.Windows.Forms.Label();
            this.tabpgIncomingFilters = new System.Windows.Forms.TabPage();
            this.btnInEnableDisable = new System.Windows.Forms.Button();
            this.lbInDisabledFilters = new System.Windows.Forms.ListBox();
            this.lblInDisabledFilters = new System.Windows.Forms.Label();
            this.lbInEnabledFilters = new System.Windows.Forms.ListBox();
            this.lblInEnabledFilters = new System.Windows.Forms.Label();
            this.lblFilterSettings = new System.Windows.Forms.Label();
            this.btnImport = new System.Windows.Forms.Button();
            this.opendlgFilters = new System.Windows.Forms.OpenFileDialog();
            this.tabFilters.SuspendLayout();
            this.tabpgConnectFilters.SuspendLayout();
            this.tabpgOutgoingFilters.SuspendLayout();
            this.tabpgIncomingFilters.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabFilters
            // 
            this.tabFilters.Controls.Add(this.tabpgConnectFilters);
            this.tabFilters.Controls.Add(this.tabpgOutgoingFilters);
            this.tabFilters.Controls.Add(this.tabpgIncomingFilters);
            this.tabFilters.Location = new System.Drawing.Point(5, 18);
            this.tabFilters.Name = "tabFilters";
            this.tabFilters.SelectedIndex = 0;
            this.tabFilters.Size = new System.Drawing.Size(456, 267);
            this.tabFilters.TabIndex = 0;
            // 
            // tabpgConnectFilters
            // 
            this.tabpgConnectFilters.Controls.Add(this.btnConnEnableDisable);
            this.tabpgConnectFilters.Controls.Add(this.lbConnDisabledFilters);
            this.tabpgConnectFilters.Controls.Add(this.lblConnDisabledFilters);
            this.tabpgConnectFilters.Controls.Add(this.lbConnEnabledFilters);
            this.tabpgConnectFilters.Controls.Add(this.lblConnEnabledFilters);
            this.tabpgConnectFilters.Location = new System.Drawing.Point(4, 22);
            this.tabpgConnectFilters.Name = "tabpgConnectFilters";
            this.tabpgConnectFilters.Padding = new System.Windows.Forms.Padding(3);
            this.tabpgConnectFilters.Size = new System.Drawing.Size(448, 241);
            this.tabpgConnectFilters.TabIndex = 0;
            this.tabpgConnectFilters.Text = "Connect Filters";
            this.tabpgConnectFilters.UseVisualStyleBackColor = true;
            // 
            // btnConnEnableDisable
            // 
            this.btnConnEnableDisable.Location = new System.Drawing.Point(386, 119);
            this.btnConnEnableDisable.Name = "btnConnEnableDisable";
            this.btnConnEnableDisable.Size = new System.Drawing.Size(60, 23);
            this.btnConnEnableDisable.TabIndex = 14;
            this.btnConnEnableDisable.Text = "Disable";
            this.btnConnEnableDisable.UseVisualStyleBackColor = true;
            this.btnConnEnableDisable.Click += new System.EventHandler(this.btnConnEnableDisable_Click);
            // 
            // lbConnDisabledFilters
            // 
            this.lbConnDisabledFilters.FormattingEnabled = true;
            this.lbConnDisabledFilters.Location = new System.Drawing.Point(10, 144);
            this.lbConnDisabledFilters.Name = "lbConnDisabledFilters";
            this.lbConnDisabledFilters.Size = new System.Drawing.Size(436, 95);
            this.lbConnDisabledFilters.TabIndex = 13;
            this.lbConnDisabledFilters.Click += new System.EventHandler(this.lbConnDisabledFilters_Click);
            // 
            // lblConnDisabledFilters
            // 
            this.lblConnDisabledFilters.AutoSize = true;
            this.lblConnDisabledFilters.Location = new System.Drawing.Point(7, 125);
            this.lblConnDisabledFilters.Name = "lblConnDisabledFilters";
            this.lblConnDisabledFilters.Size = new System.Drawing.Size(81, 13);
            this.lblConnDisabledFilters.TabIndex = 12;
            this.lblConnDisabledFilters.Text = "Disabled Filters:";
            // 
            // lbConnEnabledFilters
            // 
            this.lbConnEnabledFilters.FormattingEnabled = true;
            this.lbConnEnabledFilters.Location = new System.Drawing.Point(10, 22);
            this.lbConnEnabledFilters.Name = "lbConnEnabledFilters";
            this.lbConnEnabledFilters.Size = new System.Drawing.Size(436, 95);
            this.lbConnEnabledFilters.TabIndex = 11;
            this.lbConnEnabledFilters.Click += new System.EventHandler(this.lbConnEnabledFilters_Click);
            // 
            // lblConnEnabledFilters
            // 
            this.lblConnEnabledFilters.AutoSize = true;
            this.lblConnEnabledFilters.Location = new System.Drawing.Point(7, 6);
            this.lblConnEnabledFilters.Name = "lblConnEnabledFilters";
            this.lblConnEnabledFilters.Size = new System.Drawing.Size(79, 13);
            this.lblConnEnabledFilters.TabIndex = 10;
            this.lblConnEnabledFilters.Text = "Enabled Filters:";
            // 
            // tabpgOutgoingFilters
            // 
            this.tabpgOutgoingFilters.Controls.Add(this.btnOutEnableDisable);
            this.tabpgOutgoingFilters.Controls.Add(this.lbOutDisabledFilters);
            this.tabpgOutgoingFilters.Controls.Add(this.lblOutDisabledFilters);
            this.tabpgOutgoingFilters.Controls.Add(this.lbOutEnabledFilters);
            this.tabpgOutgoingFilters.Controls.Add(this.lblOutEnabledFilters);
            this.tabpgOutgoingFilters.Location = new System.Drawing.Point(4, 22);
            this.tabpgOutgoingFilters.Name = "tabpgOutgoingFilters";
            this.tabpgOutgoingFilters.Padding = new System.Windows.Forms.Padding(3);
            this.tabpgOutgoingFilters.Size = new System.Drawing.Size(448, 241);
            this.tabpgOutgoingFilters.TabIndex = 1;
            this.tabpgOutgoingFilters.Text = "Outgoing Data Filters";
            this.tabpgOutgoingFilters.UseVisualStyleBackColor = true;
            // 
            // btnOutEnableDisable
            // 
            this.btnOutEnableDisable.Location = new System.Drawing.Point(386, 119);
            this.btnOutEnableDisable.Name = "btnOutEnableDisable";
            this.btnOutEnableDisable.Size = new System.Drawing.Size(60, 23);
            this.btnOutEnableDisable.TabIndex = 9;
            this.btnOutEnableDisable.Text = "Disable";
            this.btnOutEnableDisable.UseVisualStyleBackColor = true;
            this.btnOutEnableDisable.Click += new System.EventHandler(this.btnOutEnableDisable_Click);
            // 
            // lbOutDisabledFilters
            // 
            this.lbOutDisabledFilters.FormattingEnabled = true;
            this.lbOutDisabledFilters.Location = new System.Drawing.Point(10, 144);
            this.lbOutDisabledFilters.Name = "lbOutDisabledFilters";
            this.lbOutDisabledFilters.Size = new System.Drawing.Size(436, 95);
            this.lbOutDisabledFilters.TabIndex = 8;
            this.lbOutDisabledFilters.Click += new System.EventHandler(this.lbOutDisabledFilters_Click);
            // 
            // lblOutDisabledFilters
            // 
            this.lblOutDisabledFilters.AutoSize = true;
            this.lblOutDisabledFilters.Location = new System.Drawing.Point(7, 125);
            this.lblOutDisabledFilters.Name = "lblOutDisabledFilters";
            this.lblOutDisabledFilters.Size = new System.Drawing.Size(81, 13);
            this.lblOutDisabledFilters.TabIndex = 7;
            this.lblOutDisabledFilters.Text = "Disabled Filters:";
            // 
            // lbOutEnabledFilters
            // 
            this.lbOutEnabledFilters.FormattingEnabled = true;
            this.lbOutEnabledFilters.Location = new System.Drawing.Point(10, 22);
            this.lbOutEnabledFilters.Name = "lbOutEnabledFilters";
            this.lbOutEnabledFilters.Size = new System.Drawing.Size(436, 95);
            this.lbOutEnabledFilters.TabIndex = 6;
            this.lbOutEnabledFilters.Click += new System.EventHandler(this.lbOutEnabledFilters_Click);
            // 
            // lblOutEnabledFilters
            // 
            this.lblOutEnabledFilters.AutoSize = true;
            this.lblOutEnabledFilters.Location = new System.Drawing.Point(7, 6);
            this.lblOutEnabledFilters.Name = "lblOutEnabledFilters";
            this.lblOutEnabledFilters.Size = new System.Drawing.Size(79, 13);
            this.lblOutEnabledFilters.TabIndex = 5;
            this.lblOutEnabledFilters.Text = "Enabled Filters:";
            // 
            // tabpgIncomingFilters
            // 
            this.tabpgIncomingFilters.Controls.Add(this.btnInEnableDisable);
            this.tabpgIncomingFilters.Controls.Add(this.lbInDisabledFilters);
            this.tabpgIncomingFilters.Controls.Add(this.lblInDisabledFilters);
            this.tabpgIncomingFilters.Controls.Add(this.lbInEnabledFilters);
            this.tabpgIncomingFilters.Controls.Add(this.lblInEnabledFilters);
            this.tabpgIncomingFilters.Location = new System.Drawing.Point(4, 22);
            this.tabpgIncomingFilters.Name = "tabpgIncomingFilters";
            this.tabpgIncomingFilters.Size = new System.Drawing.Size(448, 241);
            this.tabpgIncomingFilters.TabIndex = 2;
            this.tabpgIncomingFilters.Text = "Incoming Data Filters";
            this.tabpgIncomingFilters.UseVisualStyleBackColor = true;
            // 
            // btnInEnableDisable
            // 
            this.btnInEnableDisable.Location = new System.Drawing.Point(386, 119);
            this.btnInEnableDisable.Name = "btnInEnableDisable";
            this.btnInEnableDisable.Size = new System.Drawing.Size(60, 23);
            this.btnInEnableDisable.TabIndex = 9;
            this.btnInEnableDisable.Text = "Disable";
            this.btnInEnableDisable.UseVisualStyleBackColor = true;
            this.btnInEnableDisable.Click += new System.EventHandler(this.btnInEnableDisable_Click);
            // 
            // lbInDisabledFilters
            // 
            this.lbInDisabledFilters.FormattingEnabled = true;
            this.lbInDisabledFilters.Location = new System.Drawing.Point(10, 144);
            this.lbInDisabledFilters.Name = "lbInDisabledFilters";
            this.lbInDisabledFilters.Size = new System.Drawing.Size(436, 95);
            this.lbInDisabledFilters.TabIndex = 8;
            this.lbInDisabledFilters.Click += new System.EventHandler(this.lbInDisabledFilters_Click);
            // 
            // lblInDisabledFilters
            // 
            this.lblInDisabledFilters.AutoSize = true;
            this.lblInDisabledFilters.Location = new System.Drawing.Point(7, 125);
            this.lblInDisabledFilters.Name = "lblInDisabledFilters";
            this.lblInDisabledFilters.Size = new System.Drawing.Size(81, 13);
            this.lblInDisabledFilters.TabIndex = 7;
            this.lblInDisabledFilters.Text = "Disabled Filters:";
            // 
            // lbInEnabledFilters
            // 
            this.lbInEnabledFilters.FormattingEnabled = true;
            this.lbInEnabledFilters.Location = new System.Drawing.Point(10, 22);
            this.lbInEnabledFilters.Name = "lbInEnabledFilters";
            this.lbInEnabledFilters.Size = new System.Drawing.Size(436, 95);
            this.lbInEnabledFilters.TabIndex = 6;
            this.lbInEnabledFilters.Click += new System.EventHandler(this.lbInEnabledFilters_Click);
            // 
            // lblInEnabledFilters
            // 
            this.lblInEnabledFilters.AutoSize = true;
            this.lblInEnabledFilters.Location = new System.Drawing.Point(7, 6);
            this.lblInEnabledFilters.Name = "lblInEnabledFilters";
            this.lblInEnabledFilters.Size = new System.Drawing.Size(79, 13);
            this.lblInEnabledFilters.TabIndex = 5;
            this.lblInEnabledFilters.Text = "Enabled Filters:";
            // 
            // lblFilterSettings
            // 
            this.lblFilterSettings.AutoSize = true;
            this.lblFilterSettings.Location = new System.Drawing.Point(2, 2);
            this.lblFilterSettings.Name = "lblFilterSettings";
            this.lblFilterSettings.Size = new System.Drawing.Size(73, 13);
            this.lblFilterSettings.TabIndex = 1;
            this.lblFilterSettings.Text = "Filter Settings:";
            // 
            // btnImport
            // 
            this.btnImport.Location = new System.Drawing.Point(386, 291);
            this.btnImport.Name = "btnImport";
            this.btnImport.Size = new System.Drawing.Size(75, 23);
            this.btnImport.TabIndex = 2;
            this.btnImport.Text = "Import Filters";
            this.btnImport.UseVisualStyleBackColor = true;
            this.btnImport.Click += new System.EventHandler(this.btnImport_Click);
            // 
            // Filters
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(469, 322);
            this.Controls.Add(this.btnImport);
            this.Controls.Add(this.lblFilterSettings);
            this.Controls.Add(this.tabFilters);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(475, 350);
            this.MinimumSize = new System.Drawing.Size(475, 350);
            this.Name = "Filters";
            this.Text = "Manage Filters";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Filters_FormClosing);
            this.Load += new System.EventHandler(this.Filters_Load);
            this.Shown += new System.EventHandler(this.Filters_Shown);
            this.tabFilters.ResumeLayout(false);
            this.tabpgConnectFilters.ResumeLayout(false);
            this.tabpgConnectFilters.PerformLayout();
            this.tabpgOutgoingFilters.ResumeLayout(false);
            this.tabpgOutgoingFilters.PerformLayout();
            this.tabpgIncomingFilters.ResumeLayout(false);
            this.tabpgIncomingFilters.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TabControl tabFilters;
        private System.Windows.Forms.TabPage tabpgConnectFilters;
        private System.Windows.Forms.TabPage tabpgOutgoingFilters;
        private System.Windows.Forms.TabPage tabpgIncomingFilters;
        private System.Windows.Forms.Label lblFilterSettings;
        private System.Windows.Forms.Button btnConnEnableDisable;
        private System.Windows.Forms.ListBox lbConnDisabledFilters;
        private System.Windows.Forms.Label lblConnDisabledFilters;
        private System.Windows.Forms.ListBox lbConnEnabledFilters;
        private System.Windows.Forms.Label lblConnEnabledFilters;
        private System.Windows.Forms.Button btnOutEnableDisable;
        private System.Windows.Forms.ListBox lbOutDisabledFilters;
        private System.Windows.Forms.Label lblOutDisabledFilters;
        private System.Windows.Forms.ListBox lbOutEnabledFilters;
        private System.Windows.Forms.Label lblOutEnabledFilters;
        private System.Windows.Forms.Button btnInEnableDisable;
        private System.Windows.Forms.ListBox lbInDisabledFilters;
        private System.Windows.Forms.Label lblInDisabledFilters;
        private System.Windows.Forms.ListBox lbInEnabledFilters;
        private System.Windows.Forms.Label lblInEnabledFilters;
        private System.Windows.Forms.Button btnImport;
        private System.Windows.Forms.OpenFileDialog opendlgFilters;
    }
}