#include <GL/freeglut.h>
#include <cstdio>
#include <ctime>

#include "globals.h"
#include "player.h"
#include "camera.h"
#include "environment.h"
#include "collision.h"
#include "lighting.h"
#include "mission.h"
#include "ui.h"

// ─── Timing ───────────────────────────────────────────────────────────────────
static int   lastTime  = 0;
static float deltaTime = 0.016f;

// ─── display ─────────────────────────────────────────────────────────────────
void display()
{
    int now = glutGet(GLUT_ELAPSED_TIME);
    deltaTime = (now - lastTime) * 0.001f;
    if (deltaTime > 0.1f) deltaTime = 0.1f;
    lastTime = now;

    // ── Game logic ───────────────────────────────────────────────────────────
    updateLighting();
    updatePlayer(deltaTime);

    // Accumulate game timer (only while game is not completed)
    if (!gameCompleted) {
        gameElapsedTime += deltaTime;
    }

    updateMissions();
    updateFlash(deltaTime);

    // ── Render ───────────────────────────────────────────────────────────────
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)windowWidth / windowHeight, 0.1, 500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    applyCamera();

    drawEnvironment();
    drawMissionMarkers();
    drawPlayer();

    // 2-D HUD / Scoreboard
    drawHUD();

    glutSwapBuffers();
}

// ─── reshape ─────────────────────────────────────────────────────────────────
void reshape(int w, int h)
{
    if (h == 0) h = 1;
    windowWidth  = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}

// ─── keyboard ────────────────────────────────────────────────────────────────
void keyboard(unsigned char key, int x, int y)
{
    (void)x; (void)y;
    if (key == 27) {   // ESC — quit at any state
        glutLeaveMainLoop();
        return;
    }
    // Day-speed controls (only while playing)
    if (!gameCompleted) {
        if (key == '+' || key == '=') daySpeed *= 2.0f;
        if (key == '-' || key == '_') daySpeed *= 0.5f;
        if (key >= '0' && key <= '6') activeTransformation = key - '0';
        movePlayer(key);
    }
    glutPostRedisplay();
}

void keyboardSpecial(int key, int x, int y)
{
    (void)x; (void)y;
    if (!gameCompleted) movePlayerSpecial(key);
    glutPostRedisplay();
}

// ─── idle ────────────────────────────────────────────────────────────────────
void idle()
{
    glutPostRedisplay();
}

// ─── init ────────────────────────────────────────────────────────────────────
void init()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_NORMALIZE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    initLighting();
    initEnvironment();
    initPlayer();
    initMissions();

    setFlashMessage("Welcome to City Explorer! Follow the beacon!", 4.0f);

    lastTime = glutGet(GLUT_ELAPSED_TIME);
}

// ─── main ────────────────────────────────────────────────────────────────────
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(40, 40);

    glutCreateWindow("City Explorer  |  PUBG Edition  |  WASD: Move  |  Mouse: Camera  |  ESC: Quit");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboardSpecial);
    glutMouseFunc(cameraMouseButton);
    glutMotionFunc(cameraMouseMotion);
    glutMouseWheelFunc(cameraMouseWheel);
    glutIdleFunc(idle);

    printf("==========================================================\n");
    printf("  City Explorer  —  PUBG Edition\n");
    printf("==========================================================\n");
    printf("  Controls:\n");
    printf("    WASD / Arrow Keys  : Move player\n");
    printf("    Left-drag mouse    : Orbit camera\n");
    printf("    Scroll wheel       : Zoom in / out\n");
    printf("    + / -              : Speed up / slow down day cycle\n");
    printf("    1-6                : Presentation Transformations\n");
    printf("    0                  : Normal Mode\n");
    printf("    ESC                : Quit\n");
    printf("==========================================================\n");
    printf("  Complete all 6 missions as fast as possible!\n");
    printf("  Score = 100 pts per mission + Time Bonus.\n");
    printf("==========================================================\n");

    glutMainLoop();
    return 0;
}
