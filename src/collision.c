#include "soldier.h"
#include "collision.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


// Helper function to get the shape type as a string
static inline const char* getShapeType(Soldier* soldier, b2ShapeId shapeId) {
    if (B2_ID_EQUALS(shapeId, soldier->bodyShapeId)) {
        return "Body";
    } else if (B2_ID_EQUALS(shapeId, soldier->spearShaftShapeId)) {
        return "Spear Shaft";
    } else if (B2_ID_EQUALS(shapeId, soldier->spearTipShapeId)) {
        return "Spear Tip";
    } else {
        return "Unknown Shape";
    }
}

// Inline function to handle begin contact events with detailed logging
static inline void handleBeginContacts(b2ContactEvents contactEvents) {
    for (int i = 0; i < contactEvents.beginCount; ++i) {
        printf("Processing begin contact event %d...\n", i + 1);

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

                // Get shape types for detailed logging
                const char* shapeTypeA = getShapeType(soldierA, shapeA);
                const char* shapeTypeB = getShapeType(soldierB, shapeB);

                printf("Begin Contact (%d): A: %s, B: %s\n", i + 1, shapeTypeA, shapeTypeB);
            } else {
                printf("Begin contact event %d: Unexpected type ID(s): A (%d), B (%d)\n", i + 1, typeA, typeB);
                exit(1);
            }
        } else {
            printf("Begin contact event %d has null user data\n", i + 1);
        }
    }
}

// Inline function to handle hit contact events and update soldier hit flags
static inline void handleHitContacts(b2ContactEvents contactEvents) {
    if (contactEvents.hitCount == 0) {
        printf("No hit contacts detected.\n");
    }

    for (int i = 0; i < contactEvents.hitCount; ++i) {
        printf("Processing hit contact event %d...\n", i + 1);

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

                // Determine if it's a spear-tip-to-body hit
                bool soldierASpearHitSoldierBBody = 
                    (B2_ID_EQUALS(shapeA, soldierA->spearTipShapeId) && B2_ID_EQUALS(shapeB, soldierB->bodyShapeId)) ||
                    (B2_ID_EQUALS(shapeB, soldierA->spearTipShapeId) && B2_ID_EQUALS(shapeA, soldierB->bodyShapeId));

                bool soldierBSpearHitSoldierABody = 
                    (B2_ID_EQUALS(shapeA, soldierB->spearTipShapeId) && B2_ID_EQUALS(shapeB, soldierA->bodyShapeId)) ||
                    (B2_ID_EQUALS(shapeB, soldierB->spearTipShapeId) && B2_ID_EQUALS(shapeA, soldierA->bodyShapeId));

                // Update the flags based on the hit detection
                if (soldierASpearHitSoldierBBody) {
                    soldierA->hasHitTarget = true;  // Soldier A's spear hit Soldier B
                    soldierB->isHit = true;         // Soldier B is hit
                    printf("Soldier %p spear hit Soldier %p body.\n", (void*)soldierA, (void*)soldierB);
                } else if (soldierBSpearHitSoldierABody) {
                    soldierB->hasHitTarget = true;  // Soldier B's spear hit Soldier A
                    soldierA->isHit = true;         // Soldier A is hit
                    printf("Soldier %p spear hit Soldier %p body.\n", (void*)soldierB, (void*)soldierA);
                } else {
                    // Handle cases where it's a hit but not a spear-to-body collision
                    printf("Hit detected but not spear-to-body collision.\n");
                }
            } else {
                printf("Hit contact event %d: Unexpected type ID(s): A (%d), B (%d)\n", i + 1, typeA, typeB);
            }
        } else {
            printf("Hit contact event %d has null user data\n", i + 1);
        }
    }
}


// Inline function to handle end contact events with detailed logging
static inline void handleEndContacts(b2ContactEvents contactEvents) {
    for (int i = 0; i < contactEvents.endCount; ++i) {
        printf("Processing end contact event %d...\n", i + 1);

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

                // Get shape types for detailed logging
                const char* shapeTypeA = getShapeType(soldierA, shapeA);
                const char* shapeTypeB = getShapeType(soldierB, shapeB);

                printf("End Contact (%d): A: %s, B: %s\n", i + 1, shapeTypeA, shapeTypeB);
            } else {
                printf("End contact event %d: Unexpected type ID(s): A (%d), B (%d)\n", i + 1, typeA, typeB);
                exit(1);

            }
        } else {
            printf("End contact event %d has null user data\n", i + 1);
        }
    }
}

// Main function to process all contact events
void handleContacts(b2WorldId world) {
    // Get the contact events for the current step
    b2ContactEvents contactEvents = b2World_GetContactEvents(world);

    printf("Begin Count: %d, Hit Count: %d, End Count: %d\n",
           contactEvents.beginCount, contactEvents.hitCount, contactEvents.endCount);

    // Call each specific handler for begin, hit, and end contacts
    handleBeginContacts(contactEvents);
    handleHitContacts(contactEvents);
    handleEndContacts(contactEvents);

    fflush(stdout);
}
