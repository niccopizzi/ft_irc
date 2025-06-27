#ifndef BENDER_HPP
#define BENDER_HPP

#include "../server/Server.hpp"

#include <sstream>
#include <iomanip>

#define GANDHI  2
#define BOB     1
#define SATAN   0

struct modeArgs
{
    std::string channel;
    std::string modeChanged;
    std::string target;
};

class Bender
{
private:
    Bender();
    
protected:
    std::string                                         name;
    std::string                                         password;
    const char*                                         port;
    int                                                 connectionFd;
    int                                                 nickTries;
    pollfd                                              connectionPoll;
    std::queue<std::string>                             toSend;
    std::set<std::string>                               channels;
    std::map<std::string, std::set<std::string> >       channelToUsers;
    
public:
    Bender(const std::string& password, const std::string& name, const char* port);
    Bender(const Bender& bender);
    Bender& operator=(const Bender& other);
    virtual ~Bender();

    char            buffer[513];

    void            connectToServer();
    void            pollEvents();
    void            handleServerReply();
    void            handleServerMessage(const std::vector<std::string>& msg);
    void            handleUserAction(const std::vector<std::string>& msg);
    void            storeChannelUsers(const std::vector<std::string>& args);
    void            storeChannel(const std::vector<std::string>& args);

    virtual void            handlePrivateMsg(const std::vector<std::string>& msg) = 0; //interactions specific to the different benders
    virtual void            handleChannelMsg(const std::vector<std::string>& msg) = 0; //interactions specific to the different benders
    virtual void            handleModeChange(const std::vector<std::string>& msg) = 0; //interactions specific to the different benders

    void            enqueueMsg(const std::string& message);
    void            dequeueMsg();
};


struct benderArgs
{
    const char* lvl;
    const char* pass;
    const char* port;
};

void           split(std::string toSplit, char delimiter, std::vector<std::string>& storage);
bool           validateArgs(int argc, char* argv[], benderArgs* storage);
std::string    getTimeStamp(void);

#endif
