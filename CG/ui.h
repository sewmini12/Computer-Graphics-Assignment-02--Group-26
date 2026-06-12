#ifndef UI_H
#define UI_H

// Draw the entire HUD overlay (call after all 3D drawing, before swap)
void drawHUD();

// Flash message system
void setFlashMessage(const char* msg, float durationSec = 3.0f);
void updateFlash(float dt);

#endif // UI_H
