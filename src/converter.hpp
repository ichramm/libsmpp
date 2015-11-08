/*!
 * \file converter.hpp
 * \author ichramm
 *
 * Created on December 5, 2010, 1:14 AM
 */
#ifndef OPENSMPP_CONVERTER_HPP_
#define OPENSMPP_CONVERTER_HPP_

#include <string>

namespace opensmpp
{
	class CConverter
	{
	public:
		CConverter(const std::string &charsetFrom, const std::string &charsetTo);

		int Convert(const std::string &src, std::string &dest);

		static const char *GetCharsetFromDataCoding(unsigned char data_coding);

	private:
		std::string m_charsetFrom;
		std::string m_charsetTo;

		int ConvertInternal(const std::string &src, std::string &dest);
	};
} //namespace opensmpp

#endif // OPENSMPP_CONVERTER_HPP_
