#include "../../include/Server.hpp"

void Server::handleUser(std::vector<std::string> &tokens, int index)
{
	if (!Clients[index]->get_bool_password_implemented())
	{
		std::string msg_error = server_name + std::string(" 464 * :Password required\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	if (tokens.size() < 5)
	{
		std::string _nick = Clients[index]->get_nickname();
		std::string msg_error = server_name + std::string(" 461 ") + _nick + std::string(" USER :") + std::string("Not enough parameters\r\n") ;
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	std::string username = tokens[1];

	if (username.length() > 10)
	{
		username = username.substr(0,10);
		Clients[index]->set_username(username);
		if (DEBUG || EVAL)
			std::cout << YELLOW << "[INFO]" << PINK << "[USER]" << RESET << " Username too long, silently truncated to " << Clients[index]->get_username() << std::endl;
	}

	for (size_t i = 0; i < username.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(username[i]);
		if (c == 0x00 || c == 0x0A || c == 0x0D || c == 0x20 || c == 0x40)
		{
			std::string msg_error = server_name + std::string(" 432 ") + Clients[index]->get_nickname() + std::string(" USER :Erroneous username\r\n");
			send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
			return ;
		}
	}

	if (!std::isalpha(static_cast<unsigned char>(username[0])))
	{
		std::string msg_error = server_name + std::string(" 432 ") + Clients[index]->get_nickname() + std::string(" USER :Erroneous username\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	Clients[index]->set_username(username);
	if (DEBUG || EVAL)
		std::cout << GREEN << "[SUCCESS]" << PINK << "[USER]" << RESET << " Username set to " << username << std::endl;

	if (tokens[4][0] == ':')
		Clients[index]->set_realname(tokens[4].substr(1));
	else
		Clients[index]->set_realname(tokens[4]);

	if (!Clients[index]->get_nickname().empty() && !Clients[index]->get_username().empty())
		Clients[index]->set_bool_registered(true);

	std::cout << YELLOW << "[INFO]" << RESET << " Realname set to " << Clients[index]->get_realname() << std::endl;
}
