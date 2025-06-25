#include "CommandHandler.hpp"

bool    isNotValidFirstChar(char c)
{
    return ((c >= '0' && c <= '9' ) || c == '#' || c == ':' || c == '&'
            || c == '@' || c == '$');
}

bool    isNotValidChar(char c)
{
    return(!std::isalnum(c) 
            && c != '[' && c != ']' 
            && c != '{' && c != '}'
            && c != '\\' && c != '|'
            && c != '_');
}

bool    isNickValid(const std::string& nick)
{
    for (std::string::const_iterator it = nick.begin(); it != nick.end(); ++it)
    {
        if (it == nick.begin() && isNotValidFirstChar(*it))
            return (false);
        else if (isNotValidChar(*it))
            return (false);
    }
    return (true);
}

bool    isNickUsed(const std::string& nickName,
                const std::map<const std::string&, Connection&>& nickToConnection)
{
    return (nickToConnection.find(nickName) != nickToConnection.end());
}


void CommandHandler::notifyUsersInClientChannels(const std::string& message,
                                            const std::map<std::string, Channel>& channels,
                                            const Connection& client)
{
    const std::string& nick = client.getNickname();

    for (std::map<std::string, Channel>::const_iterator it = channels.begin();
            it != channels.end(); ++it)
    {
        if (it->second.isUserInChannel(nick))
        {
            it->second.broadCastMessage(message, client);
        }
    }
}

void CommandHandler::executeNick(const std::string& nickname, 
                                Connection& client,
                                std::map<const std::string&, Connection&>& nickToConnection,
                                std::map<std::string, Channel>& channels)
{
    if (!client.isAuthenticated())
        client.sendMessage(":You need to authenticate first\n");
    if (nickname.empty())
        client.sendMessage(Replies::NickErrReplies("", "", ERR_NONICKNAMEGIVEN));
    else if (!isNickValid(nickname))
        client.sendMessage(Replies::NickErrReplies(nickname, client.getUsername(), ERR_ERRONEUSNICKNAME));
    else if (nickname == client.getNickname())
        return;    
    else if (isNickUsed(nickname, nickToConnection))
        client.sendMessage(Replies::NickErrReplies(nickname, client.getUsername(), ERR_NICKNAMEINUSE));
    else
    {
        if (client.isRegistered())
        {
            client.sendMessage(":" + client.getMask() +  " NICK :" + nickname + "\r\n");
            notifyUsersInClientChannels(":" + client.getMask() +  " NICK :" + nickname + "\r\n", channels, client);
            client.setNickname(nickname);
        }
        else 
        {
            client.setNickname(nickname);
            if (client.isRegistered())
            {
                client.sendMessage(Replies::WelcomeMsg(nickname, client.getMask()));
            }
            nickToConnection.insert(std::pair<const std::string&, Connection&>(client.getNickname(), client));
        }
    }
}

void CommandHandler::executePass(const std::string& toMatch,
                                const std::string& toCheck, 
                                Connection& client)
{
    if (client.isAuthenticated())
    {
        client.sendMessage(Replies::PassErrReplies(ERR_ALREADYREGISTERED));
    }
    else if (toCheck.empty())
    {
        client.sendMessage(Replies::PassErrReplies(ERR_NEEDMOREPARAMS));
    }
    else if (toCheck != toMatch)
    {
        client.sendMessage(Replies::PassErrReplies(ERR_PASSWDMISMATCH));
    }
    else
        client.setAuth(true);
}
    
void CommandHandler::executePing(const std::string& token,
                            Connection& client)
{
    if (!token.empty())
        client.sendMessage("PONG :" + token + "\r\n");
    else
        client.sendMessage("PONG" "\r\n");
}

std::string    catArguments(std::vector<std::string>::const_iterator begin,
                            std::vector<std::string>::const_iterator end)
{
    std::string ret("");

    if ((end - 1)->empty())
        end = end - 1;
    while (begin != end)
    {
        ret += *begin;
        ++begin;
        if ((begin) != end)
            ret += " ";
    }
    if (ret.at(0) == ':')
        ret.erase(0, 1);
    return (ret);
}

void sendPrivateMessage(Connection& sender, 
                        Connection& recipient, 
                        const std::vector<std::string>& args)
{
    std::string incipit = ":" + sender.getMask() + " PRIVMSG " + recipient.getNickname() + " :";
    std::string message = catArguments(args.begin() + 1, args.end());

    message.append("\r\n");
    recipient.sendMessage(incipit + message);
}

void sendChannelMessage(Connection& sender,
                        Channel& channel,
                        const std::vector<std::string>& args)
{
    std::string incipit = ":" + sender.getMask() + " PRIVMSG " + channel.getName() + " :";
    std::string message = catArguments(args.begin() + 1, args.end());

    message.append("\r\n");
    channel.broadCastMessage(incipit + message, sender);
}

void CommandHandler::executePrivMsg(const std::vector<std::string>& args, 
                                Connection& client,
                                std::map<const std::string&, Connection&>& nickToConnection,
                                std::map<std::string, Channel>& channels)
{
    std::map<const std::string&, Connection&>::iterator targetClient;
    std::map<std::string, Channel>::iterator targetChannel;
    std::string  target;

    if (args.empty())
    {
        client.sendMessage(Replies::PrivMsgErrReplies(client.getNickname(), "", ERR_NORECIPIENT));
        return;
    }
    target = args.at(0);
    if (args.size() == 1)
    {
        client.sendMessage(Replies::PrivMsgErrReplies(client.getNickname(), target, ERR_NOTEXTTOSEND));
        return;
    }
    if (*target.begin() == '#' || *target.begin() == '&')
    {   
        targetChannel = channels.find(target);
        if (targetChannel == channels.end())
            client.sendMessage(Replies::PrivMsgErrReplies(client.getNickname(), target, ERR_NOSUCHNICK));
        else if (!targetChannel->second.isUserInChannel(client.getNickname()) && 
                    (targetChannel->second.getMode() & EXTERNAL_MSG) == 0)
            client.sendMessage(Replies::PrivMsgErrReplies(client.getNickname(), target, ERR_CANNOTSENDTOCHAN));
        else
            sendChannelMessage(client, targetChannel->second, args);
    }
    else
    {
        targetClient = nickToConnection.find(target);
        if (targetClient == nickToConnection.end() || !targetClient->second.isRegistered())
            client.sendMessage(Replies::PrivMsgErrReplies(client.getNickname(), target, ERR_NOSUCHNICK));
        else
            sendPrivateMessage(client, targetClient->second, args);
    }
}

void CommandHandler::executeUsername(const std::vector<std::string>& args,
                                Connection& client)
{
    if (!client.isAuthenticated())
        client.sendMessage("You need to authenticate first!\n");
    else if (args.size() < 4)
    {
        client.sendMessage(Replies::UserErrReplies(client.getNickname(), ERR_NEEDMOREPARAMS)); 
    }
    else if (client.isRegistered())
    {
        client.sendMessage(Replies::UserErrReplies(client.getNickname(), ERR_ALREADYREGISTERED));
    }
    else
    {
        client.setUsername(args.at(0));
        client.setFullname(catArguments(args.begin() + 3, args.end()));
        if (client.isRegistered())
        {
            client.sendMessage(Replies::WelcomeMsg(client.getNickname(), client.getMask()));
        }
    }
}

std::vector<std::string>    splitValues(std::string toSplit)
{
    std::vector<std::string> splitted;
    std::size_t pos;

    do
    {
        pos = toSplit.find(',');
        splitted.push_back(toSplit.substr(0, pos));
        toSplit.erase(0, pos + 1);
    } while (pos != toSplit.npos);

    return (splitted);
}

bool    isChanNameValid(const std::string& chanName)
{
    if (chanName.size() <= 1 || chanName.size() > 200)
        return (false);
    if (chanName.at(0) != '#' && chanName.at(0) != '&')
        return (false);
    for (std::string::const_iterator it = chanName.begin();
        it != chanName.end(); ++it)
    {
        if (*it == 7)
            return (false);
    }
    return (true);
}

void    createChannel(const std::string& chanName, 
                        const std::string& key, 
                        Connection& creator,
                        std::map<std::string, Channel>& channels)
{
    std::pair<std::string, Channel> newChannel;

    if (key.empty())
        newChannel = std::pair<std::string, Channel>(chanName, Channel(chanName, creator));
    else
        newChannel = std::pair<std::string, Channel>(chanName, Channel(chanName, key, creator));
    channels.insert(newChannel);
    creator.makeUserOp();
    newChannel.second.sendWelcomeMessage(creator);
}

void    handleJoinArgs(const std::vector<std::string>& channelNames,
                        const std::vector<std::string>& channelKeys,
                        std::map<std::string, Channel>& channels,
                        Connection& client)
{
    std::string chanName;
    std::string chanKey;
    std::map<std::string, Channel>::iterator it;

    for (size_t i = 0; i < channelNames.size(); ++i)
    {
        chanName = channelNames.at(i);
        if (!isChanNameValid(chanName))
        {
            client.sendMessage(Replies::JoinErrReplies(client.getNickname(), chanName, "", ERR_BADCHANMASK));
            return;
        }
        chanKey = "";
        if (i < channelKeys.size())
            chanKey = channelKeys.at(i);
        it = channels.find(chanName);
        if (it != channels.end())
        {
            if (it->second.addMember(client, chanKey) < 0)
                return;
        }
        else
        {
            createChannel(chanName, chanKey, client, channels);
        }
    }
}

void    CommandHandler::executeJoin(
                                    const std::vector<std::string>& args,
                                    Connection& client,
                                    std::map<std::string, Channel>& channels)
{
    std::vector<std::string> channelNames;
    std::vector<std::string> channelKeys;

    if (args.empty() || args.at(0).empty())
    {
        client.sendMessage(Replies::CommonErrReplies(client.getNickname(), "JOIN", ERR_NEEDMOREPARAMS));
    }
    channelNames = splitValues(args.at(0));
    if (args.size() > 1)
        channelKeys = splitValues(args.at(1));
    handleJoinArgs(channelNames, channelKeys, channels, client);
}


void    removeUsers(std::vector<std::string>& usersToKick,
                    const std::string& reason,
                    Channel& channel, 
                    Connection& kicker,
                    std::map<const std::string&, Connection&> nickToConnection)
{
    std::map<const std::string&, Connection&>::iterator toKick;
    std::string theReason;
    std::string message = ":" + kicker.getMask() + " KICK " + channel.getName() + " ";

    for (std::vector<std::string>::iterator it = usersToKick.begin(); it != usersToKick.end(); ++it)
    {
        toKick = nickToConnection.find(*it);
        if (toKick == nickToConnection.end())
        {
            kicker.sendMessage(Replies::KickErrReplies(kicker.getNickname(), *it, channel.getName(), ERR_NOSUCHNICK));
            return;
        }
        else if (!channel.isUserInChannel(toKick->first))
        {
            kicker.sendMessage(Replies::KickErrReplies(kicker.getNickname(), *it, channel.getName(), ERR_USERNOTINCHANNEL));
            return;
        }
        else
        {
            theReason = reason.empty() ? *it : reason;
            channel.broadCastMessage(message + *it + " :" + (theReason) + "\r\n", kicker);
            kicker.sendMessage(message + *it + theReason + "\r\n");
            channel.removeMember(toKick->first);
        }
    }
}

void CommandHandler::executeKick(const std::vector<std::string>& args,
                            Connection& kicker,
                            std::map<std::string, Channel>& channels,
                            std::map<const std::string&, Connection&> nickToConnection)
{
    std::map<std::string, Channel>::iterator it;
    std::vector<std::string>  usersToKick;

    usersToKick.reserve(5);
    if (args.size() < 2)
        kicker.sendMessage(Replies::CommonErrReplies(kicker.getNickname(), "KICK", ERR_NEEDMOREPARAMS));
    else if ((it = channels.find(args.at(0))) == channels.end())
        kicker.sendMessage(Replies::KickErrReplies(kicker.getNickname(), "", args.at(0), ERR_NOSUCHCHANNEL));
    else if (!it->second.isUserInChannel(kicker.getNickname()))
        kicker.sendMessage(Replies::KickErrReplies(kicker.getNickname(), "", it->first, ERR_NOTONCHANNEL));
    else if (!it->second.isUserOperator(kicker.getNickname()))
        kicker.sendMessage(Replies::KickErrReplies(kicker.getNickname(), "", it->first, ERR_CHANOPRIVSNEEDED));
    else
    {
        usersToKick = splitValues(args.at(1));
        if (args.size() >= 3 && !args.at(2).empty())
        {
            std::string reason = catArguments(args.begin() + 2, args.end());
            removeUsers(usersToKick, reason, it->second, kicker, nickToConnection);
        }
        else
            removeUsers(usersToKick, "", it->second, kicker, nickToConnection);
        if (it->second.isEmpty())
            channels.erase(it->first); //remove channel if the last users kicked himself out
    }
}

void    handleInvitation(const Connection& inviter,
                        Channel& channel, Connection& userInvited)
{
    channel.storeUserInvitation(&userInvited);
    inviter.sendMessage(":" + inviter.getMask() + " INVITE " + userInvited.getNickname() + " " + channel.getName() + "\r\n");
    userInvited.sendMessage(":" + inviter.getMask() + " INVITE " + userInvited.getNickname() + " " + channel.getName() + "\r\n");

}

void CommandHandler::executeInvite(const std::vector<std::string>& args,
                            Connection& inviter,
                            std::map<std::string, Channel>& channels,
                            std::map<const std::string&, Connection&>& nickToConnection)
{
    std::map<std::string, Channel>::iterator            targetChan;
    std::map<const std::string&, Connection&>::iterator targetClient;

    if (args.size() < 2)
    {
        inviter.sendMessage(Replies::CommonErrReplies(inviter.getNickname(), "INVITE", ERR_NEEDMOREPARAMS));
        return;
    }
    targetClient = nickToConnection.find(args.at(0));
    if (targetClient == nickToConnection.end())
    {
        inviter.sendMessage(Replies::InviteErrReplies(inviter.getNickname(), args.at(0), "", ERR_NOSUCHNICK));
    }
    targetChan = channels.find(args.at(1));
    if (targetChan == channels.end())
    {
        inviter.sendMessage(Replies::InviteErrReplies(inviter.getNickname(), "", args.at(1), ERR_NOSUCHCHANNEL));
        return;
    }
    if (!targetChan->second.isUserInChannel(inviter.getNickname()))
    {
        inviter.sendMessage(Replies::InviteErrReplies(inviter.getNickname(), "", args.at(1), ERR_NOTONCHANNEL));
        return;
    }
    if (targetChan->second.isUserInChannel(targetClient->first))
    {
        inviter.sendMessage(Replies::InviteErrReplies(inviter.getNickname(), targetClient->first, targetChan->first, ERR_USERONCHANNEL));
        return;
    }
    if ((targetChan->second.getMode() & INVITE_ONLY) && !targetChan->second.isUserOperator(inviter.getNickname()))
    {
        inviter.sendMessage(Replies::InviteErrReplies(inviter.getNickname(), targetClient->first, targetChan->first, ERR_CHANOPRIVSNEEDED));
        return;
    }
    handleInvitation(inviter, targetChan->second, targetClient->second);
}