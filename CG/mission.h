#ifndef MISSION_H
#define MISSION_H

#define MAX_MISSIONS 9

struct Mission {
    const char* title;
    const char* description;
    float       targetX, targetZ;   // world position of goal marker
    float       radius;             // proximity needed to complete
    bool        active;
    bool        completed;
    float       timeLimit;          // optional time limit (0 = no limit)
};

struct Collectible {
    float x, z;
    bool collected;
};

extern Collectible collectibles[10];

extern Mission missions[MAX_MISSIONS];
extern int     currentMission;     // index into missions[]
extern int     totalCompleted;

// Call once during init
void initMissions();

// Call each frame (checks proximity to goal)
void updateMissions();

// Draw goal markers in 3D world
void drawMissionMarkers();

#endif // MISSION_H
