#include "../../include/Server.hpp"

void Server::handlePass(std::vector<std::string> &str_vtr, int index)
{
	std::string _nick = std::string("");
	if (Clients[index]->get_bool_password_implemented()) 
	{
		if (Clients[index]->get_nickname().empty())
			_nick = "*";
		else
			_nick = Clients[index]->get_nickname();
		std::string msg_error = server_name + std::string(" 462 ") + _nick + std::string(" You may not reregister\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	if (str_vtr.size() < 2)
	{
		std::string msg_error = server_name + std::string(" 461 * PASS :Not enough parameters\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return;
	}
	if (str_vtr[1] != password)
	{
		std::string error_msg = server_name + std::string(" 464 * :Password incorrect\r\n");
		
		int fd = Clients[index]->get_fd();
		send(fd, error_msg.c_str(), error_msg.size(), 0);
		
		
		
		
		removeClient(index);
	return;
	}
	if (DEBUG || EVAL)
		std::cout << GREEN <<  "[SUCCESS] " << RESET << "Correct Password" << std::endl;
	Clients[index]->set_bool_password_implemented(true);
	return ;
}
