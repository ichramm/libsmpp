/*!
 * \file smppserver.hpp
 * \author ichramm
 */
#ifndef OPENSMPP_SMPPSERVER_HPP_
#define OPENSMPP_SMPPSERVER_HPP_
#if _MSC_VER > 1000
#pragma once
#endif

#include "smppconnection.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <map>


#define LOGIN_OK		     0
#define LOGIN_INV_USER   -1
#define LOGIN_INV_PWD    -2

namespace opensmpp
{
	class ISMPPCommand;
	class CSMPPUserManager;

	typedef boost::asio::ip::tcp::acceptor   acceptor_t;

	/*!
	 *
	 */
	class CSMPPServerImpl
		: public boost::enable_shared_from_this<CSMPPServerImpl>
	{
	public:

		CSMPPServerImpl(boost::shared_ptr<CSMPPUserManager> userManager, unsigned short port);

		~CSMPPServerImpl();

		void Start(unsigned int numThreads = 0);

		void Stop();

		bool IsRunning() const;

	private:
		typedef boost::shared_ptr<boost::thread> ThreadPtr;

		void RunIOService();
		void AcceptConnection();
		void OnNewConnection(boost::shared_ptr<CSMPPConnection> conn, const boost::system::error_code& error);
		void OnNewPDUHandler(SMPPConnectionPtr sender, boost::shared_ptr<ISMPPCommand> cmd);
		void OnConnectionError(SMPPConnectionPtr sender);

		volatile bool            m_running;
		unsigned short           m_port;
		ioservice_t              m_ioservice;
		ioservice_t::work        m_work;
		acceptor_t               m_acceptor; // must be declared after m_ioservice
		std::vector<ThreadPtr>   m_threads;
		size_t                   m_connectionCounter;
		boost::mutex             m_mutex;
		boost::shared_ptr<CSMPPUserManager>        m_userManager;
	};

} // namespace opensmpp

#endif // OPENSMPP_SMPPSERVER_HPP_
