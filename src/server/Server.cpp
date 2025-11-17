#include "../../include/Server.hpp"

Server::Server(int port_number, std::string passwrd) : port(port_number), password(passwrd)
{
	stop_server = false;
	server_fd = socket(AF_INET,SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << RESET << " socket() failed: " << std::strerror(errno) << std::endl;
		stop_server = true;
		exit(EXIT_FAILURE);
	}
	int accept = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &accept, sizeof(int));
	/* setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &accept, sizeof(int));
	 e usado aqui para evitar bind block, assim o port em questao sera sempre permitido 
	*/
	memset(&sock_addr, 0 , sizeof(sockaddr_in));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port_number);
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	pollfd	pfd;
	pfd.fd = server_fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	pollFds.push_back(pfd);
	server_name = std::string(":Ft_Irc");
	if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1)
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << RESET << " fcntl() failed: " << std::strerror(errno) << std::endl;
		stop_server = true;
		return ;
		//exit(EXIT_FAILURE);
	}
	if (bind(server_fd, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr)) == -1)
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << RESET << " bind() failed: " << std::strerror(errno) << std::endl;
		stop_server = true;
		return ;
		//exit(EXIT_FAILURE);
	}
	if (listen(server_fd, SOMAXCONN) == -1)
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << RESET << " listen() failed: " << std::strerror(errno) << std::endl;
		stop_server = true;
		return ;
		//exit(EXIT_FAILURE);
	}
}

// Added clear() to destructor to avoid dangling pointers
// Without it:
// Clients vector: [ptr1, ptr2, ptr3]
// Heap memory:    [FREED, FREED, FREED]
// So the ptrs would point to freed memory!
// This would not be an error, but could lead to confusion when printing information about clients.
Server::~Server()
{
	for (client_iterator it = Clients.begin(); it != Clients.end(); ++it)
	{
		delete *it;
	}
	Clients.clear();

	if (DEBUG || EVAL)
		std::cout << YELLOW << "[INFO]" << RESET << " All Clients disconnected and memory freed. Clients: " << Clients.size() << std::endl;
}

void Server::serverLoop()
{
	while(!stop_server)
	{
		if (poll(&pollFds[0], pollFds.size(), WAIT_FOR_EVENT) == -1)
		{
			if (DEBUG || EVAL)
				std::cout << RED << "[ERROR]" << RESET << " poll() failed: " << std::strerror(errno) << std::endl;
			return ;
		}
		for (size_t i = 0; i < pollFds.size(); i++)
		{
			short rev = pollFds[i].revents;
			if (rev == 0)
				continue;
			if ((rev & POLLIN) && pollFds[i].fd == server_fd)
			{
				newClient();
				continue;
			}
			if (rev & POLLIN)
			{
				manageMessage(i);

			}
			if (rev & (POLLHUP | POLLRDHUP))
			{
				if (pollFds[i].fd != server_fd)
					removeClient(static_cast<int>(i) - 1);
				continue;
			}
			if (rev & POLLERR)
			{
				if (pollFds[i].fd != server_fd)
					removeClient(static_cast<int>(i) - 1);
				continue;
			}
		}
	}
}


void Server::manageMessage(int index)
{
	char buffer[512];
	int fd = pollFds[index].fd;
	ssize_t bytes_r = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_r < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			return;
		}
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << RESET << " recv() failed for fd " << fd << ": " << std::strerror(errno) << std::endl;
		removeClient(static_cast<int>(index) - 1);
		return;
	}
	else if (bytes_r == 0)
	{
		if (DEBUG || EVAL)
			std::cout << YELLOW << "[INFO]" << RESET << " Client disconnected (recv==0)" << std::endl;
		removeClient(static_cast<int>(index) - 1);
		return;
	}
	buffer[bytes_r] = '\0';
	clientBuffers[fd] += std::string(buffer, bytes_r);

	// Enforce RFC message length limit: 512 bytes including CRLF
	const size_t RFC_MSG_LIMIT = 512;
	if (clientBuffers[fd].size() > RFC_MSG_LIMIT)
	{
		if (DEBUG || EVAL)
			std::cout << YELLOW << "[WARN]" << RESET << " Message from fd=" << fd << " exceeded RFC limit (" << clientBuffers[fd].size() << " bytes), disconnecting" << std::endl;
		// Close connection for protocol violation (message too long)
		removeClient(static_cast<int>(index) - 1);
		return;
	}
	std::string command = "";

	while (clientBuffers[fd].find('\n') != std::string::npos)
	{
		size_t pos = clientBuffers[fd].find('\n');
		command = clientBuffers[fd].substr(0, pos);
		if (!command.empty() && command[command.length() - 1] == '\r')
			command.erase(command.length() - 1, 1);
		clientBuffers[fd].erase(0, pos + 1);
		if (!command.empty())
		{
			if (DEBUG)
				std::cout << YELLOW << "[INFO]" << RESET << " Received command: " << command << std::endl;
			parseCommand(command, static_cast<int>(index) - 1);
		}
	}

}

void Server::parseCommand(std::string raw_msg, int index)
{
	unsigned int	i = 0;
	while (i < raw_msg.size() && std::isspace(raw_msg[i]))
		i++;
	raw_msg = raw_msg.substr(i);
	str_tokens.clear();
	str_tokens = split(raw_msg);
	if (str_tokens.empty())
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << RESET << " Empty command received" << std::endl;
		return;
	}
	checkCommandHandlers(index);
}

std::vector<std::string> Server::split(const std::string &str)
{
	std::string		 token;
	std::vector<std::string> token_vector;
	std::istringstream 	 iss(str);
	while (iss >> token)
	{
		if (!token.empty() && token[0] == ':')
		{
			std::string message;
			std::getline(iss, message);
			token += message;
			token_vector.push_back(token);
			break;
		}
		else
		{
			token_vector.push_back(token);
		}
	}
	return token_vector;
}

void Server::checkCommandHandlers(int index)
{
	if (commandHandlers.empty())
	{
		// Connection commands
		commandHandlers["NICK"] = &Server::handleNick;
		commandHandlers["USER"] = &Server::handleUser;
		commandHandlers["PASS"] = &Server::handlePass;
		// Channel commands
		commandHandlers["JOIN"] = &Server::parseJoin;
		commandHandlers["PART"] = &Server::parsePart;
		commandHandlers["KICK"] = &Server::parseKick;
		commandHandlers["MODE"] = &Server::handleMode;
		commandHandlers["TOPIC"] = &Server::handleTopic;
		commandHandlers["INVITE"] = &Server::handleInvite;
		// Messaging commands
		commandHandlers["PRIVMSG"] = &Server::handlePrivmsg;
		// Exit commands
		commandHandlers["QUIT"]  = &Server::handleQuit;
	}
	std::string command = str_tokens[0];
	for (size_t i = 0; i < command.size(); i++)
		command[i] = std::toupper(static_cast<unsigned char>(command[i]));
	std::map<std::string, void(Server::*)(std::vector<std::string> &, int)>::iterator it = commandHandlers.find(command);
	if (it != commandHandlers.end())
	{
		void (Server::*handler)(std::vector<std::string> &, int) = it->second;
		(this->*handler)(str_tokens, index);
	    	if (index >= 0 && index < static_cast<int>(Clients.size()))
			wellcoming_client(index);
	}
	else
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << RESET << " Unknown command: " << command << std::endl;
	}
}

void Server::newClient()
{
	socklen_t size = sizeof(sock_addr);
	int client_fd = accept(server_fd, (struct sockaddr *)&sock_addr, &size);
	clientBuffers[client_fd] = "";
	if (DEBUG)
		std::cout << GREEN << "[SUCCESS]" << RESET << " New client connected - FD: " << client_fd << std::endl;
	Client* client = new Client(server_fd, client_fd);
	if (client->get_bool_disconnected())
	{
		delete client;
		if (DEBUG || EVAL)
		{
			std::cout << RED << "[ERROR]" << RESET << " New client connection failed - FD: " << client_fd << std::endl;
			std::cout << RED << "[ERROR]" << RESET << " Removing client immediately after connection." << std::endl;
		}
		return;
	}
	Clients.push_back(client);
	pollFds.push_back(client->get_pollfd());
	std::cout << "New pollfd[] is stores as " << client_fd << std::endl;
	/* for (size_t i = 0; i < pollFds.size(); i++)
	{
		std::cout << pollFds[i].fd << " "; 
	} */
}

void Server::exit_from_pass(int index)
{
	if (index < 0 || index >= static_cast<int>(Clients.size()))
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << RESET << " Invalid client index: " << index << std::endl;
		return;
	}
	// Remove client from every channel it belongs to
	for (size_t i = 0; i < Channels.size(); ++i)
	{
		if (Channels[i].hasClient(Clients[index]))
			Channels[i].removeClient(Clients[index]);
	}
	int fd = Clients[index]->get_fd();
	struct linger ling;
	ling.l_onoff = 1;   // enable linger
	ling.l_linger = 0;  // timeout 0 -> abortive close (RST)
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) == -1)
	{
		if (DEBUG || EVAL)
			std::cout << YELLOW << "[WARN]" << RESET << " setsockopt(SO_LINGER) failed for fd " << fd << ": " << std::strerror(errno) << std::endl;
		if (shutdown(fd, SHUT_RDWR) == -1)
		{
			if (DEBUG || EVAL)
				std::cout << YELLOW << "[WARN]" << RESET << " shutdown() failed for fd " << fd << ": " << std::strerror(errno) << std::endl;
		}
	}
	close(fd);
	// Reset registration-related flags before deleting the client
	Clients[index]->set_bool_registered(false);
	Clients[index]->set_bool_wellcome(false);
	Clients[index]->set_bool_password_implemented(false);
	if (DEBUG || EVAL)
		std::cout << YELLOW << "[INFO]" << RESET << " Client disconnected (abortive) - FD: " << fd << std::endl;
}


void Server::removeClient(int index)
{
	if (index < 0 || index >= static_cast<int>(Clients.size()))
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << RESET << " Invalid client index: " << index << std::endl;
		return;
	}
	// Remove client from every channel it belongs to
	for (size_t i = 0; i < Channels.size(); ++i)
	{
		if (Channels[i].hasClient(Clients[index]))
			Channels[i].removeClient(Clients[index]);
	}
	int fd = Clients[index]->get_fd();
	struct linger ling;
	ling.l_onoff = 1;   // enable linger
	ling.l_linger = 0;  // timeout 0 -> abortive close (RST)
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) == -1)
	{
		if (DEBUG || EVAL)
			std::cout << YELLOW << "[WARN]" << RESET << " setsockopt(SO_LINGER) failed for fd " << fd << ": " << std::strerror(errno) << std::endl;
		if (shutdown(fd, SHUT_RDWR) == -1)
		{
			if (DEBUG || EVAL)
				std::cout << YELLOW << "[WARN]" << RESET << " shutdown() failed for fd " << fd << ": " << std::strerror(errno) << std::endl;
		}
	}
	close(fd);
	if (static_cast<size_t>(index + 1) < pollFds.size())
		pollFds.erase(pollFds.begin() + (index + 1));
	else
	{
		if (DEBUG || EVAL)
			std::cout << YELLOW << "[WARN]" << RESET << " Expected pollFds entry for client not found (index: " << index << ")" << std::endl;
	}
	// Reset registration-related flags before deleting the client
	Clients[index]->set_bool_registered(false);
	Clients[index]->set_bool_wellcome(false);
	Clients[index]->set_bool_password_implemented(false);
	clientBuffers.erase(fd);
	delete Clients[index];
	Clients.erase(Clients.begin() + index);
	if (DEBUG || EVAL)
		std::cout << YELLOW << "[INFO]" << RESET << " Client disconnected (abortive) - FD: " << fd << std::endl;
}
bool Server::checkChannelExists(std::string channel_name)
{
	for (size_t i = 0; i < channel_name.size(); i++)
		channel_name[i] = std::tolower(static_cast<unsigned char>(channel_name[i]));
	for (size_t i = 0; i < Channels.size(); i++)
	{
		if (Channels[i].get_name() == channel_name)
			return true;
	}
	return false;
}

void Server::wellcoming_client(int index)
{
	if (Clients[index]->get_bool_registered() && Clients[index]->get_bool_wellcome())
	{
		std::string msg1 = server_name + std::string(" Welcome to the Internet Relay Network ") + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost\n" ;
		send(Clients[index]->get_fd(), msg1.c_str(), msg1.size(), 0);
		Clients[index]->set_bool_wellcome(false);
	}
}

void Server::broadcastToChannel(size_t channel_index, const std::string& message, Client* exclude)
{
	if (channel_index >= Channels.size())
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << RESET << " Invalid channel index in broadcastToChannel: " << channel_index << std::endl;
		return;
	}
	const std::map<Client*, bool>& users = Channels[channel_index].get_channel_users();
	for (std::map<Client*, bool>::const_iterator it = users.begin(); it != users.end(); ++it)
	{
		Client* client = it->first;
		if (client != exclude)
		{
			send(client->get_fd(), message.c_str(), message.size(), 0);
		}
	}
}

void Server::sendTopicAndNames(size_t channel_index, int client_index)
{
	if (channel_index >= Channels.size())
		return;
	if (client_index < 0 || client_index >= static_cast<int>(Clients.size()))
		return;

	Channel &channel = Channels[channel_index];
	Client *new_guy = Clients[client_index];
	std::string nick = new_guy->get_nickname();

	// Send topic (RPL_TOPIC 332) if set
	std::string topic = channel.get_topic();
	if (!topic.empty())
	{
		std::string topic_msg = server_name + std::string(" 332 ") + nick + std::string(" ") + channel.get_name() + std::string(" :") + topic + std::string("\n");
		send(new_guy->get_fd(), topic_msg.c_str(), topic_msg.size(), 0);
		// Optional RPL_TOPICWHOTIME (333) is not implemented (no who/time tracking)
	}

	// Build NAMES list (RPL_NAMREPLY 353)
	std::string names_list = "";
	const std::map<Client*, bool>& users = channel.get_channel_users();
	for (std::map<Client*, bool>::const_iterator it = users.begin(); it != users.end(); ++it)
	{
		if (!names_list.empty())
			names_list += " ";
		if (it->second)
			names_list += "@"; // operator prefix
		names_list += it->first->get_nickname();
	}
	std::string names_msg = server_name + std::string(" 353 ") + nick + std::string(" = ") + channel.get_name() + std::string(" :") + names_list + std::string("\n");
	send(new_guy->get_fd(), names_msg.c_str(), names_msg.size(), 0);

	// End of NAMES (RPL_ENDOFNAMES 366)
	std::string end_msg = server_name + std::string(" 366 ") + nick + std::string(" ") + channel.get_name() + std::string(" :End of /NAMES list.\n");
	send(new_guy->get_fd(), end_msg.c_str(), end_msg.size(), 0);
}



std::map<std::string, bool> Server::split_target(const std::string &str)
{
    std::map<std::string, bool> tokens;
    std::stringstream ss(str);
    std::string target;

    while (std::getline(ss, target, ','))
	tokens.insert(std::make_pair(target, false));

    return tokens;
}
