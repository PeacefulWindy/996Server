#include"encode.hpp"
#include<unicode/ucnv.h>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32


std::string getEncodeString(EncodeType code);

std::pair<bool, std::string> changeEncode(std::string& value, EncodeType fromEncode, EncodeType toEncode)
{
	if (value.empty())
	{
		return { true, std::string() };
	}

	auto fromEncodeStr = getEncodeString(fromEncode);
	auto toEncodeStr = getEncodeString(toEncode);

	auto err = U_ZERO_ERROR;

	auto targetLen = value.length() * 4;
	auto data = value.data();
	auto pos = 0;
	auto outBuf = std::vector<char>(targetLen);
	auto outputLen = ucnv_convert(toEncodeStr.c_str(), fromEncodeStr.c_str(), outBuf.data(), static_cast<int32_t>(targetLen), data, static_cast<int32_t>(value.length() - pos), &err);
	if (err != U_ZERO_ERROR)
	{
		return std::pair<bool, std::string>(false, std::string());
	}

	return std::pair<bool, std::string>(true, std::string(outBuf.data(), outputLen));
}

std::string getSystemCodePage()
{
#ifdef _WIN32
	auto codePage = GetACP();//https://learn.microsoft.com/zh-cn/windows/win32/intl/code-page-identifiers
	switch (codePage)
	{
	case 936:
		return "gb2312";
	case 65001:
		return "utf-8";
	default:
		return "utf-8";
	}
#else
	return "utf-8";
#endif
}

std::string getEncodeString(EncodeType code)
{
	switch (code)
	{
	case EncodeType::IBM037:
		break;
	case EncodeType::IBM437:
		break;
	case EncodeType::IBM500:
		break;
	case EncodeType::ASMO_708:
		break;
	case EncodeType::DOS_720:
		break;
	case EncodeType::IBM737:
		break;
	case EncodeType::IBM775:
		break;
	case EncodeType::IBM850:
		break;
	case EncodeType::IBM852:
		break;
	case EncodeType::IBM855:
		break;
	case EncodeType::IBM857:
		break;
	case EncodeType::IBM00858:
		break;
	case EncodeType::IBM860:
		break;
	case EncodeType::IBM861:
		break;
	case EncodeType::DOS_862:
		break;
	case EncodeType::IBM863:
		break;
	case EncodeType::IBM864:
		break;
	case EncodeType::IBM865:
		break;
	case EncodeType::CP866:
		break;
	case EncodeType::IBM869:
		break;
	case EncodeType::IBM870:
		break;
	case EncodeType::WINDOWS_874:
		break;
	case EncodeType::CP875:
		break;
	case EncodeType::SHIFT_JIS:
		break;
	case EncodeType::GB2312:
		return "gb2312";
	case EncodeType::KS_C_5601_1987:
		break;
	case EncodeType::BIG5:
		break;
	case EncodeType::IBM1026:
		break;
	case EncodeType::IBM01047:
		break;
	case EncodeType::IBM01140:
		break;
	case EncodeType::IBM01141:
		break;
	case EncodeType::IBM01142:
		break;
	case EncodeType::IBM01143:
		break;
	case EncodeType::IBM01144:
		break;
	case EncodeType::IBM01145:
		break;
	case EncodeType::IBM01146:
		break;
	case EncodeType::IBM01147:
		break;
	case EncodeType::IBM01148:
		break;
	case EncodeType::IBM01149:
		break;
	case EncodeType::UTF16:
		return "utf-16";
	case EncodeType::UNICODEFFFE:
		break;
	case EncodeType::WINDOWS_1250:
		break;
	case EncodeType::WINDOWS_1251:
		break;
	case EncodeType::WINDOWS_1252:
		break;
	case EncodeType::WINDOWS_1253:
		break;
	case EncodeType::WINDOWS_1254:
		break;
	case EncodeType::WINDOWS_1255:
		break;
	case EncodeType::WINDOWS_1256:
		break;
	case EncodeType::WINDOWS_1257:
		break;
	case EncodeType::WINDOWS_1258:
		break;
	case EncodeType::JOHAB:
		break;
	case EncodeType::MACINTOSH:
		break;
	case EncodeType::X_MAC_JAPANESE:
		break;
	case EncodeType::X_MAC_CHINESETRAD:
		break;
	case EncodeType::X_MAC_KOREAN:
		break;
	case EncodeType::X_MAC_ARABIC:
		break;
	case EncodeType::X_MAC_HEBREW:
		break;
	case EncodeType::X_MAC_GREEK:
		break;
	case EncodeType::X_MAC_CYRILLIC:
		break;
	case EncodeType::X_MAC_CHINESESIMP:
		break;
	case EncodeType::X_MAC_romanian:
		break;
	case EncodeType::X_MAC_ukrainian:
		break;
	case EncodeType::X_MAC_THAI:
		break;
	case EncodeType::X_MAC_CE:
		break;
	case EncodeType::X_MAC_ICELANDIC:
		break;
	case EncodeType::X_MAC_TURKISH:
		break;
	case EncodeType::X_MAC_CROATIAN:
		break;
	case EncodeType::X_CHINESE_CNS:
		break;
	case EncodeType::X_CP20001:
		break;
	case EncodeType::X_CHINESE_ETEN:
		break;
	case EncodeType::X_CP20003:
		break;
	case EncodeType::X_CP20004:
		break;
	case EncodeType::X_CP20005:
		break;
	case EncodeType::X_IA5:
		break;
	case EncodeType::X_IA5_GERMAN:
		break;
	case EncodeType::X_IA5_SWEDISH:
		break;
	case EncodeType::X_IA5_NORWEGIAN:
		break;
	case EncodeType::US_ASCII:
		break;
	case EncodeType::X_CP20261:
		break;
	case EncodeType::X_CP20269:
		break;
	case EncodeType::IBM273:
		break;
	case EncodeType::IBM277:
		break;
	case EncodeType::IBM278:
		break;
	case EncodeType::IBM280:
		break;
	case EncodeType::IBM284:
		break;
	case EncodeType::IBM285:
		break;
	case EncodeType::IBM290:
		break;
	case EncodeType::IBM297:
		break;
	case EncodeType::IBM420:
		break;
	case EncodeType::IBM423:
		break;
	case EncodeType::IBM424:
		break;
	case EncodeType::X_EBCDIC_KOREANEXTENDED:
		break;
	case EncodeType::IBM_THAI:
		break;
	case EncodeType::KOI8_R:
		break;
	case EncodeType::IBM871:
		break;
	case EncodeType::IBM880:
		break;
	case EncodeType::IBM905:
		break;
	case EncodeType::IBM00924:
		break;
	case EncodeType::EUC_JP:
		break;
	case EncodeType::X_CP20936:
		break;
	case EncodeType::X_CP20949:
		break;
	case EncodeType::CP1025:
		break;
	case EncodeType::KOI8_U:
		break;
	case EncodeType::ISO_8859_1:
		break;
	case EncodeType::ISO_8859_2:
		break;
	case EncodeType::ISO_8859_3:
		break;
	case EncodeType::ISO_8859_4:
		break;
	case EncodeType::ISO_8859_5:
		break;
	case EncodeType::ISO_8859_6:
		break;
	case EncodeType::ISO_8859_7:
		break;
	case EncodeType::ISO_8859_8:
		break;
	case EncodeType::ISO_8859_9:
		break;
	case EncodeType::ISO_8859_13:
		break;
	case EncodeType::ISO_8859_15:
		break;
	case EncodeType::X_EUROPA:
		break;
	case EncodeType::ISO_8859_8_I:
		break;
	case EncodeType::ISO_2022_JP:
		break;
	case EncodeType::CSISO2022JP:
		break;
	case EncodeType::ISO_2022_KR:
		break;
	case EncodeType::X_CP50227:
		break;
	case EncodeType::EUC_CN:
		break;
	case EncodeType::EUC_KR:
		break;
	case EncodeType::HZ_GB_2312:
		break;
	case EncodeType::GB18030:
		break;
	case EncodeType::X_ISCII_DE:
		break;
	case EncodeType::X_ISCII_BE:
		break;
	case EncodeType::X_ISCII_TA:
		break;
	case EncodeType::X_ISCII_TE:
		break;
	case EncodeType::X_ISCII_AS:
		break;
	case EncodeType::X_ISCII_OR:
		break;
	case EncodeType::X_ISCII_KA:
		break;
	case EncodeType::X_ISCII_MA:
		break;
	case EncodeType::X_ISCII_GU:
		break;
	case EncodeType::X_ISCII_PA:
		break;
	case EncodeType::UTF7:
		break;
	case EncodeType::UTF8:
		return "utf-8";
	case EncodeType::UTF32:
		return "utf-32";
	case EncodeType::UTF32BE:
		break;
	default:
		return getSystemCodePage();
	}
}