#include "../../include/Server.hpp"

void Server::handleTopic(std::vector<std::string> &tokens, int index)
{
	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	if (tokens.size() < 2)
	{
		std::string msg_error = server_name + std::string(" 461 ") + Clients[index]->get_nickname() + std::string(" TOPIC :Not enough parameters\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	
	std::string channel_name = tokens[1];
	for (size_t i = 0; i < channel_name.size(); i++)
		channel_name[i] = std::tolower(static_cast<unsigned char>(channel_name[i]));
	
	if (!checkChannelExists(channel_name))
	{
		std::string msg_error = server_name + std::string(" 403 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :No such channel\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	
	for (size_t i = 0; i < Channels.size(); i++)
	{
		if (Channels[i].get_name() == channel_name)
		{
			if (tokens.size() == 2)
			{
				if (Channels[i].get_topic().empty())
				{
					std::string msg_no_topic = server_name + std::string(" 331 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :No topic is set\r\n");
					send(Clients[index]->get_fd(), msg_no_topic.c_str(), msg_no_topic.size(), 0);
				}
				else
				{
					std::string msg_topic = server_name + std::string(" 332 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :") + Channels[i].get_topic() + std::string("\r\n");
					send(Clients[index]->get_fd(), msg_topic.c_str(), msg_topic.size(), 0);
				}
			}
			else if (tokens.size() == 3)
			{
				std::string new_topic = tokens[2];
				if (!new_topic.empty() && new_topic[0] == ':')
					new_topic = new_topic.substr(1);

				if (!Channels[i].hasClient(Clients[index]))
				{
					std::string msg_error = server_name + std::string(" 442 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :You're not on that channel\r\n");
					send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
					return;
				}

				if (Channels[i].hasTopicByOpsOnlyMode() && !Channels[i].isOperator(Clients[index]))
				{
					std::string msg_error = server_name + std::string(" 482 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :You're not channel operator\r\n");
					send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
					return;
				}

				if (new_topic.length() > 390)
				{
					std::string msg_error = server_name + std::string(" 414 ") + Clients[index]->get_nickname() + std::string(" ") + channel_name + std::string(" :Topic too long\r\n");
					send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
					return;
				}

				Channels[i].set_topic(new_topic);
				std::string topic_msg = ":" + Clients[index]->get_nickname() + "!" + Clients[index]->get_username() + "@" + server_name + " TOPIC " + channel_name + " :" + new_topic + "\r\n";
				broadcastToChannel(i, topic_msg);

				if (DEBUG || EVAL)
					std::cout << GREEN << "[SUCCESS]" << PINK << "[TOPIC]" << RESET << " Topic for channel " << channel_name << " set to: " << new_topic << std::endl;
			}
			return;
		}
	}
}
