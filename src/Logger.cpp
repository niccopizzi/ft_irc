#ifdef LOG

#include "Logger.hpp"
#include <iostream>

Logger::Logger() : logfile("/var/log/default_logger.log", std::ios::trunc)
{
    logfile << "[" << getTimeStamp() << "]" << " New log session started" << std::endl;
}

Logger::Logger(const char* filepath) : logfile(filepath, std::ios_base::app)
{
    if (logfile.bad())
        std::cout << "Error in opneing the file\n";
    logfile <<"\n" << "[" << getTimeStamp() << "]" << " New log session started" << std::endl;
}
Logger::~Logger()
{
    logfile << "[" << getTimeStamp() << "]" << " Log session ended" << std::endl;
    logfile.close();
}

void Logger::logMessage(const std::string& event_value, const std::string& msg_value, int connId)
{
    logfile << "[" << getTimeStamp() << "]";
    logfile << "\n{\n  'caller': 'server',\n  'event': '" << event_value 
            << "',\n  'message': '" << msg_value << "',\n  'Connection ID': '" << connId;
    logfile << "'\n}" << std::endl; 
}

void Logger::logCommand(const std::string& command, const std::string& args, int connId)
{
    logfile << "[" << getTimeStamp() << "]";
    logfile << "\n{\n  'caller': 'server',\n  'event': 'command_received',\n"
            << "  'command': '" << command << "',\n"
            <<  "  'args': '" << args << "',\n  'Connection ID': '" << connId;
    logfile << "'\n}" << std::endl;
}

void Logger::log(const std::string& caller, const std::string& event_value)
{
    logfile << "[" << getTimeStamp() << "]";
    logfile << "\n{\n  'caller': '" << caller << "',\n  'event': '" << event_value; 
    logfile << "'\n}" << std::endl; 
}
void Logger::logConnection(const std::string& event_value, int connId)
{
    logfile << "[" << getTimeStamp() << "]";
    logfile << "\n{\n  'caller': 'server',\n  'event': '" << event_value;
    logfile << "',\n  'Connection ID': '" << connId;
    logfile << "'\n}" << std::endl;
}

std::string    getTimeStamp(void)
{
    std::time_t         currtime;
    std::tm*            t;
    std::stringstream   stringStream;
    const std::string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const std::string days[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

    currtime = std::time(NULL);
    t = std::gmtime(&currtime);
    if (t->tm_wday < 1 || t->tm_wday > 7 || t->tm_mon < 1 || t->tm_mon > 12)
    {
        return (std::string("Error"));
    }
    stringStream << days[t->tm_wday - 1] << ", " << std::setfill('0') <<
                std::setw(2) << t->tm_mday << " ";
    stringStream << months[t->tm_mon - 1] <<  " " << t->tm_year + 1900 << " ";
    stringStream << std::setfill('0') << std::setw(2) << t->tm_hour << ":"; 
    stringStream << std::setfill('0') << std::setw(2) << t->tm_min << ":";
    stringStream << std::setfill('0') << std::setw(2) << t->tm_sec << " " << t->tm_zone;
    return (stringStream.str());
}

#endif