#include "game.h"
#include "player.h"
#include<stdexcept>
#include<numeric>
#include<algorithm>
#include<vector>

typedef std::pair<std::string, std::weak_ptr<Player>> player_entry;

Game::Game(std::string _name, std::shared_ptr<Player> _host) : name{ _name } {
    if (name.empty()) throw std::runtime_error("Could not construct game, name was empty");
    if (!static_cast<bool>(_host)) throw std::runtime_error("Could not construct game, host pointer was invalid");
    host = _host;
}

std::string Game::get_name() const {
    return name;
}

bool Game::is_allowed(int n) const {
    if (std::shared_ptr<Player> host_lock = host.lock()) {
        int host_mmr = host_lock->get_mmr();
        return n * 10 > (host_mmr * 9) && n * 10 < (host_mmr * 11);
    } else {
        throw std::runtime_error("Could not check host mmr, because the host pointer was invalid");
    }
}

bool Game::remove_player(const GameKey&, std::shared_ptr<Player> p) {
    return players.erase(p->get_name());
}

bool Game::add_player(const GameKey&, std::shared_ptr<Player> p) {
    if (is_allowed(p->get_mmr())) {
        const std::string& n{ p->get_name() };
        if (players.count(n) != 0 && players.at(n).expired()) players.erase(n);

        return players.insert(std::pair<std::string, std::weak_ptr<Player>>(p->get_name(), p)).second;
    } else return false;
}

size_t Game::number_of_players() const {
    return std::accumulate(players.begin(), players.end(), 0, [](int lop, const player_entry& rop) {
        return lop + !rop.second.expired();
    });
}

std::weak_ptr<Player> Game::best_player() const {
    if (number_of_players() > 0) {
        return std::max_element(players.begin(), players.end(), [](const player_entry& lop, const player_entry& rop) {
            if (std::shared_ptr<Player> lop_lock = lop.second.lock()) {
                if (std::shared_ptr<Player> rop_lock = rop.second.lock()) {
                    return lop_lock->get_mmr() < rop_lock->get_mmr();
                } else return false;
            } else return true;
        })->second;
    } else throw std::runtime_error("No active player in the game");
}

std::weak_ptr<Player> Game::play(size_t i) {
    std::vector<std::shared_ptr<Player>> active_players;
    for (auto it = players.cbegin(); it != players.cend();) {
        if (std::shared_ptr<Player> lock_p = it->second.lock()) {
            active_players.push_back(lock_p);
            ++it;
        } else {
            it = players.erase(it);
        }
    }

    if (host.expired()) throw std::runtime_error("Cannot play game, host pointer is invalid");
    if (i < active_players.size()) {
        std::shared_ptr<Player> winner = active_players[i];
        active_players.erase(active_players.begin() + static_cast<int>(i));
        int penalty = change(false);
        for (const auto& p : active_players) {
            p->change_mmr(winner->get_mmr() < p->get_mmr() ? 2 * penalty : penalty);
        }

        winner->change_mmr(change(true));

        return winner;
    } else throw std::runtime_error("Game does not have enough active players");
}

std::ostream& Game::print(std::ostream& o) const {
    if (std::shared_ptr<Player> host_lock = host.lock()) {
        o << '[' << name << ", " << host_lock->get_name() << ", " << host_lock->get_mmr() << ", player: {";
        host_lock.reset();
    } else {
        throw std::runtime_error("host pointer was invalid");
    }

    bool first = true;
    for (const auto& p : players) {
        if (std::shared_ptr<Player> p_lock = p.second.lock()) {
            if (first) {
                first = false;
            } else {
                o << ", ";
            }
            o << '[' << p_lock->get_name() << ", " << p_lock->get_mmr() << ']';
        }
    }

    o << "}]";

    return o;
}

RGame::RGame(std::string _name, std::shared_ptr<Player> _host) : Game(_name, _host) {}

int RGame::change(bool won) const {
    return won ? 5 : -5;
}

std::ostream& RGame::print(std::ostream& o) const {
    o << "Ranked Game: ";
    return Game::print(o);
}

UGame::UGame(std::string _name, std::shared_ptr<Player> _host) : Game(_name, _host) {}

int UGame::change(bool) const {
    return 0;
}

std::ostream& UGame::print(std::ostream& o) const {
    o << "Unranked Game: ";
    return Game::print(o);
}

std::ostream& operator<<(std::ostream& o, const Game& g) {
    return g.print(o);
}
