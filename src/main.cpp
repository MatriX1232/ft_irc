#include <string>
#include <vector>
#include "../include/Server.hpp"
#include "../include/utils.hpp"
#include "../include/Message.hpp"
#include "../include/Channel.hpp"
#include "../include/Commands.hpp"
#include "../include/Headers.hpp"
#include "../include/Outline.hpp"

int to_shutdown = 0;

std::vector<Message>    check_for_new_messages(Server &server)
{
    // Use references to avoid copying client state.
    std::vector<Message>    _newMessages;
    std::vector<Client>     & _clients = server.get_clients();

    for (int i = 0; i < (int)_clients.size(); i++)
    {
        Message msg = server.recv(_clients[i]);
        if (msg.isValid())  
            _newMessages.push_back(msg);
    }
    return (_newMessages);
}

static void handleSignal(int signal)
{
    std::cout << INFO << "Received signal: " << signal << " . Shutting down." << std::endl;
    to_shutdown = 1;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << Outline("Usage: ./ircserv <port> <password>", RED, WHITE, "WARNING");
        std::cout << Outline("Example: ./ircserv 25565 mypassword", CYAN, WHITE, "INFO");
        return (EXIT_FAILURE);
    }

    signal(SIGINT, handleSignal);
    signal(SIGQUIT, handleSignal);

    std::cout << Outline("Welcome to the IRC server", GREEN, WHITE, "INFO");
    std::cout << Outline("Made by MSOLINSK and IDOMAGAL", YELLOW, WHITE, "INFO");
    std::cout << Outline("ShortText", WHITE, RED, "Example of some long text that is longer than main one") << std::endl << std::endl;

    Server server(atoi(argv[1]), argv[2]);
    server.start_listening(5);

    int listen_fd = server.getListenFd();
    int flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);

    Channel channel("general", "General channel for chatting", "");
    Channel channel2("random", "Random channel for chatting", "");
    Channel channel3("private", "Private channel for chatting", "");
    
    server.add_channel(channel);
    server.add_channel(channel2);
    server.add_channel(channel3);

    while (true)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        int maxfd = listen_fd;

        maxfd = server.assign_read_mode(listen_fd, readfds);
        if (wait_for_activity(maxfd, readfds) == -1)
        break;

        // new connection?
        if (FD_ISSET(listen_fd, &readfds))
        {
            if (server.accept_new_client() < 0)
                std::cerr << ERROR << "Error accepting new client" << std::endl;
        }
        if (to_shutdown)
            break;

        std::vector<Client> &clients = server.get_clients();
        for (size_t j = 0; j < clients.size(); ++j)
        {
            Client &client = clients[j];
            if (FD_ISSET(client.getFd(), &readfds))
            {
                Message msg = server.recv(client);
                if (!msg.isValid())
                {
                    std::cout << INFO << "Peer disconnected: " << client.getNickname() << " fd=" << client.getFd() << std::endl;
                    close(client.getSd());
                    server.remove_client(client.getSd());
                    break;
                }
                if (msg.getContent() == SERVER_SHUTDOWN)
                {
                    std::cout << "\n\n" << server << "\n\n";
                    break;
                }
                std::cout << INFO << "Received message from " << client.getNickname() << ": " << msg.getContent() << std::endl;
                if (!client.isAuthenticated())
                {
                    try {
                        server.halloy_support(client, msg); // CHANGED: pass the real client reference
                    } catch (const std::exception &e) {
                        std::cerr << ERROR << "Error processing initial message: " << e.what() << std::endl;
                    }
                }
                else
                {
                    try {
                        parse_message(server, msg);
                    } catch (const std::exception &e) {
                        std::cerr << ERROR << "Error processing message: " << e.what() << std::endl;
                    }
                }
            }
        }
    }
    server.disconnect();
    for (int i = 0; i < (int)server.get_channels().size(); i++)
    {
        std::cout << server.get_channels()[i] << std::endl;
    }
    return (EXIT_SUCCESS);
}
