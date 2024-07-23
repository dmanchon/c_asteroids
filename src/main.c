#include <stdio.h>
#include "list.h"
#include "raylib.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

//https://www.shadertoy.com/view/Ndt3Dj

#define NOT_STARTED -1
#define NUM_TARGETS  5


typedef struct asteroid asteroid;
struct  asteroid {
    float x, y, size;
    float speed_x, speed_y, accel_x, accel_y; 
    int countdown;
};


typedef struct state state;
struct state {
    int width, height;
    Rectangle targets[NUM_TARGETS];
    Rectangle player;
    int health;
    list *asteroids;
    bool time_freezed;
    int target_cursor;
};

state *init_state(int width, int height, int num_asteroids) {
    state *s = malloc(sizeof(state));
    
    s->width = width;
    s->height = height;
    s->player = (Rectangle) {width/2, height/2, 30, 30};
    s->health = 10000;

    for (int i = 0; i < NUM_TARGETS; i++) {
        s->targets[i] = (Rectangle){width/2, height/2, 6, 6};
    }

    s->asteroids = list_init();
    for (int i = 0; i < num_asteroids; i++) {
        asteroid *a = malloc(sizeof(asteroid));
        a->accel_x = GetRandomValue(-100, 100) * 0.01;
        a->accel_y = GetRandomValue(-100, 100) * 0.01;
        a->speed_x = 0;
        a->speed_y = 0;
        a->x = GetRandomValue(100, width - 100);
        a->y = GetRandomValue(100, height - 100);
        a->size = GetRandomValue(10, 60);
        a->countdown = NOT_STARTED;
        
        list_push(s->asteroids, a);
    }
    s->time_freezed = 0;

    return s;
}

void update_state(state *s) {
    for (int i = 0; i < s->asteroids->size; i++) {
        asteroid *a = list_get(s->asteroids, i);
        
        // update position
        a->accel_x *= 0.7;
        a->accel_y *= 0.7;

        a->speed_x += a->accel_x;
        a->speed_y += a->accel_y;

        a->x += a->speed_x;
        a->y += a->speed_y;


        // bounce?
        if (a->x >= s->width || a->x <= 0) {
            a->speed_x *= -1;
            a->accel_x *= -1;
            a->x += a->speed_x;
        }

        if (a->y <= 0 || a->y >= s->height) {
            a->speed_y *= -1;
            a->accel_y *= -1;
            a->y += a->speed_y;
        }


        // colisions with targets
        Rectangle r = {a->x, a->y, a->size, a->size};

        for (int j = 0; j < 5; j++) {
            if (CheckCollisionRecs(r, s->targets[j])) {
                a->countdown = 60/2; // 500ms at 60fps    
            }
        }

        // colisions with player
        if (CheckCollisionRecs(r, s->player)) {
            s->health -= a->size;
        }

        // time for destruction
        if (a->countdown != NOT_STARTED) {
            a->countdown--;
        }
    }

    list_node *n = s->asteroids->head;
    while (n) {
        asteroid *a = n->data;

        if (a->countdown == 0) {
            // split and create small parts only if big enough
            if (a->size > 40) {
                for (int i = 0; i < 4; i++) {
                    asteroid *splinter = malloc(sizeof(asteroid));
                    splinter->speed_x = a->speed_x;
                    splinter->speed_y = a->speed_y;
                    splinter->accel_x = GetRandomValue(-100, 100) * 0.01;
                    splinter->accel_y = GetRandomValue(-100, 100) * 0.01;
                    splinter->x = a->x;
                    splinter->y = a->y;
                    splinter->countdown = -1;
                    splinter->size = a->size / 4;

                    list_push(s->asteroids, splinter);
                }
            }
            
            n = list_delete_node(s->asteroids, n);
            free(a);
        } else {
            n = n->next;
        }
    }
}

void cleanup_state(state *s) {
    int size = s->asteroids->size;
    for (int i = 0; i < size; i++) {
        asteroid *e = list_pop(s->asteroids);
        free(e);
    }
    free(s->asteroids);
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 800;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");
    RenderTexture2D canvas = LoadRenderTexture(screenWidth, screenHeight);

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second

    Shader shader = LoadShader(0, "resources/fisheye.fs");
    assert(IsShaderReady(shader));


    // init state
    state *s = init_state(screenWidth, screenHeight, 50);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        if (!IsKeyDown(KEY_SPACE)) {
            // Update state
            update_state(s);
        }

        // input
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            int i = s->target_cursor++ % 5;
            Vector2 pos = GetMousePosition();
            s->targets[i].x = pos.x;
            s->targets[i].y = pos.y;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginTextureMode(canvas);       // Enable drawing to texture
            ClearBackground(RAYWHITE);  // Clear texture background
            DrawFPS(100, 100);

            DrawRectangleRec(s->player, (Color) {0, 0, 255, 200});

            for (int i = 0; i < s->asteroids->size; i++) {
                asteroid *a = list_get(s->asteroids, i);
                Rectangle r = {a->x, a->y, a->size, a->size};
                if (a->countdown >= 0) {
                    DrawRectangleRec(r, (Color){255, 200, 200, 255});   
                } else {
                    DrawRectangleRec(r, (Color){200, 200, 200, 255});   
                }
                DrawText(TextFormat("A%d", i), a->x, a->y, 4, (Color){0, 0, 0, 255});
            }

            for (int i = 0; i < 5; i++) {
                Rectangle r = s->targets[i];
                DrawRectangleRec(r, (Color){200, 0, 0, 255}); 
                DrawText(TextFormat("X%d", i), r.x, r.y, 4, (Color){0, 0, 0, 255});
            }
            DrawText(TextFormat("Power: %d", s->health), 100, 200, 20, GREEN);

        EndTextureMode();     

        BeginDrawing();
            ClearBackground(RAYWHITE);  // Clear screen background
            // Render generated texture using selected postprocessing shader
            BeginShaderMode(shader);
                // NOTE: Render texture must be y-flipped due to default OpenGL coordinates (left-bottom)
                DrawTextureRec(canvas.texture, (Rectangle){ 0, 0, (float)canvas.texture.width, (float)-canvas.texture.height }, (Vector2){ 0, 0 }, WHITE);
            EndShaderMode();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    UnloadShader(shader);       // Unload shader
    
    //clean up
    cleanup_state(s);

    //--------------------------------------------------------------------------------------

    return 0;
}