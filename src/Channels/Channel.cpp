#include "Channel.hpp"

Channel::Channel() :    name(""),
                        topic(""),
                        key(""),
                        mode(0)
{
    #ifdef DEBUG
        std::cout << "Channel default constructor called\n";
    #endif
}

Channel::Channel(const std::string& chanName, const Connection& creator):
                                name(chanName),
                                topic(""),
                                key(""),
                                mode(0)
{
    std::pair<const std::string&, const Connection&> theCreator(creator.getNickname(), creator);
    members.insert(theCreator);
    operators.insert(theCreator);
}

Channel::Channel(const std::string& chanName, const std::string& chanKey, const Connection& creator) :
                                                            name(chanName),
                                                            key(chanKey),
                                                            mode(KEY_SET)
{
    std::pair<const std::string&, const Connection&> theCreator(creator.getNickname(), creator);
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

const std::map<const std::string&, const Connection&>&  Channel::getMembers() const
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

int    Channel::addMember(const Connection& client,
                            const std::string& providedKey)
{
    const std::string&  clientNickname = client.getNickname();
    std::vector<const Connection*>::iterator invitePos;

    if (isUserInChannel(clientNickname))
        return (0);
    if ((mode & KEY_SET) != 0 && (providedKey.empty() || providedKey != key))
    {
        client.sendMessage(Replies::JoinErrReplies(clientNickname, name, "", ERR_BADCHANNELKEY));
        return (-1);
    }
    if ((mode & INVITE_ONLY) != 0)
    {
        invitePos = getInvitePos(&client);
        if (invitePos == usersInvited.end())
        {
            client.sendMessage(Replies::JoinErrReplies(clientNickname, name, "", ERR_INVITEONLYCHAN));
            return (-1);
        }
        else //passed invitation check, delete it from the list of invited users
            usersInvited.erase(invitePos);
    }
    members.insert(std::pair<const std::string&, const Connection&>(clientNickname, client));
    sendWelcomeMessage(client);
    broadCastMessage(":" + client.getMask() + " JOIN :" + name + "\r\n", client);
    return (1);
}

void    Channel::removeMember(const std::string& nickname)
{
    operators.erase(nickname);
    members.erase(nickname);
}


void    Channel::sendListofNames(const Connection& client) const
{
    std::string reply = ":localhost 353 " + client.getNickname() + " = " + name + " :";

    for (std::map<const std::string&, const Connection&>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (isUserOperator(it->first))
            reply += "@" + it->first;
        else 
            reply += it->first;
        reply += ' ';
    }
    reply += "\r\n";
    client.sendMessage(reply);
}

void    Channel::sendWelcomeMessage(const Connection& client) const
{
    std::cout << "user mask " << client.getMask() << '\n';
    client.sendMessage(":" + client.getMask() + " JOIN " + name + "\r\n");
    if (!topic.empty())
    {
        client.sendMessage(Replies::JoinWelcomeReplies(topic, client.getNickname(), name, RPL_TOPIC));
    }
    sendListofNames(client);
    client.sendMessage(Replies::JoinWelcomeReplies("", client.getNickname(), name, RPL_ENDOFNAMES));
}

void Channel::broadCastMessage(const std::string& message, const Connection& sender) const
{
    std::map<const std::string&, const Connection&>::const_iterator toSkip = members.find(sender.getNickname());

    for (std::map<const std::string&, const Connection&>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        if (it == toSkip)
            continue;
        it->second.sendMessage(message);
    }
}