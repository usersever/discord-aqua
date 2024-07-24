#include <dpp/dpp.h>
#include <vector>
#include <dpp/nlohmann/json.hpp>
#include "../header/Encode-decode.h"

using json = nlohmann::json;

std::string apikey;

//#if defined(WIN32) && !defined(UNIX)
//#define popen _popen
//#define pclose _pclose
//#endif
//
//std::map<dpp::snowflake, dpp::discord_client*> clients;
//std::map<dpp::snowflake, std::vector<std::string>> guilds_playlist;
//std::mutex client_mutex;
//
//void disconect_member_voice(const dpp::slashcommand_t& event) {
//	auto it = clients.find(event.command.guild_id);
//	if (it == clients.end()) return;
//	it->second->disconnect_voice(event.command.guild_id);
//	clients.erase(it);
//	guilds_playlist.erase(event.command.guild_id);
//}
//void co_edit_original_response_map(const dpp::slashcommand_t& event) {
//	std::string result;
//	for (const auto pair : guilds_playlist) {
//		result += std::to_string(pair.first) + ":\n";
//		for (const auto& url : pair.second) {
//			result += url + "\n";
//		}
//	}
//	event.co_edit_original_response(result);
//}
//
//void connect_member_voice(dpp::cluster& bot, const dpp::slashcommand_t& command) {
//	dpp::guild* guild = dpp::find_guild(command.command.guild_id);
//	if (!guild) {
//		bot.message_create(dpp::message(command.command.channel_id, "không tìm thấy server"));
//		return;
//	}
//	if (!guild->connect_member_voice(command.command.usr.id)) {
//		bot.message_create(dpp::message(command.command.channel_id, "hãy vào voice chat trước"));
//		return;
//	}
//
//	std::lock_guard l(client_mutex);
//	guilds_playlist.insert_or_assign(command.command.guild_id, std::vector<std::string>());
//	clients.insert_or_assign(command.command.guild_id, command.from);
//}
//void track_maker(const dpp::voice_track_marker_t& event) {
//	if (event.track_meta == "disc") {
//		std::thread t([event]() {
//			std::lock_guard l(client_mutex);
//			auto it = clients.find(event.voice_client->server_id);
//			if (it != clients.end()) return;
//			it->second->disconnect_voice(event.voice_client->server_id);
//			clients.erase(it);
//			guilds_playlist.erase(event.voice_client->server_id);
//			});
//		t.detach();
//	}
//}
//void play(
//	dpp::cluster& bot, 
//	const dpp::slashcommand_t& command, 
//	const dpp::voice_ready_t& event
//	) {
//	dpp::snowflake server_id = event.voice_client->server_id;
//	//yt-dlp -o - "URL" | ffmpeg -i pipe: ...
//	bot.message_create(dpp::message(command.command.channel_id,"đang tải..."));
//	while (guilds_playlist[command.command.guild_id].begin() !=
//		   guilds_playlist[command.command.guild_id].end()) {
//
//
//		FILE* pipe = popen(("yt-dlp.cmd -f 251 -o - " + guilds_playlist[command.command.guild_id][0]
//			+ " | ffmpeg -i - -f s16le -ar 48000 -ac 2 -").c_str(), "r");
//
//		constexpr size_t max_buffer_size = dpp::send_audio_raw_max_length;
//		char buf[max_buffer_size];
//		size_t buf_read = 0;
//		size_t current_read = 0;
//		size_t temp = 0;
//		while ((current_read = fread(buf + buf_read, 1, max_buffer_size - buf_read, pipe)) > 0) {
//			/*if (temp != 0) {
//				buf_read += temp;
//				temp = 0;
//			}*/
//			buf_read += current_read;
//			
//			/*if ((buf_read % 4) != 0) {
//				temp = buf_read % 4;
//				buf_read -= temp;
//			}*/
//			std::cout << "buf_read: " << buf_read << std::endl;
//			if (buf_read == max_buffer_size) {
//				event.voice_client->send_audio_raw((uint16_t*)buf, buf_read);
//				buf_read = 0;
//			}
//		}
//		if (buf_read > 0) {
//			event.voice_client->send_audio_raw((uint16_t*)buf, buf_read);
//		}
//
//		pclose(pipe);
//		pipe = NULL;
//		delete[] buf;
//
//		while (event.voice_client->is_playing()) {
//			std::this_thread::sleep_for(std::chrono::seconds(1));
//		}
//		guilds_playlist[command.command.guild_id]
//			.erase(guilds_playlist[command.command.guild_id].begin());
//	}
//	event.voice_client->insert_marker("disc");
//}
//void handle_void_ready(
//	dpp::cluster& bot,
//	const dpp::slashcommand_t& command,
//	const dpp::voice_ready_t& event
//	) {
//	std::thread thread([&bot, command, event] {
//		play(bot, command, event);
//		});
//	
//	thread.detach();
//}

dpp::task<void> seach(std::string text,dpp::cluster &bot,const dpp::slashcommand_t& command) {
	for (int i = 0; i < text.size(); i++) {
		if (text[i] == ' ') {
			text[i] = '%';
			text.insert(i + 1, "20");
		}
	}
	std::string url = "https://www.googleapis.com/youtube/v3/search?part=snippet&q=" + text + "&key=" + apikey;
	bot.request(url, dpp::http_method::m_get, [&bot, command](const dpp::http_request_completion_t& event) {
		int status = event.status;
		std::string result = event.body;
		if (status != 200) {
			bot.log(dpp::ll_warning, ("kết nối thất bại. mã lỗi: " + status));
			command.co_edit_original_response(dpp::message(command.command.channel_id, "kết nối thất bại."));
			return;
		}
		else {
			try {
				json j = json::parse(result);
				std::string id = j["items"][0]["id"]["videoId"].get<std::string>(),
					title = j["items"][0]["snippet"]["title"].get<std::string>(),
					img = j["items"][0]["snippet"]["thumbnails"]["default"]["url"].get<std::string>(),
					video_description = j["items"][0]["snippet"]["description"].get<std::string>();
				//bot.log(dpp::ll_debug, video_description + "\n" + title);
				if (title.size() > 45) {
					title = title.substr(0, 45) + "...";
				}
				if (video_description.size() > 150) {
					video_description = video_description.substr(0, 150) + "...";
				}
				//bot.log(dpp::ll_debug, video_description + "\n" + title);
				std::string url_video = "https://www.youtube.com/watch?v=" + id;

				dpp::embed result = dpp::embed()
					.set_title(title)
					.set_url(url_video)
					.set_thumbnail(img)
					.set_description(video_description)
					.set_footer(dpp::embed_footer()
						.set_icon("https://cdn.pixabay.com/photo/2016/11/19/03/08/youtube-1837872_960_720.png")
						.set_text("Youtube"));

				dpp::message message(command.command.channel_id, result);
				command.co_edit_original_response(message);
			}
			catch (std::exception &ex) {
				bot.log(dpp::ll_critical, ex.what() /*+ '\n' + ex.id*/);
				command.co_edit_original_response(dpp::message(command.command.channel_id, "có lỗi xảy ra"));
			}
			return;

			//bot.log(dpp::ll_info, "kết nối thành công.");
			//lỗi lúc khác sửa
			//connect_member_voice(bot, command);

			/*auto it = guilds_playlist.find(command.command.guild_id);
			auto it2 = dpp::find_channel(command.command.channel_id);
			auto members = it2->get_members();
			if (it != guilds_playlist.end()) {
				bot.message_create(dpp::message(command.command.channel_id, "đã có người dùng bot"));
				return;
			}
			else if (members.find(command.command.usr.id) != members.end()) {
				guilds_playlist[command.command.guild_id].push_back(url_video);
				bot.message_create(dpp::message(command.command.channel_id, "đã thêm vào danh sách phát"));
			}*/
			//guilds_playlist[command.command.guild_id].push_back(url_video);
		}
		});
	co_return;
}