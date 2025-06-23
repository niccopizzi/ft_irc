#include "CommandHandler.hpp"

/* Command* CommandHandler::getCommand(const std::string& command, const std::string& arg)
{
    if (command == "PASS")
        return (new Pass(arg));
    if (command == "PING")
        return (new Ping());
    if (command == "NICK")
        return (new Nick());
    if (command == "USER")
        return (new Username());
    if (command == "PRIVMSG")
        return (new Privmsg());
    if (command == "QUIT")
        return (new Quit());
    return (NULL);
}
 */

bool    isNotValidFirstChar(char c)
{
    return ((c >= '0' && c <= '9' ) || c == '#' || c == ':' || c == '&');
}

bool    isNotValidChar(char c)
{
    return(!std::isalnum(c) 
            && c != '[' && c != ']' 
            && c != '{' && c != '}'
            && c != '\\' && c != '|');
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
                const std::map<std::string, Connection&>& nickToConnection)
{
    return (nickToConnection.find(nickName) != nickToConnection.end());
}

void CommandHandler::executeNick(const std::string& nickname, 
                                Connection& client,
                                std::map<std::string, Connection&>& nickToConnection)
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
            nickToConnection.erase(client.getNickname());
            client.sendMessage("You are now known as : " + nickname + "\r\n");
            client.setNickname(nickname);
        }
        else 
        {
            client.setNickname(nickname);
            if (client.isRegistered())
                client.sendMessage(Replies::WelcomeMsg(nickname, client.getUsername()));
        }
        nickToConnection.insert(std::pair<std::string, Connection&>(nickname, client));
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
    std::string ret;

    for (; begin != end; ++begin)
    {
        ret += *begin;
        if (begin + 1 != end)
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
    std::string incipit = sender.getNickname() + "!" + sender.getUsername() + "@localhost PRIVMSG " + recipient.getNickname() + " :";
    std::string message = catArguments(args.begin() + 1, args.end());

    message.append("\r\n");
    recipient.sendMessage(incipit + message);
}

void CommandHandler::executePrivMsg(const std::vector<std::string>& args, 
                                Connection& client,
                                std::map<std::string, Connection&>& nickToConnection)
{
    std::map<std::string, Connection&>::iterator it;

    
    if (!client.isRegistered())
    {
        client.sendMessage(Replies::CommonErrReplies(client.getNickname(), "", ERR_NOTREGISTERED));
        return;
    }
    else if (args.empty())
    {
        client.sendMessage(Replies::PrivMsgErrReplies(client.getNickname(), "", ERR_NORECIPIENT));
        return;
    }
    else if (args.size() == 1)
    {
        client.sendMessage(Replies::PrivMsgErrReplies(client.getNickname(), args.at(0), ERR_NOTEXTTOSEND));
        return;
    }
    else if ((it = nickToConnection.find(args.at(0))) == nickToConnection.end())
    {
        client.sendMessage(Replies::PrivMsgErrReplies(client.getNickname(), args.at(0), ERR_NOSUCHNICK));
        return;
    }
    else
        sendPrivateMessage(client, it->second, args);
}

void CommandHandler::executeQuit(const std::vector<std::string>& args, 
                    Connection& client,
                    std::map<std::string, Connection&>& nickToConnection)
{
    (void)args;
    nickToConnection.erase(client.getNickname());
    client.sendMessage("ERROR :Client quit\r\n");
    client.closeConnection();
}

void CommandHandler::executeUsername(const std::vector<std::string>& args,
                                Connection& client)
{
    if (!client.isAuthenticated())
        client.sendMessage("You need to authenticate first!\n");
    else if (args.size() < 4)
    {
        client.sendMessage(Replies::UserErrReplies(ERR_NEEDMOREPARAMS)); 
    }
    else if (client.isRegistered())
    {
        client.sendMessage(Replies::UserErrReplies(ERR_ALREADYREGISTERED));
    }
    else
    {
        client.setUsername(args.at(0));
        client.setFullname(catArguments(args.begin() + 3, args.end()));
        if (client.isRegistered())
            client.sendMessage(Replies::WelcomeMsg(client.getNickname(), client.getUsername()));
    }
}