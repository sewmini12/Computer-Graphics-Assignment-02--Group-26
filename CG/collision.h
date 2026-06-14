#ifndef COLLISION_H
#define COLLISION_H

// A simple Axis-Aligned Bounding Box (AABB)
struct AABB {
    float minX, maxX;
    float minZ, maxZ;
};

// Maximum number of collidable objects in the scene
#define MAX_COLLIDERS 64

extern AABB colliders[MAX_COLLIDERS];
extern int  colliderCount;

// Register a box collider (call from environment setup)
void addCollider(float cx, float cz, float halfW, float halfD);

// Returns true if the point (x,z) with radius 'r' overlaps any collider
bool checkCollision(float x, float z, float r = 0.6f);

// Returns the fraction (0.0 to 1.0) of the ray from (px, pz) to (ex, ez) before it hits a collider.
float checkRayCollision(float px, float pz, float ex, float ez, float r);

// Check if player picked up a collectible
int checkPickup(float x, float z, float radius);

// Get ground height at given coordinates
float getGroundHeight(float x, float z);

#endif // COLLISION_H
