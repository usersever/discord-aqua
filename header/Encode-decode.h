#ifndef encode_decode_H
#define encode_decode_H

#include <string>
namespace base64 {
	std::string encode_File(const std::string& filename);
	std::string encode_string(std::string url);
}
namespace sha256{
	std::string encode_string(const std::string& data);
	std::string encode_file(std::string path);
}
std::string utf32_to_utf8(const std::u32string& utf32_str);
#endif //encode_decode_H