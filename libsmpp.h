/*!
 * \file libsmpp.h
 * \author ichramm
 *
 * This header exposes the plain C API
 */
#ifndef libsmpp_h__
#define libsmpp_h__
#if _MSC_VER > 1000
#pragma once
#endif

#ifdef _WIN32
# ifdef LIBSMPP_EXPORTS
#  define SMPP_API __declspec(dllexport)
# else
#  define SMPP_API __declspec(dllimport)
# endif
#else
# define SMPP_API
#endif

/*!
 * The maximum message length this library supports*
 * Longer messages are split in smaller chunks based on the \c MessageSettings configuration
 */
#define MAX_MESSAGE_LENGTH   1024

/*!
 * Opaque pointer to a SMPP Server instance
 */
typedef void* SMSC_HANDLE;

/*!
 * Opaque pointer to a SMPP Client instance
 */
typedef void* ESME_HANDLE;


typedef enum __BindType
{
	BIND_TYPE_RECEIVER,    /*!< \brief The user only wants to receive messages */
	BIND_TYPE_TRANSMITTER, /*!< \brief The user wants to send messages nor receive them */
	BIND_TYPE_TRANSCEIVER  /*!< \brief The user wants to send and receive messages */
} BindType;


typedef enum __LoginResult
{
	LOGIN_RESULT_OK = 0,
	LOGIN_RESULT_INVALIDUSR,    /*!< \brief Invalid systemId */
	LOGIN_RESULT_INVALIDPWD,    /*!< \brief Invalid password */
	LOGIN_RESULT_INVALIDADDR,   /*!< \brief The address_range is invalid for this user */
	LOGIN_RESULT_INVALIDCMD,    /*!< \brief The command is invalid, this usually means the login type is wrong */
	LOGIN_RESULT_FAIL           /*!< \brief I don't know why this happened, she was so young! */
} LoginResult;


typedef enum __DeliveryResult
{
	/*!< The message cuold be delivered ok, or will be, don't care */
	DELIVERY_OK,
	/*! Messsage was rejected, this usually means the user is logged in as Receiver */
	DELIVERY_REJECTED,
	/*! Source address is invalid */
	DELIVERY_INV_SRC_ADDR,
	/*! Destination address is invalid */
	DELIVERY_INV_DEST_ADDR,
	/*! I don't know what happen! */
	DELIVERY_UNKNOWN_ERROR
} DeliveryResult;


typedef enum __DisconnectReason
{
	DISCONNECT_REASON_UNBIND, /*!< \brief The user has disconnected his session  */
	DISCONNECT_REASON_KICKED,  /*!< \brief The user has been kicked because it didn't respond to the keep alive request */
	DISCONNECT_REASON_NETERROR /*!< \brief Connection with the client has been lost */
} DisconnectReason;


typedef enum __DataCoding
{
	/*! Use server's default enconding */
	DATA_CODING_DEFAULT = 0,

	/*! Encode messages using the GSM 03.38 character set */
	DATA_CODING_GSM0338 = 2,

	/*! Encode messages using the Latin 1 (ISO-8859-1) character set */
	DATA_CODING_LATIN1  = 3,

	/*! Encode messages using the UCS2 (ISO/IEC-10646) character set */
	DATA_CODING_UNICODE = 8,

	/*! Encode messages using UTF-8 */
	DATA_CODING_UTF8    = 11
} DataCoding;

/*!
 * Defines setting for message delivering and reception
 * Allows us to execute proper workaround to common SMSC limitations
 */
typedef struct __MessageSettings
{
	/*! Defines the data coding used to sent messages (default = 0) */
	unsigned int DeliverDataCoding;

	/*! Defines the encoding used by the SMSC, used when \c DeliverDataCoding = 0 */
	unsigned int ServerDefaultEncoding;

	/*! Determines when the message content should be encoded using GSM-7bit packing (default=0) */
	unsigned char EnableGSM7bitPacking;

	/*! The maximum count in characters of a message (default = 0: MAX_MESSAGE_LENGTH)
	 * Please note that if \c EnablePayload is set longer messages can be sent */
	unsigned int MaxMessageLength;

	/*! Allows to use the SMPP concatenated message function (default = 1) */
	unsigned char EnableMessageConcatenation;

	/*! Allows the long messages as a payload (TLV) (default = 1) */
	unsigned char EnablePayload;

	/*! This flag indicates if the SUBMIT_MULTI_SM operation is supported by the server.
	 * If it is not then requests of this type are split into individual
	 * SUBMIT_SM operations (default = 1) */
	unsigned char EnableSubmitMulti;

	/*!
	 * This ugly hack allows us to support big endian in short_message field of a DELIVER_SM request.
	 * If this option is set the client will transform the short_message's endianness
	 */
	unsigned char BigEndianUnicode;

} MessageSettings;

/*! Log functions have the same signature so let's define a type for them */
typedef void (*LogFunction)(const char *message);

/*!
 * Logger structure, set to use your own log
 */
typedef struct __Logger
{
	LogFunction debug;
	LogFunction notice;
	LogFunction warning;
	LogFunction error;
	LogFunction fatal;
	LogFunction profile;
} Logger;


/*!
 * \brief Validate a user login
 * \param connectionId Uniquely identifies the user's connection
 * \param loginType Type of login, as explained on \c LoginType
 * \param systemId The username
 * \param password That
 * \param address_range vector holding the phone numbers that the user handles (or wants to handle)
 * \param arSize the number of elements in \p address_range
 */
typedef LoginResult (*Callback_ValidateUser)(
			unsigned int connectionId,
			BindType     loginType,
			const char*  systemId,
			const char*  password,
			const char* address_range
	);

/*!
 * \brief A new message from a client ESME has come and we should deliver it
 * \param connectionId The is of the connection who sent this message
 * \param from The source address of the message (set by the ESME)
 * \param to The destination address of the message (aka: The phone number)
 * \param message The text body of the message, encoded in UTF-8
 * \param msgSize The size in characters of \p message
 */
typedef DeliveryResult (*Callback_DeliverMessage)(
			unsigned int connectionId,
			const char*  from,
			const char*  to,
			const char*  message,
			unsigned int msgSize
	);


/*!
 * \brief Advices that a user has been disconnected
 * \param connectionId The connection id
 * \param systemId The user name
 * \param reason Disconnect reason, as in \c DisconnectReason
 */
typedef void (*Callback_OnUserDisconnected)(
			unsigned int     connectionId,
			const char*      systemId,
			DisconnectReason reason
	);

/*!
 * \brief Called when a new text message is received
 * \param hClient The ESME instance who triggered this event
 * \param from Who sent the message
 * \param to The address of the receipt of the message
 * \param content The message text in UTF-8 (It may be not NULL-terminated, use \p size)
 * \param size   Size (in chars) of the string pointed by \p content
 */
typedef void (*Callback_OnIncomingMessage)(
			ESME_HANDLE  hClient,
			const char*  from,
			const char*  to,
			const char*  content,
			unsigned int size
	);

/*!
 * \brief Called when the connection with the server is lost
 * \param hClient The ESME instance who triggered this event
 * \param reason Should represent something, at the moment it doesn't...
 */
typedef void (*Callback_OnConnectionLost)(
			ESME_HANDLE hClient,
			int         reason
	);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

/*!
 * Sets a log handler to be used by the library
 */
SMPP_API int libSMPP_SetLogger(
			Logger *logger
	);

/*!
 * \return A \c LoginType value in a human-readable string
 */
SMPP_API const char *libSMPP_LoginTypeToString(
			BindType lt
	);

/*!
 * \return A \c LoginResult value in a human-readable string
 */
SMPP_API const char *libSMPP_LoginResultToString(
			LoginResult lr
	);

/*!
 * \return A \c DeliveryResult value in a human-readable string
 */
SMPP_API const char *libSMPP_DeliveryResultToString(
			DeliveryResult dr
	);

/*!
 * \return A \c DisconnectReason value in a human-readable string
 */
SMPP_API const char *libSMPP_DisconnectReasonToString(
			DisconnectReason dr
	);

/*!
 * Fills the \p ms variable with default values
 */
SMPP_API void libSMPP_CreateDefaultMessageSettings(
			MessageSettings *ms
	);

/*!
 * \brief Creates a new SMPP Server instance (or SMSC)
 * \return A pointer to the new server instance
 */
SMPP_API SMSC_HANDLE libSMPP_ServerCreate();

/*!
 * \brief Releases all memory allocated by a call to \c libSMPP_ServerCreate and
 * all consequent functions
 */
SMPP_API void libSMPP_ServerDelete (
			SMSC_HANDLE hServer
	);

/*!
 * \brief Sets the listening port for this server instance
 * \remark This function must be called BEFORE \c libSMPP_ServerStart
 */
SMPP_API void libSMPP_ServerSetPort (
			SMSC_HANDLE  hServer,
			unsigned int port
	);

/*!
 * \brief Sets the callbacks for this server instance
 * \remark This function must be called BEFORE \c libSMPP_ServerStart
 */
SMPP_API void libSMPP_ServerSetCallbacks (
			SMSC_HANDLE                 hServer,
			Callback_ValidateUser       validateFn,
			Callback_DeliverMessage     deliverFn,
			Callback_OnUserDisconnected onDisconFn
	);

/*!
 * \brief Start listening
 * \return 0 if the server could be started, no zero if something fails
 * \pre \c libSMPP_ServerSetPort and \c libSMPP_ServerSetCallbacks should be called before
 */
SMPP_API int libSMPP_ServerStart (
			SMSC_HANDLE hServer
	);

/*!
 * \brief Stop listening, kicks of all connected users
 */
SMPP_API void libSMPP_ServerStop(
			SMSC_HANDLE hServer
	);

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
SMPP_API DeliveryResult libSMPP_ServerSendMessage (
			SMSC_HANDLE,
			const char *from,
			const char *to,
			const char *message
	);


/*******************************/
/*         SMPP Client         */
/*******************************/


/*!
 * Creates a new SMPP CLient instance (ESME)
 * \param onNewMessage A function pointer which will be invoked when a new message arrives
 * \param onConnectionLost A function pointer which will be invoked when the
 * connection with the server is lost
 * \return An opaque pointer to the new client instance
 */
SMPP_API ESME_HANDLE libSMPP_ClientCreate (
			Callback_OnIncomingMessage onNewMessage,
			Callback_OnConnectionLost  onConnectionLost
	);

/*!
 * Destroys the object \p handle.
 * \param hClient The ESME instance about to be destroyed
 */
SMPP_API void libSMPP_ClientDelete (
			ESME_HANDLE hClient
	);

/*!
 * Sets the SMSC's IP address and port
 * \param hClient The ESME instance
 * \param serverIP SMSC's IP Address
 * \param serverPort SMSC's listening port
 */
SMPP_API void libSMPP_ClientSetServerAddress (
			ESME_HANDLE    hClient,
			const char*    serverIP,
			unsigned short serverPort
	);

/*!
 * Sets the Bind Mode
 * \param hClient The ESME instance
 * \param loginType a \c LoginType value specifying with Bind mode should
 * the library use: \c Transmitter, \c Receiver or \c Transceiver
 */
SMPP_API void libSMPP_ClientSetLoginType (
			ESME_HANDLE hClient,
			BindType    loginType
	);

/*!
 * Sets the systemId (aka username)
 * \param hClient The ESME instance
 * \param systemId The username used to validate with the SMSC
 */
SMPP_API void libSMPP_ClientSetSystemId (
			ESME_HANDLE hClient,
			const char* systemId
	);

/*!
 * Sets the system type, this optional but sometimes is required by the SMSC
 * \param hClient The ESME instance
 * \param systemType The system type, it's meaning depends on the SMSC, usually
 * describes something the application does (SMS Gateway, WAP Gateway, ...)
 */
SMPP_API void libSMPP_ClientSetSystemType (
			ESME_HANDLE hClient,
			const char* systemType
	);

/*!
 * Sets the password used to validate with the SMSC
 * \param hClient The ESME instance
 * \param password The password the SMSC is going to validate onto
 */
SMPP_API void libSMPP_ClientSetPassword (
			ESME_HANDLE hClient,
			const char* password
	);

/*!
 * Sets the address range, aka: a pattern that specifies which addresses are accepted by this client
 * \param hClient The ESME instance
 * \param pattern A simple regex that specifies which addresses are accepted by this client
 */
SMPP_API void libSMPP_ClientSetAddressRange (
			ESME_HANDLE hClient,
			const char* pattern
	);

/*!
 * Sets settings related to message delivering and receipt. \see MessageSettings
 * \param hClient The ESME instance
 * \param ms  The c MessageSettings structure
 */
SMPP_API void libSMPP_ClientSetMessageSettings (
			ESME_HANDLE            hClient,
			const MessageSettings *ms
	);

/*!
 * Connects and binds the client with the SNMP server (SMSC)
 * \param hClient The ESME instance
 */
SMPP_API LoginResult libSMPP_ClientBind (
			ESME_HANDLE hClient
	);

/*!
 * Unbinds (disconnect) the client from the SMSC
 * \param hClient The ESME instance
 */
SMPP_API void libSMPP_ClientUnBind (
			ESME_HANDLE hClient
	);

/*!
 * Sends the \c ENQUIRE_LINK command to the server
 * \return 1 on success, 0 on error
 */
SMPP_API int libSMPP_ClientSendKeepAlive (
			ESME_HANDLE hClient
	);

/*!
 * Send a short message
 * \param hClient The ESME instance (must be bound already)
 * \param from Sender Id, Who the message is from
 * \param to   Receipt of the message
 * \param content UTF-8 encoded message text
 * \param size   Size (in chars) of \p content
 */
SMPP_API DeliveryResult libSMPP_ClientSendMessage (
			ESME_HANDLE  hClient,
			const char*  from,
			const char*  to,
			const char*  content,
			unsigned int size
	);


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif // libsmpp_h__
