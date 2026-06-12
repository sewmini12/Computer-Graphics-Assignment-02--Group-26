#include <GL/freeglut.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "ui.h"
#include "globals.h"
#include "mission.h"

// ─── Flash message ─────────────────────────────────────────────────────────────
static char  flashMsg[128] = "";
static float flashTimer    = 0.0f;
static float flashDuration = 0.0f;

void setFlashMessage(const char* msg, float durationSec)
{
    strncpy(flashMsg, msg, 127);
    flashMsg[127] = '\0';
    flashTimer    = 0.0f;
    flashDuration = durationSec;
}

void updateFlash(float dt)
{
    if (flashTimer < flashDuration)
        flashTimer += dt;
}

// ─── 2-D overlay helpers ──────────────────────────────────────────────────────
static void beginOverlay()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void endOverlay()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// ─── Draw filled rectangle ────────────────────────────────────────────────────
static void drawRect(float x, float y, float w, float h,
                     float r, float g, float b, float a)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
        glVertex2f(x,     y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x,     y + h);
    glEnd();
    glDisable(GL_BLEND);
}

// ─── Draw outline rectangle ───────────────────────────────────────────────────
static void drawRectOutline(float x, float y, float w, float h,
                             float r, float g, float b, float a, float lw = 1.5f)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(lw);
    glColor4f(r, g, b, a);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x,     y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x,     y + h);
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
}

// ─── Draw string ─────────────────────────────────────────────────────────────
static void drawStr(float x, float y, const char* str,
                    void* font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    while (*str) { glutBitmapCharacter(font, *str++); }
}

// ─── Progress bar ─────────────────────────────────────────────────────────────
static void drawProgressBar(float x, float y, float w, float h,
                             float progress,   // 0..1
                             float r, float g, float b)
{
    // Background
    drawRect(x, y, w, h, 0.1f, 0.1f, 0.1f, 0.6f);
    // Fill
    drawRect(x, y, w * progress, h, r, g, b, 0.85f);
    // Border
    drawRectOutline(x, y, w, h, r * 1.2f, g * 1.2f, b * 1.2f, 0.7f);
}

// ─── Format time helper ───────────────────────────────────────────────────────
static void formatTime(float seconds, char* buf, int bufSize)
{
    int m = (int)(seconds / 60);
    int s = (int)(seconds) % 60;
    int ms = (int)((seconds - (int)seconds) * 10);
    snprintf(buf, bufSize, "%02d:%02d.%d", m, s, ms);
}

// ─── Rank based on score ──────────────────────────────────────────────────────
static const char* getRank(int sc)
{
    if (sc >= 1100) return "#1  CHICKEN DINNER!";
    if (sc >= 900)  return "#2  SURVIVOR";
    if (sc >= 700)  return "#3  FIGHTER";
    if (sc >= 500)  return "#4  RECRUIT";
    return "#5  ROOKIE";
}
static void getRankColor(int sc, float& r, float& g, float& b)
{
    if (sc >= 1100) { r=1.0f; g=0.85f; b=0.0f; }       // gold
    else if (sc >= 900) { r=0.8f; g=0.8f; b=0.85f; }   // silver
    else if (sc >= 700) { r=0.8f; g=0.5f; b=0.2f; }    // bronze
    else { r=0.6f; g=0.9f; b=0.6f; }                    // green
}

// ─── SCOREBOARD SCREEN ────────────────────────────────────────────────────────
static float sbAnim = 0.0f;   // scoreboard entrance animation

static void drawScoreboard()
{
    sbAnim += 0.03f;
    if (sbAnim > 1.0f) sbAnim = 1.0f;

    float W = (float)windowWidth;
    float H = (float)windowHeight;

    // Dark overlay
    drawRect(0, 0, W, H, 0.0f, 0.0f, 0.0f, 0.78f);

    // Panel dimensions
    float pw = 680.0f;
    float ph = 580.0f;
    float px = (W - pw) * 0.5f;
    float py = (H - ph) * 0.5f - (1.0f - sbAnim) * H * 0.3f;   // slide up

    // Panel background (dark military tone)
    drawRect(px, py, pw, ph, 0.06f, 0.07f, 0.05f, 0.96f);
    drawRectOutline(px, py, pw, ph, 0.85f, 0.70f, 0.10f, 1.0f, 2.5f);

    // Inner border decoration
    drawRectOutline(px + 6, py + 6, pw - 12, ph - 12,
                    0.85f, 0.70f, 0.10f, 0.35f, 1.0f);

    // ── Header banner ─────────────────────────────────────────────────────────
    float pulse = 0.7f + 0.3f * sinf(sbAnim * 3.14f + glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    drawRect(px, py + ph - 72, pw, 72, 0.85f * 0.25f, 0.70f * 0.25f, 0.10f * 0.25f, 0.95f);
    drawRectOutline(px, py + ph - 72, pw, 72, 0.85f * pulse, 0.70f * pulse, 0.10f * pulse, 0.8f, 2.0f);

    // "MATCH RESULTS" title
    glColor3f(0.95f, 0.80f, 0.05f);
    drawStr(px + pw / 2 - 120, py + ph - 30, "===  MATCH RESULTS  ===", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.75f, 0.75f, 0.75f);
    drawStr(px + pw / 2 - 100, py + ph - 52, "City Explorer Challenge", GLUT_BITMAP_HELVETICA_12);

    float cy = py + ph - 100;   // current y cursor (going downward)

    // ── RANK ──────────────────────────────────────────────────────────────────
    float rr, rg, rb;
    getRankColor(score, rr, rg, rb);
    const char* rankStr = getRank(score);

    drawRect(px + 20, cy - 48, pw - 40, 54, rr * 0.15f, rg * 0.15f, rb * 0.15f, 0.85f);
    drawRectOutline(px + 20, cy - 48, pw - 40, 54, rr, rg, rb, 0.9f, 2.0f);
    glColor3f(rr, rg, rb);
    drawStr(px + pw / 2 - 120, cy + 0, rankStr, GLUT_BITMAP_TIMES_ROMAN_24);
    cy -= 66;

    // ── Stats rows ────────────────────────────────────────────────────────────
    // Divider
    glColor3f(0.50f, 0.45f, 0.20f);
    glBegin(GL_LINES);
        glVertex2f(px + 20, cy + 2);
        glVertex2f(px + pw - 20, cy + 2);
    glEnd();
    cy -= 14;

    auto statRow = [&](const char* label, const char* value,
                       float vr = 0.2f, float vg = 1.0f, float vb = 0.5f)
    {
        drawRect(px + 20, cy - 30, pw - 40, 34, 0.10f, 0.11f, 0.08f, 0.55f);
        drawRectOutline(px + 20, cy - 30, pw - 40, 34, 0.3f, 0.3f, 0.2f, 0.4f);
        glColor3f(0.72f, 0.72f, 0.72f);
        drawStr(px + 34, cy - 10, label, GLUT_BITMAP_HELVETICA_12);
        glColor3f(vr, vg, vb);
        drawStr(px + pw - 40 - (float)strlen(value) * 9.0f, cy - 10, value, GLUT_BITMAP_HELVETICA_18);
        cy -= 42;
    };

    // Score
    char buf[128];
    snprintf(buf, 128, "%d PTS", score);
    statRow("TOTAL SCORE", buf, 0.95f, 0.85f, 0.10f);

    // Missions
    snprintf(buf, 128, "%d / %d", totalCompleted, MAX_MISSIONS);
    statRow("MISSIONS COMPLETED", buf, 0.3f, 1.0f, 0.5f);

    // Completion time
    char timeBuf[32];
    formatTime(completionTime, timeBuf, 32);
    statRow("COMPLETION TIME", timeBuf, 0.4f, 0.85f, 1.0f);

    // Time bonus
    int timeBonus = score - totalCompleted * 100;
    snprintf(buf, 128, "+ %d PTS", timeBonus > 0 ? timeBonus : 0);
    statRow("TIME BONUS", buf, 0.95f, 0.60f, 0.10f);

    // Divider
    cy += 8;
    glColor3f(0.50f, 0.45f, 0.20f);
    glBegin(GL_LINES);
        glVertex2f(px + 20, cy);
        glVertex2f(px + pw - 20, cy);
    glEnd();
    cy -= 20;

    // ── Mission log ───────────────────────────────────────────────────────────
    glColor3f(0.70f, 0.65f, 0.30f);
    drawStr(px + 34, cy, "MISSION LOG", GLUT_BITMAP_HELVETICA_12);
    cy -= 26;

    for (int i = 0; i < MAX_MISSIONS; ++i) {
        bool done = missions[i].completed;
        float bgA = done ? 0.30f : 0.12f;
        float tileR = done ? 0.08f : 0.06f;
        float tileG = done ? 0.14f : 0.08f;
        float tileB = done ? 0.08f : 0.06f;
        drawRect(px + 20, cy - 22, pw - 40, 26, tileR, tileG, tileB, bgA);

        // Tick / cross
        glColor3f(done ? 0.2f : 0.7f, done ? 0.9f : 0.25f, done ? 0.3f : 0.25f);
        drawStr(px + 32, cy - 7, done ? "[X]" : "[ ]", GLUT_BITMAP_HELVETICA_12);

        glColor3f(done ? 0.75f : 0.45f, done ? 0.95f : 0.45f, done ? 0.75f : 0.45f);
        drawStr(px + 68, cy - 7, missions[i].title, GLUT_BITMAP_HELVETICA_12);

        // +100 pts label
        if (done) {
            glColor3f(0.95f, 0.75f, 0.10f);
            drawStr(px + pw - 100, cy - 7, "+100 pts", GLUT_BITMAP_HELVETICA_12);
        }
        cy -= 29;
    }

    // ── Press key to exit ────────────────────────────────────────────────────
    float blinkA = 0.5f + 0.5f * sinf(glutGet(GLUT_ELAPSED_TIME) * 0.004f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.85f, 0.85f, 0.85f, blinkA);
    drawStr(px + pw / 2 - 110, py + 16, "Press  ESC  to exit", GLUT_BITMAP_HELVETICA_18);
    glDisable(GL_BLEND);
}

// ─── MAIN HUD (in-game) ───────────────────────────────────────────────────────
void drawHUD()
{
    beginOverlay();

    if (gameCompleted) {
        drawScoreboard();
        endOverlay();
        return;
    }

    float W = (float)windowWidth;
    float H = (float)windowHeight;

    // ─── LEFT: Mission list panel ─────────────────────────────────────────────
    float mlW = 360.0f, mlH = 30.0f + MAX_MISSIONS * 20.0f;
    float mlX = 10.0f, mlY = H - mlH - 10.0f;

    drawRect(mlX, mlY, mlW, mlH, 0.03f, 0.03f, 0.03f, 0.70f);
    drawRectOutline(mlX, mlY, mlW, mlH, 0.85f, 0.70f, 0.10f, 0.6f, 1.5f);

    drawRect(mlX, mlY + mlH - 28, mlW, 28, 0.15f, 0.12f, 0.02f, 0.85f);
    glColor3f(0.95f, 0.80f, 0.10f);
    drawStr(mlX + 12, mlY + mlH - 18, "[ MISSION LOG ]", GLUT_BITMAP_HELVETICA_18);

    for (int i = 0; i < MAX_MISSIONS; ++i) {
        Mission& m = missions[i];
        float rowY = mlY + mlH - 45.0f - i * 20.0f;
        char prefix[8] = "   ";
        if (m.completed) {
            strcpy(prefix, "[V]");
            glColor3f(0.4f, 0.8f, 0.4f);
        } else if (i == currentMission) {
            strcpy(prefix, "-> ");
            glColor3f(0.9f, 0.8f, 0.2f);
        } else {
            strcpy(prefix, "[ ]");
            glColor3f(0.6f, 0.6f, 0.6f);
        }
        
        char line[128];
        snprintf(line, 128, "%s %s", prefix, m.title);
        drawStr(mlX + 12, rowY, line, GLUT_BITMAP_HELVETICA_12);
        
        if (i == currentMission && m.timeLimit > 0.0f) {
            snprintf(line, 128, "Time Left: %.1f", missionTimeLeft);
            drawStr(mlX + 220, rowY, line, GLUT_BITMAP_HELVETICA_12);
        }
    }

    // ─── TOP-RIGHT: Minimap ───────────────────────────────────────────────────
    float mapX = W - 130.0f, mapY = H - 130.0f;
    drawRect(mapX, mapY, 120.0f, 120.0f, 0.0f, 0.0f, 0.0f, 0.55f);
    
    // Player dot
    float px = mapX + 60.0f + (playerX / 100.0f) * 55.0f;
    float py = mapY + 60.0f + (playerZ / 100.0f) * 55.0f;
    drawRect(px - 3.0f, py - 3.0f, 6.0f, 6.0f, 0.2f, 0.8f, 0.2f, 1.0f);

    // Current mission marker on map
    if (currentMission < MAX_MISSIONS) {
        Mission& m = missions[currentMission];
        float mx = mapX + 60.0f + (m.targetX / 100.0f) * 55.0f;
        float my = mapY + 60.0f + (m.targetZ / 100.0f) * 55.0f;
        drawRect(mx - 2.0f, my - 2.0f, 4.0f, 4.0f, 1.0f, 0.6f, 0.1f, 1.0f);
    }

    // ─── RIGHT: Score + Time panel (below minimap) ────────────────────────────
    float srW = 230.0f, srH = 110.0f;
    float srX = W - srW - 10.0f, srY = mapY - srH - 10.0f;

    drawRect(srX, srY, srW, srH, 0.03f, 0.03f, 0.03f, 0.70f);
    drawRectOutline(srX, srY, srW, srH, 0.85f, 0.70f, 0.10f, 0.6f, 1.5f);

    // Score
    drawRect(srX, srY + srH - 36, srW, 36, 0.12f, 0.10f, 0.02f, 0.85f);
    char buf[64];
    glColor3f(0.95f, 0.85f, 0.10f);
    snprintf(buf, 64, "SCORE:  %d", score);
    drawStr(srX + 12, srY + srH - 22, buf, GLUT_BITMAP_HELVETICA_18);

    // Elapsed time
    char timeBuf[32];
    formatTime(gameElapsedTime, timeBuf, 32);
    glColor3f(0.45f, 0.85f, 1.0f);
    snprintf(buf, 64, "TIME:  %s", timeBuf);
    drawStr(srX + 12, srY + srH - 58, buf, GLUT_BITMAP_HELVETICA_18);

    // Missions counter
    glColor3f(0.75f, 0.90f, 0.75f);
    snprintf(buf, 64, "Missions:  %d / %d", totalCompleted, MAX_MISSIONS);
    drawStr(srX + 12, srY + srH - 80, buf, GLUT_BITMAP_HELVETICA_12);

    // Day/time
    float h24 = timeOfDay * 24.0f;
    int hh    = (int)h24;
    int mm    = (int)((h24 - hh) * 60.0f);
    glColor3f(0.80f, 0.80f, 0.70f);
    snprintf(buf, 64, "In-Game: %02d:%02d", hh, mm);
    drawStr(srX + 12, srY + 10, buf, GLUT_BITMAP_HELVETICA_12);
    
    // ─── Stamina Bar ──────────────────────────────────────────────────────────
    if (isSprinting || stamina < 99.0f) {
        float barY = srY - 25.0f;
        float barW = 150.0f;
        float bw = (stamina / 100.0f) * barW;
        float r = 0.2f, g = 0.8f, b = 0.2f;
        if (stamina < 50.0f) { r = 0.8f; g = 0.8f; b = 0.1f; }
        if (stamina < 20.0f) { r = 0.9f; g = 0.2f; b = 0.2f; }
        
        drawRect(srX, barY, barW, 15.0f, 0.1f, 0.1f, 0.1f, 0.8f); // bg
        drawRect(srX, barY, bw, 15.0f, r, g, b, 0.9f); // fill
    }

    // ─── BOTTOM-LEFT: Controls ────────────────────────────────────────────────
    float clW = 300.0f, clH = 100.0f;
    drawRect(10, 10, clW, clH, 0.03f, 0.03f, 0.03f, 0.60f);
    drawRectOutline(10, 10, clW, clH, 0.4f, 0.4f, 0.3f, 0.5f);

    glColor3f(0.65f, 0.65f, 0.60f);
    drawStr(20, 92, "CONTROLS", GLUT_BITMAP_HELVETICA_12);
    drawStr(20, 76, "WASD / Arrow Keys  :  Move", GLUT_BITMAP_HELVETICA_12);
    drawStr(20, 60, "Left-drag Mouse    :  Camera", GLUT_BITMAP_HELVETICA_12);
    drawStr(20, 44, "Mouse Wheel        :  Zoom", GLUT_BITMAP_HELVETICA_12);
    drawStr(20, 28, "+ / -              :  Day Speed", GLUT_BITMAP_HELVETICA_12);
    drawStr(20, 12, "ESC                :  Quit", GLUT_BITMAP_HELVETICA_12);

    // ─── BOTTOM-RIGHT: Minimap-style Position ────────────────────────────────
    float mpW = 190.0f, mpH = 70.0f;
    float mpX = W - mpW - 10.0f;
    drawRect(mpX, 10, mpW, mpH, 0.03f, 0.03f, 0.03f, 0.60f);
    drawRectOutline(mpX, 10, mpW, mpH, 0.4f, 0.4f, 0.3f, 0.5f);

    glColor3f(0.55f, 0.85f, 1.0f);
    snprintf(buf, 64, "X: %+.1f  Z: %+.1f", playerX, playerZ);
    drawStr(mpX + 10, 60, buf, GLUT_BITMAP_HELVETICA_12);
    glColor3f(0.6f, 0.6f, 0.6f);
    snprintf(buf, 64, "Facing: %.0f deg", playerAngle);
    drawStr(mpX + 10, 42, buf, GLUT_BITMAP_HELVETICA_12);

    // Compass
    const char* dirLabel = "N";
    float ang = fmodf(playerAngle + 720.0f, 360.0f);
    if (ang > 315 || ang <= 45)  dirLabel = "S";
    else if (ang > 45 && ang <= 135)  dirLabel = "E";
    else if (ang > 135 && ang <= 225) dirLabel = "N";
    else dirLabel = "W";
    glColor3f(1.0f, 0.4f, 0.2f);
    snprintf(buf, 64, "DIR: %s", dirLabel);
    drawStr(mpX + 10, 24, buf, GLUT_BITMAP_HELVETICA_12);

    // ─── CENTER FLASH message ─────────────────────────────────────────────────
    if (flashTimer < flashDuration) {
        float fadeT = flashDuration - flashTimer;
        float alpha = (fadeT < 1.2f) ? fadeT / 1.2f : 1.0f;

        float cx = W * 0.5f;
        float cy2 = H * 0.62f;

        int msgLen = (int)strlen(flashMsg);
        float fw = msgLen * 10.5f;

        drawRect(cx - fw * 0.5f - 20, cy2 - 24, fw + 40, 48,
                 0.05f, 0.25f, 0.05f, 0.80f * alpha);
        drawRectOutline(cx - fw * 0.5f - 20, cy2 - 24, fw + 40, 48,
                        0.3f, 1.0f, 0.4f, 0.9f * alpha, 2.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.2f, 1.0f, 0.4f, alpha);
        drawStr(cx - fw * 0.5f, cy2 - 6, flashMsg, GLUT_BITMAP_HELVETICA_18);
        glDisable(GL_BLEND);
    }

    endOverlay();
}
