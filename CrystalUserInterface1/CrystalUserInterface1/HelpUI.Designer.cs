// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
namespace CrystalAEPUI
{
    partial class HelpUI
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(HelpUI));
            this.cmbTopic = new System.Windows.Forms.ComboBox();
            this.lblHelp = new System.Windows.Forms.Label();
            this.wbHelp = new System.Windows.Forms.WebBrowser();
            this.SuspendLayout();
            // 
            // cmbTopic
            // 
            this.cmbTopic.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cmbTopic.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.cmbTopic.FormattingEnabled = true;
            this.cmbTopic.Location = new System.Drawing.Point(136, 2);
            this.cmbTopic.Name = "cmbTopic";
            this.cmbTopic.Size = new System.Drawing.Size(535, 23);
            this.cmbTopic.TabIndex = 0;
            this.cmbTopic.SelectedIndexChanged += new System.EventHandler(this.cmbTopic_SelectedIndexChanged);
            // 
            // lblHelp
            // 
            this.lblHelp.AutoSize = true;
            this.lblHelp.Font = new System.Drawing.Font("Calibri", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblHelp.Location = new System.Drawing.Point(18, 4);
            this.lblHelp.Name = "lblHelp";
            this.lblHelp.Size = new System.Drawing.Size(67, 15);
            this.lblHelp.TabIndex = 1;
            this.lblHelp.Text = "Help Topic:";
            // 
            // wbHelp
            // 
            this.wbHelp.AllowWebBrowserDrop = false;
            this.wbHelp.IsWebBrowserContextMenuEnabled = false;
            this.wbHelp.Location = new System.Drawing.Point(9, 31);
            this.wbHelp.MaximumSize = new System.Drawing.Size(662, 322);
            this.wbHelp.MinimumSize = new System.Drawing.Size(662, 322);
            this.wbHelp.Name = "wbHelp";
            this.wbHelp.ScriptErrorsSuppressed = true;
            this.wbHelp.Size = new System.Drawing.Size(662, 322);
            this.wbHelp.TabIndex = 2;
            this.wbHelp.WebBrowserShortcutsEnabled = false;
            // 
            // HelpUI
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(680, 362);
            this.Controls.Add(this.wbHelp);
            this.Controls.Add(this.lblHelp);
            this.Controls.Add(this.cmbTopic);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(696, 400);
            this.MinimumSize = new System.Drawing.Size(696, 400);
            this.Name = "HelpUI";
            this.Text = "HelpUI";
            this.Load += new System.EventHandler(this.HelpUI_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cmbTopic;
        private System.Windows.Forms.Label lblHelp;
        private System.Windows.Forms.WebBrowser wbHelp;
    }
}