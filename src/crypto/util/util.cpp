#include"util.hpp"
#include <stdexcept>

const char* hexDigits = "0123456789abcdef";

std::string dataToHex(const char* digest, size_t len)
{
	auto result = std::string();
	result.reserve(len * 2);

	for (auto i = 0; i < len; ++i)
	{
		result.push_back(hexDigits[(digest[i] >> 4) & 0x0F]);
		result.push_back(hexDigits[digest[i] & 0x0F]);
	}

	return result;
}

uint8_t hexCharToValue(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    throw std::invalid_argument("Invalid hex character");
}

std::vector<uint8_t> hexToData(const std::string& digest)
{
    if (digest.size() % 2 != 0)
    {
        throw std::invalid_argument("Hex string must have an even length");
    }

    std::vector<uint8_t> data;
    data.reserve(digest.size() / 2);

    for (size_t i = 0; i < digest.size(); i += 2)
    {
        uint8_t high = hexCharToValue(digest[i]) << 4;
        uint8_t low = hexCharToValue(digest[i + 1]);
        data.push_back(high | low);
    }

    return data;
}