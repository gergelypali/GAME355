#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>

// TODO: can use a c++20 new header std::source_location for more precise logging with more information about the given line of code
class Logger
{
public:
    enum severity{
        VERBOSE = 0,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

private:
    static Logger* m_instance;
    std::ofstream outToFile{};

    severity m_logLevel{severity::WARNING};

    void theWrite(const std::string& msg)
    {
        outToFile << msg;
    };

public:
    // rule of five maybe we dont need it now, because it is only needed when we have a custom destructor or copy, move operator in the class
    // copy constructor
    Logger(Logger &other) = delete;
    // copy assignment
    Logger& operator=(const Logger&) = delete;
    /*
    // move constructor
    Logger(Logger &&other) = delete;
    // move assignment
    Logger& operator=(Logger&&) = delete;
    */

    static Logger* Instance();

    void setLogLevel(severity newLevel) { m_logLevel = newLevel; };

    void writeToFile(const std::string& msg, severity sev)
    {
        if (m_logLevel > sev)
            return;
        if (!outToFile.is_open())
            return;

        switch(sev)
        {
            case severity::VERBOSE:
                theWrite("VERBOSE: ");
                break;
            case severity::DEBUG:
                theWrite("DEBUG: ");
                break;
            case severity::INFO:
                theWrite("INFO: ");
                break;
            case severity::WARNING:
                theWrite("!WARNING!: ");
                break;
            case severity::ERROR:
                theWrite("!!ERROR!: ");
                break;
            case severity::CRITICAL:
                theWrite("!!!CRITICAL!: ");
                break;
            default:
                break;
        }
        theWrite(msg);
        outToFile << std::endl;
    };

    void logVerbose(const std::string& msg)
    {
        writeToFile(msg, severity::VERBOSE);
    };

    void log(const std::string& msg)
    {
        writeToFile(msg, severity::DEBUG);
    };

    void logInfo(const std::string& msg)
    {
        writeToFile(msg, severity::INFO);
    };

    void logWarning(const std::string& msg)
    {
        writeToFile(msg, severity::WARNING);
    };

    void logError(const std::string& msg)
    {
        writeToFile(msg, severity::ERROR);
    };

    void logCritical(const std::string& msg)
    {
        writeToFile(msg, severity::CRITICAL);
    };

protected:
    Logger()
    {
        outToFile.open("logFile.txt", std::ios::out | std::ios::trunc);
    };
    ~Logger()
    {
        outToFile.close();
    };

};

#endif