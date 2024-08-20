#include <dpp/dpp.h>
#include <dpp/unicode_emoji.h>
#include <algorithm>
#include <random>
#include <mysql/mysql.h>
#include <fmt/format.h>
#include "..\header\error.h"
#include "..\header\game.h"

extern dpp::emoji_map bot_emojis;


/********************************************************
 *						blackjack						*
 ********************************************************/


const dpp::embed no_reg = dpp::embed()
	.set_color(0xff0000)
	.set_description("có vẻ bạn chưa từng dùng bot này trước kia, hãy dùng lệnh /register");

std::vector<std::string> values = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "j", "q", "k", "a" };
std::vector<std::string> suits = { "H", "D", "C", "S" };

static void get_card(std::vector<Card>& deck, std::vector<Card>& userhand) {
	userhand.push_back(deck.back());
	deck.pop_back();
}

static dpp::emoji find_emoji(std::string name) {
	auto it = std::find_if(bot_emojis.begin(), bot_emojis.end(), [&name](const auto& pair) {
		return pair.second.name == name;
		});
	if (it != bot_emojis.end()) {
		return it->second;
	}
	else throw std::exception(fmt::format("emoji \"{}\" not found", name).c_str());
}

blackjack::blackjack(dpp::snowflake user_id, int64_t amount)
	: deck(create_deck()), playerhand(), bothand()
{
	bet_amount = amount;
	game_user_id = game_user_id;
}

blackjack::~blackjack() {

}

std::vector<Card> blackjack::create_deck() const {
	std::vector<Card> new_deck;
	for (auto& a : values)
		for (auto& b : suits)
			new_deck.push_back({ a, b });
	std::shuffle(new_deck.begin(), new_deck.end(), std::default_random_engine(std::random_device()()));
	return new_deck;
}

int blackjack::card_vaule(const Card& card) const {
	if (card.value == "j" || card.value == "q" || card.value == "k") return 10;
	if (card.value == "a") return 11;
	return std::stoi(card.value);
}

int blackjack::player_vaule(std::vector<Card>& cards) const {
	int total = 0;
	int A = 0;
	for (const Card a : cards) {
		total += card_vaule(a);
		if (a.value == "a") A++;
	}
	while (total > 21 && A > 0) {
		total -= 10;
		A--;
	}
	return total;
}

bool blackjack::is_bust(std::vector<Card>& cards) const {
	return player_vaule(cards) > 21;
}

void blackjack::bot_play(std::vector<Card>& deck, std::vector<Card>& bot) {
	while (player_vaule(bot) < 17) {
		get_card(deck, bot);
	}
}

void blackjack::distribute() {
	auto a = deck.size();
	playerhand.push_back(std::move(deck[--a]));
	playerhand.push_back(std::move(deck[--a]));
	bothand.push_back(std::move(deck[--a]));
	bothand.push_back(std::move(deck[--a]));
	deck.resize(a);
}

std::string blackjack::get_emoji_string(std::vector<Card> card) const {
	std::string str;
	for (auto& a : card) {
		dpp::emoji e = find_emoji(a.suit + a.value);
		str += e.get_mention() + ' ';
	}
	return str;
}

std::string blackjack::get_emoji_string(Card card) const {
	try {
		dpp::emoji e = find_emoji(card.suit + card.value);
		std::string mention = e.get_mention();
		return mention + " (hidden)";
	}
	catch (std::exception& ex) {
		std::printf(ex.what());
		return "(error)";
	}
}

dpp::task<void> blackjack::play(MYSQL* db, const dpp::slashcommand_t& event) {
	//get player's money
	int64_t balance = co_await get_money(db, game_user_id);
	//check if user doesn't register before
	if (balance == -1) {
		event.edit_original_response(no_reg);
		co_return;
	}
	//check if player have enough money to play this game
	if (balance < bet_amount) {
		event.edit_original_response(dpp::message("bạn không đủ số dư"));
		co_return;
	}
	//distribute the cards
	distribute();

	while (true) {
		dpp::embed game = dpp::embed()
			.set_title(fmt::format(fmt::runtime("Blackjack | cược : {}"), bet_amount))
			.add_field(
				fmt::format("Bộ bài đối thủ | {} điểm", card_vaule(bothand[0])),
				get_emoji_string(bothand[0])
			)
			.add_field(
				fmt::format("bộ bài của bạn | {} điểm", player_vaule(playerhand)),
				get_emoji_string(playerhand)
			);
		dpp::message m(game);
		m.add_component(
			dpp::component().add_component(
				dpp::component()
				.set_type(dpp::cot_button)
				.set_style(dpp::cos_primary)
				.set_label("hit")
				.set_id(std::to_string(game_user_id) + "_h")
				.set_disabled(is_bust(playerhand))
			)
		).add_component(
			dpp::component().add_component(
				dpp::component()
				.set_type(dpp::cot_button)
				.set_style(dpp::cos_danger)
				.set_label("stand")
				.set_id(std::to_string(game_user_id) + "_s")
			)
		);
		event.edit_original_response(m);

		auto result = co_await dpp::when_any{
		event.from->creator->on_button_click.when([this](const dpp::button_click_t& b) { 
			if (game_user_id == b.command.usr.id && b.custom_id == std::to_string(game_user_id) + "_h") // Button hit clicked
			{
				b.reply(dpp::ir_deferred_update_message, "");
				return true;
			}
			else if (game_user_id == b.command.usr.id && b.custom_id != std::to_string(game_user_id) + "_h") return false;
			else
			{
				b.reply("bạn không thể thực hiện hành động này");
				return false;
			}
		}),
		event.from->creator->on_button_click.when([this](const dpp::button_click_t& b) { 
			if (game_user_id == b.command.usr.id && b.custom_id == std::to_string(game_user_id) + "_s") // Button stand clicked
			{
				b.reply(dpp::ir_deferred_update_message, "");
				return true;
			}
			else if (game_user_id == b.command.usr.id && b.custom_id != std::to_string(game_user_id) + "_s") return false;
			else 
			{
				b.reply("bạn không thể thực hiện hành động này");
				return false;
			}
		}),
		event.from->creator->co_sleep(300) // wait 300 seconds
		};
		if (result.index() == 0) {
			get_card(deck, playerhand);
		}
		else if (result.index() == 1 || result.index() == 2) {
			bot_play(deck, bothand);
			break;
		}
	}
	//show result
	bool is_win = (player_vaule(playerhand) > player_vaule(bothand) && !is_bust(playerhand) && !is_bust(bothand)) || (!is_bust(playerhand) && is_bust(bothand));

	dpp::embed game = dpp::embed()
		.set_color(is_win ? 0xffff00 : 0xff0000)
		.set_title(fmt::format("Blackjack | cược : {}", bet_amount))
		.set_description("kết quả: " + std::string(is_win ? "bạn thắng" : "bạn thua"))
		.add_field(
			fmt::format("Bộ bài đối thủ | {} điểm", player_vaule(bothand)),
			get_emoji_string(bothand)
		)
		.add_field(
			fmt::format("bộ bài của bạn | {} điểm", player_vaule(playerhand)),
			get_emoji_string(playerhand)
		);
	dpp::message m(game);
	event.edit_original_response(m);

	//auto it = std::remove(userid.begin(), userid.end(), game_user_id);
	//userid.erase(it, userid.end());

	int new_balance = balance;
	if (is_win) {
		new_balance += bet_amount;
	}
	else {
		new_balance -= bet_amount;
	}
	//update player money
	update_money(db, new_balance);
}

dpp::task<bool> is_exist(MYSQL* db, dpp::snowflake game_user_id) {
	std::string qwerty = "SELECT COUNT(*) FROM players WHERE user_id = " + std::to_string(game_user_id);
	if (mysql_query(db, qwerty.c_str())) {
		REQUEST_ERROR(mysql_error(db), qwerty, "");
	}

	MYSQL_RES* res = mysql_store_result(db);
	if (res == NULL) {
		std::cerr << "mysql_store_result() failed. " << mysql_error(db) << "\n";
		REQUEST_ERROR(mysql_error(db), qwerty, "");
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	bool exists = row && std::stoll(row[0]) > 0;

	mysql_free_result(res);
	co_return exists;
}
/********************************************************
 *					coin flip							*
 ********************************************************/

coinflip::coinflip(dpp::snowflake user_id, int64_t amount)
{
	game_user_id = user_id;
	bet_amount = amount;
}

coinflip::~coinflip() {

}

dpp::task<void> coinflip::start(MYSQL* db, const dpp::slashcommand_t& event) {
	srand(time(0));
	int64_t player_amount = co_await get_money(db, game_user_id);

	if (player_amount < bet_amount) {
		event.edit_original_response(dpp::message("Bạn không đủ số dư!"));
		co_return;
	}

	dpp::embed embed = dpp::embed()
		.set_description("Đang quay ...");
	event.edit_original_response(embed);
	co_await event.from->creator->co_sleep(3);

	//false là ngửa, true là úp
	bool is_heads = rand() % 2;
	bool bet_face = bool(std::get<int64_t>(event.get_parameter("coin-face")));
	bool is_win = is_heads == bet_face;

	embed = dpp::embed()
		.set_title(fmt::format("Kết quả: {0} | tiền cược: {1}", std::string(is_win ? "Bạn thắng" : "Bạn thua"), bet_amount))
		.set_color(is_win ? 0xffff00 : 0xff0000)
		.set_description(fmt::format("```Bạn cược: {0}\nKết quả: {1}```",
			(bet_face ? dpp::unicode_emoji::white_circle : dpp::unicode_emoji::black_circle), (is_heads ? dpp::unicode_emoji::white_circle : dpp::unicode_emoji::black_circle)));
	update_money(db, (is_win ? player_amount += bet_amount : player_amount -= bet_amount));
	event.edit_original_response(embed);
	co_return;
}

/******************************************************** 
 *					game system							*
 ********************************************************/


dpp::task<void> game::reg(MYSQL* db, const dpp::slashcommand_t& event) {
	dpp::snowflake game_user_id = event.command.usr.id;
	if (co_await is_exist(db, game_user_id)) {
		event.edit_original_response(dpp::message("bạn đã đăng kí trước kia"));
		co_return;
	}
	std::string qwerty = fmt::format(fmt::runtime("INSERT INTO players (user_id) VALUES ({})"), std::to_string(game_user_id));
	if (mysql_query(db, qwerty.c_str())) {
		REQUEST_ERROR(mysql_error(db), qwerty, "");
	}
	else event.edit_original_response(dpp::message("đăng kí thành công"));
}

dpp::task<void> game::daily(MYSQL* db, const dpp::slashcommand_t& event) {
	dpp::snowflake game_user_id = event.command.usr.id;

	if (!co_await is_exist(db, game_user_id)) {
		event.edit_original_response(no_reg);
		co_return;
	}
	std::string qwerty = "SELECT last_claim FROM players WHERE user_id = " + std::to_string(game_user_id);
	if (mysql_query(db, qwerty.c_str())) {
		REQUEST_ERROR(mysql_error(db), qwerty, "");
	}
	MYSQL_RES* result = mysql_store_result(db);
	if (result == nullptr) {
		REQUEST_ERROR("failed to get result", qwerty, mysql_error(db));
	}
	MYSQL_ROW row = mysql_fetch_row(result);
	if (row[0]) {
		std::tm tm = {};
		std::istringstream ss(row[0]);
		ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
		if (ss.fail()) throw std::runtime_error("std::istringstream failed");
		else {

			// Chuyển đổi std::tm thành std::time_t
			std::time_t time = std::mktime(&tm);

			// Lấy thời gian hiện tại
			auto now = std::chrono::system_clock::now();
			std::time_t now_tt = std::chrono::system_clock::to_time_t(now);

			// Chuyển đổi thời gian hiện tại thành GMT+7
			std::tm now_tm = {};
			gmtime_s(&now_tm, &now_tt);
			now_tm.tm_hour += 7;
			std::time_t now_gmt7 = std::mktime(&now_tm);

			// Tính toán khoảng thời gian giữa thời gian hiện tại và thời gian được cung cấp
			auto duration = std::chrono::seconds(now_gmt7 - time);

			// Tạo duration cho 1 ngày
			auto one_day = std::chrono::hours(24);


			if (duration < one_day) {
				auto wait_time = std::chrono::system_clock::now() + one_day - duration;
				event.edit_original_response(dpp::message(fmt::format("hãy quay lại và nhận sau <t:{}:R>", std::to_string(std::chrono::system_clock::to_time_t(wait_time)))));
				co_return;
			}
		}
	}
	//lấy thời gian hiện tại
	std::tm now{};
	std::time_t now_tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	gmtime_s(&now, &now_tt);
	now.tm_hour += 7;
	mktime(&now);

	//format thời gian
	std::ostringstream oss;
	oss << std::put_time(&now, "%Y-%m-%d %H:%M:%S");

	srand(time(0));
	int random_money = rand() % 100000 + 1000;
	int new_balance = co_await get_money(db, game_user_id) + random_money;
	dpp::embed embed = dpp::embed()
		.set_color(0x00ff00)
		.set_title(fmt::format("bạn đã nhận được {}, hãy quay lại vào hôm sau để nhận tiếp", random_money));
	event.edit_original_response(embed);
	
	qwerty = fmt::format("UPDATE players SET last_claim = '{0}' WHERE user_id = {1}", oss.str(), std::to_string(game_user_id));
	if (mysql_query(db, qwerty.c_str())) {
		REQUEST_ERROR("update time failed", qwerty, mysql_error(db));
	}

	update_money(db, new_balance);
}
dpp::task<int64_t> game::get_money(MYSQL* db, dpp::snowflake user_id) {
	std::string qwerty = "SELECT balance FROM players WHERE user_id = " + std::to_string(user_id);
	if (mysql_query(db, qwerty.c_str())) {
		REQUEST_ERROR(mysql_error(db), qwerty, "");
	}
	MYSQL_RES* result = mysql_store_result(db);

	if (result == NULL) {
		REQUEST_ERROR("mysql store result failed", qwerty, "");
	}

	if (mysql_num_rows(result) == 0) {
		co_return -1;
	}

	MYSQL_ROW row = mysql_fetch_row(result);

	if (row == NULL) REQUEST_ERROR("mysql fetch row failed", qwerty, "");

	int64_t balance = std::stoi(row[0]);
	mysql_free_result(result);

	co_return balance;
}

void game::update_money(MYSQL* db, int balance) {
	std::string qwerty = fmt::format(fmt::runtime("UPDATE players SET balance = {0} WHERE user_id = {1}"), balance, game_user_id);
	MYSQL_STMT* stmt = mysql_stmt_init(db);
	if (!stmt) {
		REQUEST_ERROR("mysql_stmt_init() failed", qwerty, "");
	}
	if (mysql_stmt_prepare(stmt, qwerty.c_str(), qwerty.size())) {
		REQUEST_ERROR("mysql_stmt_prepare() failed", qwerty, mysql_stmt_error(stmt));
	}
	if (mysql_stmt_execute(stmt)) {
		REQUEST_ERROR("mysql_stmt_execute() failed", qwerty, mysql_stmt_error(stmt))
	}
	mysql_stmt_close(stmt);
}