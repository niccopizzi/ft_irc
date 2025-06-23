#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include <string>
#include <vector>
#include <map>
#include "../Server/Connection.hpp"
#include "../Replies/Replies.hpp"

class CommandHandler
{
public:

    static void executeNick(const std::string& nickname, 
                    Connection& client,
                    std::map<std::string, Connection&>& nickToConnection);
    static void executePass(const std::string& toMatch,
                            const std::string& toCheck, 
                            Connection& client);
    static void executePing(const std::string& token,
                            Connection& client);
    static void executePrivMsg(const std::vector<std::string>& args, 
                            Connection& client,
                            std::map<std::string, Connection&>& nickToConnection);
    static void executeQuit(const std::vector<std::string>& args, 
                            Connection& client,
                            std::map<std::string, Connection&>& nickToConnection);
    static void executeUsername(const std::vector<std::string>& args,
                                Connection& client);
};

#endif