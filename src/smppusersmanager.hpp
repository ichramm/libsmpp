/*!
 * \file smppusersmanager.hpp
 * \author ichramm
 */
#ifndef OPENSMPP_SMPPUSERSMANAGER_HPP__
#define OPENSMPP_SMPPUSERSMANAGER_HPP__
#if _MSC_VER > 1000
#pragma once
#endif

#include "../smpp.hpp"
#include "smppconnection.hpp"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <map>

namespace opensmpp
{
	class ISMPPCommand;
	class CSMPPCallback;
	class SMPPUser;

	/*!
	*\brief Holds the list of users connected to this server
	* Manages keep alive and other stuff
	*/
	class CSMPPUserManager
		: public boost::enable_shared_from_this<CSMPPUserManager>
	{
	public:
		CSMPPUserManager(boost::shared_ptr<CSMSCCallback> callbacks);

		~CSMPPUserManager(void);

		bool OnCommand (
				boost::shared_ptr<CSMPPConnection> conn,
				boost::shared_ptr<ISMPPCommand>    cmd
			);

		void SetDeliveryEncoding( DataCoding data_coding );

		DeliveryResult SendMessage (
				const std::string& from,
				const std::string& to,
				const std::string& message
			);

	private:

		typedef boost::shared_ptr<SMPPUser> UserRef;

		DataCoding                       m_encoding;
		boost::mutex                     m_mutex;
		boost::thread                    m_threadKeepAlive;
		boost::condition                 m_exitEvent;
		boost::condition                 m_newClientCondition;
		std::map<int, UserRef>           m_clients;
		boost::shared_ptr<CSMSCCallback> m_callbacks;

		// ENQUIRE_LINK
		void KeepAliveThread();
	};
} // namespace opensmpp

#endif // OPENSMPP_SMPPUSERSMANAGER_HPP__
