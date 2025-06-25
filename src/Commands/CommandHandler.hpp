#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include "../Channels/Channel.hpp"
#include "../Replies/Replies.hpp"

class CommandHandler
{
public:

    static void notifyUsersInClientChannels(const std::string& message,
                                            const std::map<std::string, Channel>& channels,
                                            const Connection& client);

    static void executeKick(const std::vector<std::string>& args,
                            Connection& kicker,
                            std::map<std::string, Channel>& channels,
                            std::map<const std::string&, Connection&> nickToConnection);
    static void executeNick(const std::string& nickname, 
                            Connection& client,
                            std::map<const std::string&, Connection&>& nickToConnection,
                            std::map<std::string, Channel>& channels);
    static void executePass(const std::string& toMatch,
                            const std::string& toCheck, 
                            Connection& client);
    static void executePing(const std::string& token,
                            Connection& client);
    static void executePrivMsg(const std::vector<std::string>& args, 
                            Connection& client,
                            std::map<const std::string&, Connection&>& nickToConnection,
                            std::map<std::string, Channel>& channels);
    static void executeUsername(const std::vector<std::string>& args,
                                Connection& client);
    static void executeJoin(const std::vector<std::string>& args,
                            Connection& client,
                            std::map<std::string, Channel>& channels);
    static void executeInvite(const std::vector<std::string>& args,
                            Connection& inviter,
                            std::map<std::string, Channel>& channels,
                            std::map<const std::string&, Connection&>& nickToConnection);
};


std::string    catArguments(std::vector<std::string>::const_iterator begin,
                            std::vector<std::string>::const_iterator end);
#endif