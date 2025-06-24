#ifndef USER_HPP
#define USER_HPP


#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/ip.h>

class Operator;

class User
{
private:
    std::string                 nickname;
    std::string                 username;
    std::string                 fullname;
    std::string                 mask;
    Operator*                   oper;
    bool                        didAuthenticate;

public:
    User();
    User(const std::string& nickname, const std::string& username);
    User(const User& user);
    User& operator=(const User& other);
    ~User();

    const std::string&          getNickname() const;    
    const std::string&          getUsername() const;
    const std::string&          getFullname() const;
    const std::string&          getMask() const;
    bool                        isAuthenticated() const;
    bool                        isRegistered() const;
    bool                        isOp() const;

    void    becomeOp();
    void    setNickname(const std::string& newNick);
    void    setUsername(const std::string& newUsername);
    void    setFullname(const std::string& newFullname);
    void    setMask(const std::string& newMask);
    void    setAuthenticate(bool auth);
};

#endif

