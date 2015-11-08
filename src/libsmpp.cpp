/*!
 * \file libsmpp.cpp
 * \author ichramm
 * 
 */
#include "stdafx.h"

#ifdef _WIN32
# pragma push_macro("SendMessage")
# undef SendMessage
#endif

#include "../libsmpp.hpp"
#include "smppserver.h"
#include "smppusersmanager.h"
#include "logger.h"

#include <boost/make_shared.hpp>

using namespace std;
using namespace boost;

namespace libsmpp
{

struct CSMPPServer::pimpl
{
public:
	pimpl(boost::shared_ptr<CSMSCCallback> callbacks, unsigned int port, unsigned int threads);
	int Start();
	void SetEnconding(DataCoding data_coding);
	void Stop();
	bool IsRunning() const;
	DeliveryResult SendMessage(const string &from,  const string &to, const string &message);

private:
	unsigned int m_threadCount;
	boost::shared_ptr<CSMPPUserManager>  m_userManager;
	boost::shared_ptr<CSMPPServerImpl>   m_server;
};

CSMPPServer::pimpl::pimpl(boost::shared_ptr<CSMSCCallback> callbacks, unsigned int port, unsigned int threads)
 : m_threadCount(threads)
 , m_userManager(new CSMPPUserManager(callbacks))
 , m_server(new CSMPPServerImpl(m_userManager, port))
{

}

int CSMPPServer::pimpl::Start()
{
	try {
		m_server->Start(m_threadCount);
		return 0;
	} catch (const std::exception &e) {
		smpp_log_error("Error starting server: %s", e.what());
	}
	return -1;
}

void CSMPPServer::pimpl::SetEnconding(DataCoding data_coding)
{
	m_userManager->SetDeliveryEncoding(data_coding);
}

void CSMPPServer::pimpl::Stop()
{
	m_server->Stop();
}

bool CSMPPServer::pimpl::IsRunning() const
{
	return m_server->IsRunning();
}

DeliveryResult CSMPPServer::pimpl::SendMessage( const string &from, const string &to, const string &message )
{
	return m_userManager->SendMessage(from, to, message);
}


/************************************************************************/
/************************************************************************/
CSMPPServer::CSMPPServer( boost::shared_ptr<CSMSCCallback> callbacks, unsigned int port, unsigned int threads )
: m_pimpl(new pimpl(callbacks, port, threads)) {
}

void CSMPPServer::SetEnconding(DataCoding data_coding)
{
	m_pimpl->SetEnconding(data_coding);
}

int CSMPPServer::Start() {
	return m_pimpl->Start();
}

void CSMPPServer::Stop() {
	m_pimpl->Stop();
}

bool CSMPPServer::IsRunning() const {
	return m_pimpl->IsRunning();
}

DeliveryResult CSMPPServer::SendMessage(const string &from, const string &to, const string &message) {
	return m_pimpl->SendMessage(from, to, message);
}


/************************************************************************/
/*     C-API implementation    */
/************************************************************************/

struct CAPICallback : public CSMSCCallback
{
	Callback_ValidateUser ValidateFn;
	Callback_DeliverMessage DeliverFn;
	Callback_OnUserDisconnected OnDisconnectFn;

	CAPICallback(Callback_ValidateUser v, Callback_DeliverMessage d, Callback_OnUserDisconnected o)
		: ValidateFn(v), DeliverFn(d), OnDisconnectFn(o)
	{ }

	LoginResult ValidateUser(unsigned int connectionId, BindType loginType, const string &systemId,
		const string &password, const string& address_range)
	{
		return ValidateFn(connectionId, loginType, systemId.c_str(), password.c_str(), address_range.c_str());
	}

	DeliveryResult DeliverMessage (unsigned int connectionId, const string &from, const string &to, const string &message)
	{
		return DeliverFn(connectionId, from.c_str(), to.c_str(), &message[0], message.size());
	}

	void OnUserDisconnected (unsigned int connectionId, const string &systemId, DisconnectReason reason)
	{
		OnDisconnectFn(connectionId, systemId.c_str(), reason);
	}
};

struct CAPIWrapper
{
	CAPIWrapper() : port(0) {}
	boost::shared_ptr<CSMSCCallback> callbacks;
	boost::shared_ptr<CSMPPServer> server;
	unsigned short port; // cached until Start() is called
};

class CAPIESMECallback : public CESMECallback
{
public:

	CAPIESMECallback(Callback_OnIncomingMessage onNewMessage, Callback_OnConnectionLost onConnectionLost)
	: m_handle(NULL), m_onNewMessage(onNewMessage), m_onConnectionLost(onConnectionLost)
	{
	}

	void SetOwner(ESME_HANDLE h)
	{
		m_handle = h;
	}

	virtual void OnIncomingMessage(const string& from, const string& to, const string& content)
	{
		if(m_handle && m_onNewMessage)
		{
			m_onNewMessage(m_handle, from.c_str(), to.c_str(), content.data(), content.size());
		}
	}

	virtual void OnConnectionLost(int cause)
	{
		if(m_handle && m_onConnectionLost)
		{
			m_onConnectionLost(m_handle, cause);
		}
	}

private:
	ESME_HANDLE                m_handle;
	Callback_OnIncomingMessage m_onNewMessage;
	Callback_OnConnectionLost  m_onConnectionLost;
};

} // namespace libsmpp


using namespace libsmpp;


extern "C"
{

SMPP_API const char *libSMPP_LoginTypeToString(BindType l)
{
	switch(l)
	{
	case BIND_TYPE_RECEIVER:
		return "Receiver";
	case BIND_TYPE_TRANSMITTER:
		return "Transmitter";
	case BIND_TYPE_TRANSCEIVER:
		return "Transceiver";
	}
	return "URKiddingRt?";
}

SMPP_API const char *libSMPP_LoginResultToString(LoginResult r)
{
	switch(r)
	{
	case LOGIN_RESULT_OK:
		return "OK";
	case LOGIN_RESULT_INVALIDUSR:
		return "InvalidUser";
	case LOGIN_RESULT_INVALIDPWD:
		return "InvalidPassword";
	case LOGIN_RESULT_INVALIDADDR:
		return "InvalidAddressRange";
	case LOGIN_RESULT_INVALIDCMD:
		return "InvalidCommand";
	case LOGIN_RESULT_FAIL:
	default: // avoid compiler warnings
		return "NetworkError";
	}
}

SMPP_API const char *libSMPP_DeliveryResultToString(DeliveryResult r)
{
	switch(r)
	{
	case DELIVERY_OK:
		return "OK";
	case DELIVERY_REJECTED:
		return "Rejected";
	case DELIVERY_INV_SRC_ADDR:
		return "Invalid Source Address";
	case DELIVERY_INV_DEST_ADDR:
		return "Invalid Destination Address";
	case DELIVERY_UNKNOWN_ERROR:
	default: // avoid compiler warnings
		return "Unknown Error";
	}
}

SMPP_API const char *libSMPP_DisconnectReasonToString(DisconnectReason r)
{
	switch(r)
	{
	case DISCONNECT_REASON_UNBIND:
		return "Unbind";
	case DISCONNECT_REASON_KICKED:
		return "Kicked";
	case DISCONNECT_REASON_NETERROR:
	default: // avoid compiler warnings
		return "NetworkError";
	}
}

SMPP_API void libSMPP_CreateDefaultMessageSettings(MessageSettings *ms)
{
	memset(ms, 0, sizeof(MessageSettings));
	ms->EnablePayload = 1;
	ms->EnableSubmitMulti = 1;
	ms->EnableMessageConcatenation = 1;
}

SMPP_API SMSC_HANDLE libSMPP_ServerCreate()
{
	SMSC_HANDLE hServer = (SMSC_HANDLE) new CAPIWrapper();
	return hServer;
}

SMPP_API void libSMPP_ServerDelete(SMSC_HANDLE hServer)
{
	CAPIWrapper *w = reinterpret_cast<CAPIWrapper*>(hServer);
	delete w;
}

SMPP_API void libSMPP_ServerSetPort(SMSC_HANDLE hServer, unsigned int port)
{
	CAPIWrapper *w = reinterpret_cast<CAPIWrapper*>(hServer);
	w->port = port;
}

SMPP_API void libSMPP_ServerSetCallbacks(SMSC_HANDLE hServer,
                                         Callback_ValidateUser validateFn,
                                         Callback_DeliverMessage deliverFn,
                                         Callback_OnUserDisconnected onDisconFn)
{
	CAPIWrapper *w = reinterpret_cast<CAPIWrapper*>(hServer);
	w->callbacks = make_shared<CAPICallback>(validateFn, deliverFn, onDisconFn);
}

SMPP_API int libSMPP_ServerStart(SMSC_HANDLE hServer)
{
	CAPIWrapper *w = reinterpret_cast<CAPIWrapper*>(hServer);
	if(!w->callbacks || !w->port) {
		return -1;
	}
	w->server = make_shared<CSMPPServer>(w->callbacks, w->port);
	return w->server->Start();
}

SMPP_API void libSMPP_ServerStop(SMSC_HANDLE hServer)
{
	CAPIWrapper *w = reinterpret_cast<CAPIWrapper*>(hServer);
	w->server->Stop();
}

SMPP_API DeliveryResult libSMPP_ServerSendMessage(SMSC_HANDLE hServer, const char *from,
                                                  const char *to, const char *message)
{
	CAPIWrapper *w = reinterpret_cast<CAPIWrapper*>(hServer);
	return w->server->SendMessage(from, to, message);
}


SMPP_API ESME_HANDLE libSMPP_ClientCreate(Callback_OnIncomingMessage onNewMessage,
                                          Callback_OnConnectionLost onConnectionLost)
{
	ESME_HANDLE hClient;

	shared_ptr<CAPIESMECallback>
		callbacks = make_shared<CAPIESMECallback>(onNewMessage, onConnectionLost);

	hClient = (ESME_HANDLE) new CSMPPClient(callbacks->shared_from_this());

	callbacks->SetOwner(hClient);

	return hClient;
}

SMPP_API void libSMPP_ClientDelete(ESME_HANDLE hClient)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	delete client;
	return ;
}

SMPP_API void libSMPP_ClientSetServerAddress(ESME_HANDLE hClient, const char* serverIP,
                                             short unsigned int serverPort)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	client->SetServerAddress(serverIP);
	client->SetServerPort(serverPort);
}

SMPP_API void libSMPP_ClientSetLoginType(ESME_HANDLE hClient, BindType loginType)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	client->SetBindMode(loginType);
}

SMPP_API void libSMPP_ClientSetSystemId(ESME_HANDLE hClient, const char *systemId)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	client->SetSystemId(systemId);
}

SMPP_API void libSMPP_ClientSetSystemType(ESME_HANDLE hClient, const char *systemType)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	client->SetSystemType(systemType);
}

SMPP_API void libSMPP_ClientSetPassword(ESME_HANDLE hClient, const char *password)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	client->SetPassword(password);
}

SMPP_API void libSMPP_ClientSetAddressRange(ESME_HANDLE hClient, const char *pattern)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	client->SetAddressRange(pattern);
}

SMPP_API void libSMPP_ClientSetMessageSettings(ESME_HANDLE hClient,
                                                  const MessageSettings *ms)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	client->SetMessageSettings(*ms);
}

SMPP_API LoginResult libSMPP_ClientBind(ESME_HANDLE hClient)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	return client->Bind();
}

SMPP_API void libSMPP_ClientUnBind(ESME_HANDLE hClient)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	client->Unbind();
}

SMPP_API int libSMPP_ClientSendKeepAlive (ESME_HANDLE hClient)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	return client->SendKeepAlive() ? 1 : 0;
}

SMPP_API DeliveryResult libSMPP_ClientSendMessage(SMSC_HANDLE hClient,
                                                  const char *from, const char *to,
                                                  const char *content,
                                                  unsigned int size)
{
	CSMPPClient *client = reinterpret_cast<CSMPPClient*>(hClient);
	return client->SendMessage(from, to, string(content, size));
}

} // extern "C"

#ifdef _WIN32
# pragma pop_macro("SendMessage")
#endif
