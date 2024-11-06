#include "soldier.h"
#include "collision.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


// // Define the minimum speed threshold for triggering a hit
// #define HIT_SPEED_THRESHOLD 1.0f


// Inline function to handle begin contact events with detailed logging
static inline void handleBeginContacts(__attribute__((unused)) b2ContactEvents contactEvents) {
    for (int i = 0; i < contactEvents.beginCount; ++i) {
        b2ContactBeginTouchEvent* beginEvent = contactEvents.beginEvents + i;
        b2ShapeId shapeA = beginEvent->shapeIdA;
        b2ShapeId shapeB = beginEvent->shapeIdB;

        b2BodyId bodyA = b2Shape_GetBody(shapeA);
        b2BodyId bodyB = b2Shape_GetBody(shapeB);

        void* userDataA = b2Body_GetUserData(bodyA);
        void* userDataB = b2Body_GetUserData(bodyB);

        if (userDataA != NULL && userDataB != NULL) {
            TypeID typeA = *((TypeID*)userDataA);
            TypeID typeB = *((TypeID*)userDataB);

            if (typeA == TYPE_SOLDIER && typeB == TYPE_SOLDIER) {
                Soldier* soldierA = (Soldier*)userDataA;
                Soldier* soldierB = (Soldier*)userDataB;

                if (soldierA->team == soldierB->team) {
                    continue;
                }

                // Check if soldier A's spear is touching soldier B's body or vice versa
                bool soldierASpearTouchesSoldierB = 
                    B2_ID_EQUALS(shapeA, soldierA->spearTipShapeId) && B2_ID_EQUALS(shapeB, soldierB->bodyShapeId);
                bool soldierBSpearTouchesSoldierA = 
                    B2_ID_EQUALS(shapeA, soldierB->spearTipShapeId) && B2_ID_EQUALS(shapeB, soldierA->bodyShapeId);

                if (soldierASpearTouchesSoldierB) {
                    atomic_fetch_add(&(soldierB->numTouch), 1);
                    printf("touch\n");

                    // soldierB->numTouch++;  // Increase numTouch for Soldier B
                    // soldierA->hasHitTarget = true;  // Soldier A's spear hit Soldier B
                }
                if (soldierBSpearTouchesSoldierA) {
                    atomic_fetch_add(&(soldierA->numTouch), 1);
                    printf("touch\n");
                    
                    // soldierA->numTouch++;  // Increase numTouch for Soldier A
                    // soldierB->hasHitTarget = true;  // Soldier B's spear hit Soldier A

                }
            }
        }
    }
}

//cant make this work right because of death
// Inline function to handle end contact events with detailed logging
static inline void handleEndContacts(__attribute__((unused)) b2ContactEvents contactEvents) {
    // for (int i = 0; i < contactEvents.endCount; ++i) {
    //     b2ContactEndTouchEvent* endEvent = contactEvents.endEvents + i;
    //     b2ShapeId shapeA = endEvent->shapeIdA;
    //     b2ShapeId shapeB = endEvent->shapeIdB;

    //     b2BodyId bodyA = b2Shape_GetBody(shapeA);
    //     b2BodyId bodyB = b2Shape_GetBody(shapeB);

    //     void* userDataA = b2Body_GetUserData(bodyA);
    //     void* userDataB = b2Body_GetUserData(bodyB);

    //     if (userDataA != NULL && userDataB != NULL) {
    //         TypeID typeA = *((TypeID*)userDataA);
    //         TypeID typeB = *((TypeID*)userDataB);

    //         if (typeA == TYPE_SOLDIER && typeB == TYPE_SOLDIER) {
    //             Soldier* soldierA = (Soldier*)userDataA;
    //             Soldier* soldierB = (Soldier*)userDataB;

    //             if (soldierA->team == soldierB->team) {
    //                 continue;
    //             }

    //             // Check if soldier A's spear is no longer touching soldier B's body or vice versa
    //             bool soldierASpearNoLongerTouchesSoldierB = 
    //                 B2_ID_EQUALS(shapeA, soldierA->spearTipShapeId) && B2_ID_EQUALS(shapeB, soldierB->bodyShapeId);
    //             bool soldierBSpearNoLongerTouchesSoldierA = 
    //                 B2_ID_EQUALS(shapeA, soldierB->spearTipShapeId) && B2_ID_EQUALS(shapeB, soldierA->bodyShapeId);

    //             if (soldierASpearNoLongerTouchesSoldierB ) {
    //                 atomic_fetch_sub(&(soldierB->numTouch), 1);

    //                 // soldierB->numTouch--;  // Decrease numTouch for Soldier B
    //             }
    //             if (soldierBSpearNoLongerTouchesSoldierA ) {
    //                 atomic_fetch_sub(&(soldierA->numTouch), 1);

    //                 // soldierA->numTouch--;  // Decrease numTouch for Soldier A
    //             }
    //         }
    //     }
    // }
}


// Inline function to handle hit contact events and update soldier hit flags
static inline void handleHitContacts(b2ContactEvents contactEvents) {
    for (int i = 0; i < contactEvents.hitCount; ++i) {
        // assert(0); //debuging

        b2ContactHitEvent* hitEvent = contactEvents.hitEvents + i;
        b2ShapeId shapeA = hitEvent->shapeIdA;
        b2ShapeId shapeB = hitEvent->shapeIdB;

        // // Check if the approach speed is high enough to trigger a hit
        // if (hitEvent->approachSpeed < HIT_SPEED_THRESHOLD) {
        //     continue;  // Skip this hit event if the speed is too low
        // }

        b2BodyId bodyA = b2Shape_GetBody(shapeA);
        b2BodyId bodyB = b2Shape_GetBody(shapeB);

        void* userDataA = b2Body_GetUserData(bodyA);
        void* userDataB = b2Body_GetUserData(bodyB);

        if (userDataA != NULL && userDataB != NULL) {
            TypeID typeA = *((TypeID*)userDataA);
            TypeID typeB = *((TypeID*)userDataB);

            if (typeA == TYPE_SOLDIER && typeB == TYPE_SOLDIER) {
                Soldier* soldierA = (Soldier*)userDataA;
                Soldier* soldierB = (Soldier*)userDataB;

                if (soldierA->team==soldierB->team){
                	return;
                }

                // Determine if it's a spear-tip-to-body hit
                bool soldierASpearHitSoldierBBody = 
                    (B2_ID_EQUALS(shapeA, soldierA->spearTipShapeId) && B2_ID_EQUALS(shapeB, soldierB->bodyShapeId)) ||
                    (B2_ID_EQUALS(shapeB, soldierA->spearTipShapeId) && B2_ID_EQUALS(shapeA, soldierB->bodyShapeId));

                bool soldierBSpearHitSoldierABody = 
                    (B2_ID_EQUALS(shapeA, soldierB->spearTipShapeId) && B2_ID_EQUALS(shapeB, soldierA->bodyShapeId)) ||
                    (B2_ID_EQUALS(shapeB, soldierB->spearTipShapeId) && B2_ID_EQUALS(shapeA, soldierA->bodyShapeId));

                // Update the flags based on the hit detection if the speed threshold is met
                if (soldierASpearHitSoldierBBody) {
                    soldierA->hasHitTarget = true;  // Soldier A's spear hit Soldier B
                    soldierB->isHit = true;         // Soldier B is hit

                    soldierB->health -= hitEvent->approachSpeed;
                    printf("health %f\n",soldierB->health );

                } else if (soldierBSpearHitSoldierABody) {
                    soldierB->hasHitTarget = true;  // Soldier B's spear hit Soldier A
                    soldierA->isHit = true;         // Soldier A is hit

                    soldierA->health -= hitEvent->approachSpeed;
                    printf("health %f\n",soldierA->health );
                }
            }
        }
    }
}

// Main function to process all contact events
void handleContacts(b2WorldId world) {
    // Get the contact events for the current step
    b2ContactEvents contactEvents = b2World_GetContactEvents(world);

    // Call each specific handler for begin, hit, and end contacts
    handleBeginContacts(contactEvents);
    handleHitContacts(contactEvents);
    handleEndContacts(contactEvents);	

    fflush(stdout);
}
