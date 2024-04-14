#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    // Создание UDP-сокета
    int serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == -1) {
        std::cerr << "Не удалось создать сокет" << std::endl;
        return 1;
    }

    // Присваивание IP-адреса и порта
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(55213);

    // Привязка сокета
    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Не удалось привязать сокет" << std::endl;
        close(serverSocket);
        return 1;
    }

    while (true) {
        // Генерация случайного числа в диапазоне от 0 до 10
        int randNum = rand() % 11;

        // Получение пакета от клиента вместе с адресом, с которого он пришел
        sockaddr_in clientAddress;
        socklen_t clientAddressLen = sizeof(clientAddress);
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        ssize_t recvLen = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddress, &clientAddressLen);
        if (recvLen == -1) {
            std::cerr << "Не удалось получить данные" << std::endl;
            continue;
        }

        std::cout << "Адрес клиента: " << inet_ntoa(clientAddress.sin_addr) << std::endl;

        // Преобразование сообщения от клиента в верхний регистр
        for (int i = 0; buffer[i]; ++i) {
            buffer[i] = toupper(buffer[i]);
        }

        // Если rand меньше 4, считаем пакет потерянным и не отвечаем
        if (randNum < 4) {
            continue;
        }

        // В противном случае сервер отвечает
        if (sendto(serverSocket, buffer, strlen(buffer), 0, (sockaddr*)&clientAddress, sizeof(clientAddress)) == -1) {
            std::cerr << "Не удалось отправить данные" << std::endl;
        }
    }

    close(serverSocket);
    return 0;
}
