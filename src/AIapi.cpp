#include <dpp/dpp.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include "../header/Encode-decode.h"
#include "../header/error.h"
using json = nlohmann::json;

std::string gemini_api, openAI_api, endpoint;
bool use_azure_openAI = false;

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

dpp::task<dpp::message> chatgpt(dpp::cluster& bot, const std::string text, unsigned int module) {
	dpp::http_headers header;
	json data;

	std::string url = use_azure_openAI ?
		fmt::format("{0}/openai/deployments/{1}/chat/completions?api-version=2024-02-15-preview", endpoint, std::string(module == 3 ? "gpt-4o" : "gpt-4o-mini"))
		: "https://api.openai.com/v1/chat/completions";

	if (use_azure_openAI) {
		header = {
			{"Content-Type", "application/json"},
			{"api-key", openAI_api}
		};
		//data = json::parse(fmt::format(fmt::runtime(R"(
		//{
		//    "messages": [
		//        {
		//            "role": "system",
		//            "content": [
		//                {
		//                    "type": "text",
		//                    "text": "You are an AI assistant that helps people find information, solving problem and many thing orther"
		//                }
		//            ]
		//        },
		//        {
		//            "role": "user",
		//            "content": [
		//                {
		//                    "type": "text",
		//                    "text": "{0}"
		//                }
		//            ]
		//        }
		//    ],
		//    "temperature": 0.7,
		//    "top_p": 0.95,
		//    "max_tokens": 800
		//}
		//)"), text));
		data = {
		{"message", {{
			{"role","system"},
			{"content",{
				{"type", "text"},
				{"text", "You are an AI assistant that helps people find information, solving problem and many thing orther"}
			}}
		},
		{
			{"role","user"},
			{"content",{
				{"type", "text"},
				{"text", text}
			}}
		}
		}},
			{"temperature",0.7},
			{"top_p",0.95},
			{"max_tokens",800}
		};
	}
	else {
		header = {
			{"Content-Type", "application/json"},
			{"Authorization", openAI_api}
		};
		//data = json::parse(fmt::format(fmt::runtime(R"(
		//	{
		//	    "model": "{0}",
		//	    "messages": [
		//			{
		//				"role": "system",
		//				"content": "You are a poetic assistant, skilled in explaining complex programming concepts with creative flair."
		//			},
		//			{
		//				"role": "user",
		//				"content": "{1}"
		//			}
		//	    ]
		//	}
		//	)"), std::string(module == 3 ? "gpt-4o" : "gpt-4o-mini"), text));
		data = {
		{"message", {{
			{"role","system"},
			{"content","You are a poetic assistant, skilled in explaining complex programming concepts with creative flair."}
		},
		{
			{"role","user"},
			{"content",text}
		}
		}},
			{"model",std::string(module == 3 ? "gpt-4o" : "gpt-4o-mini")}
		};
	}

	dpp::http_request_completion_t response = co_await bot.co_request(url, dpp::m_post, data.dump(0), "", header);
	
	bot.log(dpp::ll_debug, response.body);
	if (response.status == 200) {
		co_return dpp::message(json::parse(response.body)["choices"]["message"]["content"].get<std::string>()).add_embed(warm);
	}
	else {
		REQUEST_ERROR("response error, code:"+std::to_string(response.status), data.dump(0), response.body);
	}
}