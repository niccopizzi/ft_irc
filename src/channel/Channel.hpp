#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <map>
#include <string>
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
    std::string                                         name;
    std::string                                         topic;
    std::string                                         key;
    int                                                 mode;
    int                                                 userLimit;
    std::map<const std::string&, Connection&>           members;
    std::map<const std::string&, const Connection&>     operators;
    std::vector<const Connection*>                      usersInvited; //keep track of invited uers
    
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

    const std::map<const std::string&, Connection&>&     getMembers() const;
    const std::map<const std::string&, const Connection&>&     getOperators() const;

    void    setMode(int flags);
    void    setTopic(const std::string& topic);
    void    setName(const std::string& name);
    void    storeUserInvitation(const Connection* invitee);
    int     addMember(Connection& client,
                    const std::string& providedKey);
    void    removeMember(const std::string& nickname);
    void    addOperator(Connection& newOp);

    std::vector<const Connection*>::iterator    getInvitePos(const Connection* user);
    bool    isEmpty() const;
    bool    isUserInChannel(const std::string& nickname) const;
    bool    isUserOperator(const std::string& nickname) const;
    void    broadCastMessage(const std::string& message, const Connection& sender) const;
    void    sendWelcomeMessage(Connection& client) const;
};

#endif
