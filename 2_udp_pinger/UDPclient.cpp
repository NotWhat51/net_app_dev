#include <iostream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

int main()
{
    const char *serverName = "127.0.0.1";
    const int serverPort = 55213;

    // Создание UDP-сокета
    int clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == -1)
    {
        std::cerr << "Не удалось создать сокет" << std::endl;
        return 1;
    }

    // Установка адреса сервера
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(serverName);
    serverAddress.sin_port = htons(serverPort);

    for (int i = 0; i < 10; ++i)
    {
        // Установка таймаута на ожидание ответа
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        std::ostringstream oss;
        oss << "Ping " << (i + 1) << " to server " << serverName << ":" << serverPort;
        std::string message = oss.str();

        // Отправка сообщения серверу
        sendto(clientSocket, message.c_str(), message.length(), 0, (sockaddr *)&serverAddress, sizeof(serverAddress));

        // Получение ответа от сервера
        char modifiedMessage[2048];
        memset(modifiedMessage, 0, sizeof(modifiedMessage));
        sockaddr_in serverResponseAddress;
        socklen_t serverResponseAddressLen = sizeof(serverResponseAddress);
        ssize_t recvLen = recvfrom(clientSocket, modifiedMessage, sizeof(modifiedMessage), 0, (sockaddr *)&serverResponseAddress, &serverResponseAddressLen);

        if (recvLen == -1)
        {
            std::cout << "! Превышен таймаут !" << std::endl;
        }
        else
        {
            // Вычисление времени обработки запроса
            timespec startTime, endTime;
            clock_gettime(CLOCK_MONOTONIC, &startTime);
            clock_gettime(CLOCK_MONOTONIC, &endTime);
            double elapsedSeconds = static_cast<double>(endTime.tv_sec - startTime.tv_sec) +
                                    static_cast<double>(endTime.tv_nsec - startTime.tv_nsec) / 1000000000;

            std::cout << "Время обработки запроса: " << elapsedSeconds << " секунд" << std::endl;
            std::cout << modifiedMessage << std::endl;
        }
    }

    close(clientSocket);
    return 0;
}
