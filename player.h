#ifndef PLAYER_H
#define PLAYER_H

#include<string>
#include<memory>
#include<map>
#include<vector>
#include<iostream>
#include "game.h"


class Game;

enum class Mode {
    Ranked,
    Unranked
};

class Player : public std::enable_shared_from_this<Player> {
    std::string name;
    int mmr;
    std::shared_ptr<Game> hosted_game;
    std::map<std::string, std::weak_ptr<Game>> games;

public:
    Player(std::string, int);
    std::string get_name() const;
    int get_mmr() const;
    std::shared_ptr<Game> get_hosted_game() const;
    void change_mmr(int);
    bool host_game(std::string, Mode);
    bool join_game(std::shared_ptr<Game>);
    bool leave_game(std::shared_ptr<Game>);
    std::vector<std::weak_ptr<Player>> invite_players(const std::vector<std::weak_ptr<Player>>&);
    bool close_game();

    std::ostream& print(std::ostream&) const;
};

std::ostream& operator<<(std::ostream&, const Player&);

#endif