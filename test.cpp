#include <iostream>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

#define PORT 25565
#define BUFFER_SIZE 4096

// Helper function to log messages with timestamp
void logMessage(const std::string& prefix, const std::string& message) {
    time_t now = time(0);
    char timeBuffer[26];
    struct tm* tm_info = localtime(&now);
    strftime(timeBuffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    
    std::cout << "[" << timeBuffer << "] " << prefix << ": ";
    
    // Print message, handling non-printable characters
    for (size_t i = 0; i < message.length(); ++i) {
        if (message[i] == '\r')
            std::cout << "\\r";
        else if (message[i] == '\n')
            std::cout << "\\n";
        else
            std::cout << message[i];
    }
    std::cout << std::endl;
}

// Send a simple response to keep the connection alive
void sendResponse(int clientFd, const std::string& response) {
    send(clientFd, response.c_str(), response.length(), 0);
    logMessage("SENT", response);
}

int main() {
    int serverFd, clientFd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    std::vector<pollfd> fds;
    
    // Create socket
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }
    
    // Set socket options
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Setsockopt failed" << std::endl;
        return 1;
    }
    
    // Set non-blocking
    if (fcntl(serverFd, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "Failed to set non-blocking" << std::endl;
        return 1;
    }
    
    // Bind socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }
    
    // Listen
    if (listen(serverFd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }
    
    std::cout << "IRC Message Logger started on port " << PORT << std::endl;
    std::cout << "Waiting for Halloy client connection..." << std::endl;
    
    // Set up polling
    pollfd serverPollFd;
    serverPollFd.fd = serverFd;
    serverPollFd.events = POLLIN;
    fds.push_back(serverPollFd);
    
    // Main loop
    while (true) {
        int pollResult = poll(fds.data(), fds.size(), -1);
        
        if (pollResult < 0) {
            std::cerr << "Poll failed" << std::endl;
            break;
        }
        
        // Check for new connections
        if (fds[0].revents & POLLIN) {
            if ((clientFd = accept(serverFd, (struct sockaddr*)&address, &addrlen)) < 0) {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }
            
            // Set client socket to non-blocking
            if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0) {
                std::cerr << "Failed to set client socket non-blocking" << std::endl;
                close(clientFd);
                continue;
            }
            
            // Add client to poll list
            pollfd clientPollFd;
            clientPollFd.fd = clientFd;
            clientPollFd.events = POLLIN;
            fds.push_back(clientPollFd);
            
            std::cout << "New connection from " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << std::endl;
        }
        
        // Check for client messages
        for (size_t i = 1; i < fds.size(); i++) {
            if (fds[i].revents & POLLIN) {
                ssize_t bytesRead = recv(fds[i].fd, buffer, BUFFER_SIZE - 1, 0);
                
                if (bytesRead <= 0) {
                    if (bytesRead == 0) {
                        std::cout << "Client disconnected" << std::endl;
                    } else {
                        std::cerr << "Recv failed" << std::endl;
                    }
                    
                    close(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    i--;
                    continue;
                }
                
                buffer[bytesRead] = '\0';
                std::string message(buffer, bytesRead);
                logMessage("RECV", message);
                
                // Handle basic IRC commands to keep the connection alive
                if (message.find("CAP LS") != std::string::npos) {
                    sendResponse(fds[i].fd, "CAP * LS :\r\n");
                }
                else if (message.find("NICK ") != std::string::npos) {
                    // Extract nickname
                    size_t pos = message.find("NICK ") + 5;
                    size_t end = message.find("\r\n", pos);
                    std::string nick = message.substr(pos, end - pos);
                    
                    // Welcome the user after NICK and USER are received
                    if (message.find("USER ") != std::string::npos) {
                        sendResponse(fds[i].fd, ":localhost 001 " + nick + " :Welcome to the IRC Logger\r\n");
                    }
                }
                else if (message.find("PING") != std::string::npos) {
                    // Extract ping parameter
                    size_t pos = message.find("PING") + 5;
                    size_t end = message.find("\r\n", pos);
                    std::string param = message.substr(pos, end - pos);
                    
                    sendResponse(fds[i].fd, "PONG " + param + "\r\n");
                }
            }
        }
    }
    
    // Clean up
    for (size_t i = 0; i < fds.size(); i++) {
        close(fds[i].fd);
    }
    
    return 0;
}