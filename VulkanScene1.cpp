#include "VulkanScene1.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Logger.h"

void VulkanScene1::init()
{
    registerAction(SDL_SCANCODE_W, "UP");
    registerAction(SDL_SCANCODE_S, "DOWN");
    registerAction(SDL_SCANCODE_A, "LEFT");
    registerAction(SDL_SCANCODE_D, "RIGHT");
    registerAction(SDL_BUTTON_LEFT, "MOUSECLICK");
    registerAction(SDL_MOUSEMOTION, "MOUSEMOTION");

    SDL_GetWindowSize(m_ge->window(), &windowX, &windowY);

    m_map = m_em->addEntity("map");
    m_map->addComponent<CTransform>(MATH::Vec2{windowX/2, windowY/2});
    m_map->addComponent<CRectBody>(windowX/2, windowY/2, MATH::Vec4{1.f, 0.f, 1.f, 0.f});
    m_map->addComponent<CState>();
    m_map->addComponent<CAABB>(windowX, windowY);
    m_map->addComponent<CShape2d>("rectangleVertex", "newformIndex");

    m_player = m_em->addEntity("Player");

    int playerWidth{40}, playerHeight{40};
    m_player->addComponent<CTransform>(MATH::Vec2{300.f, 200.f}, MATH::Vec2(0.f, 0.f), 0, 90, 300);
    m_player->addComponent<CRectBody>(playerWidth, playerHeight, MATH::Vec4{0.f, 0.f, 1.f, 0.f});
    m_player->addComponent<CState>();
    m_player->addComponent<CAABB>(playerWidth, playerHeight);
    m_player->addComponent<CShape2d>("rectangleVertex", "triangleIndex");

    spawnEnemy(100.f, 50.f);
}

void VulkanScene1::endScene()
{

}

void VulkanScene1::update()
{
    m_em->update();
    playerPhysicsUpdate();
    reactToMapBorder();
    sMovement();
    sRender();
    m_currentFrame++;
}

void VulkanScene1::sDoAction(const Action& action)
{
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
    else if (action.name() == "MOUSECLICK" && action.event().type == SDL_MOUSEBUTTONDOWN)
    {
        spawnEnemy(action.event().button.x, action.event().button.y);
    }
    else if (action.name() == "MOUSEMOTION")
    {
        spawnEnemy(action.event().button.x, action.event().button.y);
    }

    state.moving = (MATH::VMath::mag(transform.vel) != 0) ? true : false;
}

void VulkanScene1::sMovement()
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

void VulkanScene1::playerPhysicsUpdate()
{
    if (m_player->hasComponent<CTransform>())
    {
        auto& transform = m_player->getComponent<CTransform>();

        transform.moveSpeed = (float)transform.maxMoveSpeed / m_ge->getFPS();
        transform.turnSpeed = transform.maxTurnSpeed / m_ge->getFPS();

        auto currentRad = atan2f(transform.vel.y, transform.vel.x);
        auto currentDeg = currentRad * 180 / M_PI;
        transform.angle = currentDeg;
    }
}

void VulkanScene1::reactToMapBorder()
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

void VulkanScene1::spawnEnemy(const float& x, const float& y)
{
    auto enemy = m_em->addEntity("Enemy");
    enemy->addComponent<CTransform>(MATH::Vec2{x, y}, MATH::Vec2(1.f, 1.f), 0, 90, 5);
    enemy->addComponent<CRectBody>(20, 20, MATH::Vec4{1.f, 1.f, 0.f, 0.f});
    enemy->addComponent<CState>(false, true);
    enemy->addComponent<CAABB>(20, 20);
    enemy->addComponent<CShape2d>("rectangleVertex", "rectangleIndex");
}
