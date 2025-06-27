#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "../channel/Channel.hpp"
#include "../replies/Replies.hpp"

class CommandHandler
{
public:

    static void notifyUsersInClientChannels(const std::string& message,
                                            const std::map<const std::string, Channel>& channels,
                                            const Connection& client);

    static void executeNick(const std::vector<std::string>* args, 
                            Connection& client,
                            std::map<const std::string, Connection&>& nickToConnection,
                            std::map<const std::string, Channel>& channels);

    static void executeUsername(const std::vector<std::string>* args,
                                Connection& client);
    
    static void executePass(const std::vector<std::string>* args,
                            Connection& client,
                            const std::string& toMatch);
                            
    static void executeJoin(const std::vector<std::string>* args,
                            Connection& client,
                            std::map<const std::string, Channel>& channels);

    static void executePing(const std::vector<std::string>* args,
                            Connection& client);

    static void executePrivMsg(const std::vector<std::string>* args, 
                            Connection& client,
                            std::map<const std::string, Channel>& channels,
                            std::map<const std::string, Connection&>& nickToConnection);

    static void executeKick(const std::vector<std::string>* args,
                            Connection& kicker,
                            std::map<const std::string, Channel>& channels,
                            std::map<const std::string, Connection&>& nickToConnection);
    
    static void executeInvite(const std::vector<std::string>* args,
                            Connection& inviter,
                            std::map<const std::string, Channel>& channels,
                            std::map<const std::string, Connection&>& nickToConnection);

    static void executeTopic(const std::vector<std::string>* args,
                            Connection& asker,
                            std::map<const std::string, Channel>& channels);

    static void executeMode(const std::vector<std::string>* args,
                            Connection& modder,
                            std::map<const std::string, Channel>& channels,
                            const std::map<const std::string, Connection&>& nickToConn);

    static void executeList(Connection& client, const std::map<const std::string, Channel>& channels);

    static void executeWho(const std::vector<std::string>* args, Connection& querier,
                             const std::map<const std::string, Channel>& channels);
};


std::string    catArguments(std::vector<std::string>::const_iterator begin,
                            std::vector<std::string>::const_iterator end);

void split(std::string toSplit, char delimiter, std::vector<std::string>& storage);

#endif