#include "raylib.h"
#include "rlgl.h"
#include "player.h"
#include "body.h"
#include "drawing.h"
#include "collider.h"
#include <iostream>
#include "collisions.h"

int INTERNAL_WIDTH = 426;
int INTERNAL_HEIGHT = 240;

int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;

int main()
{

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    //SetConfigFlags(FLAG_FULLSCREEN_MODE);

    Color background;
    background.r = 64.0f, background.g = 152.0f, background.b = 230.0f;
    background.a = 255.0f;

    Player player = Player(2.0f);
    Collider testCollider = Collider();

    testCollider.CreatePrismatoidForward(
        player.plane.params.hitboxWidth,
        player.plane.params.hitboxHeight,
        player.plane.params.hitboxWidth / 6,
        player.plane.params.hitboxHeight,
        player.plane.params.hitboxLength);

    Collider obstacleCollider = Collider();

    obstacleCollider.CreatePrismatoidUp(10500,25,10500,25,10300);

    Vector3 obstacleColliderPos = {0,300,7000};

    Transform obstacleColliderTransform = {};
    obstacleColliderTransform.translation = obstacleColliderPos;

    Color obstacleColliderColor = GREEN;

    InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"");

    Model planeModel = LoadModel("assets/sf15b.obj");

    RenderTexture2D renderTarget = LoadRenderTexture(INTERNAL_WIDTH, INTERNAL_HEIGHT);

    Texture2D groundTexture = LoadTexture("assets/groundTest.jpeg");

    //shader

    FlatShaderData shaderData = mLoadFlatShader("shaders/flatShader.vs", "shaders/flatShader.fs", 7);

    mApplyFlatShader(&shaderData, &planeModel);

    SetTargetFPS(60);
    
    rlSetClipPlanes(10,7000);

    float accumulator = 0.0f;
    float FIXED_DELTA_TIME = 1.0f/60.0f;

    while (!WindowShouldClose())
    { 
        float dt = GetFrameTime();
        //update
        accumulator += dt;

        while (accumulator >= FIXED_DELTA_TIME)
        {
            CollisionResult r;

            int iterations = 4;

            shaderData.skipIntensity = player.GetEngineGlow();

            for(int i = 0; i < iterations; i++)
            {
                player.UpdatePlayer(FIXED_DELTA_TIME, iterations);
                
                r = SAT3D(player.GetPosition(), testCollider.GetTransformedVertices(player.GetHitboxTransform()), 
                obstacleColliderPos, obstacleCollider.GetTransformedVertices(obstacleColliderTransform));

                if (r.collision)
                {
                    obstacleColliderColor = RED;
                    std::cout<<"HIT! "<<"\n";
                    std::cout<<"HIT at: "<<player.plane.GetSpeed()<<"\n";
                }
                else
                {
                    obstacleColliderColor = GREEN;
                }
            }

            player.bulletPool.UpdateBullets(dt);

            player.Fire(dt);

            player.UpdateCamera(FIXED_DELTA_TIME);
            
            accumulator -= FIXED_DELTA_TIME;
        }

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
        rlPopMatrix();
        
        DrawFlatShadedModel(player.GetTransform(), planeModel, &shaderData);

        //DrawCollider(testCollider.GetTransformedVertices(player.GetHitboxTransform()), RED);

        //DrawSphere(player.GetTransform().translation, 2, MAGENTA);

        DrawCollider(obstacleCollider.GetTransformedVertices(obstacleColliderTransform), obstacleColliderColor);

        for(int i = 0; i < player.bulletPool.activeBullets.size(); i++)
        {
            DrawBullet(player.bulletPool.activeBullets[i]->transform);
        }

        EndMode3D();
        EndTextureMode();
        
        BeginDrawing();
        ClearBackground(background);
        
        DrawTexturePro(
            renderTarget.texture,
            {0,0,(float)INTERNAL_WIDTH,(float)-INTERNAL_HEIGHT},
            {0,0,(float)SCREEN_WIDTH, (float)SCREEN_HEIGHT}, 
            {0,0},
            0.0f, WHITE
        );

        DrawFPS(10,10);


        DrawText(TextFormat("MAX SPEED: %0.2f", player.plane.GetMaxSpeed()),SCREEN_WIDTH * 0.25, SCREEN_HEIGHT * 0.15, 20, GREEN);
        DrawText(TextFormat("IDLE SPEED: %0.2f", player.plane.GetIdleSpeed()),SCREEN_WIDTH * 0.75, SCREEN_HEIGHT * 0.15, 20, GREEN);
        DrawText(TextFormat("SPEED: %0.2f", player.plane.GetSpeed()),SCREEN_WIDTH * 0.25, SCREEN_HEIGHT * 0.5, 20, GREEN);
        DrawText(TextFormat("THRUST: %0.2f", player.plane.thrust),SCREEN_WIDTH * 0.25, SCREEN_HEIGHT * 0.25, 20, GREEN);


        DrawText(TextFormat("ALTITUDE: %0.2f", player.plane.GetSpeed()),SCREEN_WIDTH * 0.7f, SCREEN_HEIGHT / 2, 20, GREEN);


        EndDrawing();

    }
    
    std::cout<<""<<std::endl;

    UnloadShader(shaderData.__shader);
    UnloadModel(planeModel);
    UnloadTexture(groundTexture);
    UnloadRenderTexture(renderTarget);
    CloseWindow();

    return 0;
}