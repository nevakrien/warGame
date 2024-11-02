#include <stdio.h>
#include <stdlib.h>
#include <box2d/box2d.h>

#include "soldier.h"


static void Soldier_Init(Soldier *soldier, b2WorldId world, Vector2 position, float rotation) {
    // Initialize the body definition
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    // bodyDef.fixedRotation = 1;
    bodyDef.position = (b2Vec2){ position.x, position.y };

    // Create the body in the Box2D world
    soldier->body = b2CreateBody(world, &bodyDef);

    // Circle Shape for Body
    b2Circle circleShape = {0};
    circleShape.radius = SOLDIER_RADIUS;
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    b2CreateCircleShape(soldier->body, &shapeDef, &circleShape);

    // Segment Shape for Spear Shaft
    b2Vec2 spearStart = { 0.0f, 0.0f };
    b2Vec2 spearEnd = { SOLDIER_SPEAR_LENGTH, 0.0f };
    b2Segment spearShape = { spearStart, spearEnd };
    b2CreateSegmentShape(soldier->body, &shapeDef, &spearShape);

    // Corrected Points for the triangular spear tip
    b2Vec2 points[] = {
        { SOLDIER_SPEAR_LENGTH, 0.0f },  // Base of the triangle at the end of the spear
        { SOLDIER_SPEAR_LENGTH - SOLDIER_SPEAR_TIP_SIZE, SOLDIER_SPEAR_TIP_SIZE },
        { SOLDIER_SPEAR_LENGTH - SOLDIER_SPEAR_TIP_SIZE, -SOLDIER_SPEAR_TIP_SIZE }
    };

    // Compute the hull of the points to create a triangle shape
    b2Hull hull = b2ComputeHull(points, 3);
    float radius = SOLDIER_SPEAR_LENGTH;  // Set to zero to avoid rounded corners
    b2Polygon spearTipShape = b2MakePolygon(&hull, radius);
    b2CreatePolygonShape(soldier->body, &shapeDef, &spearTipShape);

    // Apply initial rotation to the body
    b2Body_SetTransform(soldier->body, (b2Vec2){ position.x, position.y }, b2MakeRot(rotation));
}




Soldier Soldier_Create(b2WorldId world, Vector2 position, float rotation, Color color) {
    Soldier soldier;
    Soldier_Init(&soldier, world, position, rotation);
    soldier.color = color;
    return soldier;
}

#include <math.h>

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

void Soldier_Render(Soldier soldier) {
    // Get the soldier's position and rotation from Box2D
    b2Transform transform = b2Body_GetTransform(soldier.body);
    b2Vec2 b2Position = transform.p;
    Vector2 position = { b2Position.x, b2Position.y };

    // Retrieve the sine and cosine of the rotation directly from the transform's rotation component
    float s = transform.q.s;  // Sine of the angle
    float c = transform.q.c;  // Cosine of the angle

    // Fetch shapes attached to the body
    b2ShapeId shapeIds[3];  // Allocate space for up to 3 shapes
    int shapeCount = b2Body_GetShapes(soldier.body, shapeIds, 3);  // Get actual number of shapes

    for (int i = 0; i < shapeCount; ++i) {
        b2ShapeId shapeId = shapeIds[i];
        b2ShapeType shapeType = b2Shape_GetType(shapeId);

        if (shapeType == b2_circleShape) {
            // Render the body circle shape
            b2Circle circle = b2Shape_GetCircle(shapeId);
            DrawCircleV(position, circle.radius, soldier.color);

        } else if (shapeType == b2_segmentShape) {
            // Render the spear shaft using the segment's start and end points, with rotation
            b2Segment segment = b2Shape_GetSegment(shapeId);
            Vector2 spearStart = RotatePoint(
                (Vector2){ position.x + segment.point1.x, position.y + segment.point1.y }, position, s, c);
            Vector2 spearEnd = RotatePoint(
                (Vector2){ position.x + segment.point2.x, position.y + segment.point2.y }, position, s, c);
            DrawLineV(spearStart, spearEnd, BLACK);

        } else if (shapeType == b2_polygonShape) {
            // Render the spear tip as a triangle using the vertices in counter-clockwise order
            b2Polygon triangle = b2Shape_GetPolygon(shapeId);

            if (triangle.count == 3) {  // Ensure it has exactly 3 vertices
                Vector2 v1 = RotatePoint(
                    (Vector2){ position.x + triangle.vertices[0].x, position.y + triangle.vertices[0].y }, position, s, c);
                Vector2 v2 = RotatePoint(
                    (Vector2){ position.x + triangle.vertices[1].x, position.y + triangle.vertices[1].y }, position, s, c);
                Vector2 v3 = RotatePoint(
                    (Vector2){ position.x + triangle.vertices[2].x, position.y + triangle.vertices[2].y }, position, s, c);

                // Calculate cross product to determine if vertices are in counter-clockwise order
                float crossProduct = (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x);

                // If crossProduct is negative, swap v2 and v3 to make the vertices counter-clockwise
                if (crossProduct > 0) {
                    Vector2 temp = v2;
                    v2 = v3;
                    v3 = temp;
                }

                // Draw the triangle with vertices in counter-clockwise order
                DrawTriangle(v1, v2, v3, BLACK);
            } else {
                printf("Unexpected vertex count (%d) for polygon shape.\n", triangle.count);
                exit(1);
            }
        }
    }
}


