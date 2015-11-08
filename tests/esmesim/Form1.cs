using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using esmesim.Properties;
using System.Threading;
using System.Diagnostics;
using esmesim.smpp;
using System.Runtime.InteropServices;

namespace esmesim
{
	public partial class Form1 : Form
	{

		private static bool IsRunningOnMono()
		{
			return Type.GetType("Mono.Runtime") != null;
		}

		class MessageGridModel
		{
			public MessageGridModel(string from, string to, string message)
			{
				m_from = from;
				m_to = to;
				m_msg = message;
			}
			public string From { get { return m_from; } }
			public string To { get { return m_to; } }
			public string Message { get { return m_msg; } }
			string m_from, m_to, m_msg;
		}

		const int keepAliveTime = 55;
		private object m_keepAliveLock = new object();

		//private EsmeSession session = null;

		private smpp.SMPPClient m_client = null;

		private BindingList<MessageGridModel> m_inbox = new BindingList<MessageGridModel>();
		private BindingList<MessageGridModel> m_outbox = new BindingList<MessageGridModel>();


		public Form1()
		{
			string[] dataCodingValues = Enum.GetValues(typeof(DataCoding)).OfType<object>().Select(o => o.ToString()).ToArray();

			InitializeComponent();
			cboMode.DataSource = Enum.GetValues(typeof(BindType));
			cboMode.SelectedItem = Settings.Default.BindMode;
			cboDataCoding.DataSource = dataCodingValues;
			cboDataCoding.SelectedItem = dataCodingValues[0];
			cboServerDataCoding.DataSource = dataCodingValues;
			cboServerDataCoding.SelectedItem = dataCodingValues[0];
			chk7bitPacking.Checked = Settings.Default.Gsm7bitPacking;
			txtUser.Text = Settings.Default.User;
			txtPass.Text = Settings.Default.Password;
			txtServer.Text = Settings.Default.ServerAddress;
			txtPort.Text = Settings.Default.ServerPort.ToString();
			txtAddress.Text = Settings.Default.AddressRange;
			txtSystemType.Text = Settings.Default.SystemType;
			txtTo.Text = Settings.Default.LastPhoneNumber;
			gridInbox.AutoGenerateColumns = false;
			gridOutbox.AutoGenerateColumns = false;
			gridInbox.DataSource = m_inbox;
			gridOutbox.DataSource = m_outbox;
			gridInbox.ColumnHeadersBorderStyle = ProperColumnHeadersBorderStyle;
			gridOutbox.ColumnHeadersBorderStyle = ProperColumnHeadersBorderStyle;
			CheckForIllegalCrossThreadCalls = false;
		}

		private DataCoding GetSelectedDataCoding(ComboBox combo)
		{
			return (DataCoding)Enum.Parse(typeof(DataCoding), (string)combo.SelectedItem);
		}

		private void Form1_FormClosing(object sender, FormClosingEventArgs e)
		{
			if (m_client != null)
			{
				if (m_client.Bound)
				{
					m_client.Unbind();
				}
				m_client.Dispose();
				m_client = null;
			}

			ushort port;
			if (ushort.TryParse(txtPort.Text, out port))
			{
				Settings.Default.ServerPort = port;
			}
			Settings.Default.User = txtUser.Text;
			Settings.Default.Password = txtPass.Text;
			Settings.Default.ServerAddress = txtServer.Text;
			Settings.Default.BindMode = (BindType)cboMode.SelectedItem;
			Settings.Default.AddressRange = txtAddress.Text;
			Settings.Default.SystemType = txtSystemType.Text;
			Settings.Default.LastPhoneNumber = txtTo.Text;
			Settings.Default.DataCoding = GetSelectedDataCoding(cboDataCoding);
			Settings.Default.ServerDataCoding = GetSelectedDataCoding(cboServerDataCoding);
			Settings.Default.Gsm7bitPacking = chk7bitPacking.Checked;
			Settings.Default.Save();
		}


		private void btnBind_Click(object sender, EventArgs e)
		{
			if (m_client == null)
			{
				m_client = new esmesim.smpp.SMPPClient();

				m_client.ConnectionLost += new esmesim.smpp.ConnectionLostEventHandler(delegate()
				{
					MessageBox.Show(this, "Connection with the server has been lost!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
					SetInitialStatus(false);
				});

				m_client.NewMessage += new esmesim.smpp.NewMessageEventHandler(delegate(string from, string to, string content)
				{
					m_inbox.Add(new MessageGridModel(from, to, content));
				});
			}

			if (m_client.Bound)
			{
				m_client.Unbind();
			}
			else
			{
				ushort port;
				if (!ushort.TryParse(txtPort.Text, out port))
				{
					MessageBox.Show(this, "Server port is invalid", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
					return;
				}

				m_client.SystemId = txtUser.Text;
				m_client.Password = txtPass.Text;
				m_client.SystemType = txtSystemType.Text;
				m_client.Settings.EnableGSM7bitPacking = chk7bitPacking.Checked;
				m_client.Settings.DeliverDataCoding = GetSelectedDataCoding(cboDataCoding);
				m_client.Settings.ServerDefaultEncoding = GetSelectedDataCoding(cboServerDataCoding);
				m_client.Addresses.Clear();
				foreach (string s in txtAddress.Text.Split(','))
				{
					m_client.Addresses.Add(s);
				}

				try
				{
					m_client.Bind((esmesim.smpp.BindType)cboMode.SelectedItem, txtServer.Text, port);
					this.BringToFront();


					//TODO new Thread(KeepAliveThread
					//TODO session.SetDataCoding((DataCoding)cboDataCoding.SelectedItem, (DataCoding)cboServerDataCoding.SelectedItem, chk7bitPacking.Checked);

				}
				catch (esmesim.smpp.BindException ex)
				{
					MessageBox.Show(this, "Bind error: " + ex.Result, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
			}

			SetInitialStatus(m_client.Bound);
		}

		private void btnSend_Click(object sender, EventArgs e)
		{
			SendMessage();
		}

		bool handleKey = false; // workaround for mono bug
		private void txtMessage_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.Enter && !e.Shift)
			{
				e.Handled = true; // mono: you suck on this!
				// Mono does not stop the event propagation when e.Handled is set to true
				handleKey = true;
				SendMessage();
			}
		}
		private void txtMessage_KeyPress(object sender, KeyPressEventArgs e)
		{
			if (handleKey)
			{
				e.Handled = true;
				handleKey = false;
			}
		}

		private void ShowError(Exception ex)
		{
			MessageBox.Show(this, ex.Message, ex.GetType().Name, MessageBoxButtons.OK, MessageBoxIcon.Error);
		}

		private new void BringToFront()
		{
			this.TopMost = true;
			base.BringToFront();
			this.TopMost = false;
		}

		private void KeepAliveThread()
		{
#if FALSE
			lock (m_keepAliveLock)
			{
				for (; ; )
				{
					try
					{
						for (int timeout = keepAliveTime * 2; timeout > 0 && session != null; timeout--)
						{
							Thread.Sleep(500);
						}
						lock (this)
						{
							if (session == null)
							{
								return;
							}
							session.SendKeepAlive();
						}
					}
					catch (Exception ex)
					{
						Debug.WriteLine(ex);
					}
				}
			}
#endif
		}


		private void SetInitialStatus(bool connected)
		{
			grLogin.Enabled = !connected;
			grMessages.Enabled = connected;
			btnBind.Text = connected ? "Unbind" : "Bind";
			lblStatus.Text = String.Format("Status: {0}Bound", connected ? "" : "Not ");
			lblStatus.ForeColor = connected ? Color.Green : SystemColors.ControlText;
		}

		private void SendMessage()
		{
			if (m_client == null)
			{
				return;
			}

			try
			{
				string from = m_client.Addresses.Count == 0 ? String.Empty : m_client.Addresses.ElementAt(0).Split('-')[0];
				m_client.SendMessage(from, txtTo.Text, txtMessage.Text);
				m_outbox.Add(new MessageGridModel("", txtTo.Text, txtMessage.Text));
			}
			catch (esmesim.smpp.DeliveryException ex)
			{
				MessageBox.Show(this, "Could not send message: " + ex.Result, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		static DataGridViewHeaderBorderStyle ProperColumnHeadersBorderStyle
		{
			get
			{
				return (SystemFonts.MessageBoxFont.Name == "Segoe UI") ?
					DataGridViewHeaderBorderStyle.None : DataGridViewHeaderBorderStyle.Raised;
			}
		}

		private void grid_MouseDoubleClick(object sender, MouseEventArgs e)
		{
			DataGridView grid = sender as DataGridView;
			DataGridView.HitTestInfo hti = grid.HitTest(e.X, e.Y);
			if (hti.RowIndex >= 0 && hti.RowIndex < grid.Rows.Count)
			{
				txtTo.Text = grid.Rows[hti.RowIndex].Cells[0].Value.ToString();
				if (grid == gridInbox)
				{
					txtMessage.Text = grid.Rows[hti.RowIndex].Cells[2].Value.ToString();
				}
				else
				{
					txtMessage.Text = grid.Rows[hti.RowIndex].Cells[1].Value.ToString();
				}
			}
		}

		private void dataCoding_SelectionChanged(object sender, EventArgs e)
		{
		}
	}
}
