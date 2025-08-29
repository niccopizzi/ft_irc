#ifndef BENDER_SIG_HPP
#define BENDER_SIG_HPP

#include <signal.h>
#include <iostream>

extern volatile sig_atomic_t    benderShouldRun;

void registerSignalHandlers();

#endif