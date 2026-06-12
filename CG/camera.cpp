#include <GL/freeglut.h>
#include <cmath>
#include "camera.h"
#include "globals.h"

// ─── Constants ────────────────────────────────────────────────────────────────
static const float MIN_PITCH =   5.0f;
static const float MAX_PITCH =  80.0f;
static const float MIN_DIST  =   4.0f;
static const float MAX_DIST  =  40.0f;

// ─── applyCamera ─────────────────────────────────────────────────────────────
// Computes eye position by orbiting around the player at (playerX,playerY+1,playerZ)
void applyCamera()
{
    float yawRad   = camYaw   * 3.14159265f / 180.0f;
    float pitchRad = camPitch * 3.14159265f / 180.0f;

    // Spherical → Cartesian offset from target
    float eyeX = playerX + camDist * sinf(yawRad) * cosf(pitchRad);
    float eyeY = playerY + 1.0f + camDist * sinf(pitchRad);
    float eyeZ = playerZ + camDist * cosf(yawRad) * cosf(pitchRad);

    float targetY = playerY + 1.2f;  // look at character chest

    gluLookAt(eyeX, eyeY, eyeZ,
              playerX, targetY, playerZ,
              0.0f, 1.0f, 0.0f);
}

// ─── Mouse Button ─────────────────────────────────────────────────────────────
void cameraMouseButton(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON) {
        mouseDown = (state == GLUT_DOWN);
        lastMouseX = x;
        lastMouseY = y;
    }
}

// ─── Mouse Motion (drag) ──────────────────────────────────────────────────────
void cameraMouseMotion(int x, int y)
{
    if (!mouseDown) return;

    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    camYaw   += dx * 0.4f;
    camPitch += dy * 0.4f;

    // Clamp pitch so camera doesn't flip
    if (camPitch < MIN_PITCH) camPitch = MIN_PITCH;
    if (camPitch > MAX_PITCH) camPitch = MAX_PITCH;

    lastMouseX = x;
    lastMouseY = y;

    glutPostRedisplay();
}

// ─── Mouse Wheel (zoom) ───────────────────────────────────────────────────────
void cameraMouseWheel(int wheel, int dir, int x, int y)
{
    (void)wheel; (void)x; (void)y;
    camDist -= dir * 1.5f;
    if (camDist < MIN_DIST) camDist = MIN_DIST;
    if (camDist > MAX_DIST) camDist = MAX_DIST;
    glutPostRedisplay();
}
