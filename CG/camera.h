#ifndef CAMERA_H
#define CAMERA_H

// Apply third-person follow camera using current globals
void applyCamera();

// Mouse callbacks – register these in main.cpp
void cameraMouseButton(int button, int state, int x, int y);
void cameraMouseMotion(int x, int y);
void cameraMouseWheel(int wheel, int dir, int x, int y);

#endif // CAMERA_H
