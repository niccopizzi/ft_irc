#ifndef BENDER_HPP
#define BENDER_HPP

#include "Server.hpp"
#include "./infochan/ChannelInfo.hpp"

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
    Bender(const Bender& bender);
    Bender& operator=(const Bender& other);
    
protected:
    std::string                             name;
    std::string                             password;
    const char*                             port;
    int                                     connectionFd;
    int                                     nickTries;
    pollfd                                  connectionPoll;
    std::queue<std::string>                 toSend;
    std::set<std::string>                   channelsOnServer;
    std::set<std::string>                   channelsOperated;
    std::map<std::string, ChannelInfo*>     channelsJoined;
    
public:
    Bender(const std::string& password, const std::string& name, const char* port);
    virtual ~Bender();

    char            buffer[513];

    void            connectToServer();
    void            pollEvents();
    void            handleServerReply();
    void            handleNickChange(const std::vector<std::string>& msg);
    void            handleJoin(const std::vector<std::string>& msg);
    void            handleServerMessage(const std::vector<std::string>& msg);
    void            handleUserAction(const std::vector<std::string>& msg);
    void            storeChannelUsers(const std::vector<std::string>& args);
    void            storeChannel(const std::vector<std::string>& args);
    void            changeNick();

    virtual void            handlePrivateMsg(const std::vector<std::string>& msg) = 0; //interactions specific to the different benders
    virtual void            handleChannelMsg(const std::vector<std::string>& msg) = 0; //interactions specific to the different benders
    virtual void            handleModeChange(const std::vector<std::string>& msg) = 0; //interactions specific to the different benders

    void            enqueueMsg(const std::string& message);
    void            dequeueMsg();

    void            addChannelToOperatedSet(const std::string& chanName);
    void            printBender() const;

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
std::string    epochToTimeStamp(time_t epochTime);

#endif
