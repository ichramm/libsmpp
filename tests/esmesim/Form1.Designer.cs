namespace esmesim
{
	partial class Form1
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
			this.grLogin = new System.Windows.Forms.GroupBox();
			this.txtSystemType = new System.Windows.Forms.TextBox();
			this.label7 = new System.Windows.Forms.Label();
			this.label6 = new System.Windows.Forms.Label();
			this.label5 = new System.Windows.Forms.Label();
			this.cboMode = new System.Windows.Forms.ComboBox();
			this.txtAddress = new System.Windows.Forms.TextBox();
			this.txtPort = new System.Windows.Forms.TextBox();
			this.label4 = new System.Windows.Forms.Label();
			this.txtServer = new System.Windows.Forms.TextBox();
			this.txtUser = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.label1 = new System.Windows.Forms.Label();
			this.txtPass = new System.Windows.Forms.TextBox();
			this.btnBind = new System.Windows.Forms.Button();
			this.grMessages = new System.Windows.Forms.GroupBox();
			this.btnSend = new System.Windows.Forms.Button();
			this.txtMessage = new System.Windows.Forms.RichTextBox();
			this.label8 = new System.Windows.Forms.Label();
			this.txtTo = new System.Windows.Forms.TextBox();
			this.lblStatus = new System.Windows.Forms.Label();
			this.splitContainer1 = new System.Windows.Forms.SplitContainer();
			this.chk7bitPacking = new System.Windows.Forms.CheckBox();
			this.label3 = new System.Windows.Forms.Label();
			this.cboDataCoding = new System.Windows.Forms.ComboBox();
			this.groupOutbox = new System.Windows.Forms.GroupBox();
			this.gridOutbox = new System.Windows.Forms.DataGridView();
			this.dataGridViewTextBoxColumn2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			this.dataGridViewTextBoxColumn3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
			this.groupInbox = new System.Windows.Forms.GroupBox();
			this.gridInbox = new System.Windows.Forms.DataGridView();
			this.colFrom = new System.Windows.Forms.DataGridViewTextBoxColumn();
			this.colTo = new System.Windows.Forms.DataGridViewTextBoxColumn();
			this.colMsg = new System.Windows.Forms.DataGridViewTextBoxColumn();
			this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
			this.label9 = new System.Windows.Forms.Label();
			this.cboServerDataCoding = new System.Windows.Forms.ComboBox();
			this.grLogin.SuspendLayout();
			this.grMessages.SuspendLayout();
			this.splitContainer1.Panel1.SuspendLayout();
			this.splitContainer1.Panel2.SuspendLayout();
			this.splitContainer1.SuspendLayout();
			this.groupOutbox.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.gridOutbox)).BeginInit();
			this.groupInbox.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.gridInbox)).BeginInit();
			this.SuspendLayout();
			// 
			// grLogin
			// 
			this.grLogin.Controls.Add(this.txtSystemType);
			this.grLogin.Controls.Add(this.label7);
			this.grLogin.Controls.Add(this.label6);
			this.grLogin.Controls.Add(this.label5);
			this.grLogin.Controls.Add(this.cboMode);
			this.grLogin.Controls.Add(this.txtAddress);
			this.grLogin.Controls.Add(this.txtPort);
			this.grLogin.Controls.Add(this.label4);
			this.grLogin.Controls.Add(this.txtServer);
			this.grLogin.Controls.Add(this.txtUser);
			this.grLogin.Controls.Add(this.label2);
			this.grLogin.Controls.Add(this.label1);
			this.grLogin.Controls.Add(this.txtPass);
			this.grLogin.Location = new System.Drawing.Point(12, 13);
			this.grLogin.Name = "grLogin";
			this.grLogin.Size = new System.Drawing.Size(336, 116);
			this.grLogin.TabIndex = 0;
			this.grLogin.TabStop = false;
			this.grLogin.Text = "Bind / unbind";
			// 
			// txtSystemType
			// 
			this.txtSystemType.Location = new System.Drawing.Point(239, 71);
			this.txtSystemType.Name = "txtSystemType";
			this.txtSystemType.Size = new System.Drawing.Size(88, 20);
			this.txtSystemType.TabIndex = 6;
			this.txtSystemType.Text = "test";
			// 
			// label7
			// 
			this.label7.AutoSize = true;
			this.label7.Location = new System.Drawing.Point(168, 74);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(65, 13);
			this.label7.TabIndex = 10;
			this.label7.Text = "SystemType";
			// 
			// label6
			// 
			this.label6.AutoSize = true;
			this.label6.Location = new System.Drawing.Point(6, 74);
			this.label6.Name = "label6";
			this.label6.Size = new System.Drawing.Size(56, 13);
			this.label6.TabIndex = 7;
			this.label6.Text = "Addresses";
			// 
			// label5
			// 
			this.label5.AutoSize = true;
			this.label5.Location = new System.Drawing.Point(143, 48);
			this.label5.Name = "label5";
			this.label5.Size = new System.Drawing.Size(34, 13);
			this.label5.TabIndex = 7;
			this.label5.Text = "Mode";
			// 
			// cboMode
			// 
			this.cboMode.FormattingEnabled = true;
			this.cboMode.Location = new System.Drawing.Point(183, 45);
			this.cboMode.Name = "cboMode";
			this.cboMode.Size = new System.Drawing.Size(144, 21);
			this.cboMode.TabIndex = 4;
			// 
			// txtAddress
			// 
			this.txtAddress.Location = new System.Drawing.Point(68, 71);
			this.txtAddress.Name = "txtAddress";
			this.txtAddress.Size = new System.Drawing.Size(94, 20);
			this.txtAddress.TabIndex = 5;
			this.txtAddress.Text = "9999";
			// 
			// txtPort
			// 
			this.txtPort.Location = new System.Drawing.Point(279, 19);
			this.txtPort.Name = "txtPort";
			this.txtPort.Size = new System.Drawing.Size(48, 20);
			this.txtPort.TabIndex = 2;
			this.txtPort.Text = "9090";
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(143, 22);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(38, 13);
			this.label4.TabIndex = 7;
			this.label4.Text = "Server";
			// 
			// txtServer
			// 
			this.txtServer.Location = new System.Drawing.Point(183, 19);
			this.txtServer.Name = "txtServer";
			this.txtServer.Size = new System.Drawing.Size(90, 20);
			this.txtServer.TabIndex = 1;
			this.txtServer.Text = "127.0.0.1";
			// 
			// txtUser
			// 
			this.txtUser.Location = new System.Drawing.Point(41, 19);
			this.txtUser.Name = "txtUser";
			this.txtUser.Size = new System.Drawing.Size(91, 20);
			this.txtUser.TabIndex = 0;
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(6, 48);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(30, 13);
			this.label2.TabIndex = 5;
			this.label2.Text = "Pass";
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(6, 22);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(29, 13);
			this.label1.TabIndex = 4;
			this.label1.Text = "User";
			// 
			// txtPass
			// 
			this.txtPass.Location = new System.Drawing.Point(41, 45);
			this.txtPass.Name = "txtPass";
			this.txtPass.Size = new System.Drawing.Size(91, 20);
			this.txtPass.TabIndex = 3;
			this.txtPass.UseSystemPasswordChar = true;
			// 
			// btnBind
			// 
			this.btnBind.Location = new System.Drawing.Point(268, 135);
			this.btnBind.Name = "btnBind";
			this.btnBind.Size = new System.Drawing.Size(75, 23);
			this.btnBind.TabIndex = 7;
			this.btnBind.Text = "Bind";
			this.btnBind.UseVisualStyleBackColor = true;
			this.btnBind.Click += new System.EventHandler(this.btnBind_Click);
			// 
			// grMessages
			// 
			this.grMessages.Controls.Add(this.btnSend);
			this.grMessages.Controls.Add(this.txtMessage);
			this.grMessages.Controls.Add(this.label8);
			this.grMessages.Controls.Add(this.txtTo);
			this.grMessages.Enabled = false;
			this.grMessages.Location = new System.Drawing.Point(12, 210);
			this.grMessages.Name = "grMessages";
			this.grMessages.Size = new System.Drawing.Size(337, 146);
			this.grMessages.TabIndex = 9;
			this.grMessages.TabStop = false;
			this.grMessages.Text = "Send message";
			// 
			// btnSend
			// 
			this.btnSend.Location = new System.Drawing.Point(268, 45);
			this.btnSend.Name = "btnSend";
			this.btnSend.Size = new System.Drawing.Size(63, 23);
			this.btnSend.TabIndex = 2;
			this.btnSend.Text = "Send";
			this.btnSend.UseVisualStyleBackColor = true;
			this.btnSend.Click += new System.EventHandler(this.btnSend_Click);
			// 
			// txtMessage
			// 
			this.txtMessage.Location = new System.Drawing.Point(6, 45);
			this.txtMessage.Name = "txtMessage";
			this.txtMessage.ShowSelectionMargin = true;
			this.txtMessage.Size = new System.Drawing.Size(253, 95);
			this.txtMessage.TabIndex = 1;
			this.txtMessage.Text = "";
			this.txtMessage.KeyDown += new System.Windows.Forms.KeyEventHandler(this.txtMessage_KeyDown);
			this.txtMessage.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.txtMessage_KeyPress);
			// 
			// label8
			// 
			this.label8.AutoSize = true;
			this.label8.Location = new System.Drawing.Point(9, 26);
			this.label8.Name = "label8";
			this.label8.Size = new System.Drawing.Size(20, 13);
			this.label8.TabIndex = 8;
			this.label8.Text = "To";
			// 
			// txtTo
			// 
			this.txtTo.Location = new System.Drawing.Point(35, 19);
			this.txtTo.Name = "txtTo";
			this.txtTo.Size = new System.Drawing.Size(296, 20);
			this.txtTo.TabIndex = 0;
			// 
			// lblStatus
			// 
			this.lblStatus.AutoSize = true;
			this.lblStatus.Location = new System.Drawing.Point(14, 136);
			this.lblStatus.Name = "lblStatus";
			this.lblStatus.Size = new System.Drawing.Size(95, 13);
			this.lblStatus.TabIndex = 10;
			this.lblStatus.Text = "Status: Not Bound";
			// 
			// splitContainer1
			// 
			this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer1.Location = new System.Drawing.Point(0, 0);
			this.splitContainer1.Name = "splitContainer1";
			// 
			// splitContainer1.Panel1
			// 
			this.splitContainer1.Panel1.Controls.Add(this.label9);
			this.splitContainer1.Panel1.Controls.Add(this.cboServerDataCoding);
			this.splitContainer1.Panel1.Controls.Add(this.chk7bitPacking);
			this.splitContainer1.Panel1.Controls.Add(this.label3);
			this.splitContainer1.Panel1.Controls.Add(this.cboDataCoding);
			this.splitContainer1.Panel1.Controls.Add(this.grLogin);
			this.splitContainer1.Panel1.Controls.Add(this.lblStatus);
			this.splitContainer1.Panel1.Controls.Add(this.btnBind);
			this.splitContainer1.Panel1.Controls.Add(this.grMessages);
			// 
			// splitContainer1.Panel2
			// 
			this.splitContainer1.Panel2.Controls.Add(this.groupOutbox);
			this.splitContainer1.Panel2.Controls.Add(this.groupInbox);
			this.splitContainer1.Size = new System.Drawing.Size(625, 368);
			this.splitContainer1.SplitterDistance = 356;
			this.splitContainer1.TabIndex = 11;
			// 
			// chk7bitPacking
			// 
			this.chk7bitPacking.AutoSize = true;
			this.chk7bitPacking.Location = new System.Drawing.Point(128, 179);
			this.chk7bitPacking.Name = "chk7bitPacking";
			this.chk7bitPacking.Size = new System.Drawing.Size(88, 17);
			this.chk7bitPacking.TabIndex = 13;
			this.chk7bitPacking.Text = "7-bit Packing";
			this.chk7bitPacking.UseVisualStyleBackColor = true;
			this.chk7bitPacking.CheckedChanged += new System.EventHandler(this.dataCoding_SelectionChanged);
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(15, 160);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(65, 13);
			this.label3.TabIndex = 12;
			this.label3.Text = "Data coding";
			// 
			// cboDataCoding
			// 
			this.cboDataCoding.FormattingEnabled = true;
			this.cboDataCoding.Location = new System.Drawing.Point(17, 177);
			this.cboDataCoding.Name = "cboDataCoding";
			this.cboDataCoding.Size = new System.Drawing.Size(105, 21);
			this.cboDataCoding.TabIndex = 11;
			this.cboDataCoding.SelectedIndexChanged += new System.EventHandler(this.dataCoding_SelectionChanged);
			// 
			// groupOutbox
			// 
			this.groupOutbox.Controls.Add(this.gridOutbox);
			this.groupOutbox.Location = new System.Drawing.Point(3, 179);
			this.groupOutbox.Name = "groupOutbox";
			this.groupOutbox.Size = new System.Drawing.Size(259, 178);
			this.groupOutbox.TabIndex = 5;
			this.groupOutbox.TabStop = false;
			this.groupOutbox.Text = "Outbox";
			// 
			// gridOutbox
			// 
			this.gridOutbox.AllowUserToAddRows = false;
			this.gridOutbox.AllowUserToDeleteRows = false;
			this.gridOutbox.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
			this.gridOutbox.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
			this.gridOutbox.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.dataGridViewTextBoxColumn2,
            this.dataGridViewTextBoxColumn3});
			this.gridOutbox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.gridOutbox.Location = new System.Drawing.Point(3, 16);
			this.gridOutbox.Name = "gridOutbox";
			this.gridOutbox.RowHeadersVisible = false;
			this.gridOutbox.Size = new System.Drawing.Size(253, 159);
			this.gridOutbox.TabIndex = 1;
			this.toolTip1.SetToolTip(this.gridOutbox, "Outbox");
			this.gridOutbox.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.grid_MouseDoubleClick);
			// 
			// dataGridViewTextBoxColumn2
			// 
			this.dataGridViewTextBoxColumn2.DataPropertyName = "To";
			this.dataGridViewTextBoxColumn2.FillWeight = 61.46054F;
			this.dataGridViewTextBoxColumn2.HeaderText = "To";
			this.dataGridViewTextBoxColumn2.Name = "dataGridViewTextBoxColumn2";
			this.dataGridViewTextBoxColumn2.ReadOnly = true;
			// 
			// dataGridViewTextBoxColumn3
			// 
			this.dataGridViewTextBoxColumn3.DataPropertyName = "Message";
			this.dataGridViewTextBoxColumn3.FillWeight = 180.694F;
			this.dataGridViewTextBoxColumn3.HeaderText = "Message";
			this.dataGridViewTextBoxColumn3.Name = "dataGridViewTextBoxColumn3";
			this.dataGridViewTextBoxColumn3.ReadOnly = true;
			// 
			// groupInbox
			// 
			this.groupInbox.Controls.Add(this.gridInbox);
			this.groupInbox.Location = new System.Drawing.Point(6, 13);
			this.groupInbox.Name = "groupInbox";
			this.groupInbox.Size = new System.Drawing.Size(259, 163);
			this.groupInbox.TabIndex = 4;
			this.groupInbox.TabStop = false;
			this.groupInbox.Text = "Inbox";
			// 
			// gridInbox
			// 
			this.gridInbox.AllowUserToAddRows = false;
			this.gridInbox.AllowUserToDeleteRows = false;
			this.gridInbox.AllowUserToResizeRows = false;
			this.gridInbox.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
			this.gridInbox.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
			this.gridInbox.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.colFrom,
            this.colTo,
            this.colMsg});
			this.gridInbox.Dock = System.Windows.Forms.DockStyle.Fill;
			this.gridInbox.Location = new System.Drawing.Point(3, 16);
			this.gridInbox.Name = "gridInbox";
			this.gridInbox.RowHeadersVisible = false;
			this.gridInbox.Size = new System.Drawing.Size(253, 144);
			this.gridInbox.TabIndex = 0;
			this.toolTip1.SetToolTip(this.gridInbox, "Inbox");
			this.gridInbox.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.grid_MouseDoubleClick);
			// 
			// colFrom
			// 
			this.colFrom.DataPropertyName = "From";
			this.colFrom.FillWeight = 76.14214F;
			this.colFrom.HeaderText = "From";
			this.colFrom.Name = "colFrom";
			this.colFrom.ReadOnly = true;
			// 
			// colTo
			// 
			this.colTo.DataPropertyName = "To";
			this.colTo.FillWeight = 75.4987F;
			this.colTo.HeaderText = "To";
			this.colTo.Name = "colTo";
			this.colTo.ReadOnly = true;
			// 
			// colMsg
			// 
			this.colMsg.DataPropertyName = "Message";
			this.colMsg.FillWeight = 148.3592F;
			this.colMsg.HeaderText = "Message";
			this.colMsg.Name = "colMsg";
			this.colMsg.ReadOnly = true;
			// 
			// toolTip1
			// 
			this.toolTip1.AutomaticDelay = 100;
			this.toolTip1.IsBalloon = true;
			// 
			// label9
			// 
			this.label9.AutoSize = true;
			this.label9.Location = new System.Drawing.Point(231, 161);
			this.label9.Name = "label9";
			this.label9.Size = new System.Drawing.Size(97, 13);
			this.label9.TabIndex = 15;
			this.label9.Text = "Server data coding";
			// 
			// cboServerDataCoding
			// 
			this.cboServerDataCoding.FormattingEnabled = true;
			this.cboServerDataCoding.Location = new System.Drawing.Point(234, 177);
			this.cboServerDataCoding.Name = "cboServerDataCoding";
			this.cboServerDataCoding.Size = new System.Drawing.Size(105, 21);
			this.cboServerDataCoding.TabIndex = 14;
			this.cboServerDataCoding.SelectedIndexChanged += new System.EventHandler(this.dataCoding_SelectionChanged);
			// 
			// Form1
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(625, 368);
			this.Controls.Add(this.splitContainer1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.MaximizeBox = false;
			this.Name = "Form1";
			this.Text = "ESME Simulator";
			this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
			this.grLogin.ResumeLayout(false);
			this.grLogin.PerformLayout();
			this.grMessages.ResumeLayout(false);
			this.grMessages.PerformLayout();
			this.splitContainer1.Panel1.ResumeLayout(false);
			this.splitContainer1.Panel1.PerformLayout();
			this.splitContainer1.Panel2.ResumeLayout(false);
			this.splitContainer1.ResumeLayout(false);
			this.groupOutbox.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.gridOutbox)).EndInit();
			this.groupInbox.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)(this.gridInbox)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.GroupBox grLogin;
		private System.Windows.Forms.TextBox txtAddress;
		private System.Windows.Forms.Button btnBind;
		private System.Windows.Forms.TextBox txtPort;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.TextBox txtServer;
		private System.Windows.Forms.TextBox txtUser;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox txtPass;
		private System.Windows.Forms.Label label6;
		private System.Windows.Forms.Label label5;
		private System.Windows.Forms.ComboBox cboMode;
		private System.Windows.Forms.TextBox txtSystemType;
		private System.Windows.Forms.Label label7;
		private System.Windows.Forms.GroupBox grMessages;
		private System.Windows.Forms.Label label8;
		private System.Windows.Forms.TextBox txtTo;
		private System.Windows.Forms.Button btnSend;
		private System.Windows.Forms.RichTextBox txtMessage;
		private System.Windows.Forms.Label lblStatus;
		private System.Windows.Forms.SplitContainer splitContainer1;
		private System.Windows.Forms.DataGridView gridInbox;
		private System.Windows.Forms.DataGridView gridOutbox;
		private System.Windows.Forms.ToolTip toolTip1;
		private System.Windows.Forms.GroupBox groupInbox;
		private System.Windows.Forms.GroupBox groupOutbox;
		private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn2;
		private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn3;
		private System.Windows.Forms.DataGridViewTextBoxColumn colFrom;
		private System.Windows.Forms.DataGridViewTextBoxColumn colTo;
		private System.Windows.Forms.DataGridViewTextBoxColumn colMsg;
		private System.Windows.Forms.CheckBox chk7bitPacking;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.ComboBox cboDataCoding;
		private System.Windows.Forms.Label label9;
		private System.Windows.Forms.ComboBox cboServerDataCoding;

	}
}

