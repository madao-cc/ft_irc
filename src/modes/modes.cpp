#include "../../include/Channels.hpp"
#include "../../include/Server.hpp"

int Channel::handleKeyMode(std::string key)
{
	if (key.empty())
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Channel key cannot be empty" << std::endl;
		return -1;
	}
	if (key.size() > 30)
	{
		if (DEBUG || EVAL)
		{
			std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Channel key too long, maximum is 30 characters (defined by us)" << std::endl;
		}
		return -2;
	}	

	for (size_t i = 0; i < key.size(); i++)
	{
		unsigned char c = static_cast<unsigned char>(key[i]);
		if (c == 0x00 || c == 0x0A || c == 0x0D || c == 0x20 || c == 0x40)
		{
			if (DEBUG || EVAL)
				std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " Channel key contains invalid characters" << std::endl;
			return -3;
		}
	}

	addMode('k');  // Use the string-based mode system
	channel_key = key;

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS]" << RESET << "Channel key for channel " << name << " set to " << channel_key << std::endl;
	}
	return 0;
}

int Channel::removeKey()
{
	if (!hasMode('k') && channel_key.empty())
	{
		// Nothing to remove
		return -1;
	}
	removeMode('k');  // Use the string-based mode system
	channel_key = "";

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS]" << RESET << "Channel key for channel " << name << " removed" << std::endl;
	}
	return 0;
}

int Channel::handleTopicMode()
{
	addMode('t');  // Use the string-based mode system

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS]" << RESET << "Channel topic for channel " << name << " set to be changed by operators only" << std::endl;
	}
	return 0;
}

int Channel::removeTopicMode()
{
	if (!hasMode('t'))
	{
		return -1;
	}
	removeMode('t');  // Use the string-based mode system

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS]" << RESET << "Channel topic for channel " << name << " can now be changed by anyone" << std::endl;
	}
	return 0;
}

int Channel::handleInviteMode()
{
	addMode('i');  // Use the string-based mode system

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS]" << RESET << "Invite only mode set for channel " << name << std::endl;
	}
	return 0;
}

int Channel::removeInviteMode()
{
	if (!hasMode('i'))
	{
		return -1;
	}
	removeMode('i');  // Use the string-based mode system

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS]" << RESET << "Invite only mode removed for channel " << name << std::endl;
	}
	return 0;
}

int Channel::handleLimitMode(int limit)
{
	if (limit <= 0)
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " User limit must be positive" << std::endl;
		return -1;
	}

	if (limit > 999999)
	{
		if (DEBUG || EVAL)
			std::cout << RED << "[ERROR]" << PINK << "[MODE]" << RESET << " User limit too high, maximum is 999999" << std::endl;
		return -2;
	}

	addMode('l');  // Use the string-based mode system
	user_limit = limit;

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS]" << RESET << "User limit set to " << limit << " for channel " << name << std::endl;
	}
	return 0;
}

int Channel::removeLimitMode()
{
	if (!hasMode('l') && user_limit < 0)
	{
		return -1;
	}
	removeMode('l');  // Use the string-based mode system
	user_limit = -1;  // -1 means no limit

	if (DEBUG || EVAL)
	{
		std::cout << GREEN << "[SUCCESS]" << RESET << "User limit removed for channel " << name << std::endl;
	}
	return 0;
}
