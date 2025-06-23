#include "Nick.hpp"

Nick::Nick()
{
    #ifdef DEBUG
        std::cout << "Nick constructor called\n";
    #endif
}

/* Nick::Nick(const Nick& nick)
{
    #ifdef DEBUG
        std::cout << "Nick copy constructor called\n";
    #endif
}

Nick& Nick::operator=(const Nick& other)
{
    #ifdef DEBUG
        std::cout << "Nick copy operator called\n";
    #endif
    return (*this);
} */

Nick::~Nick()
{
    #ifdef DEBUG
        std::cout << "Nick destructor called\n";
    #endif
}

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

void Nick::execute(const std::vector<std::string>& args, 
                    Connection& client,
                    std::map<std::string, Connection&>& nickToConnection)
{
    const std::string& nick = args.at(0);

    if (!client.isAuthenticated())
        client.sendMessage(":You need to authenticate first\n");
    else if (args.empty())
        client.sendMessage(Replies::NickErrReplies("", "", ERR_NONICKNAMEGIVEN));
    else if (!isNickValid(nick))
        client.sendMessage(Replies::NickErrReplies(nick, client.getUsername(), ERR_ERRONEUSNICKNAME));
    else if (nick == client.getNickname())
        return;    
    else if (isNickUsed(nick, nickToConnection))
        client.sendMessage(Replies::NickErrReplies(nick, client.getUsername(), ERR_NICKNAMEINUSE));
    else
    {
        if (client.isRegistered())
        {
            nickToConnection.erase(client.getNickname());
            client.sendMessage("You are now known as : " + nick + "\r\n");
            client.setNickname(nick);
        }
        else 
        {
            client.setNickname(nick);
            if (client.isRegistered())
                client.sendMessage(Replies::WelcomeMsg(nick, client.getUsername()));
        }
        nickToConnection.insert(std::pair<std::string, Connection&>(nick, client));
    }
}