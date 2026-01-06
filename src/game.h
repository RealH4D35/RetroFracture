#ifndef GAME_H
#define GAME_H

#include "splashkit.h"
#include "player.h"

class Game {
private:
    Player player;
    bool quit_game;
    bitmap background;
    
    // Camera variables
    point_2d camera_pos;
    float background_original_width;
    float background_original_height;
    float background_scaled_width;
    float background_scaled_height;
    float scale_factor;
    float background_y_offset;
    
    // Level boundaries
    float ground_level;  // Add this
    
public:
    Game();
    void run();
    void update();
    void draw();
    void handle_input();
    void check_boundaries();
    
    // Camera methods
    void update_camera();
    point_2d world_to_screen(point_2d world_pos);
};

#endif