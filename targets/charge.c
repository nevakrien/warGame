#include "raylib.h"
#include "box2d/box2d.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "collision.h"
#include "team.h"
#include "soldier.h"

#define NUM_SOLDIERS_TEAM1 12
#define NUM_SOLDIERS_TEAM2 8
#define SPEED 10.0f
#define SOLDIER_SPACING 40.0f
#define STARTING_HEALTH 100.0f
#define LOW_HEALTH_THRESHOLD 20.0f
#define ROTATION_SPEED_CAP 2.0f  // Max angular velocity to limit physical rotation speed
#define ATTACK_ANGLE_THRESHOLD 0.1f // Angle within which a soldier can attack
#define ANGLE_DIFF_THRESHOLD 0.01f  // Angle difference at which to stop further rotation

const Vector2 BASE_TEAM1 = { 200.0f, 300.0f };
const Vector2 BASE_TEAM2 = { 600.0f, 300.0f };

void InitSoldiers(Soldier soldiers[], Team* team1, Team* team2, b2WorldId world) {
    int index = 0;

    for (int i = 0; i < NUM_SOLDIERS_TEAM1; i++) {
        Vector2 position = (Vector2){ BASE_TEAM1.x, BASE_TEAM1.y + (i * SOLDIER_SPACING) };
        Soldier_Init(&soldiers[index], world, position, -PI/2.0f, team1, STARTING_HEALTH);
        index++;
    }
    
    for (int i = 0; i < NUM_SOLDIERS_TEAM2; i++) {
        Vector2 position = (Vector2){ BASE_TEAM2.x, BASE_TEAM2.y + (i * SOLDIER_SPACING) + 20.0f };
        Soldier_Init(&soldiers[index], world, position, PI/2.0f, team2, STARTING_HEALTH);
        index++;
    }
}

float CalculateAngle(b2Vec2 from, b2Vec2 to) {
    return atan2f(to.y - from.y, to.x - from.x);
}

void UpdateSoldier(Soldier* soldier, Soldier* closestEnemy) {
    b2Vec2 enemyPos = b2Body_GetPosition(closestEnemy->body);
    b2Vec2 soldierPos = b2Body_GetPosition(soldier->body);

    // Calculate the angle to align the front side of the soldier towards the enemy
    float angleToEnemy = CalculateAngle(soldierPos, enemyPos);
    float targetAngle = angleToEnemy - PI / 2;  // Offset by -90 degrees to present the front

    // Wrap the target angle within [-PI, PI] for consistent rotation direction
    if (targetAngle > PI) targetAngle -= 2 * PI;
    if (targetAngle < -PI) targetAngle += 2 * PI;

    // Get the current angle from the transform's sine and cosine components
    b2Transform transform = b2Body_GetTransform(soldier->body);
    float currentAngle = atan2f(transform.q.s, transform.q.c);

    // Calculate the angle difference and wrap it within [-PI, PI]
    float angleDiff = targetAngle - currentAngle;
    if (angleDiff > PI) angleDiff -= 2 * PI;
    if (angleDiff < -PI) angleDiff += 2 * PI;

    // Rotate smoothly towards the target angle
    if (fabsf(angleDiff) > ANGLE_DIFF_THRESHOLD) {
        // Set angular velocity proportionally, capped by ROTATION_SPEED_CAP
        float angularVelocity = fminf(ROTATION_SPEED_CAP, fabsf(angleDiff)) * (angleDiff > 0 ? 1 : -1);
        b2Body_SetAngularVelocity(soldier->body, angularVelocity);
    } else {
        // Stop rotation if within the angle threshold
        b2Body_SetAngularVelocity(soldier->body, 0.0f);
    }

    // Only move if front side is facing the target within ATTACK_ANGLE_THRESHOLD
    if (fabsf(angleDiff) < ATTACK_ANGLE_THRESHOLD) {
        float distance = b2Distance(soldierPos, enemyPos);
        b2Vec2 velocity = (b2Vec2){ (enemyPos.x - soldierPos.x) * SPEED / distance,
                                    (enemyPos.y - soldierPos.y) * SPEED / distance };
        b2Body_SetLinearVelocity(soldier->body, velocity);
    } else {
        // Stop if not facing the target correctly
        b2Body_SetLinearVelocity(soldier->body, (b2Vec2){ 0.0f, 0.0f });
    }
}



int main() {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){ 0.0f, 0.0f };
    b2WorldId world = b2CreateWorld(&worldDef);

    Soldier soldiers[NUM_SOLDIERS_TEAM1 + NUM_SOLDIERS_TEAM2];
    Team team1 = (Team){ GRAY, DARKGRAY };
    Team team2 = (Team){ BLUE, DARKGRAY };
    InitSoldiers(soldiers, &team1, &team2, world);

    InitWindow(800, 600, "Soldier Simulation with Spear Orientation Awareness");
    SetTargetFPS(60);

    Camera2D camera = { 0 };
    camera.target = (Vector2){ 400.0f, 300.0f };
    camera.offset = (Vector2){ 400.0f, 300.0f };
    camera.zoom = 1.0f;

    printf("Soldiers initialized.\n");

    while (!WindowShouldClose()) {
        // Camera controls
        if (IsKeyDown(KEY_RIGHT)) camera.target.x += 10;
        if (IsKeyDown(KEY_LEFT)) camera.target.x -= 10;
        if (IsKeyDown(KEY_DOWN)) camera.target.y += 10;
        if (IsKeyDown(KEY_UP)) camera.target.y -= 10;
        if (IsKeyDown(KEY_W)) camera.zoom += 0.05f * camera.zoom;
        if (IsKeyDown(KEY_S)) camera.zoom -= 0.05f * camera.zoom;
        if (camera.zoom < 0.1f) camera.zoom = 0.1f;

        // Update soldier AI and movement
        for (int i = 0; i < NUM_SOLDIERS_TEAM1 + NUM_SOLDIERS_TEAM2; i++) {
            if (Soldier_IsAlive(soldiers + i)) {
                Soldier* soldier = soldiers + i;

                // Find the closest enemy
                Soldier* closestEnemy = NULL;
                float closestDistance = FLT_MAX;
                for (int j = 0; j < NUM_SOLDIERS_TEAM1 + NUM_SOLDIERS_TEAM2; j++) {
                    if (soldiers[j].team != soldier->team && Soldier_IsAlive(soldiers + j)) {
                        float distance = b2Distance(b2Body_GetPosition(soldier->body), b2Body_GetPosition(soldiers[j].body));
                        if (distance < closestDistance) {
                            closestDistance = distance;
                            closestEnemy = soldiers + j;
                        }
                    }
                }

                if (closestEnemy != NULL) {
                    UpdateSoldier(soldier, closestEnemy);
                }
            }
        }

        b2World_Step(world, 1.0f / 60.0f, 1);
        handleContacts(world);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);

        // Draw bases
        DrawCircleV(BASE_TEAM1, 15, DARKGRAY);
        DrawCircleV(BASE_TEAM2, 15, BLUE);

        // Render soldiers
        for (int i = 0; i < NUM_SOLDIERS_TEAM1 + NUM_SOLDIERS_TEAM2; i++) {
            Soldier_RenderDead(soldiers + i);
            Soldier_RenderAlive(soldiers + i);
            Soldier_FrameReset(soldiers + i);
        }

        EndMode2D();

        DrawText("Use arrow keys to move the camera", 10, 10, 20, DARKGRAY);
        DrawText("Use W/S to zoom in/out", 10, 40, 20, DARKGRAY);
        DrawText("Gray and Blue circles represent each team's base", 10, 70, 20, DARKGRAY);
        DrawFPS(10, 100);

        EndDrawing();
    }

    CloseWindow();
    b2DestroyWorld(world);
    return 0;
}
