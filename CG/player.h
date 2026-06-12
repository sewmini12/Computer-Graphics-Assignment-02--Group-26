#ifndef PLAYER_H
#define PLAYER_H

// Initialise player state
void initPlayer();

// Handle keyboard-based movement (WASD + arrow keys)
void movePlayer(unsigned char key);
void movePlayerSpecial(int key);

// Draw the player character (coloured humanoid proxy)
void drawPlayer();

// Update animation timers (call every frame)
void updatePlayer(float dt);

#endif // PLAYER_H
