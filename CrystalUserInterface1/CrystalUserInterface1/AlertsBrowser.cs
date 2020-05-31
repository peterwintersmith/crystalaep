// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Xml;
using System.Globalization;
using System.Threading;

namespace CrystalAEPUI
{
    public partial class AlertsBrowser : Form
    {
        public AlertsBrowser()
        {
            InitializeComponent();
        }

        private void AlertsBrowser_Load(object sender, EventArgs e)
        {
            try
            {
                cmbRepos.Items.Add("All");

                foreach (string name in EventManager.RepositoryNames)
                {
                    cmbRepos.Items.Add(name);
                }

                cmbRepos.Text = cmbRepos.Items[0].ToString();
                cmbRepos.DropDownStyle = ComboBoxStyle.DropDownList;

                cmbOrderBy.Items.Add("Date");
                cmbOrderBy.Items.Add("Date (Reverse)");
                cmbOrderBy.Items.Add("Risk Rating");
                cmbOrderBy.Items.Add("Risk Rating (Reverse)");
                cmbOrderBy.Items.Add("Process Name");

                cmbOrderBy.Text = cmbOrderBy.Items[0].ToString();
                cmbOrderBy.DropDownStyle = ComboBoxStyle.DropDownList;

                cmbThenBy.Items.Add("None");
                cmbThenBy.Items.Add("Date");
                cmbThenBy.Items.Add("Date (Reverse)");
                cmbThenBy.Items.Add("Risk Rating");
                cmbThenBy.Items.Add("Risk Rating (Reverse)");
                cmbThenBy.Items.Add("Process Name");

                cmbThenBy.Text = cmbThenBy.Items[0].ToString();
                cmbThenBy.DropDownStyle = ComboBoxStyle.DropDownList;

                cmbCount.Items.Add("10");
                cmbCount.Items.Add("20");
                cmbCount.Items.Add("50");
                cmbCount.Items.Add("100");
                cmbCount.Items.Add("500");
                cmbCount.Items.Add("1000");
                cmbCount.Items.Add("All");

                cmbCount.DropDownStyle = ComboBoxStyle.DropDownList;
                cmbCount.Text = cmbCount.Items[4].ToString(); // 500 default

                lvAlerts.Font = new Font("Calibri", 9f, FontStyle.Regular);
                lvAlerts.Scrollable = true;
                lvAlerts.View = View.Details;

                lvAlerts.FullRowSelect = true;

                EventManager.NotifyNewEvent += DisplayNewEvent;

                EventManager.InitialSortCriterion = EEventSortCriteria.SortByDate;
                foreach (EventItem evt in EventManager.AllEvents)
                {
                    DisplayNewEvent(evt.EventRepository.Value, evt, false);
                }

                lvAlerts.Columns[4].Width = lvAlerts.Width - (lvAlerts.Columns[0].Width + lvAlerts.Columns[1].Width + lvAlerts.Columns[2].Width + lvAlerts.Columns[3].Width + 20);

                formInit = true;
            }
            catch (Exception ex)
            {
                if (true)
                {
                    MessageBox.Show(
                            "The form could not be initialised.\n\n" + ex.ToString(),
                            "Alerts Browser",
                            MessageBoxButtons.OK,
                            MessageBoxIcon.Error
                        );

                    this.Close();
                }
            }
        }

        private bool formInit = false;
        private delegate void DisplayNewEventDelegate(EEventRepository r, EventItem e, bool isRealtime);
        private object displaylock = new object();

        private void DisplayNewEvent(EEventRepository r, EventItem e, bool isRealtime)
        {
            try
            {
                if (this.lvAlerts.InvokeRequired)
                {
                    lvAlerts.Invoke(new DisplayNewEventDelegate(DisplayNewEvent), new object[] { r, e, isRealtime });
                }
                else
                {
                    if (String.Compare(cmbRepos.Text, r.ToString(), true) != 0 && String.Compare(cmbRepos.Text, "All", true) != 0)
                    {
                        return;
                    }

                    lock (displaylock)
                    {
                        EThreatRiskRating risk = (EThreatRiskRating)Enum.Parse(typeof(EThreatRiskRating), e.EventRisk, true);
                        Color riskColor = Color.LightGray;

                        switch (risk)
                        {
                            case EThreatRiskRating.Information:
                                riskColor = Color.LightGray;
                                break;
                            case EThreatRiskRating.LowRisk:
                                riskColor = Color.LightGreen;
                                break;
                            case EThreatRiskRating.MediumRisk:
                                riskColor = Color.Orange;
                                break;
                            case EThreatRiskRating.HighRisk:
                                riskColor = Color.OrangeRed;
                                break;
                        }

                        string date = e.EventDate.ToString().Substring(0, e.EventDate.ToString().IndexOf(' '));
                        string time = e.EventDate.ToString().Substring(e.EventDate.ToString().IndexOf(' ') + 1);

                        ListViewItem lvi = new ListViewItem(new string[] { date, time, e.EventProcess, e.EventRisk, e.EventTitle });
                        lvi.BackColor = riskColor;
                        lvi.Tag = e;

                        bool showAllCount = false;
                        int showMax = 0;

                        if (!GetFormItemsCount(out showMax, out showAllCount))
                        {
                            showAllCount = true;
                        }


                        if (!showAllCount && lvAlerts.Items.Count == showMax)
                            lvAlerts.Items.RemoveAt(showMax - 1);

                        lvAlerts.Items.Insert(0, lvi);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }

        private void DblClick(object sender, EventArgs e)
        {
            EventItem evt = (EventItem)lvAlerts.SelectedItems[0].Tag;
            if (evt == null)
                return;

            IndividualAlert ia = new IndividualAlert();
            ia.ShowEvent(evt);
            ia.Show();
        }

        private bool GetFormRepository(out EEventRepository? repos, out bool showAll)
        {
            repos = null;
            showAll = false;

            try
            {
                repos = (EEventRepository)Enum.Parse(typeof(EEventRepository), cmbRepos.Text);
            }
            catch (ArgumentException)
            {
                if (String.Compare(cmbRepos.Text, "All") == 0)
                {
                    showAll = true;
                }
                else
                {
                    return false;
                }
            }
            catch
            {
                return false;
            }

            return true;
        }

        private bool GetFormItemsCount(out int count, out bool showAll)
        {
            count = 0;
            showAll = false;

            try
            {
                if (String.Compare(cmbCount.Text, "All") == 0)
                {
                    showAll = true;
                }
                else
                {
                    count = int.Parse(cmbCount.Text);
                }
            }
            catch
            {
                return false;
            }

            return true;
        }

        private bool GetFormSortCriterion(out EEventSortCriteria? sortCriterion)
        {
            sortCriterion = null;

            try
            {
                switch (cmbOrderBy.Text)
                {
                    case "Date":
                        sortCriterion = EEventSortCriteria.SortByDate;
                        break;
                    case "Date (Reverse)":
                        sortCriterion = EEventSortCriteria.SortByDateReverse;
                        break;
                    case "Risk Rating":
                        sortCriterion = EEventSortCriteria.SortByRisk;
                        break;
                    case "Risk Rating (Reverse)":
                        sortCriterion = EEventSortCriteria.SortByRiskReverse;
                        break;
                    case "Process Name":
                        sortCriterion = EEventSortCriteria.SortByProcess;
                        break;
                }

                if (sortCriterion == null)
                    return false;
            }
            catch
            {
                return false;
            }

            return true;
        }

        private void UpdateAlertsDisplay()
        {
            bool showAllRepos = false;
            EEventRepository? repos = null;
            EEventSortCriteria? sortCriterion = null;

            if (!GetFormRepository(out repos, out showAllRepos) ||
                !GetFormSortCriterion(out sortCriterion))
            {
                lvAlerts.Clear();

                DateTime dateNow = DateTime.Now;
                string date = dateNow.ToString().Substring(0, dateNow.ToString().IndexOf(' '));
                string time = dateNow.ToString().Substring(dateNow.ToString().IndexOf(' ') + 1);

                ListViewItem lvi = new ListViewItem(new string[] { date, time, "", "", "Unable to display alerts: Display/order settings invalid." });
                lvAlerts.Items.Insert(0, lvi);

                return;
            }

            lvAlerts.Items.Clear();

            EventManager.InitialSortCriterion = sortCriterion.Value;

            foreach (EventItem evt in EventManager.AllEvents)
            {
                if (evt.EventRepository.HasValue)
                {
                    if (showAllRepos || evt.EventRepository == repos.Value)
                    {
                        DisplayNewEvent(evt.EventRepository.Value, evt, false);
                    }
                }
            }

            lvAlerts.Refresh();
        }

        private void cmbRepos_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!formInit)
                return;

            UpdateAlertsDisplay();
        }

        private void cmbOrderBy_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!formInit)
                return;

            UpdateAlertsDisplay();
        }

        private void cmbCount_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (!formInit)
                return;

            UpdateAlertsDisplay();
        }

        private void AlertsBrowser_FormClosing(object sender, FormClosingEventArgs e)
        {
            Hide();

            if(!MainUI.ShuttingDownProcess)
                e.Cancel = true;
        }

        string HtmlEncode(string input)
        {
            return input.Replace("<", "&lt;").Replace(">", "&gt;").Replace("'", "&#x27;").Replace("\"", "&quot;");
        }

        private string GenerateReportFromTable()
        {
            StringBuilder sbHtml = new StringBuilder(lvAlerts.Items.Count * 300);

            string date = DateTime.Now.ToString().Substring(0, DateTime.Now.ToString().IndexOf(' '));
            string time = DateTime.Now.ToString().Substring(DateTime.Now.ToString().IndexOf(' ') + 1);

            sbHtml.Append("<!-- saved from url=(0014)about:internet -->\r\n");
            sbHtml.AppendFormat("<center><h3>Crystal Browser Security Report [Generated on {0} {1}]</h3></center><br>\r\n", date, time);
            sbHtml.Append("<table border=\"1\"><tr><th width=\"15%\">Event Repository</th><th width=\"10%\">Alert Date</th><th width=\"10%\">Alert Time</th><th width=\"20%\">Process Name</th><th width=\"10%\">Risk Rating</th><th>Alert Title</th></tr>\r\n");

            foreach (ListViewItem lvi in lvAlerts.Items)
            {
                EventItem evt = (EventItem)lvi.Tag;

                string riskColor = null;

                switch ((EThreatRiskRating)Enum.Parse(typeof(EThreatRiskRating), evt.EventRisk))
                {
                    case EThreatRiskRating.Information:
                        riskColor = "grey";
                        break;
                    case EThreatRiskRating.LowRisk:
                        riskColor = "green";
                        break;
                    case EThreatRiskRating.MediumRisk:
                        riskColor = "orange";
                        break;
                    case EThreatRiskRating.HighRisk:
                        riskColor = "red";
                        break;
                }

                if (riskColor == null)
                    continue;

                string template = "<tr bgcolor=\"{0}\"><td>{7}</td><td>{1}</td><td>{2}</td><td>{3}</td><td>{4}</td><td>{5}</td></tr><tr><td colspan=\"6\">{6}</td></tr>\r\n";

                date = evt.EventDate.ToString().Substring(0, evt.EventDate.ToString().IndexOf(' '));
                time = evt.EventDate.ToString().Substring(evt.EventDate.ToString().IndexOf(' ') + 1);

                string htmlEntry = String.Format(template, riskColor, date, time, evt.EventProcess, evt.EventRisk, evt.EventTitle, HtmlEncode(evt.EventMessage), evt.EventRepository.Value.ToString());

                sbHtml.Append(htmlEntry);
            }

            sbHtml.Append("</table>\r\n");
            return sbHtml.ToString();
        }

        private void btnSaveAs_Click(object sender, EventArgs e)
        {
            saveDlg.Filter = "HTML Reports (*.htm)|*.htm";

            DialogResult dlgResult = saveDlg.ShowDialog();

            if (dlgResult == DialogResult.OK && !String.IsNullOrEmpty(saveDlg.FileName))
            {
                string report = GenerateReportFromTable();
                if (report != null)
                {
                    File.WriteAllText(saveDlg.FileName, report);
                }
            }
        }
    }
}
