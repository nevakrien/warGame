#include "soldier.h"
#include "collision.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


// Define the minimum speed threshold for triggering a hit
#define HIT_SPEED_THRESHOLD 10.0f


// Inline function to handle begin contact events with detailed logging
static inline void handleBeginContacts(b2ContactEvents  __attribute__((unused))  contactEvents) {

}



// Inline function to handle hit contact events and update soldier hit flags
static inline void handleHitContacts(b2ContactEvents contactEvents) {
    for (int i = 0; i < contactEvents.hitCount; ++i) {
        b2ContactHitEvent* hitEvent = contactEvents.hitEvents + i;
        b2ShapeId shapeA = hitEvent->shapeIdA;
        b2ShapeId shapeB = hitEvent->shapeIdB;

        // Check if the approach speed is high enough to trigger a hit
        if (hitEvent->approachSpeed < HIT_SPEED_THRESHOLD) {
            continue;  // Skip this hit event if the speed is too low
        }

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

                // Update the flags based on the hit detection if the speed threshold is met
                if (soldierASpearHitSoldierBBody) {
                    soldierA->hasHitTarget = true;  // Soldier A's spear hit Soldier B
                    soldierB->isHit = true;         // Soldier B is hit
                } else if (soldierBSpearHitSoldierABody) {
                    soldierB->hasHitTarget = true;  // Soldier B's spear hit Soldier A
                    soldierA->isHit = true;         // Soldier A is hit
                }
            }
        }
    }
}


// Inline function to handle end contact events with detailed logging
static inline void handleEndContacts(b2ContactEvents  __attribute__((unused)) contactEvents) {

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
