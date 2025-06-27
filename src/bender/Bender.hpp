#ifndef BENDER_HPP
#define BENDER_HPP

#include "../server/Server.hpp"


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
    std::string             name;
    std::string             password;
    const char*             port;
    int                     levelOfHoliness;
    int                     connectionFd;
    int                     nickTries;
    pollfd                  connectionPoll;
    std::queue<std::string> toSend;
    std::set<std::string>   channels;

public:
    Bender();
    Bender(const std::string& password, const char* port);
    Bender(const std::string& password,
            const char* port, 
            int levelOfHoliness);
    Bender(const Bender& bender);
    Bender& operator=(const Bender& other);
    ~Bender();

    char            buffer[513];

    void            connectToServer();
    void            pollEvents();
    void            handleServerReply();
    void            handleLocalhostMsg(const std::string& num);
    void            handlePrivateMsg(const std::vector<std::string>& msg);
    void            handleChannelMsg(const std::vector<std::string>& msg);
    void            handleUserAction(const std::vector<std::string>& msg);
    void            handleModeChange(const std::vector<std::string>& msg);

    void            enqueueMsg(const std::string& message);
    void            dequeueMsg();
};


#endif
