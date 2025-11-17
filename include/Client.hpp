#pragma once

#include "Common.hpp"

class Client
{
	private:
		int		server_fd;
		std::string	username;
		std::string	nickname;
		std::string	realname;
		struct pollfd	pfd;
		int		fd;
		bool		bool_disconnected;
		bool		bool_registered;
		bool		bool_password_implemented;
		bool		bool_wellcome;

	public:
		Client(int server_fd, int client_fd);
		~Client();
		
		void set_fd(int _fd);
		void set_username(std::string name);
		void set_nickname(std::string name);
		void set_realname(std::string name);
	
		void set_bool_wellcome(bool x);
		void set_bool_registered(bool x);
		void set_bool_password_implemented(bool x);
		

		pollfd		get_pollfd()				const;
		int		get_fd()				const;
		std::string	get_hostname()				const;
		std::string	get_nickname()				const;
		std::string	get_username()				const;
		std::string	get_realname()				const;
		bool		get_bool_disconnected() 		const;
		bool 		get_bool_registered() 			const;
		bool 		get_bool_password_implemented() 	const;
		bool		get_bool_wellcome()				const;
		 
		bool check_nickname(std::string _nick_name);

};
