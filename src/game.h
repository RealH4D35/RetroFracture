#ifndef GAME_H
#define GAME_H

#include "splashkit.h"
#include "player.h"

class Game {
private:
    Player player;
    bool quit_game;
    
public:
    Game();
    void run();
    void update();
    void draw();
    void handle_input();
    void check_boundaries();
};

#endif