/*!
 * \file smppdefs.h
 * \author ichramm
 */
#ifndef smppdefs_h_
#define smppdefs_h_

#define SMPP_HEADER_SIZE	16

#define SMPP_RESPONSE_BIT (1 << 31)

// we take a reserver smpp value for us
#define SMPP_DATA_CODING_UTF8 0x0B

//pre: array cannot be empty
#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

/*!
 * A tener en cuenta
 * 1- Hay que incluir stdint antes de la libsmpp sino se garca
 * 2- Hay que castear los char* a uint8_t* porque como es c++ el casting no es explicito
 */

/*!
 * \brief Classification of Bind request
 */
enum BindMode
{
	/*! \brief User wants to receive messages */
	BIND_MODE_RECEIVER,
	/*! \brief User wants to send messages */
	BIND_MODE_TRANSMITTER,
	/*! \brief User wants to send and receive messages*/
	BIND_MODE_TRANSCEIVER,

	BIND_MODE_INVALID
};

/*!
 * \brief High level mapping for bind results
 */
enum BindResult
{
	/*! \brief Bind went OK */
	BIND_OK,
	/*! \brief Invalid system id (in human language: username) */
	BIND_INVALID_ID,
	/*! \brief Invalid password */
	BIND_INVALID_PWD,
	/*! \brief The SMSC does not support the kind of bind used, you can try another one */
	BIND_INVALID_CMD,
	/*! \brief Other generic error, local (pack, unpack) or remote (SMSC error) */
	BIND_FAIL
};


/*! \brief Destination flag */
enum DestAddressFlag
{
	DEST_FLAG_SME = 1,
	DEST_FLAG_DISTRIBUTION_LIST = 2
};

/*!
 * \brief Type of number of the ESME address
 */
enum TypeOfNumber
{
	TON_UNKNOWN = 0,
	TON_INTERNATIONAL = 1,
	TON_NATIONAL = 2,
	TON_NETWORK_SPECIFIC = 3,
	TON_SUSCRIBER_NUMBER = 4,
	TON_ALPHANUMERIC = 5,
	TON_ABBREVIATED = 6
};

/*!
 * \brief Numbering Plan Indicator for ESME address
 */
enum NumberingPlanIndicator
{
	NPI_UNKNOWN = 0,
	NPI_ISDN_E163_E164_ = 1,
	NPI_DATA_X121 = 3,
	NPI_TELEX_F69 = 4,
	NPI_LAND_MOBILE_E212 = 6,
	NPI_NATIONAL = 8,
	NPI_PRIVATE = 9,
	NPI_ERMES_ = 10,
	NPI_INTERNET_IP = 14,
	NPI_WAP_CLIENT_ID = 18
};

/*!
 * \brief Indicates the SMS Application service associated with the message
 */
enum ServiceType
{
	/*! \brief Default */
	SERVICE_TYPE_NULL = 0,

	/*! \brief Cellular messaging */
	SERVICE_TYPE_CMT,

	/*! \brief Cellular Paging */
	SERVICE_TYPE_CPT,

	/*! \brief Voive Mail Notification */
	SERVICE_TYPE_VMN,

	/*! \brief Voice Mail Alerting */
	SERVICE_TYPE_VMA,

	/*! \brief Wireless Application Protocol */
	SERVICE_TYPE_WAP,

	/*! \brief Unstructured Supplementary Services Data */
	SERVICE_TYPE_USSD
};

enum Client2ServerMessageAttributes
{
	//Messaging mode (bits 1-0)

	/*! \brief Use default SMSC mode */
	C2S_MSGATTR_MODE_SMSC = 0,
	/*! \brief Datagram mode */
	C2S_MSGATTR_MODE_DATAGRAM = (1 << 0),
	/*! \brief Forward (aka transaccion) mode */
	C2S_MSGATTR_MODE_FORWARD = (1 << 1),
	/*! \brief Store and Forward mode (in case SMSC mode is not Store and Forward */
	C2S_MSGATTR_MODE_ST_AND_FWD = (1 << 1) || (1 << 0),

	// Message Type (bits 5-2)

	/*! \brief Default message type */
	C2S_MSGATTR_TYPE_NORMAL = 0,
	/*! \brief Message contains ESME Delivery Acknowledgement */
	C2S_MSGATTR_TYPE_ESME_DELIV_ACK = (1 << 3),
	/*! \brief Message contains ESME Manual/User Acknowledgement */
	C2S_MSGATTR_TYPE_ESME_USER_ACK = (1 << 4),

	// GSM Network Specific Features (bits 7-6)

	/*! \brief No specific features */
	C2S_MSGATTR_GSMSPEC_NONE = 0,

	/*! \brief UDHI indicator (only relevant for short messages) */
	C2S_MSGATTR_GSMSPEC_UDHI = (1 << 6),
	/*! \brief Set Reply Path (only relevant for GSM network) */
	C2S_MSGATTR_GSMSPEC_REPLY = (1 << 7),
	/*! \brief set UDHI and Reply Path (only relevant for GDM network) */
	C2S_MSGATTR_GSMSPEC_UDHI_REPLY = (C2S_MSGATTR_GSMSPEC_UDHI | C2S_MSGATTR_GSMSPEC_REPLY),


	/*! \brief Combines all normal atributes */
	C2S_MSGATTR_DEFAULT = C2S_MSGATTR_MODE_SMSC | C2S_MSGATTR_TYPE_NORMAL | C2S_MSGATTR_GSMSPEC_NONE
};

enum Server2ClientMessageAttributes
{
	// Messaging mode not applicable (bits 1-0)

	//Messagins type (bits 5-2)

	/*! \brief Default message type */
	S2C_MSGATTR_TYPE_NORMAL = 0x0,
	/*! \brief Message contains SMSC Delivery Receipt */
	S2C_MSGATTR_TYPE_SMSC_DELIV_RCPT = 1 << 2,
	/*! \brief Message contains SME Delivery Acknowledgement */
	S2C_MSGATTR_TYPE_SME_DELIV_ACK = 1 << 3,
	/*! \brief Message contains SME Manual/User Delivery Acknowledgement */
	S2C_MSGATTR_TYPE_USER_ACK = 1 << 4,
	/*! \brief Message contains Conversation Abort (Korean CDMA) */
	S2C_MSGATTR_TYPE_CONV_ABORT = (1 << 3) | (1 << 4),
	/*! \brief Message contains Intermediate Delivery Notification */
	S2C_MSGATTR_TYPE_INT_DELIV_NOTIF = (1 << 5),

	// GSM Network Specific Features (bits 7-6)

	/*! \brief No specific features selected */
	S2C_MSGATTR_GSMSPEC_NONE = 0,
	/*! \brief UDHI indicator set */
	S2C_MSGATTR_GSMSPEC_UDHI = (1 << 6),
	/*! \brief Reply Path */
	S2C_MSGATTR_GSMSPEC_REPLY = (1 << 7),
	/*! \brief UDHI and Reply Path */
	S2C_MSGATTR_GSMSPEC_UDHI_REPLY = S2C_MSGATTR_GSMSPEC_UDHI | S2C_MSGATTR_GSMSPEC_REPLY
};


/*!
 * \brief Used to set the oriority_flag parameter
 * Allows the originating SME to assign a priority level to the short message
 */
enum MessagePriority
{
	MSG_PRI_LOWEST = 0,
	MSG_PRIO_LVL1 = 1,
	MSG_PRIO_LVL2 = 2,
	MSG_PRIO_HIGHEST = 3
};


//implemented in smppsession.cpp
//const char *bindresult2str(BindResult b);
//const char *bindmode2str(BindMode m);

#endif	/* smppdefs_h_ */

