#ifndef SCENE_H
#define SCENE_H

#include <map>
#include <string>
#include <memory>

#include "Action.h"

class GameEngine;
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

public:
    Scene() = delete;
    Scene(GameEngine* ge): m_ge(ge) {};
    virtual ~Scene() {};
    virtual void update() = 0;
    virtual void sDoAction(const Action& action) = 0;
    virtual void sRender() = 0;

    void doAction(Action action);
    void registerAction(int i, const std::string& name);

    std::map<int, std::string>& getActionMap() { return m_actionMap; };

    // rendering methods
    void drawRect(std::shared_ptr<Entity> &entity);
    void drawTexture(std::shared_ptr<Entity> &entity);
    void drawSpriteSet(std::shared_ptr<Entity> &entity);
    void drawSpriteStack(std::shared_ptr<Entity> &entity);
    void drawAnimation(std::shared_ptr<Entity> &entity);
};

#endif