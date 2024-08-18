#ifndef game_H
#define game_H
#include <dpp/dpp.h>
struct Card {
	std::string value;
	std::string suit;
};

dpp::task<void> reg(MYSQL* db, const dpp::slashcommand_t& event);
dpp::task<int64_t> get_money(MYSQL* db, dpp::snowflake user_id);
dpp::task<void> daily(MYSQL* db, const dpp::slashcommand_t& event);

class blackjack {
public:
	blackjack(dpp::snowflake user_id, int amount);
	dpp::task<void> play(MYSQL* db, const dpp::slashcommand_t& event);
	~blackjack();

private:
	//hàm
	std::vector<Card> create_deck() const;
	int card_vaule(const Card& card) const;
	int player_vaule(std::vector<Card>& cards) const;
	bool is_bust(std::vector<Card>& cards) const;
	void bot_play(std::vector<Card>& deck, std::vector<Card>& bot);
	void distribute();
	std::string get_emoji_string(std::vector<Card> card) const;
	std::string get_emoji_string(Card card) const;

	//biến
	std::vector<Card> deck;
	std::vector<Card> playerhand;
	std::vector<Card> bothand;
	int amount;
	dpp::snowflake user_id;
};

#endif // !game_H
