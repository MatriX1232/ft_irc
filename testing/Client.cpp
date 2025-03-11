#include "Client.hpp"

Client::Client(std::string ip, int port) : _port(port), _ip(ip)
{
    std::cout << "Created new CLIENT" << std::endl;
}

Client::~Client()
{
    disconnect();
    std::cout << "Destroyed CLIENT" << std::endl;
}

int Client::try_connect()
{
    struct hostent* host = gethostbyname(this->_ip.c_str());
    if(host == NULL)
    {
        std::cout << "Error: Host not found" << std::endl;
        return -1;
    } 
    sockaddr_in sendSockAddr;   
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(this->_port);
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);

    int status = connect(clientSd, (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        std::cout<<"Error connecting to socket!"<<std::endl;
        return -1;
    }
    this->_clientSd = clientSd; // Assign the socket descriptor to the member variable
    std::cout << "Connected to the server!" << std::endl;
    return 0;
}

int Client::disconnect()
{
    close(this->_clientSd);
    std::cout << "Disconnected from the server!" << std::endl;
    return 0;
}

int Client::send(std::string msg)
{
    char _msg[150];
    bzero(_msg, 150);
    strcpy(_msg, msg.c_str());
    int status = ::send(this->_clientSd, (char*)&_msg, 150, 0);
    if(status < 0)
    {
        std::cout << "Error sending message" << std::endl;
        return -1;
    }
    std::cout << "Message sent..." << std::endl;
    return 0;
}

std::string Client::recv()
{
    bzero(this->_msg, 150);
    long status = ::recv(this->_clientSd, (char*)this->_msg, sizeof(this->_msg), 0);
    if(status < 0)
    {
        std::cout << "Error receiving message" << std::endl;
        return NULL;
    }
    std::cout << "Message received: " << this->_msg << std::endl;
    return this->_msg;
}

int Client::send_file(std::string filename)
{
    this->send(filename);
    std::ifstream file;
    file.open(filename.c_str(), std::ios::in);
    if(!file)
    {
        std::cout << "Error opening file" << std::endl;
        return -1;
    }
    char buffer[256];
    bzero(buffer, 256);
    int nread = 0;
    while((nread = file.readsome(buffer, 256)) > 0)
    {
        if(::send(this->_clientSd, buffer, nread, 0) < 0)
        {
            std::cout << "Error sending file" << std::endl;
            return -1;
        }
        bzero(buffer, 256);
    }
    file.close();
    std::cout << "File sent successfully" << std::endl;
    return 0;
}