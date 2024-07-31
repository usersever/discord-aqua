#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <regex>
#include <fmt/format.h>
#include <mysql/mysql.h>
#include <cstdlib>
#include "header/game.h"
#include "header/youtubeapi.h"
#include "header/bypasslink.h"
#include "header/AIapi.h"
#include "header/error.h"

using json = nlohmann::json;

constexpr dpp::snowflake dev_id = 1036979020568477747;

dpp::emoji_map bot_emojis;

std::string token;
unsigned int port = 0;

#define REPLIT false
#if REPLIT

void get_data(MYSQL*& db) {
    token = std::getenv("discord");
    apikey = std::getenv("youtube");
    gemini_api = std::getenv("gemini");
    db = mysql_init(NULL);
    if (!db) {
        throw std::exception("sql init failed");
    }
    if (mysql_ssl_set(db, NULL, NULL, NULL, NULL, NULL)) {
        throw std::exception(mysql_error(db));
    }
    if (mysql_real_connect(db, std::getenv("address"), std::getenv("username"), std::getenv("password"), std::getenv("name"), std::stoi(std::getenv("port")), nullptr, 0) == NULL) {
        throw std::exception(mysql_error(db));
    }
}
#else
void get_data(MYSQL*& db) {
    std::ifstream f("token.json");
    json j = json::parse(f);
    if (j["discord"].is_string() && j["youtube"].is_string() && j["gemini"].is_string()
        
        && j["database"]["address"].is_string() && j["database"]["name"].is_string() && j["database"]["username"].is_string()&& j["database"]["password"].is_string()&& j["database"]["port"].is_number_integer()
        ) 
    {
        token = j["discord"].get<std::string>();
        apikey = j["youtube"].get<std::string>();
        gemini_api = j["gemini"].get<std::string>();
        db = mysql_init(NULL);
        if (!db) {
            throw std::exception("sql init failed");
        }
        if (j["database"]["ssl"].is_boolean() && j["database"]["ssl"].get<bool>()) {
            if (mysql_ssl_set(db, NULL, NULL, NULL, NULL, NULL)) {
                throw std::exception(mysql_error(db));
            }
        }
        if (mysql_real_connect(db, j["database"]["address"].get<std::string>().c_str(), j["database"]["username"].get<std::string>().c_str(), j["database"]["password"].get<std::string>().c_str(), j["database"]["name"].get<std::string>().c_str(), j["database"]["port"].get<int>(), nullptr, 0) == NULL) {
            throw std::exception(mysql_error(db));
        }
    }
    else throw std::exception("something are missing");
}

#endif

bool isURL(const std::string& str) {
    std::regex url_pattern(
        R"((https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w\.-]*)*\/?\b)",
        std::regex_constants::icase
    );

    return std::regex_match(str, url_pattern);
}

std::vector<std::string> findURLs(std::string text) {
    std::regex url_regex(R"((https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w\.-]*)*\/?\b)");
    std::smatch matches;
    std::vector<std::string> urls;
    while (std::regex_search(text, matches, url_regex)) {
        urls.push_back(matches[0]);
        text = matches.suffix().str();
    }
    return urls;
}

signed main() {
    MYSQL* database;
    try {
        get_data(database);
    }
    catch (std::exception ex) {
        std::printf(ex.what());
        return -1;
    }
    dpp::cluster bot(token, dpp::i_default_intents | dpp::i_message_content);

    bot.on_log(dpp::utility::cout_logger());


    bot.on_ready([&bot](const dpp::ready_t& event) {
        bot.application_emojis_get([](const dpp::confirmation_callback_t& callback) {
            if (callback.is_error()) return;
            bot_emojis = callback.get<dpp::emoji_map>();
            });

        if (dpp::run_once<struct register_bot_commands>()) {
            dpp::slashcommand info("info", "thông tin bot", bot.me.id),
                help("help", "xem lệnh của bot", bot.me.id),
                seach("seach", "tìm video(không hỗ trợ stream)", bot.me.id),
                bypass("bypass", "vượt link", bot.me.id),
                /*addemoji("add_emoji", "thêm emoji vào máy chủ", bot.me.id),*/
                aichat("ask", "trò chuyện với AI", bot.me.id),
                game("game", "chơi trò chơi", bot.me.id),
                reg("register", "register", bot.me.id);

            seach.add_option(dpp::command_option(dpp::co_string, "text", "text", true));

            bypass.add_option(dpp::command_option(dpp::co_string, "url ", "nhập url", true));

            /*addemoji.set_default_permissions(dpp::p_manage_emojis_and_stickers);
            addemoji.add_option(dpp::command_option(dpp::co_string, "emoji", "nhập emoji", true));*/

            aichat.add_option(dpp::command_option(dpp::co_string, "content", "nội dung muốn hỏi", true))
                .add_option(dpp::command_option(dpp::co_integer, "module", "chọn module AI", true)
                    .add_choice(dpp::command_option_choice("gemini-1.5-flash", 1))
                    .add_choice(dpp::command_option_choice("gemini-1.0-pro", 2))
                )
                .add_option(dpp::command_option(dpp::co_attachment, "file", "tệp dính kèm"));
            

            game.add_option(dpp::command_option(dpp::co_string, "game", "chọn trò chơi", true)
                    .add_choice(dpp::command_option_choice("blackjack", "blackjack"))
                )
                .add_option(dpp::command_option(dpp::co_integer, "amount", "số tiền cược", true));

            bot.global_bulk_command_create({ info,seach,bypass,help,/*addemoji,*/aichat,game,reg });
        }
        });

    // lệnh '/'
    bot.on_slashcommand([&bot, &database](const dpp::slashcommand_t& event) -> dpp::task<void> {
        
        event.co_thinking(false);

        //giới thiệu
        if (event.command.get_command_name() == "info") {
            dpp::embed text = dpp::embed().
                set_color(0x0099ff).
                set_title("aqua bot").
                set_url("https://www.youtube.com/watch?v=dQw4w9WgXcQ").
                set_description("bot của anos :)").
                set_thumbnail("https://www.nautiljon.com/images/perso/00/20/anos_voldigoad_19602.webp?1680033786").
                set_timestamp(time(0));
            dpp::message about(event.command.channel_id, text);
            event.co_edit_original_response(about);
        }

        //lệnh bảo trì
        if (event.command.get_command_name() == "bypass") {
            if (event.command.usr.id == dev_id) {
                std::string url = std::get<std::string>(event.get_parameter("url"));
                if (isURL(url))
                    co_await bypass_link(url, bot, event);
                else
                    event.co_edit_original_response(dpp::message("không chứa url trong tin nhắn"));
            }
            else event.co_edit_original_response(dpp::message("lệnh đang bảo trì"));
        }
        if (event.command.get_command_name() == "ask") {
            
            std::string data = std::get<std::string>(event.get_parameter("content"));

            int module = std::get<int64_t>(event.get_parameter("module"));

            bool have_file = std::holds_alternative<std::monostate>(event.get_parameter("file"));

            dpp::attachment file(NULL);
            
            if (!have_file) {
                // parameter is not empty
                dpp::snowflake file_id = std::get<dpp::snowflake>(event.get_parameter("file"));
                file = event.command.get_resolved_attachment(file_id);
            }
            try {
                event.edit_original_response(co_await(have_file ? gemini(bot, data, module) : gemini(bot, data, file, module)));
            }
            catch (request_error& ex) {
                bot.log(dpp::ll_error, ex.what());
                ex.send_error(bot, event, dev_id);
            }
            catch (std::exception& ex) {
                bot.log(dpp::ll_critical, ex.what());
                event.edit_original_response(dpp::message("có lỗi ngiêm trọng xảy ra"));
            }
        }

        //tìm video
        if (event.command.get_command_name() == "seach") {
            std::string text = std::get<std::string>(event.get_parameter("text"));
            co_await seach(text, bot, event);
        }

        if (event.command.get_command_name() == "register") {
            try {
                co_await reg(database, event.command.usr.id, event);
            }
            catch (request_error& ex) {
                bot.log(dpp::ll_error, ex.what());
                ex.send_error(bot, event, dev_id);
            }
            catch (std::exception& ex) {
                bot.log(dpp::ll_critical, ex.what());
                event.edit_original_response(dpp::message("có lỗi ngiêm trọng xảy ra"));
            }
        }

        if (event.command.get_command_name() == "game") {
            std::string game = std::get<std::string>(event.get_parameter("game"));
            dpp::snowflake user = event.command.usr.id;
            int amount = std::get<int64_t>(event.get_parameter("amount"));
            if (game == "blackjack") {
                try {
                    blackjack a(user, amount);
                    co_await a.play(database, event);
                }
                catch (request_error& ex) {
                    bot.log(dpp::ll_error, ex.what());
                    ex.send_error(bot, event, dev_id);
                }
                catch (std::exception& ex) {
                    bot.log(dpp::ll_critical, ex.what());
                    event.edit_original_response(dpp::message("có lỗi ngiêm trọng xảy ra"));
                }
            }
        }

        //trợ giúp
        if (event.command.get_command_name() == "help") {
            dpp::message text(event.command.channel_id,dpp::embed()
                .add_field("slash command", "tạm thời chưa sử dụng được")
                .add_field("command", "sẽ update sau")
                .set_timestamp(time(0)));
            event.co_edit_original_response(text);
        }
        /*bot.on_voice_ready([&bot, event](const dpp::voice_ready_t& voice) {
            play(bot, event, voice);
            });*/
        });
    //bot.on_voice_track_marker(track_maker);

    //lệnh tin nhắn

    bot.on_message_create([&bot](const dpp::message_create_t& event) -> dpp::task<void> 
        {
        if (event.msg.author.is_bot()) {
			co_return;
		}
        if (event.msg.content.find("!bypass") != std::string::npos) {
            if (event.msg.author.id == dev_id)
            {
                std::vector<std::string> urls = findURLs(event.msg.content);
                if (urls.size() < 1) co_return;
                else if (urls.size() == 1) {
                    bot.log(dpp::ll_debug, urls[0]);
                    co_await bypass_link(urls[0], bot, event);
                }
                else event.reply("bạn chỉ có thể vượt tối đa 1 url cùng lúc");

                co_return;
            }
            else event.reply("lệnh đang bảo trì");
        }
        }
    );

    bot.start(dpp::st_wait);

    mysql_close(database);
    return EXIT_SUCCESS;
}