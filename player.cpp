#include "player.h"
#include "gamekey.h"
#include<stdexcept>

constexpr int MMR_MIN = 0;
constexpr int MMR_MAX = 9999;

inline const std::string& check_name(const std::string& name) {
    if (name.empty()) throw std::runtime_error("Name passed was empty");
    return name;
}

inline int check_in(int num, int min, int max) {
    if (num < min) throw std::runtime_error("MMR passed was smaller than " + std::to_string(min));
    if (num > max) throw std::runtime_error("MMR passed was greater than " + std::to_string(max));
    return num;
}

inline int coerce_in(int num, int min, int max) {
    return num > max ? max : (num < min ? min : num);
}

Player::Player(std::string _name, int _mmr) : name{ check_name(_name) }, mmr{ check_in(_mmr, MMR_MIN, MMR_MAX) } {}

std::string Player::get_name() const {
    return name;
}

int Player::get_mmr() const {
    return mmr;
}

std::shared_ptr<Game> Player::get_hosted_game() const {
    return hosted_game;
}

void Player::change_mmr(int n) {
    mmr = coerce_in(mmr + n, MMR_MIN, MMR_MAX);
}

bool Player::host_game(std::string s, Mode m) {
    if (s.empty()) throw std::runtime_error("cannot host game with empty string as name");

    if (!hosted_game) {
        switch (m) {
            case Mode::Ranked:
                hosted_game = std::make_shared<RGame>(s, shared_from_this());
                break;
            case Mode::Unranked:
                hosted_game = std::make_shared<UGame>(s, shared_from_this());
                break;
        }
        return true;
    } else return false;
}

bool Player::join_game(std::shared_ptr<Game> g) {
    bool success = g->add_player(GameKey(), shared_from_this());

    if (success) {
        games[g->get_name()] = g;
    }

    return success;
}

bool Player::leave_game(std::shared_ptr<Game> g) {
    return games.erase(g->get_name()) && g->remove_player(GameKey(), shared_from_this());
}

std::vector<std::weak_ptr<Player>> Player::invite_players(const std::vector<std::weak_ptr<Player>>& v) {
    std::vector<std::weak_ptr<Player>> failed;
    for (const std::weak_ptr<Player>& p : v) {
        if (std::shared_ptr<Player> lock_p = p.lock()) {
            if (!lock_p->join_game(hosted_game)) {
                failed.push_back(p);
            }
        } else {
            failed.push_back(p);
        }
    }
    return failed;
}

bool Player::close_game() {
    if (hosted_game) {
        hosted_game.reset();
        return true;
    } else return false;
}

std::ostream& Player::print(std::ostream& o) const {
    o << '[' << name << ", " << mmr << ", hosts: " << (hosted_game ? hosted_game->get_name() : "nothing") << ", games: {";
    bool first = true;
    for (const auto& entry : games) {
        if (std::shared_ptr<Game> g = entry.second.lock()) {
            if (first) {
                first = false;
            } else {
                o << ", ";
            }
            o << g->get_name();
        }
    }
    o << "}]";

    return o;
}


std::ostream& operator<<(std::ostream& o, const Player& p) {
    return p.print(o);
}