#include <dpp/dpp.h>
#include <algorithm>
#include <random>
#include <mysql/mysql.h>
#include <fmt/format.h>
#include "..\header\error.h"
#include "..\header\game.h"

extern dpp::emoji_map bot_emojis;

//std::vector<dpp::snowflake> userid;

const dpp::embed no_reg = dpp::embed()
	.set_color(0xff0000)
	.set_description("có vẻ bạn chưa từng dùng bot này trước kia, hãy dùng lệnh /register");

std::vector<std::string> values = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "j", "q", "k", "a" };
std::vector<std::string> suits = { "H", "D", "C", "S" };

void get_card(std::vector<Card>& deck, std::vector<Card>& userhand) {
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

blackjack::blackjack(dpp::snowflake user_id, int amount)
	:amount(amount), user_id(user_id), deck(create_deck()), playerhand(), bothand()
{

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
	while (total > 0 && A > 0) {
		total -= 10;
		A--;
	}
	return total;
}

bool blackjack::is_bust(std::vector<Card>& cards) const {
	return player_vaule(cards) > 21;
}

int blackjack::get_money(MYSQL* db, dpp::snowflake user_id) const {
	std::string qwerty = "SELECT balance FROM players WHERE user_id = " + std::to_string(user_id);
	if (mysql_query(db, qwerty.c_str())) {
		REQUEST_ERROR(mysql_error(db), qwerty, "");
	}
	MYSQL_RES* result = mysql_store_result(db);

	if (result == NULL) {
		REQUEST_ERROR("mysql store result failed", qwerty, "");
	}

	if (mysql_num_rows(result) == 0) {
		return -1;
	}

	MYSQL_ROW row = mysql_fetch_row(result);

	if (row == NULL) REQUEST_ERROR("mysql fetch row failed", qwerty, "");

	int balance = std::stoi(row[0]);
	mysql_free_result(result);

	return balance;
}

void blackjack::update_money(MYSQL* db, dpp::snowflake user_id, int balance) const {
	std::string qwerty = fmt::format(fmt::runtime("UPDATE players SET balance = {0} WHERE user_id = {1}"), balance, user_id);
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
	int balance = get_money(db, user_id);
	//check if user doesn't register before
	if (balance == -1) {
		event.edit_original_response(no_reg);
		co_return;
	}
	//check if player have enough money to play this game
	if (balance < amount) {
		event.edit_original_response(dpp::message("bạn không đủ số dư"));
		co_return;
	}
	//distribute the cards
	distribute();

	while (true) {
		dpp::embed game = dpp::embed()
			.set_title(fmt::format(fmt::runtime("Blackjack | cược : {}"), amount))
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
				.set_id(std::to_string(user_id) + "_h")
				.set_disabled(is_bust(playerhand))
			)
		).add_component(
			dpp::component().add_component(
				dpp::component()
				.set_type(dpp::cot_button)
				.set_style(dpp::cos_danger)
				.set_label("stand")
				.set_id(std::to_string(user_id) + "_s")
			)
		);
		event.edit_original_response(m);

		auto result = co_await dpp::when_any{
		event.from->creator->on_button_click.when([this](const dpp::button_click_t& b) { 
			if (user_id == b.command.usr.id && b.custom_id == std::to_string(user_id) + "_h") // Button hit clicked
			{
				b.reply(dpp::ir_deferred_update_message, "");
				return true;
			}
			else if (user_id == b.command.usr.id && b.custom_id != std::to_string(user_id) + "_h") return false;
			else
			{
				b.reply("bạn không thể thực hiện hành động này");
				return false;
			}
		}),
		event.from->creator->on_button_click.when([this](const dpp::button_click_t& b) { 
			if (user_id == b.command.usr.id && b.custom_id == std::to_string(user_id) + "_s") // Button stand clicked
			{
				b.reply(dpp::ir_deferred_update_message, "");
				return true;
			}
			else if (user_id == b.command.usr.id && b.custom_id != std::to_string(user_id) + "_s") return false;
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
		.set_title(fmt::format("Blackjack | cược : {}", amount))
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

	//auto it = std::remove(userid.begin(), userid.end(), user_id);
	//userid.erase(it, userid.end());

	int new_balance = balance;
	if (is_win) {
		new_balance += amount;
	}
	else {
		new_balance -= amount;
	}
	//update player money
	update_money(db, user_id, new_balance);
}

dpp::task<bool> is_exist(MYSQL* db, dpp::snowflake user_id) {
	std::string qwerty = "SELECT COUNT(*) FROM players WHERE user_id = " + std::to_string(user_id);
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

dpp::task<void> reg(MYSQL* db, dpp::snowflake user_id, const dpp::slashcommand_t& event) {
	if (co_await is_exist(db, user_id)) {
		event.edit_original_response(dpp::message("bạn đã đăng kí trước kia"));
		co_return;
	}
	std::string qwerty = fmt::format(fmt::runtime("INSERT INTO players (user_id) VALUES ({})"), user_id);
	if (mysql_query(db, qwerty.c_str())) {
		REQUEST_ERROR(mysql_error(db), qwerty, "");
	}
	else event.edit_original_response(dpp::message("đăng kí thành công"));
}
