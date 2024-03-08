#ifndef VULKANSCENE1_H
#define VULKANSCENE1_H

#include "Scene.h"
#include <memory>
#include "Vector.h"

class VulkanScene1: public Scene
{
private:
    std::shared_ptr<Entity> m_player{nullptr};
    std::shared_ptr<Entity> m_map{nullptr};
    int windowX, windowY;

    void init() override;
    void endScene() override;

    // systems
    void sDoAction(const Action& action) override;

    void sMovement();
    void playerPhysicsUpdate();
    void reactToMapBorder();

    void spawnEnemy(const float& x, const float& y);

public:
    VulkanScene1() = delete;
    VulkanScene1(GameEngine* ge): Scene(ge) { init(); };
    ~VulkanScene1() { endScene(); };
    void update() override;

};

#endif