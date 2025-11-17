#include "../../include/Channels.hpp"
#include <iostream>
#include <algorithm>

Channel::Channel(std::string _name, Client* creator) : mode(""), name(_name), topic(""), user_limit(-1)
{
	if (channel_users.empty())
	{
		channel_users.insert(std::make_pair(creator, true));
	}
	else
	{
		channel_users.insert(std::make_pair(creator, false));
	}
	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS] " << RESET << "Channel " << name << " created by " << creator->get_nickname() << std::endl;
	}
	
}

Channel::~Channel() {}

std::string Channel::get_name() const {return name;}

void Channel::addClient(Client* client)
{
	// Check if client is already in the channel
	if (channel_users.find(client) == channel_users.end())
	{
		channel_users.insert(std::make_pair(client, false));

		if (DEBUG)
		{
			std::cout << YELLOW << "[INFO] " << RESET << "Client's address: " << client << std::endl;
		}
	}
	else
	{
		if (DEBUG || EVAL)
		{
			std::cout << YELLOW << "[INFO] " << RESET << "Client " << client->get_nickname() << " is already in channel " << name << std::endl;
		}
	}
	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS] " << RESET << "Client " << client->get_nickname() << " added to channel " << name << std::endl;
	}


}

void Channel::removeClient(Client *client)
{
	channel_users.erase(client);

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS] " << RESET << "Client " << client->get_nickname() << " removed from channel " << name << std::endl;
	}
}

bool Channel::hasClient(Client *client) const
{
	if (DEBUG)
	{
		std::cout << YELLOW << "[INFO] " << RESET << "Checking if client's address: " << client << " is in channel " << name << std::endl;
	}
	if (channel_users.find(client) != channel_users.end())
	{
		if (DEBUG)
			std::cout << YELLOW << "[DEBUG]" << PINK << "[CHANNEL]" << RESET << " hasClient: lookup pointer=" << client << " nick=" << client->get_nickname() << " found=true" << std::endl;
		return true;
	}
	if (DEBUG)
		std::cout << YELLOW << "[DEBUG]" << PINK << "[CHANNEL]" << RESET << " hasClient: lookup pointer=" << client << " nick=" << client->get_nickname() << " found=false" << std::endl;
	return false;
}

bool Channel::isOperator(Client *client) const
{
	std::map<Client*, bool>::const_iterator it = channel_users.find(client);
	if (it != channel_users.end())
	{
		return it->second; // Return true if the client is an operator, false otherwise
	}
	return false; // Client not found in the channel
}

void Channel::set_mode(int m) {
	// Legacy function - convert int to string for backward compatibility
	mode = "";
	if (m == 1)
		mode = "i";
	else if (m == 2)
		mode = "t";
	else if (m == 3)
		mode = "k";
	else if (m == 5)
		mode = "l";
}

std::string Channel::get_mode() const {return mode;}

void Channel::set_topic(const std::string& new_topic) { topic = new_topic; }

std::string Channel::get_topic() const { return topic; }

const std::map<Client*, bool>& Channel::get_channel_users() const { return channel_users; }

const std::string Channel::get_channel_key() const { return channel_key; }

int Channel::get_user_limit() const { return user_limit; }

bool Channel::checkInvite(std::string nickname)
{
	for (size_t i = 0; i < invited_users.size(); i++)
	{
		if (invited_users[i] == nickname)
		{
			// Remove the user from the invited list after they join
			invited_users.erase(invited_users.begin() + i);
			return true;
		}
	}
	return false;
}

void Channel::addInvite(std::string nickname)
{
	// Check if the user is not already invited
	for (size_t i = 0; i < invited_users.size(); i++)
	{
		if (invited_users[i] == nickname)
			return; // Already invited
	}
	invited_users.push_back(nickname);
}

// Example: MODE <channel> {[+|-]|o} <user>
// tokens[0] = "MODE" ; tokens[1] = <channel> ; tokens[2] = {[+|-]|o} ; tokens[3] = <user>
int Channel::handleOperatorMode(Client* client, bool give, std::vector<std::string> &tokens)
{
	(void)client; // Unused parameter    
	if (tokens.size() < 4)
	{
		if (DEBUG || EVAL)
			std::cout << YELLOW << "[INFO] " << RESET << "Not enough parameters for MODE" << std::endl;
		return -1;
	}

	// Check channel name matches this channel
	// case insensitive check
	std::string channel_name = tokens[1];
	for (size_t i = 0; i < channel_name.size(); i++)
		channel_name[i] = std::tolower(static_cast<unsigned char>(channel_name[i]));
	std::string this_channel_name = name;
	for (size_t i = 0; i < this_channel_name.size(); i++)
		this_channel_name[i] = std::tolower(static_cast<unsigned char>(this_channel_name[i]));
	if (channel_name != this_channel_name)
	{
		if (DEBUG || EVAL)
			std::cout << YELLOW << "[INFO] " << RESET << "Channel name does not match" << std::endl;
		return -1;
	}
    

	std::string targetNick = tokens[3];
	Client* target = 0;

	// Find the target client in this channel
	for (std::map<Client*, bool>::iterator it = channel_users.begin(); it != channel_users.end(); ++it)
	{
		if (it->first->get_nickname() == targetNick)
		{
			target = it->first;
			break;
		}
	}

	if (!target)
	{
		if (DEBUG || EVAL)
			std::cout << YELLOW << "[INFO] " << RESET << "User " << targetNick << " is not in channel " << name << std::endl;
		return -1;
	}

	// Give operator
	if (give)
	{
		channel_users[target] = true;
		if (DEBUG || EVAL)
			std::cout << GREEN << "[SUCCESS] " << RESET << "Client " << targetNick << " is now an operator in channel " << name << std::endl;
		return 0;
	}
	// Remove operator
	else
	{
		if (!channel_users[target])
		{
			if (DEBUG || EVAL)
				std::cout << YELLOW << "[INFO] " << RESET << "Client " << targetNick << " is not an operator in channel " << name << std::endl;
			return -1;
		}
		else
		{
			channel_users[target] = false;
			if (DEBUG || EVAL)
				std::cout << GREEN << "[SUCCESS] " << RESET << "Operator removed from client " << targetNick << " in channel " << name << std::endl;
			return 0;
		}
	}
	return -1;
}

// Mode checking helper functions
bool Channel::hasInviteOnlyMode() const
{
	return mode.find('i') != std::string::npos;
}

bool Channel::hasTopicByOpsOnlyMode() const
{
	return mode.find('t') != std::string::npos;
}

bool Channel::hasKeyMode() const
{
	return mode.find('k') != std::string::npos;
}

bool Channel::hasLimitMode() const
{
	return mode.find('l') != std::string::npos;
}

// New mode management functions
void Channel::addMode(char mode_char)
{
	if (mode.find(mode_char) == std::string::npos)
	{
		mode += mode_char;
		// Keep modes sorted for consistency (optional)
		std::sort(mode.begin(), mode.end());
	}
}

void Channel::removeMode(char mode_char)
{
	size_t pos = mode.find(mode_char);
	if (pos != std::string::npos)
	{
		mode.erase(pos, 1);
	}
}

bool Channel::hasMode(char mode_char) const
{
	return mode.find(mode_char) != std::string::npos;
}
