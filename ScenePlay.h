#ifndef SCENEPLAY_H
#define SCENEPLAY_H

#include "Scene.h"
#include <memory>
#include "Vector.h"

class Entity;

class ScenePlay: public Scene
{
private:
    struct HUD
    {
    public:
        std::shared_ptr<Entity> mainScoreNumber{nullptr};
        std::shared_ptr<Entity> mainScoreText{nullptr};
        std::shared_ptr<Entity> upperBar{nullptr};

    };
    std::shared_ptr<Entity> m_player;
    std::shared_ptr<Entity> m_map;
    HUD m_HUD;
    std::shared_ptr<Entity> m_camera;

    MATH::Vec2 enemySpawnLocs[9] = {
        MATH::Vec2{200, 600},
        MATH::Vec2{200, 200},
        MATH::Vec2{1000, 600},
        MATH::Vec2{500, 400},
        MATH::Vec2{1000, 100},
        MATH::Vec2{100, 300},
        MATH::Vec2{300, 500},
        MATH::Vec2{100, 100},
        MATH::Vec2{1250, 30}
        };

    // store the window's size
    int windowX, windowY;

    void init() override;
    void endScene() override;

    void initMapTiles();
    bool checkEntityCollision(std::shared_ptr<Entity>& one, std::shared_ptr<Entity>& two);
    std::pair<bool, bool> checkInsideEntity(std::shared_ptr<Entity>& one, std::shared_ptr<Entity>& two);
    void changePlayerSkin(const std::string& name, int frameCount);

    // systems
    void sMovement();
    void sDoAction(const Action& action) override;
    void sPhysics();
    void sCheckGameState();
    void sUpdateCamera();

    // additional methods for the systems, that are for a specific function
    void playerPhysicsUpdate();
    void reactToMapBorder();
    void spawnEnemy(int number);
    void spawnPlayer();
    void collisionWithPlayer();
    void checkLifetime();
    void fadeOut();
    void checkEnd();
    void createHUD();

public:
    ScenePlay() = delete;
    ScenePlay(GameEngine* ge): Scene(ge) { init(); };
    ~ScenePlay() { endScene(); };
    void update() override;

};

#endif