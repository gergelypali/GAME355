#include "ScenePlay.h"
#include "GameEngine.h"
#include "Entity.h"
#include "EntityManager.h"
#include "AssetManager.h"

#include <math.h>
#include <fstream>

void ScenePlay::init()
{
    m_em = std::make_shared<EntityManager>();

    m_bg = m_em->addEntity("BG");
    int windowX, windowY;
    SDL_GetWindowSize(m_ge->window(), &windowX, &windowY);
    m_bg->addComponent<CRectBody>(windowX, windowY, MATH::Vec4(0, 0x0, 0xFF, 0x0));
    m_bg->addComponent<CAABB>(windowX, windowY);
    m_bg->addComponent<CTransform>(MATH::Vec2{windowX/2, windowY/2});

    // add the tiles here for creating the map's visualization
    initMapTiles();

    spawnPlayer();

    spawnEnemy(0);

    // create actionMap for this scene; can create a function from this, so the init will call it every time, pure virtual function in scene
    registerAction(SDL_SCANCODE_W, "UP");
    registerAction(SDL_SCANCODE_S, "DOWN");
    registerAction(SDL_SCANCODE_A, "LEFT");
    registerAction(SDL_SCANCODE_D, "RIGHT");
}

void ScenePlay::endScene()
{
    
}

void ScenePlay::update()
{
    m_em->update();
    sPhysics();
    sMovement();
    sRender();
    sCheckGameState();
    m_currentFrame++;
}

void ScenePlay::sRender()
{
    SDL_SetRenderDrawColor(m_ge->renderer(), 0xFF, 0xFF, 0xFF, 0x0);
    SDL_RenderClear(m_ge->renderer());
    SDL_SetRenderDrawBlendMode(m_ge->renderer(), SDL_BLENDMODE_BLEND);

    for (auto& entity: m_em->getEntities())
    {
        if (entity->hasComponent<CTransform>() && entity->hasComponent<CRectBody>())
        {
            if (entity->hasComponent<CTexture>())
            {
                drawTexture(entity);
            }
            else if (entity->hasComponent<CSpriteSet>())
            {
                if (entity->hasComponent<CAnimation>())
                    drawAnimation(entity);
                else
                    drawSpriteSet(entity);
            }
            else if (entity->hasComponent<CSpriteStack>())
            {
                drawSpriteStack(entity);
            }
            else
            {
                drawRect(entity);
            }
        }
    }

    // render everything at the end of each render loop
    SDL_RenderPresent(m_ge->renderer());
}

void ScenePlay::sDoAction(const Action& action)
{
    if (!m_player->hasComponent<CTransform>() || !m_player->hasComponent<CState>())
        return;
    auto& transform = m_player->getComponent<CTransform>();
    auto& state = m_player->getComponent<CState>();
    if (action.name() == "UP")
    {
        state.moving = action.type() == "START";
    }
    else if (action.name() == "DOWN")
    {
        state.moving = action.type() == "START";
        transform.moveSpeed *= -1;
    }
    else if (action.name() == "LEFT")
    {
        state.turning = action.type() == "START";
        transform.turnSpeed *= -1;
    }
    else if (action.name() == "RIGHT")
    {
        state.turning = action.type() == "START";
    }
    /* this control is for a simple up down left right movement system
    if (action.name() == "UP")
    {
        m_player->getComponent<CTransform>().vel.y = (action.type() == "START") ? -2.f : 0.f;
    }
    else if (action.name() == "DOWN")
    {
        m_player->getComponent<CTransform>().vel.y = (action.type() == "START") ? 2.f : 0.f;
    }
    else if (action.name() == "LEFT")
    {
        m_player->getComponent<CTransform>().vel.x = (action.type() == "START") ? -2.f : 0.f;
    }
    else if (action.name() == "RIGHT")
    {
        m_player->getComponent<CTransform>().vel.x = (action.type() == "START") ? 2.f : 0.f;
    }
    */
}

void ScenePlay::sMovement()
{
    for (auto& entity: m_em->getEntities())
    {
        if (entity->hasComponent<CTransform>() && entity->hasComponent<CState>())
        {
            auto& transform = entity->getComponent<CTransform>();
            auto& state = entity->getComponent<CState>();

            if (state.moving)
                transform.pos = transform.pos + transform.vel;

            if (state.turning)
                transform.angle = transform.angle + transform.turnSpeed / m_ge->getFPS();

        }
    }
}

void ScenePlay::playerPhysicsUpdate()
{
    if (m_player->hasComponent<CTransform>())
    {
        auto& transform = m_player->getComponent<CTransform>();
        double realAngle = fmod(transform.angle, 360.0) * M_PI / 180.0;
        auto currentMoveSpeed = (float)transform.moveSpeed / m_ge->getFPS();
        transform.vel = MATH::Vec2{cosf(realAngle), sinf(realAngle)} * currentMoveSpeed;
    }
}

void ScenePlay::reactToMapBorder()
{
    // check every enemy
    for ( auto& enemy: m_em->getEntities("Enemy"))
    {
        std::pair<bool, bool> checkRes{checkInsideEntity(enemy, m_bg)};
        if (checkRes.first)
            enemy->getComponent<CTransform>().vel.x *= -1;
        if (checkRes.second)
            enemy->getComponent<CTransform>().vel.y *= -1;
    }

    // player part of the checking
    std::pair<bool, bool> playerCheckRes{checkInsideEntity(m_player, m_bg)};
    if (playerCheckRes.first)
        m_player->getComponent<CTransform>().vel.x = 0;
    if (playerCheckRes.second)
        m_player->getComponent<CTransform>().vel.y = 0;
}

void ScenePlay::sPhysics()
{
    playerPhysicsUpdate();
    reactToMapBorder();
    collisionWithPlayer();
}

void ScenePlay::sCheckGameState()
{
    checkEnd();
}

void ScenePlay::spawnEnemy(int number)
{
    int windowX, windowY;
    SDL_GetWindowSize(m_ge->window(), &windowX, &windowY);
    int width{32}, height{32};
    auto enemy = m_em->addEntity("Enemy");
    enemy->addComponent<CTransform>(enemySpawnLocs[number]);
    enemy->addComponent<CRectBody>(width, height);
    enemy->addComponent<CAABB>(width, height);
    int w, h;
    SDL_QueryTexture(m_ge->assetManager()->GetTexture("characters"), NULL, NULL, &w, &h);
    enemy->addComponent<CSpriteSet>("characters", 8, 4, w, h);
    // just to test the animation loading and changing
    switch(number)
    {
        case(1): enemy->addComponent<CAnimation>(m_ge->assetManager()->GetAnimation("walkDown")); break;
        case(2): enemy->addComponent<CAnimation>(m_ge->assetManager()->GetAnimation("walkLeft")); break;
        case(3): enemy->addComponent<CAnimation>(m_ge->assetManager()->GetAnimation("walkUp")); break;
        default: enemy->addComponent<CAnimation>(m_ge->assetManager()->GetAnimation("walkRight")); break;
    }
}

void ScenePlay::spawnPlayer()
{
    m_player = m_em->addEntity("Player");

    int playerWidth{32}, playerHeight{32};
    m_player->addComponent<CTransform>(MATH::Vec2(200.f, 200.f), MATH::Vec2(0.f, 0.f), 0, 90, 128);
    m_player->addComponent<CRectBody>(playerWidth, playerHeight);
    m_player->addComponent<CAABB>(playerWidth, playerHeight);
    m_player->addComponent<CScore>();
    int w, h;
    SDL_QueryTexture(m_ge->assetManager()->GetTexture("spriteStackPurpleCar"), NULL, NULL, &w, &h);
    m_player->addComponent<CSpriteStack>("spriteStackPurpleCar", 1, 8, w, h, playerHeight/h);
    m_player->addComponent<CState>();
}

void ScenePlay::collisionWithPlayer()
{
    for (auto& enemy: m_em->getEntities("Enemy"))
    {
        if (checkEntityCollision(enemy, m_player))
        {
            m_player->getComponent<CScore>().score += 1;
            std::string newName{""};
            int frameCount{0};
            switch(m_player->getComponent<CScore>().score)
            {
                case(1) : newName = "spriteStackRedMotorcycle", frameCount = 10; break;
                case(2) : newName = "spriteStackGreenBigCar", frameCount = 10; break;
                case(3) : newName = "spriteStackWhiteMotorcycle", frameCount = 9; break;
                case(4) : newName = "spriteStackRedCar", frameCount = 8; break;
                case(5) : newName = "spriteStackYellowCar", frameCount = 12; break;
                case(6) : newName = "spriteStackBlueCar", frameCount = 9; break;
                case(7) : newName = "spriteStackBrownMotorcycle", frameCount = 10; break;
                default : newName = "spriteStackGreenCar", frameCount = 7; break;
            }
            changePlayerSkin(newName, frameCount);
            spawnEnemy(m_player->getComponent<CScore>().score);
            enemy->destroy();
        }
    }
}

void ScenePlay::initMapTiles()
{
    int tileWidth{128}, tileHeight{128};
    std::ifstream infile("map.txt");
    std::string tileName;
    int row, col;
    int w, h;
    SDL_QueryTexture(m_ge->assetManager()->GetTexture("roadParts"), NULL, NULL, &w, &h);
    while (infile >> tileName >> row >> col)
    {
        int tileRow{0}, tileCol{0};

        if (tileName == "roadLeftRight")
            tileRow = 0, tileCol = 2;
        else if (tileName == "roadUpDown")
            tileRow = 0, tileCol = 3;
        else if (tileName == "roadRightDown")
            tileRow = 1, tileCol = 0;
        else if (tileName == "roadLeftDown")
            tileRow = 1, tileCol = 1;
        else if (tileName == "roadLeftUp")
            tileRow = 1, tileCol = 2;
        else if (tileName == "roadRigthUp")
            tileRow = 1, tileCol = 3;
        else if (tileName == "roadLeftDouble")
            tileRow = 0, tileCol = 0;
        else if (tileName == "roadRightDouble")
            tileRow = 0, tileCol = 1;
        else if (tileName == "roadUpEnd")
            tileRow = 0, tileCol = 4;
        else if (tileName == "roadDownEnd")
            tileRow = 0, tileCol = 5;
        else if (tileName == "roadLeftEnd")
            tileRow = 0, tileCol = 6;
        else if (tileName == "roadRightEnd")
            tileRow = 0, tileCol = 7;
        else if (tileName == "roadLeftRightUp")
            tileRow = 1, tileCol = 4;
        else if (tileName == "roadLeftRightDown")
            tileRow = 1, tileCol = 5;
        else if (tileName == "roadLeftUpDown")
            tileRow = 1, tileCol = 6;
        else if (tileName == "roadRigthUpDown")
            tileRow = 1, tileCol = 7;
        else if (tileName == "roadCross")
            tileRow = 2, tileCol = 0;
        else if (tileName == "roadEnd")
            tileRow = 2, tileCol = 1;

        auto mapTile = m_em->addEntity("mapTile");
        mapTile->addComponent<CTransform>(MATH::Vec2{col * tileHeight + tileHeight / 2, row * tileWidth + tileWidth / 2});
        mapTile->addComponent<CRectBody>(tileWidth, tileHeight);
        mapTile->addComponent<CSpriteSet>("roadParts", 3, 8, w, h, tileRow, tileCol);
    }
}

bool ScenePlay::checkEntityCollision(std::shared_ptr<Entity> &one, std::shared_ptr<Entity> &two)
{
        if (one->hasComponent<CTransform>() && one->hasComponent<CAABB>() &&
            two->hasComponent<CTransform>() && two->hasComponent<CAABB>())
        {
            auto& oneTransform = one->getComponent<CTransform>();
            auto& oneAABB = one->getComponent<CAABB>();
            auto& twoTransform = two->getComponent<CTransform>();
            auto& twoAABB = two->getComponent<CAABB>();

            MATH::Vec2 diff{fabsf(oneTransform.pos.x - twoTransform.pos.x),
                            fabsf(oneTransform.pos.y - twoTransform.pos.y)};
            return diff.x < abs(oneAABB.halfWidth + twoAABB.halfWidth) &&
                    diff.y < abs(oneAABB.halfHeight + twoAABB.halfHeight);
        }
        else
        {
            return false;
        }
}

std::pair<bool, bool> ScenePlay::checkInsideEntity(std::shared_ptr<Entity>& one, std::shared_ptr<Entity>& two)
{
        if (one->hasComponent<CTransform>() && one->hasComponent<CAABB>() &&
            two->hasComponent<CTransform>() && two->hasComponent<CAABB>())
        {
            auto& oneTransform = one->getComponent<CTransform>();
            auto& oneAABB = one->getComponent<CAABB>();
            auto& twoTransform = two->getComponent<CTransform>();
            auto& twoAABB = two->getComponent<CAABB>();

            bool outsideX = oneTransform.pos.x - oneAABB.halfWidth + oneTransform.vel.x < twoTransform.pos.x - twoAABB.halfWidth ||
                oneTransform.pos.x + oneAABB.halfWidth + oneTransform.vel.x > twoTransform.pos.x + twoAABB.halfWidth;
            bool outsideY = oneTransform.pos.y - oneAABB.halfHeight + oneTransform.vel.y < twoTransform.pos.y - twoAABB.halfHeight ||
                oneTransform.pos.y + oneAABB.halfHeight + oneTransform.vel.y > twoTransform.pos.y + twoAABB.halfHeight;

            return std::make_pair(outsideX, outsideY);
        }
        else
        {
            return std::make_pair(false, false);
        }
}

void ScenePlay::checkLifetime()
{
    for (auto& enemy: m_em->getEntities())
    {
        if (enemy->hasComponent<CLifetime>() && m_currentFrame - enemy->getComponent<CLifetime>().startFrame == enemy->getComponent<CLifetime>().maxLifetime)
            enemy->destroy();
    }
}

void ScenePlay::fadeOut()
{
    auto alpha = (1.f - (float)m_player->getComponent<CScore>().score / 273.f);
    m_em->getEntities("BG")[0]->getComponent<CRectBody>().color().w = std::fmaxf(std::fminf(0xFF * alpha, 0xFF), 0x0);

    for (auto& enemy: m_em->getEntities("Enemy"))
    {
        if (enemy->hasComponent<CLifetime>() && enemy->hasComponent<CRectBody>())
            enemy->getComponent<CRectBody>().color().w = enemy->getComponent<CRectBody>().color().w - 0xFF / (float)enemy->getComponent<CLifetime>().maxLifetime;
    }
}

void ScenePlay::checkEnd()
{
    if (m_player->getComponent<CScore>().score > 2)
        m_ge->changeScene("SceneEnd");
}

void ScenePlay::changePlayerSkin(const std::string &name, int frameCount)
{
    int w, h;
    SDL_QueryTexture(m_ge->assetManager()->GetTexture(name), NULL, NULL, &w, &h);
    m_player->addComponent<CSpriteStack>(name, 1, frameCount, w, h, m_player->getComponent<CRectBody>().height()/h);
}
