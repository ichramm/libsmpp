/*!
 * \file smppconnection.hpp
 * \author ichramm
 */
#ifndef OPENSMPP_SMPPCONNECTION_HPP_
#define OPENSMPP_SMPPCONNECTION_HPP_
#pragma once


#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/condition.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>

#define RESULT_OK          0
#define RESULT_TIMEOUT    -1
#define RESULT_NETERROR   -2
#define RESULT_INVRESP    -3
#define RESULT_SYSERROR   -4

namespace opensmpp
{
	class ISMPPCommand;
	class CSMPPConnection;
	typedef boost::shared_ptr<CSMPPConnection>  SMPPConnectionPtr;

	typedef boost::asio::io_service          ioservice_t;
	typedef boost::asio::ip::tcp::socket     socket_t;
	typedef boost::asio::ip::tcp::resolver   resolver_t;

	/*!
	* \brief A user server connection
	*/
	class CSMPPConnection
		: public boost::enable_shared_from_this<CSMPPConnection>
	{
	public:

		typedef boost::function<void (
				SMPPConnectionPtr /*sender*/,
				boost::shared_ptr<ISMPPCommand>
			) > NewCommandCallback;

		typedef boost::function<void (
				SMPPConnectionPtr /*sender*/
			) > ConnectionLostCallback;

		virtual ~CSMPPConnection();

		/*! \return The connection id as specified on construction */
		unsigned int GetConnectionId() const;

		unsigned int NextSequenceNumber();

		/* \brief Sends a SMPP request and wait response */
		int SendRequest(boost::shared_ptr<ISMPPCommand> cmd);

		/* \brief Sends a SMPP response for the given command */
		int SendResponse(boost::shared_ptr<ISMPPCommand> cmd);

		virtual void Close();

		socket_t& socket();

		/* \brief Starts an async read operation */
		void ReadAsync();

	protected:

		CSMPPConnection(
					unsigned int                  connectionId,
					ioservice_t&                  ioservice,
					const NewCommandCallback&     onNewData,
					const ConnectionLostCallback& onConnectionLost
			);

		/*! \brief Invoked when an async read has complete */
		void ReadHandler(const boost::system::error_code& error);

		/*
		* \brief Creates an ISMPPCommand object based on \p commandId, with
		* sequence number \p seqNumber and filled  with the data on \p buffer
		*/
		boost::shared_ptr<ISMPPCommand> CreateCommandFromBuffer(
					unsigned int       commandId,
					unsigned int       seqNumber,
					const std::string& buffer
			);

		/* \brief Sends a SMPP packet, no locking implementation */
		int SendPDU(boost::shared_ptr<ISMPPCommand> cmd, bool response);

		/*! \brief PDU Header: Basic unit of every SMPP packet */
		struct PDUHeader
		{
			unsigned int command_length;  /*!< Length in octets of the SMPP message */
			unsigned int command_id;      /*!< Type of message the SMPP PDU represents */
			unsigned int command_status;  /*!< Indicates the success or failure of an SMPP request */
			unsigned int sequence_number; /*!< Allows a response PDU to be correlated with a request PDU */
		};

		/*! \brief Holds data about a response that the user is expecting due to a call to SendPDU */
		struct PendingResponse
		{
			boost::shared_ptr<ISMPPCommand> command;  /*!< The response packet (header+body) */
			boost::condition *condition;  /*!< Activates when the response is has come */
		};

		typedef std::map<int, PendingResponse> MapPendingResponse;

		unsigned int                   m_connectionId, m_nextSequenceNumber;
		bool                           m_connectionError, m_closeRequested;
		ioservice_t&                   m_ioservice;
		socket_t                       m_socket;
		PDUHeader                      m_pduHeader;
		MapPendingResponse             m_pendingResponses;
		boost::mutex                   m_mutexCounter;
		boost::recursive_mutex         m_mutex;
		NewCommandCallback             m_onNewDataEvent;
		ConnectionLostCallback         m_onConnectionLostEvent;
	};


	/*!
	 *
	 */
	class CSMPPClientConnection
		: public CSMPPConnection
	{
	public:

		CSMPPClientConnection(
					ioservice_t&                  ioservice,
					const NewCommandCallback&     onNewData,
					const ConnectionLostCallback& onConnectionLost
			);

		~CSMPPClientConnection();

		int Connect(const std::string server, unsigned short port);
	};

	/*!
	 *
	 */
	class CSMPPServerConnection : public CSMPPConnection
	{
	public:

		CSMPPServerConnection(
					unsigned int                  connectionId,
					ioservice_t&                  ioservice,
					const NewCommandCallback&     onNewData,
					const ConnectionLostCallback& onConnectionLost
			);

		~CSMPPServerConnection();
	};

} // namespace opensmpp

#endif // OPENSMPP_SMPPCONNECTION_HPP_
