#ifndef youtubeapi_H
#define youtubeapi_H

#include <string>
#include <dpp/dpp.h>
extern std::string apikey;
dpp::task<void> seach(std::string text, dpp::cluster& bot, const dpp::slashcommand_t& command);
//void reply_map(const dpp::slashcommand_t& event);
//void play(dpp::cluster& bot, const dpp::slashcommand_t& command, const dpp::voice_ready_t& event);
//void track_maker(const dpp::voice_track_marker_t& event);
//void handle_void_ready(dpp::cluster& bot, const dpp::slashcommand_t& command, const dpp::voice_ready_t& event);
//void disconect_member_voice(const dpp::slashcommand_t& event);
#endif //youtubeapi_H