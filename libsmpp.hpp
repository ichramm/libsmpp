/*!
 * \file libsmpp.hpp
 * \author ichramm
 * 
 * This header exposes the C++ API
 */
#ifndef libsmpp_hpp__
#define libsmpp_hpp__
#if _MSC_VER > 1000
#pragma once
#endif

#ifdef _WIN32
# pragma push_macro("SendMessage")
# undef SendMessage
#endif

#if _MSC_VER > 1000
# pragma warning(push)	// disable for this header only
# pragma warning(disable: 4251)  // warning C4251: class 'C' needs to have dll-interface to be used by clients of class 'Y'
#endif

// include the C API
#include "libsmpp.h"

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <vector>

namespace libsmpp
{
	/*!
	* \brief This class must be implemented by the user, each method speaks by it self
	*/
	class SMPP_API CSMSCCallback
		: public boost::enable_shared_from_this<CSMSCCallback>
	{
	public:

		/*!
		* \brief Validate a user login
		* \param connectionId Uniquely identifies the user's connection
		* \param loginType Type of login, as explained on \c LoginType
		* \param systemId The username
		* \param password That
		* \param address_range vector holding the phone numbers that the user handles (or wants to handle)
		*/
		virtual LoginResult ValidateUser (
					unsigned int       connectionId,
					BindType           loginType,
					const std::string& systemId,
					const std::string& password,
					const std::string& address_range
			) = 0;


		/*!
		* \brief A new message from a client ESME has come and we should deliver it
		* \param connectionId The is of the connection who sent this message
		* \param from The source address of the message (set by the ESME)
		* \param to The destination address of the message (aka: The phone number)
		* \param message The text body of the message, encoded in UTF-8
		*/
		virtual DeliveryResult DeliverMessage (
					unsigned int       connectionId,
					const std::string& from,
					const std::string& to,
					const std::string& message
			) = 0;


		/*!
		* \brief Advices that a user has been disconnected
		* \param connectionId The connection id
		* \param systemId The user name
		* \param reason Disconnect reason, as in \c DisconnectReason
		*/
		virtual void OnUserDisconnected (
					unsigned int       connectionId,
					const std::string& systemId,
					DisconnectReason   reason
			) = 0;


		virtual ~CSMSCCallback(){}
	};

	/*!
	* \brief This class serves as the SMSC interface
	*/
	class SMPP_API CSMPPServer
		: public boost::enable_shared_from_this<CSMPPServer>
	{
	public:

		/*!
		* \brief Constructor
		* \param callbacks The callbacks to invoke when needed, see \c CSMPPCallback
		* \param port Port where to start listening for incoming connections
		*/
		CSMPPServer (
					boost::shared_ptr<CSMSCCallback> callbacks,
					unsigned int                     port,
					unsigned int                     threads = 0
			);


		/*!
		 * Set the default encoding for message delivering
		 *
		 * \param data_coding Data coding to use
		 */
		void SetEnconding(DataCoding data_coding);

		/*!
		* \brief Start listening
		* \return 0 if ok, no zero if something fails
		*/
		int Start();

		/*!
		* \brief Stop listening, kicks of all connected users
		*/
		void Stop();

		/*!
		* \return \c true if the server is accepting connections
		*/
		bool IsRunning() const;

		/*!
		* \brief Sends a message to an ESME connected to this SMSC
		* \param from Source address of the short message
		* \param to Destination address (should match a connected user)
		* \param message The text body encoded in UTF-8
		* \return \c DELIVERY_OK if everything runs smoothly
		* \return \c DELIVERY_REJECTED if the user rejects the message
		* \return \c DELIVERY_INV_DEST_ADDR if there is no user with address \p to
		* \return \c DELIVERY_UNKNOWN_ERROR if something gets broken (connection lost, unknown error, whatever)
		*/
		DeliveryResult SendMessage(
					const std::string& from,
					const std::string& to,
					const std::string& message
			);

	private:
		struct pimpl;
		boost::shared_ptr<pimpl> m_pimpl;
	};


	/*******************************/
	/*         SMPP Client         */
	/*******************************/


	class SMPP_API CESMECallback
		: public boost::enable_shared_from_this<CESMECallback>
	{
	public:

		/*!
		* Connection Lost callback
		*/
		virtual void OnConnectionLost (
					int cause
			) = 0;

		/*!
		* Called when a new message arrives
		* \param from Who send the message
		* \param to The number the message is sent to
		* \param content UTF-8 encoded message text
		*/
		virtual void OnIncomingMessage (
					const std::string &from,
					const std::string &to,
					const std::string &content
			) = 0;


		virtual ~CESMECallback(){}
	};

	class SMPP_API CSMPPClient
		: public boost::enable_shared_from_this<CSMPPClient>
	{
	public:

		/*!
		* Constructor
		* \param callback User supplied callbacks
		* \param smscIP IP Address of the SMSC this class should connect to
		* \param smscPort Port where the SMSC is linstening
		* \param loginMode Defines how to bind with the SMSC
		*/
		CSMPPClient(
				boost::shared_ptr<CESMECallback> callbacks,
				const std::string&               smscIP = "",
				unsigned short                   smscPort = 0,
				BindType                         loginMode = BIND_TYPE_TRANSCEIVER
		);

		~CSMPPClient();

	public:

		/*! Sets the number of client threads, use it if you are going
		* to create a lot of clients. Default is 2 threads */
		static void SetClientThreads(unsigned int count);

	public: // Gets

		/*! \return The SMSC's IP address */
		std::string GetServerAddress() const;

		/*! \return The SMSC's listening port */
		unsigned short GetServerPort() const;

		/*! \return The Bind mode */
		BindType GetBindMode() const;

		/*! \return The system  Id (aka username) */
		std::string GetSystemId() const;

		/*! \return The system type */
		std::string GetSystemType() const;

		/*! \return The password used to validate with the SMSC */
		std::string GetPassword() const;

		/*! \return The address range: \see SetAddressRange */
		std::string GetAddressRange() const;

		/*! \return \c true if this client is bound to a SMSC, otherwise \c false */
		bool IsBound() const;

		/*! \return The systemId of the server this client is connected to */
		std::string GetServerSystemId() const;

		/*! Copies message settings to \p ms */
		void GetMessageSettings(MessageSettings *ms);

	public: // Sets

		/*! Sets the server address */
		void SetServerAddress(const std::string& server);

		/*! Sets the server port */
		void SetServerPort(unsigned short port);

		/*! Sets the bind mode */
		void SetBindMode(BindType lt);

		/*! Sets the systemId (aka username) */
		void SetSystemId(const std::string& sysId);

		/*! Sets the system type, this optional but sometimes is required by the SMSC */
		void SetSystemType(const std::string& sysType);

		/*! Sets the password used to validate with the SMSC */
		void SetPassword(const std::string& password);

		/*! Sets the address range, aka: a pattern that specifies which addresses are accepted by this client */
		void SetAddressRange(const std::string& pattern);

		/*! Sets settings related to message delivering and receipt. \see MessageSettings */
		void SetMessageSettings(const MessageSettings& ms);

	public: // Non-atomic functions


		/*! Connects and binds the client with the SNMP server (SMSC) */
		LoginResult Bind();


		/*! Unbinds (disconnect) the client from the SMSC */
		void Unbind();

		/*!
		 * Send the \c ENQUIRE_LINK command to the server
		 */
		bool SendKeepAlive();

		/*!
		* Sends a short message
		* \param from Sender Id, Who the message is from
		* \param to   Receipt of the message
		* \param content UTF-8 encoded message text
		*/
		DeliveryResult SendMessage (
					const std::string& from,
					const std::string& to,
					const std::string& content
			);

	private:
		struct impl;
		boost::scoped_ptr<impl> pimpl;
	};

} //namespace libsmpp

#ifdef _WIN32
# pragma pop_macro("SendMessage")
#endif

#if _MSC_VER > 1000
# pragma warning(pop)  	// restore original warning level
#endif

#endif // libsmpp_hpp__
