#ifndef LIGHTING_H
#define LIGHTING_H

// Setup lighting – call once after GL init
void initLighting();

// Update sun/sky colour every frame based on timeOfDay
void updateLighting();

#endif // LIGHTING_H
