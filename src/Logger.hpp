#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iomanip>
#include <fstream>
#include <sstream>
#include <ctime>

class Logger
{
private:
    std::ofstream logfile;

public:
    Logger();
    Logger(const char* filepath);

    ~Logger();

    void logCommand(const std::string& command, const std::string& args, int connId);
    void logMessage(const std::string& event_value, const std::string& msg_value, int connId);
    void logConnection(const std::string& event, int connId);
    void log(const std::string& caller, const std::string& event);
};

std::string    getTimeStamp(void);

#endif // LOGGER.HPP

