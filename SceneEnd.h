/// not used after Vulkan migration so it is outdated

#ifndef SCENEEND_H
#define SCENEEND_H

#include "Scene.h"
#include <memory>
#include "Vector.h"

class Entity;

class SceneEnd: public Scene
{
private:
    std::shared_ptr<Entity> m_bg;
    std::shared_ptr<Entity> m_retryButton;
    std::shared_ptr<Entity> m_exitButton;
    int m_activeMenu{0};
    int m_maxMenuNumber{1};

    void init() override;
    void endScene() override;

    // systems
    void sMovement();
    void sDoAction(const Action& action) override;

    // scene specific methods
    void changeActiveMenu(int moveBy);
    void moveUpOneMenu();
    void moveDownOneMenu();
    void useActiveMenu();

public:
    SceneEnd() = delete;
    SceneEnd(GameEngine* ge): Scene(ge) { init(); };
    ~SceneEnd() { endScene(); };
    void update() override;

};

#endif