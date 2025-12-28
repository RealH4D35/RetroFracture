#include "splashkit.h"
#include "src/game.h"

int main() {
    // Open window (within arcade machine specs)
    open_window("RetroFracture", 800, 600);
    
    // Remove border for arcade machine compliance
    window_toggle_border("RetroFracture");
    
    // Create and run the game
    Game game;
    game.run();
    
    return 0;
}