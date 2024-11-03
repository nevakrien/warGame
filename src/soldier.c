#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <box2d/box2d.h>

#include "soldier.h"



// Helper function to initialize the soldier's shapes
static void Soldier_Init_Phisics(Soldier *soldier, b2WorldId world, Vector2 position, float rotation) {
    // Initialize the body definition
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.userData = &soldier->id;

    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){ position.x, position.y };
    soldier->body = b2CreateBody(world, &bodyDef);

    // Common shape definition
    b2ShapeDef shapeDef = b2DefaultShapeDef();

    // Circle Shape for Soldier's Body
    b2Circle circleShape = {0};
    circleShape.radius = SOLDIER_RADIUS;
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.5f;
    shapeDef.restitution = 0.3f;
    shapeDef.enableContactEvents = true;
    shapeDef.enableHitEvents = true;

    // Create the body shape and store the shape ID
    soldier->bodyShapeId = b2CreateCircleShape(soldier->body, &shapeDef, &circleShape);

    // Adjusting spear start and end points: spear held on the right, pointing upwards
    float offsetX = SOLDIER_RADIUS * 0.5f;  // Offset to move spear to the right of the circle
    b2Vec2 spearStart = { offsetX, 0.0f };
    b2Vec2 spearEnd = { offsetX, SOLDIER_SPEAR_LENGTH };  // Pointing upwards in the Y direction
    b2Segment spearShape = { spearStart, spearEnd };
    shapeDef.density = 0.5f;
    shapeDef.enableContactEvents = false;
    shapeDef.enableHitEvents = false;


    // Create the spear shaft shape and store the shape ID
    soldier->spearShaftShapeId = b2CreateSegmentShape(soldier->body, &shapeDef, &spearShape);

    // Polygon Shape for Spear Tip
    b2Vec2 points[] = {
        { offsetX, SOLDIER_SPEAR_LENGTH },  // Base of the triangle at the end of the spear
        { offsetX - SOLDIER_SPEAR_TIP_SIZE, SOLDIER_SPEAR_LENGTH - SOLDIER_SPEAR_TIP_SIZE },
        { offsetX + SOLDIER_SPEAR_TIP_SIZE, SOLDIER_SPEAR_LENGTH - SOLDIER_SPEAR_TIP_SIZE }
    };

    b2Hull hull = b2ComputeHull(points, 3);
    b2Polygon spearTipShape = b2MakePolygon(&hull, 0.0f);  // No rounded corners
    shapeDef.density = 0.3f;
    shapeDef.enableContactEvents = true;
    shapeDef.enableHitEvents = true;

    // Create the spear tip shape and store the shape ID
    soldier->spearTipShapeId = b2CreatePolygonShape(soldier->body, &shapeDef, &spearTipShape);

    // Apply initial rotation
    b2Body_SetTransform(soldier->body, (b2Vec2){ position.x, position.y }, b2MakeRot(rotation));
}

void Soldier_Init(Soldier* soldier,b2WorldId world, Vector2 position, float rotation, Color color) {
    // Soldier soldier;
    soldier->id=TYPE_SOLDIER;
    Soldier_Init_Phisics(soldier, world, position, rotation);
    soldier->color = color;
    soldier->isHit = false;
    soldier->hasHitTarget = false;

    // return soldier;
}

// Helper function to rotate a point around a center by a given sine and cosine
static Vector2 RotatePoint(Vector2 point, Vector2 center, float s, float c) {
    // Translate point to origin
    point.x -= center.x;
    point.y -= center.y;

    // Rotate point using sine and cosine
    float newX = point.x * c - point.y * s;
    float newY = point.x * s + point.y * c;

    // Translate point back
    point.x = newX + center.x;
    point.y = newY + center.y;

    return point;
}

void Soldier_Render(Soldier* soldier) {
    // Get the soldier's transform
    b2Transform transform = b2Body_GetTransform(soldier->body);
    b2Vec2 position = transform.p;
    float s = transform.q.s;
    float c = transform.q.c;

    // Determine the color based on the soldier's state
    Color bodyColor = soldier->color;
    if (soldier->isHit) {
        bodyColor = RED;  // Change color if the soldier is hit
    }

    // Render the soldier's body
    b2Circle circle = b2Shape_GetCircle(soldier->bodyShapeId);
    DrawCircleV((Vector2){ position.x, position.y }, circle.radius, bodyColor);

    // Render the spear shaft
    b2Segment segment = b2Shape_GetSegment(soldier->spearShaftShapeId);
    Vector2 spearStart = RotatePoint(
        (Vector2){ position.x + segment.point1.x, position.y + segment.point1.y }, 
        (Vector2){ position.x, position.y }, s, c);
    Vector2 spearEnd = RotatePoint(
        (Vector2){ position.x + segment.point2.x, position.y + segment.point2.y }, 
        (Vector2){ position.x, position.y }, s, c);
    DrawLineV(spearStart, spearEnd, BLACK);

    // Render the spear tip
    b2Polygon spearTip = b2Shape_GetPolygon(soldier->spearTipShapeId);
    Vector2 v1 = RotatePoint(
        (Vector2){ position.x + spearTip.vertices[0].x, position.y + spearTip.vertices[0].y }, 
        (Vector2){ position.x, position.y }, s, c);
    Vector2 v2 = RotatePoint(
        (Vector2){ position.x + spearTip.vertices[1].x, position.y + spearTip.vertices[1].y }, 
        (Vector2){ position.x, position.y }, s, c);
    Vector2 v3 = RotatePoint(
        (Vector2){ position.x + spearTip.vertices[2].x, position.y + spearTip.vertices[2].y }, 
        (Vector2){ position.x, position.y }, s, c);

    // Ensure vertices are in counter-clockwise order
    float crossProduct = (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x);
    if (crossProduct > 0) {
        Vector2 temp = v2;
        v2 = v3;
        v3 = temp;
    }

    // Change spear tip color if it has hit a target
    Color spearColor = BLACK;
    if (soldier->hasHitTarget) {
        spearColor = GREEN;  // Indicate a successful hit
    }

    // Draw the spear tip
    DrawTriangle(v1, v2, v3, spearColor);
}

void Soldier_FrameReset(Soldier* soldier){
    soldier->isHit = false;
    soldier->hasHitTarget = false;
}