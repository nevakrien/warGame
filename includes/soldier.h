#ifndef SOLDIER_H
#define SOLDIER_H

#include <raylib.h>
#include <box2d/box2d.h>
#include "id.h"
#include "team.h"
#include <stdatomic.h>

#define SOLDIER_RADIUS 10.0f
#define SOLDIER_SPEAR_LENGTH 25.0f
#define SOLDIER_SPEAR_TIP_SIZE 0.5f
#define SOLDIER_SPEAR_TIP_LEN 2.0f
#define SOLDIER_HAND_OFFSET (SOLDIER_RADIUS * 0.5f)

typedef struct {
    TypeID id;
    Team* team;
    b2BodyId body;                // Soldier's main body
    b2ShapeId bodyShapeId;        // Shape ID for the soldier's body (circle)
    b2ShapeId spearShaftShapeId;  // Shape ID for the spear shaft (segment)
    b2ShapeId spearTipShapeId;    // Shape ID for the spear tip (polygon)
    
    bool isHit;                   // Flag indicating if the soldier has been hit
    bool hasHitTarget;            // Flag indicating if the soldier's spear has hit a target
    atomic_int numTouch;

    float health;
} Soldier;

void Soldier_Init(Soldier* soldier,b2WorldId world, Vector2 position, float rotation, Team* team,float health);
void Soldier_RenderAlive(Soldier* soldier);
void Soldier_RenderDead(Soldier* soldier);
void Soldier_FrameReset(Soldier* soldier,b2WorldId world);

static inline bool Soldier_IsAlive(Soldier* soldier){
    return !isnan(soldier->health);
    // return !B2_ID_EQUALS(soldier->body,b2_nullBodyId);
}

// void handle_residual_touch(Soldier* deadSoldier,b2WorldId world);

static inline void Soldier_Die(Soldier* soldier,b2WorldId world){
    // b2DestroyBody(soldier->body);
    b2Body_Disable  (soldier->body);
    // soldier->body = b2_nullBodyId;
    soldier->health = NAN;//make sure we wont triger health check again.
    // if(soldier->hasHitTarget){
    //     handle_residual_touch(soldier,world);
    // }
}

static inline void moveSoldier(Soldier* src,Soldier* dest){
    if(Soldier_IsAlive(src)){
        b2Body_SetUserData(src->body,dest);
    }
    *dest = *src;
}


#endif // SOLDIER_H
