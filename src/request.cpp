#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
using json = nlohmann::json;
dpp::task<json> get_report(std::string id, std::string apikey_vt, dpp::cluster& bot) {
	std::string url = "https://www.virustotal.com/api/v3/analyses/" + id;
	dpp::http_headers headers = {
		{"accept", "application/json"},
		{"x-apikey", apikey_vt}
	};
	std::string status = "start";
	json result;
	while (status != "completed") {
		dpp::http_request_completion_t response = co_await bot.co_request(url, dpp::m_get, "", "", headers);
		co_await bot.co_sleep(10);
		if (response.status == 200) {
			status = json::parse(response.body)["data"]["attributes"]["status"].get<std::string>();
			result = json::parse(response.body);
		}
		else {
			std::string error = json::parse(response.body)["error"]["message"].get<std::string>(),
				code = json::parse(response.body)["error"]["code"].get<std::string>();
			bot.log(dpp::ll_error, "scan error: " + std::to_string(response.status) +
				'\n' + error + '\n' + code);
			result = NULL;
			break;
		}
	}
	co_return result;
}
namespace file {
	/*gửi file đến virustotal khi file nhỏ hơn 32mb*/
	dpp::task<std::string> send_(std::string data_encode, std::string name, std::string filetype, dpp::cluster& bot, std::string apikey_vt) {
		std::string url = "https://www.virustotal.com/api/v3/files",
			//formdata có vấn đề
			formdata = "--form\r\nContent-Disposition: form-data; name=\"file\"; filename=\""
			+ name + "\"\r\nContent-Type: " + filetype + "\r\n\r\n" + data_encode +
			"\r\n--form--";
		/*"--form\r\nContent-Disposition: "
		"form-data; name=\"file\"\r\n\r\ndata:"+filetype+";name="+name+";base64,"
		+data_encode+"\r\n--form--\r\n\r\n";*/

		dpp::http_headers headers = {
			{"accept", "application/json"},
			{"x-apikey", apikey_vt},
			{"Content-Type", "multipart/form-data; boundary=form"}
		};
		dpp::http_request_completion_t request = co_await bot.co_request(url, dpp::m_post, formdata, "", headers);
		if (request.status == 200) {
			bot.log(dpp::ll_info, "sent file");
			std::string id = json::parse(request.body)["data"]["id"].get<std::string>();
			//đợi hỗ trợ dpp::message_create_t::co_reply

			/*event.reply("đang scan file...", true, [id, &bot, event](const dpp::confirmation_callback_t& callback) {
				get_result(bot, event, callback, id);
				});*/
			co_return id;
		}
		else {
			std::string error = json::parse(request.body)["error"]["message"],
				code = json::parse(request.body)["error"]["code"];
			bot.log(dpp::ll_error, "request error:" + code + '\n' + error);
		}
		co_return NULL;
	}

}
namespace url {
	dpp::task<std::string> send(std::string url, std::string apikey_vt, dpp::cluster &bot) {
		dpp::http_headers header = {
			{"accept","application/json"},
			{"x-apikey", apikey_vt}
		};
		dpp::http_request_completion_t callback = co_await bot.co_request("https://www.virustotal.com/api/v3/urls", dpp::m_post, "url=" + url, "application/x-www-form-urlencoded", header);
		if (callback.status == 200) {
			bot.log(dpp::ll_info, "sent url");
			std::string id = json::parse(callback.body)["data"]["id"].get<std::string>();
			co_return id;
		}
		else {
			std::string error = json::parse(callback.body)["error"]["message"],
				code = json::parse(callback.body)["error"]["code"];
			bot.log(dpp::ll_error, "request error:" + code + '\n' + error);
		}
		co_return NULL;
	}
	dpp::task<json> bypass(std::string url, dpp::cluster& bot) {
		dpp::http_headers header = {
			{"Content-Type","application/x-www-form-urlencoded"}
		};
		dpp::http_request_completion_t callback = co_await bot.co_request("https://api.bypass.vip/bypass?url=" + url, dpp::m_post, "", "text/plain", header);
		co_return json::parse(callback.body);
	}
}