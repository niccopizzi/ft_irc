#include "Replies.hpp"

const std::string Replies::CommonErr(const std::string& nickname, 
                                    const std::string& command, int err)
{
    std::string reply = ":localhost ";
    if (err == ERR_NOTREGISTERED)
    {
        reply += "451 ";
        nickname.empty() ? reply += '*' : reply += nickname;
        reply += " :You have not registered\r\n";
    }
    else if (err == ERR_NEEDMOREPARAMS)
    {
        reply += " 461 ";
        if (!nickname.empty())
            reply += nickname + " ";
        reply += command + " :Not enough parameters\r\n";
    }
    return (reply);
}

const std::string Replies::NickErr(const std::string& nickname, 
                                    const std::string& oldnick, int err)
{
    std::string reply = ":localhost ";

    if (err == ERR_NONICKNAMEGIVEN)
    {
        reply += "431 :No nickname given\r\n";
    }
    else if (err == ERR_ERRONEUSNICKNAME)
    {
        reply += "432 " + (oldnick.empty() ? "*" : oldnick) + " ";
        reply += nickname + " :Erroneous Nickname\r\n";
    }
    else if (err == ERR_NICKNAMEINUSE)
    {
        reply += "433 " + (oldnick.empty() ? "*" : oldnick) + " ";
        reply += nickname + " :Nickname is already in use\r\n";
    }
    return (reply);
}

const std::string Replies::UserErr(const std::string& nickname, int err)
{
    std::string reply = ":localhost ";

    if (err == ERR_NEEDMOREPARAMS)
    {
        return (Replies::CommonErr(nickname, "USER", ERR_NEEDMOREPARAMS));
    }
    else if (err == ERR_ALREADYREGISTERED)
    {
        reply += "462 :You may not reregister\r\n";
    }
    return (reply);
}

const std::string Replies::PassErr(int err)
{
    std::string reply = ":localhost ";

    if (err == ERR_NEEDMOREPARAMS)
        return Replies::CommonErr("", "PASS", ERR_NEEDMOREPARAMS);
    else if (err == ERR_ALREADYREGISTERED)
        reply += "462 :You may not reregister\r\n";
    else if (err == ERR_PASSWDMISMATCH)
        reply += "464 :Password incorrect\r\n";
    
    return (reply);
}

const std::string Replies::WelcomeMsg(const std::string& nickname,
                                        const std::string& mask)
{
    std::string reply = ":localhost 001 " + nickname + " Welcome to the Internet Relay Chat Network! ";

    reply += ":" + mask + "\r\n";

    return (reply);
}

const std::string Replies::PrivMsgErr(const std::string& sender, 
                                    const std::string& recipient,
                                    int err)
{
    std::string reply = ":localhost ";
    if (err == ERR_NORECIPIENT)
    {
        reply += "411 " + sender + " :No recipient given (PRIVMSG)\r\n";
    }
    else if (err == ERR_NOTEXTTOSEND)
    {
        reply += "412 " + sender + " :No text to send\r\n";
    }
    else if (err == ERR_NOSUCHNICK)
    {
        reply += "401 " + sender + " " + recipient + " :No such nick/channel\r\n";
    }
    else if (err == ERR_CANNOTSENDTOCHAN)
        reply += "404" + sender + " " + recipient + " :Cannot send to chan\r\n";
    return (reply);
}

const std::string Replies::JoinWelcome(const std::string& topic,
                                    const std::string& nickname,
                                    const std::string& channelname,
                                    int status)
{
    std::string reply;

    if (status == RPL_TOPIC)
        reply = ":localhost 332 " + nickname + " " + channelname + " :" + topic + "\r\n";
    else if (status == RPL_ENDOFNAMES)
        reply = ":localhost 366 " + nickname + " " + channelname + " :End of /NAMES list\r\n";
    return (reply);
}

const std::string   Replies::JoinErr(const std::string& nickname,
                                    const std::string& chanName,
                                    const std::string& key,
                                    int err)
{
    std::string reply = ":localhost ";
    
    (void)key;
    if (err == ERR_BADCHANMASK)
    {
        reply += "476 " + nickname;
        if (!chanName.empty())
            reply += " " + chanName;
        reply += " :Bad Channel Mask\r\n";
    }
    else if (err == ERR_BADCHANNELKEY)
    {
        reply += "475 " + nickname + " " + chanName + " :Cannot join channel (+k)\r\n";
    }
    else if (err == ERR_INVALIDKEY)
    {
        reply += "525 " + nickname + " " + chanName + " :Key is not well-formed\r\n";
    }
    else if (err == ERR_INVITEONLYCHAN)
    {
        reply += "473 " + nickname + " " + chanName + " :Cannot join channel (+i)\r\n";
    }
    return (reply);
}

const std::string Replies::KickErr(const std::string& name,
                                  const std::string& toKick,
                                  const std::string& chanName,
                                  int err)
{
    std::string reply = ":localhost ";

    if (err == ERR_NOSUCHCHANNEL)
    {
        reply += "403 " + name + " " + chanName + " :No such channel\r\n";
    }
    else if (err == ERR_CHANOPRIVSNEEDED)
    {
        reply += "482 " + name + " " + chanName + " :You're not channel operator\r\n";
    }
    else if (err == ERR_NOSUCHNICK)
    {
        reply += "401 " + name + " " + toKick + " :No such nick/channel\r\n"; 
    }
    else if (err == ERR_USERNOTINCHANNEL)
    {
        reply += "441 " + name + " " + toKick + " " + chanName + " :They aren't on that channel\r\n";
    }
    return (reply);
}

const std::string Replies::InviteErr(const std::string& nickname,
                                    const std::string& invited, 
                                    const std::string& channel,   
                                    int err)
{
    std::string reply = ":localhost ";

    if (err == ERR_NOSUCHNICK)
        reply += "401 " + nickname + " " + invited + " :No such nick/channel\r\n";
    else if (err == ERR_NOSUCHCHANNEL)
        reply += "403" + nickname, + " " + channel + " :No such channel\r\n";
    else if (err == ERR_NOTONCHANNEL)
        reply += "442 " + nickname + " " + channel + " :You are not on that channel\r\n";
    else if (err == ERR_USERONCHANNEL)
        reply += "443" + nickname + " " + invited + " " + channel + " :is already on channel\r\n";
    else if (err == ERR_CHANOPRIVSNEEDED)
        reply += "482 " + nickname + " " + channel + " :You're not channel operator\r\n";
    return (reply); 
}