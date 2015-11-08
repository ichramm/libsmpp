/*!
 * \file converter.cpp
 * \author ichramm
 *
 * Created on December 5, 2010, 1:14 AM
 */
#include "stdafx.h"
#include "converter.hpp"
#include "smppdefs.h"
#include "iconv/gsm7.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _MSC_VER
#include "iconv/iconv.h"
#define snprintf _snprintf
#else
#include <iconv.h>
#endif

#ifdef EXTREME_DEBUG
#define CONVERTER_DEBUG(__msg, __str) do { \
	std::string copy = __str; \
	for (unsigned i = 0; i < copy.size(); ++i) \
		if (!copy[i]) copy[i] = '.'; \
	fprintf(stderr, "%s: size[%lu] text[%s]\n", (const char *)__msg, copy.size(), copy.c_str()); \
} while(false)
#else
#define CONVERTER_DEBUG(__msg, __str)
#endif

static const size_t INCONV_BUFFER_SIZE = 32768;

#ifdef _WIN32
	extern DWORD dwTlsIndexIconvBuffer;
#elif GCC_VERSION > 0
	__thread char iconv_buffer__[INCONV_BUFFER_SIZE];
#endif


static inline char* get_inconv_buffer()
{
#ifdef _WIN32
	char *iconv_buffer__ = (char *)TlsGetValue(dwTlsIndexIconvBuffer);
	if (iconv_buffer__ == NULL) {
		iconv_buffer__ = (char *)LocalAlloc(LMEM_FIXED, INCONV_BUFFER_SIZE);
		TlsSetValue(dwTlsIndexIconvBuffer, (LPVOID)iconv_buffer__);
	}
#endif
	memset(iconv_buffer__, 0, INCONV_BUFFER_SIZE);
	return iconv_buffer__;
}


using namespace std;

static std::string  utf8ToGsm7 ( const std::string  &src );
static std::string  gsm7ToUtf8 ( const std::string  &src );
static std::wstring utf8toWStr ( const std::string  &src );
static std::string  wstrToUtf8 ( const std::wstring &src );


namespace
{
// the second parameter of iconv() may or may not be const
// so, lets the compiler choose the right type
class iconv_arg
{
public:
	iconv_arg(const char** ptr) : m_ptr(ptr) { }
	operator const char**() {
		return m_ptr;
	}
	operator char**() {
		return const_cast <char**>(m_ptr);
	}
private:
	const char** m_ptr;
};

class iconv_wrapper
{
private:
	iconv_t cd;

public:
	iconv_wrapper(const std::string &charsetFrom, const std::string &charsetTo) {
		cd = iconv_open(charsetTo.c_str(), charsetFrom.c_str());
		if (cd == reinterpret_cast <iconv_t>(-1)) {
			cd = NULL;
		}
	}

	~iconv_wrapper() {
		if(cd) {
			iconv_close(cd);
			cd = NULL;
		}
	}

	int convert(const std::string& src, string &dest)
	{
		if( cd == NULL )
		{
			return -1;
		}

		const char *inbuf = &src[0];
		size_t inbytesleft = src.size();
		char *outbuf = get_inconv_buffer();
		size_t outbytesleft = INCONV_BUFFER_SIZE;
		char *outbuf_begin = outbuf;

		while (inbytesleft > 0)
		{
			if(iconv(cd, iconv_arg(&inbuf), &inbytesleft, &outbuf, &outbytesleft) == size_t(-1))
			{
				if(errno == E2BIG)
				{ // should not happen, my buffer is big enough
					return -2;
				}

				if (errno == EILSEQ && outbytesleft > 0)
				{ // invalid byte sequence
					outbuf[INCONV_BUFFER_SIZE-outbytesleft] = '?';
					++inbuf;
					--inbytesleft;
					++outbuf;
					--outbytesleft;
					continue;
				}

				// unrecoverable error
				return -3;
			}
		}

		dest = string(outbuf_begin, INCONV_BUFFER_SIZE-outbytesleft);
		return 0;
	}
};

} // namespace


namespace opensmpp
{

CConverter::CConverter(const std::string &charsetFrom, const std::string &charsetTo)
	: m_charsetFrom(charsetFrom), m_charsetTo(charsetTo)
{
	smpp_log_debug("Converter: from=%s to=%s", m_charsetFrom.c_str(), m_charsetTo.c_str());
}

int CConverter::Convert(const std::string &src, std::string &dest)
{
	CONVERTER_DEBUG("Converting",  src);

	int res = ConvertInternal(src, dest);

	CONVERTER_DEBUG("Converted",  dest);

	if(res) {
		smpp_log_error("Failed to convert text: %d", res);
	}

	return res;
}

int CConverter::ConvertInternal(const std::string &src, std::string &dest)
{
	if(m_charsetFrom == "GSM7" || m_charsetTo == "GSM7" ) {
		std::string tmp;
		if(m_charsetFrom == "GSM7") {
			tmp = gsm7ToUtf8(src);
			if(m_charsetTo == "UTF-8") {
				dest = tmp;
				return 0;
			}
			iconv_wrapper iw("UTF-8", m_charsetTo);
			return iw.convert(tmp, dest);
		}

		iconv_wrapper iw(m_charsetFrom, "UTF-8");
		int res = iw.convert(src, tmp);
		if(res) {
			return res;
		}

		dest = utf8ToGsm7(tmp);
		return 0;
	}

	iconv_wrapper iw(m_charsetFrom, m_charsetTo);
	return iw.convert(src, dest);
}

const char *CConverter::GetCharsetFromDataCoding(unsigned char data_coding)
{
	// we check only the first 4 bytes
	switch(data_coding & 0x0F)
	{
	// default encoding is UTF-8
	case 0:
		return "GSM7";
	case SMPP_DATA_CODING_UTF8: // 0x0B
		return "UTF-8";

	case 0x01: // 0 0 0 0 0 0 0 1 IA5 (CCITT T.50)/ASCII (ANSI X3.4)
		return "ANSI_X3.4-1986";
	case 0x02: // 0 0 0 0 0 0 1 0 Octet unspecified (8-bit binary)
	case 0x04: // 0 0 0 0 0 1 0 0 Octet unspecified (8-bit binary)
		//These coding schemes are common to GSM, TDMA and CDMA. The SMPP protocol
		//allows ESME applications to use the same DCS value (i.e. the GSM 03.38 value) for all
		//three technologies.
		return "GSM7";
	case 0x03: // 0 0 0 0 0 0 1 1 Latin 1 (ISO-8859-1)
		return "ISO-8859-1";
	case 0x05: //0 0 0 0 0 1 0 1 JIS (X 0208-1990)
	case 0x0D: // 0 0 0 0 1 1 0 1 Extended Kanji JIS(X 0212-1990)
		return "EUC-JP";
	case 0x06: //0 0 0 0 0 1 1 0 Cyrillic (ISO-8859-5)
		return "iso-8859-5";
	case 0x07: // 0 0 0 0 0 1 1 1 Latin/Hebrew (ISO-8859-8)
		return "iso8859-8";
	case 0x08: // 0 0 0 0 1 0 0 0 UCS2 (ISO/IEC-10646)
		return "UCS-2";
	case 0x09: //0 0 0 0 1 0 0 1 Pictogram Encoding
		//In cases where a Data Coding Scheme is defined for TDMA and/ or CDMA but not
		//defined for GSM, SMPP uses GSM 03.38 reserved values.
		return "GSM7";
	case 0x0A: //0 0 0 0 1 0 1 0 ISO-2022-JP (Music Codes)
		return "ISO-2022-JP";
	case 0x0E: // 0 0 0 0 1 1 1 0 KS C 5601
		return "ks_c_5601-1987";
	}

	if (
		(data_coding & 0xC0) || // 1 1 0 0 x x x x GSM MWI control - see [GSM 03.38]
		(data_coding & 0xD0) || // 1 1 0 1 x x x x GSM MWI control - see [GSM 03.38]
		(data_coding & 0xF0)	// 1 1 1 1 x x x x GSM message class control - see [GSM 03.38]
		) {
			//0xC0 and 0xD0: The data_coding parameter will evolve to specify Character code settings only. Thus the
			// recommended way to specify GSM MWI control is by specifying the relevant settings in
			// the optional parameters _ms_msg_wait_facilities and ms_validity.
			//0xF0: The data_coding parameter will evolve to specify Character code settings only. Thus the
			// recommended way to specify GSM message class control is by specifying the relevant
			// setting in the optional parameter dest_addr_subunit.
			//
			// messages has waiting indication, we do nothing here
	}

	//anyway, if nothing matches we fall into GSM7
	return "GSM7";
}

} // namespace opensmpp


/*static*/
string utf8ToGsm7(const string &src)
{
	int res;
	string gsm7Text;
	unsigned char rbuff[2];
	wstring wstr = utf8toWStr(src);
	for (size_t i = 0; i < wstr.size(); i++)
	{
		res = gsm7_wctomb(rbuff, (ucs4_t)wstr[i], 2);

		if (res > 0) {
			gsm7Text.insert(gsm7Text.end(), (const char*)rbuff, (const char*)(rbuff+res));
		} else {
			gsm7Text.insert(gsm7Text.end(), '?');
		}
	}

	return gsm7Text;
}

/*static*/
string gsm7ToUtf8(const string &src)
{
	wstring wstrText;
	size_t i = 0; int len;
	while ( i < src.size())
	{
		ucs4_t tmp;
		len = gsm7_mbtowc(&tmp, (const unsigned char *)&src[i], src.size()-i);
		i += len > 0 ? len : 1;
		if(len > 0) {
			wstrText.insert(wstrText.end(), (wchar_t)(tmp & 0xFFFF));
			/*wstrText.resize(wstrText.size() + 2);
			wstrText[wstrText.size() - 2] = (tmp & 0xFF00);
			wstrText[wstrText.size() - 1] = (tmp & 0x00FF);*/
		} else {
			wstrText.insert(wstrText.end(), (wchar_t)'?'); //send a '?'
		}
	}

	return wstrToUtf8(wstrText);
}

/*static*/
string wstrToUtf8(const wstring& src)
{
#ifdef _MSC_VER
# pragma warning(disable: 4333)	//  '>>' : right shift by too large amount, data loss (no aplica en Windows)
#endif
	string dest;
	for (size_t i = 0; i < src.size(); i++)
	{
		wchar_t w = src[i];
		if (w <= 0x7f) {
			dest.push_back((char)w);
		} else if (w <= 0x7ff) {
			dest.push_back(0xc0 | ((w >> 6)& 0x1f));
			dest.push_back(0x80| (w & 0x3f));
		} else if (w <= 0xffff) {
			dest.push_back(0xe0 | ((w >> 12)& 0x0f));
			dest.push_back(0x80| ((w >> 6) & 0x3f));
			dest.push_back(0x80| (w & 0x3f));
		} else if (w <= 0x10ffff) {
			dest.push_back(0xf0 | ((w >> 18)& 0x07));
			dest.push_back(0x80| ((w >> 12) & 0x3f));
			dest.push_back(0x80| ((w >> 6) & 0x3f));
			dest.push_back(0x80| (w & 0x3f));
		} else {
			dest.push_back('?');
		}
	}
	return dest;
}

/*static*/
wstring utf8toWStr(const string &src)
{
	wstring dest;
	wchar_t w = 0;
	int bytes = 0;
	wchar_t err = 0xfffd;

	for (size_t i = 0; i < src.size(); i++)
	{
		unsigned char c = (unsigned char)src[i];
		if (c <= 0x7f) { //first byte
			if (bytes) {
				dest.push_back(err);
				bytes = 0;
			}
			dest.push_back((wchar_t)c);
		} else if (c <= 0xbf) { //second/third/etc byte
			if (bytes) {
				w = ((w << 6)|(c & 0x3f));
				bytes--;
				if (bytes == 0) {
					dest.push_back(w);
				}
			} else {
				dest.push_back(err);
			}
		} else if (c <= 0xdf) { //2byte sequence start
			bytes = 1;
			w = c & 0x1f;
		} else if (c <= 0xef) { //3byte sequence start
			bytes = 2;
			w = c & 0x0f;
		} else if (c <= 0xf7) { //3byte sequence start
			bytes = 3;
			w = c & 0x07;
		} else {
			dest.push_back(err);
			bytes = 0;
		}
	}

	if (bytes) {
		dest.push_back(err);
	}

	return dest;
}
