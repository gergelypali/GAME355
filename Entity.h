#ifndef ENTITY_H
#define ENTITY_H

#include <string>

#include "Component.h"

class Entity
{
private:
    std::tuple<
        CTransform,
        CState,
        CAABB,
        CInput,
        CRectBody,
        CLifetime,
        CScore,
        CTexture,
        CShape2d,
        CSpriteSet,
        CSpriteStack,
        CAnimation,
        CVoxel,
        CText
        > m_components;
    const std::string m_tag = "NONE";
    bool m_active{true};
    const size_t m_id{0};

public:
    Entity() = delete;
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(const std::string& tag, size_t id): m_tag(tag), m_id(id) {};

    const std::string& tag() { return m_tag; };
    bool isActive() { return m_active; };

    void destroy();

    // templated functions
    template<typename T, typename... TArgs>
    void addComponent(TArgs&&... mArgs)
    {
        auto & component = getComponent<T>();
        component = T(std::forward<TArgs>(mArgs)...);
        component.added = true;
    };

    template <typename T>
    void removeComponent()
    {
        auto & component = getComponent<T>();
        component.added = false;
    };

    template <typename T>
    bool hasComponent()
    {
        return getComponent<T>().added;
    };

    template <typename T>
    T& getComponent()
    {
        return std::get<T>(m_components);
    };

};

#endif