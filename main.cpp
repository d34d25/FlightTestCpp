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

    //the parameters passed are the half size, so the height is actually 4
    testCollider.CreatePrismatoidForward(
        player.params.hitboxWidth,
        player.params.hitboxHeight,
        player.params.hitboxWidth / 6,
        player.params.hitboxHeight,
        player.params.hitboxLength);

    //test obstacle
    Collider obstacleCollider = Collider();

    obstacleCollider.CreatePrismatoidUp(10500,4,10500,4,10300);

    Vector3 obstacleColliderPos = {0,300,7000};

    Transform obstacleColliderTransform = {};
    obstacleColliderTransform.translation = obstacleColliderPos;

    Color ogObstacleColliderColor = {100,100,100,255};

    Color obstacleColliderColor = ogObstacleColliderColor;

    // test targets
    Target tgt1 = CreateTarget({0,100,400}, 10,10,10, 100, EnemyColliderType::BOX);
    
    Target tgt2 = CreateTarget({500,0,400}, 10,10,10, 100, EnemyColliderType::BOX);

    Target tgt3 = CreateTarget({0,600,400}, 10,10,10, 100, EnemyColliderType::PRISMATOID_FORWARD);

    Target tgt4 = CreateTarget({0,0,900}, 10,10,10, 100, EnemyColliderType::PRISMATOID_UP);

    Enemy aatank1 = Enemy(tgt1, EnemyType::AA_TANK);
    Enemy aatank2 = Enemy(tgt2, EnemyType::AA_TANK);
    Enemy aatank3 = Enemy(tgt3, EnemyType::AA_TANK);
    Enemy aatank4 = Enemy(tgt4, EnemyType::AA_TANK);

    std::vector<std::unique_ptr<Enemy>> enemyList;

    enemyList.push_back(std::make_unique<Enemy>(tgt1, EnemyType::AA_TANK));
    enemyList.push_back(std::make_unique<Enemy>(tgt2, EnemyType::AA_TANK));
    enemyList.push_back(std::make_unique<Enemy>(tgt3, EnemyType::AA_TANK));
    enemyList.push_back(std::make_unique<Enemy>(tgt4, EnemyType::AA_TANK));

    for(int i = 0; i < enemyList.size(); i++)
    {
        player.targets.push_back(std::make_shared<Target>(enemyList[i]->target));
    }

    InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"");

    Model planeModel = LoadModel(player.params.modelPath);

    RenderTexture2D renderTarget = LoadRenderTexture(INTERNAL_WIDTH, INTERNAL_HEIGHT);

    Texture2D groundTexture = LoadTexture("assets/groundTest.jpeg");

    //shader

    FlatShaderData shaderData = mLoadFlatShader("shaders/flatShader.vs", "shaders/flatShader.fs", player.params.skipMaterialIndex);

    mApplyFlatShader(&shaderData, &planeModel);

    SetTargetFPS(60);
    
    rlSetClipPlanes(10,10000);

    float accumulator = 0.0f;
    float FIXED_DELTA_TIME = 1.0f/60.0f;

    int iterations = 10;

    if (iterations < 1)
    {
        std::cerr<<"ITERATIONS CAN'T BE LESS THAN 1"<< std::endl;
        return 0;
    }

    while (!WindowShouldClose())
    { 
        
        float dt = GetFrameTime();
        //update
        accumulator += dt;

        while (accumulator >= FIXED_DELTA_TIME)
        {
            CollisionResult r;

            shaderData.skipIntensity = player.GetEngineGlow();

            for(int i = 0; i < iterations; i++)
            {
                player.UpdatePlayer(FIXED_DELTA_TIME, iterations);
                
                r = SAT3DPrism(player.GetPosition(), testCollider.GetTransformedVertices(player.GetHitboxTransform()), 
                obstacleColliderPos, obstacleCollider.GetTransformedVertices(obstacleColliderTransform));

                for (int b = 0; b < player.bulletPool.activeBullets.size(); b++)
                {
                    Bullet* currentBullet = player.bulletPool.activeBullets[b];
                    
                    CollisionResult rb;

                    rb = PrismVsSphere(obstacleColliderPos,obstacleCollider.GetTransformedVertices(obstacleColliderTransform),
                    currentBullet->transform.translation, currentBullet->radius);
                    
                    if(rb.collision)
                    {
                        std::cout<<"BULLET HIT at: "<<Vector3Length(currentBullet->linearVelocity)<<"\n";
                        currentBullet->didHit = true;
                    }
                }

                if (r.collision)
                {
                    std::cout<<"PLAYER HIT at: "<<player.GetSpeed()<<"\n";
                }
                
                for (int m = 0; m < player.missilePoolA.activeMissiles.size(); m++)
                {
                    Missile* currentMissile = player.missilePoolA.activeMissiles[m];

                    CollisionResult rm;

                    rm = PrismVsSphere(obstacleColliderPos,obstacleCollider.GetTransformedVertices(obstacleColliderTransform),
                    currentMissile->body.transform.translation, currentMissile->radius);

                    if(rm.collision)
                    {
                        std::cout<<"MISSILE HIT at: "<<Vector3Length(currentMissile->body.linearVelocity)<<"\n";
                        currentMissile->didHit = true;
                    }
                }

                for (int m = 0; m < player.missilePoolB.activeMissiles.size(); m++)
                {
                    Missile* currentMissile = player.missilePoolB.activeMissiles[m];

                    CollisionResult rm;

                    rm = PrismVsSphere(obstacleColliderPos,obstacleCollider.GetTransformedVertices(obstacleColliderTransform),
                    currentMissile->body.transform.translation, currentMissile->radius);

                    if(rm.collision)
                    {
                        std::cout<<"MISSILE HIT at: "<<Vector3Length(currentMissile->body.linearVelocity)<<"\n";
                        currentMissile->didHit = true;
                    }
                }

                for (int m = 0; m < player.missilePoolA.activeMissiles.size(); m++)
                {
                    Missile* currentMissile = player.missilePoolA.activeMissiles[m];

                    CollisionResult rm;

                    for(int e = 0; e < enemyList.size(); e++)
                    {
                        float a = 100;

                        rm = PrismVsSphere(enemyList[e]->target.body.transform.translation,
                            enemyList[e]->target.hitbox.GetTransformedVertices(enemyList[e]->target.body.transform),
                            currentMissile->body.transform.translation, currentMissile->radius);

                        if(rm.collision)
                        {
                            currentMissile->didHit = true;
                        }
                    }
                }

                for (int m = 0; m < player.missilePoolB.activeMissiles.size(); m++)
                {
                    Missile* currentMissile = player.missilePoolB.activeMissiles[m];

                    CollisionResult rm;

                    for(int e = 0; e < enemyList.size(); e++)
                    {
                        float a = 100;

                        rm = PrismVsSphere(enemyList[e]->target.body.transform.translation,
                            enemyList[e]->target.hitbox.GetTransformedVertices(enemyList[e]->target.body.transform),
                            currentMissile->body.transform.translation, currentMissile->radius);

                        if(rm.collision)
                        {
                            currentMissile->didHit = true;
                        }
                    }
                }

                for (int a = 0; a < enemyList.size(); a++)
                {
                    enemyList[a]->UpdateEnemy(FIXED_DELTA_TIME, iterations, 
                        player.GetPosition(), 
                        player.missilePoolA.activeMissiles, player.missilePoolB.activeMissiles);
                }

                obstacleColliderColor = ogObstacleColliderColor;
            }

            for (int a = 0; a < enemyList.size(); a++)
            {
                enemyList[a]->FireB(FIXED_DELTA_TIME, player.GetPosition(), player.body.GetTrueVelocity());
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

        //DrawColliderWire(testCollider.GetTransformedVertices(player.GetHitboxTransform()), RED);

        //DrawSphere(player.GetTransform().translation, 2, MAGENTA);

        for(int i = 0; i < player.bulletPool.activeBullets.size(); i++)
        {
            Bullet* currentBullet = player.bulletPool.activeBullets[i];

            DrawBullet(currentBullet->transform, currentBullet->radius, BULLET_YELLOW);
        }        

        rlPushMatrix();
        rlRotatef(90,1,0,0);
        int scale = 100;
        Vector2 grpos;
        grpos.x = -(groundTexture.width / 2) * scale;
        grpos.y = -(groundTexture.height / 2) * scale; 
        DrawTextureEx(groundTexture, grpos, 0.0f,scale,WHITE);
        rlPopMatrix();
        
        DrawCollider(obstacleCollider.GetTransformedVertices(obstacleColliderTransform), {255,255,0,100});
        DrawColliderWire(obstacleCollider.GetTransformedVertices(obstacleColliderTransform), obstacleColliderColor);
        
        for(int i = 0; i < player.missilePoolA.activeMissiles.size(); i++)
        {
            Missile* currentMissile = player.missilePoolA.activeMissiles[i];

            DrawMissile(currentMissile->body.transform, currentMissile->radius);

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

            for(int j = 0; j < currentMissile->particlePool.activeParticles.size(); j++)
            {
                Particle* currentParticle = currentMissile->particlePool.activeParticles[j];

                RotateTowardsCamera(&currentParticle->transform, player.camera);
                DrawCircleRotated3D(currentParticle->transform, currentParticle->radius, {255,255,255,static_cast<unsigned char>(currentParticle->alpha)});
            }
        }

        for (int i = 0; i < enemyList.size(); i++)
        {
            DrawTgt(enemyList[i]->target);

            for(int j = 0; j < enemyList[i]->bulletPool.activeBullets.size(); j++)
            {
                Bullet* currentBullet = enemyList[i]->bulletPool.activeBullets[j];

                DrawBullet(currentBullet->transform, currentBullet->radius, BULLET_YELLOW);
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
        DrawText(TextFormat("SPEED: %0.2f", player.GetTrueSpeed()),SCREEN_WIDTH * 0.25, SCREEN_HEIGHT * 0.5, 20, GREEN);
        DrawText(TextFormat("THRUST: %0.2f", player.thrust),SCREEN_WIDTH * 0.25, SCREEN_HEIGHT * 0.25, 20, GREEN);


        DrawText(TextFormat("ALTITUDE: %0.2f", player.GetPosition().y),SCREEN_WIDTH * 0.7f, SCREEN_HEIGHT / 2, 20, GREEN);


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


/*

when creating the scene system, before drawing create a temp list of the objects to be drawn
out of the active ones

*/