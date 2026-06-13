#include <GL/freeglut.h>
#include <GL/glu.h>
#include <cmath>
#include "player.h"
#include "globals.h"
#include "collision.h"

// ─── Animation State ──────────────────────────────────────────────────────────
static float legSwing         = 0.0f;   // primary swing angle for THIGH (forward/back)
static float legDir           = 1.0f;
static bool  isMoving         = false;
static float runSpeed         = 0.0f;
static float bobY             = 0.0f;
static float bobPhase         = 0.0f;
static float breathPhase      = 0.0f;
static float headTurn         = 0.0f;
static float armSwing         = 0.0f;
static float targetPlayerAngle = 0.0f;  // desired facing direction (set by movePlayer)

// Derived per-leg knee bend angles (computed from thigh swing)
static float kneeBend    = 0.0f;   // shin angle relative to thigh (always bends forward)

// ─── GLU quadric (shared) ─────────────────────────────────────────────────────
static GLUquadric* qObj = nullptr;

void initPlayer()
{
    playerX     =  0.0f;
    playerY     =  0.0f;
    playerZ     =  5.0f;
    playerAngle =  0.0f;
    runSpeed    =  0.0f;
    legSwing    =  0.0f;
    qObj = gluNewQuadric();
    gluQuadricNormals(qObj, GLU_SMOOTH);
}

float getPlayerLegSwing() { return legSwing; }


// ─── Helpers ─────────────────────────────────────────────────────────────────

// Vertical cylinder centered at origin (height along +Y)
static void cyl(float radius, float height, int slices = 14)
{
    glTranslatef(0, -height * 0.5f, 0);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(qObj, radius, radius, height, slices, 1);
    gluDisk(qObj, 0, radius, slices, 1);
    glTranslatef(0, 0, height);
    gluDisk(qObj, 0, radius, slices, 1);
    glRotatef(90, 1, 0, 0);
    glTranslatef(0, height * 0.5f, 0);
}

// Tapered cylinder (top narrower)
static void cylTaper(float rBot, float rTop, float height, int slices = 14)
{
    glTranslatef(0, -height * 0.5f, 0);
    glRotatef(-90, 1, 0, 0);
    gluCylinder(qObj, rBot, rTop, height, slices, 1);
    gluDisk(qObj, 0, rBot, slices, 1);
    glTranslatef(0, 0, height);
    gluDisk(qObj, 0, rTop, slices, 1);
    glRotatef(90, 1, 0, 0);
    glTranslatef(0, height * 0.5f, 0);
}

// Sphere shortcut
static void sph(float r, int sl = 14, int st = 10)
{
    glutSolidSphere(r, sl, st);
}

// ─── Animation update ────────────────────────────────────────────────────────
void updatePlayer(float dt)
{
    float targetSpeed = isMoving ? 1.0f : 0.0f;
    runSpeed += (targetSpeed - runSpeed) * 9.0f * dt;

    breathPhase += dt * 1.3f;

    // ── Smooth player facing rotation ────────────────────────────────────────
    // Interpolate playerAngle toward targetPlayerAngle using the shortest
    // angular path (handles 350°→10° wrapping correctly).
    {
        float diff = targetPlayerAngle - playerAngle;
        // Wrap diff into [-180, +180] so we always turn the short way
        while (diff >  180.0f) diff -= 360.0f;
        while (diff < -180.0f) diff += 360.0f;
        // Turn speed: 12 rad/s equivalent — fast enough to feel responsive
        float turnRate = 720.0f * dt;   // degrees per frame (max)
        if (fabsf(diff) < turnRate)
            playerAngle = targetPlayerAngle;   // snap when very close
        else
            playerAngle += (diff > 0 ? turnRate : -turnRate);
        // Keep playerAngle in [0, 360)
        while (playerAngle >  360.0f) playerAngle -= 360.0f;
        while (playerAngle < -360.0f) playerAngle += 360.0f;
    }

    if (runSpeed > 0.01f) {
        bobPhase  += dt * 10.5f * runSpeed;
        // ── THIGH swings forward and back (primary leg swing) ─────────────
        legSwing  += legDir * 280.0f * dt * runSpeed;
        if (legSwing >  44.0f) legDir = -1.0f;
        if (legSwing < -44.0f) legDir =  1.0f;

        // ── KNEE bends when leg swings back (trailing leg) ─────────────────
        kneeBend  = fabsf(legSwing) * 0.55f;   // 0..~24 degrees of knee flex

        armSwing   = -legSwing * 0.75f;
        bobY       = sinf(bobPhase) * 0.07f * runSpeed;
        headTurn   = legSwing * 0.06f;
    } else {
        legSwing  *= 0.85f;
        kneeBend   = fabsf(legSwing) * 0.55f;
        armSwing   = -legSwing * 0.75f;
        bobPhase   = 0.0f;
        bobY       = sinf(breathPhase) * 0.012f;
        headTurn  *= 0.88f;
    }

    isMoving = false;
}

// ─── Player movement ─────────────────────────────────────────────────────────
void movePlayer(unsigned char key)
{
    isSprinting = ((glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0) || (key >= 'A' && key <= 'Z');

    float yawRad   = camYaw * 3.14159265f / 180.0f;
    float forwardX = -sinf(yawRad);
    float forwardZ = -cosf(yawRad);
    float rightX   =  cosf(yawRad);
    float rightZ   = -sinf(yawRad);

    float dx = 0.0f, dz = 0.0f;
    float currentSpeed = playerSpeed;
    if (isSprinting) {
        currentSpeed = playerSpeed * 1.8f;
    }

    switch (key) {
        case 'w': case 'W':
            dx = forwardX * currentSpeed; dz = forwardZ * currentSpeed;
            targetPlayerAngle = -camYaw; break;           // face forward (same as camera)
        case 's': case 'S':
            dx = -forwardX * currentSpeed; dz = -forwardZ * currentSpeed;
            targetPlayerAngle = -camYaw + 180.0f; break;  // face backward
        case 'a': case 'A':
            dx = -rightX * currentSpeed; dz = -rightZ * currentSpeed;
            targetPlayerAngle = -camYaw + 90.0f; break;   // face left
        case 'd': case 'D':
            dx = rightX * currentSpeed; dz = rightZ * currentSpeed;
            targetPlayerAngle = -camYaw - 90.0f; break;   // face right
        default: return;
    }

    float nx = playerX + dx, nz = playerZ + dz;
    if (!checkCollision(nx, playerZ)) playerX = nx;
    if (!checkCollision(playerX, nz)) playerZ = nz;

    const float BORDER = 93.0f;
    if (playerX < -BORDER) playerX = -BORDER;
    if (playerX >  BORDER) playerX =  BORDER;
    if (playerZ < -BORDER) playerZ = -BORDER;
    if (playerZ >  BORDER) playerZ =  BORDER;

    isMoving = true;
}

void movePlayerSpecial(int key)
{
    unsigned char fake = 0;
    if (key == GLUT_KEY_UP)    fake = 'w';
    if (key == GLUT_KEY_DOWN)  fake = 's';
    if (key == GLUT_KEY_LEFT)  fake = 'a';
    if (key == GLUT_KEY_RIGHT) fake = 'd';
    if (fake) movePlayer(fake);
}

// ══════════════════════════════════════════════════════════════════════════════
//   DRAW HUMAN BODY — helper that draws the body at the current matrix state
//   Used by both main draw and the reflected / shadowed copies.
// ══════════════════════════════════════════════════════════════════════════════

// Color constants (global to body drawing)
static const float SKIN_R  = 0.83f, SKIN_G  = 0.66f, SKIN_B  = 0.50f;
static const float SHIRT_R = 0.34f, SHIRT_G = 0.32f, SHIRT_B = 0.19f;
static const float PANT_R  = 0.28f, PANT_G  = 0.26f, PANT_B  = 0.16f;
static const float BOOT_R  = 0.16f, BOOT_G  = 0.11f, BOOT_B  = 0.07f;
static const float ARMOR_R = 0.18f, ARMOR_G = 0.18f, ARMOR_B = 0.15f;

// ─── Draw one leg ─────────────────────────────────────────────────────────────
//   side:  -1 = left,  +1 = right
//   thighAngle: forward/back swing of entire leg from hip (degrees)
//   kneeAngle:  shin bend forward from thigh (degrees, always >= 0)
static void drawLeg(float side, float thighAngle, float kneeAngle)
{
    // ── HIP JOINT (translation to hip socket position) ────────────────────
    glPushMatrix();
    glTranslatef(side * 0.13f, 0.80f, 0);

    // ── THIGH ROTATION (rotation around X = forward/back swing) ──────────
    glRotatef(thighAngle, 1, 0, 0);

    // Upper thigh cylinder
    glColor3f(PANT_R, PANT_G, PANT_B);
    glPushMatrix();
        glTranslatef(0, -0.22f, 0);
        cylTaper(0.100f, 0.085f, 0.44f);
    glPopMatrix();

    // ── KNEE JOINT (translate to knee position, then apply knee bend) ────
    glPushMatrix();
        glTranslatef(0, -0.46f, 0);

        // Knee sphere
        glColor3f(PANT_R * 0.8f, PANT_G * 0.8f, PANT_B * 0.8f);
        sph(0.092f, 12, 8);

        // ── SHIN ROTATION (knee bend — shin rotates back relative to thigh) ─
        glRotatef(kneeAngle, 1, 0, 0);

        // Shin / lower leg cylinder
        glPushMatrix();
            glTranslatef(0, -0.22f, 0);
            glColor3f(PANT_R, PANT_G, PANT_B);
            cylTaper(0.085f, 0.075f, 0.38f);
        glPopMatrix();

        // ── ANKLE JOINT ──────────────────────────────────────────────────
        glPushMatrix();
            glTranslatef(0, -0.43f, 0);

            // Ankle/boot cuff
            glColor3f(BOOT_R, BOOT_G, BOOT_B);
            glPushMatrix();
                glTranslatef(0, 0.10f, 0);
                cyl(0.090f, 0.20f);
            glPopMatrix();

            // ── FOOT (translation forward + scale = shoe shape) ──────────
            glPushMatrix();
                // Translate foot forward so it points in walking direction
                glTranslatef(0, 0.04f, 0.05f);
                // Shear-like scale: flatten vertically, extend forward
                glScalef(1.0f, 0.35f, 1.6f);
                sph(0.090f, 12, 8);
            glPopMatrix();

        glPopMatrix(); // ankle
    glPopMatrix(); // knee

    glPopMatrix(); // hip
}

// ─── Draw the full body (no outer matrix applied) ─────────────────────────────
static void drawBody(float alpha = 1.0f)
{
    // ════════════════════════════════════════════════════════
    //   LEFT LEG  (side = -1)
    //   thighAngle =  legSwing  (swings forward when positive)
    //   kneeAngle  =  kneeBend  (trailing leg bends its knee more)
    // ════════════════════════════════════════════════════════
    // Left leg swings forward → knee barely bends
    // Left leg trails back → knee bends noticeably
    float leftKnee  = (legSwing < 0) ? kneeBend : kneeBend * 0.3f;
    float rightKnee = (legSwing > 0) ? kneeBend : kneeBend * 0.3f;

    drawLeg(-1.0f,  legSwing, leftKnee);   // LEFT  — thigh swings +legSwing
    drawLeg( 1.0f, -legSwing, rightKnee);  // RIGHT — opposite phase

    // ════════════════════════════════
    //   PELVIS / HIP sphere
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(0, 0.82f, 0);
    glColor3f(PANT_R * 1.1f, PANT_G * 1.1f, PANT_B * 1.1f);
    glScalef(1.5f, 0.8f, 0.9f);
    sph(0.16f, 14, 10);
    glPopMatrix();

    // CG Concept: Shearing
    if (isSprinting) {
        glPushMatrix();
        float sh = -0.18f; // Lean forward
        float shearMat[16] = {
            1.0f,    sh, 0.0f, 0.0f, // col 0
            0.0f,  1.0f, 0.0f, 0.0f, // col 1
            0.0f,  0.0f, 1.0f, 0.0f, // col 2
            0.0f,  0.0f, 0.0f, 1.0f  // col 3
        };
        glMultMatrixf(shearMat);
    }

    // ════════════════════════════════
    //   TORSO
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(0, 1.30f, 0);
    glColor3f(SHIRT_R, SHIRT_G, SHIRT_B);
    cylTaper(0.185f, 0.230f, 0.72f, 16);
    glPopMatrix();

    // Chest armor plate
    glPushMatrix();
    glTranslatef(0, 1.55f, 0.15f);
    glColor3f(ARMOR_R, ARMOR_G, ARMOR_B);
    glScalef(1.8f, 1.3f, 0.5f);
    sph(0.17f, 14, 10);
    glPopMatrix();

    // Belly
    glPushMatrix();
    glTranslatef(0, 1.12f, 0.04f);
    glColor3f(SHIRT_R * 0.9f, SHIRT_G * 0.9f, SHIRT_B * 0.9f);
    glScalef(1.4f, 0.7f, 0.6f);
    sph(0.16f, 12, 8);
    glPopMatrix();

    // ════════════════════════════════
    //   SHOULDERS
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(-0.30f, 1.72f, 0);
    glColor3f(SHIRT_R, SHIRT_G, SHIRT_B);
    sph(0.115f, 12, 8);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.30f, 1.72f, 0);
    glColor3f(SHIRT_R, SHIRT_G, SHIRT_B);
    sph(0.115f, 12, 8);
    glPopMatrix();

    // ════════════════════════════════
    //   ARMS
    // ════════════════════════════════
    // LEFT ARM
    glPushMatrix();
    glTranslatef(-0.30f, 1.70f, 0);
    glRotatef(armSwing, 1, 0, 0);

        glColor3f(SHIRT_R, SHIRT_G, SHIRT_B);
        glPushMatrix(); glTranslatef(0, -0.17f, 0); cylTaper(0.075f, 0.065f, 0.34f); glPopMatrix();
        glPushMatrix(); glTranslatef(0, -0.36f, 0);
            glColor3f(SKIN_R * 0.9f, SKIN_G * 0.9f, SKIN_B * 0.9f);
            sph(0.068f, 10, 8);
        glPopMatrix();
        glPushMatrix(); glTranslatef(0, -0.55f, 0);
            glColor3f(SKIN_R, SKIN_G, SKIN_B);
            cylTaper(0.065f, 0.055f, 0.32f);
        glPopMatrix();
        glPushMatrix(); glTranslatef(0, -0.73f, 0);
            glColor3f(SKIN_R, SKIN_G, SKIN_B);
            sph(0.058f, 10, 8);
        glPopMatrix();
        glPushMatrix(); glTranslatef(0, -0.84f, 0.02f);
            glColor3f(SKIN_R, SKIN_G, SKIN_B);
            glScalef(0.8f, 0.6f, 1.2f);
            sph(0.072f, 10, 8);
        glPopMatrix();

    glPopMatrix(); // left arm

    // RIGHT ARM
    glPushMatrix();
    glTranslatef(0.30f, 1.70f, 0);
    glRotatef(-armSwing, 1, 0, 0);

        glColor3f(SHIRT_R, SHIRT_G, SHIRT_B);
        glPushMatrix(); glTranslatef(0, -0.17f, 0); cylTaper(0.075f, 0.065f, 0.34f); glPopMatrix();
        glPushMatrix(); glTranslatef(0, -0.36f, 0);
            glColor3f(SKIN_R * 0.9f, SKIN_G * 0.9f, SKIN_B * 0.9f);
            sph(0.068f, 10, 8);
        glPopMatrix();
        glPushMatrix(); glTranslatef(0, -0.55f, 0);
            glColor3f(SKIN_R, SKIN_G, SKIN_B);
            cylTaper(0.065f, 0.055f, 0.32f);
        glPopMatrix();
        glPushMatrix(); glTranslatef(0, -0.73f, 0);
            glColor3f(SKIN_R, SKIN_G, SKIN_B);
            sph(0.058f, 10, 8);
        glPopMatrix();
        glPushMatrix(); glTranslatef(0, -0.84f, 0.02f);
            glColor3f(SKIN_R, SKIN_G, SKIN_B);
            glScalef(0.8f, 0.6f, 1.2f);
            sph(0.072f, 10, 8);
        glPopMatrix();

    glPopMatrix(); // right arm

    // ════════════════════════════════
    //   NECK
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(0, 1.82f, 0);
    glColor3f(SKIN_R, SKIN_G, SKIN_B);
    cylTaper(0.072f, 0.065f, 0.16f, 12);
    glPopMatrix();

    // ════════════════════════════════
    //   HEAD
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(0, 2.06f, 0);
    glRotatef(headTurn, 0, 1, 0);

        // Skull
        glColor3f(SKIN_R, SKIN_G, SKIN_B);
        glPushMatrix();
        glScalef(1.0f, 1.15f, 0.95f);
        sph(0.185f, 16, 12);
        glPopMatrix();

        // Jaw
        glColor3f(SKIN_R * 0.97f, SKIN_G * 0.97f, SKIN_B * 0.97f);
        glPushMatrix();
        glTranslatef(0, -0.10f, 0.04f);
        glScalef(0.9f, 0.55f, 0.85f);
        sph(0.165f, 14, 10);
        glPopMatrix();

        // Left eye
        glPushMatrix();
        glTranslatef(-0.065f, 0.03f, 0.16f);
        glColor3f(0.05f, 0.05f, 0.05f);
        sph(0.022f, 8, 6);
        glColor3f(0.95f, 0.95f, 0.95f);
        glTranslatef(0, 0, -0.004f);
        sph(0.026f, 8, 6);
        glPopMatrix();

        // Right eye
        glPushMatrix();
        glTranslatef(0.065f, 0.03f, 0.16f);
        glColor3f(0.05f, 0.05f, 0.05f);
        sph(0.022f, 8, 6);
        glColor3f(0.95f, 0.95f, 0.95f);
        glTranslatef(0, 0, -0.004f);
        sph(0.026f, 8, 6);
        glPopMatrix();

        // Nose
        glPushMatrix();
        glTranslatef(0, -0.02f, 0.182f);
        glColor3f(SKIN_R * 0.92f, SKIN_G * 0.92f, SKIN_B * 0.88f);
        glScalef(0.5f, 0.6f, 1.0f);
        sph(0.038f, 8, 6);
        glPopMatrix();

        // Mouth
        glPushMatrix();
        glTranslatef(0, -0.07f, 0.168f);
        glColor3f(0.72f, 0.42f, 0.38f);
        glScalef(1.6f, 0.5f, 0.7f);
        sph(0.030f, 8, 6);
        glPopMatrix();

        // Ears
        glPushMatrix();
        glTranslatef(-0.183f, 0.0f, 0.0f);
        glColor3f(SKIN_R * 0.95f, SKIN_G * 0.95f, SKIN_B * 0.90f);
        glScalef(0.4f, 0.7f, 0.5f);
        sph(0.065f, 8, 6);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.183f, 0.0f, 0.0f);
        glColor3f(SKIN_R * 0.95f, SKIN_G * 0.95f, SKIN_B * 0.90f);
        glScalef(0.4f, 0.7f, 0.5f);
        sph(0.065f, 8, 6);
        glPopMatrix();

        // Eyebrows
        glPushMatrix();
        glTranslatef(-0.065f, 0.075f, 0.168f);
        glColor3f(0.25f, 0.18f, 0.10f);
        glScalef(1.5f, 0.4f, 0.6f);
        sph(0.030f, 8, 6);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.065f, 0.075f, 0.168f);
        glColor3f(0.25f, 0.18f, 0.10f);
        glScalef(1.5f, 0.4f, 0.6f);
        sph(0.030f, 8, 6);
        glPopMatrix();

        // Hair cap
        glColor3f(0.12f, 0.09f, 0.06f);
        glPushMatrix();
        glTranslatef(0, 0.10f, -0.02f);
        glScalef(1.02f, 0.80f, 0.96f);
        sph(0.188f, 16, 8);
        glPopMatrix();

        // Tactical helmet
        glColor3f(0.20f, 0.21f, 0.17f);
        glPushMatrix();
        glTranslatef(0, 0.12f, -0.01f);
        glScalef(1.08f, 0.88f, 1.04f);
        sph(0.195f, 16, 10);
        glPopMatrix();

        // Helmet visor
        glColor3f(0.15f, 0.15f, 0.12f);
        glPushMatrix();
        glTranslatef(0, -0.01f, 0.15f);
        glScalef(1.6f, 0.25f, 0.8f);
        sph(0.095f, 10, 6);
        glPopMatrix();

        // Chin strap
        glColor3f(0.18f, 0.16f, 0.12f);
        glPushMatrix();
        glTranslatef(-0.10f, -0.14f, 0.10f);
        glScalef(0.3f, 0.3f, 1.0f);
        sph(0.050f, 8, 6);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.10f, -0.14f, 0.10f);
        glScalef(0.3f, 0.3f, 1.0f);
        sph(0.050f, 8, 6);
        glPopMatrix();

    glPopMatrix(); // head

    // ════════════════════════════════
    //   WEAPON (right side, moves with right arm)
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(0.30f, 1.60f, 0);
    glRotatef(-armSwing, 1, 0, 0);
    glTranslatef(0, -0.60f, 0.05f);

    glColor3f(0.13f, 0.10f, 0.07f);
    glPushMatrix(); glTranslatef(0, 0, -0.18f); glRotatef(90,1,0,0); cyl(0.030f, 0.22f, 8); glPopMatrix();
    glColor3f(0.18f, 0.18f, 0.18f);
    glPushMatrix(); glTranslatef(0, 0, -0.04f); glRotatef(90,1,0,0); cyl(0.028f, 0.30f, 8); glPopMatrix();
    glColor3f(0.22f, 0.22f, 0.22f);
    glPushMatrix(); glTranslatef(0, 0, 0.20f); glRotatef(90,1,0,0); cyl(0.014f, 0.34f, 8); glPopMatrix();
    glColor3f(0.12f, 0.09f, 0.07f);
    glPushMatrix(); glTranslatef(0, -0.10f, 0.02f); cyl(0.022f, 0.15f, 8); glPopMatrix();
    glColor3f(0.16f, 0.14f, 0.10f);
    glPushMatrix(); glTranslatef(0, -0.12f, -0.05f); cyl(0.020f, 0.13f, 8); glPopMatrix();

    glPopMatrix(); // weapon

    if (isSprinting) {
        glPopMatrix();
    }
}

// ══════════════════════════════════════════════════════════════════════════════
//   drawPlayer — renders the player WITH reflection, shadow, and the body
// ══════════════════════════════════════════════════════════════════════════════
void drawPlayer()
{
    // ─────────────────────────────────────────────────────────────────────────
    // 1. GROUND REFLECTION  (Reflection transform: Y → -Y about the ground plane Y=0)
    //    Matrix used:  Sy(-1) combined with translation to ground level.
    //    This mirrors the body below the ground, creating a reflection effect.
    // ─────────────────────────────────────────────────────────────────────────
    glPushMatrix();
        // TRANSLATION: move to player world position
        glTranslatef(playerX, playerY + bobY, playerZ);
        // ROTATION: face correct direction
        glRotatef(playerAngle, 0, 1, 0);
        // REFLECTION: flip Y axis (scale Y by -1 about ground plane Y=0)
        glScalef(1.0f, -1.0f, 1.0f);

        // Draw semi-transparent reflected body
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glColor4f(0.4f, 0.5f, 0.6f, 0.22f);   // cool tinted, very transparent
        // Tint all subsequent colors toward reflection color
        glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT);
            glDisable(GL_LIGHTING);
            // Draw reflection body with a uniform bluish tint
            glColor4f(0.35f, 0.45f, 0.55f, 0.22f);
            drawBody();
        glPopAttrib();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    glPopMatrix();

    // ─────────────────────────────────────────────────────────────────────────
    // 2. OBLIQUE SHADOW (Shear + projection transform onto ground plane Y=0)
    //    A shear matrix skews the body along XZ based on sun direction.
    //    This approximates a directional shadow by shearing Y→XZ.
    //    Shear matrix:
    //       [ 1   0   0   0 ]
    //       [ sx  0   sz  0 ]    <- Y contribution sheared into X and Z
    //       [ 0   0   1   0 ]
    //       [ 0   0   0   1 ]
    // ─────────────────────────────────────────────────────────────────────────
    {
        // Sun direction for shadow (varies slightly with time-of-day feel)
        float sunX = -0.6f, sunZ = -0.4f;  // oblique direction of light

        // Build shear-projection matrix (column-major for OpenGL)
        // Projects each point (x,y,z) → (x + sunX*y, 0, z + sunZ*y)
        float shadowMat[16] = {
            1.0f,  0.0f,  0.0f,  0.0f,   // col 0
            sunX,  0.0f,  sunZ,  0.0f,   // col 1 (Y row sheared into X,Z)
            0.0f,  0.0f,  1.0f,  0.0f,   // col 2
            0.0f,  0.001f, 0.0f, 1.0f    // col 3 (slight Y bias to avoid z-fighting)
        };

        glPushMatrix();
            glTranslatef(playerX, playerY, playerZ);
            glRotatef(playerAngle, 0, 1, 0);
            // Apply shear/shadow matrix
            glMultMatrixf(shadowMat);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);
            glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT);
                glDisable(GL_LIGHTING);
                glColor4f(0.05f, 0.05f, 0.05f, 0.45f);  // dark, semi-transparent shadow
                drawBody();
            glPopAttrib();
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        glPopMatrix();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // 3. ACTUAL PLAYER BODY
    //    Standard TRANSLATION to world position  +  ROTATION about Y-axis
    // ─────────────────────────────────────────────────────────────────────────
    glPushMatrix();
        // TRANSLATION — move player to world-space position (with breathing bob)
        glTranslatef(playerX, playerY + bobY, playerZ);
        // ROTATION — turn player to face movement direction
        glRotatef(playerAngle, 0, 1, 0);

        drawBody();
    glPopMatrix();
}

// CG Concept: Reflection helper
void drawPlayerBody()
{
    drawBody();
}

float getPlayerBobY()
{
    return bobY;
}

