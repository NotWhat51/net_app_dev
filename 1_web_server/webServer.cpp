#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <fstream>

const int PORT = 8081;
const std::string CRLF = "\r\n";

void send_bytes(std::ifstream &fis, int client_socket);
std::string content_type(const std::string &file_name);

void process_request(int client_socket)
{
    // Получаем ссылки на входной и выходной потоки сокета.
    char buffer[1024];
    int bytes_received;
    std::string request_line;

    // Извлекаем имя файла из строки запроса.
    std::istringstream iss(request_line);
    std::string method;
    std::string file_name;
    iss >> method >> file_name;

    // Добавляем "." к имени для указания на текущий каталог.
    file_name = "." + file_name;

    // Открываем запрошенный файл.
    std::ifstream fis;
    bool file_exists = false;
    fis.open(file_name, std::ios::in | std::ios::binary);
    if (fis.is_open())
    {
        file_exists = true;
    }

    // Отладочная информация для внутреннего пользования
    std::cout << "Incoming!!!" << std::endl;
    std::cout << request_line << std::endl;
    std::string header_line;

    header_line = std::string(buffer, bytes_received);
    std::cout << header_line << std::endl;

    // Создаем ответное сообщение.
    std::string status_line;
    std::string content_type_line;
    if (file_exists)
    {
        status_line = "HTTP/1.0 200 OK" + CRLF;
        content_type_line = "Content-Type: " + content_type(file_name) + CRLF;
        std::cout << "200 OK" << std::endl;
        std::cout << std::endl;
    }
    else
    {
        status_line = "HTTP/1.0 404 Not Found" + CRLF;
        content_type_line = "Content-Type: text/html" + CRLF;
        std::cout << "404 Not Found" << std::endl;
        std::cout << std::endl;
    }

    // Отправляем строку состояния.
    send(client_socket, status_line.c_str(), status_line.length(), 0);

    // Отправляем строку типа содержимого.
    send(client_socket, content_type_line.c_str(), content_type_line.length(), 0);

    // Отправляем пустую строку для указания конца заголовков.
    send(client_socket, CRLF.c_str(), CRLF.length(), 0);

    // Отправляем тело объекта.
    if (file_exists)
    {
        send_bytes(fis, client_socket);
        fis.close();
    };

    // Закрываем сокет.
    close(client_socket);
}

void send_bytes(std::ifstream &fis, int client_socket)
{
    char buffer[1024];
    int bytes_read;

    while ((bytes_read = fis.read(buffer, sizeof(buffer)).gcount()) > 0)
    {
        send(client_socket, buffer, bytes_read, 0);
    }
}

std::string content_type(const std::string &file_name)
{
    std::string extension = file_name.substr(file_name.find_last_of(".") + 1);

    if (extension == "html" || extension == "htm")
    {
        return "text/html";
    }
    else if (extension == "jpg" || extension == "jpeg")
    {
        return "image/jpeg";
    }
    else if (extension == "png")
    {
        return "image/png";
    }
    else if (extension == "gif")
    {
        return "image/gif";
    }
    else
    {
        return "image/jpeg";
    }
}

int main(int argc, char *argv[])
{
    // Создаем сокет
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Устанавливаем параметры сокета
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Начинаем слушать входящие запросы
    listen(server_socket, 10);

    std::cout << "Server started on port " << PORT << std::endl;

    // Обрабатываем HTTP-запросы в бесконечном цикле
    while (true)
    {
        // Слушаем входящие запросы по TCP
        int client_socket = accept(server_socket, NULL, NULL);

        // Создаем новый поток для обработки запроса
        std::thread thread(process_request, client_socket);

        // Отделяем поток от текущего потока выполнения
        thread.detach();
    }

    // Закрываем сокет
    close(server_socket);

    return 0;
}