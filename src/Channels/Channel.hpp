#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <map>
#include <string>
#include "../Server/Connection.hpp"
#include "../Replies/Replies.hpp"


#define NOT_INVITED 0
#define INVITED 1

#define INVITE_ONLY     1
#define TOPIC_RESTRICT  (1 << 1)
#define KEY_SET         (1 << 2)
#define USER_LIMIT      (1 << 3)
#define EXTERNAL_MSG    (1 << 4)

class Channel
{
private:
    std::string                         name;
    std::string                         topic;
    std::string                         key;
    int                                 mode;
    std::map<std::string, const Connection&>  members;
    std::map<std::string, const Connection&>  operators;
    
    void    sendListofNames(const Connection& client) const;

public:
    Channel();
    Channel(const std::string& name, const Connection& creator);
    Channel(const std::string& name, const std::string& key, const Connection& creator);
    Channel(const Channel& chan);
    ~Channel();

    const std::string&      getName() const;
    const std::string&      getTopic() const;
    int                     getMode() const;

    const std::map<std::string, const Connection&>&     getMembers() const;
    const std::map<std::string, const Connection&>&     getOperators() const;

    void    setMode(int flags);
    void    setTopic(const std::string& topic);
    void    setName(const std::string& name);
    int     addMember(const Connection& client,
                    const std::string& providedKey,
                    bool invited);
    void    removeMember(const std::string& nickname);
    
    bool    isEmpty() const;
    bool    isUserInChannel(const std::string& nickname) const;
    bool    isUserOperator(const std::string& nickname) const;
    void    broadCastMessage(const std::string& message, const Connection& sender) const;
    void    sendWelcomeMessage(const Connection& client) const;
};

#endif
