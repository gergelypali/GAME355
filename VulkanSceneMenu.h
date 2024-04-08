#ifndef VULKANSCENEMENU_H
#define VULKANSCENEMENU_H

#include "Scene.h"
#include <memory>

class VulkanSceneMenu : public Scene
{
public:
    VulkanSceneMenu() = delete;
    VulkanSceneMenu(GameEngine* ge): Scene(ge) { init(); };
    ~VulkanSceneMenu() { endScene(); };
    void update() override;

private:
    std::shared_ptr<Entity> m_bg{nullptr};
    std::shared_ptr<Entity> m_newGameButton{nullptr};
    std::shared_ptr<Entity> m_exitGameButton{nullptr};
    int m_windowX, m_windowY;

    void init() override;
    void endScene() override;

    // systems
    void sDoAction(const Action& action) override;

};

#endif
