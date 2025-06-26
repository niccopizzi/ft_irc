#include "Channel.hpp"

Channel::Channel() :    mode(0),
                        userLimit(-1),
                        name(""),
                        topic(""),
                        key("")
{
    #ifdef DEBUG
        std::cout << "Channel default constructor called\n";
    #endif
}

Channel::Channel(const std::string& chanName, Connection& creator):
                                                mode(0),
                                                userLimit(-1),
                                                name(chanName),
                                                topic(""),
                                                key("")
{
    std::pair<connectionID, Connection*> theCreator(creator.getConnectionId(), &creator);
    members.insert(theCreator);
    operators.insert(theCreator.first);
}

Channel::Channel(const std::string& chanName, const std::string& 
                chanKey, Connection& creator) :
                                                mode(FLG_KEY),
                                                userLimit(-1),
                                                name(chanName),
                                                key(chanKey)
{
    std::pair<connectionID, Connection*> theCreator(creator.getConnectionId(), &creator);
    members.insert(theCreator);
    operators.insert(theCreator.first);
    #ifdef DEBUG
        std::cout << "Channel constructor called\n";
    #endif
}

Channel::Channel(const Channel& chan) : mode(chan.mode),
                                        userLimit(chan.userLimit),
                                        name(chan.name),
                                        topic(chan.topic),
                                        key(chan.key),
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

const std::map<connectionID, Connection*>& Channel::getMembers() const
{
    return (members);
}

const std::set<connectionID>& Channel::getOperators() const
{
    return (operators);
}

void    Channel::setName(const std::string& name)
{
    this->name = name;
}

void    Channel::setTopic(const std::string& topic, Connection& changer)
{
    this->topic = topic;
    broadCastMessage(":" + changer.getMask() + " TOPIC " + name + " :" + topic + "\r\n");
}

void    Channel::setKey(const std::string& newKey)
{
    this->key = newKey;
    mode |= FLG_KEY;
}

void Channel::setUserLimit(int newLimit)
{
    if (newLimit > 0)
        userLimit = newLimit;
    mode |= FLG_USER_LIMIT;
}

void    Channel::storeUserInvitation(const Connection* invitee)
{
    usersInvited.insert(invitee->getConnectionId());
}

void    Channel::setMode(int flags)
{
    mode |= flags;
}

bool    Channel::isUserInChannel(connectionID id) const
{
    return (members.find(id) != members.end());
}

bool    Channel::isUserOperator(connectionID id) const 
{
    return (operators.find(id) != operators.end());
}

bool    Channel::isEmpty() const
{
    return (members.empty());
}

int    Channel::addMember(Connection& client,
                            const std::string& providedKey)
{
    const std::string&      clientNickname = client.getNickname();
    const connectionID&     clientId = client.getConnectionId();

    if (isUserInChannel(clientId))
        return (0);
    if ((mode & FLG_KEY) && (providedKey.empty() || providedKey != key))
    {
        client.enqueueMsg(Replies::JoinErr(clientNickname, name, "", ERR_BADCHANNELKEY));
        return (-1);
    }
    if ((mode &FLG_USER_LIMIT) && (int)members.size() == userLimit)
    {
        client.enqueueMsg(":localhost 471 " + clientNickname + " " + name + " :Cannot join channel (+l)\r\n");
        return (-1);
    }
    if (mode & FLG_INVITE_ONLY)
    {
        if (usersInvited.find(clientId) == usersInvited.end())
        {
            client.enqueueMsg(Replies::JoinErr(clientNickname, name, "", ERR_INVITEONLYCHAN));
            return (-1);
        }
        else //passed invitation check, delete it from the list of invited users
            usersInvited.erase(clientId);
    }
    members.insert(std::pair<const connectionID, Connection*>(clientId, &client));
    sendWelcomeMessage(client);
    sendChanMessage(":" + client.getMask() + " JOIN :" + name + "\r\n", client);
    return (1);
}

void    Channel::removeMember(const connectionID clientId)
{
    operators.erase(clientId);
    members.erase(clientId);
}

void Channel::addOperator(Connection& newOp)
{
    operators.insert(newOp.getConnectionId());
}

void Channel::removeOperator(Connection& op)
{
    operators.erase(op.getConnectionId());
}

void    Channel::sendListofNames(Connection& client) const
{
    std::string reply = ":localhost 353 " + client.getNickname() + " = " + name + " :";

    for (std::map<const connectionID, Connection*>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (isUserOperator(it->first))
            reply += "@" + it->second->getNickname();
        else 
            reply += it->second->getNickname();
        reply += ' ';
    }
    reply += "\r\n";
    client.enqueueMsg(reply);
}

void    Channel::sendWelcomeMessage(Connection& client) const
{
    client.enqueueMsg(":" + client.getMask() + " JOIN " + name + "\r\n");
    if (!topic.empty())
        client.enqueueMsg(":localhost 332 " + client.getMask() + " " + name + " :" + topic + "\r\n");
    sendListofNames(client);
    client.enqueueMsg(Replies::JoinWelcome("", client.getNickname(), name, RPL_ENDOFNAMES));
}

void Channel::broadCastMessage(const std::string& message) const
{
    std::map<const connectionID, Connection*>::const_iterator it;
    std::map<const connectionID, Connection*>::const_iterator end = members.end();

    for (it = members.begin(); it != end; ++it)
    {
        it->second->enqueueMsg(message);
    }
}

void Channel::sendChanMessage(const std::string& message, const Connection& sender) const
{
    std::map<const connectionID, Connection*>::const_iterator toSkip = members.find(sender.getConnectionId());
    std::map<const connectionID, Connection*>::const_iterator it;
    std::map<const connectionID, Connection*>::const_iterator end = members.end();

    for (it = members.begin(); it != end; ++it)
    {
        if (it == toSkip)
            continue;
        it->second->enqueueMsg(message);
    }
}

void Channel::sendModeMessage(Connection& asker) const
{
    std::string incipit(":localhost 324 " + asker.getNickname() + " " + name + " :");
    std::string modestring("+");

    if (mode & FLG_INVITE_ONLY)
        modestring.push_back('i');
    if (mode & FLG_TOPIC_RESTRICT)
        modestring.push_back('t');
    if (mode & FLG_KEY)
        modestring.push_back('k');
    if (mode & FLG_USER_LIMIT)
        modestring.push_back('l');
    if (mode & FLG_EXTERNAL_MSG)
        modestring.push_back('n');
    asker.enqueueMsg(incipit + modestring + "\r\n");
}

void Channel::sendTopic(Connection& asker) const
{
    if (topic.empty())
        asker.enqueueMsg(":localhost 331 " + asker.getMask() + " " + name + " :No topic is set.\r\n");
    else
        asker.enqueueMsg(":localhost 332 " + asker.getMask() + " " + name + " :" + topic + "\r\n");
}

void Channel::unsetChanMode(char flag, Connection& unsetter)
{
    int newMask;

    switch (flag)
    {
    case 'k':
        newMask = mode & (~FLG_KEY);
        break;
    case 'l':
        newMask = mode & (~FLG_USER_LIMIT);
        break;
    case 'i':
        newMask = mode & (~FLG_INVITE_ONLY);
        break;
    case 't':
        newMask = mode & (~FLG_TOPIC_RESTRICT);
        break;
    case 'n':
        newMask = mode & (~FLG_EXTERNAL_MSG);
        break;
    default:
        return;
    }
    if (mode == newMask)
        return;
    mode = newMask;
    broadCastMessage(":" + unsetter.getMask() + " MODE " + name + " :-" + flag + "\r\n");
}

void Channel::setChanMode(char flag, Connection& setter, const std::string& arg)
{
    int limit;
    std::string msg(":" + setter.getMask() + " MODE " + name + " :+" + flag);

    switch (flag)
    {
    case 'k':
        setKey(arg);
        msg += " " + arg; 
        break;
    case 'l':
        limit = std::atoi(arg.data());
        if (limit < 1)
            return;
        setUserLimit(limit);
        msg += " " + arg;
        break;
    case 'i':
        mode |= FLG_INVITE_ONLY;
        break;
    case 't':
        mode |= FLG_TOPIC_RESTRICT;
        break;
    case 'n':
        mode |= FLG_EXTERNAL_MSG;
        break;
    default:
        return;
    }
    msg.append("\r\n");
    broadCastMessage(msg);
}
