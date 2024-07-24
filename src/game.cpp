#include <dpp/dpp.h>
#include <random>


struct Card {
	std::string value;
	std::string suit;
};
//						deck						playerhand			bothand
using cards = std::pair<std::vector<Card>, std::pair<std::vector<Card>, std::vector<Card>>>;
std::unordered_map < dpp::snowflake, cards> gameplay;

std::vector<std::string> values = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "j", "q", "k", "a" };
std::vector<std::string> suits = { "H", "D", "C", "S" };

static std::vector<Card> deck() {
	std::vector<Card> new_deck;
	for (auto& a : values)
		for (auto& b : suits)
			new_deck.push_back({ a, b });
	std::shuffle(new_deck.begin(), new_deck.end(), std::default_random_engine(std::random_device()()));
	return new_deck;
}


int card_vaule(const Card& card) {
	if (card.value == "j" || card.value == "q" || card.value == "k") return 10;
	if (card.value == "a") return 11;
	return std::stoi(card.value);
}

int player_vaule(std::vector<Card>& cards) {
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
bool is_bust(std::vector<Card>& cards) {
	return player_vaule(cards) > 21;
}
std::string emoji_card(const Card& card) {
	return card.suit + card.value;
}