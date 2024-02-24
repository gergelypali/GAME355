#include "Logger.h"

Logger* Logger::m_instance = 0;
Logger* Logger::Instance() {
    if (m_instance == 0)
        m_instance = new Logger;
    return m_instance;
};
