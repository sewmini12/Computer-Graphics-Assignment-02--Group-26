#include "collision.h"

AABB colliders[MAX_COLLIDERS];
int  colliderCount = 0;

void addCollider(float cx, float cz, float halfW, float halfD)
{
    if (colliderCount >= MAX_COLLIDERS) return;
    colliders[colliderCount].minX = cx - halfW;
    colliders[colliderCount].maxX = cx + halfW;
    colliders[colliderCount].minZ = cz - halfD;
    colliders[colliderCount].maxZ = cz + halfD;
    ++colliderCount;
}

bool checkCollision(float x, float z, float r)
{
    for (int i = 0; i < colliderCount; ++i) {
        // Expand AABB by player radius
        if (x > colliders[i].minX - r &&
            x < colliders[i].maxX + r &&
            z > colliders[i].minZ - r &&
            z < colliders[i].maxZ + r)
        {
            return true;
        }
    }
    return false;
}
