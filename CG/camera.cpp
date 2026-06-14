#include <GL/freeglut.h>
#include <cmath>
#include "camera.h"
#include "globals.h"
#include "collision.h"

// ─── Constants ────────────────────────────────────────────────────────────────
static const float MIN_PITCH =   5.0f;
static const float MAX_PITCH =  80.0f;
static const float MIN_DIST  =   4.0f;
static const float MAX_DIST  =  40.0f;

// Mouse sensitivity (degrees per pixel)
static const float YAW_SENS   = 0.30f;   // horizontal drag sensitivity
static const float PITCH_SENS = 0.25f;   // vertical drag sensitivity — lower = more control

// Smoothed camera angles (interpolated toward target each frame)
static float smoothYaw   = 0.0f;
static float smoothPitch = 25.0f;
static float targetYaw   = 0.0f;
static float targetPitch = 25.0f;

static float currentCamDist = 18.0f;

// ─── applyCamera ─────────────────────────────────────────────────────────────
// Orbits around the player using smoothed yaw/pitch.
void applyCamera()
{
    // Smooth interpolation toward target angles (simple exponential ease)
    smoothYaw   += (targetYaw   - smoothYaw)   * 0.18f;
    smoothPitch += (targetPitch - smoothPitch) * 0.18f;

    // Write back smoothed values so player movement reads correct yaw
    camYaw   = smoothYaw;
    camPitch = smoothPitch;

    float yawRad   = camYaw   * 3.14159265f / 180.0f;
    float pitchRad = camPitch * 3.14159265f / 180.0f;

    if (firstPerson) {
        float yawRadFirst = camYaw * 3.14159265f / 180.f;
        float eyeX = playerX, eyeY = playerY + 1.8f, eyeZ = playerZ;
        float lx = eyeX - sinf(yawRadFirst);
        float lz = eyeZ - cosf(yawRadFirst);
        gluLookAt(eyeX, eyeY, eyeZ, lx, eyeY, lz, 0.0f, 1.0f, 0.0f);
    } else {
        float targetY = playerY + 1.2f;  // look at character chest

        // Compute un-obstructed target camera coordinates in XZ plane
        float targetEyeX = playerX + camDist * sinf(yawRad) * cosf(pitchRad);
        float targetEyeZ = playerZ + camDist * cosf(yawRad) * cosf(pitchRad);

        // Raycast to find if path is blocked
        float t = checkRayCollision(playerX, playerZ, targetEyeX, targetEyeZ, 0.5f);
        
        // Allowed distance based on collision (clamp to slightly above 0 to prevent exact player overlap)
        float allowedDist = camDist * t;
        if (allowedDist < 1.0f) allowedDist = 1.0f;

        // Smoothly interpolate current camera distance
        currentCamDist += (allowedDist - currentCamDist) * 0.2f;

        // Spherical → Cartesian offset using currentCamDist
        float eyeX = playerX + currentCamDist * sinf(yawRad) * cosf(pitchRad);
        float eyeY = playerY + 1.0f + currentCamDist * sinf(pitchRad);
        float eyeZ = playerZ + currentCamDist * cosf(yawRad) * cosf(pitchRad);

        gluLookAt(eyeX, eyeY, eyeZ,
                  playerX, targetY, playerZ,
                  0.0f, 1.0f, 0.0f);
    }
}

// ─── Mouse Button ─────────────────────────────────────────────────────────────
void cameraMouseButton(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        mouseDown  = (state == GLUT_DOWN);
        lastMouseX = x;
        lastMouseY = y;
    }
}

// ─── Mouse Motion (drag) ──────────────────────────────────────────────────────
// Dragging LEFT/RIGHT orbits the camera horizontally (yaw).
// Dragging UP   rotates the camera DOWN  (pitch increases → higher eye point).
// Dragging DOWN rotates the camera UP (pitch decreases → lower angle).
void cameraMouseMotion(int x, int y)
{
    if (!mouseDown) return;

    int dx = x - lastMouseX;   // positive = mouse moved right
    int dy = y - lastMouseY;   // positive = mouse moved down (screen-space)

    // Horizontal: drag right → yaw increases (orbit right)
    targetYaw += dx * YAW_SENS;

    // Vertical: drag UP (dy < 0) → pitch increases (camera goes higher / looks more downward)
    //           drag DOWN (dy > 0) → pitch decreases (camera goes lower / levels out)
    targetPitch -= dy * PITCH_SENS;

    // Clamp pitch so camera can't flip over or clip underground
    if (targetPitch < MIN_PITCH) targetPitch = MIN_PITCH;
    if (targetPitch > MAX_PITCH) targetPitch = MAX_PITCH;

    lastMouseX = x;
    lastMouseY = y;

    glutPostRedisplay();
}

// ─── Mouse Wheel (zoom) ───────────────────────────────────────────────────────
void cameraMouseWheel(int wheel, int dir, int x, int y)
{
    (void)wheel; (void)x; (void)y;
    camDist -= dir * 1.2f;   // slightly gentler zoom step
    if (camDist < MIN_DIST) camDist = MIN_DIST;
    if (camDist > MAX_DIST) camDist = MAX_DIST;
    glutPostRedisplay();
}
