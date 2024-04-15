#include <iostream>
#include <cstring>
#include <ctime>
#include <cmath>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#define ICMP_ECHO_REQUEST 8

unsigned short checksum(void *b, int len) {
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

double receiveOnePing(int mySocket, int ID, int timeout, const char* destAddr) {
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(mySocket, &readfds);

    if (select(mySocket + 1, &readfds, NULL, NULL, &tv) <= 0) {
        return -1; // Timeout
    }

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    char buffer[1024];
    recvfrom(mySocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &addr_len);

    struct icmphdr* icmp_hdr = (struct icmphdr*)(buffer + 20); // Skip IP header
    if (icmp_hdr->type != ICMP_ECHOREPLY || icmp_hdr->un.echo.id != ID) {
        return -1; // Receive error
    }

    return difftime(time(0), *(time_t*)(buffer + 28));
}

void sendOnePing(int mySocket, const char* destAddr, int ID) {
    struct icmphdr icmp_hdr;
    icmp_hdr.type = ICMP_ECHO_REQUEST;
    icmp_hdr.code = 0;
    icmp_hdr.checksum = 0;
    icmp_hdr.un.echo.id = ID;
    icmp_hdr.un.echo.sequence = 1;

    icmp_hdr.checksum = checksum(&icmp_hdr, sizeof(icmp_hdr));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr(destAddr);

    sendto(mySocket, &icmp_hdr, sizeof(icmp_hdr), 0, (struct sockaddr*)&addr, sizeof(addr));
}

double doOnePing(const char* destAddr, int timeout) {
    int mySocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (mySocket == -1) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        exit(1);
    }   

    int myID = getpid() & 0xFFFF;
    sendOnePing(mySocket, destAddr, myID);
    double delay = receiveOnePing(mySocket, myID, timeout, destAddr);

    close(mySocket);
    return delay;
}

void ping(const char* host, int timeout=1) {
    struct hostent* hostent;

    if ((hostent = gethostbyname(host)) == NULL) {
        std::cerr << "Хост не найден" << std::endl;
        exit(1);
    }
    
    char *  addr = inet_ntoa(*(struct in_addr*)hostent->h_addr_list[0]);

    std::cout << "Ping  " << addr << ":" << std::endl << std::endl;

    while (true) {
        double delay = doOnePing(addr, timeout);
        if (delay >= 0) {
            std::cout << "Reply received in " << delay << " seconds" << std::endl;
        } else {
            std::cout << "Request timed out" << std::endl;
        }
        sleep(1); // one second
    }
}

int main() {
    ping("8.8.8.8");
    return 0;
}
