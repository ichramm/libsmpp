/*!
 * \file smppcommands.h
 * \author ichramm
 */
#ifndef smppcommands_h_
#define smppcommands_h_

#include "smppdefs.h"
#include "libsmpp34/smpp34.h"
#include "libsmpp34/smpp34_structs.h"
#include "libsmpp34/smpp34_params.h"

#if _MSC_VER > 1000
#pragma warning(push)	// disable for this header only
#pragma warning(disable: 4250)  //  warning C4250: 'classA' : inherits 'classB' via dominance
#pragma warning(disable: 4996) // '_snprintf': This function or variable may be unsafe. Consider using _snprintf_s instead.
#endif

#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <string.h>

namespace libsmpp
{

class ISMPPCommand : public boost::enable_shared_from_this<ISMPPCommand>
{
public:
	// get command and response id
	virtual unsigned int request_id() const = 0;
	virtual unsigned int response_id() const = 0;
	virtual unsigned int sequence_number() const = 0;
	// no sense to ask for the status code of a request, right?
	virtual int command_status() const = 0;
	// set response status (useful when handling GENERICK NACK responses)
	virtual void command_status(int val) = 0;
	//get pointer to request and response
	virtual void *request_ptr() = 0;
	virtual void *response_ptr() = 0;
	// pack to a buffer
	virtual unsigned int pack_request(char *buffer, unsigned int bufferLen) = 0;
	virtual unsigned int pack_request(char *buffer, unsigned int bufferLen, int &err) = 0;
	virtual unsigned int pack_response(char *buffer, unsigned int bufferLen) = 0;
	virtual unsigned int pack_response(char *buffer, unsigned int bufferLen, int &err) = 0;
	// unpack from buffer
	virtual int unpack_request(const char *buffer, int bufferLen) = 0;
	virtual int unpack_request(const char *buffer, int bufferLen, int &err) = 0;
	virtual int unpack_response(const char *buffer, int bufferLen) = 0;
	virtual int unpack_response(const char *buffer, int bufferLen, int &err) = 0;

	virtual ~ISMPPCommand(){}
};

template <typename request_t, typename response_t>
class CSMPPCommand : public virtual ISMPPCommand
{
public:

	CSMPPCommand(int commandId, int sequence_number)
	{
		memset(&_request, 0, sizeof(_request));
		memset(&_response, 0, sizeof(_response));
		_request.command_id  = commandId;
		_response.command_id = commandId | SMPP_RESPONSE_BIT;
		_request.sequence_number  = sequence_number;
		_response.sequence_number = sequence_number;
		_request.command_status  = ESME_ROK;
		_response.command_status = ESME_ROK;
	}

	virtual ~CSMPPCommand()
	{
	}

	request_t& request()
	{
		return _request;
	}

	response_t& response()
	{
		return _response;
	}

	unsigned int request_id() const
	{
		return _request.command_id;
	}

	unsigned int response_id() const
	{
		return _response.command_id; // (_request.command_id | SMPP_RESPONSE_BIT)
	}

	unsigned int sequence_number() const
	{
		return _request.sequence_number;
	}

	int command_status() const
	{
		return _response.command_status;
	}

	void command_status(int status)
	{
		_response.command_status = status;
	}

	void *request_ptr()
	{
		return &_request;
	}

	void *response_ptr()
	{
		return &_response;
	}

	int unsigned pack_request(char *buffer, unsigned int bufferLen)
	{
		int err;
		int len = this->pack_request(buffer, bufferLen, err);
		if ( err != 0 )
		{
			std::ostringstream error;
			error << "SMPP error " << smpp34_errno << ": " << smpp34_strerror;
			throw std::runtime_error(error.str());
		}
		return len;
	}

	int unsigned pack_request(char *buffer, unsigned int bufferLen, int &err)
	{
		int len;
		err = smpp34_pack(_request.command_id, (uint8_t *)buffer, bufferLen, &len, &_request);
		return len;
	}

	int unsigned pack_response(char *buffer, unsigned int bufferLen)
	{
		int err;
		int len = this->pack_response(buffer, bufferLen, err);
		if ( err == 0 ) {
			return len;
		}

		std::ostringstream error;
		error << "SMPP error " << smpp34_errno << ": " << smpp34_strerror;
		throw std::runtime_error(error.str());
	}

	int unsigned pack_response(char *buffer, unsigned int bufferLen, int &err)
	{
		int len;
		int respCmd = _request.command_id | SMPP_RESPONSE_BIT;
		err = smpp34_pack(respCmd, (uint8_t *)buffer, bufferLen, &len, &_response);
		return len;
	}

	int unpack_request(const char *buffer, int bufferLen)
	{
		int err;
		int res = this->unpack_request(buffer, bufferLen, err);
		if ( err == 0 ) {
			return res;
		}

		std::ostringstream error;
		if(ESME_RINVCMDID == err) {
			error << "Invalid command id: " << res;
		} else {
			error << "SMPP error " << smpp34_errno << ": " << smpp34_strerror;
		}
		throw std::runtime_error(error.str());
	}

	int unpack_request(const char *buffer, int bufferLen, int &err)
	{
		unsigned int command_id = _request.command_id;
		err = smpp34_unpack(command_id, &_request, (uint8_t *)buffer, bufferLen);
		if(!err && _request.command_id != command_id)
		{
			std::swap((unsigned int&)_request.command_id, command_id);
			err = ESME_RINVCMDID;
			return command_id;
		}
		return _request.command_status;
	}

	int unpack_response(const char *buffer, int bufferLen)
	{
		int err;
		int res = this->unpack_response(buffer, bufferLen, err);
		if ( err == 0 ) {
			return res;
		}

		std::ostringstream error;
		if(ESME_RINVCMDID == err) {
			error << "Invalid response command id: " << res;
		} else {
			error << "SMPP error " << smpp34_errno << ": " << smpp34_strerror;
		}
		throw std::runtime_error(error.str());
	}

	int unpack_response(const char *buffer, int bufferLen, int &err)
	{
		unsigned int command_id = _response.command_id;
		err = smpp34_unpack(command_id, &_response, (uint8_t *)buffer, bufferLen);
		if(!err && _response.command_id != command_id)
		{
			std::swap((unsigned int&)_response.command_id, command_id);
			err = ESME_RINVCMDID;
			return command_id;
		}
		return _response.command_status;
	}

protected:
	request_t  _request;
	response_t _response;
};

/************************************************************************/
class ISMPPBind : public virtual ISMPPCommand
{
public:
	virtual std::string getSystemId() const = 0;
	virtual std::string getPasssword() const = 0;
	virtual std::string getSystemType() const = 0;
	virtual std::string getAddressRange() const = 0;
	virtual TypeOfNumber getAddressTON() const = 0;
	virtual NumberingPlanIndicator getAdressNPI() const = 0;
	virtual void setSystemInfo(const std::string &systemId, const std::string &password, const std::string &systemType) = 0;
	virtual void setAddress(const std::string &addressRange, TypeOfNumber ton, NumberingPlanIndicator npi) = 0;

public:
	virtual std::string getResponseSystemId() const = 0;
	virtual int getResponseInterfaceVersion() const = 0;
	virtual void setResponseSystemId(const std::string &id) = 0;

public:
	virtual ~ISMPPBind(){}
};

/************************************************************************/
template <typename request_t, typename response_t>
class CBindCommand : public ISMPPBind, public CSMPPCommand<request_t, response_t>
{
public:
	CBindCommand(int commandId, int sequence_number)
		: CSMPPCommand<request_t, response_t>(commandId, sequence_number)
	{
		this->_request.interface_version = SMPP_VERSION;
	}

	std::string getSystemId() const;
	std::string getPasssword() const;
	std::string getSystemType() const;
	std::string getAddressRange() const;
	TypeOfNumber getAddressTON() const;
	NumberingPlanIndicator getAdressNPI() const;
	void setSystemInfo(const std::string &systemId, const std::string &password, const std::string &systemType);
	void setAddress(const std::string &addressRange, TypeOfNumber ton, NumberingPlanIndicator npi);

	std::string getResponseSystemId() const;
	int getResponseInterfaceVersion() const;
	void setResponseSystemId(const std::string &id);
};

/************************************************************************/
class CBindReceiver : public CBindCommand<bind_receiver_t, bind_receiver_resp_t>
{
public:
	CBindReceiver(int sequence_number)
		: CBindCommand<bind_receiver_t, bind_receiver_resp_t>(BIND_RECEIVER, sequence_number)
	{ }
};

/************************************************************************/
class CBindTransmitter : public CBindCommand<bind_transmitter_t, bind_transmitter_resp_t>
{
public:
	CBindTransmitter(int sequence_number)
		: CBindCommand<bind_transmitter_t, bind_transmitter_resp_t>(BIND_TRANSMITTER, sequence_number)
	{ }
};

/************************************************************************/
class CBindTransceiver : public CBindCommand<bind_transceiver_t, bind_transceiver_resp_t>
{
public:
	CBindTransceiver(int sequence_number)
		: CBindCommand<bind_transceiver_t, bind_transceiver_resp_t>(BIND_TRANSCEIVER, sequence_number)
	{ }
};

/************************************************************************/
class CSMPPUnbind : public CSMPPCommand<unbind_t, unbind_resp_t>
{
public:
	CSMPPUnbind(int sequence_number)
		: CSMPPCommand<unbind_t, unbind_resp_t>(UNBIND, sequence_number)
	{ }
};

/************************************************************************/
class CSMPPGenericNack : public CSMPPCommand<generic_nack_t, generic_nack_t>
{
public:
	CSMPPGenericNack(int sequence_number=0)
		: CSMPPCommand<generic_nack_t, generic_nack_t>(GENERIC_NACK, sequence_number)
	{ }
};

/************************************************************************/
class CSMPPEnquireLink : public CSMPPCommand<enquire_link_t, enquire_link_resp_t>
{
public:
	CSMPPEnquireLink(int sequence_number)
		: CSMPPCommand<enquire_link_t, enquire_link_resp_t>(ENQUIRE_LINK, sequence_number)
	{ }
};

/************************************************************************/
template <typename request_t, typename response_t>
class CSMPPSubmit : public CSMPPCommand<request_t, response_t>
{
protected:
	CSMPPSubmit(int commandId, int sequence_number)
		: CSMPPCommand<request_t, response_t>(commandId, sequence_number)
	{ }

public:

	virtual ~CSMPPSubmit()
	{
		if(this->_request.tlv)
		{
			destroy_tlv(this->_request.tlv);
			this->_request.tlv = NULL;
		}
	}

	std::string getText() const
	{
		std::string res;
		if(this->_request.sm_length)
		{
			res.resize(this->_request.sm_length);
			memcpy(&res[0], this->_request.short_message, this->_request.sm_length);
		}
		else
		{
			for (tlv_t *tlv = this->_request.tlv; tlv; tlv = tlv->next)
			{
				if (tlv->tag == TLVID_message_payload)
				{
					res.resize(tlv->length);
					memcpy(&res[0], tlv->value.octet, tlv->length);
				}
			}
		}
		return res;
	}

	std::string getSourceAddress() const
	{
		return std::string((char*)this->_request.source_addr);
	}

	TypeOfNumber getSourceAddressTON() const
	{
		return (TypeOfNumber)this->_request.source_addr_ton;
	}

	NumberingPlanIndicator getSourceAddressNPI() const
	{
		return (NumberingPlanIndicator)this->_request.source_addr_npi;
	}

	void setSourceAddress(const std::string& address, TypeOfNumber ton, NumberingPlanIndicator npi)
	{
		snprintf((char *)this->_request.source_addr, ARRAY_LEN(this->_request.source_addr), "%s", address.c_str());
		this->_request.source_addr_ton = ton;
		this->_request.source_addr_npi = npi;
	}

	void setServiceType(const std::string& serviceType)
	{
		snprintf((char *)this->_request.service_type, ARRAY_LEN(this->_request.service_type), "%s", serviceType.c_str());
	}

	void setText( const std::string &text, bool forceUsingPayload=false)
	{
		if (!forceUsingPayload && text.length() <= 254)
		{
			this->_request.sm_length = text.size();
			//snprintf((char *) this->_request.short_message, ARRAY_LEN(this->_request.short_message), "%s", &text[0]);
			memcpy(this->_request.short_message, &text[0], this->_request.sm_length);
		}
		else
		{
			tlv_t tlv;
			this->_request.sm_length = 0; // should be already zero 'cause the memset(), but it clarifies the code
			memset(&tlv, 0, sizeof(tlv));
			tlv.tag = TLVID_message_payload;
			tlv.length = sizeof(tlv.value.octet) > text.size() ? text.size() : sizeof(tlv.value.octet); // sizeof(tlv.value.octet) = 1024
			memcpy(tlv.value.octet, &text[0], tlv.length);
			build_tlv(&(this->_request.tlv), &tlv);
		}
	}

	void setConcatenatedMessageArgs(int total_segments, int msg_ref_num, int segment_seqnum)
	{
#define BUILD_TLV(tag__, field__, val__) do { \
	tlv_t tlv; \
	memset(&tlv, 0, sizeof(tlv)); \
	tlv.tag           = (tag__); \
	tlv.length        = sizeof(tlv.value.field__); \
	tlv.value.field__ = (val__); \
	build_tlv(&(this->_request.tlv), &tlv); \
} while(0)
		BUILD_TLV(TLVID_sar_total_segments,    val08, total_segments);
		BUILD_TLV(TLVID_sar_msg_ref_num,       val16, msg_ref_num);
		BUILD_TLV(TLVID_sar_segment_seqnum,    val08, segment_seqnum);
		BUILD_TLV(TLVID_more_messages_to_send, val08, (total_segments > segment_seqnum ? 1: 0));
	}
};

/************************************************************************/
class CSMPPSubmitSingle : public CSMPPSubmit<submit_sm_t, submit_sm_resp_t>
{
public:
	CSMPPSubmitSingle(int sequence_number)
		: CSMPPSubmit<submit_sm_t, submit_sm_resp_t>(SUBMIT_SM, sequence_number) { }

	std::string getDestinationAddress() const
	{
		return (char *)this->_request.destination_addr;
	}

	TypeOfNumber getDestinationAddressTON() const
	{
		return (TypeOfNumber)this->_request.dest_addr_ton;
	}

	NumberingPlanIndicator getDestinationAddressNPI() const
	{
		return (NumberingPlanIndicator)this->_request.dest_addr_npi;
	}

	void setDestination(const std::string& address, TypeOfNumber ton = TON_UNKNOWN,
			NumberingPlanIndicator npi = NPI_UNKNOWN)
	{
		size_t bytesToCopy = std::min(address.size(), sizeof(this->_request.destination_addr));
		memcpy(this->_request.destination_addr, &address[0], bytesToCopy);
		this->_request.dest_addr_ton = ton;
		this->_request.dest_addr_npi = npi;
	}
};

/************************************************************************/
class CSMPPSubmitMulti : public CSMPPSubmit<submit_multi_t, submit_multi_resp_t>
{
public:
	CSMPPSubmitMulti(int sequence_number)
		: CSMPPSubmit<submit_multi_t, submit_multi_resp_t>(SUBMIT_MULTI, sequence_number)
	{ }

	void setDestinations(const std::vector<std::string> &destinations)
	{
		dad_t dad;
		if(this->_request.dest_addr_def) {
			destroy_dad(this->_request.dest_addr_def);
		}

		for(unsigned int i = destinations.size(); i > 0; i--)
		{ // build_dad() inserts at the beginning of the list
			const std::string &address = destinations[i-1];
			size_t bytesToCopy = std::min(address.size(), sizeof(dad.value.sme.destination_addr));

			memset(&dad, 0, sizeof(dad));
			dad.dest_flag = DEST_FLAG_SME;
			memcpy(dad.value.sme.destination_addr, &address[0], bytesToCopy);
			build_dad(&this->_request.dest_addr_def, &dad);
		}
		this->_request.number_of_dests = (uint8_t)destinations.size();
	}

	std::vector<std::string> getFailedAddresses() const
	{
		std::vector<std::string> res;

		if (this->_response.no_unsuccess > 0)
		{
			for (udad_t *udad = this->_response.unsuccess_smes; udad; udad = udad->next)
			{
				res.push_back( std::string((char*)udad->destination_addr) );
			}
		}

		return res;
	}

	~CSMPPSubmitMulti()
	{
		if(this->_request.dest_addr_def)
		{
			destroy_dad(this->_request.dest_addr_def);
		}
		if(this->_response.unsuccess_smes)
		{
			destroy_udad(this->_response.unsuccess_smes);
		}
	}
};

/************************************************************************/
class CSMPPDelivery : public CSMPPSubmit<deliver_sm_t, deliver_sm_resp_t>
{
public:
	CSMPPDelivery(int sequence_number)
		: CSMPPSubmit<deliver_sm_t, deliver_sm_resp_t>(DELIVER_SM, sequence_number)
	{ }

	std::string getDestinationAddress() const
	{
		return (char *)this->_request.destination_addr;
	}

	void setDestination(const std::string &address, TypeOfNumber ton = TON_UNKNOWN,
			NumberingPlanIndicator npi = NPI_UNKNOWN)
	{
		size_t bytesToCopy = std::min(sizeof(this->_request.destination_addr), address.size());
		memcpy(this->_request.destination_addr, &address[0], bytesToCopy);
		this->_request.dest_addr_ton = ton;
		this->_request.dest_addr_npi = npi;
	}
};


/************************************************************************/
/************************************************************************/
template <typename request_t, typename response_t>
std::string CBindCommand<request_t, response_t>::getSystemId() const
{
	return std::string((char*)this->_request.system_id);
}

template <typename request_t, typename response_t>
std::string CBindCommand<request_t, response_t>::getPasssword() const
{
	return std::string((char*)this->_request.password);
}

template <typename request_t, typename response_t>
std::string CBindCommand<request_t, response_t>::getSystemType() const
{
	return std::string((char*)this->_request.system_type);
}

template <typename request_t, typename response_t>
std::string CBindCommand<request_t, response_t>::getAddressRange() const
{
	return std::string((char*)this->_request.address_range);
}

template <typename request_t, typename response_t>
TypeOfNumber CBindCommand<request_t, response_t>::getAddressTON() const
{
	return (TypeOfNumber)this->_request.addr_ton;
}

template <typename request_t, typename response_t>
NumberingPlanIndicator CBindCommand<request_t, response_t>::getAdressNPI() const
{
	return (NumberingPlanIndicator)this->_request.addr_npi;
}

template <typename request_t, typename response_t>
void CBindCommand<request_t, response_t>::setSystemInfo(const std::string &systemId, const std::string &password, const std::string &systemType)
{
	snprintf((char*)this->_request.system_id, ARRAY_LEN(this->_request.system_id), "%s", systemId.c_str());
	snprintf((char*)this->_request.password, ARRAY_LEN(this->_request.password), "%s", password.c_str());
	if(!systemType.empty()) {
		snprintf((char*)this->_request.system_type, ARRAY_LEN(this->_request.system_type), "%s", systemType.c_str());
	}
}

template <typename request_t, typename response_t>
void CBindCommand<request_t, response_t>::setAddress(const std::string &addressRange, TypeOfNumber ton, NumberingPlanIndicator npi)
{
	this->_request.addr_ton = (uint8_t) ton;
	this->_request.addr_npi = (uint8_t) npi;
	if(!addressRange.empty()) {
		snprintf((char*)this->_request.address_range, ARRAY_LEN(this->_request.address_range), "%s", addressRange.c_str());
	}
}

template <typename request_t, typename response_t>
std::string CBindCommand<request_t, response_t>::getResponseSystemId() const
{
	return std::string((const char*)this->_response.system_id);
}

template <typename request_t, typename response_t>
int CBindCommand<request_t, response_t>::getResponseInterfaceVersion() const
{
	for (tlv_t *tlv = this->_response.tlv; tlv; tlv = tlv->next)
	{
		if (TLVID_sc_interface_version == tlv->tag)
		{
			return (int)tlv->value.val08;
			break;
		}
	}
	return 0;
}

template <typename request_t, typename response_t>
void CBindCommand<request_t, response_t>::setResponseSystemId(const std::string &id)
{
	memcpy(this->_response.system_id, &id[0], id.size());
	tlv_t tlv;
	memset(&tlv, 0, sizeof(tlv));
	tlv.tag = TLVID_sc_interface_version;
	tlv.length = 1;
	tlv.value.val08 = SMPP_VERSION;
	build_tlv(&this->_response.tlv, &tlv);
}

} // namespace libsmpp


#if _MSC_VER > 1000
#pragma warning(pop)  	// restore original warning level
#endif

#endif // smppcommands_h_
