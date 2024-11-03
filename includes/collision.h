#ifndef COLLISION_H
#define COLLISION_H

#include "box2d/box2d.h"


// Your declarations here
typedef enum  {
	TYPE_SOLDIER=1
} TypeID;


void handleContacts(b2WorldId world);

#endif // COLLISION_H
