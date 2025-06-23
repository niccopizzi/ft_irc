#ifndef IRC_HPP
#define IRC_HPP

#include <iostream>
#include "Server/Server.hpp"

bool    areArgsValid(int argc, char* argv[]);
void    startServer(char* port, char* password);
#endif