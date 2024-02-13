#ifndef ACTION_H
#define ACTION_H

#include <string>
#include <SDL.h>

class Action
{
private:
    const std::string m_name = "NONE";
    const std::string m_type = "NONE";
    const SDL_Event m_event;

public:
    Action() = delete;
    Action(const std::string& name, const std::string& type, SDL_Event& event): m_name(name), m_type(type), m_event(event) {};

    const std::string& name() const { return m_name; };
    const std::string& type() const { return m_type; };
    const SDL_Event& event() const { return m_event; };

};

#endif