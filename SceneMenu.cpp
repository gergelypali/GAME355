#include "SceneMenu.h"
#include "GameEngine.h"
#include "Entity.h"
#include "EntityManager.h"
#include "AssetManager.h"

#include <math.h>
#include <fstream>

void SceneMenu::init()
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

    m_startButton = m_em->addEntity("StartGameButton");
    m_startButton->addComponent<CTransform>(MATH::Vec2{windowX / 2, windowY / 3});
    SDL_QueryTexture(m_ge->assetManager()->GetTexture("startGameButtonAnim"), NULL, NULL, &w, &h);
    m_startButton->addComponent<CSpriteSet>("startGameButtonAnim", 3, 3, w, h);
    m_startButton->addComponent<CRectBody>(w / 3, h / 3);
    m_startButton->addComponent<CAnimation>(m_ge->assetManager()->GetAnimation("menuButtonAnim"));

    m_exitButton = m_em->addEntity("ExitGameButton");
    m_exitButton->addComponent<CTransform>(MATH::Vec2{windowX / 2, 2 * windowY / 3});
    SDL_QueryTexture(m_ge->assetManager()->GetTexture("exitGameButtonAnim"), NULL, NULL, &w, &h);
    m_exitButton->addComponent<CRectBody>(w / 3, h / 3);
    m_exitButton->addComponent<CSpriteSet>("exitGameButtonAnim", 3, 3, w, h, 2, 2);

    // create actionMap for this scene; can create a function from this, so the init will call it every time, pure virtual function in scene
    registerAction(SDL_SCANCODE_W, "UP");
    registerAction(SDL_SCANCODE_S, "DOWN");
    registerAction(SDL_SCANCODE_E, "USE");
    registerAction(SDL_SCANCODE_LEFTBRACKET, "VolumeDown");
    registerAction(SDL_SCANCODE_RIGHTBRACKET, "VolumeUp");

    // add some menu music and button sounds
    m_ge->assetManager()->AddSound("buttonClick", "audio/buttonClick.mp3");
    m_ge->assetManager()->AddMusic("menuMusic", "audio/menuMusic.mp3");
    m_ge->playMusic("menuMusic");
}

void SceneMenu::endScene()
{
    
}

void SceneMenu::update()
{
    m_em->update();
    sRender();
    m_currentFrame++;
}

void SceneMenu::sRender()
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
            // final option is to just draw a colored rectangle
                drawRect(entity);
            }
        }
    }

    // render everything at the end of each render loop
    SDL_RenderPresent(m_ge->renderer());
}

void SceneMenu::sDoAction(const Action& action)
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
        m_ge->playSound("buttonClick");
        useActiveMenu();
    }
    else if (action.name() == "VolumeUp")
    {
        m_ge->changeSoundsVolume(10);
        m_ge->changeMusicVolume(10);
    }
    else if (action.name() == "VolumeDown")
    {
        m_ge->changeSoundsVolume(-10);
        m_ge->changeMusicVolume(-10);
    }
}

void SceneMenu::moveUpOneMenu()
{
    changeActiveMenu(-1);
}

void SceneMenu::moveDownOneMenu()
{
    changeActiveMenu(1);
}

void SceneMenu::useActiveMenu()
{
    switch(m_activeMenu)
    {
        case(0): m_ge->changeScene("ScenePlay"); break;
        case(1): m_ge->stopGameloop(); break;
        default: break;
    }
}

void SceneMenu::changeActiveMenu(int moveBy)
{
    m_activeMenu = std::min(m_maxMenuNumber, std::max(0, m_activeMenu + moveBy));

    switch(m_activeMenu)
    {
        case(0):
            m_startButton->addComponent<CAnimation>(m_ge->assetManager()->GetAnimation("menuButtonAnim"));
            m_exitButton->removeComponent<CAnimation>();
            m_exitButton->getComponent<CSpriteSet>().rowNumber = 2;
            m_exitButton->getComponent<CSpriteSet>().columnNumber = 2;
            break;
        case(1):
            m_exitButton->addComponent<CAnimation>(m_ge->assetManager()->GetAnimation("menuButtonAnim"));
            m_startButton->removeComponent<CAnimation>();
            m_startButton->getComponent<CSpriteSet>().rowNumber = 2;
            m_startButton->getComponent<CSpriteSet>().columnNumber = 2;
            break;
        default: break;
    }
}
