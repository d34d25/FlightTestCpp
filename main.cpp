#include "raylib.h"
#include "rlgl.h"
#include "player.h"
#include "body.h"
#include "drawing.h"
#include "collider.h"
#include <iostream>
#include "collisions.h"
#include "target.h"

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

    Player player = Player();
    Collider testCollider = Collider();

    //thinnest thickness of a collider is 2
    //best n for 2x speed is 35, for x1 speed is 18 or 20
    testCollider.CreatePrismatoidForward(
        player.params.hitboxWidth,
        player.params.hitboxHeight,
        player.params.hitboxWidth /6,
        player.params.hitboxHeight,
        player.params.hitboxLength
    );
    
    Color testColliderColor = WHITE;

    float testColliderRadius = 2.0f;

    //test obstacle
    Collider obstacleCollider = Collider();

    obstacleCollider.CreatePrismatoidUp(500,2,50,2,50000);

    Transform obstacleColliderTransform = {};
    obstacleColliderTransform.translation = {0,300,7000};

    Color ogObstacleColliderColor = {100,100,100,255};

    Color obstacleColliderColor = ogObstacleColliderColor;

    Target tgt1 = Target({100,4,400}, 4,4,4,100,EnemyColliderType::BOX,EnemyType::AA_GUN);
    Target tgt2 = Target({500,4,400}, 4,4,4,100,EnemyColliderType::BOX,EnemyType::AA_GUN);
    Target tgt3 = Target({0,4,400}, 4,4,4,100,EnemyColliderType::BOX,EnemyType::AA_GUN);
    Target tgt4 = Target({0,4,900}, 4,4,4,100,EnemyColliderType::BOX,EnemyType::SAM);

    std::vector<Target*> enemyList;

    enemyList.push_back(&tgt1);
    enemyList.push_back(&tgt2);
    enemyList.push_back(&tgt3);
    enemyList.push_back(&tgt4);

    for(int i = 0; i < enemyList.size(); i++)
    {
        std::cout<<"Enemy address init: "<<enemyList[i]<<"\n";
        player.targets.push_back(enemyList[i]);
    }

    InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"");

    Model planeModel = LoadModel(player.params.modelPath);

    RenderTexture2D renderTarget = LoadRenderTexture(INTERNAL_WIDTH, INTERNAL_HEIGHT);

    Texture2D groundTexture = LoadTexture("assets/groundTest.jpeg");

    int groundsize = 100000;
    Model ground = GenerateGroundMesh(groundTexture, groundsize,groundsize,20);

    //shader

    FlatShaderData shaderData = mLoadFlatShader("shaders/flatShader.vs", "shaders/flatShader.fs", player.params.skipMaterialIndex);

    mApplyFlatShader(&shaderData, &planeModel);

    SetTargetFPS(60);
    
    rlSetClipPlanes(10,10000);

    float accumulator = 0.0f;
    float FIXED_DELTA_TIME = 1.0f/60.0f;

    Vector3 obstacleVel = {0,0,0};

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        //update
        accumulator += dt;

        while (accumulator >= FIXED_DELTA_TIME)
        {
            shaderData.skipIntensity = player.GetEngineGlow();

            bool r = SAT3DPoly_CCD(
                testCollider, player.GetHitboxTransform(), player.body.GetAbsoluteVelocity(),
                obstacleCollider, obstacleColliderTransform, obstacleVel, FIXED_DELTA_TIME
            );

            /*bool r = PolyVsSphere_CCD(
                obstacleCollider, obstacleColliderTransform, obstacleVel,
                player.GetPosition(), testColliderRadius, player.body.GetAbsoluteVelocity(),
                FIXED_DELTA_TIME
            );*/

            if (r)
            {
                std::cout<<"PLAYER HIT at: "<<player.GetAbsoluteSpeed()<<"\n";
                //player.body.transform.translation = {0,400,-700};

                //obstacleColliderTransform.translation = {0,300,7000};

                obstacleColliderColor = ORANGE;
                testColliderColor = RED;
            }
            else
            {
                obstacleColliderColor = GRAY;
                testColliderColor = WHITE;
            }

            obstacleColliderTransform.translation += obstacleVel * FIXED_DELTA_TIME;
            player.UpdatePlayer(FIXED_DELTA_TIME);

            for (int b = 0; b < player.bulletPool.activeBullets.size(); b++)
            {
                Bullet* currentBullet = player.bulletPool.activeBullets[b];

                bool rb = PolyVsSphere_CCD(
                    obstacleCollider,
                    obstacleColliderTransform,
                    obstacleVel,
                    currentBullet->transform.translation,
                    currentBullet->radius, 
                    currentBullet->linearVelocity,
                    FIXED_DELTA_TIME
                );
                        
                if(rb)
                {
                    std::cout<<"BULLET HIT at: "<<Vector3Length(currentBullet->linearVelocity)<<"\n";
                    currentBullet->didHit = true;
                }
            }

            player.bulletPool.UpdateBullets(FIXED_DELTA_TIME);

            for (int m = 0; m < player.missilePoolA.activeMissiles.size(); m++)
            {
                Missile* currentMissile = player.missilePoolA.activeMissiles[m];

                bool rm = PolyVsSphere_CCD(
                    obstacleCollider,
                    obstacleColliderTransform, obstacleVel,
                    currentMissile->body.transform.translation,
                    currentMissile->radius, currentMissile->body.GetAbsoluteVelocity(), 
                    FIXED_DELTA_TIME
                );

                if(rm)
                {
                    std::cout<<"MISSILE A HIT at: "<<Vector3Length(currentMissile->body.linearVelocity)<<"\n";
                    currentMissile->didHit = true;
                }
            }

            for (int m = 0; m < player.missilePoolB.activeMissiles.size(); m++)
            {
                Missile* currentMissile = player.missilePoolB.activeMissiles[m];

                bool rm = PolyVsSphere_CCD(
                    obstacleCollider,
                    obstacleColliderTransform, obstacleVel,
                    currentMissile->body.transform.translation,
                    currentMissile->radius, currentMissile->body.GetAbsoluteVelocity(), 
                    FIXED_DELTA_TIME
                );

                if(rm)
                {
                    std::cout<<"MISSILE B HIT at: "<<Vector3Length(currentMissile->body.linearVelocity)<<"\n";
                    currentMissile->didHit = true;
                }
            }

            for (int m = 0; m < player.missilePoolA.activeMissiles.size(); m++)
            {
                Missile* currentMissile = player.missilePoolA.activeMissiles[m];

                for(int e = 0; e < enemyList.size(); e++)
                {

                    bool rm = PolyVsSphere_CCD(
                        enemyList[e]->hitbox,
                        enemyList[e]->body.transform,
                        enemyList[e]->body.GetAbsoluteVelocity(),
                        currentMissile->body.transform.translation, 
                        currentMissile->radius,
                        currentMissile->body.GetAbsoluteVelocity(),
                        FIXED_DELTA_TIME
                    );

                    if(rm)
                    {
                        std::cout<<"MISSILE HIT ENEMY at: "<<Vector3Length(currentMissile->body.linearVelocity)<<"\n";
                        currentMissile->didHit = true;
                    }
                }
            }

            for (int m = 0; m < player.missilePoolB.activeMissiles.size(); m++)
            {
                Missile* currentMissile = player.missilePoolB.activeMissiles[m];

                for(int e = 0; e < enemyList.size(); e++)
                {
            
                    bool rm = PolyVsSphere_CCD(
                        enemyList[e]->hitbox,
                        enemyList[e]->body.transform,
                        enemyList[e]->body.GetAbsoluteVelocity(),
                        currentMissile->body.transform.translation, 
                        currentMissile->radius,
                        currentMissile->body.GetAbsoluteVelocity(),
                        FIXED_DELTA_TIME
                    );

                    if(rm)
                    {
                        std::cout<<"MISSILE HIT ENEMY at: "<<Vector3Length(currentMissile->body.linearVelocity)<<"\n";
                        currentMissile->didHit = true;
                    }
                }
            }

            player.missilePoolA.UpdateMissiles(FIXED_DELTA_TIME);
            player.missilePoolB.UpdateMissiles(FIXED_DELTA_TIME);

            for (int a = 0; a < enemyList.size(); a++)
            {
                enemyList[a]->UpdateEnemy(
                    FIXED_DELTA_TIME, 
                    player.GetPosition(),
                    player.missilePoolA.activeMissiles, player.missilePoolB.activeMissiles
                );
            }

            for (int a = 0; a < enemyList.size(); a++)
            {
                enemyList[a]->FireBullet(FIXED_DELTA_TIME, player.GetPosition(), player.body.GetTrueLinearVelocity());
                enemyList[a]->FireMissile(FIXED_DELTA_TIME, player.GetPosition());
            }

            player.ChooseTarget();
            player.FireB(FIXED_DELTA_TIME);
            player.FireM(FIXED_DELTA_TIME);

            player.UpdateCamera(FIXED_DELTA_TIME);
            
            accumulator -= FIXED_DELTA_TIME;
        }

        //drawing
        BeginTextureMode(renderTarget);

        ClearBackground(background);

        BeginMode3D(player.camera);
        
        rlPushMatrix();
        rlScalef(-1,-1,-1);
        rlPopMatrix();
        
        DrawFlatShadedModel(player.GetTransform(), planeModel, &shaderData);

        //DrawColliderWire(testCollider,player.GetHitboxTransform(), RED);
        //DrawCollider(testCollider, player.GetHitboxTransform(), testColliderColor);
        //DrawColliderFaceNormals(testCollider, player.GetHitboxTransform(), 10);

        //DrawSphere(player.GetTransform().translation, testColliderRadius, testColliderColor);

        for(int i = 0; i < player.bulletPool.activeBullets.size(); i++)
        {
            Bullet* currentBullet = player.bulletPool.activeBullets[i];

            DrawBullet(currentBullet->transform, currentBullet->radius, BULLET_YELLOW);
        }        

        DrawGround(ground);
        
        DrawCollider(obstacleCollider, obstacleColliderTransform, obstacleColliderColor);
        DrawColliderWire(obstacleCollider,obstacleColliderTransform, BLACK);
        
        DrawColliderFaceNormals(obstacleCollider,obstacleColliderTransform, 10);

        for(int i = 0; i < player.missilePoolA.activeMissiles.size(); i++)
        {
            Missile* currentMissile = player.missilePoolA.activeMissiles[i];

            DrawMissile(currentMissile->body.transform, currentMissile->radius);

            //DrawSphere(currentMissile->body.transform.translation, currentMissile->radius, RED);

            for(int j = 0; j < currentMissile->particlePool.activeParticles.size(); j++)
            {
                Particle* currentParticle = currentMissile->particlePool.activeParticles[j];

                RotateTowardsCamera(&currentParticle->transform, player.camera);
                DrawCircleRotated3D(currentParticle->transform, currentParticle->radius, {255,255,255,static_cast<unsigned char>(currentParticle->alpha)});
            }
        }

        for(int i = 0; i < player.missilePoolB.activeMissiles.size(); i++)
        {
            Missile* currentMissile = player.missilePoolB.activeMissiles[i];

            DrawMissile(currentMissile->body.transform, currentMissile->radius);

            //DrawSphere(currentMissile->body.transform.translation, currentMissile->radius, RED);

            for(int j = 0; j < currentMissile->particlePool.activeParticles.size(); j++)
            {
                Particle* currentParticle = currentMissile->particlePool.activeParticles[j];

                RotateTowardsCamera(&currentParticle->transform, player.camera);
                DrawCircleRotated3D(currentParticle->transform, currentParticle->radius, {255,255,255,static_cast<unsigned char>(currentParticle->alpha)});
            }
        }

        for (int i = 0; i < enemyList.size(); i++)
        {
            DrawTgt(*enemyList[i]);

            for(int j = 0; j < enemyList[i]->bulletPool.activeBullets.size(); j++)
            {
                Bullet* currentBullet = enemyList[i]->bulletPool.activeBullets[j];

                DrawBullet(currentBullet->transform, currentBullet->radius, BULLET_YELLOW);

                //DrawSphere(currentBullet->transform.translation,currentBullet->radius, BULLET_YELLOW);
            }

            for (int j = 0; j < enemyList[i]->missilePool.activeMissiles.size(); j++)
            {
                Missile* currentMissile = enemyList[i]->missilePool.activeMissiles[j];

                DrawMissile(currentMissile->body.transform, currentMissile->radius);

                for(int p = 0; p < currentMissile->particlePool.activeParticles.size(); p++)
                {
                    Particle* currentParticle = currentMissile->particlePool.activeParticles[p];

                    RotateTowardsCamera(&currentParticle->transform, player.camera);
                    DrawCircleRotated3D(currentParticle->transform, currentParticle->radius, {255,255,255,static_cast<unsigned char>(currentParticle->alpha)});
                }
            }
        }

        if(player.currentTarget)
        {
            DrawLine3D(player.GetPosition(), player.currentTarget->body.transform.translation, RED);
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


        DrawText(TextFormat("MAX SPEED: %0.2f", player.GetMaxSpeed()),SCREEN_WIDTH * 0.25, SCREEN_HEIGHT * 0.15, 20, GREEN);
        DrawText(TextFormat("IDLE SPEED: %0.2f", player.GetIdleSpeed()),SCREEN_WIDTH * 0.75, SCREEN_HEIGHT * 0.15, 20, GREEN);
        DrawText(TextFormat("TRUE SPEED: %0.2f", player.GetTrueLinearSpeed()),SCREEN_WIDTH * 0.25, SCREEN_HEIGHT * 0.5, 20, GREEN);
        //DrawText(TextFormat("DISPLAY SPEED: %0.2f", player.GetTrueSpeed() * 2.0f),SCREEN_WIDTH * 0.25, SCREEN_HEIGHT * 0.65, 20, GREEN);
        DrawText(TextFormat("THRUST: %0.2f", player.thrust),SCREEN_WIDTH * 0.25, SCREEN_HEIGHT * 0.25, 20, GREEN);


        DrawText(TextFormat("ALTITUDE: %0.2f", player.GetPosition().y),SCREEN_WIDTH * 0.7f, SCREEN_HEIGHT / 2, 20, GREEN);


        EndDrawing();

    }
    
    std::cout<<""<<std::endl;

    UnloadShader(shaderData.__shader);
    UnloadModel(planeModel);
    UnloadModel(ground);
    UnloadTexture(groundTexture);
    UnloadRenderTexture(renderTarget);
    CloseWindow();

    return 0;
}


/*

when creating the scene system, before drawing create a temp list of the objects to be drawn
out of the active ones

*/