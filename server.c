#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define MAX 256
#define PORT 6000

#define FLAG "00000000"
#define SRC  "S000000S"
#define DST  "D000000D"

// Même fonction CRC que avant
int checkCRC(const char* msgWithCrc) {
    char poly[] = "10011";
    int n = strlen(poly);
    char temp[200];
    strcpy(temp, msgWithCrc);

    for (int i = 0; i < strlen(temp) - n + 1;) {
        if (temp[i] == '1')
            for (int j = 0; j < n; j++)
                temp[i+j] = (temp[i+j] == poly[j]) ? '0' : '1';
        i++;
    }
    for (int i = strlen(temp)-4; i < strlen(temp); i++)
        if (temp[i] != '0') return 0;
    return 1;
}

void func(SOCKET connfd) {
    char buff[MAX];
    int n;

    for (;;) {
        memset(buff, 0, MAX);
        int recvd = recv(connfd, buff, sizeof(buff)-1, 0);
        if (recvd <= 0) break;
        buff[recvd] = '\0';

        // Extraction du payload + CRC
        int total_len = strlen(buff);
        if (total_len < 24 + 4 + 8) {
            printf("Frame too short!\n");
            continue;
        }

        int payload_len = total_len - 24 - 4 - 8;
        char msgWithCrc[200] = "";
        strncpy(msgWithCrc, buff + 24, payload_len);
        strncpy(msgWithCrc + payload_len, buff + 24 + payload_len, 4);
        msgWithCrc[payload_len + 4] = '\0';

        printf("Received frame: %s\n", buff);

        if (checkCRC(msgWithCrc)) {
            printf("From client: %.*s\n", payload_len, buff + 24);
        } else {
            printf("CRC ERROR! Message corrupted.\n");
        }

        // Envoyer réponse
        printf("Enter binary msg: ");
        fgets(buff, MAX, stdin);
        buff[strcspn(buff, "\n")] = 0;

        send(connfd, buff, strlen(buff), 0);

        if (strncmp(buff, "exit", 4) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
}

int main() {
    WSADATA wsa;
    SOCKET sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    int len = sizeof(cli);

    // Initialiser Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket created\n");

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("Bind success\n");

    if (listen(sockfd, 5) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("Server listening on port %d...\n", PORT);

    connfd = accept(sockfd, (struct sockaddr*)&cli, &len);
    if (connfd == INVALID_SOCKET) {
        printf("Accept failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("Client connected\n");

    func(connfd);

    closesocket(connfd);
    closesocket(sockfd);
    WSACleanup();
    return 0;
}