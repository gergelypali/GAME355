#include "VulkanSceneMenu.h"
#include "Entity.h"
#include "EntityManager.h"
#include "AssetManager.h"

void VulkanSceneMenu::update()
{
    m_em->update();
    sRender();
    m_currentFrame++;
}

void VulkanSceneMenu::init()
{
    registerAction(SDL_BUTTON_LEFT, "MOUSECLICK");
    registerAction(SDL_MOUSEMOTION, "MOUSEMOTION");

    m_ge->getWindowSize(m_windowX, m_windowY);

    m_bg = m_em->addEntity("map");
    m_bg->addComponent<CTransform>(MATH::Vec2{m_windowX/2, m_windowY/2});
    m_bg->addComponent<CRectBody>(m_windowX, m_windowY);
    m_bg->addComponent<CState>();
    m_bg->addComponent<CAABB>(m_windowX, m_windowY);
    m_bg->addComponent<CShape2d>("rectangleVertex", "rectangleIndex");
    m_bg->addComponent<CTexture>("brick");

    int buttonWidth{0}, buttonHeight{0};

    m_newGameButton = m_em->addEntity("button");
    m_newGameButton->addComponent<CTransform>(MATH::Vec2{m_windowX/2, 2 * m_windowY/5});
    m_newGameButton->addComponent<CTexture>("startButton");
    buttonWidth = m_ge->assetManager()->GetVulkanTextureSize(m_newGameButton->getComponent<CTexture>().name).x;
    buttonHeight = m_ge->assetManager()->GetVulkanTextureSize(m_newGameButton->getComponent<CTexture>().name).y;
    m_newGameButton->addComponent<CRectBody>(buttonWidth, buttonHeight);
    m_newGameButton->addComponent<CState>();
    m_newGameButton->addComponent<CAABB>(buttonWidth, buttonHeight);
    m_newGameButton->getComponent<CAABB>().scale = 0.5f;
    m_newGameButton->addComponent<CShape2d>("rectangleVertex", "rectangleIndex");
    m_newGameButton->getComponent<CRectBody>().scale = 0.5f;

    m_exitGameButton = m_em->addEntity("button");
    m_exitGameButton->addComponent<CTransform>(MATH::Vec2{m_windowX/2, 3 * m_windowY/5});
    m_exitGameButton->addComponent<CTexture>("exitButton");
    buttonWidth = m_ge->assetManager()->GetVulkanTextureSize(m_exitGameButton->getComponent<CTexture>().name).x;
    buttonHeight = m_ge->assetManager()->GetVulkanTextureSize(m_exitGameButton->getComponent<CTexture>().name).y;
    m_exitGameButton->addComponent<CRectBody>(buttonWidth, buttonHeight);
    m_exitGameButton->addComponent<CState>();
    m_exitGameButton->addComponent<CAABB>(buttonWidth, buttonHeight);
    m_exitGameButton->getComponent<CAABB>().scale = 0.5f;
    m_exitGameButton->addComponent<CShape2d>("rectangleVertex", "rectangleIndex");
    m_exitGameButton->getComponent<CRectBody>().scale = 0.5f;
}

void VulkanSceneMenu::endScene()
{
}

void VulkanSceneMenu::sDoAction(const Action &action)
{
    if (action.type() != "START")
        return;
    if (action.name() == "MOUSECLICK")
    {
        MATH::Vec2 mouseLocation{action.event().button.x, action.event().button.y};
        for (auto& entity: m_em->getEntities("button"))
        {
            auto res = checkPointInsideEntity(mouseLocation, entity);
            if (res.first && res.second)
            {
                if(entity->getComponent<CTexture>().name == "startButton")
                    m_ge->changeScene("VulkanScene1");
                if(entity->getComponent<CTexture>().name == "exitButton")
                    m_ge->stopGameloop();
            }
        }
    }
    else if (action.name() == "MOUSEMOTION")
    {
        MATH::Vec2 mouseLocation{action.event().button.x, action.event().button.y};
    }
}
