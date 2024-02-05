#ifndef ENTITYMANAGER_H
#define ENTITYMANAGER_H

#include <vector>
#include <memory>
#include <map>
#include <string>

class Entity;

typedef std::vector<std::shared_ptr<Entity>> EntityVector;
typedef std::map<std::string, EntityVector> EntityMap;

class EntityManager
{
private:
    EntityVector m_entities;
    EntityMap m_entityMap;
    size_t m_totalEntities{0};
    EntityVector m_toAdd;

    void init();

public:
    EntityManager() {};

    void update();
    std::shared_ptr<Entity> addEntity(const std::string& tag);
    EntityVector& getEntities();
    EntityVector& getEntities(const std::string& tag);

};

#endif