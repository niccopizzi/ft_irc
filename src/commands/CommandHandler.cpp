#include "CommandHandler.hpp"

void split(std::string toSplit, char delimiter, std::vector<std::string>& storage)
{
    std::string val;
    std::size_t pos;

    do
    {
        pos = toSplit.find(delimiter);
        val = toSplit.substr(0, pos);
        if (!val.empty())
            storage.push_back(toSplit.substr(0, pos));
        toSplit.erase(0, pos + 1);
    } while (pos != toSplit.npos);
}

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
    std::string::const_iterator it = nick.begin();
    std::string::const_iterator end = nick.end();

    for (; it != end; ++it)
    {
        if (it == nick.begin() && isNotValidFirstChar(*it))
            return (false);
        if (isNotValidChar(*it))
            return (false);
    }
    return (true);
}

bool    isNickUsed(const std::string& nickName,
                const std::map<const std::string, Connection&>& nickToConnection)
{
    return (nickToConnection.find(nickName) != nickToConnection.end());
}


void CommandHandler::notifyUsersInClientChannels(const std::string& message,
                                            const std::map<const std::string, Channel>& channels,
                                            const Connection& client)
{
    const connectionID clientId = client.getConnectionId();
    std::map<std::string, Channel>::const_iterator it = channels.begin();
    std::map<std::string, Channel>::const_iterator end = channels.end();

    for (; it != end; ++it)
    {
        if (it->second.isUserInChannel(clientId))
        {
            it->second.sendChanMessage(message, client);
        }
    }
}

bool    validateNickArgs(const std::vector<std::string>* args, Connection& client,
                        std::map<const std::string, Connection&>& nickToConnection)
{
    if (!client.isAuthenticated())
    {
        client.enqueueMsg(":localhost :You need to authenticate first!\r\n");
        return (false);
    }
    if (args == NULL)
    {
        client.enqueueMsg(Replies::NickErr("", "", ERR_NONICKNAMEGIVEN));
        return (false);
    }
    const std::string& nickname = args->at(0);
    if (!isNickValid(nickname))
    {
        client.enqueueMsg(Replies::NickErr(nickname, client.getUsername(), ERR_ERRONEUSNICKNAME));
        return (false);
    }
    if (nickname == client.getNickname())
        return (false);
    if (isNickUsed(nickname, nickToConnection))
    {
        client.enqueueMsg(Replies::NickErr(nickname, client.getUsername(), ERR_NICKNAMEINUSE));
        return (false);
    }
    return (true);
}

void CommandHandler::executeNick(const std::vector<std::string>* args, 
                            Connection& client,
                            std::map<const std::string, Connection&>& nickToConnection,
                            std::map<const std::string, Channel>& channels)
{
    if (!validateNickArgs(args, client, nickToConnection))
        return;
    
    const std::string& nickname = args->at(0);
    if (client.isRegistered())
    {
        nickToConnection.erase(client.getNickname());
        const std::string message(":" + client.getMask() + " NICK :" + nickname + "\r\n");
        client.enqueueMsg(message);
        notifyUsersInClientChannels(message, channels, client);
        client.setNickname(nickname);
    }
    else 
    {
        client.setNickname(nickname);
        if (client.isRegistered())
        {
            client.enqueueMsg(Replies::WelcomeMsg(nickname, client.getMask()));
        }
    }
    nickToConnection.insert(std::pair<const std::string, Connection&>(nickname, client));
}

void CommandHandler::executeUsername(const std::vector<std::string>* args, Connection& client)
{
    if (!client.isAuthenticated())
        client.enqueueMsg(":localhost :You need to authenticate first!\r\n");
    else if (args == NULL || args->size() < 4)
    {
        client.enqueueMsg(Replies::UserErr(client.getNickname(), ERR_NEEDMOREPARAMS)); 
    }
    else if (client.isRegistered())
    {
        client.enqueueMsg(Replies::UserErr(client.getNickname(), ERR_ALREADYREGISTERED));
    }
    else
    {
        client.setUsername(args->at(0));
        client.setFullname(catArguments(args->begin() + 3, args->end()));
        if (client.isRegistered())
        {
            client.enqueueMsg(Replies::WelcomeMsg(client.getNickname(), client.getMask()));
        }
    }
}


void CommandHandler::executePass(const std::vector<std::string>* args,
                            Connection& client,
                            const std::string& toMatch)
{
    if (client.isAuthenticated())
    {
        client.enqueueMsg(Replies::PassErr(ERR_ALREADYREGISTERED));
    }
    else if (args == NULL)
    {
        client.enqueueMsg(Replies::PassErr(ERR_NEEDMOREPARAMS));
    }
    else if (args->at(0) != toMatch)
    {
        client.enqueueMsg(Replies::PassErr(ERR_PASSWDMISMATCH));
    }
    else
        client.setAuth(true);
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
                        std::map<const std::string, Channel>& channels)
{
    std::pair<std::string, Channel> newChannel;

    if (key.empty())
        newChannel = std::pair<const std::string, Channel>(chanName, Channel(chanName, creator));
    else
        newChannel = std::pair<const std::string, Channel>(chanName, Channel(chanName, key, creator));
    channels.insert(newChannel);
    newChannel.second.sendWelcomeMessage(creator);
}

void    handleJoinArgs(const std::vector<std::string>& channelNames,
                        const std::vector<std::string>& channelKeys,
                        std::map<const std::string, Channel>& channels,
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
            client.enqueueMsg(Replies::JoinErr(client.getNickname(), chanName, "", ERR_BADCHANMASK));
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

void    CommandHandler::executeJoin(const std::vector<std::string>* args,
                                    Connection& client,
                                    std::map<const std::string, Channel>& channels)
{
    std::vector<std::string> channelNames;
    std::vector<std::string> channelKeys;

    if (args == NULL)
    {
        client.enqueueMsg(Replies::CommonErr(client.getNickname(), "JOIN", ERR_NEEDMOREPARAMS));
        return;
    }
    split(args->at(0), ',', channelNames);
    if (args->size() > 1)
        split(args->at(1), ',', channelKeys);
    handleJoinArgs(channelNames, channelKeys, channels, client);
}

void CommandHandler::executePing(const std::vector<std::string>* args, Connection& client)
{
    if (args == NULL)
        client.enqueueMsg("PONG\r\n");
    else
        client.enqueueMsg("PONG :" + args->at(0) + "\r\n");
}

std::string    catArguments(std::vector<std::string>::const_iterator begin,
                            std::vector<std::string>::const_iterator end)
{
    std::string ret("");

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
                        const std::vector<std::string>* args)
{
    std::string incipit(":" + sender.getMask() + " PRIVMSG " + recipient.getNickname() + " :");
    std::string message(catArguments(args->begin() + 1, args->end()));

    message.append("\r\n");
    recipient.enqueueMsg(incipit + message);
}

void sendChannelMessage(Connection& sender,
                        Channel& channel,
                        const std::vector<std::string>* args)
{
    std::string incipit(":" + sender.getMask() + " PRIVMSG " + channel.getName() + " :");
    std::string message(catArguments(args->begin() + 1, args->end()));

    message.append("\r\n");
    channel.sendChanMessage(incipit + message, sender);
}

void CommandHandler::executePrivMsg(const std::vector<std::string>* args, 
                                    Connection& client,
                                    std::map<const std::string, Channel>& channels,
                                    std::map<const std::string, Connection&>& nickToConnection)
{
    std::map<const std::string, Connection&>::iterator  targetClient;
    std::map<std::string, Channel>::iterator            targetChannel;
    std::string  target;

    if (args == NULL)
    {
        client.enqueueMsg(Replies::PrivMsgErr(client.getNickname(), "", ERR_NORECIPIENT));
        return;
    }
    if (args->size() == 1)
    {
        client.enqueueMsg(Replies::PrivMsgErr(client.getNickname(), target, ERR_NOTEXTTOSEND));
        return;
    }
    target = args->at(0);
    if (target.at(0) == '#' || target.at(0) == '&')
    {   
        targetChannel = channels.find(target);
        if (targetChannel == channels.end())
            client.enqueueMsg(Replies::PrivMsgErr(client.getNickname(), target, ERR_NOSUCHNICK));
        else if (!targetChannel->second.isUserInChannel(client.getConnectionId()) && 
                    (targetChannel->second.getMode() & FLG_EXTERNAL_MSG) == 0)
            client.enqueueMsg(Replies::PrivMsgErr(client.getNickname(), target, ERR_CANNOTSENDTOCHAN));
        else
            sendChannelMessage(client, targetChannel->second, args);
    }
    else
    {
        targetClient = nickToConnection.find(target);
        if (targetClient == nickToConnection.end() || !targetClient->second.isRegistered())
            client.enqueueMsg(Replies::PrivMsgErr(client.getNickname(), target, ERR_NOSUCHNICK));
        else
            sendPrivateMessage(client, targetClient->second, args);
    }
}

void    removeUsers(std::vector<std::string>& usersToKick,
                    const std::string& reason,
                    Channel& channel, 
                    Connection& kicker,
                    std::map<const std::string, Connection&> nickToConnection)
{
    std::map<const std::string, Connection&>::iterator toKick;
    std::string theReason;
    std::string message = ":" + kicker.getMask() + " KICK " + channel.getName() + " ";

    for (std::vector<std::string>::iterator it = usersToKick.begin(); it != usersToKick.end(); ++it)
    {
        toKick = nickToConnection.find(*it);
        if (toKick == nickToConnection.end())
        {
            kicker.enqueueMsg(Replies::KickErr(kicker.getNickname(), *it, channel.getName(), ERR_NOSUCHNICK));
            return;
        }
        else if (!channel.isUserInChannel(toKick->second.getConnectionId()))
        {
            kicker.enqueueMsg(Replies::KickErr(kicker.getNickname(), *it, channel.getName(), ERR_USERNOTINCHANNEL));
            return;
        }
        else
        {
            theReason = reason.empty() ? *it : reason;
            channel.broadCastMessage(message + *it + " :" + (theReason) + "\r\n"); //check if it works the same
            /* channel.broadCastMessage(message + *it + " :" + (theReason) + "\r\n", kicker);
            kicker.enqueueMsg(message + *it + " :" + theReason + "\r\n"); */
            channel.removeMember(toKick->second.getConnectionId());
        }
    }
}

void CommandHandler::executeKick(const std::vector<std::string>* args,
                                Connection& kicker,
                                std::map<const std::string, Channel>& channels,
                                std::map<const std::string, Connection&>& nickToConnection)
{
    std::map<std::string, Channel>::iterator    it;
    std::vector<std::string>                    usersToKick;

    usersToKick.reserve(5);
    if (args == NULL || args->size() < 2)
        kicker.enqueueMsg(Replies::CommonErr(kicker.getNickname(), "KICK", ERR_NEEDMOREPARAMS));
    else if ((it = channels.find(args->at(0))) == channels.end())
        kicker.enqueueMsg(Replies::KickErr(kicker.getNickname(), "", args->at(0), ERR_NOSUCHCHANNEL));
    else if (!it->second.isUserInChannel(kicker.getConnectionId()))
        kicker.enqueueMsg(Replies::KickErr(kicker.getNickname(), "", it->first, ERR_NOTONCHANNEL));
    else if (!it->second.isUserOperator(kicker.getConnectionId()))
        kicker.enqueueMsg(Replies::KickErr(kicker.getNickname(), "", it->first, ERR_CHANOPRIVSNEEDED));
    else
    {
        split(args->at(1), ',', usersToKick);
        if (args->size() >= 3)
        {
            std::string reason = catArguments(args->begin() + 2, args->end());
            removeUsers(usersToKick, reason, it->second, kicker, nickToConnection);
        }
        else
            removeUsers(usersToKick, "", it->second, kicker, nickToConnection);
        if (it->second.isEmpty())
            channels.erase(it->first); //remove channel if the last users kicked herself out
    }
}

void    handleInvitation(Connection& inviter,
                        Channel& channel, Connection& userInvited)
{
    const std::string message(":" + inviter.getMask() + " INVITE " + userInvited.getNickname() + " " + channel.getName() + "\r\n");

    channel.storeUserInvitation(&userInvited);
    inviter.enqueueMsg(message);
    userInvited.enqueueMsg(message);

}

void CommandHandler::executeInvite(const std::vector<std::string>* args,
                            Connection& inviter,
                            std::map<const std::string, Channel>& channels,
                            std::map<const std::string, Connection&>& nickToConnection)
{
    std::map<const std::string, Channel>::iterator          targetChan;
    std::map<const std::string, Connection&>::iterator      targetClient;

    if (args == NULL || args->size() < 2)
    {
        inviter.enqueueMsg(Replies::CommonErr(inviter.getNickname(), "INVITE", ERR_NEEDMOREPARAMS));
        return;
    }

    const std::string& invitedClient = args->at(0);  
    targetClient = nickToConnection.find(invitedClient);
    if (targetClient == nickToConnection.end())
    {
        inviter.enqueueMsg(Replies::InviteErr(inviter.getNickname(), invitedClient, "", ERR_NOSUCHNICK));
        return;
    }
    targetChan = channels.find(args->at(1));
    if (targetChan == channels.end())
    {
        inviter.enqueueMsg(Replies::InviteErr(inviter.getNickname(), "", args->at(1), ERR_NOSUCHCHANNEL));
        return;
    }
    if (!targetChan->second.isUserInChannel(inviter.getConnectionId()))
    {
        inviter.enqueueMsg(Replies::InviteErr(inviter.getNickname(), "", args->at(1), ERR_NOTONCHANNEL));
        return;
    }
    if (targetChan->second.isUserInChannel(targetClient->second.getConnectionId()))
    {
        inviter.enqueueMsg(Replies::InviteErr(inviter.getNickname(), targetClient->first, targetChan->first, ERR_USERONCHANNEL));
        return;
    }
    if ((targetChan->second.getMode() & FLG_INVITE_ONLY) && 
        !targetChan->second.isUserOperator(inviter.getConnectionId()))
    {
        inviter.enqueueMsg(Replies::InviteErr(inviter.getNickname(), targetClient->first, targetChan->first, ERR_CHANOPRIVSNEEDED));
        return;
    }
    handleInvitation(inviter, targetChan->second, targetClient->second);
}


void CommandHandler::executeTopic(const std::vector<std::string>* args,
                                    Connection& asker,
                                    std::map<const std::string, Channel>& channels)
{
    std::map<const std::string, Channel>::iterator    targetChan;

    if (args == NULL)
    {
        asker.enqueueMsg(Replies::CommonErr(asker.getNickname(), "TOPIC", ERR_NEEDMOREPARAMS));
        return;
    }
    const std::string& chanName = args->at(0);
    targetChan = channels.find(chanName);
    if (targetChan == channels.end())
    {
        asker.enqueueMsg(Replies::KickErr(asker.getNickname(), "", chanName, ERR_NOSUCHCHANNEL));
        return;
    }
    if (!targetChan->second.isUserInChannel(asker.getConnectionId()))
    {
        asker.enqueueMsg(Replies::InviteErr(asker.getNickname(), "", targetChan->first, ERR_NOTONCHANNEL));
        return;
    }
    if (args->size() >= 2)
    {
        if (targetChan->second.getMode() & FLG_TOPIC_RESTRICT 
            && !targetChan->second.isUserOperator(asker.getConnectionId()))
        {
            asker.enqueueMsg(":localhost 482 " + asker.getNickname() + 
                            " " + targetChan->first + " :You are not a channel operator\r\n");
            return;
        }
        std::string newTopic = catArguments(args->begin() + 1, args->end());
        targetChan->second.setTopic(newTopic, asker);
    }
    else
        targetChan->second.sendTopic(asker);
}

bool handleOpChange(bool set, Connection& client, Channel& chan, const std::string& toChange,
                    const std::map<const std::string, Connection&> nickToConn)
{
    std::map<const std::string, Connection&>::const_iterator it;
    bool    isTargetOp;

    it = nickToConn.find(toChange);
    if (it == nickToConn.end())
    {
        client.enqueueMsg(Replies::KickErr(client.getNickname(), toChange, "", ERR_NOSUCHNICK));
        return (false);
    }
    if (!chan.isUserInChannel(it->second.getConnectionId()))
    {
        client.enqueueMsg(Replies::InviteErr(client.getNickname(), toChange,
                 chan.getName(), ERR_USERNOTINCHANNEL));
        return(false);
    }
    isTargetOp = chan.isUserOperator(it->second.getConnectionId());
    if (set && !isTargetOp)
    {
        chan.addOperator(it->second);
        chan.broadCastMessage(":" + client.getMask() + " MODE " + chan.getName() + " :+o " + toChange + "\r\n");
    }
    else if (isTargetOp)
    {
        chan.removeOperator(it->second);
        chan.broadCastMessage(":" + client.getMask() + " MODE " + chan.getName() + " :-o " + toChange + "\r\n");
    }
    return (true);
}

void handleModeArgs(const std::vector<std::string>* args, Connection& client, Channel& chan,
            const std::map<const std::string, Connection&> nickToConn)
{
    char    curr;
    bool    set;
    const std::string& modeString = args->at(1);
    std::vector<std::string>::const_iterator params = args->begin() + 2;
    std::vector<std::string>::const_iterator argsEnd = args->end();
    std::string::const_iterator it;
    std::string::const_iterator end;
    std::string param;

    it = modeString.begin();
    end = modeString.end();

    for (; it != end; ++it)
    {
        curr = *it;
        if (!std::strchr("+-klnito", curr))
        {
            std::string msg;
            msg.push_back(curr);
            msg += " :is an unknown mode char to me\r\n";
            client.enqueueMsg(msg);
            return;
        }
        if (curr == '+')
        {
            set = true;
            continue;
        }
        if (curr == '-')
        {
            set = false;
            continue;
        }
        if (((curr == 'k' || curr == 'l') && set) || curr == 'o')
        {
            if (params == argsEnd)
            {
                client.enqueueMsg(Replies::CommonErr(client.getNickname(), "MODE", ERR_NEEDMOREPARAMS));
                return;
            }
            param = *params;
            ++params;
        }
        if (curr == 'o' && !handleOpChange(set, client, chan, param, nickToConn))
            return;
        else
            set ? chan.setChanMode(curr, client, param) : chan.unsetChanMode(curr, client);
    }
}

void CommandHandler::executeMode(const std::vector<std::string>* args,
                            Connection& modder,
                            std::map<const std::string, Channel>& channels,
                            const std::map<const std::string, Connection&>& nickToConn)
{
    std::map<const std::string, Channel>::iterator    targetChannel;

    if (args == NULL)
    {
        modder.enqueueMsg(Replies::CommonErr(modder.getNickname(), "MODE", ERR_NEEDMOREPARAMS));
        return;
    }
    const std::string& target = args->at(0);
    if (target.at(0) != '#' && target.at(0) != '&') //handle only mode command for channels
        return;
    targetChannel = channels.find(target);
    if (targetChannel == channels.end())
    {
        modder.enqueueMsg(Replies::KickErr(modder.getNickname(), "", target, ERR_NOSUCHCHANNEL));
        return;
    }
    if (args->size() == 1)
    {
        targetChannel->second.sendModeMessage(modder);
        return;
    }
    if (!targetChannel->second.isUserOperator(modder.getConnectionId())) //if here means the user is trying to change the keys
    {
        modder.enqueueMsg(Replies::KickErr(modder.getNickname(), "", targetChannel->first, ERR_CHANOPRIVSNEEDED));
        return;
    }
    else
        handleModeArgs(args, modder, targetChannel->second, nickToConn);
}

void CommandHandler::executeList(Connection& client, const std::map<const std::string, Channel>& channels)
{
    std::string     incipit(":localhost 322 " + client.getNickname() + " ");
    std::string     msg;
    std::stringstream    channelNum("");

    for (std::map<const std::string, Channel>::const_iterator it = channels.begin();
        it != channels.end(); ++it)
    {
        channelNum << (it->second.getMembers().size());
        channelNum >> msg;
        client.enqueueMsg(incipit + it->first + " " + msg + " :" + it->second.getTopic() + "\r\n");
        msg.clear();
        channelNum.clear();
    }
    client.enqueueMsg(":localhost 323 " + client.getNickname() + " :End of /LIST\r\n");
}

void CommandHandler::executeWho(const std::vector<std::string>* args, Connection& querier,
                             const std::map<const std::string, Channel>& channels)
{
    std::map<const std::string, Channel>::const_iterator it;

    if (args == NULL)
        return;
    const std::string& target = args->at(0);
    if (target.at(0) != '#' && target.at(0) != '&')
        return;
    it = channels.find(target);
    if (it == channels.end())
    {
        querier.enqueueMsg(":localhost 403 " + querier.getNickname() + " " + target + " :No such nick/channel\r\n");
        return;
    }
    std::string incipit(":localhost 352 " + querier.getNickname() + " " + target + " ");
    querier.enqueueMsg(incipit + it->second.getNamesList() +"\r\n");
    querier.enqueueMsg(":localhost 315 " + querier.getNickname() + " " + target + " :End of /WHO list\r\n");
}