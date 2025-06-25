#include "Channel.hpp"

Channel::Channel() :    name(""),
                        topic(""),
                        key(""),
                        mode(0),
                        userLimit(-1)
{
    #ifdef DEBUG
        std::cout << "Channel default constructor called\n";
    #endif
}

Channel::Channel(const std::string& chanName, Connection& creator):
                                name(chanName),
                                topic(""),
                                key(""),
                                mode(0),
                                userLimit(-1)
{
    std::pair<const std::string&, Connection&> theCreator(creator.getNickname(), creator);
    members.insert(theCreator);
    operators.insert(theCreator);
}

Channel::Channel(const std::string& chanName, const std::string& chanKey, Connection& creator) :
                                                            name(chanName),
                                                            key(chanKey),
                                                            mode(FLG_KEY),
                                                            userLimit(-1)
{
    std::pair<const std::string&, Connection&> theCreator(creator.getNickname(), creator);
    members.insert(theCreator);
    operators.insert(theCreator);
    #ifdef DEBUG
        std::cout << "Channel constructor called\n";
    #endif
}

Channel::Channel(const Channel& chan) : name(chan.name),
                                        topic(chan.topic),
                                        key(chan.key),
                                        mode(chan.mode),
                                        userLimit(chan.userLimit),
                                        members(chan.members),
                                        operators(chan.operators)
{
    #ifdef DEBUG
        std::cout << "Channel copy constructor called\n";
    #endif
}

Channel::~Channel()
{
    #ifdef DEBUG
        std::cout << "Channel destructor called\n";
    #endif
}

const std::string&  Channel::getName() const
{
    return (name);
}

const std::string&  Channel::getTopic() const
{
    return (topic);
}

const std::string& Channel::getKey() const
{
    return (key);
}

int Channel::getMode() const
{
    return (mode);
}

int Channel::getUserLimit() const
{
    return (userLimit);
}

const std::map<const std::string&, Connection&>&  Channel::getMembers() const
{
    return (members);
}

const std::map<const std::string&, const Connection&>&   Channel::getOperators() const
{
    return (operators);
}

void    Channel::setName(const std::string& name)
{
    this->name = name;
}

void    Channel::setTopic(const std::string& topic)
{
    this->topic = topic;
}

void    Channel::storeUserInvitation(const Connection* invitee)
{
    usersInvited.push_back(invitee);
}

void    Channel::setMode(int flags)
{
    mode |= flags;
}

bool    Channel::isUserInChannel(const std::string& nickname) const
{
    return (members.find(nickname) != members.end());
}

bool    Channel::isUserOperator(const std::string& nickname) const 
{
    return (operators.find(nickname) != operators.end());
}

std::vector<const Connection*>::iterator    Channel::getInvitePos(const Connection* user)
{
    std::vector<const Connection*>::iterator it;
    std::vector<const Connection*>::iterator end;   

    end = usersInvited.end();
    for (it = usersInvited.begin(); it != end; ++it)
    {
        if (*it == user)
            return (it);
    }
    return (end);
}

bool    Channel::isEmpty() const
{
    return (members.empty());
}

int    Channel::addMember(Connection& client,
                            const std::string& providedKey)
{
    const std::string&  clientNickname = client.getNickname();
    std::vector<const Connection*>::iterator invitePos;

    if (isUserInChannel(clientNickname))
        return (0);
    if ((mode & FLG_KEY) && (providedKey.empty() || providedKey != key))
    {
        client.enqueueMsg(Replies::JoinErrReplies(clientNickname, name, "", ERR_BADCHANNELKEY));
        return (-1);
    }
    if (mode & FLG_INVITE_ONLY)
    {
        invitePos = getInvitePos(&client);
        if (invitePos == usersInvited.end())
        {
            client.enqueueMsg(Replies::JoinErrReplies(clientNickname, name, "", ERR_INVITEONLYCHAN));
            return (-1);
        }
        else //passed invitation check, delete it from the list of invited users
            usersInvited.erase(invitePos);
    }
    if ((mode & FLG_USER_LIMIT) && members.size() == userLimit)
    {
        client.enqueueMsg(":localhost 471 " + client.getNickname() + " " + name + " :Cannot join channel (+l)\r\n");
        return;
    }
    members.insert(std::pair<const std::string&, Connection&>(clientNickname, client));
    sendWelcomeMessage(client);
    broadCastMessage(":" + client.getMask() + " JOIN :" + name + "\r\n", client);
    return (1);
}

void    Channel::removeMember(const std::string& nickname)
{
    operators.erase(nickname);
    members.erase(nickname);
}

void Channel::addOperator(Connection& newOp)
{
    operators.insert(std::make_pair<const std::string&, Connection&>(newOp.getNickname(), newOp));
}

void    Channel::sendListofNames(Connection& client) const
{
    std::string reply = ":localhost 353 " + client.getNickname() + " = " + name + " :";

    for (std::map<const std::string&, Connection&>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (isUserOperator(it->first))
            reply += "@" + it->first;
        else 
            reply += it->first;
        reply += ' ';
    }
    reply += "\r\n";
    client.enqueueMsg(reply);
}

void    Channel::sendWelcomeMessage(Connection& client) const
{
    client.enqueueMsg(":" + client.getMask() + " JOIN " + name + "\r\n");
    if (!topic.empty())
        client.enqueueMsg(":localhost 332 " + name + " :" + topic + "\r\n");
    sendListofNames(client);
    client.enqueueMsg(Replies::JoinWelcomeReplies("", client.getNickname(), name, RPL_ENDOFNAMES));
}

void Channel::broadCastMessage(const std::string& message, const Connection& sender) const
{
    std::map<const std::string&, Connection&>::const_iterator toSkip = members.find(sender.getNickname());

    for (std::map<const std::string&, Connection&>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (it == toSkip)
            continue;
        it->second.enqueueMsg(message);
    }
}