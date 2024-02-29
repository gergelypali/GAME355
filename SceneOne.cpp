#include "SceneOne.h"
#include "GameEngine.h"
#include "Entity.h"
#include "EntityManager.h"
#include "AssetManager.h"

#include <math.h>
#include <fstream>

void SceneOne::init()
{
    m_em = std::make_shared<EntityManager>();

    SDL_GetWindowSize(m_ge->window(), &windowX, &windowY);
    m_map = m_em->addEntity("map");
    m_map->addComponent<CRectBody>(2000, 2000);
    m_map->addComponent<CTexture>("cityBG");
    m_map->addComponent<CAABB>(2000, 2000);
    m_map->addComponent<CTransform>(MATH::Vec2{1000, 1000});

    spawnPlayer();

    spawnEnemy();

    createHUD();

    m_camera = m_em->addEntity("Camera");
    m_camera->addComponent<CTransform>(MATH::Vec2{0,0});

    // create actionMap for this scene; can create a function from this, so the init will call it every time, pure virtual function in scene
    registerAction(SDL_SCANCODE_W, "UP");
    registerAction(SDL_SCANCODE_S, "DOWN");
    registerAction(SDL_SCANCODE_A, "LEFT");
    registerAction(SDL_SCANCODE_D, "RIGHT");
    registerAction(SDL_BUTTON_LEFT, "LEFTMOUSE");
    registerAction(SDL_BUTTON_RIGHT, "RIGHTMOUSE");
    registerAction(SDL_MOUSEMOTION, "MOUSEMOTION");

    //m_ge->playMusic("menuMusic");
}

void SceneOne::endScene()
{

}

void SceneOne::update()
{
    // TODO: find a way to cycle through the entities only one time per update; not in every method: once for physics, once for movement etc...
    m_em->update();
    sPhysics();
    sMovement();
    sUpdateCamera();
    sRender();
    sCheckGameState();
    m_currentFrame++;
}

void SceneOne::sDoAction(const Action& action)
{
    // basic ARPG movement
    if (!m_player->hasComponent<CTransform>() || !m_player->hasComponent<CState>() || !m_player->hasComponent<CRectBody>())
        return;
    auto& transform = m_player->getComponent<CTransform>();
    auto& state = m_player->getComponent<CState>();
    auto& body = m_player->getComponent<CRectBody>();

    if (action.name() == "LEFTMOUSE")
    {
        state.moving = action.type() == "START";
        MATH::Vec2 mouseLocation{action.event().button.x, action.event().button.y};
        transform.vel = MATH::VMath::normalize(mouseLocation - transform.cameraViewPos);
    }
    else if (action.name() == "MOUSEMOTION")
    {
        MATH::Vec2 mouseLocation{action.event().motion.x, action.event().motion.y};
        transform.vel = MATH::VMath::normalize(mouseLocation - transform.cameraViewPos);
    }

    /* basic movement in four main direction
    if (!m_player->hasComponent<CTransform>() || !m_player->hasComponent<CState>())
        return;
    auto& transform = m_player->getComponent<CTransform>();
    auto& state = m_player->getComponent<CState>();

    if (action.name() == "UP")
    {
        transform.vel.y = (action.type() == "START") ? -1.f : 0.f;
    }
    else if (action.name() == "DOWN")
    {
        transform.vel.y = (action.type() == "START") ? 1.f : 0.f;
    }
    else if (action.name() == "LEFT")
    {
        transform.vel.x = (action.type() == "START") ? -1.f : 0.f;
    }
    else if (action.name() == "RIGHT")
    {
        transform.vel.x = (action.type() == "START") ? 1.f : 0.f;
    }

    state.moving = (MATH::VMath::mag(transform.vel) != 0) ? true : false;
    */
}

void SceneOne::sMovement()
{
    for (auto& entity: m_em->getEntities())
    {
        if (entity->hasComponent<CTransform>() && entity->hasComponent<CState>())
        {
            auto& transform = entity->getComponent<CTransform>();
            auto& state = entity->getComponent<CState>();

            if (state.moving)
                transform.pos = transform.pos + transform.vel * transform.moveSpeed;

            if (state.turning)
                transform.angle = fmod(transform.angle + transform.turnDirection * transform.turnSpeed, 360);

        }
    }
}

void SceneOne::playerPhysicsUpdate()
{
    if (m_player->hasComponent<CTransform>())
    {
        auto& transform = m_player->getComponent<CTransform>();
        auto& state = m_player->getComponent<CState>();

        transform.moveSpeed = (float)transform.maxMoveSpeed / m_ge->getFPS();
        transform.turnSpeed = transform.maxTurnSpeed / m_ge->getFPS();

        auto currentRad = atan2f(transform.vel.y, transform.vel.x);
        auto currentDeg = currentRad * 180 / M_PI;
        transform.angle = currentDeg;
    }
}

void SceneOne::reactToMapBorder()
{
    // check every enemy
    for ( auto& enemy: m_em->getEntities("Enemy"))
    {
        std::pair<bool, bool> checkRes{checkInsideEntity(enemy, m_map)};
        if (checkRes.first)
            enemy->getComponent<CTransform>().vel.x *= -1;
        if (checkRes.second)
            enemy->getComponent<CTransform>().vel.y *= -1;
    }

    // player part of the checking
    std::pair<bool, bool> playerCheckRes{checkInsideEntity(m_player, m_map)};
    if (playerCheckRes.first)
        m_player->getComponent<CTransform>().vel.x = 0;
    if (playerCheckRes.second)
        m_player->getComponent<CTransform>().vel.y = 0;
}

void SceneOne::sPhysics()
{
    playerPhysicsUpdate();
    reactToMapBorder();
    collisionWithPlayer();
}

void SceneOne::sCheckGameState()
{
    m_time += m_ge->getFrameTime();
    m_HUD.timeNumber->getComponent<CText>().text = std::to_string(m_time / 1000);
    checkEnd();
}

void SceneOne::sUpdateCamera()
{
    // here we move the camera around as an entity
    // update the location of the camera after everything moved with sMovement
    // update the transform's camera's world position for every entity so the rendering will use that position
    auto& camera = m_camera->getComponent<CTransform>();
    // this is the main logic to move the camera around, now we just follow the player and adjust the view to be center of the screen
    camera.pos.x = m_player->getComponent<CTransform>().pos.x - windowX / 2;
    camera.pos.y = m_player->getComponent<CTransform>().pos.y - windowY / 2;

    for (auto& entity: m_em->getEntities())
    {
        if (!entity->getComponent<CState>().cameraIndependent && entity->hasComponent<CTransform>())
        {
            auto& transform = entity->getComponent<CTransform>();
            transform.cameraViewPos.x = transform.pos.x - camera.pos.x;
            transform.cameraViewPos.y = transform.pos.y - camera.pos.y;
        }
    }
}

void SceneOne::spawnEnemy()
{
    int width{32}, height{32};
    auto enemy = m_em->addEntity("Enemy");
    enemy->addComponent<CTransform>(MATH::Vec2{500,500});
    enemy->addComponent<CRectBody>(width, height, MATH::Vec4{0xFF, 0, 0, 0xFF});
    enemy->addComponent<CAABB>(width, height);
}

void SceneOne::spawnRect(MATH::Vec2 start, MATH::Vec2 end)
{
    // nothing yet
}

void SceneOne::spawnPlayer()
{
    m_player = m_em->addEntity("Player");

    int playerWidth{40}, playerHeight{40};
    m_player->addComponent<CTransform>(MATH::Vec2{windowX/2, windowY/2}, MATH::Vec2(0.f, 0.f), 0, 90, 300);
    m_player->addComponent<CRectBody>(playerWidth, playerHeight);
    m_player->addComponent<CAABB>(playerWidth, playerHeight);
    m_player->addComponent<CScore>();
    int w, h;
    SDL_QueryTexture(m_ge->assetManager()->GetTexture("spriteStackPurpleCar"), NULL, NULL, &w, &h);
    m_player->addComponent<CSpriteStack>("spriteStackPurpleCar", 1, 8, w, h, playerHeight/h);
    //m_player->addComponent<CVoxel>("house1", 80, 1, w, h, playerHeight);
    m_player->addComponent<CState>();
}

void SceneOne::collisionWithPlayer()
{
    for (auto& enemy: m_em->getEntities("Enemy"))
    {
        if (checkEntityCollision(enemy, m_player))
        {
            m_player->getComponent<CScore>().score += 1;
            m_HUD.mainScoreNumber->getComponent<CText>().text = std::to_string(m_player->getComponent<CScore>().score);
            enemy->destroy();
            m_ge->playSound("buttonClick");
        }
    }
}

bool SceneOne::checkEntityCollision(std::shared_ptr<Entity> &one, std::shared_ptr<Entity> &two)
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

std::pair<bool, bool> SceneOne::checkInsideEntity(std::shared_ptr<Entity>& one, std::shared_ptr<Entity>& two)
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

void SceneOne::checkEnd()
{
    if (m_player->getComponent<CScore>().score > 5)
        m_ge->changeScene("SceneEnd");
}

void SceneOne::createHUD()
{
    // TODO: create a better render ordering system; with layers and order in a given layer, so we can give precise order to render the objects
    // TODO: also some kind of culling system, so we are not rendering objects that are not on the screen, just calculate the other changes: physics, collision, etc
    m_HUD.upperBar = m_em->addEntity("HUDelement");
    m_HUD.upperBar->addComponent<CRectBody>(windowX, 48, MATH::Vec4{0xFF, 0, 0xFF, 0xFF});
    m_HUD.upperBar->addComponent<CTransform>(MATH::Vec2{windowX / 2, 24});
    m_HUD.upperBar->addComponent<CState>();
    m_HUD.upperBar->getComponent<CState>().cameraIndependent = true;
    m_HUD.mainScoreText = m_em->addEntity("HUDelement");
    m_HUD.mainScoreText->addComponent<CText>("SCORE: ", m_ge->assetManager()->GetFont("Nasa21"), SDL_Color{0,0,0}, 30);
    m_HUD.mainScoreText->addComponent<CTransform>(MATH::Vec2{100, 25});
    m_HUD.mainScoreText->addComponent<CState>();
    m_HUD.mainScoreText->getComponent<CState>().cameraIndependent = true;
    m_HUD.mainScoreNumber = m_em->addEntity("HUDelement");
    m_HUD.mainScoreNumber->addComponent<CText>(m_player->getComponent<CScore>().score, m_ge->assetManager()->GetFont("Nasa21"), SDL_Color{0,0,0}, 30);
    m_HUD.mainScoreNumber->addComponent<CTransform>(MATH::Vec2{200, 25});
    m_HUD.mainScoreNumber->addComponent<CState>();
    m_HUD.mainScoreNumber->getComponent<CState>().cameraIndependent = true;
    m_HUD.timeText = m_em->addEntity("HUDelement");
    m_HUD.timeText->addComponent<CText>("TIME: ", m_ge->assetManager()->GetFont("Nasa21"), SDL_Color{0,0,0}, 30);
    m_HUD.timeText->addComponent<CTransform>(MATH::Vec2{300, 25});
    m_HUD.timeText->addComponent<CState>();
    m_HUD.timeText->getComponent<CState>().cameraIndependent = true;
    m_HUD.timeNumber = m_em->addEntity("HUDelement");
    m_HUD.timeNumber->addComponent<CText>(m_time, m_ge->assetManager()->GetFont("Nasa21"), SDL_Color{0,0,0}, 30);
    m_HUD.timeNumber->addComponent<CTransform>(MATH::Vec2{400, 25});
    m_HUD.timeNumber->addComponent<CState>();
    m_HUD.timeNumber->getComponent<CState>().cameraIndependent = true;
    m_HUD.fpsText = m_em->addEntity("HUDelement");
    m_HUD.fpsText->addComponent<CText>("FPS: ", m_ge->assetManager()->GetFont("Nasa21"), SDL_Color{0,0,0}, 30);
    m_HUD.fpsText->addComponent<CTransform>(MATH::Vec2{500, 25});
    m_HUD.fpsText->addComponent<CState>();
    m_HUD.fpsText->getComponent<CState>().cameraIndependent = true;
    m_HUD.fpsNumber = m_em->addEntity("HUDelement");
    m_HUD.fpsNumber->addComponent<CText>((int)m_ge->getFPS(), m_ge->assetManager()->GetFont("Nasa21"), SDL_Color{0,0,0}, 30);
    m_HUD.fpsNumber->addComponent<CTransform>(MATH::Vec2{600, 25});
    m_HUD.fpsNumber->addComponent<CState>();
    m_HUD.fpsNumber->getComponent<CState>().cameraIndependent = true;
}
