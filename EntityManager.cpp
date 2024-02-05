#include "EntityManager.h"
#include "Entity.h"
#include <algorithm>

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
    auto entity = std::make_shared<Entity>(tag, m_totalEntities++);
    m_toAdd.push_back(entity);
    return entity;
}

EntityVector& EntityManager::getEntities()
{
    return m_entities;
}

EntityVector& EntityManager::getEntities(const std::string& tag)
{
    return m_entityMap[tag];
}

void EntityManager::update()
{
    for (auto entity : m_toAdd)
    {
        m_entities.push_back(entity);
        m_entityMap[entity->tag()].push_back(entity);
    }
    EntityVector temp;
    for (auto entity : m_entities)
    {
        if (!entity->isActive())
            temp.push_back(entity);
    }
    for (auto entity : temp)
    {
        m_entityMap[entity->tag()].erase(std::find(m_entityMap[entity->tag()].begin(), m_entityMap[entity->tag()].end(), entity));
        m_entities.erase(std::find(m_entities.begin(), m_entities.end(), entity));
    }
    m_toAdd.clear();
}