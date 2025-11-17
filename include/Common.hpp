#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <iterator>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cerrno>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <csignal>


// ANSI color codes
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m" 
#define BLUE  "\033[34m"
#define OLIVE "\033[38;5;3m"
#define PURPLE "\033[38;5;5m"
#define ORANGE "\033[38;5;208m"
#define CYAN "\033[38;5;6m"
#define PINK  "\033[38;5;13m"
#define LIGHT_GRAY "\033[38;5;7m"
#define DARK_GRAY "\033[38;5;8m"
#define RESET  "\033[0m"

#define WAIT_FOR_EVENT -1
#define DEBUG 0
#define CHANNEL_DEBUG 0
#define EVAL 1
