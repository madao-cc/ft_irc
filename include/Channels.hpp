#pragma once

#include "Common.hpp"
#include "Client.hpp"

class Channel
{
	private:
		std::string	mode;
		std::string	name;
		std::string	topic;
		std::string	channel_key;
		int		user_limit; 			 //? -1 means no limit
		std::vector<std::string>	invited_users;
		std::map<Client*, bool> channel_users;

	public:
		Channel(std::string _name, Client* creator);
		~Channel();

		std::string			get_name() const;
		std::string			get_mode() const;
		std::string			get_topic() const;
		const std::map<Client*, bool>&	get_channel_users() const;
		const std::string		get_channel_key() const;
		int				get_user_limit() const;

		void			set_mode(int mode);
		void			set_topic(const std::string& new_topic);

		void			addMode(char mode_char);
		void			removeMode(char mode_char);
		bool			hasMode(char mode_char) const;

		void addClient(Client* client);
		void removeClient(Client* client);
		bool hasClient(Client* client) const;
		bool isOperator(Client* client) const;

	int handleKeyMode(std::string key);
	int removeKey();
	int handleTopicMode();
	int removeTopicMode();
	int handleInviteMode();
	int removeInviteMode();
	int handleOperatorMode(Client* client, bool give, std::vector<std::string> &tokens);
	int handleLimitMode(int limit);
	int removeLimitMode();

		bool checkInvite(std::string nickname);
		void addInvite(std::string nickname);

		bool hasInviteOnlyMode() const;
		bool hasTopicByOpsOnlyMode() const;
		bool hasKeyMode() const;
		bool hasLimitMode() const;
};
