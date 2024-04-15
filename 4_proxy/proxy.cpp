#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define MAX_PENDING 5
#define MAX_LINE 1024
int main()
{
    int tcpSerPort = 8080;
    int tcpSerSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcpSerSock == -1)
    {
        std::cerr << "Ошибка создания сокета: " << strerror(errno) << std::endl;
        return 1;
    }
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(tcpSerPort);
    if (bind(tcpSerSock, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << "Ошибка привязки сокета: " << strerror(errno) << std::endl;
        close(tcpSerSock);
        return 1;
    }
    if (listen(tcpSerSock, MAX_PENDING) == -1)
    {
        std::cerr << "Ошибка прослушивания сокета: " << strerror(errno) << std::endl;
        close(tcpSerSock);
        return 1;
    }
    while (true)
    {
        std::cout << "Готов к обслуживанию...\n";
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int tcpCliSock = accept(tcpSerSock, (sockaddr *)&clientAddr, &clientAddrLen);
        std::cout << "Получено соединение от: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "\n";
        char message[MAX_LINE];
        memset(message, 0, MAX_LINE);
        recv(tcpCliSock, message, MAX_LINE, 0);
        std::cout << message << std::endl;
        std::istringstream iss(message);
        std::string requestLine;
        std::getline(iss, requestLine);
        std::istringstream requestStream(requestLine);
        std::string method, url, protocol;
        requestStream >> method >> url >> protocol;
        std::cout << url << std::endl;
        std::string filename;
        if (url.find("http://") != std::string::npos)
        {
            filename = url.substr(url.find("http://") + 7);
        }
        else
        {
            filename = url;
        }
        bool fileExist = false;
        std::string filetouse = "/" + filename;
        close(tcpCliSock);
    }
    close(tcpSerSock);
    return 0;
}
