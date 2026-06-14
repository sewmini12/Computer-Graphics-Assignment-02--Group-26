#ifndef GLOBALS_H
#define GLOBALS_H

// ─── Player Position ───────────────────────────────────────────────────────────
extern float playerX;
extern float playerY;
extern float playerZ;

// ─── Player Rotation (degrees, Y-axis) ────────────────────────────────────────
extern float playerAngle;

// ─── Player Speed ─────────────────────────────────────────────────────────────
extern float playerSpeed;
extern bool isSprinting;
extern float stamina;
extern float playerVelY;
extern bool isGrounded;

// ─── Camera Orbit (mouse drag) ────────────────────────────────────────────────
extern float camYaw;      // horizontal orbit around player
extern float camPitch;    // vertical orbit angle
extern float camDist;     // distance from player
extern int   lastMouseX;
extern int   lastMouseY;
extern bool  mouseDown;
extern bool  firstPerson;

// ─── Day / Night cycle ────────────────────────────────────────────────────────
extern float timeOfDay;   // 0.0 = midnight, 0.5 = noon, wraps 0..1
extern float daySpeed;

// ─── Game State ───────────────────────────────────────────────────────────────
extern int  score;
extern bool gameRunning;

// ─── Game Timer ───────────────────────────────────────────────────────────────
extern float gameElapsedTime;   // total elapsed seconds while playing
extern bool  gameCompleted;     // true when all missions done
extern float completionTime;    // time in seconds when game was completed
extern float missionTimeLeft;   // time left for current mission

// ─── Window Size ──────────────────────────────────────────────────────────────
extern int windowWidth;
extern int windowHeight;

// ─── Graphics Concepts State ──────────────────────────────────────────────────
extern bool showReflection;      // R key
extern bool showTransformDemo;   // T key
extern bool splitScreen;         // V key
extern bool isSprinting;         // Shift key (active movement)

#endif // GLOBALS_H

