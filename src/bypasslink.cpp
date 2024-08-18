#include <dpp/dpp.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include "../header/request.h"

using json = nlohmann::json;

dpp::task<void> bypass_link(std::string url, dpp::cluster& bot, const dpp::message_create_t& event) {
	json data = co_await url::bypass(url, bot);
	bot.log(dpp::ll_debug, data.dump().c_str());
	if (data["status"].get<std::string>() == "success") {

		std::string res = std::format("vượt link thành công!\nlink đầu ra: {}", data["result"].get<std::string>());
		
		event.reply(res);
	}
	else event.reply(std::format("có lỗi trong khi vượt : {}", data["message"].get<std::string>()));
}
dpp::task<void> bypass_link(std::string url, dpp::cluster& bot, const dpp::slashcommand_t& event) {
	json data = co_await url::bypass(url, bot);
	bot.log(dpp::ll_debug, data.dump().c_str());
	if (data["status"].get<std::string>() == "success") {

		std::string res = std::format("vượt link thành công!\nlink đầu ra: {}", data["result"].get<std::string>());

		event.co_edit_original_response(dpp::message(res));
	}
	else event.co_edit_original_response(dpp::message(std::format("có lỗi trong khi vượt : {}", data["message"].get<std::string>())));
}