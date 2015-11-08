using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.ComponentModel;

namespace esmesim.smpp
{
	public enum BindType : int
	{
		Receiver,
		Transmitter,
		Transceiver
	}

	public enum BindResult : int
	{
		Ok,
		InvalidUser,
		InvalidPassword,
		InvalidAddress,
		InvalidCommand,
		Fail
	}

	public enum DeliveryResult : int
	{
		Ok,
		Rejected,
		InvalidSourceAddress,
		InvalidDestinationAddress,
		Fail
	}

	public enum DisconnectReason : int
	{
		Unbind,
		Kicked,
		NetworkError
	}

	public enum DataCoding : int
	{
		Default = 0,
		Ansi    = 1,
		Gsm0338 = 2,
		Latin1  = 3,
		Unicode = 8,
		UTF8    = 11
	}

	/// <summary>
	/// Defines setting for message delivering and reception
	/// Allows us to execute proper workaround to common SMSC limitations
	/// </summary>
	[StructLayout(LayoutKind.Sequential)]
	public sealed class MessageSettings
	{
		/// <summary>
		/// Defines the data coding used to sent messages (default = 0)
		/// </summary>
		[MarshalAs(UnmanagedType.U4)]
		public DataCoding DeliverDataCoding;

		/// <summary>
		/// Defines the encoding used by the SMSC, used when \c DeliverDataCoding = 0
		/// </summary>
		[MarshalAs(UnmanagedType.U4)]
		public DataCoding ServerDefaultEncoding;

		/// <summary>
		/// Determines when the message content should be encoded using GSM-7bit packing (default=0)
		/// </summary>
		[MarshalAs(UnmanagedType.U1)]
		public bool EnableGSM7bitPacking;

		/// <summary>
		/// The maximum count in characters of a message (default = 0: MAX_MESSAGE_LENGTH)
		/// Please note that if \c EnablePayload is set longer messages can be sent
		/// </summary>
		[MarshalAs(UnmanagedType.U4)]
		public int MaxMessageLength;

		/// <summary>
		/// Allows to use the SMPP concatenated message function (default = 1)
		/// </summary>
		[MarshalAs(UnmanagedType.U1)]
		public bool EnableMessageConcatenation;

		/// <summary>
		/// Allows the long messages as a payload (TLV) (default = 1)
		/// </summary>
		[MarshalAs(UnmanagedType.U1)]
		public bool EnablePayload;

		/// <summary>
		/// This flag indicates if the SUBMIT_MULTI_SM operation is supported by the server.
		/// If it is not then requests of this type are split into individual
		/// SUBMIT_SM operations (default = 1)
		/// </summary>
		[MarshalAs(UnmanagedType.U1)]
		public bool EnableSubmitMulti;
	}

	/// <summary>
	/// Represents the method that will handle the <typeparamref name="SMPPClient.NewMessage"/> event.
	/// </summary>
	/// <param name="from">Number the message is from</param>
	/// <param name="to">Number the message is directed to</param>
	/// <param name="content">The message content</param>
	public delegate void NewMessageEventHandler(string from, string to, string content);

	/// <summary>
	/// Represents the method that will handle the <typeparamref name="SMPPClient.ConnectionLost"/> event.
	/// </summary>
	public delegate void ConnectionLostEventHandler();

	/// <summary>
	///
	/// </summary>
	public class DeliveryException : Exception
	{
		public DeliveryException(DeliveryResult r)
		{
			m_result = r;
		}

		public DeliveryResult Result
		{
			get
			{
				return m_result;
			}

		}

		private DeliveryResult m_result;
	}

	/// <summary>
	///
	/// </summary>
	public class BindException : Exception
	{
		/// <summary>
		///
		/// </summary>
		/// <param name="r"></param>
		public BindException(BindResult r)
		{
			m_result = r;
		}

		/// <summary>
		///
		/// </summary>
		public BindResult Result
		{
			get
			{
				return m_result;
			}

		}

		private BindResult m_result;
	}

	public unsafe class SMPPClient : IDisposable
	{

#if(DEBUG)
		const string LIB_SMPP = "smpp-d";
#else
		const string LIB_SMPP = "smpp";
#endif

		/// <summary>
		/// Internal structure to handle incoming text messages
		/// </summary>
		private struct MessageInfo
		{
			public readonly string From;
			public readonly string To;
			public readonly string Message;
			public MessageInfo(string from, string to, string msg)
			{
				From = from;
				To = to;
				Message = msg;
			}
		}

		#region Native Types

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate void Callback_OnIncomingMessage(
							 IntPtr hClient,
							 string from,
							 string to,
							 [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 4)] byte[] content,
							 [MarshalAs(UnmanagedType.U4)] int bufferSize
				  );

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate void Callback_OnConnectionLost(
							 IntPtr hClient,
							 [MarshalAs(UnmanagedType.I4)] int reason
				  );

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern void libSMPP_CreateDefaultMessageSettings(
			[MarshalAs(UnmanagedType.Struct)] ref MessageSettings ms
		);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern IntPtr libSMPP_ClientCreate(
					[MarshalAs(UnmanagedType.FunctionPtr)] Callback_OnIncomingMessage onNewMessage,
					[MarshalAs(UnmanagedType.FunctionPtr)] Callback_OnConnectionLost onConnectionLost
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern IntPtr libSMPP_ClientDelete(
					IntPtr hClient
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern void libSMPP_ClientSetServerAddress (
					IntPtr hClient, string serverIP, ushort serverPort
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern void libSMPP_ClientSetLoginType(
					IntPtr hClient, BindType loginType
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern void libSMPP_ClientSetSystemId(
					IntPtr hClient, string systemId
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern void libSMPP_ClientSetSystemType(
					IntPtr hClient, string systemType
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern void libSMPP_ClientSetPassword(
					IntPtr hClient, string password
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern void libSMPP_ClientSetAddressRange(
					IntPtr hClient, string pattern
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern void libSMPP_ClientSetMessageSettings(
					IntPtr hClient,
					[MarshalAs(UnmanagedType.LPStruct)] MessageSettings ms
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern BindResult libSMPP_ClientBind(
					IntPtr hClient
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern void libSMPP_ClientUnBind(
					IntPtr hClient
			);

		[DllImport(LIB_SMPP, CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		private static extern DeliveryResult libSMPP_ClientSendMessage(
					IntPtr hClient,
					string from,
					string to,
					[MarshalAs(UnmanagedType.LPArray)] byte[] content,
					[MarshalAs(UnmanagedType.U4)] int size
			);

		#endregion // Native Types

		private IntPtr m_handle;
		private MessageSettings m_settings;
		private bool m_bound = false;
		private string m_systemId   = String.Empty;
		private string m_systemType = String.Empty;
		private string m_password   = String.Empty;
		private List<string> m_addresses = new List<string>();

		private NewMessageEventHandler     m_onNewMessage;
		private ConnectionLostEventHandler m_onConnectionLost;

		private Callback_OnIncomingMessage m_onNewMessageNative;
		private Callback_OnConnectionLost  m_onConnectionLostNative;

		public SMPPClient()
		{
			Queue<MessageInfo> messageQueue = new Queue<MessageInfo>();

			m_onNewMessageNative = new Callback_OnIncomingMessage(delegate(IntPtr hClient, string from, string to, byte[] content, int bufferSize)
			{
				lock (messageQueue)
				{
					int size = messageQueue.Count;
					messageQueue.Enqueue(new MessageInfo(from, to, Encoding.UTF8.GetString(content)));
					if (size == 0)
					{
						BackgroundWorker bw = new BackgroundWorker();
						bw.DoWork += new DoWorkEventHandler(delegate(object sender, DoWorkEventArgs e)
						{
							MessageInfo info;
							while (true)
							{
								lock (messageQueue)
								{
									if (messageQueue.Count == 0)
									{
										break;
									}
									info = messageQueue.Dequeue();
								}
								try
								{
									if (m_onNewMessage != null)
									{
										m_onNewMessage(info.From, info.To, info.Message);
									}
								}
								catch (Exception ex)
								{
									Debug.WriteLine(ex.ToString());
								}
							}
						});
						bw.RunWorkerAsync();
					}
				}
			});

			m_onConnectionLostNative = new Callback_OnConnectionLost(delegate(IntPtr hClient, int reason)
			{
				if (!m_bound)
				{
					Debug.WriteLine("OnConnectionLost called unbound");
				}

				m_bound = false;

				if (m_onConnectionLost != null)
				{
					m_onConnectionLost();
				}
			});

			m_handle = libSMPP_ClientCreate(m_onNewMessageNative, m_onConnectionLostNative);

			m_settings = new MessageSettings();
			libSMPP_CreateDefaultMessageSettings(ref m_settings);
		}

		/// <summary>
		/// Fired when a new text message arrives
		/// </summary>
		public event NewMessageEventHandler NewMessage
		{
			add
			{
				m_onNewMessage += value;
			}
			remove
			{
				m_onNewMessage -= value;
			}
		}

		/// <summary>
		/// Fired when the connection with the server is lost
		/// </summary>
		public event ConnectionLostEventHandler ConnectionLost
		{
			add
			{
				m_onConnectionLost += value;
			}
			remove
			{
				m_onConnectionLost -= value;
			}
		}

		/// <summary>
		/// Returns <c>true</c> if the client is connected with the server
		/// </summary>
		public bool Bound
		{
			get
			{
				return m_bound;
			}
		}

		/// <summary>
		/// Gets or sets the login user (aka system id)
		/// </summary>
		public string SystemId
		{
			get
			{
				return m_systemId;
			}
			set
			{
				OnUsernameChanged(value);
			}
		}

		/// <summary>
		/// Gets or sets the user password
		/// </summary>
		public string Password
		{
			get
			{
				return m_password;
			}
			set
			{
				OnPasswordChanged(value);
			}
		}

		/// <summary>
		/// Gets or sets the system type
		/// </summary>
		public string SystemType
		{
			get
			{
				return m_systemType;
			}
			set
			{
				OnSystemTypeChanged(value);
			}
		}

		/// <summary>
		/// Contains the list of numbers being handled by this user
		/// </summary>
		public ICollection<string> Addresses
		{
			get
			{
				return m_addresses;
			}
		}

		public MessageSettings Settings
		{
			get
			{
				return m_settings;
			}
		}

		/// <summary>
		///
		/// </summary>
		/// <param name="bindType"></param>
		/// <param name="server"></param>
		/// <param name="port"></param>
		public void Bind(BindType bindType, string server, ushort port)
		{
			if (m_bound)
			{
				throw new InvalidOperationException("Already bound");
			}

			libSMPP_ClientSetLoginType(m_handle, bindType);
			libSMPP_ClientSetServerAddress(m_handle, server, port);
			libSMPP_ClientSetAddressRange(m_handle, String.Join("|", m_addresses.ToArray()));
			libSMPP_ClientSetMessageSettings(m_handle, m_settings);

			BindResult result = libSMPP_ClientBind(m_handle);

			if (result != BindResult.Ok)
			{
				throw new BindException(result);
			}

			m_bound = true;
		}

		/// <summary>
		///
		/// </summary>
		public void Unbind()
		{
			if (!m_bound)
			{
				throw new InvalidOperationException("Not bound");
			}

			libSMPP_ClientUnBind(m_handle);
			m_bound = false;
		}

		/// <summary>
		///
		/// </summary>
		/// <param name="from"></param>
		/// <param name="to"></param>
		/// <param name="content"></param>
		public void SendMessage(string from, string to, string content)
		{
			if (!m_bound)
			{
				throw new InvalidOperationException("Not bound");
			}

			byte[] contentBytes = Encoding.UTF8.GetBytes(content);

			DeliveryResult result = libSMPP_ClientSendMessage(m_handle, from, to, contentBytes, contentBytes.Length);

			if (result != DeliveryResult.Ok)
			{
				throw new DeliveryException(result);
			}
		}

		private void OnUsernameChanged(string newvalue)
		{
			if (m_bound)
			{
				throw new InvalidOperationException("Already bound");
			}

			if (newvalue != m_systemId)
			{
				m_systemId = newvalue;
				libSMPP_ClientSetSystemId(m_handle, m_systemId);
			}
		}

		private void OnPasswordChanged(string newvalue)
		{
			if (m_bound)
			{
				throw new InvalidOperationException("Already bound");
			}

			if (newvalue != m_password)
			{
				m_password = newvalue;
				libSMPP_ClientSetPassword(m_handle, m_password);
			}
		}

		private void OnSystemTypeChanged(string newvalue)
		{
			if (m_bound)
			{
				throw new InvalidOperationException("Already bound");
			}

			if (newvalue != m_systemType)
			{
				m_systemType = newvalue;
				libSMPP_ClientSetSystemType(m_handle, m_systemType);
			}
		}

		#region IDisposable implementation
		public void Dispose()
		{
			if (m_bound)
			{
				Unbind();
			}

			libSMPP_ClientDelete(m_handle);
			m_handle = IntPtr.Zero;

			GC.SuppressFinalize(this);
		}
		#endregion
	}
}
