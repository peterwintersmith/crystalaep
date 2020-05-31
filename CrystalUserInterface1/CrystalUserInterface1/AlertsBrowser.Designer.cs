// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
namespace CrystalAEPUI
{
    partial class AlertsBrowser
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AlertsBrowser));
            this.cmbRepos = new System.Windows.Forms.ComboBox();
            this.lblRepos = new System.Windows.Forms.Label();
            this.btnSaveAs = new System.Windows.Forms.Button();
            this.lblOrderBy = new System.Windows.Forms.Label();
            this.cmbOrderBy = new System.Windows.Forms.ComboBox();
            this.lblCount = new System.Windows.Forms.Label();
            this.cmbCount = new System.Windows.Forms.ComboBox();
            this.lblThenBy = new System.Windows.Forms.Label();
            this.cmbThenBy = new System.Windows.Forms.ComboBox();
            this.lvAlerts = new System.Windows.Forms.ListView();
            this.colDate = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colTime = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colProc = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colRisk = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.colTitle = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.saveDlg = new System.Windows.Forms.SaveFileDialog();
            this.SuspendLayout();
            // 
            // cmbRepos
            // 
            this.cmbRepos.FormattingEnabled = true;
            this.cmbRepos.Location = new System.Drawing.Point(75, 6);
            this.cmbRepos.Name = "cmbRepos";
            this.cmbRepos.Size = new System.Drawing.Size(107, 21);
            this.cmbRepos.TabIndex = 0;
            this.cmbRepos.SelectedIndexChanged += new System.EventHandler(this.cmbRepos_SelectedIndexChanged);
            // 
            // lblRepos
            // 
            this.lblRepos.AutoSize = true;
            this.lblRepos.Location = new System.Drawing.Point(9, 9);
            this.lblRepos.Name = "lblRepos";
            this.lblRepos.Size = new System.Drawing.Size(60, 13);
            this.lblRepos.TabIndex = 1;
            this.lblRepos.Text = "Repository:";
            // 
            // btnSaveAs
            // 
            this.btnSaveAs.Location = new System.Drawing.Point(529, 317);
            this.btnSaveAs.Name = "btnSaveAs";
            this.btnSaveAs.Size = new System.Drawing.Size(106, 21);
            this.btnSaveAs.TabIndex = 4;
            this.btnSaveAs.Text = "Export Web Report";
            this.btnSaveAs.UseVisualStyleBackColor = true;
            this.btnSaveAs.Click += new System.EventHandler(this.btnSaveAs_Click);
            // 
            // lblOrderBy
            // 
            this.lblOrderBy.AutoSize = true;
            this.lblOrderBy.Location = new System.Drawing.Point(188, 9);
            this.lblOrderBy.Name = "lblOrderBy";
            this.lblOrderBy.Size = new System.Drawing.Size(51, 13);
            this.lblOrderBy.TabIndex = 5;
            this.lblOrderBy.Text = "Order By:";
            // 
            // cmbOrderBy
            // 
            this.cmbOrderBy.FormattingEnabled = true;
            this.cmbOrderBy.Location = new System.Drawing.Point(245, 6);
            this.cmbOrderBy.Name = "cmbOrderBy";
            this.cmbOrderBy.Size = new System.Drawing.Size(85, 21);
            this.cmbOrderBy.TabIndex = 6;
            this.cmbOrderBy.SelectedIndexChanged += new System.EventHandler(this.cmbOrderBy_SelectedIndexChanged);
            // 
            // lblCount
            // 
            this.lblCount.AutoSize = true;
            this.lblCount.Location = new System.Drawing.Point(483, 9);
            this.lblCount.Name = "lblCount";
            this.lblCount.Size = new System.Drawing.Size(87, 13);
            this.lblCount.TabIndex = 7;
            this.lblCount.Text = "Number of Items:";
            // 
            // cmbCount
            // 
            this.cmbCount.FormattingEnabled = true;
            this.cmbCount.Location = new System.Drawing.Point(576, 6);
            this.cmbCount.Name = "cmbCount";
            this.cmbCount.Size = new System.Drawing.Size(59, 21);
            this.cmbCount.TabIndex = 8;
            this.cmbCount.SelectedIndexChanged += new System.EventHandler(this.cmbCount_SelectedIndexChanged);
            // 
            // lblThenBy
            // 
            this.lblThenBy.AutoSize = true;
            this.lblThenBy.Location = new System.Drawing.Point(336, 9);
            this.lblThenBy.Name = "lblThenBy";
            this.lblThenBy.Size = new System.Drawing.Size(50, 13);
            this.lblThenBy.TabIndex = 9;
            this.lblThenBy.Text = "Then By:";
            this.lblThenBy.Visible = false;
            // 
            // cmbThenBy
            // 
            this.cmbThenBy.FormattingEnabled = true;
            this.cmbThenBy.Location = new System.Drawing.Point(392, 6);
            this.cmbThenBy.Name = "cmbThenBy";
            this.cmbThenBy.Size = new System.Drawing.Size(85, 21);
            this.cmbThenBy.TabIndex = 10;
            this.cmbThenBy.Visible = false;
            // 
            // lvAlerts
            // 
            this.lvAlerts.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.colDate,
            this.colTime,
            this.colProc,
            this.colRisk,
            this.colTitle});
            this.lvAlerts.Location = new System.Drawing.Point(12, 33);
            this.lvAlerts.Name = "lvAlerts";
            this.lvAlerts.Size = new System.Drawing.Size(623, 278);
            this.lvAlerts.TabIndex = 11;
            this.lvAlerts.UseCompatibleStateImageBehavior = false;
            this.lvAlerts.DoubleClick += new System.EventHandler(this.DblClick);
            // 
            // colDate
            // 
            this.colDate.Text = "Alert Date";
            this.colDate.Width = 80;
            // 
            // colTime
            // 
            this.colTime.Text = "Alert Time";
            this.colTime.Width = 80;
            // 
            // colProc
            // 
            this.colProc.Text = "Process Name";
            this.colProc.Width = 90;
            // 
            // colRisk
            // 
            this.colRisk.Text = "Risk Rating";
            this.colRisk.Width = 80;
            // 
            // colTitle
            // 
            this.colTitle.Text = "Alert Title";
            // 
            // AlertsBrowser
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(647, 347);
            this.Controls.Add(this.lvAlerts);
            this.Controls.Add(this.cmbThenBy);
            this.Controls.Add(this.lblThenBy);
            this.Controls.Add(this.cmbCount);
            this.Controls.Add(this.lblCount);
            this.Controls.Add(this.cmbOrderBy);
            this.Controls.Add(this.lblOrderBy);
            this.Controls.Add(this.btnSaveAs);
            this.Controls.Add(this.lblRepos);
            this.Controls.Add(this.cmbRepos);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(663, 385);
            this.MinimumSize = new System.Drawing.Size(663, 385);
            this.Name = "AlertsBrowser";
            this.Text = "Alerts Browser";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.AlertsBrowser_FormClosing);
            this.Load += new System.EventHandler(this.AlertsBrowser_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cmbRepos;
        private System.Windows.Forms.Label lblRepos;
        private System.Windows.Forms.Button btnSaveAs;
        private System.Windows.Forms.Label lblOrderBy;
        private System.Windows.Forms.ComboBox cmbOrderBy;
        private System.Windows.Forms.Label lblCount;
        private System.Windows.Forms.ComboBox cmbCount;
        private System.Windows.Forms.Label lblThenBy;
        private System.Windows.Forms.ComboBox cmbThenBy;
        private System.Windows.Forms.ListView lvAlerts;
        private System.Windows.Forms.ColumnHeader colDate;
        private System.Windows.Forms.ColumnHeader colTime;
        private System.Windows.Forms.ColumnHeader colProc;
        private System.Windows.Forms.ColumnHeader colRisk;
        private System.Windows.Forms.ColumnHeader colTitle;
        private System.Windows.Forms.SaveFileDialog saveDlg;
    }
}