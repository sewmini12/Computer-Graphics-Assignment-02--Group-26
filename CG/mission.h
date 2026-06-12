#ifndef MISSION_H
#define MISSION_H

#define MAX_MISSIONS 6

struct Mission {
    const char* title;
    const char* description;
    float       targetX, targetZ;   // world position of goal marker
    float       radius;             // proximity needed to complete
    bool        active;
    bool        completed;
};

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
