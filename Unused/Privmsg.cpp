#include "Privmsg.hpp"

Privmsg::Privmsg()
{
    #ifdef DEBUG
        std::cout << "Privmsg default constructor called\n";
    #endif
}
    
Privmsg::~Privmsg()
{
    #ifdef DEBUG
        std::cout << "Privmsg destructor called\n";
    #endif
}

void sendPrivateMessage(Connection& sender, 
                        Connection& recipient, 
                        const std::vector<std::string>& args)
{
    std::string incipit = sender.getNickname() + "!" + sender.getUsername() + "@localhost PRIVMSG " + recipient.getNickname() + " :";
    std::string message;

    for (std::vector<std::string>::const_iterator it = args.begin() + 1; 
            it != args.end(); ++it)
    {
        message += *it;
        if (it + 1 != args.end())
            message += " ";
    }
    if (message.at(0) == ':')
        message.erase(0, 1);
    message.append("\r\n");
    recipient.sendMessage(incipit + message);
}

void Privmsg::execute(const std::vector<std::string>& args, 
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