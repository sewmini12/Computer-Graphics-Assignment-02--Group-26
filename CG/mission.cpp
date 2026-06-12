#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include "mission.h"
#include "globals.h"
#include "ui.h"

Mission missions[MAX_MISSIONS] = {
    { "First Steps",    "Reach the North Gate",           0.0f,  -60.0f, 4.0f, true,  false },
    { "City Center",    "Visit the City Hall plaza",       0.0f,   0.0f,  5.0f, false, false },
    { "East District",  "Explore the east apartments",    40.0f, -40.0f,  5.0f, false, false },
    { "Park Walk",      "Find the quiet west park",       -40.0f,  40.0f, 5.0f, false, false },
    { "South Bridge",   "Reach the south bridge end",      0.0f,  70.0f,  4.0f, false, false },
    { "Full Explorer",  "Return to the start point",       0.0f,   5.0f,  4.0f, false, false },
};

int currentMission  = 0;
int totalCompleted  = 0;

static float markerPulse = 0.0f;

// Bonus points awarded based on completion time
static int calcTimeBonus(float seconds)
{
    if (seconds < 120.0f)  return 500;
    if (seconds < 240.0f)  return 300;
    if (seconds < 360.0f)  return 150;
    return 50;
}

void initMissions()
{
    currentMission = 0;
    totalCompleted = 0;
    missions[0].active = true;
    for (int i = 1; i < MAX_MISSIONS; ++i)
        missions[i].active = false;

    gameElapsedTime = 0.0f;
    gameCompleted   = false;
    completionTime  = 0.0f;
}

void updateMissions()
{
    // Update game timer
    if (!gameCompleted && gameRunning) {
        gameElapsedTime += 0.016f;   // approximate; main loop refines this
    }

    markerPulse += 0.05f;
    if (markerPulse > 6.2832f) markerPulse -= 6.2832f;

    if (currentMission >= MAX_MISSIONS) return;

    Mission& m = missions[currentMission];
    if (!m.active || m.completed) return;

    float dx   = playerX - m.targetX;
    float dz   = playerZ - m.targetZ;
    float dist = sqrtf(dx * dx + dz * dz);

    if (dist < m.radius) {
        m.completed = true;
        m.active    = false;
        ++totalCompleted;
        score += 100;   // base per mission

        char msg[128];
        int  next = currentMission + 1;

        if (next < MAX_MISSIONS) {
            missions[next].active = true;
            currentMission = next;
            snprintf(msg, 128, "Mission Complete! +100 pts  |  Next: %s",
                     missions[next].title);
            setFlashMessage(msg, 3.5f);
        } else {
            // All missions done!
            currentMission = MAX_MISSIONS;
            gameCompleted  = true;
            completionTime = gameElapsedTime;

            // Time bonus
            int bonus = calcTimeBonus(completionTime);
            score += bonus;

            snprintf(msg, 128, "ALL MISSIONS COMPLETE!  Time Bonus: +%d pts!", bonus);
            setFlashMessage(msg, 99.0f);
        }
    }
}

// ─── Draw a pulsing ring + vertical beacon at mission target ──────────────────
void drawMissionMarkers()
{
    if (currentMission >= MAX_MISSIONS) return;
    Mission& m = missions[currentMission];
    if (!m.active || m.completed) return;

    float tx    = m.targetX;
    float tz    = m.targetZ;
    float pulse = 0.5f + 0.5f * sinf(markerPulse);

    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(tx, 0.05f, tz);

    // Ground ring (outer glow)
    glColor3f(1.0f, 0.9f * pulse, 0.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 48; ++i) {
        float a = i * 6.2832f / 48.0f;
        glVertex3f(m.radius * cosf(a), 0, m.radius * sinf(a));
    }
    glEnd();

    // Inner ring
    glColor3f(1.0f, 0.5f, 0.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 48; ++i) {
        float a = i * 6.2832f / 48.0f;
        glVertex3f(1.0f * cosf(a), 0, 1.0f * sinf(a));
    }
    glEnd();

    // Vertical beacon
    glColor4f(1.0f, 0.85f, 0.0f, 0.6f);
    glBegin(GL_LINES);
        glVertex3f(0, 0,     0);
        glVertex3f(0, 35.0f, 0);
    glEnd();

    // Star at beacon top
    glColor3f(1.0f, 1.0f, 0.0f);
    glTranslatef(0, 35.0f + pulse * 0.8f, 0);
    glScalef(0.6f, 0.6f, 0.6f);
    glutSolidOctahedron();

    glPopMatrix();
    glEnable(GL_LIGHTING);
}
