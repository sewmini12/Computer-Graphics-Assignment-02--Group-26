#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

// Register all building/tree colliders – call once during init
void initEnvironment();

void drawGround();
void drawRoads();
void drawBuildings();
void drawTrees();
void drawStreetLamps();
void drawSkyDome();

// Convenience: draw everything
void drawEnvironment();

#endif // ENVIRONMENT_H
