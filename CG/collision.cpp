#include "collision.h"
#include "mission.h"
#include <cmath>
#include <algorithm>

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

float checkRayCollision(float px, float pz, float ex, float ez, float r)
{
    float min_t = 1.0f;
    float dx = ex - px;
    float dz = ez - pz;
    
    for (int i = 0; i < colliderCount; ++i) {
        float minX = colliders[i].minX - r;
        float maxX = colliders[i].maxX + r;
        float minZ = colliders[i].minZ - r;
        float maxZ = colliders[i].maxZ + r;
        
        float t_near = -1e30f;
        float t_far = 1e30f;
        
        if (std::abs(dx) < 1e-6f) {
            if (px < minX || px > maxX) continue; // Ray is parallel and outside
        } else {
            float t1 = (minX - px) / dx;
            float t2 = (maxX - px) / dx;
            if (t1 > t2) std::swap(t1, t2);
            if (t1 > t_near) t_near = t1;
            if (t2 < t_far) t_far = t2;
        }
        
        if (std::abs(dz) < 1e-6f) {
            if (pz < minZ || pz > maxZ) continue;
        } else {
            float t3 = (minZ - pz) / dz;
            float t4 = (maxZ - pz) / dz;
            if (t3 > t4) std::swap(t3, t4);
            if (t3 > t_near) t_near = t3;
            if (t4 < t_far) t_far = t4;
        }
        
        if (t_near < t_far && t_far > 0.0f) {
            float t = (t_near > 0.0f) ? t_near : 0.0f;
            if (t < min_t) min_t = t;
        }
    }
    return min_t;
}

int checkPickup(float x, float z, float radius) {
    for (int i = 0; i < 10; ++i) {
        if (!collectibles[i].collected) {
            float dx = collectibles[i].x - x;
            float dz = collectibles[i].z - z;
            if (std::sqrt(dx*dx + dz*dz) <= radius) {
                return i;
            }
        }
    }
    return -1;
}

float getGroundHeight(float x, float z) {
    // Small hill near park
    if (x > -50 && x < -30 && z > 30 && z < 50)
        return 1.5f * std::sin((x+50)*0.157f) * std::sin((z-30)*0.157f);
    return 0.0f;
}
