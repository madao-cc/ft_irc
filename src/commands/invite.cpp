#include "../../include/Server.hpp"

void Server::handleInvite(std::vector<std::string> &tokens, int index)
{
	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	if (tokens.size() < 3)
	{
		std::string msg_error = server_name + std::string(" 461 ") + Clients[index]->get_nickname() + std::string(" INVITE :Not enough parameters\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	std::string target_nickname = tokens[1];
	std::string channel_name = tokens[2];

	int target_index = -1;
	for (size_t i = 0; i < Clients.size(); i++)
	{
		if (Clients[i]->get_nickname() == target_nickname)
		{
			target_index = i;
			break;
		}
	}

	if (target_index == -1)
	{
		std::string msg_error = server_name + std::string(" 401 ") + Clients[index]->get_nickname() + std::string(" ") + target_nickname + std::string(" :No such nick\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	bool channel_found = false;
	for (size_t i = 0; i < Channels.size(); i++)
	{
		if (Channels[i].get_name() == channel_name)
		{
			channel_found = true;

					if (!Channels[i].hasClient(Clients[index]))
					{
						std::string msg_error = server_name + std::string(" 442 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :You're not on that channel\r\n");
						send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
						return ;
					}

			if (Channels[i].hasClient(Clients[target_index]))
			{
				std::string msg_error = server_name + std::string(" 443 ") + Clients[index]->get_nickname() + std::string(" ") + target_nickname + std::string(" ") + channel_name + std::string(" :is already on channel\r\n");
				send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
				return ;
			}

			if (Channels[i].hasInviteOnlyMode())
			{
				if (!Channels[i].isOperator(Clients[index]))
				{
					std::string msg_error = server_name + std::string(" 482 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :You're not channel operator\r\n");
					send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
					return ;
				}
			}

			Channels[i].addInvite(target_nickname);

			std::string invite_response = server_name + std::string(" 341 ") + Clients[index]->get_nickname() + std::string(" ") + target_nickname + std::string(" ") + channel_name + std::string("\r\n");
			send(Clients[index]->get_fd(), invite_response.c_str(), invite_response.size(), 0);

			std::string invite_msg = std::string(":") + Clients[index]->get_nickname() + std::string("!") + Clients[index]->get_username() + std::string("@localhost INVITE ") + target_nickname + std::string(" :") + channel_name + std::string("\r\n");
			send(Clients[target_index]->get_fd(), invite_msg.c_str(), invite_msg.size(), 0);

			if (DEBUG || EVAL)
			{
				std::cout << GREEN << "[SUCCESS]" << PINK << "[INVITE]" << RESET << " " << Clients[index]->get_nickname() 
						  << " invited " << target_nickname << " to " << channel_name << std::endl;
			}
			return ;
		}
	}

	if (!channel_found)
	{
		std::string msg_error = server_name + std::string(" 403 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :No such channel\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
}
