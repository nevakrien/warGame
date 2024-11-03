#ifndef SOLDIER_H
#define SOLDIER_H

#include <raylib.h>
#include <box2d/box2d.h>
#include "collision.h"

#define SOLDIER_RADIUS 10.0f
#define SOLDIER_SPEAR_LENGTH 25.0f
#define SOLDIER_SPEAR_TIP_SIZE 1.0f

typedef struct {
    TypeID id;
    Color color;
    b2BodyId body;                // Soldier's main body
    b2ShapeId bodyShapeId;        // Shape ID for the soldier's body (circle)
    b2ShapeId spearShaftShapeId;  // Shape ID for the spear shaft (segment)
    b2ShapeId spearTipShapeId;    // Shape ID for the spear tip (polygon)
    
    bool isHit;                   // Flag indicating if the soldier has been hit
    bool hasHitTarget;            // Flag indicating if the soldier's spear has hit a target
} Soldier;

void Soldier_Init(Soldier* soldier,b2WorldId world, Vector2 position, float rotation, Color color);
void Soldier_Render(Soldier* soldier);
void Soldier_FrameReset(Soldier* soldier);



#endif // SOLDIER_H
