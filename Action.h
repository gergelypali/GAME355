#ifndef ACTION_H
#define ACTION_H

#include <string>

class Action
{
private:
    const std::string m_name = "NONE";
    const std::string m_type = "NONE";

public:
    Action() = delete;
    Action(const std::string& name, const std::string& type): m_name(name), m_type(type) {};

    const std::string& name() const { return m_name; };
    const std::string& type() const { return m_type; };

};

#endif