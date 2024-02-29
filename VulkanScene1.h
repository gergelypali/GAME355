#ifndef VULKANSCENE1_H
#define VULKANSCENE1_H

#include "Scene.h"
#include <memory>
#include "Vector.h"

class VulkanScene1: public Scene
{
private:
    std::shared_ptr<Entity> m_player{nullptr};

    void init() override;
    void endScene() override;

    // systems
    void sDoAction(const Action& action) override;

    void sMovement();
    void playerPhysicsUpdate();

public:
    VulkanScene1() = delete;
    VulkanScene1(GameEngine* ge): Scene(ge) { init(); };
    ~VulkanScene1() { endScene(); };
    void update() override;

};

#endif