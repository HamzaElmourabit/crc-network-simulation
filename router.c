#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define PORT_CLIENT 5000
#define PORT_SERVER 6000
#define MAX 256

int main() {
    WSADATA wsa;
    SOCKET sock_client, sock_server, client_fd;
    struct sockaddr_in addr_client, addr_server, cli;
    int len = sizeof(cli);
    char buff[MAX];
    fd_set readfds;
    struct timeval tv;

    // Initialiser Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    // Socket pour recevoir du client
    sock_client = socket(AF_INET, SOCK_STREAM, 0);
    addr_client.sin_family = AF_INET;
    addr_client.sin_addr.s_addr = INADDR_ANY;
    addr_client.sin_port = htons(PORT_CLIENT);

    bind(sock_client, (struct sockaddr*)&addr_client, sizeof(addr_client));
    listen(sock_client, 3);
    printf("Router listening on port %d (from client)\n", PORT_CLIENT);

    // Socket pour envoyer au serveur
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr_server.sin_port = htons(PORT_SERVER);

    // Essayer de se connecter au serveur
    while (connect(sock_server, (struct sockaddr*)&addr_server, sizeof(addr_server)) == SOCKET_ERROR) {
        Sleep(100);  // Attendre 100ms
    }
    printf("Router connected to server on port %d\n", PORT_SERVER);

    client_fd = accept(sock_client, (struct sockaddr*)&cli, &len);
    printf("Client connected to router\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(client_fd, &readfds);
        FD_SET(sock_server, &readfds);
        
        // Timeout pour éviter le blocage
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int maxfd = (client_fd > sock_server) ? client_fd : sock_server;
        
        if (select(maxfd + 1, &readfds, NULL, NULL, &tv) == SOCKET_ERROR) {
            printf("Select error: %d\n", WSAGetLastError());
            break;
        }

        // Client → Serveur
        if (FD_ISSET(client_fd, &readfds)) {
            int n = recv(client_fd, buff, MAX-1, 0);
            if (n <= 0) break;
            buff[n] = '\0';

            // OPTIONAL: flip one bit to test CRC error (décommenter pour tester)
            // if (strlen(buff) > 30) buff[30] = (buff[30]=='0')?'1':'0';

            printf("Router forwarding → Server: %s\n", buff);
            send(sock_server, buff, strlen(buff), 0);
        }

        // Serveur → Client
        if (FD_ISSET(sock_server, &readfds)) {
            int n = recv(sock_server, buff, MAX-1, 0);
            if (n <= 0) break;
            buff[n] = '\0';
            printf("Router forwarding → Client: %s\n", buff);
            send(client_fd, buff, strlen(buff), 0);
        }
    }

    closesocket(client_fd);
    closesocket(sock_server);
    closesocket(sock_client);
    WSACleanup();
    printf("Router shutdown\n");
    return 0;
}