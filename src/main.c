#include <stdio.h>
#include "list.h"
#include "raylib.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

//https://www.shadertoy.com/view/Ndt3Dj


typedef struct asteroid asteroid;
struct  asteroid {
    float x, y, size;
    float speed_x, speed_y, accel_x, accel_y; 
    int countdown;
};

typedef struct target target;
struct target {
    int x, y;
};

float random_number(int from, int to) {
    int r = GetRandomValue(0, 100) % 2 == 0;
    if (r) {
        return -1 * GetRandomValue(from, to);
    } else {
        return GetRandomValue(from, to);
    }
}

bool check_colision(asteroid *a, target *t) {
    return (fabs(a->x - t->x) < a->size*2 && fabs(a->y - t->y) < a->size*2);
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

    target targets[5] = {
        {0}, {0}, {0}, {0}, {0},
    };
    int target_cursor = 0;

    bool time_freezed = 0;

    list *asteroids = list_init();
    for (int i = 0; i < 50; i++) {
        asteroid *a = malloc(sizeof(asteroid));
        a->accel_x = random_number(10, 100) * 0.01;
        a->accel_y = random_number(10, 100) * 0.01;
        a->speed_x = 0;
        a->speed_y = 0;
        a->x = GetRandomValue(100, screenWidth - 100);
        a->y = GetRandomValue(100, screenHeight - 100);
        a->size = GetRandomValue(2, 16);
        a->countdown = -1;
        
        list_push(asteroids, a);
    }


    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        if (IsKeyDown(KEY_SPACE)) {
            goto skip_update;
        }
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------
        for (int i = 0; i < asteroids->size; i++) {
            asteroid *a = list_get(asteroids, i);
            
            // update position
            a->accel_x *= 0.7;
            a->accel_y *= 0.7;

            a->speed_x += a->accel_x;
            a->speed_y += a->accel_y;

            a->x += a->speed_x;
            a->y += a->speed_y;


            // bounce?
            if (a->x >= screenWidth || a->x <= 0) {
                a->speed_x *= -1;
                a->accel_x *= -1;
                a->x += a->speed_x;
            }

            if (a->y <= 0 || a->y >= screenHeight) {
                a->speed_y *= -1;
                a->accel_y *= -1;
                a->y += a->speed_y;
            }


            // colisions with targets
            for (int j = 0; j < 5; j++) {
                if (check_colision(a, &targets[j])) {
                    a->countdown = 60/2; // 500ms at 60fps    
                }
            }

            // time for destruction
            if (a->countdown != -1) {
                a->countdown--;
            }
        }

        // how to abstract this?
        // list_delete_by_fn(list *l, void (*destroy)(void *data))
        list_node *prev = NULL;
        list_node *n = asteroids->head;
        while (n) {
            asteroid *a = n->data;

            if (a->countdown == 0) {
                // split and create small parts
                if (a->size > 10) {
                    for (int i = 0; i < 4; i++) {
                        asteroid *splinter = malloc(sizeof(asteroid));
                        splinter->speed_x = a->speed_x;
                        splinter->speed_y = a->speed_y;
                        splinter->accel_x = random_number(-100, 100) * 0.01;
                        splinter->accel_y = random_number(-100, 100) * 0.01;
                        splinter->x = a->x;
                        splinter->y = a->y;
                        splinter->countdown = -1;
                        splinter->size = a->size / 4;

                        list_push(asteroids, splinter);
                    }
                }
                if (!prev) {
                    asteroids->head = n->next;
                    free(a);
                    free(n);
                    n = asteroids->head;
                } else {
                    prev->next = n->next;
                 
                    free(a);
                    free(n);

                    n = prev->next;
                }

                asteroids->size--;
                continue;
            }
            prev = n;
            n = n->next;
        }

skip_update:
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            int i = target_cursor++ % 5;
            Vector2 pos = GetMousePosition();
            targets[i].x = pos.x;
            targets[i].y = pos.y;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginTextureMode(canvas);       // Enable drawing to texture
            ClearBackground(RAYWHITE);  // Clear texture background
            DrawFPS(100, 100);

            for (int i = 0; i < asteroids->size; i++) {
                asteroid *a = list_get(asteroids, i);
                if (a->countdown >= 0) {
                    DrawRectangle(a->x - a->size*3, a->y - a->size*3, a->size*6, a->size*6, (Color){255, 200, 200, 255});   
                } else {
                    DrawRectangle(a->x - a->size*3, a->y - a->size*3, a->size*6, a->size*6, (Color){200, 200, 200, 255});   
                }
                DrawText(TextFormat("A%d", i), a->x, a->y, 4, (Color){0, 0, 0, 255});
            }

            for (int i = 0; i < 5; i++) {
                target t = targets[i];
                DrawRectangle(t.x-6, t.y-6, 12, 12, (Color){200, 0, 0, 255}); 
                DrawText(TextFormat("X%d", i), t.x, t.y, 4, (Color){0, 0, 0, 255});
            }
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
    int size = asteroids->size;
    for (int i = 0; i < size; i++) {
        asteroid *e = list_pop(asteroids);
        free(e);
    }
    free(asteroids);

    //--------------------------------------------------------------------------------------

    return 0;
}