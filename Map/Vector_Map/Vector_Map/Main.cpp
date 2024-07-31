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
    SHPHandle hSHP;
    DBFHandle hDBF;
    int numberOfEntities = 0;
    int numberOfShapeType = 0;
    int i = 0;
    int j = 0;
    hSHP = SHPOpenSHPOpen("C:\\Users\\DMAP\\Desktop\\Ege\\VectorMap\\Data\\turkey-latest-free.shp\\gis_osm_landuse_a_free_1.shp","rb");
    hDBF = DBFOpen("C:\\Users\\DMAP\\Desktop\\Ege\\VectorMap\\Data\\turkey-latest-free.shp\\gis_osm_landuse_a_free_1.dbf","rb");
    if (hSHP == NULL || hDBF == NULL) {
        printf("ShapeFile veya DBF null\n");
    }
    SHPGetInfo(hSHP, &numberOfEntities, &numberOfShapeType, NULL, NULL);
    Polygon *polygons= (Polygon*)malloc(numberOfEntities * sizeof(Polygon));
    printf("NumberOfEntities:%d\n", numberOfEntities);
    printf("NumberOfShapeType:%d\n", numberOfShapeType);
    char fieldName[50];
    int fieldWidth, fieldDecimals;
    for (i = 0; i < numberOfEntities; i++) {
        SHPObject* psShape = SHPReadObject(hSHP, i);
        DBFFieldType fieldType = DBFGetFieldInfo(hDBF, i, fieldName, &fieldWidth, &fieldDecimals);
        if (psShape == NULL) {
            //printf("Shapefile Object cannot read. Index:%d\n");
            continue;
        }
        printf("Alan %d: %s (Width:%d, Decimals:%d)\n", i, fieldName, fieldWidth, fieldDecimals);
        Polygon p;
        p.verticies = (Vector2*)malloc(psShape->nVertices * sizeof(Vector2));
        p.verticeCount = psShape->nVertices;
        p.shape = psShape->nSHPType;
        //printf("EntityIndex:%d EntityNumberOfVerticies:%d\n", i, psShape->nVertices);
        //printf("ShapeType:%d\n", psShape->nSHPType);

        for (j = 0; j < psShape->nVertices; j++) {
            //printf("Lon:%lf\tLat:%lf\tAltitude:%lf\n",psShape->padfX[j], psShape->padfY[j], psShape->padfZ[j]);
            p.verticies[j].x = psShape->padfX[j];
            p.verticies[j].y = psShape->padfY[j];
            const char *fieldValue = DBFReadStringAttribute(hDBF, i, j);
            if (fieldType == FTString) {
                const char* fieldValue = DBFReadStringAttribute(hDBF, i, j);
                printf("%s\n", fieldValue);
            }
            else if (fieldType == FTInteger) {
                int fielValue = DBFReadIntegerAttribute(hDBF, i, j);
                printf("%d\n", fieldValue);
            }
            else if (fieldType == FTDouble) {
                double fieldValue = DBFReadDoubleAttribute(hDBF, i, j);
                printf("%lf\n", fieldValue);
            }
            else {
                printf("UnknownType\n");
            }
        }
        polygons[i] = p;
        SHPDestroyObject(psShape);
    }
    SHPClose(hSHP);
    DBFClose(hDBF);


    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - basic shapes drawing");
    //SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    // Main game loop
    i = 0;
    j = 0;
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
        for (i=0; i < numberOfEntities; i++) {
            for (j=0; j < polygons[i].verticeCount; j++) {
                if (j == polygons[i].verticeCount - 1) {
                    DrawLine((polygons[i].verticies[j].x*DEG2LON)- LONOFSET, LATOFSET - polygons[i].verticies[j].y*DEG2LAT,
                        (polygons[i].verticies[0].x * DEG2LON)-LONOFSET, LATOFSET - polygons[i].verticies[0].y * DEG2LAT, RED);
                }
                else {
                    DrawLine((polygons[i].verticies[j].x * DEG2LON) - LONOFSET,LATOFSET- polygons[i].verticies[j].y * DEG2LAT,
                        (polygons[i].verticies[j + 1].x * DEG2LON)-LONOFSET, LATOFSET - polygons[i].verticies[j].y * DEG2LAT, RED);
                }
            }
        }
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