#ifndef COLLISION_H
#define COLLISION_H

#include "box2d/box2d.h"
#include "soldier.h"
#include "id.h"

void handleContacts(b2WorldId world);
void handleTouch(b2WorldId world, Soldier* soldiers, int numSoldiers);

#endif // COLLISION_H
