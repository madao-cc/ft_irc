#include "../../include/Server.hpp"

void Server::handleMode(std::vector<std::string> &tokens, int index)
{
	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	if (tokens.size() < 2)
	{
		if (DEBUG)
			std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Not enough parameters" << std::endl;
		return ;
	}
	std::string channel_name = tokens[1];
	for (size_t i = 0; i < channel_name.size(); i++)
		channel_name[i] = std::tolower(static_cast<unsigned char>(channel_name[i]));

	std::string channel_mode = tokens[2];
	if (tokens.size() == 2)
	{
		channel_mode = "";
	}

	if (!checkChannelExists(channel_name))
	{
		if (DEBUG)
			std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Channel " << channel_name << " does not exist" << std::endl;
		// Send ERR_NOSUCHCHANNEL (403)
		std::string msg_error = server_name + std::string(" 403 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :No such channel\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	for (size_t i = 0; i < Channels.size(); i++)
	{
		if (Channels[i].get_name() == channel_name)
		{
			if (!Channels[i].hasClient(Clients[index]))
			{
				// Send ERR_NOTONCHANNEL (442)
				std::string msg_error = server_name + std::string(" 442 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :You're not on that channel\r\n");
				send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
				return ;
			}
			else if (!Channels[i].isOperator(Clients[index]) && tokens.size() > 2)
			{
				// Send ERR_CHANOPRIVSNEEDED (482)
				std::string msg_error = server_name + std::string(" 482 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :You're not channel operator\r\n");
				send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
				return ;
			}
			break ;
		}
	}

	if (DEBUG)
	{
		std::cout << YELLOW << "[DEBUG]" << PINK << "[MODE]" << RESET << " Client " << Clients[index]->get_nickname() << " is setting mode " << channel_mode << " for channel " << channel_name << std::endl;
	}
	
	for (size_t i = 0; i < Channels.size(); i++)
	{
		if (Channels[i].get_name() == channel_name)
		{
			std::string result_message = "";
			size_t param_index = 3; // Start from tokens[3] for parameters
			bool is_adding = true;

			// If only passed mode #b
			// Just return current modes
			if (tokens.size() == 2)
			{
				std::string current_modes = Channels[i].get_mode();
				std::string mode_msg = ":" + server_name + " MODE " + channel_name + " " + current_modes + "\r\n";
				send(Clients[index]->get_fd(), mode_msg.c_str(), mode_msg.size(), 0);
				std::cout << GREEN << "[SUCCESS]" << PINK << "[MODE]" << RESET << " Current modes for " << channel_name 
						  << ": " << current_modes << std::endl;
				return ;
			}

			
			for (size_t j = 0; j < channel_mode.length(); j++)
			{
				char c = channel_mode[j];
				if (c == '+')
				{
					is_adding = true;
					continue;
				}
				else if (c == '-')
				{
					is_adding = false;
					continue;
				}

				std::string key_msg; // Declare before switch

				if (is_adding)
				{
					switch (c)
					{
						case 'i':
							Channels[i].handleInviteMode();
							result_message += "+i ";
							broadcastToChannel(i, ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost MODE " + channel_name + " +i\r\n");
							break;
						case 't':
							if (Channels[i].handleTopicMode() == 0)
							{
								result_message += "+t ";
								broadcastToChannel(i, ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost MODE " + channel_name + " +t\r\n");
							}
							break;
						case 'k':
							if (param_index >= tokens.size())
							{
								// CHANGE: send 461 instead of only logging
								std::string err = ":" + server_name + " 461 " + Clients[index]->get_nickname() + " MODE :Not enough parameters\r\n";
								send(Clients[index]->get_fd(), err.c_str(), err.size(), 0);
								if (DEBUG)
									std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Channel key not provided for +k" << std::endl;
								return ;
							}
							if (Channels[i].handleKeyMode(tokens[param_index]) == 0)
							{
								result_message += "+k " + tokens[param_index] + " ";
								broadcastToChannel(i, ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost MODE " + channel_name + " +k *\r\n");
								key_msg = ":" + server_name + " MODE " + channel_name + " +k " + tokens[param_index] + "\r\n";
								send(Clients[index]->get_fd(), key_msg.c_str(), key_msg.size(), 0);
								param_index++;
							}
							break;
						case 'l':
							if (param_index >= tokens.size())
							{
								// CHANGE: send 461 instead of only logging
								std::string err = ":" + server_name + " 461 " + Clients[index]->get_nickname() + " MODE :Not enough parameters\r\n";
								send(Clients[index]->get_fd(), err.c_str(), err.size(), 0);
								if (DEBUG)
									std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " User limit not provided for +l" << std::endl;
								return ;
							}
							if (Channels[i].handleLimitMode(std::atoi(tokens[param_index].c_str())) == 0)
							{
								result_message += "+l " + tokens[param_index] + " ";
								broadcastToChannel(i, ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost MODE " + channel_name + " +l " + tokens[param_index] + "\r\n");
								param_index++;
							}
							break;
						case 'o':
						{
							if (param_index >= tokens.size())
							{
								// CHANGE: send 461 instead of only logging
								std::string err = ":" + server_name + " 461 " + Clients[index]->get_nickname() + " MODE :Not enough parameters\r\n";
								send(Clients[index]->get_fd(), err.c_str(), err.size(), 0);
								if (DEBUG)
									std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Username not provided for +o" << std::endl;
								return ;
							}
							int ret = Channels[i].handleOperatorMode(Clients[index], true, tokens);
							if (ret == 0)
							{
								result_message += "+o " + tokens[param_index] + " ";
								broadcastToChannel(i, ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost MODE " + channel_name + " +o " + tokens[param_index] + "\r\n");
								param_index++;
							}
						}
							break;
						default:
							// Unknown mode character
							if (DEBUG || EVAL)
								std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Unknown mode character: " << c << std::endl;
							break;
					}
				}
				else
				{
					switch (c)
					{
						case 'i':
							if (Channels[i].removeInviteMode() == 0)
							{
								result_message += "-i ";
								broadcastToChannel(i, ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost MODE " + channel_name + " -i\r\n");
							}
							break;
						case 't':
							if (Channels[i].removeTopicMode() == 0)
							{
								result_message += "-t ";
								broadcastToChannel(i, ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost MODE " + channel_name + " -t\r\n");
							}
							break;
						case 'k':
							if (Channels[i].removeKey() == 0)
							{
								result_message += "-k ";
								broadcastToChannel(i, ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost MODE " + channel_name + " -k\r\n");
							}
							break;
						case 'l':
							if (Channels[i].removeLimitMode() == 0)
							{
								result_message += "-l ";
								broadcastToChannel(i, ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost MODE " + channel_name + " -l\r\n");
							}
							break;
						case 'o':
						{
							if (param_index >= tokens.size())
							{
								// CHANGE: send 461 instead of only logging
								std::string err = ":" + server_name + " 461 " + Clients[index]->get_nickname() + " MODE :Not enough parameters\r\n";
								send(Clients[index]->get_fd(), err.c_str(), err.size(), 0);
								if (DEBUG)
									std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Username not provided for -o" << std::endl;
								return ;
							}
							int ret = Channels[i].handleOperatorMode(Clients[index], false, tokens);
							if (ret == 0)
							{
								result_message += "-o " + tokens[param_index] + " ";
								broadcastToChannel(i, ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@localhost MODE " + channel_name + " -o " + tokens[param_index] + "\r\n");
								param_index++;
							}
						}
							break;
						default:
							// Unknown mode character
							if (DEBUG || EVAL)
								std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Unknown mode character: " << c << std::endl;
							break;
					}
				}
			}
			if (DEBUG || EVAL)
				std::cout << GREEN << "[SUCCESS]" << PINK << "[MODE]" << RESET << " Channel modes for " << channel_name << " changed: " << result_message << "(active modes: " << Channels[i].get_mode() << ")" << std::endl;
			return ;
		}
	}
	if (DEBUG || EVAL)
		std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Invalid channel mode" << std::endl;
	std::string err = ":" + server_name + " 472 " + Clients[index]->get_nickname() + " * :is unknown mode char to me\r\n";
	send(Clients[index]->get_fd(), err.c_str(), err.size(), 0);
	return ;
}