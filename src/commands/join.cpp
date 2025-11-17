#include "../../include/Server.hpp"

// GOOD EXAMPLE:
// JOIN #channel1,#channel2 key1,key2
// tokens[1] = "#channel1,#channel2"
// tokens[2] = "key1,key2"

// BAD EXAMPLE:
// JOIN #channel1 , #channel2 key1 , key2
// tokens[1] = "#channel1"
// tokens[2] = ","
void Server::parseJoin(std::vector<std::string> &tokens, int index)
{
	if (CHANNEL_DEBUG)
	{
		for (size_t t = 0; t < tokens.size(); ++t)
		{
			std::cout << "[JOIN TOKENS] token[" << t << "] = \"" << tokens[t] << "\"" << std::endl;
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
		std::string nickname = Clients[index]->get_nickname();
		std::string msg_error = server_name + std::string(" 461 ") + nickname + std::string(" JOIN :") + std::string("Not enough parameters\r\n") ;
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

	for (size_t i = 0; i < channel_names.size(); ++i)
	{
		for (size_t j = 0; j < channel_names[i].size(); ++j)
			channel_names[i][j] = std::tolower(static_cast<unsigned char>(channel_names[i][j]));
	}

	// Special case: JOIN 0 -> part from all channels the client is in
	if (channel_names.size() == 1 && channel_names[0] == "0")
	{
		std::vector<std::string> user_channels;
		for (size_t i = 0; i < Channels.size(); ++i)
		{
			if (Channels[i].hasClient(Clients[index]))
				user_channels.push_back(Channels[i].get_name());
		}

		for (size_t i = 0; i < user_channels.size(); ++i)
		{
			std::vector<std::string> workaround_tokens;
			workaround_tokens.push_back("PART");
			workaround_tokens.push_back(user_channels[i]);
			handlePart(workaround_tokens, index);
		}

		return;
	}

	std::vector<std::string> channel_keys;
	if (tokens.size() >= 3)
	{
		std::string keys_str = tokens[2];
		while (keys_str.find(delimiter) != std::string::npos)
		{
			size_t pos = keys_str.find(delimiter);
			channel_keys.push_back(keys_str.substr(0, pos));
			keys_str.erase(0, pos + delimiter.length());
		}
		channel_keys.push_back(keys_str);
	}
	// Above block:
	// JOIN #channel1,#channel2 key1,key2
	// channel_names = ["#channel1", "#channel2"]
	// channel_keys = ["key1", "key2"]

	//JOIN #channel1,#channel2,#channel3 key1,,key3
	// channel_names = ["#channel1", "#channel2", "#channel3"]
	// channel_keys = ["key1", "", "key3"]

	for (size_t i = 0; i < channel_names.size(); i++)
	{
		std::vector<std::string> workaround_tokens;
		workaround_tokens.push_back("JOIN");
		workaround_tokens.push_back(channel_names[i]);
		if (i < channel_keys.size())
			workaround_tokens.push_back(channel_keys[i]);
		else
			workaround_tokens.push_back("");
		handleJoin(workaround_tokens, index);
	}

}

void Server::handleJoin(std::vector<std::string> &tokens, int index)
{
	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	
	if (tokens.size() < 2)
	{
		std::string nickname = Clients[index]->get_nickname();
		std::string msg_error = server_name + std::string(" 461 ") + nickname + std::string(" JOIN :") + std::string("Not enough parameters\r\n") ;
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	std::string channel_name = tokens[1];

	if (channel_name.empty() || channel_name[0] != '#')
	{
		std::string msg_error = server_name + std::string(" 476 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :Invalid channel name\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	if (channel_name.length() < 2)
	{
		std::string msg_error = server_name + std::string(" 476 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :Invalid channel name\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	for (size_t i = 0; i < channel_name.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(channel_name[i]);
		if (c == 0x00 || c == 0x07 || c == 0x20 || c == 0x2C)
		{
			std::string msg_error = server_name + std::string(" 476 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :Invalid channel name\r\n");
			send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
			return ;
		}
	}

	if (channel_name.length() > 200)
	{
		std::string msg_error = server_name + std::string(" 476 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :Invalid channel name\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	for (size_t i = 0; i < channel_name.size(); i++)
		channel_name[i] = std::tolower(static_cast<unsigned char>(channel_name[i]));

	if (Channels.empty())
	{
		Channel new_channel(channel_name, Clients[index]);
		Channels.push_back(new_channel);

		size_t new_channel_index = Channels.size() - 1;
		std::string join_msg = std::string(":") + Clients[index]->get_nickname() + std::string("!") + Clients[index]->get_username() + std::string("@localhost JOIN ") + channel_name + std::string("\r\n");
		broadcastToChannel(new_channel_index, join_msg);
		// Send topic and NAMES list to the joining client
		sendTopicAndNames(new_channel_index, index);
	}
	else
	{
		for (size_t i = 0; i < Channels.size(); i++)
		{
				if (Channels[i].get_name() == channel_name)
			{
				if (Channels[i].hasClient(Clients[index]))
					return ;
				else
				{
					// If user is invited, bypass all other mode checks
						bool invited = Channels[i].checkInvite(Clients[index]->get_nickname());
					if (!invited)
					{
						// Enforce invite-only
						if (Channels[i].hasInviteOnlyMode())
						{
							std::string msg_error = server_name + std::string(" 473 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :Cannot join channel (+i)\r\n");
							send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
							return ;
						}

						// Enforce key
						if (Channels[i].hasKeyMode())
						{
							if (tokens.size() < 3 || tokens[2] != Channels[i].get_channel_key())
							{
								std::string msg_error = server_name + std::string(" 475 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :Cannot join channel (+k)\r\n");
								send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
								return ;
							}
						}

						// Enforce limit
						if (Channels[i].hasLimitMode())
						{
							int current_users = Channels[i].get_channel_users().size();
							int limit = Channels[i].get_user_limit();
							if (current_users >= limit)
							{
								std::string msg_error = server_name + std::string(" 471 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :Cannot join channel (+l)\r\n");
								send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
								return ;
							}
						}
					}

					// Allow join
						Channels[i].addClient(Clients[index]);
						std::string join_msg = std::string(":") + Clients[index]->get_nickname() + std::string("!") + Clients[index]->get_username() + std::string("@localhost JOIN ") + channel_name + std::string("\r\n");
					broadcastToChannel(i, join_msg);
					// Send topic and NAMES list to the joining client
					sendTopicAndNames(i, index);
					return ;
				}
			}
		}
		Channel new_channel(channel_name, Clients[index]);
		Channels.push_back(new_channel);
		size_t new_channel_index = Channels.size() - 1;
		std::string join_msg = std::string(":") + Clients[index]->get_nickname() + std::string("!") + Clients[index]->get_username() + std::string("@localhost JOIN ") + channel_name + std::string("\r\n");
		broadcastToChannel(new_channel_index, join_msg);
		// Send topic and NAMES list to the joining client
		sendTopicAndNames(new_channel_index, index);
	}

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS]" << PINK << "[JOIN]" << RESET << " Client " << Clients[index]->get_nickname() << " joined channel " << channel_name << std::endl;
	}
}
