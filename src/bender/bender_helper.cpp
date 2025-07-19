#include "Bender.hpp"

std::string    epochToTimeStamp(time_t epochTime)
{
    std::tm*            t;
    std::stringstream   stringStream;
    const std::string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const std::string days[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

    t = std::gmtime(&epochTime);
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


void split(std::string toSplit, char delimiter, std::vector<std::string>& storage)
{
    std::string val;
    std::size_t pos;

    do
    {
        pos = toSplit.find(delimiter);
        val = toSplit.substr(0, pos);
        if (!val.empty())
            storage.push_back(toSplit.substr(0, pos));
        toSplit.erase(0, pos + 1);
    } while (pos != toSplit.npos);
}

bool    isPortValid(char* port)
{
    size_t  atoi;

    if (port[0] == 0)
    {
        std::cout << "Port cannot be empty\n";
        return (false);
    }
    atoi = 0;
    for (size_t i = 0; port[i] != 0; ++i)
    {
        if (!std::isdigit(port[i]))
        {
            std::cout << "Port cannot have non-digit values\n";
            return (false);
        }
        atoi = atoi * 10 + (port[i] - 48);
        if (atoi > __INT_MAX__)
        {
            std::cout << "Port number too big, please insert a smaller number\n";
            return (false);
        }
    }
    if (atoi < 1024)
    {
        std::cout << "Port cannot be below 1024\n";
        return (false);
    }
    return (true);
}

bool validateArgs(int argc, char* argv[], benderArgs* storage)
{
    if (argc < 4)
    {
        std::cout << "Bender usage <Server port> <Server password> <level of holiness [SATAN / BOB / GANDHI]\n";
        return (false);
    }
    if (!isPortValid(argv[1]))
        return (false);
    storage->port = argv[1];
    storage->pass = argv[2];
    if (std::strcmp(argv[3], "SATAN") && std::strcmp(argv[3], "BOB") && std::strcmp(argv[3], "GANDHI"))
    {
        std::cout << "Level of holiness not recognized, defaulting to BOB\n";
        storage->lvl = "BOB";
    }
    else
        storage->lvl = argv[3];
    return (true);
}