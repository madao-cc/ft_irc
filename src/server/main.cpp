#include "../../include/Server.hpp"

//? Global server pointer for signal handler access
Server* server = NULL;

void handleArguments(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << RED << "[ERROR]" << RESET << " Usage: ./ircserv <port> <password>" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Check if port argument is not empty
	if (!argv[1] || std::strlen(argv[1]) == 0)
	{
		std::cerr << RED << "[ERROR]" << RESET << " Port cannot be empty" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Check if port contains only digits
	for (size_t i = 0; i < std::strlen(argv[1]); i++)
	{
		if (!std::isdigit(static_cast<unsigned char>(argv[1][i])))
		{
			std::cerr << RED << "[ERROR]" << RESET << " Port must contain only digits" << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	// Convert port to integer and check for overflow
	long port_long = std::atol(argv[1]);
	if (port_long < 0 || port_long > 65535)
	{
		std::cerr << RED << "[ERROR]" << RESET << " Port number must be between 0 and 65535" << std::endl;
		exit(EXIT_FAILURE);
	}

	int port = static_cast<int>(port_long);
	
	// Check for valid port range (privileged ports below 1024 require root)
	if (port < 1024)
	{
		std::cerr << RED << "[ERROR]" << RESET << " Port number must be 1024 or higher (ports below 1024 require root privileges)" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Check if password argument exists and is not empty
	if (!argv[2] || std::strlen(argv[2]) == 0)
	{
		std::cerr << RED << "[ERROR]" << RESET << " Password cannot be empty" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Check password length constraints
	if (std::strlen(argv[2]) > 256)
	{
		std::cerr << RED << "[ERROR]" << RESET << " Password is too long (maximum 256 characters)" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Check for invalid characters in password (control characters)
	for (size_t i = 0; i < std::strlen(argv[2]); i++)
	{
		if (std::iscntrl(static_cast<unsigned char>(argv[2][i])))
		{
			std::cerr << RED << "[ERROR]" << RESET << " Password contains invalid control characters" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

void handleSigs(int signum)
{

	if (signum == SIGINT)
	{
		if (DEBUG || EVAL)
			std::cout << LIGHT_GRAY << "[SERVER]" << YELLOW << "[INFO]" << RESET << " Server shutting down..." << std::endl;
		delete server;
		exit(128 + signum);  // Unix convention: 128 + signal number (SIGINT = 2, so exits with 130)
	}
	else if (signum == SIGTSTP)
	{
		if (DEBUG || EVAL)
			std::cout << LIGHT_GRAY << "[SERVER]" << YELLOW << "[INFO]" << RESET << " Server stopped!!" << std::endl;
		delete server;
		exit(128 + signum);  // Unix convention: 128 + signal number (SIGTSTP = 20, so exits with 148)
		
	}
}

void define_sigs()
{
	struct sigaction signal;

	memset(&signal, 0 , sizeof(signal));

	signal.sa_handler = handleSigs;
	sigaction(SIGINT, &signal, NULL);

}

int main(int argc, char **argv)
{
	handleArguments(argc, argv);
	define_sigs();
	server = new Server(std::atoi(argv[1]), argv[2]);
	server->serverLoop();
	delete server;
	return 0;
}
