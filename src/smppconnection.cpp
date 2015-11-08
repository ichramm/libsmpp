/*!
 * \file smppconnection.cpp
 * \author ichramm
 */
#include "stdafx.h"

#include "smppconnection.h"
#include "smppcommands.h"
#include "logger.h"
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <cstdlib>

//#define DISABLE_SMPP_BUFFER_DUMP

#if !defined(DUMP_BUFFER_SIZE)
 #define DUMP_BUFFER_SIZE 8192
#endif

#if GCC_VERSION > 0
 __thread char __dump_buffer[DUMP_BUFFER_SIZE];
 #define DUMP_SMPP_PDU(connId, cmdId, pduptr, msg) do { \
 	if (0 == smpp34_dumpPdu( cmdId, (uint8_t *)__dump_buffer, DUMP_BUFFER_SIZE-1, pduptr)) \
 		fprintf(stderr, "Connection %d - %s:\n%s\n", connId, msg, __dump_buffer); \
 } while(0)

 #define DUMP_SMPP_BUFFER(connId, msg, inbuff, inbufflen) do { \
	if (0 == smpp34_dumpBuf((uint8_t *)__dump_buffer, DUMP_BUFFER_SIZE-1, (uint8_t *)inbuff, inbufflen)) \
		fprintf(stderr, "Connection %d - %s:\n%s\n", connId, msg, __dump_buffer); \
 } while (0)

#elif defined(_WIN32)
 extern DWORD dwTlsIndexDumpBuffer;
 #define ENSURE_BUFFER() \
	char *__dump_buffer = (char *)TlsGetValue(dwTlsIndexDumpBuffer); \
	if (__dump_buffer == NULL) { \
 		__dump_buffer = (char *)LocalAlloc(LPTR, DUMP_BUFFER_SIZE); \
 		TlsSetValue(dwTlsIndexDumpBuffer, (LPVOID)__dump_buffer); \
 	}
 #define DUMP_SMPP_PDU(connId, cmdId, pduptr, msg) do { \
	ENSURE_BUFFER() \
 	if (0 == smpp34_dumpPdu( cmdId, (uint8_t *)__dump_buffer, DUMP_BUFFER_SIZE-1, pduptr)) \
 		fprintf(stderr, "Connection %d - %s:\n%s\n", connId, msg, __dump_buffer); \
 } while (0)

 #define DUMP_SMPP_BUFFER(connId, msg, inbuff, inbufflen) do { \
	ENSURE_BUFFER() \
	if (0 == smpp34_dumpBuf((uint8_t *)__dump_buffer, DUMP_BUFFER_SIZE-1, (uint8_t *)inbuff, inbufflen)) \
		fprintf(stderr, "Connection %d - %s:\n%s\n", connId, msg, __dump_buffer); \
 } while (0)

#endif

// default response timeout
#ifndef RESPONSE_TIMEOUT
#define RESPONSE_TIMEOUT ((unsigned int)40)
#endif

// defines a range for initial randomized sequence numbers
#ifndef SEQ_NUM_INITIAL_RANGE
#define SEQ_NUM_INITIAL_RANGE   0x00004000
#endif

// creates a randomized number from 1 to SEQ_NUM_INITIAL_RANGE
#define INITIAL_SEQ_NUMBER()  ((unsigned int)(rand() % SEQ_NUM_INITIAL_RANGE + 1))


using namespace std;
using namespace boost;

namespace libsmpp
{


CSMPPConnection::CSMPPConnection(unsigned int connectionId, ioservice_t &ioservice,
 const NewCommandCallback& onNewData, const ConnectionLostCallback& onConnectionLost)
: m_connectionId(connectionId), m_nextSequenceNumber(INITIAL_SEQ_NUMBER()),
  m_connectionError(false), m_closeRequested(false), m_ioservice(ioservice), m_socket(m_ioservice),
  m_onNewDataEvent(onNewData), m_onConnectionLostEvent(onConnectionLost)
{
	SMPP_TRACE();
}

CSMPPConnection::~CSMPPConnection()
{
	SMPP_TRACE();
	Close();
}

socket_t& CSMPPConnection::socket()
{
	return m_socket;
}

unsigned int CSMPPConnection::GetConnectionId() const
{
	return m_connectionId;
}

unsigned int CSMPPConnection::NextSequenceNumber()
{
	SMPP_TRACE();
	lock_guard<recursive_mutex> lock(m_mutex);

	m_nextSequenceNumber++;

	if (m_nextSequenceNumber == UINT_MAX)
	{
		m_nextSequenceNumber = INITIAL_SEQ_NUMBER();
	}

	return m_nextSequenceNumber;
}

void CSMPPConnection::Close()
{
	lock_guard<recursive_mutex> lock(m_mutex);
	if (m_closeRequested) {
		return;
	}

	smpp_log_profile("Connection %u: Closing socket", m_connectionId);
	m_closeRequested = true;
	if(m_socket.is_open()) {
		boost::system::error_code err;
		m_socket.close(err);
	}
	m_connectionError = false;
	m_pendingResponses.clear();
}

void CSMPPConnection::ReadAsync()
{
	SMPP_TRACE();
	asio::async_read(socket(), asio::buffer(&m_pduHeader, sizeof(PDUHeader)),
			bind(&CSMPPConnection::ReadHandler, shared_from_this(), asio::placeholders::error)
		);
}

int CSMPPConnection::SendRequest(shared_ptr<ISMPPCommand> cmd)
{
	SMPP_TRACE();

	recursive_mutex::scoped_lock lock(m_mutex);

	int res = SendPDU(cmd, false);
	if(res != RESULT_OK)
	{
		smpp_log_warning("Connection %u: Failed to send PDU of type %#X", m_connectionId, cmd->request_id());
		return res;
	}

	PendingResponse respdata = {cmd, new condition()};
	m_pendingResponses[cmd->sequence_number()] = respdata;

	bool timed_out = !respdata.condition->timed_wait(lock, posix_time::seconds(RESPONSE_TIMEOUT));

	m_pendingResponses.erase(cmd->sequence_number());
	delete respdata.condition;

	if(timed_out) {
		return RESULT_TIMEOUT;
	}
	if(m_connectionError) {
		return RESULT_NETERROR;
	}
	if(-1 == cmd->command_status()) {
		return RESULT_INVRESP;
	}
	return RESULT_OK;
}

int CSMPPConnection::SendResponse(shared_ptr<ISMPPCommand> cmd)
{
	SMPP_TRACE();
	lock_guard<recursive_mutex> lock(m_mutex);
	return SendPDU(cmd, true);
}

int CSMPPConnection::SendPDU(shared_ptr<ISMPPCommand> cmd, bool response)
{
	SMPP_TRACE();
	if(!socket().is_open())
	{
		return RESULT_NETERROR;
	}

	unsigned int (ISMPPCommand::*pack_fn)(char*, unsigned int, int&);

	if(response)
	{ // select the proper function and dump the response pdu
		pack_fn = &ISMPPCommand::pack_response;
		DUMP_SMPP_PDU(m_connectionId, cmd->response_id(), cmd->response_ptr(), "Sending PDU");
	}
	else
	{
		pack_fn = &ISMPPCommand::pack_request;
		DUMP_SMPP_PDU(m_connectionId, cmd->request_id(), cmd->request_ptr(), "Sending PDU");
	}

	int err;
	std::vector<char> buffer(256);
	unsigned int len = ((*cmd).*pack_fn)(&buffer[0], buffer.size(), err);
	while(-1 == err && buffer.size() < (2 << 11) ) // 4096 bytes max
	{
		buffer.resize(buffer.size()*2);
		len = ((*cmd).*pack_fn)(&buffer[0], buffer.size(), err);
	}

	if(err == -1)
	{
		smpp_log_error("Connection %u: Failed to unpack pDU", m_connectionId);
		return RESULT_SYSERROR;
	}

	if (len < buffer.size())
	{ // len cannot be bigger than size()
		buffer.resize(len);
	}

	try
	{
		asio::write(socket(), asio::buffer(&buffer[0], len));
		return RESULT_OK;
	}
	catch (const std::exception& e)
	{
		smpp_log_error("Failed to call asio::write: %s", e.what());
		return RESULT_NETERROR;
	}
}

void CSMPPConnection::ReadHandler(const boost::system::error_code& error)
{
	smpp_log_warning("Connection %u", m_connectionId);
	recursive_mutex::scoped_lock lock(m_mutex);

	if(error)
	{
		m_connectionError = true;

		MapPendingResponse::iterator it = m_pendingResponses.begin();
		for ( ; it != m_pendingResponses.end(); it++)
		{
			it->second.command->command_status(-1);
			it->second.condition->notify_one();
		}

		if (!m_closeRequested)
		{
			smpp_log_warning("Connection %u: Network error: %s", m_connectionId, error.message().c_str());
			try
			{
				Close();
				lock.unlock();
				m_onConnectionLostEvent(shared_from_this());
			}
			catch (const std::exception &e)
			{
				smpp_log_warning("Connection %u: Error cleaning up: %s", m_connectionId, e.what());
			}
		}
		return;
	}

	unsigned int commandId = ntohl(m_pduHeader.command_id);
	unsigned int seqNumber = ntohl(m_pduHeader.sequence_number);
	unsigned int commandLength = ntohl(m_pduHeader.command_length);

	try
	{
		std::string pdu((char *)&m_pduHeader, sizeof(PDUHeader));

		if (commandLength > sizeof(PDUHeader))
		{ // read PDU body
			pdu.resize(commandLength, 0);
			asio::read(socket(), asio::buffer(&pdu[0]+sizeof(PDUHeader), pdu.size() - sizeof(PDUHeader)));
		}

		DUMP_SMPP_BUFFER(m_connectionId, "Read buffer", &pdu[0], pdu.size());

		if (commandId & SMPP_RESPONSE_BIT)
		{ // response packet
			if(m_pendingResponses.count(seqNumber))
			{ // someone is waiting for this packet (to be honest, there are not so many people using this API, but is good for self-esteem)
				int err;
				shared_ptr<ISMPPCommand> cmd = m_pendingResponses[seqNumber].command;

				if( (commandId & SUBMIT_SM) == SUBMIT_SM)
				{ // Handle non-standard response PDU's
					size_t nullPos = pdu.find_first_of('\0', SMPP_HEADER_SIZE); // look for the first NULL starting after the header
					if(nullPos != string::npos && nullPos < pdu.size()-1)
					{ // make sure message_id is the last field
						pdu.resize(nullPos+1, 0);
						uint32_t tmp = htonl(pdu.size());
						memcpy(&pdu[0], &tmp, sizeof(uint32_t));
					}
				}

				cmd->unpack_response(&pdu[0], pdu.size(), err);
				if (err)
				{ // this should not happen, I suppose...
					cmd->command_status(-1);
				}
				else
				{ // ok, now we are ready to see the packet in a human readable format
					DUMP_SMPP_PDU(m_connectionId, cmd->response_id(), cmd->response_ptr(), "Read PDU");
				}
				// tell the guy on the door that his response has come...
				m_pendingResponses[seqNumber].condition->notify_one();
			}
			else
			{ // ups, what is going on here?
				smpp_log_warning("Connection %u: Unexpected PDU response: cmd[%#x], seqnumber[%d]", m_connectionId, commandId, seqNumber);
			}
		}
		else if(m_onNewDataEvent)
		{ // new packet
			shared_ptr<ISMPPCommand> cmd = CreateCommandFromBuffer(commandId, seqNumber, pdu);
			if(cmd)
			{
				DUMP_SMPP_PDU(m_connectionId, cmd->request_id(), cmd->request_ptr(), "Read PDU");

				// We should not wait until the callback returns, must start reading before that
				ReadAsync();

				lock.unlock();

				m_onNewDataEvent(shared_from_this(), cmd);

				return; // we dont want to call ReadAsync() twice!
			}
			else
			{ // failed to unpack the buffer? You gotta be kidding me!
				shared_ptr<ISMPPCommand> cmd(new CSMPPGenericNack(seqNumber));
				SendPDU(cmd, true);
			}
		}
		else
		{ // no handler? this is evil... ok, send a NO-ACK and forget about it
			shared_ptr<ISMPPCommand> cmd(new CSMPPGenericNack(seqNumber));
			SendPDU(cmd, true);
		}

		// keep reading!
		ReadAsync();
	}
	catch (const std::exception &e)
	{ // I think the only one who could throw exception is asio::read, but you can never be sure ...
		if (!m_closeRequested)
		{ // if the connection was closed the read fails, so ignore this error
			smpp_log_warning("Connection %u: Something failed while reading, the exception message is: %s", m_connectionId, e.what());
			if (commandId & SMPP_RESPONSE_BIT)
			{
				m_pendingResponses[seqNumber].command->command_status(-1);
				m_pendingResponses[seqNumber].condition->notify_one();
			}
			Close();
		}
	}
}


shared_ptr<ISMPPCommand> CSMPPConnection::CreateCommandFromBuffer(unsigned int commandId, unsigned int seqNumber, const std::string &buffer)
{
	shared_ptr<ISMPPCommand> res;
	switch (commandId)
	{
	case BIND_RECEIVER:
		res.reset(new CBindReceiver(seqNumber));
		break;
	case BIND_TRANSMITTER:
		res.reset(new CBindTransmitter(seqNumber));
		break;
	case BIND_TRANSCEIVER:
		res.reset(new CBindTransceiver(seqNumber));
		break;
	case UNBIND:
		res.reset(new CSMPPUnbind(seqNumber));
		break;
	case SUBMIT_SM:
		res.reset(new CSMPPSubmitSingle(seqNumber));
		break;
	case ENQUIRE_LINK:
		res.reset(new CSMPPEnquireLink(seqNumber));
		break;
	case DELIVER_SM:
		res.reset(new CSMPPDelivery(seqNumber));
		break;
	case QUERY_SM:
	case REPLACE_SM:
	case CANCEL_SM:
	case SUBMIT_MULTI:
	default:
		res.reset(new CSMPPGenericNack(seqNumber));
		break;
	}

	if(res->request_id() != GENERIC_NACK)
	{
		int err;
		res->unpack_request(&buffer[0], buffer.size(), err);
		if(err)
		{ // sorry, it didn't work
			res.reset();
		}
	}

	return res;
}

/************************************************************************/
/************************************************************************/
static volatile unsigned int g_clientConnectionCounter = 1000;

CSMPPClientConnection::CSMPPClientConnection(ioservice_t& ioservice,
 const NewCommandCallback& onNewData, const ConnectionLostCallback& onConnectionLost)
: CSMPPConnection(++g_clientConnectionCounter, ioservice, onNewData, onConnectionLost)
{
	SMPP_TRACE();
}

int CSMPPClientConnection::Connect( const std::string server, unsigned short port )
{
	resolver_t resolver(m_ioservice);
	resolver_t::query query(server, lexical_cast<std::string>(port));
	resolver_t::iterator endpoint_iterator = resolver.resolve(query);
	resolver_t::iterator end;
	boost::system::error_code error = asio::error::host_not_found;

	smpp_log_info("Connection %u: Connecting to server %s:%d", m_connectionId, server.c_str(), port);

	while (error && endpoint_iterator != end)
	{
		socket().close();
		socket().connect(*endpoint_iterator++, error);
	}

	if (error)
	{
		smpp_log_warning("Connection %u: Cannot connect to host %s:%d: %s", m_connectionId, server.c_str(), port, error.message().c_str());
		return -1;
	}

	m_connectionError = false;
	m_closeRequested = false;
	ReadAsync();

	return 0;
}

CSMPPClientConnection::~CSMPPClientConnection()
{
	SMPP_TRACE();
}

/************************************************************************/
/************************************************************************/

CSMPPServerConnection::CSMPPServerConnection(unsigned int connectionId, ioservice_t &ioservice,
 const NewCommandCallback& onNewData, const ConnectionLostCallback& onConnectionLost)
: CSMPPConnection(connectionId, ioservice, onNewData, onConnectionLost)
{
	SMPP_TRACE();
}

CSMPPServerConnection::~CSMPPServerConnection()
{
	SMPP_TRACE();
}

} // namespace libsmpp
