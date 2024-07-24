#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/buffer.h>
#include <unicode/unistr.h>
#include <unicode/ucnv.h>
#include <unicode/ustream.h>

#pragma warning(disable : 4996)
namespace base64 {
    std::string encode_File(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
        BIO* bio, * b64;
        BUF_MEM* bufferPtr;
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, buffer.data(), buffer.size());
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);
        std::string base64String(bufferPtr->data, bufferPtr->length);
        size_t lastNonEqual = base64String.find_last_not_of('=');
        if (lastNonEqual != std::string::npos) {
            return base64String.substr(0, lastNonEqual + 1);
        }
        return base64String;
    }
    std::string encode_string(std::string url) {
        BIO* bio, * b64;
        BUF_MEM* bufferPtr;

        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, url.c_str(), url.size());
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);

        std::string base64String(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);

        return base64String;
    }
}
namespace sha256 {
    std::string encode_string(const std::string& data) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, data.c_str(), data.size());
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    std::string encode_file(std::string path) {
        std::ifstream file(path, std::ios::binary);
        std::string data(std::istreambuf_iterator<char>(file), {});
        return encode_string(data);
    }
}
std::string utf32_to_utf8(char32_t utf32_char) {
    std::string utf8_str;
    if (utf32_char <= 0x7F) {
        utf8_str.push_back(static_cast<char>(utf32_char));
    }
    else if (utf32_char <= 0x7FF) {
        utf8_str.push_back(static_cast<char>((utf32_char >> 6) | 0xC0));
        utf8_str.push_back(static_cast<char>((utf32_char & 0x3F) | 0x80));
    }
    else if (utf32_char <= 0xFFFF) {
        utf8_str.push_back(static_cast<char>((utf32_char >> 12) | 0xE0));
        utf8_str.push_back(static_cast<char>(((utf32_char >> 6) & 0x3F) | 0x80));
        utf8_str.push_back(static_cast<char>((utf32_char & 0x3F) | 0x80));
    }
    else if (utf32_char <= 0x10FFFF) {
        utf8_str.push_back(static_cast<char>((utf32_char >> 18) | 0xF0));
        utf8_str.push_back(static_cast<char>(((utf32_char >> 12) & 0x3F) | 0x80));
        utf8_str.push_back(static_cast<char>(((utf32_char >> 6) & 0x3F) | 0x80));
        utf8_str.push_back(static_cast<char>((utf32_char & 0x3F) | 0x80));
    }
    else {
        throw std::runtime_error("Invalid UTF-32 character.");
    }
    return utf8_str;
}

// Hàm để chuyển đổi chuỗi UTF-32 sang UTF-8
std::string utf32_to_utf8(const std::u32string& utf32_str) {
    std::string utf8_str;
    for (char32_t utf32_char : utf32_str) {
        utf8_str += utf32_to_utf8(utf32_char);
    }
    return utf8_str;
}