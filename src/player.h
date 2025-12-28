#ifndef PLAYER_H
#define PLAYER_H

#include "splashkit.h"
#include <map>

enum PlayerState {
    STATE_IDLE,
    STATE_RUN,
    STATE_JUMP,
    STATE_FALL
};

struct Animation {
    bitmap anim_bitmap;
    int frame_count;
    int frame_width;
    int frame_height;
    int animation_speed;
    bool loops;
    std::string name;
};

class Player {
private:
    point_2d position;
    float speed;
    float jump_force;
    float gravity;
    float vertical_velocity;
    bool is_grounded;
    bool facing_right;
    
    PlayerState current_state;
    int current_frame_index;
    int animation_timer;
    std::map<PlayerState, Animation> animations;
    Animation* current_animation;
    
public:
    Player();
    
    void update();
    void draw();
    
    void set_state(PlayerState new_state);
    PlayerState get_state() const { return current_state; }
    
    void move_left();
    void move_right();
    void jump();
    void stop_moving();
    
    point_2d get_position() const { return position; }
    bool get_is_grounded() const { return is_grounded; }
    bool get_facing_right() const { return facing_right; }
    
    int get_current_frame_index() const { return current_frame_index; }
    std::string get_current_animation_name() const { 
        return current_animation ? current_animation->name : "None"; 
    }
    
private:
    void setup_animations();
    void load_animation(const std::string& filename, PlayerState state, 
                       int frame_count, int frame_width, int frame_height, 
                       int speed, bool loops);
    void update_animation();
};

#endif