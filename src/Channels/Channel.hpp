#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <map>
#include <string>
#include "../Server/Connection.hpp"


#define INVITE_ONLY     1
#define TOPIC_RESTRICT  (1 << 1)
#define KEY_SET         (1 << 2)
#define USER_LIMIT      (1 << 3)

class Channel
{
private:
    std::string                         name;
    std::string                         topic;
    int                                 mode;
    std::map<std::string, Connection&>  members;
    
public:
    Channel();
    ~Channel();

    const std::string&                  getName() const;
    const std::string&                  getTopic() const;
    int                                 getMode() const;
    std::map<std::string, Connection&>  getMembers() const;

    void    setMode(int flags);
    void    setTopic(const std::string& topic);
    void    setName(const std::string& name);
    void    addMember(Connection& client);

    void    broadCastMessage(const std::string& message);
};

#endif
/* 
You must be able to authenticate, set a nickname, a username, join a channel,
send and receive private messages using your reference client

∗ KICK - Eject a client from the channel
∗ INVITE - Invite a client to a channel
∗ TOPIC - Change or view the channel topic
∗ MODE - Change the channel’s mode:
· i: Set/remove Invite-only channel
· t: Set/remove the restrictions of the TOPIC command to channel operators
· k: Set/remove the channel key (password) 
· o: Give/take channel operator privilege
· l: Set/remove the user limit to channe*/