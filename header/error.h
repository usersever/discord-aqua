#pragma once

#include <exception>
#include <string>
#include <dpp/dpp.h>
#include <fmt/format.h>

const dpp::embed error = dpp::embed()
    .set_color(0xff0000)
    .set_title("có lỗi xảy ra trong yêu cầu của bạn, lỗi này đã được báo tới dev💻");

class request_error : public std::exception {
private:
    std::string message;
    std::string file;
    std::string request;
    std::string response;

public:
    // Constructor with message, file, line, and request content
    request_error(const std::string& msg, const std::string& file, const std::string& request, const std::string& res)
        : message(msg), file(file), request(request), response(res) {}

    // Override the what() function from std::exception
    const char* what() const noexcept override {
        return message.c_str();
    }

    void send_error(dpp::cluster& bot, const dpp::slashcommand_t& event, dpp::snowflake dev_id) {
        dpp::message contain(fmt::format(fmt::runtime("Phát hiện lỗi yêu cầu:\nFile bị lỗi:{0}\nNội dung yêu cầu:{1}\nnội dung phản hồi:"), file, request));
        if (response.empty()) contain.add_file("response.txt", response);
        bot.direct_message_create(dev_id, contain);

        event.edit_original_response(dpp::message(error));
    }
};
#define REQUEST_ERROR(msg, request, response) \
    throw request_error(msg, __FILE__, request, response);