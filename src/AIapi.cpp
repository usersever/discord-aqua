#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <fmt/format.h>
#include "../header/Encode-decode.h"
#include "../header/error.h"
using json = nlohmann::json;

std::string gemini_api;

const dpp::embed warm = dpp::embed()
	.set_color(0xFFFF00)
	.set_description("AI có thể mắc lỗi. Hãy kiểm tra các thông tin quan trọng.");


//gemini với văn bản
dpp::task<dpp::message> gemini(dpp::cluster& bot, std::string text,unsigned int module) {
	dpp::http_headers header = {
		{"Content-Type","application/json"}
	};
	json data = {
		{"contents" ,
			{{{"parts",{{"text",text}}}}}
		}
	};
	dpp::http_request_completion_t response = co_await bot.co_request(fmt::format("https://generativelanguage.googleapis.com/v1/models/{1}:generateContent?key={0}", gemini_api, module == 1 ? "gemini-1.5-flash" : "gemini-1.0-pro"), dpp::m_post, data.dump(0), "", header);
	json res = json::parse(response.body);
	if (response.status == 200 && !res.contains("error")) {
		co_return dpp::message(res["candidates"][0]["content"]["parts"][0]["text"].get<std::string>()).add_embed(warm);
	}
	else {
		REQUEST_ERROR("response error", data.dump(0), response.body);
	}
}

//gemini với văn bản và hình ảnh
dpp::task<dpp::message> gemini(dpp::cluster& bot, const std::string text, dpp::attachment file, unsigned int module) {
	dpp::http_headers header = {
		{"Content-Type","application/json"}
	};
	dpp::http_request_completion_t img = co_await bot.co_request(file.url, dpp::m_get);
	//unsigned const char* i = img.body.c_str();
	const std::string img_encode = dpp::base64_encode((unsigned const char*)img.body.c_str(), img.body.size());


	/*format of json data: 
	 *	{
	 *	  "contents": [
	 *		{
	 *		  "parts": [
	 *			{
	 *			  "text": "question"
	 *			},
	 *			{
	 *			  "inline_data": {
	 *				"data": "base64 file",
	 *				"mime_type": "image type"
	 *			  }
	 *			}
	 *		  ]
	 *		}
	 *	  ]
	 *	}
	 */

	json data = {
		{"contents" ,
			{{{"parts",{
				{{"text",text}},
				{{"inline_data",
				{{"mime_type",file.content_type},{"data",img_encode}}
				}}
				}}}}
		}
	};
	dpp::http_request_completion_t response = co_await bot.co_request(fmt::format("https://generativelanguage.googleapis.com/v1/models/{1}:generateContent?key={0}", gemini_api, module == 1 ? "gemini-1.5-flash" : "gemini-1.0-pro"), dpp::m_post, data.dump(), "", header);
	json res = json::parse(response.body);
	if (response.status == 200 && !res.contains("error")) {
		co_return dpp::message(res["candidates"][0]["content"]["parts"][0]["text"].get<std::string>()).add_embed(warm);
	}
	else {
		REQUEST_ERROR("response error", data.dump(0), response.body);
	}
}