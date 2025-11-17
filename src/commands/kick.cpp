#include "../../include/Server.hpp"

void Server::parseKick(std::vector<std::string> &tokens, int index)
{
	if (CHANNEL_DEBUG)
	{
		for (size_t t = 0; t < tokens.size(); ++t)
		{
			std::cout << "[KICK TOKENS] token[" << t << "] = \"" << tokens[t] << "\"" << std::endl;
		}
	}

	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	if (tokens.size() < 3)
	{
		std::string msg_error = server_name + std::string(" 461 ") + Clients[index]->get_nickname() + std::string(" KICK :Not enough parameters\r\n");
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

	std::vector<std::string> target_nicks;
	while (tokens[2].find(delimiter) != std::string::npos)
	{
		size_t pos = tokens[2].find(delimiter);
		target_nicks.push_back(tokens[2].substr(0, pos));
		tokens[2].erase(0, pos + delimiter.length());
	}
	target_nicks.push_back(tokens[2]);

	if (channel_names.size() == 1)
	{
		for (size_t i = 0; i < target_nicks.size(); i++)
		{
			std::vector<std::string> workaround_tokens;
			workaround_tokens.push_back("KICK");
			workaround_tokens.push_back(channel_names[0]);
			workaround_tokens.push_back(target_nicks[i]);
			if (tokens.size() >= 4)
				workaround_tokens.push_back(tokens[3]);
			handleKick(workaround_tokens, index);
		}
	}
	else if (channel_names.size() == target_nicks.size())
	{
		for (size_t i = 0; i < channel_names.size(); i++)
		{
			std::vector<std::string> workaround_tokens;
			workaround_tokens.push_back("KICK");
			workaround_tokens.push_back(channel_names[i]);
			workaround_tokens.push_back(target_nicks[i]);
			if (tokens.size() >= 4)
				workaround_tokens.push_back(tokens[3]);
			handleKick(workaround_tokens, index);
		}
	}
	else
	{
		std::string msg_error = server_name + std::string(" 461 ") + Clients[index]->get_nickname() + std::string(" KICK :Not enough parameters\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
}

// KICK <channel> <user> [<comment>]
// Replies used: 461 (Not enough parameters), 403 (No such channel),
// 442 (You're not on that channel), 482 (You're not channel operator),
// 441 (They aren't on that channel)
void Server::handleKick(std::vector<std::string> &tokens, int index)
{
	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	if (tokens.size() < 3)
	{
		std::string msg_error = server_name + std::string(" 461 ") + Clients[index]->get_nickname() + std::string(" KICK :Not enough parameters\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	std::string channel_name = tokens[1];
	for (size_t i = 0; i < channel_name.size(); i++)
		channel_name[i] = std::tolower(static_cast<unsigned char>(channel_name[i]));

	std::string target_nick = tokens[2];

	// Optional comment/reason
	std::string reason = "";
	if (tokens.size() >= 4)
	{
		reason = tokens[3];
		if (!reason.empty() && reason[0] == ':')
			reason = reason.substr(1);
	}

	// Find channel
	int chan_index = -1;
	for (size_t i = 0; i < Channels.size(); i++)
	{
		if (Channels[i].get_name() == channel_name)
		{
			chan_index = static_cast<int>(i);
			break;
		}
	}
	if (chan_index == -1)
	{
		std::string msg_error = server_name + std::string(" 403 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :No such channel\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	Channel &chan = Channels[chan_index];
	// Must be on channel
	if (!chan.hasClient(Clients[index]))
	{
		std::string msg_error = server_name + std::string(" 442 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :You're not on that channel\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	// Must be operator
	if (!chan.isOperator(Clients[index]))
	{
		std::string msg_error = server_name + std::string(" 482 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :You're not channel operator\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	// Find target client by nickname within server
	Client *target_client = NULL;
	for (size_t i = 0; i < Clients.size(); i++)
	{
		if (Clients[i]->get_nickname() == target_nick)
		{
			target_client = Clients[i];
			break;
		}
	}
	if (!target_client || !chan.hasClient(target_client))
	{
		std::string msg_error = server_name + std::string(" 441 ") + Clients[index]->get_nickname() + std::string(" ") + target_nick + std::string(" ") + channel_name + std::string(" :They aren't on that channel\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	// Build KICK message per RFC: :nick!user@host KICK #chan target :reason
	std::string kick_msg = ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost KICK " + channel_name + " " + target_nick;
	if (!reason.empty())
		kick_msg += " :" + reason;
	kick_msg += "\r\n";

	// Broadcast to channel (everyone sees the KICK)
	broadcastToChannel(chan_index, kick_msg);

	// Remove target from channel
	chan.removeClient(target_client);

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCESS]" << PINK << "[KICK]" << RESET << " Client " << Clients[index]->get_nickname() << " was kicked from channel " << channel_name << std::endl;
	}

	// If channel becomes empty, delete it
	if (chan.get_channel_users().empty())
		Channels.erase(Channels.begin() + chan_index);
}
