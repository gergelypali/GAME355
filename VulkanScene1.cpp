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

    int windowX, windowY;
    SDL_GetWindowSize(m_ge->window(), &windowX, &windowY);

    auto bg = m_em->addEntity("bg");
    bg->addComponent<CTransform>(MATH::Vec2{windowX/2, windowY/2}, MATH::Vec2(0.f, 0.f), 0, 0, 0);
    bg->addComponent<CRectBody>(windowX, windowY, MATH::Vec4{1.f, 0.f, 1.f, 0.f});
    bg->addComponent<CState>();

    m_player = m_em->addEntity("Player");

    int playerWidth{40}, playerHeight{40};
    m_player->addComponent<CTransform>(MATH::Vec2{300.f, 200.f}, MATH::Vec2(0.f, 0.f), 0, 90, 300);
    m_player->addComponent<CRectBody>(playerWidth, playerHeight, MATH::Vec4{0.f, 0.f, 1.f, 0.f});
    m_player->addComponent<CState>();
}

void VulkanScene1::endScene()
{

}

void VulkanScene1::update()
{
    m_em->update();
    playerPhysicsUpdate();
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
