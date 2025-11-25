#include "raylib.h"
#include "rlgl.h"
#include "player.h"
#include "body.h"
#include <iostream>

int INTERNAL_WIDTH = 426;
int INTERNAL_HEIGHT = 240;

int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;


int main()
{
    Vector2 origin;
    origin.x = 0;
    origin.y = 0;

    //SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    //SetConfigFlags(FLAG_FULLSCREEN_MODE);

    Color background;
    background.r = 64.0f, background.g = 152.0f, background.b = 230.0f;
    background.a = 255.0f;

    Player player = Player();

    InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"");

    Model planeModel = LoadModel("assets/sf15b.obj");

    RenderTexture2D renderTarget = LoadRenderTexture(INTERNAL_WIDTH, INTERNAL_HEIGHT);

    Texture2D groundTexture = LoadTexture("assets/groundTest.jpeg");    

    SetTargetFPS(60);
    
    rlSetClipPlanes(10,7000);

    while (!WindowShouldClose())
    {
        //update

        float dt = GetFrameTime();

        player.UpdatePlayer(dt,1);
        player.UpdateCamera(dt);

        //drawing
        BeginTextureMode(renderTarget);

        ClearBackground(background);

        BeginMode3D(player.camera);

        DrawPlane({0,0,0},{1000,1000},GREEN);

        rlPushMatrix();
        rlRotatef(90,1,0,0);
        int scale = 100;
        Vector2 grpos;
        grpos.x = -(groundTexture.width / 2) * scale;
        grpos.y = -(groundTexture.height / 2) * scale; 
        DrawTextureEx(groundTexture, grpos, 0.0f,scale,WHITE);
        rlPopMatrix();
        
        rlPushMatrix();
        rlScalef(-1,-1,-1);
        DrawText("TEST", 200,-90,100,RED);
        rlPopMatrix();
        
        rlPushMatrix();
        const float* m = GetLocalMatrixTransform(player.GetTransform()).data();
        rlMultMatrixf(m);
        DrawModel(planeModel, {0,0,0}, 2, WHITE);
        rlPopMatrix();

        EndMode3D();
        EndTextureMode();
        
        BeginDrawing();
        ClearBackground(background);

        Rectangle source;
        source.x = 0, source.y = 0, source.width = INTERNAL_WIDTH, source.height = -INTERNAL_HEIGHT;

        Rectangle dest;
        dest.x = 0, dest.y = 0, dest.width = SCREEN_WIDTH, dest.height = SCREEN_HEIGHT;
        
        DrawTexturePro(
            renderTarget.texture,
            source, dest, origin,
            0.0f, WHITE
        );

        DrawFPS(10,10);

        DrawText(TextFormat("%0.2f", player.plane.GetSpeed()),SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, 20, GREEN);


        EndDrawing();

    }
    
    std::cout<<""<<std::endl;

    UnloadModel(planeModel);
    UnloadTexture(groundTexture);
    UnloadRenderTexture(renderTarget);
    CloseWindow();

    return 0;
}