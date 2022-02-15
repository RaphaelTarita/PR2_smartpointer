#ifndef GAME_H
#define GAME_H

#include<string>
#include<memory>
#include<map>
#include<iostream>

class Player;
class GameKey;

class Game : public std::enable_shared_from_this<Game> {
    std::string name;
    std::weak_ptr<Player> host;
    std::map<std::string, std::weak_ptr<Player>> players;

public:
    Game(std::string, std::shared_ptr<Player>);
    virtual ~Game() = default;
    std::string get_name() const;
    bool is_allowed(int) const;
    bool remove_player(const GameKey&, std::shared_ptr<Player>);
    bool add_player(const GameKey&, std::shared_ptr<Player>);
    size_t number_of_players() const;
    std::weak_ptr<Player> best_player() const;
    virtual int change(bool) const = 0;
    std::weak_ptr<Player> play(size_t i);

    virtual std::ostream& print(std::ostream&) const;
};

class RGame : public Game {
public:
    RGame(std::string, std::shared_ptr<Player>);
    int change(bool) const override;

    std::ostream& print(std::ostream&) const override;
};

class UGame : public Game {
public:
    UGame(std::string, std::shared_ptr<Player>);
    int change(bool) const override;

    std::ostream& print(std::ostream&) const override;
};

std::ostream& operator<<(std::ostream&, const Game&);

#endif