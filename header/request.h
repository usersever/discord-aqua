#ifndef REQUEST_VT
#define REQUEST_VT
#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
using json = nlohmann::json;
dpp::task<json> get_report(std::string id, std::string apikey_vt, dpp::cluster& bot);
namespace file {
	/*gửi file đến virustotal khi file nhỏ hơn 32mb*/
	dpp::task<std::string> send_(std::string data_encode, std::string name, std::string filetype, dpp::cluster& bot, std::string apikey_vt);
}
namespace url {
	//gửi url đến virustotal
	dpp::task<std::string> send(std::string url, std::string apikey_vt, dpp::cluster& bot);
	//bypass link
	dpp::task<json> bypass(std::string url, dpp::cluster& bot);
}
#endif REQUEST_VT