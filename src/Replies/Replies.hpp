#ifndef REPLIES_HPP
#define REPLIES_HPP

#include <string>
#include <vector>

#define HOSTNAME "Sambatime"

#define RPL_TOPIC               332
#define RPL_ENDOFNAMES          366
#define ERR_NOSUCHNICK          401
#define ERR_NOSUCHCHANNEL       403
#define ERR_CANNOTSENDTOCHAN    404
#define ERR_NORECIPIENT         411
#define ERR_NOTEXTTOSEND        412
#define ERR_NONICKNAMEGIVEN     431
#define ERR_ERRONEUSNICKNAME    432
#define ERR_NICKNAMEINUSE       433
#define ERR_NOTONCHANNEL        442
#define ERR_NOTREGISTERED       451
#define ERR_NEEDMOREPARAMS      461
#define ERR_ALREADYREGISTERED   462
#define ERR_PASSWDMISMATCH      464
#define ERR_INVITEONLYCHAN      473
#define ERR_BADCHANNELKEY       475
#define ERR_BADCHANMASK         476
#define ERR_CHANOPRIVSNEEDED    482
#define ERR_INVALIDKEY          525

class Replies
{
public:
    static const std::string NickErrReplies(const std::string& nickname, 
                                    const std::string& username,
                                    int err);
    static const std::string JoinWelcomeReplies(const std::string& topic,
                                    const std::string& nickname,
                                    const std::string& channelname, int status);
    static const std::string JoinErrReplies(const std::string& nickname,
                                    const std::string& chanName,
                                    const std::string& key,
                                    int err); 
    static const std::string UserErrReplies(int err);
    static const std::string PrivMsgErrReplies(const std::string& sender, 
                                                const std::string& recipient,
                                                int err);
    static const std::string PassErrReplies(int err);
    static const std::string WelcomeMsg(const std::string& nickname,
                                        const std::string& mask);
    static const std::string KickErrReplies(const std::string& name,
                                            const std::string& chanName,
                                            int err);
    static const std::string CommonErrReplies(const std::string& nickname, 
                                                const std::string& command,
                                                int err);

};

#endif