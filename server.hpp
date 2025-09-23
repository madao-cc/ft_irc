#pragma once

#include <iostream> // std::cerr, std::endl, std::string

#include <cstring> // std::strlen

#include <cstdlib> // std::atoi, EXIT_FAILURE, EXIT_SUCCESS

#include <unistd.h> // close, read, write

#include <arpa/inet.h>
// sockaddr_in, inet_ntoa, htons, htonl, ntohs, ntohl

#include <sys/socket.h> // socket, bind, listen, accept

#include <fcntl.h> // fcntl, F_GETFL, F_SETFL, O_NONBLOCK

#include <poll.h> // poll, struct pollfd, POLLIN

#include <vector>

#define MAX_CLIENTS 10

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN "\033[1;36m"
#define RESET "\033[0m"

class Server
{
	private:
		int 			port;
		std::string 		password;
		int 			server_fd;
		struct sockaddr_in	server_addr;
		//struct pollfd		*pollfd_ptr;
		std::vector<pollfd>	poll_fds;

	public:
		Server(const char *port, const char *password);
		~Server();
		void startServer();
		void startLoop();
		void newClient();
		void handleMessage(int index);
};