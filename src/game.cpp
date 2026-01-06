#include "game.h"
#include <iostream>

Game::Game() : quit_game(false), camera_pos(point_at(0, 0)) {
    // Load the background image
    background = load_bitmap("background", "assets/envs/Apartment Hallway.png");
    
    // Get original background dimensions
    background_original_width = bitmap_width(background);
    background_original_height = bitmap_height(background);
    
    // Instead of forcing the height to 600, let's scale proportionally
    // Calculate how to show the entire width (639px) in the 800px window
    // This means we'll have void on top and bottom, but can see the whole hallway width
    scale_factor = 800.0f / background_original_width;  // Scale to fit width in window
    
    // Calculate scaled dimensions
    background_scaled_width = background_original_width * scale_factor;  // Should be ~800
    background_scaled_height = background_original_height * scale_factor;  // Should be ~450
    
    write_line("Original background: " + std::to_string(background_original_width) + "x" + 
               std::to_string(background_original_height));
    write_line("Scaled background: " + std::to_string(background_scaled_width) + "x" + 
               std::to_string(background_scaled_height));
    write_line("Scale factor: " + std::to_string(scale_factor));
    
    // Calculate where to position the background vertically (center it)
    background_y_offset = (600 - background_scaled_height) / 2;
    
    // Set initial player position (center of screen)
    float start_x = 400;  // Center of screen
    float start_y = 265;  // Middle of background vertically
    
    player.set_position(point_at(start_x, start_y));
    
    // Start camera at 0,0 (will center on player)
    camera_pos = point_at(0, 0);
    
    write_line("Player starting at: " + std::to_string(start_x) + ", " + std::to_string(start_y));
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
    
    // NO CLAMPING to world boundaries - camera can follow player anywhere
    // This allows us to see the entire image by moving the player
    
    write_line("Camera: " + std::to_string(camera_pos.x) + ", " + std::to_string(camera_pos.y));
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
    
    // Visual indicator for image boundaries
    float screen_left_boundary = -camera_pos.x;
    float screen_right_boundary = -camera_pos.x + background_scaled_width;
    float screen_top_boundary = background_y_offset - camera_pos.y;
    float screen_bottom_boundary = background_y_offset - camera_pos.y + background_scaled_height;
    
    // Draw image boundaries (only if they're visible on screen)
    if (screen_left_boundary >= 0 && screen_left_boundary <= 800) {
        draw_line(COLOR_GREEN, screen_left_boundary, 0, screen_left_boundary, 600);
        draw_text("LEFT", COLOR_GREEN, screen_left_boundary + 5, 20);
    }
    
    if (screen_right_boundary >= 0 && screen_right_boundary <= 800) {
        draw_line(COLOR_GREEN, screen_right_boundary, 0, screen_right_boundary, 600);
        draw_text("RIGHT", COLOR_GREEN, screen_right_boundary - 40, 20);
    }
    
    if (screen_top_boundary >= 0 && screen_top_boundary <= 600) {
        draw_line(COLOR_GREEN, 0, screen_top_boundary, 800, screen_top_boundary);
        draw_text("TOP", COLOR_GREEN, 10, screen_top_boundary + 5);
    }
    
    if (screen_bottom_boundary >= 0 && screen_bottom_boundary <= 600) {
        draw_line(COLOR_GREEN, 0, screen_bottom_boundary, 800, screen_bottom_boundary);
        draw_text("BOTTOM", COLOR_GREEN, 10, screen_bottom_boundary - 20);
    }
    
    // Draw center crosshair to show camera center
    draw_line(COLOR_RED, 398, 298, 402, 302);
    draw_line(COLOR_RED, 402, 298, 398, 302);
}

void Game::check_boundaries() {
    point_2d pos = player.get_position();
    
    // REMOVED LEFT AND RIGHT BOUNDARIES - player can move freely horizontally
    
    // Ground level - bottom of background (in world coordinates)
    float ground_level = background_scaled_height - 185;
    
    // Can't go below ground
    if (pos.y > ground_level) {
        pos.y = ground_level;
        if (!player.get_is_grounded() && player.get_state() == STATE_FALL) {
            player.set_state(STATE_IDLE);
        }
        player.set_grounded(true);
    }
    
    // Can't go above top of world
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
    
    // Debug: reset camera and player position
    if (key_typed(R_KEY)) {
        camera_pos = point_at(0, 0);
        player.set_position(point_at(400, 300));
    }
    
    // Debug: move camera with arrow keys
    if (key_down(LEFT_KEY)) {
        camera_pos.x -= 10;
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