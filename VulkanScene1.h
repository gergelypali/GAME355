#ifndef VULKANSCENE1_H
#define VULKANSCENE1_H

#include "Scene.h"
#include <memory>
#include "Vector.h"

class Grid;

class VulkanScene1: public Scene
{
private:
    std::shared_ptr<Entity> m_player{nullptr};
    std::shared_ptr<Entity> m_map{nullptr};
    int windowX{0}, windowY{0};
    int mazeX{40}, mazeY{20};
    std::shared_ptr<Grid> m_grid{nullptr};

    void init() override;
    void endScene() override;

    // systems
    void sDoAction(const Action& action) override;

    void sMovement();
    void playerPhysicsUpdate();
    void reactToMapBorder();
    void checkWalls();

    void spawnEnemy(const float& x, const float& y);
    void spawnMarker(float x, float y, int lifetime, std::string markerName);
    void generateMaze();
    void checkEndMap();

public:
    VulkanScene1() = delete;
    VulkanScene1(GameEngine* ge): Scene(ge) { init(); };
    ~VulkanScene1() { endScene(); };
    void update() override;

};

#endif