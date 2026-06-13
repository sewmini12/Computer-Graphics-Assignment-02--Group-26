#include <GL/freeglut.h>
#include <cmath>
#include "environment.h"
#include "collision.h"
#include "globals.h"
#include "player.h"

// ─── Building descriptor ──────────────────────────────────────────────────────
struct Building {
    float x, z;
    float w, d, h;
    float r, g, b;
};

static const Building buildings[] = {
    // City center block
    { -14,  -18,  6,  6, 14,  0.55f, 0.60f, 0.90f },
    {  14,  -18,  5,  5, 10,  0.70f, 0.45f, 0.40f },
    { -14,   18,  6,  6, 18,  0.45f, 0.70f, 0.55f },
    {  14,   18,  5,  5, 12,  0.80f, 0.70f, 0.30f },
    // Mid-ring
    { -28,    0,  4,  4,  8,  0.60f, 0.60f, 0.60f },
    {  28,    0,  4,  4,  9,  0.50f, 0.65f, 0.75f },
    {   0,  -30,  7,  4, 16,  0.65f, 0.50f, 0.80f },
    {   0,   30,  7,  4, 11,  0.40f, 0.55f, 0.70f },
    // Suburbs
    { -40,  -40,  5,  5,  6,  0.75f, 0.60f, 0.45f },
    {  40,  -40,  5,  5,  7,  0.55f, 0.75f, 0.55f },
    { -40,   40,  5,  5,  5,  0.70f, 0.50f, 0.60f },
    {  40,   40,  5,  5,  8,  0.60f, 0.70f, 0.50f },
    {  25,  -50,  4,  4,  5,  0.80f, 0.55f, 0.35f },
    { -25,   50,  4,  4,  6,  0.35f, 0.65f, 0.80f },
};

static const int NUM_BUILDINGS = sizeof(buildings) / sizeof(buildings[0]);

// ─── Tree descriptor ──────────────────────────────────────────────────────────
struct Tree { float x, z; float scale; };

static const Tree trees[] = {
    {  10, -12, 1.0f }, { -10, -12, 0.9f },
    {  10,  15, 1.1f }, { -10,  20, 1.0f },
    {  20,  -5, 0.8f }, { -20,  -8, 1.2f },
    {  35,  15, 1.0f }, { -35,  20, 0.9f },
    {  45,  -5, 1.1f }, { -45,  10, 1.0f },
    { -55, -55, 1.3f }, {  55, -55, 1.0f },
    { -55,  55, 1.0f }, {  55,  55, 1.2f },
    {  70,   0, 1.0f }, { -70,   0, 0.9f },
    {   0,  70, 1.1f }, {   0, -70, 1.0f },
};
static const int NUM_TREES = sizeof(trees) / sizeof(trees[0]);

// ─── Street lamp positions ────────────────────────────────────────────────────
struct Lamp { float x, z; };
static const Lamp lamps[] = {
    { 6.5f, -25.0f }, { 6.5f,  0.0f }, { 6.5f,  25.0f },
    {-6.5f, -25.0f }, {-6.5f,  0.0f }, {-6.5f,  25.0f },
    { 25.0f, 6.5f  }, {  0.0f, 6.5f }, { -25.0f, 6.5f },
    { 25.0f,-6.5f  }, {  0.0f,-6.5f }, { -25.0f,-6.5f },
};
static const int NUM_LAMPS = sizeof(lamps) / sizeof(lamps[0]);

// ─── initEnvironment ──────────────────────────────────────────────────────────
void initEnvironment()
{
    for (int i = 0; i < NUM_BUILDINGS; ++i) {
        addCollider(buildings[i].x, buildings[i].z,
                    buildings[i].w * 0.5f + 0.3f,
                    buildings[i].d * 0.5f + 0.3f);
    }
    // Tree trunks
    for (int i = 0; i < NUM_TREES; ++i) {
        addCollider(trees[i].x, trees[i].z, 0.5f, 0.5f);
    }
}

// ─── drawGround ──────────────────────────────────────────────────────────────
void drawGround()
{
    // Draw checkerboard green grass pattern
    float size = 10.0f;
    for (float x = -100.0f; x < 100.0f; x += size) {
        for (float z = -100.0f; z < 100.0f; z += size) {
            // Alternate colors slightly
            if (((int)(x/size) + (int)(z/size)) % 2 == 0) {
                glColor3f(0.28f, 0.60f, 0.22f);
            } else {
                glColor3f(0.32f, 0.66f, 0.26f);
            }
            glBegin(GL_QUADS);
                glNormal3f(0, 1, 0);
                glVertex3f(x,        0.0f, z);
                glVertex3f(x + size, 0.0f, z);
                glVertex3f(x + size, 0.0f, z + size);
                glVertex3f(x,        0.0f, z + size);
            glEnd();
        }
    }

    // Sidewalk strips along roads
    glColor3f(0.72f, 0.72f, 0.68f);
    glBegin(GL_QUADS);
        glNormal3f(0,1,0);
        // Along Z-road
        glVertex3f( -7.5f, 0.005f, -100);
        glVertex3f(  7.5f, 0.005f, -100);
        glVertex3f(  7.5f, 0.005f,  100);
        glVertex3f( -7.5f, 0.005f,  100);
    glEnd();
    glBegin(GL_QUADS);
        glNormal3f(0,1,0);
        // Along X-road
        glVertex3f(-100, 0.005f, -7.5f);
        glVertex3f( 100, 0.005f, -7.5f);
        glVertex3f( 100, 0.005f,  7.5f);
        glVertex3f(-100, 0.005f,  7.5f);
    glEnd();
}

// ─── drawRoads ───────────────────────────────────────────────────────────────
void drawRoads()
{
    glColor3f(0.18f, 0.18f, 0.20f);

    // North-South road
    glBegin(GL_QUADS);
        glNormal3f(0,1,0);
        glVertex3f(-5, 0.01f, -100);
        glVertex3f( 5, 0.01f, -100);
        glVertex3f( 5, 0.01f,  100);
        glVertex3f(-5, 0.01f,  100);
    glEnd();

    // East-West road
    glBegin(GL_QUADS);
        glNormal3f(0,1,0);
        glVertex3f(-100, 0.01f, -5);
        glVertex3f( 100, 0.01f, -5);
        glVertex3f( 100, 0.01f,  5);
        glVertex3f(-100, 0.01f,  5);
    glEnd();

    // Road side lines (white lanes)
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
        glNormal3f(0,1,0);
        glVertex3f(-4.8f, 0.015f, -100);
        glVertex3f(-4.6f, 0.015f, -100);
        glVertex3f(-4.6f, 0.015f, 100);
        glVertex3f(-4.8f, 0.015f, 100);

        glVertex3f(4.6f, 0.015f, -100);
        glVertex3f(4.8f, 0.015f, -100);
        glVertex3f(4.8f, 0.015f, 100);
        glVertex3f(4.6f, 0.015f, 100);
    glEnd();

    // ── Centre line dashes ────────────────────────────────────────────────────
    glColor3f(0.95f, 0.95f, 0.1f);
    for (int i = -9; i <= 9; ++i) {
        float z0 = i * 10.5f;
        float z1 = z0 + 5.5f;
        glBegin(GL_QUADS);
            glNormal3f(0,1,0);
            glVertex3f(-0.15f, 0.02f, z0);
            glVertex3f( 0.15f, 0.02f, z0);
            glVertex3f( 0.15f, 0.02f, z1);
            glVertex3f(-0.15f, 0.02f, z1);
        glEnd();
        glBegin(GL_QUADS);
            glNormal3f(0,1,0);
            glVertex3f(z0, 0.02f, -0.15f);
            glVertex3f(z1, 0.02f, -0.15f);
            glVertex3f(z1, 0.02f,  0.15f);
            glVertex3f(z0, 0.02f,  0.15f);
        glEnd();
    }
}

// ─── Helper: draw one building ───────────────────────────────────────────────
static void drawOneBuilding(const Building& b)
{
    glPushMatrix();
    glTranslatef(b.x, b.h * 0.5f, b.z);
    glScalef(b.w, b.h, b.d);

    // CG Concept: Texture-like procedural checkerboard shading
    glBegin(GL_QUADS);
    // 1. Front face (+Z)
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            bool dark = (i + j) % 2 == 0;
            if (dark) {
                glColor3f(b.r * 0.75f, b.g * 0.75f, b.b * 0.75f);
            } else {
                glColor3f(b.r, b.g, b.b);
            }
            float x0 = -0.5f + i * 0.25f;
            float x1 = x0 + 0.25f;
            float y0 = -0.5f + j * 0.25f;
            float y1 = y0 + 0.25f;
            glVertex3f(x0, y0, 0.5f);
            glVertex3f(x1, y0, 0.5f);
            glVertex3f(x1, y1, 0.5f);
            glVertex3f(x0, y1, 0.5f);
        }
    }

    // 2. Back face (-Z)
    glNormal3f(0.0f, 0.0f, -1.0f);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            bool dark = (i + j) % 2 == 0;
            if (dark) {
                glColor3f(b.r * 0.75f, b.g * 0.75f, b.b * 0.75f);
            } else {
                glColor3f(b.r, b.g, b.b);
            }
            float x0 = 0.5f - i * 0.25f;
            float x1 = x0 - 0.25f;
            float y0 = -0.5f + j * 0.25f;
            float y1 = y0 + 0.25f;
            glVertex3f(x0, y0, -0.5f);
            glVertex3f(x1, y0, -0.5f);
            glVertex3f(x1, y1, -0.5f);
            glVertex3f(x0, y1, -0.5f);
        }
    }

    // 3. Left face (-X)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            bool dark = (i + j) % 2 == 0;
            if (dark) {
                glColor3f(b.r * 0.75f, b.g * 0.75f, b.b * 0.75f);
            } else {
                glColor3f(b.r, b.g, b.b);
            }
            float z0 = -0.5f + i * 0.25f;
            float z1 = z0 + 0.25f;
            float y0 = -0.5f + j * 0.25f;
            float y1 = y0 + 0.25f;
            glVertex3f(-0.5f, y0, z0);
            glVertex3f(-0.5f, y0, z1);
            glVertex3f(-0.5f, y1, z1);
            glVertex3f(-0.5f, y1, z0);
        }
    }

    // 4. Right face (+X)
    glNormal3f(1.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            bool dark = (i + j) % 2 == 0;
            if (dark) {
                glColor3f(b.r * 0.75f, b.g * 0.75f, b.b * 0.75f);
            } else {
                glColor3f(b.r, b.g, b.b);
            }
            float z0 = 0.5f - i * 0.25f;
            float z1 = z0 - 0.25f;
            float y0 = -0.5f + j * 0.25f;
            float y1 = y0 + 0.25f;
            glVertex3f(0.5f, y0, z0);
            glVertex3f(0.5f, y0, z1);
            glVertex3f(0.5f, y1, z1);
            glVertex3f(0.5f, y1, z0);
        }
    }

    // 5. Top face (+Y)
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            bool dark = (i + j) % 2 == 0;
            if (dark) {
                glColor3f(b.r * 0.75f, b.g * 0.75f, b.b * 0.75f);
            } else {
                glColor3f(b.r, b.g, b.b);
            }
            float x0 = -0.5f + i * 0.25f;
            float x1 = x0 + 0.25f;
            float z0 = -0.5f + j * 0.25f;
            float z1 = z0 + 0.25f;
            glVertex3f(x0, 0.5f, z0);
            glVertex3f(x1, 0.5f, z0);
            glVertex3f(x1, 0.5f, z1);
            glVertex3f(x0, 0.5f, z1);
        }
    }

    // 6. Bottom face (-Y)
    glNormal3f(0.0f, -1.0f, 0.0f);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            bool dark = (i + j) % 2 == 0;
            if (dark) {
                glColor3f(b.r * 0.75f, b.g * 0.75f, b.b * 0.75f);
            } else {
                glColor3f(b.r, b.g, b.b);
            }
            float x0 = -0.5f + i * 0.25f;
            float x1 = x0 + 0.25f;
            float z0 = 0.5f - j * 0.25f;
            float z1 = z0 - 0.25f;
            glVertex3f(x0, -0.5f, z0);
            glVertex3f(x1, -0.5f, z0);
            glVertex3f(x1, -0.5f, z1);
            glVertex3f(x0, -0.5f, z1);
        }
    }
    glEnd();
    glPopMatrix();

    // Draw Glowing Windows on all 4 sides of the building
    // Let's determine how many floors and windows per floor
    int floors = (int)(b.h / 1.8f);
    int cols = (int)(b.w / 1.5f);

    glDisable(GL_LIGHTING);
    // Yellowish glowing color for windows
    glColor3f(0.95f, 0.95f, 0.6f);

    for (int f = 0; f < floors; ++f) {
        float y = 0.8f + f * 1.8f;
        for (int c = 0; c < cols; ++c) {
            float xOffset = -b.w * 0.5f + 0.6f + c * 1.5f;
            if (xOffset > b.w * 0.5f - 0.4f) continue;

            // Front face windows (z = b.z + b.d*0.5 + epsilon)
            glBegin(GL_QUADS);
                glVertex3f(b.x + xOffset - 0.2f, y - 0.3f, b.z + b.d * 0.5f + 0.02f);
                glVertex3f(b.x + xOffset + 0.2f, y - 0.3f, b.z + b.d * 0.5f + 0.02f);
                glVertex3f(b.x + xOffset + 0.2f, y + 0.3f, b.z + b.d * 0.5f + 0.02f);
                glVertex3f(b.x + xOffset - 0.2f, y + 0.3f, b.z + b.d * 0.5f + 0.02f);
            glEnd();

            // Back face windows
            glBegin(GL_QUADS);
                glVertex3f(b.x + xOffset - 0.2f, y - 0.3f, b.z - b.d * 0.5f - 0.02f);
                glVertex3f(b.x + xOffset + 0.2f, y - 0.3f, b.z - b.d * 0.5f - 0.02f);
                glVertex3f(b.x + xOffset + 0.2f, y + 0.3f, b.z - b.d * 0.5f - 0.02f);
                glVertex3f(b.x + xOffset - 0.2f, y + 0.3f, b.z - b.d * 0.5f - 0.02f);
            glEnd();
        }
    }

    int sideCols = (int)(b.d / 1.5f);
    for (int f = 0; f < floors; ++f) {
        float y = 0.8f + f * 1.8f;
        for (int c = 0; c < sideCols; ++c) {
            float zOffset = -b.d * 0.5f + 0.6f + c * 1.5f;
            if (zOffset > b.d * 0.5f - 0.4f) continue;

            // Left face windows (x = b.x - b.w*0.5 - epsilon)
            glBegin(GL_QUADS);
                glVertex3f(b.x - b.w * 0.5f - 0.02f, y - 0.3f, b.z + zOffset - 0.2f);
                glVertex3f(b.x - b.w * 0.5f - 0.02f, y - 0.3f, b.z + zOffset + 0.2f);
                glVertex3f(b.x - b.w * 0.5f - 0.02f, y + 0.3f, b.z + zOffset + 0.2f);
                glVertex3f(b.x - b.w * 0.5f - 0.02f, y + 0.3f, b.z + zOffset - 0.2f);
            glEnd();

            // Right face windows
            glBegin(GL_QUADS);
                glVertex3f(b.x + b.w * 0.5f + 0.02f, y - 0.3f, b.z + zOffset - 0.2f);
                glVertex3f(b.x + b.w * 0.5f + 0.02f, y - 0.3f, b.z + zOffset + 0.2f);
                glVertex3f(b.x + b.w * 0.5f + 0.02f, y + 0.3f, b.z + zOffset + 0.2f);
                glVertex3f(b.x + b.w * 0.5f + 0.02f, y + 0.3f, b.z + zOffset - 0.2f);
            glEnd();
        }
    }
    glEnable(GL_LIGHTING);

    // Roof edge trim
    glPushMatrix();
    glTranslatef(b.x, b.h + 0.15f, b.z);
    glScalef(b.w + 0.2f, 0.3f, b.d + 0.2f);
    glColor3f(b.r * 0.7f, b.g * 0.7f, b.b * 0.7f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void drawBuildings()
{
    for (int i = 0; i < NUM_BUILDINGS; ++i)
        drawOneBuilding(buildings[i]);
}

// ─── Helper: draw one tree ───────────────────────────────────────────────────
static void drawOneTree(const Tree& t)
{
    glPushMatrix();
    glTranslatef(t.x, 0, t.z);
    glScalef(t.scale, t.scale, t.scale);

    // Trunk
    glColor3f(0.45f, 0.28f, 0.07f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q, 0.18f, 0.14f, 2.2f, 12, 4);
    gluDeleteQuadric(q);
    glPopMatrix();

    // Foliage – 3 layered spheres
    glColor3f(0.10f, 0.62f, 0.15f);
    glPushMatrix();
    glTranslatef(0, 2.5f, 0);
    glutSolidSphere(1.1f, 14, 14);
    glPopMatrix();

    glColor3f(0.08f, 0.52f, 0.12f);
    glPushMatrix();
    glTranslatef(0.2f, 3.3f, -0.1f);
    glutSolidSphere(0.85f, 12, 12);
    glPopMatrix();

    glColor3f(0.15f, 0.70f, 0.20f);
    glPushMatrix();
    glTranslatef(-0.1f, 3.8f, 0.15f);
    glutSolidSphere(0.65f, 10, 10);
    glPopMatrix();

    glPopMatrix();
}

void drawTrees()
{
    for (int i = 0; i < NUM_TREES; ++i)
        drawOneTree(trees[i]);
}

// ─── Street Lamps ────────────────────────────────────────────────────────────
static void drawOneLamp(float x, float z)
{
    glPushMatrix();
    glTranslatef(x, 0, z);

    // Pole
    glColor3f(0.45f, 0.45f, 0.45f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    GLUquadric* q = gluNewQuadric();
    gluCylinder(q, 0.1f, 0.08f, 5.5f, 8, 2);
    gluDeleteQuadric(q);
    glPopMatrix();

    // Lamp head
    glColor3f(0.9f, 0.85f, 0.5f);
    glPushMatrix();
    glTranslatef(0.4f, 5.5f, 0);
    glutSolidSphere(0.25f, 10, 10);
    glPopMatrix();

    // Arm
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.2f, 5.4f, 0);
    glScalef(0.5f, 0.08f, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}

void drawStreetLamps()
{
    for (int i = 0; i < NUM_LAMPS; ++i)
        drawOneLamp(lamps[i].x, lamps[i].z);
}

// ─── Sky Dome (simple large hemisphere for atmosphere) ──────────────────────
void drawSkyDome()
{
    // Rendered without lighting so the clear colour provides sky feel
    // A coloured dome sphere drawn behind everything
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0, -10, 0);
    glColor3f(0.5f, 0.75f, 1.0f);
    glutSolidSphere(150.0f, 24, 12);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

// ─── CG Concept: Reflection ──────────────────────────────────────────────────
void drawReflectivePool()
{
    // Draw the concrete border around the pool
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(-49.0f, 0.008f, 31.0f);
        glVertex3f(-31.0f, 0.008f, 31.0f);
        glVertex3f(-31.0f, 0.008f, 49.0f);
        glVertex3f(-49.0f, 0.008f, 49.0f);
    glEnd();

    if (showReflection) {
        // Clear stencil buffer
        glClear(GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);

        // Step 1: write pool area to stencil buffer
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        // Disable color and depth writes
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        // Draw the pool area quad to stencil buffer
        glBegin(GL_QUADS);
            glVertex3f(-48.0f, 0.01f, 32.0f);
            glVertex3f(-32.0f, 0.01f, 32.0f);
            glVertex3f(-32.0f, 0.01f, 48.0f);
            glVertex3f(-48.0f, 0.01f, 48.0f);
        glEnd();

        // Re-enable color and depth writes
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        // Step 2: draw reflected scene only where stencil=1
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        glPushMatrix();
            // Reflection plane is y=0.01f, so mirror about it:
            // y' = -y + 0.02
            glTranslatef(0.0f, 0.02f, 0.0f);
            glScalef(1.0f, -1.0f, 1.0f);
            glFrontFace(GL_CW); // Flip normals winding

            // Draw player body reflected under the pool
            glPushMatrix();
                glTranslatef(playerX, playerY + getPlayerBobY(), playerZ);
                glRotatef(playerAngle, 0, 1, 0);
                drawPlayerBody();
            glPopMatrix();

            glFrontFace(GL_CCW); // Restore normal winding
        glPopMatrix();

        glDisable(GL_STENCIL_TEST);
    }

    // Step 3: draw semi-transparent blue water quad on top
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glColor4f(0.1f, 0.4f, 0.8f, 0.45f); // alpha ~0.45
    glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(-48.0f, 0.01f, 32.0f);
        glVertex3f(-32.0f, 0.01f, 32.0f);
        glVertex3f(-32.0f, 0.01f, 48.0f);
        glVertex3f(-48.0f, 0.01f, 48.0f);
    glEnd();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

// ─── drawEnvironment ─────────────────────────────────────────────────────────
void drawEnvironment()
{
    drawGround();
    drawRoads();
    drawBuildings();
    drawTrees();
    drawStreetLamps();
    drawReflectivePool();
}
