#ifndef bypass_URL
#include <dpp/dpp.h>
dpp::task<void> bypass_link(std::string url, dpp::cluster& bot, const dpp::message_create_t& event);
dpp::task<void> bypass_link(std::string url, dpp::cluster& bot, const dpp::slashcommand_t& event);
#endif // !bypass_URL