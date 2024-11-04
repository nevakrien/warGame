#include "raylib.h"
#include "box2d/box2d.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#ifdef _OPENMP
#include <omp.h>  // OpenMP is available
#endif

#include "collision.h"
#include "team.h"
#include "soldier.h"

#define NUM_SOLDIERS_TEAM1 2400
#define NUM_SOLDIERS_TEAM2 1600
#define SPEED 10.0f
#define SOLDIER_SPACING 40.0f
#define STARTING_HEALTH 100.0f
#define LOW_HEALTH_THRESHOLD 20.0f
#define ROTATION_SPEED_CAP 2.0f  // Max angular velocity to limit physical rotation speed
#define ATTACK_ANGLE_THRESHOLD 0.1f // Angle within which a soldier can attack
//#define ANGLE_DIFF_THRESHOLD 0.01f  // Angle difference at which to stop further rotation

const Vector2 BASE_TEAM1 = { 300.0f, 300.0f };
const Vector2 BASE_TEAM2 = { 500.0f, 300.0f };

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
    b2Transform transform = b2Body_GetTransform(soldier->body);

    // Spear offset: adjust these values to position the spear tip correctly
    b2Vec2 spearOffset = {
        SOLDIER_HAND_OFFSET * transform.q.c - SOLDIER_SPEAR_LENGTH * transform.q.s,
        SOLDIER_HAND_OFFSET * transform.q.s + SOLDIER_SPEAR_LENGTH * transform.q.c
    };
    b2Vec2 spearTipPos = { soldierPos.x + spearOffset.x, soldierPos.y + spearOffset.y };

    // Calculate angle to align the spear tip towards the enemy
    float angleToEnemy = CalculateAngle(spearTipPos, enemyPos);
    float targetAngle = angleToEnemy - PI/2;

    // Get the current angle from the soldier's transform
    float currentAngle = atan2f(transform.q.s, transform.q.c);

    // Calculate the angle difference and wrap within [-PI, PI]
    float angleDiff = targetAngle - currentAngle;
    if (angleDiff > PI) angleDiff -= 2 * PI;
    if (angleDiff < -PI) angleDiff += 2 * PI;

    // Smooth rotation: proportional to angle difference, with a cap on angular velocity
    float angularVelocity = fminf(ROTATION_SPEED_CAP, fabsf(angleDiff)) * (angleDiff > 0 ? 1 : -1);
    b2Body_SetAngularVelocity(soldier->body, angularVelocity);

    // Move forward only if the spear is aligned within the attack threshold
    if (fabsf(angleDiff) < ATTACK_ANGLE_THRESHOLD) {
        float distance = b2Distance(spearTipPos, enemyPos);
        b2Vec2 velocity = {
            (enemyPos.x - spearTipPos.x) * SPEED / distance,
            (enemyPos.y - spearTipPos.y) * SPEED / distance
        };
        b2Body_SetLinearVelocity(soldier->body, velocity);
    } else {
        // Stop if the spear is not aligned correctly
        b2Body_SetLinearVelocity(soldier->body, (b2Vec2){ 0.0f, 0.0f });
    }
}

// void UpdateSoldier(Soldier* soldier, Soldier* closestEnemy) {
//     b2Vec2 enemyPos = b2Body_GetPosition(closestEnemy->body);
//     b2Vec2 soldierPos = b2Body_GetPosition(soldier->body);
//     b2Transform transform = b2Body_GetTransform(soldier->body);

//     //should be span with the transform
//     // b2Vec2 spearEnd = { soldierPos.x SOLDIER_HAND_OFFSET,soldierPos.y+SOLDIER_SPEAR_LENGTH };  // Pointing upwards in the Y direction

//     // Calculate the angle to align the front side of the soldier towards the enemy
//     float angleToEnemy = CalculateAngle(soldierPos, enemyPos);
//     float targetAngle = angleToEnemy - PI / 2;  // Offset by -90 degrees to present the front

//     // Get the current angle from the soldier's transform
//     float currentAngle = atan2f(transform.q.s, transform.q.c);

//     // Calculate the angle difference and wrap it within [-PI, PI]
//     float angleDiff = targetAngle - currentAngle;
//     if (angleDiff > PI) angleDiff -= 2 * PI;
//     if (angleDiff < -PI) angleDiff += 2 * PI;

//     // Smooth rotation: proportional to angle difference, with a cap on angular velocity
//     float angularVelocity = fminf(ROTATION_SPEED_CAP, fabsf(angleDiff)) * (angleDiff > 0 ? 1 : -1);
//     b2Body_SetAngularVelocity(soldier->body, angularVelocity);

//     // Only move if front side is facing the target within ATTACK_ANGLE_THRESHOLD
//     if (fabsf(angleDiff) < ATTACK_ANGLE_THRESHOLD) {
//         float distance = b2Distance(soldierPos, enemyPos);
//         b2Vec2 velocity = (b2Vec2){
//             (enemyPos.x - soldierPos.x) * SPEED / distance,
//             (enemyPos.y - soldierPos.y) * SPEED / distance
//         };
//         b2Body_SetLinearVelocity(soldier->body, velocity);
//     } else {
//         // Stop if not facing the target correctly
//         b2Body_SetLinearVelocity(soldier->body, (b2Vec2){ 0.0f, 0.0f });
//     }
// }



#include "knn_api.h"

// Assuming the necessary headers and Soldier structure definitions are provided

void UpdateAllSoldiers(Soldier soldiers[], int numSoldiers, Team* team1, Team* team2) {
    // Create KD-trees for each team
    KNN_Tree* team1_tree = knn_create_tree(numSoldiers);
    KNN_Tree* team2_tree = knn_create_tree(numSoldiers);

    // Populate the KD-trees with soldier positions and their IDs
    for (int i = 0; i < numSoldiers; i++) {
        Soldier* soldier = &soldiers[i];
        if (Soldier_IsAlive(soldier)) {
            b2Vec2 pos = b2Body_GetPosition(soldier->body);
            if (soldier->team == team1) {
                knn_insert_point(team1_tree, pos.x, pos.y, i);
            } else if (soldier->team == team2) {
                knn_insert_point(team2_tree, pos.x, pos.y, i);
            } else {
                printf("non existent team\n");
                exit(1);
            }
        }
    }

    // Build the KD-trees after inserting all points
    knn_build_tree(team1_tree);
    knn_build_tree(team2_tree);

    // Process each soldier to find the nearest enemy and update
    #pragma omp parallel for
    for (int i = 0; i < numSoldiers; i++) {
        Soldier* soldier = &soldiers[i];
        if (!Soldier_IsAlive(soldier)) continue;

        Soldier* closestEnemy = NULL;
        float closestDistance = FLT_MAX;
        b2Vec2 soldierPos = b2Body_GetPosition(soldier->body);

        // Choose the opposing team's KD-tree to search in
        KNN_Tree* enemy_tree = (soldier->team == team1) ? team2_tree : team1_tree;

        size_t nearest_id;
        float distance;

        // Query the nearest enemy in the opposing KD-tree
        if (knn_find_nearest(enemy_tree, soldierPos.x, soldierPos.y, &nearest_id, &distance) == 0) {
            closestEnemy = &soldiers[nearest_id];
            closestDistance = distance;
        }

        // Update soldier based on the closest enemy
        if (closestEnemy) {
            UpdateSoldier(soldier, closestEnemy);
        }
    }

    // Free both KD-trees after use
    knn_free_tree(team1_tree);
    knn_free_tree(team2_tree);
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
        if (IsKeyDown(KEY_RIGHT)) camera.target.x += 10/camera.zoom;
        if (IsKeyDown(KEY_LEFT)) camera.target.x -= 10/camera.zoom;
        if (IsKeyDown(KEY_DOWN)) camera.target.y += 10/camera.zoom;
        if (IsKeyDown(KEY_UP)) camera.target.y -= 10/camera.zoom;
        if (IsKeyDown(KEY_W)) camera.zoom += 0.05f * camera.zoom;
        if (IsKeyDown(KEY_S)) camera.zoom -= 0.05f * camera.zoom;
        if (camera.zoom < 0.00001f) camera.zoom = 0.00001f;

        // Parallelized soldier updates
        UpdateAllSoldiers(soldiers, NUM_SOLDIERS_TEAM1 + NUM_SOLDIERS_TEAM2, &team1,&team2);

        b2World_Step(world, 1.0f / 60.0f, 1);
        handleContacts(world);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(camera);

        // Draw bases
        DrawCircleV(BASE_TEAM1, 15, DARKGRAY);
        DrawCircleV(BASE_TEAM2, 15, BLUE);

        for (int i = 0; i < NUM_SOLDIERS_TEAM1 + NUM_SOLDIERS_TEAM2; i++) {
            Soldier_RenderDead(&soldiers[i]);
        }

        // Render soldiers
        for (int i = 0; i < NUM_SOLDIERS_TEAM1 + NUM_SOLDIERS_TEAM2; i++) {
            Soldier_RenderAlive(&soldiers[i]);
            Soldier_FrameReset(&soldiers[i]);
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
