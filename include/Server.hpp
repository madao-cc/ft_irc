#pragma once

#include "Common.hpp"
#include "Client.hpp"
#include "Channels.hpp"

typedef std::vector<Client*>::iterator client_iterator;
typedef std::vector<pollfd>::iterator poll_iterator;
typedef std::vector<Channel>::iterator channel_iterator;
typedef std::vector<std::string>::iterator str_iterator;

//? Global server pointer for signal handler access

class Server
{
	private:
		int			port;
		int			server_fd;
		bool			stop_server;

		struct pollfd		*pollFdPtr; 
		int			pollVectorSize;

		const std::string	password;
	
		std::vector <Client*>	Clients;
		std::vector <pollfd>	pollFds;
		std::vector <Channel>	Channels;
	
		struct sockaddr_in	sock_addr;
		int			sender_index;
		char			buffer[512];

		int		targetFd;
		std::string	cmd;
		std::string	target;
		std::string	msg2send;
		std::string	server_name;

		std::vector<std::string>	str_tokens;
		std::map<int, std::string>	clientBuffers;

		std::map<std::string, void(Server::*)(std::vector<std::string> &, int)> commandHandlers;

	public:
		Server(int port_number, std::string password);
		~Server();

		void serverLoop();
		void newClient();

		void manageMessage(int i);
		void parseCommand(std::string raw_msg, int index);
		std::vector<std::string> split(const std::string &str);
		std::map<std::string, bool> split_target(const std::string &str);
		void sendMessage();
		void Client_registration();
		void removeClient(int index);
		void exit_from_pass(int index);

		void checkCommandHandlers(int index);
		void wellcoming_client(int index);

		// Connection commands
		void handleNick(std::vector<std::string> &tokens, int client_index);
		void handleUser(std::vector<std::string> &tokens, int client_index);
		void handlePass(std::vector<std::string> &tokens, int client_index);

		// Channel commands
		void handleJoin(std::vector<std::string> &tokens, int client_index);
		void handlePart(std::vector<std::string> &tokens, int client_index);
		void handleTopic(std::vector<std::string> &tokens, int client_index);
		void handleNames(std::vector<std::string> &tokens, int client_index);
		void handleList(std::vector<std::string> &tokens, int client_index);
		void handleKick(std::vector<std::string> &tokens, int client_index);
		void handleInvite(std::vector<std::string> &tokens, int client_index);
		void handleMode(std::vector<std::string> &tokens, int client_index);

		// Messaging commands
		void handleNotice(std::vector<std::string> &tokens, int client_index);
		void handlePrivmsg(std::vector<std::string> &str_vtr, int client_index);

		void send_quit_msg(std::vector<std::string> &str_vtr, int index, int channel_index);

		// Server commands
		void handlePing(std::vector<std::string> &tokens, int client_index);
		void handlePong(std::vector<std::string> &tokens, int client_index);
		void handleQuit(std::vector<std::string> &tokens, int client_index);
		void handleWho(std::vector<std::string> &tokens, int client_index);
		void handleWhois(std::vector<std::string> &tokens, int client_index);

		bool checkChannelExists(std::string channel_name);

		void broadcastToChannel(size_t channel_index, const std::string& message, Client* exclude = NULL);

		// After a successful JOIN, send topic (332) and NAMES (353) + end (366)
		void sendTopicAndNames(size_t channel_index, int client_index);

		void parseJoin(std::vector<std::string> &tokens, int index);
		void parsePart(std::vector<std::string> &tokens, int index);
		void parseKick(std::vector<std::string> &tokens, int index);
};
