#include "server.hpp"

void Server::newClient()
{
	int client_fd = accept(serverFd, ())
}

void Server::startLoop()
{
	for (;;)
	{
		if (poll(&poll_fds[0], poll_fds.size(), -1) == -1) // -1 makes it wait indefinitely until an event occurs
		{
			std::cerr << RED << "ERROR: " << RESET << "Poll error." << std::endl;
			close(server_fd);
			exit(EXIT_FAILURE);
		}
		for (size_t i = 0; i < poll_fds.size(); i++)
		{
			if (poll_fds[i].revents & POLLIN)
			{
				if (poll_fds[i].fd == server_fd)
				{
					newClient();
				}
				else
				{
					handleMessage(i);
				}
			}
		}
	}
}

void Server::startServer()
{
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		std::cerr << RED << "ERROR: " << RESET << "Failed to create socket." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int flags = fcntl(server_fd, F_GETFL, 0);
	if (flags == -1)
	{
		std::cerr << RED << "ERROR: " << RESET << "Failed to get socket flags." << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		std::cerr << RED << "ERROR: " << RESET << "Failed to set socket to non-blocking." << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	
	if (bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		std::cerr << RED << "ERROR: " << RESET << "Failed to bind socket." << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, MAX_CLIENTS) == -1)
	{
		std::cerr << RED << "ERROR: " << RESET << "Failed to listen on socket." << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	pollfd server_pollfd;
	server_pollfd.fd = server_fd;
	server_pollfd.events = POLLIN;
	server_pollfd.revents = 0;
	poll_fds.push_back(server_pollfd);

	/*----------------------------------------------------------------------------------------------*/

	std::cout << GREEN << "SUCCESS: " << RESET << "Server listening on port " << port << std::endl;

	int client_fd = accept(server_fd, NULL, NULL);
	if (client_fd == -1)
	{
		std::cerr << RED << "ERROR: " << RESET << "Failed to accept connection" << std::endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}

	std::cout << GREEN << "SUCCESS: " << RESET << "Client connected" << std::endl;

	for (;;)
	{
		sleep(1);
	}
}

bool isValidNumber(const char *str)
{
	if (!str || !*str)
		return false;
	for (int i = 0; str[i]; ++i)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

Server::Server(const char *port, const char *password)
{
	this->port = std::atoi(port);
	if (this->port <= 0 || this->port > 65535)
	{
		std::cerr << RED << "ERROR: " << RESET << "Invalid port number." << std::endl;
		exit(EXIT_FAILURE);
	}

	this->password = password;
	if (this->password.empty())
	{
		std::cerr << RED << "ERROR: " << RESET << "Password cannot be empty." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << GREEN << "SUCCESS: " << RESET << "Server initialized on port " << this->port << " with password '" << this->password << "'" << std::endl;
}

Server::~Server()
{
	close(server_fd);
	std::cout << YELLOW << "INFO: " << RESET << "Server shut down." << std::endl;
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << RED << "ERROR: " << RESET << "Invalid number of arguments." << std::endl << "Usage: " << argv[0] << " <PORT> <PASSWORD>" << std::endl;
		return EXIT_FAILURE;
	}

	const char *port = argv[1];
	if (!isValidNumber(port))
	{
		std::cerr << RED << "ERROR: " << RESET << "Invalid port number." << std::endl;
		return EXIT_FAILURE;
	}
	const char *password = argv[2];

	Server server(port, password);

	server.startServer();
	server.startLoop();
}