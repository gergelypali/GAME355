#include "Entity.h"
#include "Component.h"

void Entity::destroy()
{
    m_active = false;
}
