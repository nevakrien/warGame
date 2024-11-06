#ifndef SOLDIER_H
#define SOLDIER_H

#include <stddef.h>
#include <raylib.h>
#include <box2d/box2d.h>
#include "id.h"
#include "team.h"
#include <stdatomic.h>
#include <assert.h>
#include <stdio.h>

#define SOLDIER_RADIUS 10.0f
#define SOLDIER_SPEAR_LENGTH 25.0f
#define SOLDIER_SPEAR_TIP_SIZE 0.5f
#define SOLDIER_SPEAR_TIP_LEN 2.0f
#define SOLDIER_HAND_OFFSET (SOLDIER_RADIUS * 0.5f)

#define MAX_TOUCHING_SPEAR 100

typedef struct Soldier Soldier;

struct Soldier{
    TypeID id;
    Team* team;
    b2BodyId body;                // Soldier's main body
    b2ShapeId bodyShapeId;        // Shape ID for the soldier's body (circle)
    b2ShapeId spearShaftShapeId;  // Shape ID for the spear shaft (segment)
    b2ShapeId spearTipShapeId;    // Shape ID for the spear tip (polygon)
    
    bool isHit;                   // Flag indicating if the soldier has been hit
    bool hasHitTarget;            // Flag indicating if the soldier's spear has hit a target
    
    volatile int numTouch;
    volatile Soldier* touchingSpear[MAX_TOUCHING_SPEAR];

    float health;
} ;

void Soldier_Init(Soldier* soldier,b2WorldId world, Vector2 position, float rotation, Team* team,float health);
void Soldier_RenderAlive(Soldier* soldier);
void Soldier_RenderDead(Soldier* soldier);
void Soldier_FrameReset(Soldier* soldier,b2WorldId world);

static inline bool Soldier_IsAlive(Soldier* soldier){
    return !isnan(soldier->health);
    // return !B2_ID_EQUALS(soldier->body,b2_nullBodyId);
}

static inline void Soldier_AddMe(Soldier* other,Soldier* me){
    void* null = NULL;
    for(int i=0;i<10*MAX_TOUCHING_SPEAR;i++){
        if(atomic_compare_exchange_strong(
            other->touchingSpear+i%MAX_TOUCHING_SPEAR,
            &null,
            me
        )){
            return;
        }
    }
    assert(0 && "failled to find a spot for me");
}



static inline void Soldier_DelMe(Soldier* other, Soldier* me) {
    for (int i = 0; i < MAX_TOUCHING_SPEAR; i++) {        
        if (atomic_compare_exchange_strong(
                other->touchingSpear+i,  
                &me,      
                NULL
            )) {
            return;
        }
    }
    // assert(0 && "failed to find me");
    printf("warning soldier was not removed!!!");
}


static inline void Soldier_Die(Soldier* soldier,b2WorldId world){
    for(int i=0;i<MAX_TOUCHING_SPEAR;i++){
        volatile Soldier* other = soldier->touchingSpear[i];
        if(other==NULL){
            continue;
        }
        atomic_fetch_sub(&other->numTouch, 1);  // Pass the address of numTouch

    }

    // b2DestroyBody(soldier->body);
    b2Body_Disable  (soldier->body);
    // soldier->body = b2_nullBodyId;
    soldier->health = NAN;//make sure we wont triger health check again.

}

static inline void moveSoldier(Soldier* src,Soldier* dest){
    if(Soldier_IsAlive(src)){
        b2Body_SetUserData(src->body,dest);
    }
    *dest = *src;
}


#endif // SOLDIER_H
