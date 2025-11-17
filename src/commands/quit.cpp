#include "../../include/Server.hpp"

void Server::send_quit_msg(std::vector<std::string> &str_vtr, int index, int channel_index)
{
	std::string privmsg;
	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	sender_index = index;
	if (str_vtr.size() < 2)
	{
		privmsg = ":" + Clients[sender_index]->get_nickname() + "!" + Clients[sender_index]->get_username() + "@localhost QUIT: Client disconnected with no message\r\n";
		broadcastToChannel(channel_index,  privmsg, Clients[sender_index]);
		return;
	}
	std::string message = str_vtr[1];
	if (!message.empty() && message[0] == ':')
		message = message.substr(1);
	msg2send = message + "\r\n";
	std::cout << RED<< msg2send <<RESET<< std::endl;
	
	if (msg2send.empty())
	{
		privmsg = ":" + Clients[sender_index]->get_nickname() + "!" + Clients[sender_index]->get_username() + "@localhost QUIT: Client disconnected with no message \r\n";
		broadcastToChannel(channel_index,  privmsg, Clients[sender_index]);	
	}
	else
	{	
		privmsg = ":" + Clients[sender_index]->get_nickname() + "!" + Clients[sender_index]->get_username() + "@localhost QUIT: Client disconnected with the message: " + msg2send ;
		broadcastToChannel(channel_index,  privmsg, Clients[sender_index]);
	}
	msg2send.clear();
	message.clear();
	return;
}

void Server::handleQuit(std::vector<std::string> &tokens, int index)
{
	if (!Clients[index]->get_bool_registered())
	{
		std::string msg_error = server_name + std::string(" 451 * : You have not registered\r\n");
		send(Clients[index]->get_fd(), msg_error.c_str(), msg_error.size(), 0);
		return ;
	}
	// todo serach for every channel that im in , and send a quiting msg 
		int channel_index = 0;
		for (std::vector<Channel>::iterator it = Channels.begin(); it != Channels.end(); it++) // travel through channels
		{
			
			for (std::map<Client *, bool>::const_iterator jt  = it->get_channel_users().begin() ; jt != it->get_channel_users().end() ; jt++)
			{// travel through users of each channel
				if (jt->first->get_nickname() == Clients[index]->get_nickname())
				{
					//todo brodcast
					send_quit_msg(tokens, index, channel_index);
					break;
				}
			}
			channel_index++;
		}
	tokens.clear();

	removeClient(index);
}
