#include "../../include/Server.hpp"

void Server::parsePart(std::vector<std::string> &tokens, int index)
{
	if (CHANNEL_DEBUG)
	{
		for (size_t t = 0; t < tokens.size(); ++t)
		{
			std::cout << "[PART TOKENS] token[" << t << "] = \"" << tokens[t] << "\"" << std::endl;
		}
	}

	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	if (tokens.size() < 2)
	{
		std::string msg_error = server_name + std::string(" 461 ") + Clients[index]->get_nickname() + std::string(" PART :Not enough parameters\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	std::vector<std::string> channel_names;
	std::string delimiter = ",";
	while (tokens[1].find(delimiter) != std::string::npos)
	{
		size_t pos = tokens[1].find(delimiter);
		channel_names.push_back(tokens[1].substr(0, pos));
		tokens[1].erase(0, pos + delimiter.length());
	}
	channel_names.push_back(tokens[1]);

	for (size_t i = 0; i < channel_names.size(); i++)
	{
		std::vector<std::string> workaround_tokens;
		workaround_tokens.push_back("PART");
		workaround_tokens.push_back(channel_names[i]);
		if (tokens.size() >= 3)
			workaround_tokens.push_back(tokens[2]);
		handlePart(workaround_tokens, index);
	}
}

void Server::handlePart(std::vector<std::string> &tokens, int index)
{
	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	if (tokens.size() < 2)
	{
		std::string msg_error = server_name + std::string(" 461 ") + Clients[index]->get_nickname() + std::string(" PART :Not enough parameters\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	std::string channel_name = tokens[1];
	for (size_t i = 0; i < channel_name.size(); i++)
		channel_name[i] = std::tolower(static_cast<unsigned char>(channel_name[i]));

	std::string part_reason = "";
	if (tokens.size() >= 3)
	{
		part_reason = tokens[2];
		if (!part_reason.empty() && part_reason[0] == ':')
			part_reason = part_reason.substr(1);
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

			std::string part_msg;
			if (part_reason.empty())
				part_msg = std::string(":") + Clients[index]->get_nickname() + std::string("!") + Clients[index]->get_username() + std::string("@localhost PART ") + channel_name + std::string("\r\n");
			else
				part_msg = std::string(":") + Clients[index]->get_nickname() + std::string("!") + Clients[index]->get_username() + std::string("@localhost PART ") + channel_name + std::string(" :") + part_reason + std::string("\r\n");
			broadcastToChannel(i, part_msg, Clients[index]);

			Channels[i].removeClient(Clients[index]);

			if (Channels[i].get_channel_users().empty())
			{
				Channels.erase(Channels.begin() + i);
				if (DEBUG || EVAL)
					std::cout << YELLOW << "[INFO]" << PINK << "[PART]" << RESET << " Channel " << channel_name << " removed (empty)" << std::endl;
			}

			if (DEBUG || EVAL)
			{
				std::cout << GREEN << "[SUCCESS]" << PINK << "[PART]" << RESET << " Client " << Clients[index]->get_nickname() << " left channel " << channel_name;
				if (!part_reason.empty())
					std::cout << " with reason: " << part_reason;
				std::cout << std::endl;
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
