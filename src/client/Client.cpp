#include "../../include/Client.hpp"

Client::Client(int _server_fd, int client_fd) : server_fd(_server_fd),fd(client_fd), bool_disconnected(false), bool_registered(false), bool_password_implemented(false), bool_wellcome(true)
{
	if (DEBUG)
		std::cout << YELLOW << "[INFO] " << RESET << "Constructor for new Client was called, fd: " << client_fd << RESET << std::endl;
	pfd.fd = fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
}

Client::~Client() {}

bool Client::get_bool_disconnected() const {return bool_disconnected;}
bool Client::get_bool_registered() const {return bool_registered;}
bool Client::get_bool_password_implemented() const {return bool_password_implemented;}
bool Client::get_bool_wellcome() const {return bool_wellcome;}

int Client::get_fd() const {return fd;}
pollfd Client::get_pollfd() const {return pfd;}
std::string Client::get_username() const {return username;}
std::string Client::get_nickname() const {return nickname;}
std::string Client::get_realname() const {return realname;}

void Client::set_fd(int _fd) {fd = _fd;}
void Client::set_username(std::string name) {username = name;}
void Client::set_nickname(std::string name) {nickname = name;}
void Client::set_realname(std::string name) {realname = name;}
void Client::set_bool_wellcome(bool x) {bool_wellcome = x;}
void Client::set_bool_registered(bool x) {bool_registered = x;}
void Client::set_bool_password_implemented(bool x) {bool_password_implemented = x;}
