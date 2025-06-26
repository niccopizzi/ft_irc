#ifndef BENDER_HPP
#define BENDER_HPP

#include "../server/Server.hpp"


#define GANDHI  2
#define BOB     1
#define SATAN   0

#define PASS        0
#define NICK        1
#define NICK_TRY    2
#define USER        3
#define REGISTERED  4

class Bender
{
private:
    std::string name;
    std::string password;
    const char* port;
    int         levelOfHoliness;
    int         connectionFd;
    int         state;
    int         nickTries;
    pollfd      connectionPoll;
    std::queue<std::string> toSend;

public:
    Bender();
    Bender(const std::string& password, const char* port);
    Bender(const std::string& name, 
            const std::string& password,
            const char* port, 
            int levelOfHoliness);
    Bender(const Bender& bender);
    Bender& operator=(const Bender& other);
    ~Bender();

    char        buffer[513];

    void            connectToServer();
    void            registerToServer();
    void            pollEvents();
    void            handleServerReply();
    void            sendMessageToServer();

    void            enqueueMsg(const std::string& message);
    void            dequeueMsg();
};


#endif
