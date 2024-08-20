#include <dpp/dpp.h>
#include <dpp/unicode_emoji.h>
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

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#elif __linux__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif

using json = nlohmann::json;

constexpr dpp::snowflake dev_id = 1036979020568477747;

dpp::emoji_map bot_emojis;

std::string token;
unsigned int port = 0;

double getCpuUsage() {
    double usage = 0.0;
#ifdef _WIN32
    FILETIME idleTime, kernelTime, userTime;
    GetSystemTimes(&idleTime, &kernelTime, &userTime);

    ULARGE_INTEGER a, b;
    a.LowPart = idleTime.dwLowDateTime;
    a.HighPart = idleTime.dwHighDateTime;

    b.LowPart = kernelTime.dwLowDateTime + userTime.dwLowDateTime;
    b.HighPart = kernelTime.dwHighDateTime + userTime.dwHighDateTime;

    double idle = a.QuadPart;
    double kernelUser = b.QuadPart;

    usage = (1 - idle / kernelUser) * 100;
#elif __linux__
    // Code to get CPU Usage on Linux
    char cpuUsageCommand[50];
    FILE* pipe = popen("top -b -n 1 | grep Cpu | awk '{print $2}'", "r");
    if (!pipe) return 0.0;

    fscanf(pipe, "%lf", &usage);
    pclose(pipe);
#endif

    return usage;
}

double getRamUsage() {
    double usage = 0.0;
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));

    usage = static_cast<double>(pmc.WorkingSetSize);
#elif __linux__
    // Code to get RAM Usage on Linux
    char ramUsageCommand[50];
    FILE* pipe = popen("free | grep Mem | awk '{print $3 * 1024}'", "r");
    if (!pipe) return 0.0;

    fscanf(pipe, "%lf", &usage);
    pclose(pipe);
#endif

    return usage;
}

double getRamUsage_percent() {
    double usage = 0.0;
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    usage = ((memInfo.ullTotalPhys - memInfo.ullAvailPhys) * 100) / memInfo.ullTotalPhys;
#elif __linux__
    // Code to get RAM Usage on Linux
    char ramUsageCommand[50];
    FILE* pipe = popen("free | grep Mem | awk '{print $3/$2 * 100.0}'", "r");
    if (!pipe) return 0.0;

    fscanf(pipe, "%lf", &usage);
    pclose(pipe);
#endif

    return usage;
}

#ifdef REPLIT
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
    if (!f.is_open()) {
        throw std::runtime_error("can't open \"token.json\" , check the file");
    }
    //std::string a;
    //while (std::getline(f, a)) {
    //    std::cout << a;
    //}

    json j = json::parse(f);
    if (j["discord"].is_string() && j["youtube"].is_string() && j["gemini"].is_string()
        
        && j["database"]["address"].is_string() && j["database"]["name"].is_string() && j["database"]["username"].is_string()&& j["database"]["password"].is_string()&& j["database"]["port"].is_number_integer()
        ) 
    {
        token = j["discord"].get<std::string>();
        apikey = j["youtube"].get<std::string>();
        gemini_api = j["gemini"].get<std::string>();

        //dùng azure openAI
        if (j.contains("openAI-azure") && j.contains("endpoint") && !j.contains("openAI")) {
            use_azure_openAI = true;
            openAI_api = j["openAI-azure"].get<std::string>();
            endpoint = j["endpoint"].get<std::string>();
        }
        // dùng openAI
        else if (j.contains("openAI") && !j.contains("openAI-azure")) {
            openAI_api = j["openAI"].get<std::string>();
        }
        // các lỗi
        else if (j.contains("openAI-azure") && !j.contains("endpoint") && !j.contains("openAI")) {
            throw std::runtime_error("you are missing \"endpoint\", edit it in token.json");
        }
        else if (j.contains("openAI-azure") && j.contains("endpoint") && j.contains("openAI")) {
            throw std::runtime_error("you can only use one in one season, remove \"openAI-azure\" and \"endpoint\" or \"openAI\"");
        }

        //khởi tạo kết nối
        db = mysql_init(nullptr);
        if (db == nullptr) {
            throw std::runtime_error("mysql_init() failed");
        }

        // Cấu hình SSL nếu cần thiết
        if (j["database"]["ssl"].is_boolean() && j["database"]["ssl"].get<bool>()) {
            if (mysql_ssl_set(db, nullptr, nullptr, nullptr, nullptr, nullptr)) {
                std::string error_message = "mysql_ssl_set() failed: " + std::string(mysql_error(db));
                mysql_close(db);
                throw std::runtime_error(error_message);
            }
        }

        // Kết nối đến cơ sở dữ liệu
        std::string host = j["database"]["address"].get<std::string>().c_str();
        std::string user = j["database"]["username"].get<std::string>().c_str();
        std::string password = j["database"]["password"].get<std::string>().c_str();
        std::string database = j["database"]["name"].get<std::string>().c_str();
        unsigned int port = j["database"]["port"].get<int>();

        if (mysql_real_connect(db, host.c_str(), user.c_str(), password.c_str(), database.c_str(), port, nullptr, 0) == nullptr) {
            std::string error_message = "mysql_real_connect() failed: " + std::string(mysql_error(db));
            mysql_close(db);
            throw std::runtime_error(error_message);
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
    catch (json::exception ex) {
        std::cerr << ex.what();
        return -1;
    }
    catch (std::exception ex) {
        std::cerr << ex.what();
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
            dpp::slashcommand info("info", "xem trạng thái bot", bot.me.id),
                help("help", "xem lệnh của bot", bot.me.id),
                seach("search", "tìm video(không hỗ trợ stream)", bot.me.id),
                bypass("bypass", "vượt link", bot.me.id),
                /*addemoji("add_emoji", "thêm emoji vào máy chủ", bot.me.id),*/
                aichat("ask", "trò chuyện với AI", bot.me.id),
                game("game", "chơi trò chơi", bot.me.id),
                reg("register", "register", bot.me.id),
                daily("daily", "nhận tiền mỗi ngày", bot.me.id),
                balance("balance", "xem số dư của bạn", bot.me.id);

            seach.add_option(dpp::command_option(dpp::co_string, "text", "text", true));

            bypass.add_option(dpp::command_option(dpp::co_string, "url ", "nhập url", true));

            /*addemoji.set_default_permissions(dpp::p_manage_emojis_and_stickers);
            addemoji.add_option(dpp::command_option(dpp::co_string, "emoji", "nhập emoji", true));*/

            aichat.add_option(dpp::command_option(dpp::co_string, "content", "nội dung muốn hỏi", true))
                .add_option(dpp::command_option(dpp::co_integer, "module", "chọn module AI", true)
                    .add_choice(dpp::command_option_choice("gemini-1.5-flash", 1))
                    .add_choice(dpp::command_option_choice("gemini-1.0-pro", 2))
                    .add_choice(dpp::command_option_choice("chatgpt-4o",3))
                    .add_choice(dpp::command_option_choice("chatgpt-4o-mini", 4))
                )
                .add_option(dpp::command_option(dpp::co_attachment, "file", "tệp dính kèm"));
            

            game.add_option(dpp::command_option(dpp::co_sub_command, "blackjack", "Game blackjack")
                .add_option(dpp::command_option(dpp::co_integer, "amount", "Số tiền cược", true)))
                .add_option(dpp::command_option(dpp::co_sub_command, "coinflip", "Game coinflip")
                    .add_option(dpp::command_option(dpp::co_integer, "amount", "Số tiền cược", true))
                    .add_option(dpp::command_option(dpp::co_integer, "coin-face", "Mặt xu bạn cược", true)
                        .add_choice(dpp::command_option_choice(std::string(dpp::unicode_emoji::white_circle) + " mặt ngửa", 0))
                        .add_choice(dpp::command_option_choice(std::string(dpp::unicode_emoji::black_circle) + " mặt úp", 1))));

            bot.global_bulk_command_create({ info,seach,bypass,help,/*addemoji,*/aichat,game,reg,daily,balance });
        }
        });

    // lệnh '/'
    bot.on_slashcommand([&bot, &database](const dpp::slashcommand_t& event) -> dpp::task<void> {

        event.co_thinking(false);

        //giới thiệu
        /*if (event.command.get_command_name() == "info") {
            dpp::embed text = dpp::embed().
                set_color(0x0099ff).
                set_title("Aqua bot").
                set_url("https://www.youtube.com/watch?v=dQw4w9WgXcQ").
                set_description("bot của Anos :)").
                set_thumbnail("https://www.nautiljon.com/images/perso/00/20/anos_voldigoad_19602.webp?1680033786").
                set_timestamp(time(0));
            dpp::message about(event.command.channel_id, text);
            event.co_edit_original_response(about);
        }*/

        //lệnh bảo trì
        if (event.command.get_command_name() == "bypass") {
            if (event.command.usr.id == dev_id) {
                std::string url = std::get<std::string>(event.get_parameter("url"));
                if (isURL(url))
                    co_await bypass_link(url, bot, event);
                else
                    event.edit_original_response(dpp::message("không chứa url trong tin nhắn"));
            }
            else event.edit_original_response(dpp::message("lệnh đang bảo trì"));
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
                if (module == 1 || module == 2) {
                    event.edit_original_response(co_await(have_file ? gemini(bot, data, module) : gemini(bot, data, file, module)));
                }
                if (module == 3 || module == 4) {
                    if (!have_file) {
                        event.edit_original_response(dpp::message("Tạm thời bot không hỗ trợ file cho module này"));
                    }
                    else {
                        event.edit_original_response(co_await chatgpt(bot, data, module));
                    }
                }
            }
            catch (request_error& ex) {
                bot.log(dpp::ll_error, ex.what());
                ex.send_error(bot, event, dev_id);
            }
            catch (std::exception& ex) {
                bot.log(dpp::ll_critical, ex.what());
                event.edit_original_response(dpp::message("có lỗi nghiêm trọng xảy ra"));
            }
        }

        //tìm video
        if (event.command.get_command_name() == "search") {
            std::string text = std::get<std::string>(event.get_parameter("text"));
            co_await seach(text, bot, event);
        }


        if (event.command.get_command_name() == "info") {
            dpp::embed info = dpp::embed()
                .add_field("Mức sử dụng cpu(đơn vị:%)", fmt::format("{} %", getCpuUsage()))
                .add_field("Mức sử dụng ram(đơn vị:MB)", fmt::format("{} MB ({} %)", getRamUsage() / 1024, getRamUsage_percent()));
            event.edit_original_response(info);
        }
        //hàm liên quan đến game
        try {
            game game;
            if (event.command.get_command_name() == "register") {
                co_await game.reg(database, event);
            }

            if (event.command.get_command_name() == "game") {
                std::string game = event.command.get_command_interaction().options[0].name;

                dpp::snowflake user = event.command.usr.id;
                int amount = std::get<int64_t>(event.get_parameter("amount"));
                if (game == "blackjack") {
                    blackjack a(user, amount);
                    co_await a.play(database, event);
                }
                else if (game == "coinflip") {
                    coinflip c(user, amount);
                    co_await c.start(database, event);
                }
            }

            if (event.command.get_command_name() == "balance") {
                int64_t ammount = co_await game.get_money(database, event.command.usr.id);
                event.edit_original_response(dpp::message(fmt::format("Số dư của bạn là: {}", ammount)));
            }

            if (event.command.get_command_name() == "daily") {
                co_await game.daily(database, event);
            }
        }
        catch (request_error& ex) {
            bot.log(dpp::ll_error, ex.what());
            ex.send_error(bot, event, dev_id);
        }
        catch (std::exception& ex) {
            bot.log(dpp::ll_critical, ex.what());
            event.edit_original_response(dpp::message("có lỗi nghiêm trọng xảy ra"));
        }
        //trợ giúp
        if (event.command.get_command_name() == "help") {
            dpp::message text(event.command.channel_id, dpp::embed()
                .set_title("Các lệnh của bot")
                .add_field("Slash command", "hãy dùng \"/\" để xem chi tiết")
                .add_field("Command", "Không có sẵn")
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