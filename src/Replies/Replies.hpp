#ifndef REPLIES_HPP
#define REPLIES_HPP

#include <string>

#define ERR_NOSUCHNICK          401
#define ERR_NORECIPIENT         411
#define ERR_NOTEXTTOSEND        412
#define ERR_NONICKNAMEGIVEN     431
#define ERR_ERRONEUSNICKNAME    432
#define ERR_NICKNAMEINUSE       433
#define ERR_NOTREGISTERED       451
#define ERR_NEEDMOREPARAMS      461
#define ERR_ALREADYREGISTERED   462
#define ERR_PASSWDMISMATCH      464

class Replies
{
public:
    static const std::string NickErrReplies(const std::string& nickname, 
                                    const std::string& username,
                                    int err);
    
    static const std::string UserErrReplies(int err);
    static const std::string PrivMsgErrReplies(const std::string& sender, 
                                                const std::string& recipient,
                                                int err);
    static const std::string PassErrReplies(int err);
    static const std::string WelcomeMsg(const std::string& nickname,
                                        const std::string& username);

    static const std::string CommonErrReplies(const std::string& nickname, 
                                                const std::string& command,
                                                int err);
};

#endif