/*!
 * \file smppusersmanager.cpp
 * \author ichramm
 *
 * User management and PDU processing
 */
#include "stdafx.h"

#ifdef _WIN32
# pragma push_macro("SendMessage")
# undef SendMessage
#endif

#include "smppusersmanager.hpp"
#include "smppcommands.hpp"
#include "iconv/gsm7.h"
#include "converter.hpp"
#include "logger.h"

#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <vector>


#define KEEP_ALIVE_TIMEOUT  50

using namespace std;
using namespace boost;

namespace opensmpp
{

static BindType bindtype_to_logintype(int cmd)
{
	if(cmd == BIND_RECEIVER){
		return BIND_TYPE_RECEIVER;
	}
	if(cmd == BIND_TRANSMITTER){
		return BIND_TYPE_TRANSMITTER;
	}
	if(cmd == BIND_TRANSCEIVER){
		return BIND_TYPE_TRANSCEIVER;
	}
	return (BindType)-1;
}

static string ConvertTextToUTF8(uint8_t data_coding, const string &text) {
	const char *charset = CConverter::GetCharsetFromDataCoding(data_coding);
	CConverter conv(charset, "UTF-8");
	string result;
	if (conv.Convert(text, result)) {
		// if conversion fails send text as is
		result = text;
	}
	if(!strcmp(charset, "GSM7")) {
		// we will not support GSM 7-bit packing...
	}
	return result;
}
/*
static string ConvertTextToGSM7(const string &utf8String)
{
	string result;
	CConverter c("UTF-8", "GSM7");
	c.Convert(utf8String, result);
	return result;
}
*/

/*!
 * Base class for the different kinds of addresses
 */
class UserAddress
{
public:
	virtual bool matches(const std::string& address) = 0;
};

/*!
 * Represents an explicit address
 */
class ExplicitUserAddress : public UserAddress
{
public:
	ExplicitUserAddress(const std::string& address)
		: m_address(address)
	{ }

	bool matches(const std::string& address)
	{
		return (address == m_address);
	}

private:
	std::string m_address;
};

/*!
 * Represents a range of addresses
 */
class RangeUserAddress : public UserAddress
{
public:
	RangeUserAddress(const std::string& range_start, const std::string& range_end)
	 : m_addressStart(range_start), m_addressEnd(range_end)
	{
		m_startInt = atoi(m_addressStart.c_str());
		m_endInt = atoi(m_addressEnd.c_str());
	}

	bool matches(const std::string& address)
	{
		if (address.size() < m_addressStart.size() || address.size() > m_addressEnd.size())
		{ // early validation to make sure boundaries are fine
			return false;
		}

		char *first_not_number = NULL;
		int address_int = (int)strtol(address.c_str(), &first_not_number, 10);
		if (first_not_number && *first_not_number)
		{ // not a valid number
			return 0;
		}

		return ((address_int >= m_startInt) && (address_int <= m_endInt));
	}

private:
	std::string m_addressStart;
	std::string m_addressEnd;
	int         m_startInt;
	int         m_endInt;
};

/*!
 * A connected SMPP user
 */
class SMPPUser
{
public:
	SMPPUser() : errCount(0), lastKeepAlive(0) {}

	bool ownsAddress(const std::string& address)
	{
		vector<shared_ptr<UserAddress> >::const_iterator it, end;
		for (it = m_addresses.begin(), end = m_addresses.end(); it != end; it++)
		{
			const boost::shared_ptr<UserAddress>& current_address = *it;
			if ( current_address->matches(address) )
			{
				return true;
			}
		}
		return false;
	}

	void addAddress(boost::shared_ptr<UserAddress> address)
	{
		m_addresses.push_back(address);
	}

	std::vector<boost::shared_ptr<UserAddress> > m_addresses;


	SMPPConnectionPtr         connection;
	int                       bindMode;
	std::string               systemId;
	unsigned int              errCount;
	volatile time_t           lastKeepAlive;
};



CSMPPUserManager::CSMPPUserManager(shared_ptr<CSMSCCallback> callbacks)
 : m_encoding(DATA_CODING_UTF8)
 , m_callbacks(callbacks)
{
	m_threadKeepAlive = thread(&CSMPPUserManager::KeepAliveThread, this);
}

CSMPPUserManager::~CSMPPUserManager(void)
{
	//mutex::scoped_lock lock(m_mutex);
	m_newClientCondition.notify_one();
	m_exitEvent.notify_one();
	m_threadKeepAlive.join();
}

bool CSMPPUserManager::OnCommand(shared_ptr<CSMPPConnection> conn, shared_ptr<ISMPPCommand> cmd)
{
	UserRef user;
	int status = ESME_RSYSERR;
	unsigned int connectionId = conn->GetConnectionId();

	mutex::scoped_lock lock(m_mutex);

	if(!cmd)
	{ // network error
		if(m_clients.count(connectionId))
		{ // if this is not true then the user has unbound it self
			m_callbacks->OnUserDisconnected(connectionId, m_clients[connectionId]->systemId, DISCONNECT_REASON_NETERROR);
			m_clients.erase(connectionId);
		}

		return false; // the connection is already closed
	}

	if (!m_clients.count(connectionId))
	{
		if (cmd->request_id() != BIND_RECEIVER && cmd->request_id() != BIND_TRANSMITTER && cmd->request_id() != BIND_TRANSCEIVER)
		{
			status = ESME_RINVBNDSTS;
		}
		else if(m_callbacks)
		{
			ISMPPBind *cmdBind = dynamic_cast<ISMPPBind*>(cmd.get());
			user.reset(new SMPPUser);
			user->connection = conn;
			user->systemId = cmdBind->getSystemId();
			user->bindMode = cmd->request_id();

			string user_addresses = cmdBind->getAddressRange();
			if(user->bindMode != BIND_TRANSMITTER)
			{
				if (user_addresses.empty())
				{
					status = ESME_RINVCMDID;
					goto send_response;
				}

				vector<string> address_set; // user set of addresses: 2000-2999|4000-2999
				algorithm::split(address_set, user_addresses, algorithm::is_any_of("|"));
				for (vector<string>::iterator it = address_set.begin(); it != address_set.end(); it++)
				{
					shared_ptr<UserAddress> address;

					vector<string> address_range;
					algorithm::split(address_range, *it, algorithm::is_any_of("-"));

					if (address_range.size() == size_t(1))
					{ // simple address, put it back
						address = make_shared<ExplicitUserAddress>(*it);
					}
					else if (address_range.size() == size_t(2))
					{ // numeric address range: e.g. 2000-2010
						const string& address_start = address_range[0];
						const string& address_end = address_range[1];

						bool invalid_range = (address_start > address_end);
						if (!invalid_range)
						{
							try
							{
								lexical_cast<int>(address_start);
								lexical_cast<int>(address_end);
							}
							catch (const std::exception&)
							{
								invalid_range = true;
							}
						}

						if (invalid_range)
						{
							status = ESME_RINVCMDID;
							goto send_response;
						}

						address = make_shared<RangeUserAddress>(address_start, address_end);
					}

					if(!address)
					{
						smpp_log_info("Rejecting connection %u because address range [%s] is empty  or invalid",
							connectionId, user_addresses.c_str());
						status = ESME_RINVCMDID;
						goto send_response;
					}

					user->addAddress(address);
				}
			}

			// this is a big move... will be fine if I release the lock here?
			// should be...
			lock.unlock();

			LoginResult res = m_callbacks->ValidateUser(connectionId, bindtype_to_logintype(cmd->request_id()),
					user->systemId, cmdBind->getPasssword(), user_addresses
				);

			// But I must acquire the lock as soon as the callback ends
			// A lot can happen while the callback has the control flow,
			// but, for this client, no request will be attended
			lock.lock();

			switch(res)
			{
			case LOGIN_RESULT_OK:
				smpp_log_info("BIND request on connection %u accepted", connectionId);
				status = ESME_ROK;
				cmdBind->setResponseSystemId("SMSC");
				break;
			case LOGIN_RESULT_INVALIDPWD:
				smpp_log_info("Rejecting BIND request on connection %u due to invalid password", connectionId);
				status = ESME_RINVPASWD;
				break;
			case LOGIN_RESULT_INVALIDUSR:
				smpp_log_info("Rejecting BIND request on connection %u due to invalid systemId", connectionId);
				status = ESME_RINVSYSID;
				break;
			case LOGIN_RESULT_INVALIDCMD:
				smpp_log_info("Rejecting BIND request on connection %u due to... invalid command? really? that's my job dude!", connectionId);
				status = ESME_RINVCMDID;
			default: // LOGIN_RESULT_INALIDADDR or LOGIN_RESULT_FAIL
				smpp_log_info("Rejecting BIND request on connection %u due to some user-specific error", connectionId);
				status = ESME_RBINDFAIL;
				break;
			}
		}

		if(status == ESME_ROK)
		{
			m_clients.insert(make_pair(connectionId, user));
			m_newClientCondition.notify_one();
		}

		// we dont need the lock anymore...
		lock.unlock();

send_response:
		// set and send the response status
		cmd->command_status(status);
		conn->SendResponse(cmd);

		return (status == ESME_ROK);
	}

	if (cmd->request_id() == BIND_RECEIVER || cmd->request_id() == BIND_TRANSMITTER || cmd->request_id() == BIND_TRANSCEIVER)
	{ // already bound
		cmd->command_status(ESME_RALYBND);
		conn->SendResponse(cmd);
		return true;
	}

	// get the user instance
	user = m_clients[connectionId];

	if (cmd->request_id() == UNBIND)
	{
		smpp_log_profile(" ==>> UNBIND(%u)", connectionId);

		m_clients.erase(connectionId);
		lock.unlock();

		if(m_callbacks)
		{
			m_callbacks->OnUserDisconnected(connectionId, user->systemId, DISCONNECT_REASON_UNBIND);
		}

		cmd->command_status(ESME_ROK);
		conn->SendResponse(cmd);
		return false;
	}

	if(cmd->request_id() == SUBMIT_SM)
	{
		lock.unlock();

		smpp_log_profile(" ==>> SUBMIT_SM");

		if (user->bindMode == BIND_RECEIVER)
		{ // only transmitter and transceiver can send messages
			cmd->command_status(ESME_RINVCMDID);
			conn->SendResponse(cmd);
			return true;
		}

		shared_ptr<CSMPPSubmitSingle> cmdSubmit = dynamic_pointer_cast<CSMPPSubmitSingle>(cmd);
		if(!cmdSubmit)
		{ // this should never happen!
			cmd->command_status(ESME_RUNKNOWNERR);
			conn->SendResponse(cmd);
			return true;
		}

		string from = cmdSubmit->getSourceAddress();
		string to = cmdSubmit->getDestinationAddress();

		if (user->bindMode != BIND_TRANSMITTER)
		{ // check user addresses for conflict
			if (!user->ownsAddress(from))
			{ // user does not own the addres is sending from
				cmd->command_status(ESME_RINVSRCADR);
				conn->SendResponse(cmd);
				return true;
			}
			else if (user->ownsAddress(to))
			{ // user is sending a message to it self
				cmd->command_status(ESME_RINVDSTADR);
				conn->SendResponse(cmd);
				return true;
			}
		}

		string text = ConvertTextToUTF8(cmdSubmit->request().data_coding, cmdSubmit->getText());

		if (m_callbacks)
		{ // callbacks are set only once, it's safe to check/call here
			DeliveryResult res = m_callbacks->DeliverMessage(connectionId, from, to, text);
			switch (res)
			{
				case DELIVERY_OK:
					status = ESME_ROK;
					break;
				case DELIVERY_REJECTED:
					status = ESME_RTHROTTLED;
					break;
				case DELIVERY_INV_SRC_ADDR:
					status = ESME_RINVSRCADR;
					break;
				case DELIVERY_INV_DEST_ADDR:
					status = ESME_RINVDSTADR;
					break;
				case DELIVERY_UNKNOWN_ERROR:
				default:
					status = ESME_RSYSERR;
					break;
			}
		}
		else
		{
			cmd.reset(new CSMPPGenericNack(cmd->sequence_number()));
			status = ESME_ROK;
		}

		cmd->command_status(status);
		conn->SendResponse(cmd);

		return true;
	}

	if (cmd->request_id() == ENQUIRE_LINK)
	{
		user->lastKeepAlive = time(NULL);

		lock.unlock();
		smpp_log_profile(" ==>> ENQUIRE_LINK");
		cmd->command_status(ESME_ROK);
		conn->SendResponse(cmd);
		return true;
	}

	lock.unlock();
	cmd->command_status(ESME_RINVCMDID);
	conn->SendResponse(cmd);
	return true;
}


void CSMPPUserManager::SetDeliveryEncoding(DataCoding data_coding)
{
	lock_guard<mutex> lock(m_mutex);
	m_encoding = data_coding;
}

DeliveryResult CSMPPUserManager::SendMessage(const string &from,  const string &to, const string &message)
{
	bool bFound = false;
	SMPPConnectionPtr conn;

	{
		lock_guard<mutex> lock(m_mutex);
		map<int, UserRef>::iterator it, end;
		for (it = m_clients.begin(), end = m_clients.end(); !bFound && it != end; it++)
		{
			if (it->second->ownsAddress(to))
			{
				bFound = true;
				conn = it->second->connection;
				break;
			}
		}
	}

	if(!bFound)
	{
		return DELIVERY_INV_DEST_ADDR;
	}

	shared_ptr<CSMPPDelivery> cmd(new CSMPPDelivery(conn->NextSequenceNumber()));

	cmd->setSourceAddress(from, TON_UNKNOWN, NPI_UNKNOWN);
	cmd->setDestination(to);

	string encoded_message;
	CConverter c("UTF-8", CConverter::GetCharsetFromDataCoding(m_encoding));
	c.Convert(message, encoded_message);
	// TODO: Create a setter
	cmd->request().data_coding = m_encoding;
	cmd->setText(encoded_message);

	int res = conn->SendRequest(cmd->shared_from_this());

	if(res != RESULT_OK)
	{
		return DELIVERY_UNKNOWN_ERROR;
	}

	if(cmd->command_status() != ESME_ROK)
	{
		return DELIVERY_REJECTED;
	}

	return DELIVERY_OK;
}


void CSMPPUserManager::KeepAliveThread()
{
	mutex::scoped_lock lock(m_mutex);
	while(true)
	{
		if(m_clients.empty())
		{ // wait until a new client is connected
			m_newClientCondition.wait(lock);

			if (m_clients.empty())
			{ // stopping
				break;
			}
		}

		// wait KEEP_ALIVE_TIMEOUT seconds until sending the first keep alive
		if(m_exitEvent.timed_wait(lock, posix_time::seconds(KEEP_ALIVE_TIMEOUT)))
		{
			break;
		}

		vector<UserRef> users;
		users.reserve(m_clients.size());
		for (map<int, UserRef>::iterator mit = m_clients.begin(); mit != m_clients.end(); mit++) {
			users.push_back(mit->second);
		}

		lock.unlock();

		shared_ptr<ISMPPCommand> cmd;
		for (vector<UserRef>::iterator it = users.begin(); it != users.end(); it++)
		{ // send keep alive for each user
			UserRef user = *it;

			if(user->lastKeepAlive && user->lastKeepAlive > time(NULL) - KEEP_ALIVE_TIMEOUT )
			{
				continue;
			}

			try
			{
				cmd.reset(new CSMPPEnquireLink(user->connection->NextSequenceNumber()));

				smpp_log_profile(" == ENQUIRE_LINK (%s)", user->systemId.c_str());
					int res = user->connection->SendRequest(cmd);
				smpp_log_profile(" == ENQUIRE_LINK (%s) done with result %d", user->systemId.c_str(), res);

				if ( (res != RESULT_OK && res != RESULT_SYSERROR) || cmd->command_status() != ESME_ROK)
				{ // if the ENQUIRE_LINK command fails you better close the connection
					smpp_log_info(" == ENQUIRE_LINK to %s failed, closing connection", user->systemId.c_str());
					user->connection->Close();

					lock.lock();
					m_clients.erase(user->connection->GetConnectionId());
					lock.unlock();
				}
				else
				{
					user->lastKeepAlive = time(NULL);
				}
			}
			catch (const std::exception &e)
			{
				smpp_log_warning(" == ERROR on CSMPPUserManager::KeepAliveThread: %s", e.what());
			}
		}

		lock.lock();
	}
}

} // namespace opensmpp

#ifdef _WIN32
# pragma pop_macro("SendMessage")
#endif
