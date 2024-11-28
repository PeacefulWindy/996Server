#pragma once
#include<string>
#include <vector>

std::string dataToHex(const char* digest,size_t len);
std::vector<uint8_t> hexToData(const std::string& digest);