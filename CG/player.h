#ifndef PLAYER_H
#define PLAYER_H

// Initialise player state
void initPlayer();

// Handle keyboard-based movement (WASD + arrow keys)
void movePlayer(unsigned char key);
void movePlayerSpecial(int key);

// Draw the player character (coloured humanoid proxy)
void drawPlayer();

// Draw only the player's body (without shadow/reflections) for stencil reflection
void drawPlayerBody();
float getPlayerBobY();

// Update animation timers (call every frame)
void updatePlayer(float dt);

// ─── Getters for HUD display ───────────────────────────────────────────────
float getPlayerLegSwing();   // current thigh swing angle (degrees)

#endif // PLAYER_H
