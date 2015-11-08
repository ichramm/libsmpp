/*!
 * \file smppclient.cpp
 * \author ichramm
 *
 * Created on November 19, 2011, 12:17 AM
 */
#include "stdafx.h"
#include "../libsmpp.hpp"

#include "smppconnection.h"
#include "smppcommands.h"
#include "converter.h"
#include "logger.h"

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/detail/thread.hpp>

#include <algorithm>
#include <vector>

#ifdef _WIN32
# pragma push_macro("SendMessage")
# undef SendMessage
#endif

using namespace std;
using namespace boost;

#define CLIENT_THREAD_COUNT   ((unsigned int)2)

namespace libsmpp
{

class ClientThread
{
public:

	static shared_ptr<ClientThread> GetInstance()
	{
		if(!s_flag) {
			lock_guard<mutex> lock(s_mutex);
			if(!s_flag) {
				s_flag = true;
				s_instance = make_shared<ClientThread>();
			}
		}
		return s_instance;
	}

	ClientThread()
		: m_running(true), m_work(m_ioservice)
	{
		SetThreads(CLIENT_THREAD_COUNT);
	}

	~ClientThread()
	{
		m_running = false;
		m_ioservice.stop();
		for ( ; !m_threads.empty(); m_threads.pop_back())
		{
			m_threads.back()->interrupt();
			m_threads.back()->join();
		}
	}

	void Run()
	{
		for( ; m_running ; )
		{
			try
			{
				m_ioservice.run();
			}
			catch (const thread_interrupted& )
			{
				break;
			}
			catch (const std::exception& e)
			{
				smpp_log_error("Error running ioservice: %s", e.what());
			}
		}
	}

	void SetThreads(unsigned int count)
	{
		if (count > m_threads.size())
		{
			for (unsigned int i = m_threads.size(); i < count; i++)
			{
				m_threads.push_back(make_shared<thread>(&ClientThread::Run, this));
			}
		}
		else if (count < m_threads.size())
		{
			for ( ; m_threads.size() > count; m_threads.pop_back())
			{
				m_threads.back()->interrupt();
				m_threads.back()->join();
			}
		}
	}

	ioservice_t& GetIOService()
	{
		return m_ioservice;
	}

private:
	volatile bool                   m_running;
	asio::io_service                m_ioservice;
	asio::io_service::work          m_work;
	vector<shared_ptr<thread> >     m_threads;
	static volatile bool            s_flag;
	static mutex                    s_mutex;
	static shared_ptr<ClientThread> s_instance;
};

volatile bool            ClientThread::s_flag = false;
mutex                    ClientThread::s_mutex;
shared_ptr<ClientThread> ClientThread::s_instance;

struct CSMPPClient::impl
{
	impl(shared_ptr<CESMECallback> callbacks, const std::string &ip, unsigned short port, BindType mode)
	 : m_isBound(false)
	 , m_callbacks(callbacks)
	 , m_serverIP(ip)
	 , m_serverPort(port)
	 , m_loginMode(mode)
	 , m_threadPool(ClientThread::GetInstance())
	{
		libSMPP_CreateDefaultMessageSettings(&m_settings);
		//m_connection.reset(new CSMPPClientConnection(m_threadPool->GetIOService(), bind(&impl::OnNewData, this, _1, _2),  bind(&impl::OnConnectionLost, this, _1)));
	}

	~impl()
	{
		Unbind();
	}

	void CreateConnection()
	{
#ifdef _MSC_VER
// you wan'it babe! you wan'it!
		m_connection.reset(new CSMPPClientConnection(
				m_threadPool->GetIOService(),
				bind(&impl::OnNewData, this, _1, _2),
				bind(&impl::OnConnectionLost, this, _1)
			));
#else
// we use make_shared whenever it's possible
		m_connection = make_shared <
				CSMPPClientConnection,
				ioservice_t&,
				CSMPPConnection::NewCommandCallback,
				CSMPPConnection::ConnectionLostCallback
			>(
					m_threadPool->GetIOService(),
					bind(&impl::OnNewData, this, _1, _2),
					bind(&impl::OnConnectionLost, this, _1)
			);
#endif
	}

 	void OnNewData(SMPPConnectionPtr con, shared_ptr<ISMPPCommand> icmd)
	{
		SMPP_TRACE();
		switch (icmd->request_id())
		{
		case DELIVER_SM:
			{ // Got new message
				shared_ptr<CSMPPDelivery> cmd = dynamic_pointer_cast<CSMPPDelivery>(icmd);

				string text_in = cmd->getText(), text_out;

				const char* charset_out = "UTF-8";
				const char* charset_in = CConverter::GetCharsetFromDataCoding(cmd->request().data_coding);

				if (cmd->request().data_coding == 0x08 && m_settings.BigEndianUnicode)
				{ // ok, this is the case, transform the string before converting it!
					if (text_in.size() & 1u)
					{ // log a warning with invalid unicode strings
						string aux = text_in;
						for ( unsigned i = 0; i < aux.size(); ++i ) {
							if ( !aux[i] ) aux[i] = '.';
						}
						smpp_log_warning("Received unicode string with odd length: %s", aux.c_str());
						// insert null in the second-last element
						text_in.insert(text_in.end()-1, '\0');
					}
					for (unsigned i = 0; i < text_in.size(); i += 2)
					{ // two bytes only, to restore endianness it's enough to swap bytes
						swap(text_in[i], text_in[i+1]); // i+1 is always valid
					}
				}

				CConverter c(charset_in, charset_out);
				if (0 != c.Convert(text_in, text_out))
				{
					smpp_log_error("Failed to convert text [%s] to UTF-8", text_in.c_str());
					cmd->command_status(ESME_RSYSERR);
					con->SendResponse(icmd);
					return;
				}

				m_callbacks->OnIncomingMessage(cmd->getSourceAddress(), cmd->getDestinationAddress(), text_out);
			}
			icmd->command_status(ESME_ROK);
			break;
		case ENQUIRE_LINK:
			icmd->command_status(ESME_ROK);
			break;
		default: // reject any unknown command
			icmd = make_shared<CSMPPGenericNack>(icmd->sequence_number());
			break;
		}

		con->SendResponse(icmd);
	}

	void OnConnectionLost(SMPPConnectionPtr)
	{
		if (m_isBound == false) {
			return; // thrown when disconnecting
		}

		smpp_log_warning("Connection with server %s:%d has been lost", m_serverIP.c_str(), m_serverPort);

		Reset();
		m_callbacks->OnConnectionLost(0);
	}

	LoginResult Bind()
	{
		int res;
		shared_ptr<ISMPPBind> cmd;

		CreateConnection();

		if (0 != m_connection->Connect(m_serverIP, m_serverPort))
		{
			smpp_log_error("Failed to connect to server %s:%u", m_serverIP.c_str(), m_serverPort);
			return LOGIN_RESULT_FAIL;
		}

		if(m_loginMode == BIND_TYPE_RECEIVER)
		{
			cmd = make_shared<CBindReceiver>(m_connection->NextSequenceNumber());
		}
		else if (m_loginMode == BIND_TYPE_TRANSMITTER)
		{
			cmd = make_shared<CBindTransmitter>(m_connection->NextSequenceNumber());
		}
		else
		{
			cmd = make_shared<CBindTransceiver>(m_connection->NextSequenceNumber());
		}

		if(!cmd)
		{
			smpp_log_fatal("Out of Memory");
			return LOGIN_RESULT_FAIL;
		}

		cmd->setSystemInfo(m_systemId, m_password, m_systemType);
		cmd->setAddress(m_addressRange, TON_UNKNOWN, NPI_UNKNOWN);

		res = m_connection->SendRequest(cmd->shared_from_this());
		if(res != RESULT_OK)
		{
			smpp_log_error("Failed to send bind request (%d)", res);
			return LOGIN_RESULT_FAIL;
		}

		res = cmd->command_status();
		if(ESME_ROK == res)
		{ // done
			m_isBound = true;
			m_serverSystemId = cmd->getResponseSystemId();
			return LOGIN_RESULT_OK;
		}

		// the connection must be closed if bind fails
		m_connection->Close();

		if(ESME_RINVSYSID == res) {
			return LOGIN_RESULT_INVALIDUSR;
		}
		if(ESME_RINVPASWD == res) {
			return LOGIN_RESULT_INVALIDPWD;
		}
		if(ESME_RINVCMDID == res) {
			return LOGIN_RESULT_INVALIDCMD;
		}
		if(ESME_RBINDFAIL != res) {
			smpp_log_error("Uknown bind error: %d", res);
		}
		return LOGIN_RESULT_FAIL;
	}

	void Unbind()
	{
		if(m_isBound == false)
		{
			return;
		}

		Reset();

		shared_ptr<CSMPPUnbind> cmd = make_shared<CSMPPUnbind>(m_connection->NextSequenceNumber());
		m_connection->SendRequest(cmd->shared_from_this());
		m_connection->Close();
	}

	bool SendKeepAlive()
	{
		int res;
		shared_ptr<CSMPPEnquireLink> cmd = make_shared<CSMPPEnquireLink>(m_connection->NextSequenceNumber());

		res = m_connection->SendRequest(cmd->shared_from_this());
		if ( res == RESULT_OK )
		{
			res = cmd->command_status();
			if ( res == ESME_ROK )
			{
				return true;
			}

			smpp_log_warning("Unexpected response status (%d)", res);
			return false;
		}
		smpp_log_warning("Failed to send ENQUIRE_LINK packet (%d)", res);
		return false;
	}

	DeliveryResult HandleMessageResponse( shared_ptr<CSMPPSubmitSingle> cmd )
	{
		int status = cmd->command_status();
		switch(status)
		{
			case ESME_ROK:
				return DELIVERY_OK;
			case ESME_RINVSRCADR:
				return DELIVERY_INV_SRC_ADDR;
			case ESME_RINVDSTADR:
				return DELIVERY_INV_DEST_ADDR;
			case ESME_RINVCMDID:
				return DELIVERY_REJECTED;
			default:
				return DELIVERY_UNKNOWN_ERROR;
		}
	}

	DeliveryResult SendMessage ( const string &from, const string &to, const string &content)
	{
		if (m_isBound == false)
		{
			return DELIVERY_UNKNOWN_ERROR;
		}

		string text;
		const char *encoding = CConverter::GetCharsetFromDataCoding(m_settings.DeliverDataCoding ? m_settings.DeliverDataCoding : m_settings.ServerDefaultEncoding);
		CConverter converter("UTF-8", encoding);
		converter.Convert(content, text);

		shared_ptr<CSMPPSubmitSingle> cmd;

		if (text.length() <= MAX_MESSAGE_LENGTH &&
			( m_settings.EnablePayload || text.length() <= (m_settings.MaxMessageLength?min(m_settings.MaxMessageLength, 254u):254) ))
		{
			cmd = make_shared<CSMPPSubmitSingle>(m_connection->NextSequenceNumber());
			cmd->setDestination(to);
			cmd->setSourceAddress(from, TON_UNKNOWN, NPI_UNKNOWN);
			cmd->setText(text);
			cmd->request().data_coding = m_settings.DeliverDataCoding;

			int res;
			int retry = 0;
			do {
				res = m_connection->SendRequest(cmd->shared_from_this());
			}
			while (res != RESULT_OK && ++retry<3);

			if(res != RESULT_OK)
			{
				smpp_log_warning("Failed to send message: %d", res);
				return DELIVERY_UNKNOWN_ERROR;
			}

			return HandleMessageResponse(cmd);
		}

		// at this point the message is too long

		unsigned int sar_msg_ref_num = 0;
		unsigned int sar_total_segments = 0;
		unsigned int sar_segment_seqnum = 0;
		unsigned int messageLength = m_settings.EnablePayload ? 1024 : m_settings.MaxMessageLength ? min(m_settings.MaxMessageLength, 254u) : 254;

		if (m_settings.EnableMessageConcatenation)
		{
			sar_msg_ref_num = m_connection->NextSequenceNumber();
			sar_total_segments = (text.length()/messageLength) + (text.length()%messageLength ? 1:0);
		}

		for (unsigned int i = 0; i < text.length(); i += messageLength)
		{
			if(text.length()-i < messageLength) {
				messageLength = text.length() - i;
			}
			shared_ptr<CSMPPSubmitSingle> cmd = make_shared<CSMPPSubmitSingle>(m_connection->NextSequenceNumber());
			cmd->setDestination(to);
			cmd->setSourceAddress(from, TON_UNKNOWN, NPI_UNKNOWN);
			cmd->setText( text.substr(i, messageLength) );
			cmd->request().data_coding = m_settings.DeliverDataCoding;
			if (m_settings.EnableMessageConcatenation)
			{
				cmd->setConcatenatedMessageArgs(sar_total_segments, sar_msg_ref_num, ++sar_segment_seqnum);
			}

			int res;
			int retry = 0;
			do {
				res = m_connection->SendRequest(cmd->shared_from_this());
			}
			while (res != RESULT_OK && ++retry<3);

			if(res != RESULT_OK)
			{
				smpp_log_warning("Failed to send message chunk %d: %d", i, res);
				return DELIVERY_UNKNOWN_ERROR;
			}

			DeliveryResult delres = HandleMessageResponse(cmd);
			if (delres != DELIVERY_OK)
			{
				return delres;
			}
		}

		return DELIVERY_OK;
	}

	void Reset()
	{
		m_isBound = false;
		m_serverSystemId = "";
	}

	void SetMessageSettings(const MessageSettings &ms)
	{
		memcpy(&m_settings, &ms, sizeof(MessageSettings));
	}

	volatile bool                      m_isBound;
	shared_ptr<CESMECallback>          m_callbacks;
	string                             m_serverIP;
	unsigned short                     m_serverPort;
	BindType                           m_loginMode;
	std::string                        m_systemId;
	std::string                        m_systemType;
	std::string                        m_password;
	std::string                        m_addressRange;
	std::string                        m_serverSystemId;
	MessageSettings                    m_settings;
	shared_ptr<CSMPPClientConnection>  m_connection;
	shared_ptr<ClientThread>           m_threadPool;
};



CSMPPClient::CSMPPClient(shared_ptr<CESMECallback> callbacks, const std::string &ip, unsigned short port, BindType mode)
: pimpl(new impl(callbacks, ip, port, mode))
{

}

CSMPPClient::~CSMPPClient()
{
	// mmmh, nothing here...
}

string CSMPPClient::GetServerAddress() const {
	return pimpl->m_serverIP;
}
unsigned short CSMPPClient::GetServerPort() const {
	return pimpl->m_serverPort;
}
BindType CSMPPClient::GetBindMode() const {
	return pimpl->m_loginMode;
}
std::string CSMPPClient::GetSystemId() const {
	return pimpl->m_systemId;
}
std::string CSMPPClient::GetSystemType() const {
	return pimpl->m_systemType;
}
std::string CSMPPClient::GetPassword() const {
	return pimpl->m_password;
}
std::string CSMPPClient::GetAddressRange() const {
	return pimpl->m_addressRange;
}
bool CSMPPClient::IsBound() const {
	return pimpl->m_isBound;
}
std::string CSMPPClient::GetServerSystemId() const {
	return pimpl->m_serverSystemId;
}
void CSMPPClient::GetMessageSettings(MessageSettings *ms)
{
	memcpy(ms, &pimpl->m_settings, sizeof(MessageSettings));
}

void CSMPPClient::SetServerAddress(const string &server) {
	pimpl->m_serverIP = server;
}
void CSMPPClient::SetServerPort(unsigned short port) {
	pimpl->m_serverPort = port;
}
void CSMPPClient::SetBindMode(BindType lt) {
	pimpl->m_loginMode = lt;
}
void CSMPPClient::SetSystemId(const std::string &sysId) {
	pimpl->m_systemId = sysId;
}
void CSMPPClient::SetSystemType(const std::string &sysType) {
	pimpl->m_systemType = sysType;
}
void CSMPPClient::SetPassword(const std::string &password) {
	pimpl->m_password = password;
}
void CSMPPClient::SetAddressRange(const std::string &pattern) {
	pimpl->m_addressRange = pattern;
}
void CSMPPClient::SetMessageSettings(const MessageSettings &ms) {
	pimpl->SetMessageSettings(ms);
}

LoginResult CSMPPClient::Bind()
{
	return pimpl->Bind();
}

void CSMPPClient::Unbind()
{
	pimpl->Unbind();
}

bool CSMPPClient::SendKeepAlive()
{
	return pimpl->SendKeepAlive();
}

DeliveryResult CSMPPClient::SendMessage ( const string &from, const string &to, const string &content)
{
	return pimpl->SendMessage(from, to, content);
}

void CSMPPClient::SetClientThreads(unsigned int count)
{
	ClientThread::GetInstance()->SetThreads(count);
}

} // namespace libsmpp

#ifdef _WIN32
# pragma pop_macro("SendMessage")
#endif
