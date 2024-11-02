#include "soldier.h"
#include "collision.h"
#include <stddef.h>
#include <stdio.h>


// Inline function to handle begin contact events
static inline void handleBeginContacts(b2ContactEvents contactEvents) {
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
                printf("Begin contact between Soldier %p and Soldier %p\n", (void*)soldierA, (void*)soldierB);
            }
        }
    }
}

// Inline function to handle hit contact events with spear and body check
static inline void handleHitContacts(b2ContactEvents contactEvents) {
    for (int i = 0; i < contactEvents.hitCount; ++i) {
        b2ContactHitEvent* hitEvent = contactEvents.hitEvents + i;

        b2ShapeId shapeA = hitEvent->shapeIdA;
        b2ShapeId shapeB = hitEvent->shapeIdB;

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

                // Check if soldierA's spear tip collided with soldierB's body
                bool soldierASpearHitSoldierBBody = 
                    (B2_ID_EQUALS(shapeA, soldierA->spearTipShapeId) && B2_ID_EQUALS(shapeB, soldierB->bodyShapeId)) ||
                    (B2_ID_EQUALS(shapeB, soldierA->spearTipShapeId) && B2_ID_EQUALS(shapeA, soldierB->bodyShapeId));

                // Check if soldierB's spear tip collided with soldierA's body
                bool soldierBSpearHitSoldierABody = 
                    (B2_ID_EQUALS(shapeA, soldierB->spearTipShapeId) && B2_ID_EQUALS(shapeB, soldierA->bodyShapeId)) ||
                    (B2_ID_EQUALS(shapeB, soldierB->spearTipShapeId) && B2_ID_EQUALS(shapeA, soldierA->bodyShapeId));

                // Process hits
                if (soldierASpearHitSoldierBBody) {
                    soldierA->hasHitTarget = true;
                    soldierB->isHit = true;

                    if (hitEvent->approachSpeed > 10.0f) {
                        printf("High-speed impact: Soldier %p (spear) hit Soldier %p (body) with speed %.2f m/s\n", (void*)soldierA, (void*)soldierB, hitEvent->approachSpeed);
                    } else {
                        printf("Soldier %p (spear) hit Soldier %p (body)\n", (void*)soldierA, (void*)soldierB);
                    }
                } else if (soldierBSpearHitSoldierABody) {
                    soldierB->hasHitTarget = true;
                    soldierA->isHit = true;

                    if (hitEvent->approachSpeed > 10.0f) {
                        printf("High-speed impact: Soldier %p (spear) hit Soldier %p (body) with speed %.2f m/s\n", (void*)soldierB, (void*)soldierA, hitEvent->approachSpeed);
                    } else {
                        printf("Soldier %p (spear) hit Soldier %p (body)\n", (void*)soldierB, (void*)soldierA);
                    }
                }
            }
        }
    }
}

// Inline function to handle end contact events
static inline void handleEndContacts(b2ContactEvents contactEvents) {
    for (int i = 0; i < contactEvents.endCount; ++i) {
        b2ContactEndTouchEvent* endEvent = contactEvents.endEvents + i;

        b2ShapeId shapeA = endEvent->shapeIdA;
        b2ShapeId shapeB = endEvent->shapeIdB;

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
                printf("End contact between Soldier %p and Soldier %p\n", (void*)soldierA, (void*)soldierB);
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
}
