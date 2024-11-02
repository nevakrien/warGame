#ifndef SOLDIER_H
#define SOLDIER_H

#include <raylib.h>
#include <box2d/box2d.h>

#define SOLDIER_RADIUS 10.0f
#define SOLDIER_SPEAR_LENGTH 25.0f
#define SOLDIER_SPEAR_TIP_SIZE 1.0f

typedef struct {
    Color color;
    b2BodyId body;
} Soldier;

Soldier Soldier_Create(b2WorldId world, Vector2 position, float rotation, Color color);
void Soldier_Render(Soldier soldier);

#endif // SOLDIER_H
