#include "raylib.h"
#include "C:\\\\Users\\\\DMAP\\\\Desktop\\\\Ege\\\\VectorMap\\\\Dependencies\\\\shpEge\\\\shapefil.h"
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#define DEG2LAT 72.83811
#define DEG2LON 54.13864
#define LONOFSET 1294.23832784
#define LATOFSET 3292.13689578
using namespace std;

typedef struct Polygon {
    Vector2 *verticies;
    int verticeCount;
    int shape;
}Polygon;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    
	int nEntities = 0, nShapeType = 0;

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 908;
    float rotation = 0.0f;



    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - basic shapes drawing");
    //SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        rotation += 0.2f;
        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(BLACK);
        //Draw
        DrawFPS(100, 100);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}