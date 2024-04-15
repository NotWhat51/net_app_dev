#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SERVER_ADDR "77.88.21.158" //"smtp.yandex.ru"
#define SERVER_PORT 465
#define BUFFER_SIZE 1024

std::string base64_encode(const std::string &data)
{
    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string result;
    result.reserve((data.size() + 2) / 3 * 4);

    for (size_t i = 0; i < data.size(); i += 3)
    {
        uint32_t bytes =
            (static_cast<uint8_t>(data[i]) << 16) |
            (i + 1 < data.size() ? static_cast<uint8_t>(data[i + 1]) << 8 : 0) |
            (i + 2 < data.size() ? static_cast<uint8_t>(data[i + 2]) : 0);

        result.append(1, base64_chars[(bytes >> 18) & 0x3F]);
        result.append(1, base64_chars[(bytes >> 12) & 0x3F]);
        result.append(1, base64_chars[(bytes >> 6) & 0x3F]);
        result.append(1, base64_chars[bytes & 0x3F]);
    }

    while (result.size() % 4 != 0)
    {
        result.append(1, '=');
    }

    return result;
}

int main()
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);

    if (!ctx)
    {
        std::cerr << "Не удалось создать SSL-контекст" << std::endl;
        return -1;
    }

    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE] = {0};

    // Создание сокета
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return -1;
    }

    // Инициализация адреса сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr) <= 0)
    {
        std::cerr << "Неверный адрес/ Адрес не поддерживается" << std::endl;
        return -1;
    }

    // Подключение к серверу
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Подключение не удалось" << std::endl;
        return -1;
    }

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, clientSocket);

    if (SSL_connect(ssl) <= 0)
    {
        std::cerr << "Ошибка SSL-соединения" << std::endl;
        ERR_print_errors_fp(stderr);
        return -1;
    }

    // Получение сообщения после запроса на подключение
    int bytesReceived = SSL_read(ssl, buffer, BUFFER_SIZE - 1);
    if (bytesReceived <= 0)
    {
        std::cerr << "Ошибка при получении сообщения от сервера" << std::endl;
        return -1;
    }
    buffer[bytesReceived] = '\0';
    std::cout << "Сообщение после запроса на подключение: " << buffer << std::endl;

    // Команда HELO
    std::string heloCommand = "HELO Alice\r\n";
    SSL_write(ssl, heloCommand.c_str(), heloCommand.length());
    memset(buffer, 0, BUFFER_SIZE);
    bytesReceived = SSL_read(ssl, buffer, BUFFER_SIZE - 1);
    if (bytesReceived <= 0)
    {
        std::cerr << "Ошибка при получении сообщения от сервера" << std::endl;
        return -1;
    }
    buffer[bytesReceived] = '\0';
    std::cout << buffer;

    // Команда AUTH PLAIN
    std::string username = "belskaya.se@edu.spbstu.ru";
    std::string password = "qwerty"; // пример пароля, не надо его использовать
    std::string authMsg = "AUTH PLAIN " + base64_encode("\0" + username + "\0" + password) + "\r\n";
    send(clientSocket, authMsg.c_str(), authMsg.length(), 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(clientSocket, buffer, BUFFER_SIZE, 0);
    std::cout << buffer << std::endl;

    // Команда MAIL FROM
    std::string mailFrom = "MAIL FROM: <belskaya.se@edu.spbstu.ru>\r\n";
    send(clientSocket, mailFrom.c_str(), mailFrom.length(), 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(clientSocket, buffer, BUFFER_SIZE, 0);
    std::cout << "После команды MAIL FROM: " << buffer << std::endl;

    // Команда RCPT TO
    std::string rcptTo = "RCPT TO: <sonbe51@mail.ru>\r\n";
    send(clientSocket, rcptTo.c_str(), rcptTo.length(), 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(clientSocket, buffer, BUFFER_SIZE, 0);
    std::cout << "После команды RCPT TO: " << buffer << std::endl;

    // Команда DATA
    std::string data = "DATA\r\n";
    send(clientSocket, data.c_str(), data.length(), 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(clientSocket, buffer, BUFFER_SIZE, 0);
    std::cout << "После команды DATA: " << buffer << std::endl;

    // Отправка данных сообщения
    std::string subject = "Subject: Тестирование SMTP-клиента\r\n\r\n";
    send(clientSocket, subject.c_str(), subject.length(), 0);
    std::string message = "Hello, world!";
    send(clientSocket, message.c_str(), message.length(), 0);
    send(clientSocket, "\r\n.\r\n", 5, 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(clientSocket, buffer, BUFFER_SIZE, 0);
    std::cout << "Ответ после отправки тела сообщения: " << buffer << std::endl;

    // Команда QUIT
    std::string quit = "QUIT\r\n";
    SSL_write(ssl, quit.c_str(), quit.length());
    memset(buffer, 0, BUFFER_SIZE);
    bytesReceived = SSL_read(ssl, buffer, BUFFER_SIZE - 1);
    if (bytesReceived <= 0)
    {
        std::cerr << "Ошибка при получении сообщения от сервера" << std::endl;
        return -1;
    }
    buffer[bytesReceived] = '\0';
    std::cout << "Сообщение: " << buffer << std::endl;

    SSL_free(ssl);
    close(clientSocket);
    SSL_CTX_free(ctx);
    EVP_cleanup();

    return 0;
}
