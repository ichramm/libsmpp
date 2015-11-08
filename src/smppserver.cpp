/*!
 * \file smppserver.cpp
 * \author ichramm
 */
#include "stdafx.h"

#ifdef _WIN32
# pragma push_macro("SendMessage")
# undef SendMessage
#endif

#include "smppserver.h"
#include "smppdefs.h"
#include "smppcommands.h"
#include "smppusersmanager.h"
#include "logger.h"
#include <boost/lexical_cast.hpp>
#include <vector>

#ifndef STARTUP_THREAD_COUNT
# define STARTUP_THREAD_COUNT	((unsigned int)20)
#endif

#ifndef MAX_THREAD_COUNT
#define MAX_THREAD_COUNT   ((unsigned int)200)
#endif

using namespace std;
using namespace boost;

namespace libsmpp
{

CSMPPServerImpl::CSMPPServerImpl(shared_ptr<CSMPPUserManager> userManager, unsigned short port)
: m_running(false), m_port(port), m_work(m_ioservice), m_acceptor(m_ioservice), m_connectionCounter(0), m_userManager(userManager)
{
	SMPP_TRACE();
}

CSMPPServerImpl::~CSMPPServerImpl()
{
	SMPP_TRACE();
	Stop();
}

void CSMPPServerImpl::Start(unsigned int numThreads)
{
	SMPP_TRACE();

	if(!numThreads) {
		numThreads = STARTUP_THREAD_COUNT;
	}

	if(numThreads > MAX_THREAD_COUNT) {
		numThreads = MAX_THREAD_COUNT;
	}

	if(!m_acceptor.is_open())
	{
		try {
			asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), m_port);
			asio::ip::tcp::acceptor::reuse_address option(true);
			m_acceptor.open(endpoint.protocol());
			m_acceptor.set_option(option);
			m_acceptor.bind(endpoint);
			m_acceptor.listen();
		}
		catch (boost::system::system_error &e) {
			throw std::runtime_error("Failed to start listening on port " + lexical_cast<string>(m_port) + " - " + e.what());
		}

		// cleanup ioservice's internal flag 'stopped'
		m_ioservice.reset();
	}

	AcceptConnection();

	for (unsigned int i = 0; i < numThreads; i++)
	{
		m_threads.push_back(ThreadPtr(new thread(bind(&CSMPPServerImpl::RunIOService, this))));
	}

	m_running = true;
}

void CSMPPServerImpl::Stop()
{
	SMPP_TRACE();
	if(m_running)
	{
		boost::system::error_code err;
		m_running = false;
		m_acceptor.close(err);
		m_ioservice.stop();
		for ( ; !m_threads.empty(); m_threads.pop_back()) {
			m_threads.back()->join();
		}
	}
}

bool CSMPPServerImpl::IsRunning() const
{
	return m_running;
}

void CSMPPServerImpl::RunIOService()
{
	smpp_log_profile(" -- ioservice thread %#0lx start", (unsigned long int)pthread_self());
	do
	{
		try
		{
			m_ioservice.run();
		}
		catch (std::exception &e)
		{
			smpp_log_error(" == RunIOService Error: %s", e.what());
		}
	}
	while (m_running); // keep running if there is an error
	smpp_log_profile(" -- ioservice thread %#0lx end", (unsigned long int)pthread_self());
}

void CSMPPServerImpl::AcceptConnection()
{
	SMPP_TRACE();
	lock_guard<mutex> lock(m_mutex);
	SMPPConnectionPtr conn(new CSMPPServerConnection(++m_connectionCounter, m_ioservice,
											bind(&CSMPPServerImpl::OnNewPDUHandler, shared_from_this(), _1, _2),
											bind(&CSMPPServerImpl::OnConnectionError, shared_from_this(), _1)));
	m_acceptor.async_accept(conn->socket(), bind(&CSMPPServerImpl::OnNewConnection, this, conn, asio::placeholders::error));
}

void CSMPPServerImpl::OnNewConnection(SMPPConnectionPtr conn, const boost::system::error_code& error)
{
	SMPP_TRACE();
	if(!error)
	{
		// accept new connections before
		// just in case the server connection runs synchronously
		AcceptConnection();

		// let the connection start reading
		conn->ReadAsync();
	}
	else
	{
		smpp_log_warning("OnNewConnection: ERROR %s", error.message().c_str());
	}
}


void CSMPPServerImpl::OnNewPDUHandler(SMPPConnectionPtr sender, shared_ptr<ISMPPCommand> cmd)
{
	SMPP_TRACE();
	if(!m_userManager->OnCommand(sender, cmd))
	{ // close the connection but with some delay
		this_thread::sleep(boost::posix_time::milliseconds(500));
		smpp_log_profile("Closing connection %u", sender->GetConnectionId());
		sender->Close();
	}
};

void CSMPPServerImpl::OnConnectionError(SMPPConnectionPtr sender)
{
	SMPP_TRACE();
	lock_guard<mutex> lock(m_mutex);
	sender->Close();
	m_userManager->OnCommand(sender, shared_ptr<ISMPPCommand>());
}

} // namespace libsmpp

#ifdef _WIN32
# pragma pop_macro("SendMessage")
#endif
