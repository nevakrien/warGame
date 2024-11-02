#include "raylib.h"
#include "box2d/box2d.h"
#include <stdio.h>

#include "soldier.h"

#define NUM_SOLDIERS 20
#define SPEED 1.0f
#define SOLDIER_SPACING 20.0f  // Spacing between soldiers to avoid initial overlap

// Function to initialize soldiers in a small clustered area with varying rotations
void InitSoldiers(Soldier soldiers[], b2WorldId world) {
    for (int i = 0; i < NUM_SOLDIERS; i++) {
        float rotation = i * (PI / NUM_SOLDIERS);  // Different starting angles in radians
        Vector2 position = {
            380.0f + (i % 5) * SOLDIER_SPACING,  // Horizontal spacing
            280.0f + (i / 5) * SOLDIER_SPACING   // Vertical spacing
        };
        soldiers[i] = Soldier_Create(world, position, rotation, GRAY);
    }
}


int main() {
    // Initialize Box2D world with zero gravity
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, 0.0f};
    b2WorldId world = b2CreateWorld(&worldDef);

    // Array of soldiers
    Soldier soldiers[NUM_SOLDIERS];
    InitSoldiers(soldiers, world);

    InitWindow(800, 600, "Soldier Simulation with Camera Control");
    SetTargetFPS(60);

    // Set up a Camera2D for moving and zooming
    Camera2D camera = { 0 };
    camera.target = (Vector2){ 400.0f, 300.0f };  // Center of screen
    camera.offset = (Vector2){ 400.0f, 300.0f };  // Render at screen center
    camera.zoom = 1.0f;

    printf("Soldiers initialized.\n");

    // Game loop
    while (!WindowShouldClose()) {
        // Camera movement
        if (IsKeyDown(KEY_RIGHT)) {
            camera.target.x += 10;
            printf("RIGHT key processed\n");
        }
        if (IsKeyDown(KEY_LEFT)) {
            camera.target.x -= 10;
            printf("LEFT key processed\n");
        }
        if (IsKeyDown(KEY_DOWN)) {
            camera.target.y += 10;
            printf("DOWN key processed\n");
        }
        if (IsKeyDown(KEY_UP)) {
            camera.target.y -= 10;
            printf("UP key processed\n");
        }

        // Zoom controls with debug prints
        if (IsKeyDown(KEY_W)) {
            camera.zoom += 0.05f * camera.zoom;
            printf("W key (zoom in) processed\n");
        }
        if (IsKeyDown(KEY_S)) {
            camera.zoom -= 0.05f * camera.zoom;
            printf("S key (zoom out) processed\n");
        }
        if (camera.zoom < 0.1f) camera.zoom = 0.1f;  // Prevent zooming out too far

        // Update soldier positions in the Box2D world
        for (int i = 0; i < NUM_SOLDIERS; i++) {
            b2Vec2 velocity = { SPEED * cosf(i * (PI / 10)), SPEED * sinf(i * (PI / 10)) };
            b2Body_SetLinearVelocity(soldiers[i].body, velocity);
        }

        // Step Box2D world to process physics
        b2World_Step(world, 1.0f / 60.0f, 1);

        // Draw everything
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(camera);

        // Render each soldier with the combined camera
        for (int i = 0; i < NUM_SOLDIERS; i++) {
            Soldier_Render(soldiers[i]);
        }

        // Draw a fixed reference circle at the center of the screen
        DrawCircle(400, 300, 10, BLUE);

        EndMode2D();

        

        // Display instructions and FPS counter
        DrawText("Use arrow keys to move the camera", 10, 10, 20, DARKGRAY);
        DrawText("Use W/S to zoom in/out", 10, 40, 20, DARKGRAY);
        DrawText("Blue circle is the center reference", 10, 70, 20, DARKGRAY);
        DrawFPS(10, 100);  // FPS counter for performance monitoring

        EndDrawing();
    }

    // Cleanup
    CloseWindow();
    return 0;
}
