#include "client.hpp"

Client::Client(int client_fd, int server_fd) : client_fd(client_fd), server_fd(server_fd)
{
    std::cout << GREEN << "SUCCESS: " << RESET << "Client created with fd " << client_fd << std::endl;

    poll_fd.fd = client_fd;
    poll_fd.events = POLLIN;
    poll_fd.revents = 0;

    
}

