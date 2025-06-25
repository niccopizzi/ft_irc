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
                const std::map<const std::string&, Connection&>& nickToConnection)
{
    return (nickToConnection.find(nickName) != nickToConnection.end());
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
    recipient.enqueueMsg(incipit + message);
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
    newChannel.second.sendWelcomeMessage(creator);
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