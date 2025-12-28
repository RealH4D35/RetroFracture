#include "game.h"

Game::Game() : quit_game(false) {
}

void Game::run() {
    while (!quit_game && !quit_requested()) {
        process_events();
        handle_input();
        update();
        draw();
        refresh_screen(60);
    }
}

void Game::update() {
    player.update();
    check_boundaries();
}

void Game::draw() {
    // Clear with sky blue
    clear_screen(COLOR_SKY_BLUE);
    
    // Draw ground
    fill_rectangle(COLOR_DARK_GRAY, 0, 560, 800, 40);
    draw_line(COLOR_BLACK, 0, 560, 800, 560);
    
    // Draw player (with sprites)
    player.draw();
    
    // Draw simple instructions
    draw_text("CONTROLS: A (Left), D (Right), W (Jump)", COLOR_BLACK, 10, 10);
    draw_text("ESC to quit", COLOR_BLACK, 10, 30);
    
    // Show current animation state
    std::string state_text = "STATE: " + player.get_current_animation_name();
    draw_text(state_text, COLOR_BLACK, 10, 50);
    
    std::string frame_text = "FRAME: " + std::to_string(player.get_current_frame_index());
    draw_text(frame_text, COLOR_BLACK, 10, 70);
    
    std::string grounded_text = "GROUNDED: " + std::string(player.get_is_grounded() ? "YES" : "NO");
    draw_text(grounded_text, COLOR_BLACK, 10, 90);
}

void Game::check_boundaries() {
    point_2d pos = player.get_position();
    
    // Keep player on screen with sprite width consideration
    // Sprite is 96px wide, so half is 48px
    if (pos.x < 48) pos.x = 48;
    if (pos.x > 800 - 48) pos.x = 800 - 48;
    
    // Can't go below ground (player feet at y=560)
    if (pos.y > 560) pos.y = 560;
    
    // Can't go above screen (sprite height is 96px)
    if (pos.y < 96) pos.y = 96;
}

void Game::handle_input() {
    if (key_down(A_KEY)) {
        player.move_left();
    }
    else if (key_down(D_KEY)) {
        player.move_right();
    }
    else {
        player.stop_moving();
    }
    
    if (key_typed(W_KEY)) {
        player.jump();
    }
    
    if (key_down(ESCAPE_KEY)) {
        quit_game = true;
    }
}