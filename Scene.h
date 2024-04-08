#ifndef SCENE_H
#define SCENE_H

#include <map>
#include <string>
#include <memory>
#include "GameEngine.h"

#include "Action.h"

class EntityManager;
class Entity;

class Scene
{
protected:
    GameEngine* m_ge{nullptr};
    std::shared_ptr<EntityManager> m_em;
    int m_currentFrame{0};
    std::map<int, std::string> m_actionMap;

    virtual void init() = 0;
    virtual void endScene() = 0;

    bool checkEntityCollision(std::shared_ptr<Entity> &one, std::shared_ptr<Entity> &two);
    std::pair<bool, bool> checkInsideEntity(std::shared_ptr<Entity>& one, std::shared_ptr<Entity>& two);
    std::pair<bool, bool> checkPointInsideEntity(MATH::Vec2& point, std::shared_ptr<Entity>& entity);
    void checkEntityLifetime(std::shared_ptr<Entity> &entity);

public:
    Scene() = delete;
    Scene(GameEngine* ge);
    virtual ~Scene() {};
    virtual void update() = 0;
    virtual void sDoAction(const Action& action) = 0;
    void sRender();

    void doAction(Action action);
    void registerAction(int i, const std::string& name);

    std::map<int, std::string>& getActionMap() { return m_actionMap; };

    // rendering methods
    void drawRect(std::shared_ptr<Entity> &entity);
    void drawShape2d(std::shared_ptr<Entity> &entity);
    void drawTexture(std::shared_ptr<Entity> &entity);
    void drawSpriteSet(std::shared_ptr<Entity> &entity);
    void drawSpriteStack(std::shared_ptr<Entity> &entity);
    void drawVoxel(std::shared_ptr<Entity> &entity);
    void drawAnimation(std::shared_ptr<Entity> &entity);
    void drawText(std::shared_ptr<Entity> &entity);
};

#endif