#include <GL/freeglut.h>
#include <cmath>
#include "lighting.h"
#include "globals.h"

// ─── initLighting ─────────────────────────────────────────────────────────────
void initLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);      // directional sun / moon
    glEnable(GL_LIGHT1);      // street lamp 1
    glEnable(GL_LIGHT2);      // street lamp 2
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);

    // ─── Street lamp 1 ──────────────────────────────────────────────────────
    {
        GLfloat pos[]  = { 12.0f, 5.5f, -15.0f, 1.0f };
        GLfloat dif[]  = { 1.0f,  0.9f,  0.6f,  1.0f };
        GLfloat spec[] = { 1.0f,  1.0f,  0.8f,  1.0f };
        GLfloat amb[]  = { 0.05f, 0.05f, 0.02f, 1.0f };
        glLightfv(GL_LIGHT1, GL_POSITION, pos);
        glLightfv(GL_LIGHT1, GL_DIFFUSE,  dif);
        glLightfv(GL_LIGHT1, GL_SPECULAR, spec);
        glLightfv(GL_LIGHT1, GL_AMBIENT,  amb);
        glLightf (GL_LIGHT1, GL_CONSTANT_ATTENUATION,  0.5f);
        glLightf (GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.05f);
        glLightf (GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.01f);
    }

    // ─── Street lamp 2 ──────────────────────────────────────────────────────
    {
        GLfloat pos[]  = {-12.0f, 5.5f,  10.0f, 1.0f };
        GLfloat dif[]  = { 1.0f,  0.9f,  0.6f,  1.0f };
        GLfloat spec[] = { 1.0f,  1.0f,  0.8f,  1.0f };
        GLfloat amb[]  = { 0.05f, 0.05f, 0.02f, 1.0f };
        glLightfv(GL_LIGHT2, GL_POSITION, pos);
        glLightfv(GL_LIGHT2, GL_DIFFUSE,  dif);
        glLightfv(GL_LIGHT2, GL_SPECULAR, spec);
        glLightfv(GL_LIGHT2, GL_AMBIENT,  amb);
        glLightf (GL_LIGHT2, GL_CONSTANT_ATTENUATION,  0.5f);
        glLightf (GL_LIGHT2, GL_LINEAR_ATTENUATION,    0.05f);
        glLightf (GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.01f);
    }
}

// ─── updateLighting ───────────────────────────────────────────────────────────
// timeOfDay 0.0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset
void updateLighting()
{
    // Advance day cycle
    timeOfDay += daySpeed;
    if (timeOfDay > 1.0f) timeOfDay -= 1.0f;

    float t = timeOfDay;

    // ── Sun elevation angle: 0 at midnight, peak at noon ─────────────────────
    float sunAngle = (t - 0.25f) * 2.0f * 3.14159265f; // full revolution
    float sunElev  = sinf(sunAngle);          // -1..+1
    float sunAzi   = cosf(sunAngle);

    // ── Sky & clear colour ───────────────────────────────────────────────────
    float r, g, b;
    if (sunElev < 0.0f) {
        // Night: dark navy
        r = 0.02f; g = 0.02f; b = 0.08f;
    } else if (sunElev < 0.15f) {
        // Sunrise / sunset: orange-pink blend
        float k = sunElev / 0.15f;
        r = 0.9f * k + 0.02f * (1.0f - k);
        g = 0.4f * k + 0.02f * (1.0f - k);
        b = 0.2f * k + 0.08f * (1.0f - k);
    } else {
        // Day: blue sky
        float k = (sunElev - 0.15f) / 0.85f;
        r = 0.4f  + 0.1f  * (1.0f - k);
        g = 0.65f + 0.15f * k;
        b = 1.0f;
    }
    glClearColor(r, g, b, 1.0f);

    // ── Sun light direction ───────────────────────────────────────────────────
    GLfloat sunPos[] = { sunAzi, sunElev * 2.0f, -0.5f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, sunPos);

    float bright = (sunElev > 0.0f) ? sunElev : 0.0f;
    float warmR  = 1.0f;
    float warmG  = 0.85f + 0.15f * bright;
    float warmB  = 0.7f  + 0.3f  * bright;

    GLfloat sunDif[]  = { warmR * bright, warmG * bright, warmB * bright, 1.0f };
    GLfloat sunSpec[] = { bright * 0.8f,  bright * 0.8f,  bright * 0.8f, 1.0f };
    float ambI = 0.05f + 0.25f * bright;
    GLfloat sunAmb[]  = { ambI, ambI, ambI * 0.9f, 1.0f };

    glLightfv(GL_LIGHT0, GL_DIFFUSE,  sunDif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpec);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  sunAmb);

    // ── Street lamps: only at night ───────────────────────────────────────────
    if (sunElev < 0.05f) {
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHT2);
    } else {
        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHT2);
    }
}
