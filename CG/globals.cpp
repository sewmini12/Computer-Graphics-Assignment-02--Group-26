#include "globals.h"

// ─── Player ───────────────────────────────────────────────────────────────────
float playerX     =  0.0f;
float playerY     =  0.5f;
float playerZ     =  0.0f;
float playerAngle =  0.0f;
float playerSpeed =  0.30f;   // slightly faster for better feel
bool isSprinting  = false;
float stamina     = 100.0f;
float playerVelY  =  0.0f;
bool isGrounded   = true;

// ─── Camera ───────────────────────────────────────────────────────────────────
float camYaw   =  0.0f;
float camPitch = 25.0f;
float camDist  = 18.0f;
int   lastMouseX = 0;
int   lastMouseY = 0;
bool  mouseDown  = false;
bool  firstPerson = false;

// ─── Day/Night ────────────────────────────────────────────────────────────────
float timeOfDay = 0.3f;   // start mid-morning
float daySpeed  = 0.0002f;

// ─── Game State ───────────────────────────────────────────────────────────────
int  score       = 0;
bool gameRunning = true;

// ─── Game Timer ───────────────────────────────────────────────────────────────
float gameElapsedTime = 0.0f;
bool  gameCompleted   = false;
float completionTime  = 0.0f;
float missionTimeLeft = 0.0f;

// ─── Window ───────────────────────────────────────────────────────────────────
int windowWidth  = 1280;
int windowHeight =  800;

// ─── Graphics Concepts State ──────────────────────────────────────────────────
bool showReflection    = false;
bool showTransformDemo = false;
bool splitScreen       = false;


