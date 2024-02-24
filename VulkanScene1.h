#ifndef VULKANSCENE1_H
#define VULKANSCENE1_H

#include "Scene.h"
#include <memory>
#include "Vector.h"

class VulkanScene1: public Scene
{
private:
    void init() override;
    void endScene() override;

    // systems
    void sRender() override;
    void sDoAction(const Action& action) override;

public:
    VulkanScene1() = delete;
    VulkanScene1(GameEngine* ge): Scene(ge) { init(); };
    ~VulkanScene1() { endScene(); };
    void update() override;

};

#endif