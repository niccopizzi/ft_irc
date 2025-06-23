#include "Replies.hpp"

const std::string Replies::CommonErrReplies(const std::string& nickname, 
                                            const std::string& command,
                                            int err)
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
        reply += " 461 " + command + " :Not enough parameters\r\n";
    }
    return (reply);
}

const std::string Replies::NickErrReplies(const std::string& nickname, 
                                    const std::string& username,
                                    int err)
{
    std::string reply = ":localhost ";

    if (err == ERR_NONICKNAMEGIVEN)
    {
        reply += "431 :No nickname given\r\n";
    }
    else if (err == ERR_ERRONEUSNICKNAME)
    {
        reply += "432 ";
        if (!username.empty())
            reply += username + " ";    
        reply += nickname + " :Erroneous Nickname\r\n";
    }
    else if (err == ERR_NICKNAMEINUSE)
    {
        reply += "433 " + nickname + " :Nickname is already in use\r\n";
    }
    return (reply);
}

const std::string Replies::UserErrReplies(int err)
{
    std::string reply = ":localhost ";

    if (err == ERR_NEEDMOREPARAMS)
    {
        return (Replies::CommonErrReplies("", "USER", ERR_NEEDMOREPARAMS));
    }
    else if (err == ERR_ALREADYREGISTERED)
    {
        reply += "462 :You may not reregister\r\n";
    }
    return (reply);
}

const std::string Replies::PassErrReplies(int err)
{
    std::string reply = ":localhost ";

    if (err == ERR_NEEDMOREPARAMS)
        return Replies::CommonErrReplies("", "PASS", ERR_NEEDMOREPARAMS);
    else if (err == ERR_ALREADYREGISTERED)
        reply += "462 :You may not reregister\r\n";
    else if (err == ERR_PASSWDMISMATCH)
        reply += "464 :Password incorrect\r\n";
    
    return (reply);
}

const std::string Replies::WelcomeMsg(const std::string& nickname,
                                        const std::string& username)
{
    std::string reply = ":localhost 001 " + nickname + " Welcome to the Internet Relay Chat Network,";

    reply += " " + nickname + "!" + username + "@" + "localhost\r\n";

    return (reply);
}

const std::string Replies::PrivMsgErrReplies(const std::string& sender, 
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

    return (reply);
}