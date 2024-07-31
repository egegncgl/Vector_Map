#include "raylib.h"
#include "C:\\\\Users\\\\DMAP\\\\Desktop\\\\Ege\\\\VectorMap\\\\Dependencies\\\\shpEge\\\\shapefil.h"
#include <string>
#include <iostream>
#include <vector>
using namespace std;
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{

	
	SHPHandle h = SHPOpenSHPOpen("C:\\Users\\DMAP\\Desktop\\Ege\\VectorMap\\Data\\turkey-latest-free.shp\\gis_osm_landuse_a_free_1.shp", "rb");

	if (h == nullptr)
	{
		cout << "Failed to open shp" << endl;
		return -1;
	}

	int nEntities = 0, nShapeType = 0;

	SHPGetInfo(h, &nEntities, &nShapeType, nullptr, nullptr);

	cout << nEntities << endl;
	for (int i = 0; i < nEntities; i++)
	{
		SHPObject* obj = SHPReadObject(h, i);
		if (obj == nullptr) continue;

		int& shpType = obj->nSHPType; //Shape Type (SHPT_* - see list above)
		int& shapeId = obj->nShapeId; //Shape Number (-1 is unknown/unassigned)

		int& parts = obj->nParts; //# of Parts (0 implies single part with no info)
		int* panPartStart = obj->panPartStart;  //Start Vertex of part
		int* panPartType = obj->panPartType;   //Part Type (SHPP_RING if not SHPT_MULTIPATCH)
		int& vertices = obj->nVertices; //Vertex list 

		double& xMin = obj->dfXMin; //Bounds in X, Y, Z and M dimensions
		double& yMin = obj->dfYMin;
		double& zMin = obj->dfZMin;

		double& xMax = obj->dfXMax;
		double& yMax = obj->dfYMax;
		double& zMax = obj->dfZMax;
		cout << SHPTypeName(shpType) << "," << xMin << "," << xMax << "," << yMin << "," << yMax << endl;
		cout << i << "," << vertices << "," << parts << endl;

		std::vector<int> partStart, partType;
		for (int k = 0; k < parts; k++)
		{
			partStart.push_back(panPartStart[k]);
			partType.push_back(panPartType[k]);
			//cout << panPartStart[k] << "," << SHPPartTypeName(panPartType[k]) << endl;
		}

		int part = -1;
		for (int j = 0; j < vertices; j++)
		{
			if ((part + 1) < partStart.size() and partStart[part + 1] == j)
				part++;
			int pt = -1;
			if (part >= 0)
				pt = partType[part];
			//cout << part << "," << obj->padfX[j] << "," << obj->padfY[j] << "," << obj->padfZ[j] << endl;
			//cout << SHPPartTypeName(pt) << endl;
		}

		SHPDestroyObject(obj);
	}

	SHPClose(h);


    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    float rotation = 0.0f;
    SHPHandle hSHP;
    DBFHandle hDBF;
    int numberOfEntities = 0;
    int numberOfShapeType = 0;
    int i = 0;
    hSHP = SHPOpenSHPOpen("C:\\Users\\DMAP\\Desktop\\Ege\\VectorMap\\Data\\turkey-latest-free.shp\\gis_osm_landuse_a_free_1.shp","rb");
    hDBF = DBFOpen("C:\\Users\\DMAP\\Desktop\\Ege\\VectorMap\\Data\\turkey-latest-free.shp\\gis_osm_landuse_a_free_1.dbf","rb");
    if (hSHP == NULL || hDBF == NULL) {
        printf("ShapeFile veya DBF null\n");
    }

    SHPGetInfo(hSHP, &numberOfEntities, &numberOfShapeType, NULL, NULL);
    printf("NumberOfEntities:%d\n", numberOfEntities);
    printf("NumberOfShapeType:%d\n", numberOfShapeType);

    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - basic shapes drawing");
    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        rotation += 0.2f;
        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("some basic shapes available on raylib", 20, 20, 20, DARKGRAY);
        DrawLine(18, 42, screenWidth - 18, 42, BLACK);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}