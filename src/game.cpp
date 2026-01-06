#include "game.h"
#include <iostream>

Game::Game() : quit_game(false), camera_pos(point_at(0, 0)) {
    // Load the background image
    background = load_bitmap("background", "assets/envs/Apartment Hallway.png");
    
    // Get original background dimensions
    background_original_width = bitmap_width(background);
    background_original_height = bitmap_height(background);
    
    // Scale to fit width (800px)
    scale_factor = 800.0f / background_original_width;
    
    // Calculate scaled dimensions
    background_scaled_width = background_original_width * scale_factor;
    background_scaled_height = background_original_height * scale_factor;
    
    // Calculate where to position the background vertically (center it)
    background_y_offset = (600 - background_scaled_height) / 2;
    
    // Set ground level
    ground_level = background_scaled_height - 189;
    
    // ====================================================
    // START PLAYER AT LEFT EDGE ON THE GROUND
    // ====================================================
    float start_x = 48.0f;  // Half sprite width (96/2), so player doesn't go off-screen
    float start_y = ground_level;  // On the ground
    
    player.set_position(point_at(start_x, start_y));
    
    // Set player to grounded state immediately
    player.set_grounded(true);
    player.set_state(STATE_IDLE);
    
    // Start camera so player is at left edge of screen
    // Camera x = player.x - 400 (center of screen)
    camera_pos = point_at(start_x - 400, start_y - 300);
    
    // Clamp camera to not go past left edge of background
    if (camera_pos.x < 0) camera_pos.x = 0;
    
    write_line("Player starting at left edge: " + std::to_string(start_x) + ", " + std::to_string(start_y));
    write_line("Background Y offset: " + std::to_string(background_y_offset));
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
    update_camera();
    check_boundaries();
}

void Game::update_camera() {
    point_2d player_pos = player.get_position();
    
    // Center camera on player (both horizontally and vertically)
    float target_camera_x = player_pos.x - 400;  // Half of window width
    float target_camera_y = player_pos.y - 300;  // Half of window height
    
    // Smooth camera movement
    camera_pos.x = camera_pos.x * 0.9f + target_camera_x * 0.1f;
    camera_pos.y = camera_pos.y * 0.9f + target_camera_y * 0.1f;
    
    // Clamp camera to left edge only (can't go left of background)
    if (camera_pos.x < 0) camera_pos.x = 0;
}

point_2d Game::world_to_screen(point_2d world_pos) {
    // Convert world coordinates to screen coordinates
    return point_at(world_pos.x - camera_pos.x, 
                   world_pos.y - camera_pos.y + background_y_offset);
}

void Game::draw() {
    // Clear with a dark color (better for hallway)
    clear_screen(COLOR_BLACK);
    
    // Draw the scaled background at the correct Y offset
    float draw_x = -camera_pos.x;
    float draw_y = background_y_offset - camera_pos.y;  // Apply vertical camera movement too
    
    drawing_options opts = option_scale_bmp(scale_factor, scale_factor);
    draw_bitmap(background, draw_x, draw_y, opts);
    
    // Draw player at screen position
    point_2d screen_pos = world_to_screen(player.get_position());
    player.draw_at(screen_pos);
    
    // Draw simple instructions
    draw_text("CONTROLS: A (Left), D (Right), W (Jump)", COLOR_YELLOW, 10, 10);
    draw_text("ESC to quit | R to reset", COLOR_YELLOW, 10, 30);
    
    // Debug info - top section
    point_2d world_pos = player.get_position();
    std::string world_text = "WORLD: (" + std::to_string((int)world_pos.x) + 
                            ", " + std::to_string((int)world_pos.y) + ")";
    draw_text(world_text, COLOR_YELLOW, 10, 50);
    
    std::string camera_text = "CAMERA: (" + std::to_string((int)camera_pos.x) + 
                             ", " + std::to_string((int)camera_pos.y) + ")";
    draw_text(camera_text, COLOR_YELLOW, 10, 70);
    
    std::string bg_text = "BG: " + std::to_string((int)background_scaled_width) + 
                         "x" + std::to_string((int)background_scaled_height);
    draw_text(bg_text, COLOR_YELLOW, 10, 90);
    
    // Draw ground line
    float screen_ground_y = world_to_screen(point_at(0, ground_level)).y;
    draw_line(COLOR_RED, 0, screen_ground_y, 800, screen_ground_y);
    draw_text("GROUND", COLOR_RED, 10, screen_ground_y - 10);
    
    // Draw left boundary
    if (camera_pos.x <= 0) {
        draw_line(COLOR_RED, 0, 0, 0, 600);
        draw_text("LEFT BOUNDARY", COLOR_RED, 5, 20);
    }
}

void Game::check_boundaries() {
    point_2d pos = player.get_position();
    
    // ====================================================
    // LEFT-SIDE COLLISION ONLY
    // ====================================================
    float sprite_half_width = 48;  // 96px sprite, half is 48px
    
    // Left boundary - player can't go past left edge
    if (pos.x < sprite_half_width) {
        pos.x = sprite_half_width;
    }
    
    // No right boundary - player can move freely to the right
    
    // Ground collision
    if (pos.y > ground_level) {
        pos.y = ground_level;
        if (!player.get_is_grounded() && player.get_state() == STATE_FALL) {
            player.set_state(STATE_IDLE);
        }
        player.set_grounded(true);
    }
    
    // Ceiling collision
    float ceiling_level = 50;
    if (pos.y < ceiling_level) {
        pos.y = ceiling_level;
        player.stop_vertical_movement();
    }
    
    // Update player position
    player.set_position(pos);
}

void Game::handle_input() {
    if (key_down(A_KEY)) {
        // Check if player is at left boundary
        point_2d pos = player.get_position();
        float sprite_half_width = 48;
        if (pos.x > sprite_half_width) {  // Only move left if not at boundary
            player.move_left();
        }
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
    
    // Debug: reset to starting position
    if (key_typed(R_KEY)) {
        float start_x = 48.0f;
        player.set_position(point_at(start_x, ground_level));
        player.set_grounded(true);
        player.set_state(STATE_IDLE);
        camera_pos = point_at(start_x - 400, ground_level - 300);
        if (camera_pos.x < 0) camera_pos.x = 0;
    }
    
    // Debug: move camera with arrow keys
    if (key_down(LEFT_KEY)) {
        camera_pos.x -= 10;
        if (camera_pos.x < 0) camera_pos.x = 0;
    }
    if (key_down(RIGHT_KEY)) {
        camera_pos.x += 10;
    }
    if (key_down(UP_KEY)) {
        camera_pos.y -= 10;
    }
    if (key_down(DOWN_KEY)) {
        camera_pos.y += 10;
    }
}