#ifndef AIAPI_H
#define AIAPI_H
#include <dpp/dpp.h>
extern std::string gemini_api;

dpp::task<dpp::message> gemini(dpp::cluster& bot, std::string text, unsigned int module);
dpp::task<dpp::message> gemini(dpp::cluster& bot, const std::string text, dpp::attachment file, unsigned int module);
#endif // !AIAPI_H
