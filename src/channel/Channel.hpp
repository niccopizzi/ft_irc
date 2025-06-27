#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <map>
#include <string>
#include <set>
#include "../server/Connection.hpp"
#include "../replies/Replies.hpp"

#define FLG_INVITE_ONLY     1
#define FLG_TOPIC_RESTRICT  (1 << 1)
#define FLG_KEY             (1 << 2)
#define FLG_USER_LIMIT      (1 << 3)
#define FLG_EXTERNAL_MSG    (1 << 4)

class Channel
{
private:
    int                                                 mode;
    int                                                 userLimit;
    std::string                                         name;
    std::string                                         topic;
    std::string                                         key;
    std::map<connectionID, Connection*>                 members;
    std::set<connectionID>                              operators;
    std::set<connectionID>                              usersInvited; //keep track of invited user, better to store the nick invited or the id of the connection?
    
    void    sendListofNames(Connection& client) const;

public:
    Channel();
    Channel(const std::string& name, Connection& creator);
    Channel(const std::string& name, const std::string& key, Connection& creator);
    Channel(const Channel& chan);
    ~Channel();

    const std::string&      getName() const;
    const std::string&      getTopic() const;
    const std::string&      getKey() const;
    int                     getMode() const;
    int                     getUserLimit() const;
    std::string             getNamesList() const;

    const std::map<connectionID, Connection*>&        getMembers() const;
    const std::set<connectionID>&                     getOperators() const;

    void    setKey(const std::string& newKey);
    void    setMode(int flags);
    void    setTopic(const std::string& topic, Connection& changer);
    void    setName(const std::string& name);
    void    setUserLimit(int newLimit);
    void    storeUserInvitation(const Connection* invitee);
    int     addMember(Connection& client, const std::string& providedKey);
    void    removeMember(const connectionID clientId);
    void    addOperator(Connection& newOp);
    void    removeOperator(Connection& op);
    
    
    bool    isEmpty() const;
    bool    isUserInChannel(connectionID id) const;
    bool    isUserOperator(connectionID id) const;
    void    broadCastMessage(const std::string& message) const;
    void    sendChanMessage(const std::string& message, const Connection& sender) const;
    void    sendWelcomeMessage(Connection& client) const;
    void    sendModeMessage(Connection& asker) const;
    void    sendTopic(Connection& asker) const;
    void    unsetChanMode(char flag, Connection& unsetter);
    void    setChanMode(char flag, Connection& setter, const std::string& arg);
};

#endif
