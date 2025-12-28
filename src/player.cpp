#include "player.h"
#include <iostream>

Player::Player() {
    // Set initial position
    position = point_at(400, 300);
    
    // Set movement properties
    speed = 4.0f;
    jump_force = -15.0f;
    gravity = 0.8f;
    vertical_velocity = 0;
    is_grounded = true;
    facing_right = true;
    
    // Animation properties
    current_frame_index = 0;
    animation_timer = 0;
    current_state = STATE_IDLE;
    current_animation = nullptr;
    
    // Setup all animations
    setup_animations();
    
    // Start with idle animation
    set_state(STATE_IDLE);
}

void Player::setup_animations() {
    // Load only the animations we need for basic movement
    
    load_animation("assets/sprites/striker/spr_StrikerIdle_strip.png", 
                   STATE_IDLE, 8, 96, 96, 10, true);
    
    load_animation("assets/sprites/striker/spr_StrikerRun_strip.png", 
                   STATE_RUN, 8, 96, 96, 6, true);
    
    load_animation("assets/sprites/striker/spr_StrikerJump_strip.png", 
                   STATE_JUMP, 12, 96, 96, 8, false);
    
    // For falling, we'll use the same as jump but start at a later frame
    load_animation("assets/sprites/striker/spr_StrikerJump_strip.png", 
                   STATE_FALL, 12, 96, 96, 8, false);
    
    std::cout << "Setup animations for basic movement" << std::endl;
}

void Player::load_animation(const std::string& filename, PlayerState state, 
                           int frame_count, int frame_width, int frame_height, 
                           int speed, bool loops) {
    Animation anim;
    anim.anim_bitmap = load_bitmap("anim_" + std::to_string(state), filename);
    anim.frame_count = frame_count;
    anim.frame_width = frame_width;
    anim.frame_height = frame_height;
    anim.animation_speed = speed;
    anim.loops = loops;
    
    switch(state) {
        case STATE_IDLE: anim.name = "Idle"; break;
        case STATE_RUN: anim.name = "Run"; break;
        case STATE_JUMP: anim.name = "Jump"; break;
        case STATE_FALL: anim.name = "Fall"; break;
        default: anim.name = "Unknown";
    }
    
    animations[state] = anim;
}

void Player::update() {
    // Apply gravity if not grounded
    if (!is_grounded) {
        vertical_velocity += gravity;
        position.y += vertical_velocity;
        
        // Check if landed
        if (position.y >= 600) {
            position.y = 600;
            vertical_velocity = 0;
            is_grounded = true;
            
            if (current_state == STATE_JUMP || current_state == STATE_FALL) {
                set_state(STATE_IDLE);
            }
        } else if (vertical_velocity > 0 && current_state == STATE_JUMP) {
            // Start falling
            set_state(STATE_FALL);
        }
    }
    
    // Update animation
    update_animation();
}

void Player::update_animation() {
    if (!current_animation) {
        return;
    }
    
    animation_timer++;
    
    if (animation_timer >= current_animation->animation_speed) {
        animation_timer = 0;
        current_frame_index++;
        
        if (current_frame_index >= current_animation->frame_count) {
            if (current_animation->loops) {
                current_frame_index = 0;
            } else {
                current_frame_index = current_animation->frame_count - 1;
                
                // For jump/fall, just stay on the last frame until we land
                if (current_state == STATE_JUMP && !is_grounded) {
                    set_state(STATE_FALL);
                }
            }
        }
    }
}

void Player::set_state(PlayerState new_state) {
    if (current_state == new_state && current_animation != nullptr) {
        return;
    }
    
    current_state = new_state;
    
    if (animations.find(new_state) != animations.end()) {
        current_animation = &animations[new_state];
        current_frame_index = 0;
        animation_timer = 0;
        
        // For fall state, start at frame 6 (mid-air frame)
        if (new_state == STATE_FALL) {
            current_frame_index = 6;
        }
    }
}

void Player::draw() {
    if (!current_animation) {
        return;
    }
    
    // Get the rectangle for the current frame
    int frame_x = current_frame_index * current_animation->frame_width;
    int frame_y = 0;
    
    rectangle frame_rect = rectangle_from(frame_x, frame_y, 
                                         current_animation->frame_width, 
                                         current_animation->frame_height);
    
    // Calculate draw position (center the sprite horizontally, align feet to position.y)
    float draw_x = position.x - current_animation->frame_width / 2;
    float draw_y = position.y - current_animation->frame_height;
    
    // Create drawing options
    drawing_options opts = option_part_bmp(frame_rect);
    
    // Flip the sprite if facing left
    if (!facing_right) {
        opts = option_scale_bmp(-1.0, 1.0, opts);
    }
    
    draw_bitmap(current_animation->anim_bitmap, draw_x, draw_y, opts);
}

void Player::move_left() {
    position.x -= speed;
    facing_right = false;
    
    // Only change to RUN state if not jumping/falling
    if (is_grounded && current_state != STATE_JUMP && current_state != STATE_FALL) {
        set_state(STATE_RUN);
    }
}

void Player::move_right() {
    position.x += speed;
    facing_right = true;
    
    // Only change to RUN state if not jumping/falling
    if (is_grounded && current_state != STATE_JUMP && current_state != STATE_FALL) {
        set_state(STATE_RUN);
    }
}

void Player::jump() {
    if (is_grounded) {
        vertical_velocity = jump_force;
        is_grounded = false;
        set_state(STATE_JUMP);
    }
}

void Player::stop_moving() {
    if (is_grounded && current_state == STATE_RUN) {
        set_state(STATE_IDLE);
    }
}