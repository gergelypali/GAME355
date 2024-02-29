#ifndef SCENEONE_H
#define SCENEONE_H

#include "Scene.h"
#include <memory>
#include "Vector.h"

class Entity;

class SceneOne: public Scene
{
private:
    struct HUD
    {
    public:
        std::shared_ptr<Entity> mainScoreNumber{nullptr};
        std::shared_ptr<Entity> mainScoreText{nullptr};
        std::shared_ptr<Entity> upperBar{nullptr};
        std::shared_ptr<Entity> timeNumber{nullptr};
        std::shared_ptr<Entity> timeText{nullptr};
        std::shared_ptr<Entity> fpsNumber{nullptr};
        std::shared_ptr<Entity> fpsText{nullptr};
    };
    std::shared_ptr<Entity> m_player;
    std::shared_ptr<Entity> m_map;
    HUD m_HUD;
    std::shared_ptr<Entity> m_camera;

    int m_time{0};

    // store the window's size
    int windowX, windowY;

    void init() override;
    void endScene() override;

    bool checkEntityCollision(std::shared_ptr<Entity>& one, std::shared_ptr<Entity>& two);
    std::pair<bool, bool> checkInsideEntity(std::shared_ptr<Entity>& one, std::shared_ptr<Entity>& two);

    // systems
    void sMovement();
    void sDoAction(const Action& action) override;
    void sPhysics();
    void sCheckGameState();
    void sUpdateCamera();

    // additional methods for the systems, that are for a specific function
    void playerPhysicsUpdate();
    void reactToMapBorder();
    void spawnEnemy();
    void spawnRect(MATH::Vec2 start, MATH::Vec2 end);
    void spawnPlayer();
    void collisionWithPlayer();
    void checkEnd();
    void createHUD();

public:
    SceneOne() = delete;
    SceneOne(GameEngine* ge): Scene(ge) { init(); };
    ~SceneOne() { endScene(); };
    void update() override;

};

#endif