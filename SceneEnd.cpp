#include "SceneEnd.h"
#include "GameEngine.h"
#include "Entity.h"
#include "EntityManager.h"
#include "AssetManager.h"

#include <math.h>
#include <fstream>

void SceneEnd::init()
{
    m_em = std::make_shared<EntityManager>();

    m_bg = m_em->addEntity("BG");
    int windowX, windowY;
    SDL_GetWindowSize(m_ge->window(), &windowX, &windowY);
    m_bg->addComponent<CTransform>(MATH::Vec2{windowX/2, windowY/2});
    int w, h;
    SDL_QueryTexture(m_ge->assetManager()->GetTexture("menuBG"), NULL, NULL, &w, &h);
    m_bg->addComponent<CRectBody>(w, h);
    m_bg->addComponent<CTexture>("menuBG");

    m_retryButton = m_em->addEntity("RetryGameButton");
    m_retryButton->addComponent<CTransform>(MATH::Vec2{windowX / 2, windowY / 3});
    SDL_QueryTexture(m_ge->assetManager()->GetTexture("retryGameButtonAnim"), NULL, NULL, &w, &h);
    m_retryButton->addComponent<CSpriteSet>("retryGameButtonAnim", 3, 3, w, h);
    m_retryButton->addComponent<CRectBody>(w / 3, h / 3);
    m_retryButton->addComponent<CAnimation>(m_ge->assetManager()->GetAnimation("menuButtonAnim"));

    m_exitButton = m_em->addEntity("ExitGameButton");
    m_exitButton->addComponent<CTransform>(MATH::Vec2{windowX / 2, 2 * windowY / 3});
    SDL_QueryTexture(m_ge->assetManager()->GetTexture("exitGameButtonAnim"), NULL, NULL, &w, &h);
    m_exitButton->addComponent<CRectBody>(w / 3, h / 3);
    m_exitButton->addComponent<CSpriteSet>("exitGameButtonAnim", 3, 3, w, h, 2, 2);

    // create actionMap for this scene; can create a function from this, so the init will call it every time, pure virtual function in scene
    registerAction(SDL_SCANCODE_W, "UP");
    registerAction(SDL_SCANCODE_S, "DOWN");
    registerAction(SDL_SCANCODE_E, "USE");
}

void SceneEnd::endScene()
{
    
}

void SceneEnd::update()
{
    m_em->update();
    sMovement();
    sRender();
    m_currentFrame++;
}

void SceneEnd::sRender()
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

void SceneEnd::sDoAction(const Action& action)
{
    if (action.type() != "START")
        return;
    if (action.name() == "UP")
    {
        // move to the previous menu
        moveUpOneMenu();
    }
    else if (action.name() == "DOWN")
    {
        // move to the next menu
        moveDownOneMenu();
    }
    else if (action.name() == "USE")
    {
        // activate the currently selected menu
        useActiveMenu();
    }
}

void SceneEnd::sMovement()
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

void SceneEnd::moveUpOneMenu()
{
    changeActiveMenu(-1);
}

void SceneEnd::moveDownOneMenu()
{
    changeActiveMenu(1);
}

void SceneEnd::useActiveMenu()
{
    switch(m_activeMenu)
    {
        case(0): m_ge->changeScene("ScenePlay"); break;
        case(1): m_ge->stopGameloop(); break;
        default: break;
    }
}

void SceneEnd::changeActiveMenu(int moveBy)
{
    m_activeMenu = std::min(m_maxMenuNumber, std::max(0, m_activeMenu + moveBy));

    switch(m_activeMenu)
    {
        case(0):
            m_retryButton->addComponent<CAnimation>(m_ge->assetManager()->GetAnimation("menuButtonAnim"));
            m_exitButton->removeComponent<CAnimation>();
            m_exitButton->getComponent<CSpriteSet>().rowNumber = 2;
            m_exitButton->getComponent<CSpriteSet>().columnNumber = 2;
            break;
        case(1):
            m_exitButton->addComponent<CAnimation>(m_ge->assetManager()->GetAnimation("menuButtonAnim"));
            m_retryButton->removeComponent<CAnimation>();
            m_retryButton->getComponent<CSpriteSet>().rowNumber = 2;
            m_retryButton->getComponent<CSpriteSet>().columnNumber = 2;
            break;
        default: break;
    }
}
