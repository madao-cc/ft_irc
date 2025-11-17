#include "../../include/Server.hpp"

// Example: PRIVMSG <destination> :<message>
void Server::handlePrivmsg(std::vector<std::string> &str_vtr, int index)
{
	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	sender_index = index;
	if (str_vtr.size() < 3)
	{
		std::string msg_error = server_name + std::string(" 461 ") + Clients[index]->get_nickname() + std::string(" PRIVMSG : Not enough parameters\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}

	target = str_vtr[1];
	std::string message = str_vtr[2];
	if (!message.empty() && message[0] == ':')
		message = message.substr(1);

	msg2send = message;

	//! INICIO DA PARTE DO NANDO
	if(!target.empty() && target.find(',') != std::string::npos) // if i find ','
	{
		std::map<std::string, bool> all_targets  = split_target(target); // the bool will be used to check if it exist or not, then after all the loop we send a error msg in case on being false the existence of it
		for (std::map<std::string, bool>::iterator it = all_targets.begin(); it != all_targets.end(); it++)
		{
			for (client_iterator client_it = Clients.begin(); client_it !=  Clients.end(); client_it++)
			{
				if (it->first == (*client_it)->get_nickname())
				{
					//std::cout << RED << it->first << RESET << std::endl; 
					it->second = true;
					std::string privmsg = ":" + Clients[sender_index]->get_nickname() + "!" + Clients[sender_index]->get_username() + "@" + server_name + " PRIVMSG " + it->first + " :" + msg2send + "\r\n";
					send((*client_it)->get_fd(), privmsg.c_str(), privmsg.size(), 0);
				}
			}			
			for (channel_iterator channel_it = Channels.begin(); channel_it != Channels.end(); channel_it++)
			{
				if (it->first == channel_it->get_name())
				{
					it->second = true;
					int chan_index = std::distance(Channels.begin(), channel_it);
					if (!Channels[chan_index].hasClient(Clients[index]))
					{
						std::string msg_error = server_name + std::string(" 404 ") + Clients[index]->get_nickname() + std::string(" ") + it->first + std::string(" :Cannot send to channel\r\n");
						send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
					}
					else
					{
						std::string privmsg = ":" + Clients[sender_index]->get_nickname() + "!" + Clients[sender_index]->get_username() + "@" + server_name + " PRIVMSG " + it->first + " :" + msg2send + "\r\n";
						int channel_index = std::distance(Channels.begin(), channel_it);
						broadcastToChannel(channel_index, privmsg.c_str(), Clients[index]);
					}
				}
			}
		}
		// todo
		//! user -> cant send to itself
		//!user -> need to be in channel to send msg
		//todo -> check again for the not found, they dont exist, send error msg to the sender
		for (std::map<std::string, bool>::iterator it = all_targets.begin(); it != all_targets.end(); it++)
		{
			if (it->second == false)
			{
				if(it->first.find('#', 0) != std::string::npos) // channel
				{
					std::string msg_error = server_name + std::string(" 403 ") + Clients[index]->get_nickname() + std::string(" ") + it->first + std::string(" :No such channel\r\n");
					send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
				}
				else // client
				{
					std::string msg_error = server_name + std::string(" 401 ") + Clients[index]->get_nickname() + std::string(" ") + it->first + std::string(" :No such nick\r\n");
					send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
				}
			}
		}

		return ;
	}
	//! FIM DA PARTE DO NANDO

	// If target is a channel (starts with '#'), broadcast to channel
	if (!target.empty() && target[0] == '#')
	{
		// Find channel index
		size_t chan_idx = Channels.size();
		for (size_t i = 0; i < Channels.size(); ++i)
		{
			if (Channels[i].get_name() == target)
			{
				chan_idx = i;
				break;
			}
		}
		if (chan_idx == Channels.size())
		{
			// No such channel
			std::string msg_error = server_name + std::string(" 403 ") + Clients[index]->get_nickname() + std::string(" ") + target + std::string(" :No such channel\r\n");
			send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
			return;
		}
		else
		{
			if (!Channels[chan_idx].hasClient(Clients[index]))
			{
				std::string msg_error = server_name + std::string(" 404 ") + Clients[index]->get_nickname() + std::string(" ") + target + std::string(" :Cannot send to channel\r\n");
				send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
				// Clean up
				msg2send.clear();
				target.clear();
				str_tokens.clear();
				return ;
			}
			if (msg2send.empty())
			{
				std::string msg_error = server_name + std::string(" 412 ") + Clients[sender_index]->get_nickname() + std::string(" PRIVMSG :No text to send\r\n");
				send(Clients[sender_index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);

			msg2send.clear();
			target.clear();
			str_tokens.clear();
			return ;
			}
			// Build the message and broadcast to channel (exclude sender)
			std::string privmsg = ":" + Clients[sender_index]->get_nickname() + "!" + Clients[sender_index]->get_username() + "@" + server_name + " PRIVMSG " + target + " :" + msg2send + "\r\n";
			broadcastToChannel(chan_idx, privmsg, Clients[sender_index]);
		}
		// Clean up
		msg2send.clear();
		target.clear();
		str_tokens.clear();
		return;
	}

	// Otherwise, target is a user nickname
	bool found = false;
	for (size_t i = 0; i < Clients.size(); i++)
	{
		if (target == Clients[i]->get_nickname())
		{
			found = true;
			if (msg2send.empty())
			{
				std::string msg_error = server_name + std::string(" 412 ") + Clients[sender_index]->get_nickname() + std::string(" PRIVMSG :No text to send\r\n");
				send(Clients[sender_index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
				msg2send.clear();
				target.clear();
				str_tokens.clear();
				return ;
			}
			std::string privmsg = ":" + Clients[sender_index]->get_nickname() + "!" + Clients[sender_index]->get_username() + "@" + server_name + " PRIVMSG " + target + " :" + msg2send + "\r\n";
			send(Clients[i]->get_fd(), privmsg.c_str(), privmsg.size(), 0);

			msg2send.clear();
			target.clear();
			str_tokens.clear();

			break ;
		}
	}
	if (!found)
	{
		std::string err = server_name + std::string(" 401 ") + Clients[sender_index]->get_nickname() + std::string(" ") + target + std::string(" :No such nick\r\n");
		send(Clients[sender_index]->get_fd(), err.c_str(), err.size(), 0);
	}
}