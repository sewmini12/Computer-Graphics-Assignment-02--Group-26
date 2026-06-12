#include <GL/freeglut.h>
#include <GL/glu.h>
#include <cmath>
#include "player.h"
#include "globals.h"
#include "collision.h"

// ─── Animation State ──────────────────────────────────────────────────────────
static float legSwing    = 0.0f;
static float legDir      = 1.0f;
static bool  isMoving    = false;
static float runSpeed    = 0.0f;
static float bobY        = 0.0f;
static float bobPhase    = 0.0f;
static float breathPhase = 0.0f;
static float headTurn    = 0.0f;
static float armSwing    = 0.0f;

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

// ─── Helpers ─────────────────────────────────────────────────────────────────

// Vertical cylinder centered at origin (height along +Y)
static void cyl(float radius, float height, int slices = 14)
{
    glTranslatef(0, -height * 0.5f, 0);
    glRotatef(-90, 1, 0, 0);   // align with Y
    gluCylinder(qObj, radius, radius, height, slices, 1);
    gluDisk(qObj, 0, radius, slices, 1);   // bottom cap
    glTranslatef(0, 0, height);
    gluDisk(qObj, 0, radius, slices, 1);   // top cap
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

// Flat disk (shoe sole)
static void disk(float r, int slices = 14)
{
    gluDisk(qObj, 0, r, slices, 1);
}

// ─── Animation update ────────────────────────────────────────────────────────
void updatePlayer(float dt)
{
    float targetSpeed = isMoving ? 1.0f : 0.0f;
    runSpeed += (targetSpeed - runSpeed) * 9.0f * dt;

    breathPhase += dt * 1.3f;

    if (runSpeed > 0.01f) {
        bobPhase  += dt * 10.5f * runSpeed;
        legSwing  += legDir * 280.0f * dt * runSpeed;
        if (legSwing >  44.0f) legDir = -1.0f;
        if (legSwing < -44.0f) legDir =  1.0f;
        armSwing   = -legSwing * 0.75f;
        bobY       = sinf(bobPhase) * 0.07f * runSpeed;
        headTurn   = legSwing * 0.06f;
    } else {
        legSwing  *= 0.85f;
        armSwing   = -legSwing * 0.75f;
        bobPhase   = 0.0f;
        bobY       = sinf(breathPhase) * 0.012f;
        headTurn  *= 0.88f;
    }

    // Gravity & Jump
    playerVelY -= 9.8f * dt;
    playerY += playerVelY * dt;
    
    // Will be overridden by getGroundHeight later if needed, but for now clamp to 0
    float groundY = getGroundHeight(playerX, playerZ);
    if (playerY <= groundY) {
        playerY = groundY;
        playerVelY = 0.0f;
        isGrounded = true;
    } else {
        isGrounded = false;
    }

    if (isMoving && isSprinting && stamina > 0) {
        stamina -= 30.0f * dt; 
    } else {
        stamina = fmin(100.f, stamina + 10.0f * dt);
    }

    isMoving = false;
}

// ─── Player movement ─────────────────────────────────────────────────────────
void movePlayer(unsigned char key)
{
    float yawRad   = camYaw * 3.14159265f / 180.0f;
    float forwardX = -sinf(yawRad);
    float forwardZ = -cosf(yawRad);
    float rightX   =  cosf(yawRad);
    float rightZ   = -sinf(yawRad);

    float dx = 0.0f, dz = 0.0f;

    isSprinting = (glutGetModifiers() & GLUT_ACTIVE_SHIFT);
    float speed = playerSpeed;
    if (isSprinting && stamina > 0) {
        speed *= 1.8f;
    }

    switch (key) {
        case 'w': case 'W':
            dx = forwardX * speed; dz = forwardZ * speed;
            playerAngle = -camYaw; break;
        case 's': case 'S':
            dx = -forwardX * speed; dz = -forwardZ * speed;
            playerAngle = -camYaw + 180.0f; break;
        case 'a': case 'A':
            dx = -rightX * speed; dz = -rightZ * speed;
            playerAngle = -camYaw + 90.0f; break;
        case 'd': case 'D':
            dx = rightX * speed; dz = rightZ * speed;
            playerAngle = -camYaw - 90.0f; break;
        case ' ':
            if (isGrounded) {
                playerVelY = 5.0f;
                isGrounded = false;
            }
            return;
        default: return;
    }

    float nx = playerX + dx, nz = playerZ + dz;
    bool movedX = false, movedZ = false;

    if (!checkCollision(nx, playerZ)) {
        playerX = nx;
        movedX = true;
    }
    if (!checkCollision(playerX, nz)) {
        playerZ = nz;
        movedZ = true;
    }

    // Improve: add a small push-out vector when both fail
    if (!movedX && !movedZ && (dx != 0.0f || dz != 0.0f)) {
        // Try a smaller step just to slide slightly away
        if (!checkCollision(playerX - dx * 0.5f, playerZ)) playerX -= dx * 0.5f;
        if (!checkCollision(playerX, playerZ - dz * 0.5f)) playerZ -= dz * 0.5f;
    }

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

// ─── DRAW PLAYER — Realistic human proportions with round limbs ───────────────
void drawPlayer()
{
    glPushMatrix();
    glTranslatef(playerX, playerY + bobY, playerZ);
    glRotatef(playerAngle, 0, 1, 0);

    // ── Skin color ──────────────────────────────────────────────────────
    const float SKIN_R = 0.83f, SKIN_G = 0.66f, SKIN_B = 0.50f;
    // Clothing colors
    const float SHIRT_R = 0.34f, SHIRT_G = 0.32f, SHIRT_B = 0.19f;  // olive
    const float PANT_R  = 0.28f, PANT_G  = 0.26f, PANT_B  = 0.16f;  // dark olive
    const float BOOT_R  = 0.16f, BOOT_G  = 0.11f, BOOT_B  = 0.07f;  // dark brown
    const float ARMOR_R = 0.18f, ARMOR_G = 0.18f, ARMOR_B = 0.15f;  // dark plate

    // ════════════════════════════════
    //   FEET / BOOTS
    // ════════════════════════════════
    // Left boot
    glPushMatrix();
    glTranslatef(-0.13f, 0.0f, 0);
    glRotatef(legSwing * 0.8f, 1, 0, 0);
    // Ankle
    glColor3f(BOOT_R, BOOT_G, BOOT_B);
    glPushMatrix(); glTranslatef(0, 0.10f, 0); cyl(0.090f, 0.20f); glPopMatrix();
    // Sole + toe (stretched sphere)
    glPushMatrix();
    glTranslatef(0, 0.04f, 0.05f);
    glScalef(1.0f, 0.35f, 1.6f);
    sph(0.090f, 12, 8);
    glPopMatrix();
    glPopMatrix();

    // Right boot
    glPushMatrix();
    glTranslatef(0.13f, 0.0f, 0);
    glRotatef(-legSwing * 0.8f, 1, 0, 0);
    glColor3f(BOOT_R, BOOT_G, BOOT_B);
    glPushMatrix(); glTranslatef(0, 0.10f, 0); cyl(0.090f, 0.20f); glPopMatrix();
    glPushMatrix();
    glTranslatef(0, 0.04f, 0.05f);
    glScalef(1.0f, 0.35f, 1.6f);
    sph(0.090f, 12, 8);
    glPopMatrix();
    glPopMatrix();

    // ════════════════════════════════
    //   LEGS  (upper + lower with knee sphere)
    // ════════════════════════════════
    // LEFT LEG
    glPushMatrix();
    glTranslatef(-0.13f, 0.80f, 0);
    glRotatef(legSwing, 1, 0, 0);

        // Upper leg
        glColor3f(PANT_R, PANT_G, PANT_B);
        glPushMatrix(); glTranslatef(0, -0.22f, 0); cylTaper(0.100f, 0.085f, 0.44f); glPopMatrix();
        // Knee sphere
        glPushMatrix(); glTranslatef(0, -0.46f, 0);
            glColor3f(PANT_R * 0.8f, PANT_G * 0.8f, PANT_B * 0.8f);
            sph(0.092f, 12, 8);
        glPopMatrix();
        // Lower leg
        glPushMatrix(); glTranslatef(0, -0.68f, 0);
            glColor3f(PANT_R, PANT_G, PANT_B);
            cylTaper(0.085f, 0.075f, 0.38f);
        glPopMatrix();

    glPopMatrix();  // left leg

    // RIGHT LEG
    glPushMatrix();
    glTranslatef(0.13f, 0.80f, 0);
    glRotatef(-legSwing, 1, 0, 0);

        glColor3f(PANT_R, PANT_G, PANT_B);
        glPushMatrix(); glTranslatef(0, -0.22f, 0); cylTaper(0.100f, 0.085f, 0.44f); glPopMatrix();
        glPushMatrix(); glTranslatef(0, -0.46f, 0);
            glColor3f(PANT_R * 0.8f, PANT_G * 0.8f, PANT_B * 0.8f);
            sph(0.092f, 12, 8);
        glPopMatrix();
        glPushMatrix(); glTranslatef(0, -0.68f, 0);
            glColor3f(PANT_R, PANT_G, PANT_B);
            cylTaper(0.085f, 0.075f, 0.38f);
        glPopMatrix();

    glPopMatrix();  // right leg

    // ════════════════════════════════
    //   PELVIS / HIP sphere
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(0, 0.82f, 0);
    glColor3f(PANT_R * 1.1f, PANT_G * 1.1f, PANT_B * 1.1f);
    glScalef(1.5f, 0.8f, 0.9f);
    sph(0.16f, 14, 10);
    glPopMatrix();

    // ════════════════════════════════
    //   TORSO  (tapered cylinder — wider at shoulders)
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(0, 1.30f, 0);
    glColor3f(SHIRT_R, SHIRT_G, SHIRT_B);
    cylTaper(0.185f, 0.230f, 0.72f, 16);  // hips-to-shoulders taper
    glPopMatrix();

    // ── Chest armor plate ──
    glPushMatrix();
    glTranslatef(0, 1.55f, 0.15f);
    glColor3f(ARMOR_R, ARMOR_G, ARMOR_B);
    glScalef(1.8f, 1.3f, 0.5f);
    sph(0.17f, 14, 10);
    glPopMatrix();

    // ── Belly (slightly rounded) ──
    glPushMatrix();
    glTranslatef(0, 1.12f, 0.04f);
    glColor3f(SHIRT_R * 0.9f, SHIRT_G * 0.9f, SHIRT_B * 0.9f);
    glScalef(1.4f, 0.7f, 0.6f);
    sph(0.16f, 12, 8);
    glPopMatrix();

    // ════════════════════════════════
    //   SHOULDERS (spheres)
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
    //   ARMS (upper + elbow + forearm + hand)
    // ════════════════════════════════
    // LEFT ARM
    glPushMatrix();
    glTranslatef(-0.30f, 1.70f, 0);
    glRotatef(armSwing, 1, 0, 0);   // opposite swing to right leg

        // Upper arm
        glColor3f(SHIRT_R, SHIRT_G, SHIRT_B);
        glPushMatrix(); glTranslatef(0, -0.17f, 0); cylTaper(0.075f, 0.065f, 0.34f); glPopMatrix();
        // Elbow sphere
        glPushMatrix(); glTranslatef(0, -0.36f, 0);
            glColor3f(SKIN_R * 0.9f, SKIN_G * 0.9f, SKIN_B * 0.9f);
            sph(0.068f, 10, 8);
        glPopMatrix();
        // Forearm (skin visible — rolled sleeve)
        glPushMatrix(); glTranslatef(0, -0.55f, 0);
            glColor3f(SKIN_R, SKIN_G, SKIN_B);
            cylTaper(0.065f, 0.055f, 0.32f);
        glPopMatrix();
        // Wrist sphere
        glPushMatrix(); glTranslatef(0, -0.73f, 0);
            glColor3f(SKIN_R, SKIN_G, SKIN_B);
            sph(0.058f, 10, 8);
        glPopMatrix();
        // Hand
        glPushMatrix(); glTranslatef(0, -0.84f, 0.02f);
            glColor3f(SKIN_R, SKIN_G, SKIN_B);
            glScalef(0.8f, 0.6f, 1.2f);
            sph(0.072f, 10, 8);
        glPopMatrix();

    glPopMatrix();  // left arm

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
        // Hand with weapon (right)
        glPushMatrix(); glTranslatef(0, -0.84f, 0.02f);
            glColor3f(SKIN_R, SKIN_G, SKIN_B);
            glScalef(0.8f, 0.6f, 1.2f);
            sph(0.072f, 10, 8);
        glPopMatrix();

    glPopMatrix();  // right arm

    // ════════════════════════════════
    //   NECK
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(0, 1.82f, 0);
    glColor3f(SKIN_R, SKIN_G, SKIN_B);
    cylTaper(0.072f, 0.065f, 0.16f, 12);
    glPopMatrix();

    // ════════════════════════════════
    //   HEAD  (oblate sphere — human skull shape)
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(0, 2.06f, 0);
    glRotatef(headTurn, 0, 1, 0);

        // Skull
        glColor3f(SKIN_R, SKIN_G, SKIN_B);
        glPushMatrix();
        glScalef(1.0f, 1.15f, 0.95f);   // slightly taller than wide
        sph(0.185f, 16, 12);
        glPopMatrix();

        // Jaw / chin (lower face)
        glColor3f(SKIN_R * 0.97f, SKIN_G * 0.97f, SKIN_B * 0.97f);
        glPushMatrix();
        glTranslatef(0, -0.10f, 0.04f);
        glScalef(0.9f, 0.55f, 0.85f);
        sph(0.165f, 14, 10);
        glPopMatrix();

        // Eyes (left)
        glPushMatrix();
        glTranslatef(-0.065f, 0.03f, 0.16f);
        glColor3f(0.05f, 0.05f, 0.05f);
        sph(0.022f, 8, 6);
        // Eye white
        glColor3f(0.95f, 0.95f, 0.95f);
        glTranslatef(0, 0, -0.004f);
        sph(0.026f, 8, 6);
        glPopMatrix();

        // Eyes (right)
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

        // Mouth / lips
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

        // Hair cap (dark cap/short hair)
        glColor3f(0.12f, 0.09f, 0.06f);
        glPushMatrix();
        glTranslatef(0, 0.10f, -0.02f);
        glScalef(1.02f, 0.80f, 0.96f);
        sph(0.188f, 16, 8);
        glPopMatrix();

        // ── PUBG Tactical helmet (over hair) ──────────────────────────────
        glColor3f(0.20f, 0.21f, 0.17f);
        glPushMatrix();
        glTranslatef(0, 0.12f, -0.01f);
        glScalef(1.08f, 0.88f, 1.04f);
        sph(0.195f, 16, 10);
        glPopMatrix();

        // Helmet brim/visor
        glColor3f(0.15f, 0.15f, 0.12f);
        glPushMatrix();
        glTranslatef(0, -0.01f, 0.15f);
        glScalef(1.6f, 0.25f, 0.8f);
        sph(0.095f, 10, 6);
        glPopMatrix();

        // Helmet chin strap
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

    glPopMatrix();  // end head

    // ════════════════════════════════
    //   WEAPON (right side)
    // ════════════════════════════════
    glPushMatrix();
    glTranslatef(0.30f, 1.60f, 0);
    glRotatef(-armSwing, 1, 0, 0);
    glTranslatef(0, -0.60f, 0.05f);

    // Stock
    glColor3f(0.13f, 0.10f, 0.07f);
    glPushMatrix(); glTranslatef(0, 0, -0.18f); glRotatef(90,1,0,0); cyl(0.030f, 0.22f, 8); glPopMatrix();
    // Body / receiver
    glColor3f(0.18f, 0.18f, 0.18f);
    glPushMatrix(); glTranslatef(0, 0, -0.04f); glRotatef(90,1,0,0); cyl(0.028f, 0.30f, 8); glPopMatrix();
    // Barrel
    glColor3f(0.22f, 0.22f, 0.22f);
    glPushMatrix(); glTranslatef(0, 0, 0.20f); glRotatef(90,1,0,0); cyl(0.014f, 0.34f, 8); glPopMatrix();
    // Grip
    glColor3f(0.12f, 0.09f, 0.07f);
    glPushMatrix(); glTranslatef(0, -0.10f, 0.02f); cyl(0.022f, 0.15f, 8); glPopMatrix();
    // Magazine
    glColor3f(0.16f, 0.14f, 0.10f);
    glPushMatrix(); glTranslatef(0, -0.12f, -0.05f); cyl(0.020f, 0.13f, 8); glPopMatrix();

    glPopMatrix();  // weapon

    glPopMatrix();  // entire player
}
