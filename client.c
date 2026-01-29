#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define MAX 256
#define PORT 5000

#define FLAG "00000000"
#define SRC  "S000000S"
#define DST  "D000000D"

// Compute CRC-4 et retourne string 4-bit
char* computeCRC(const char* data) {
    char poly[] = "10011";
    int n = strlen(poly);
    char temp[200] = "";
    strcpy(temp, data);
    for (int i = 0; i < n-1; i++) temp[strlen(temp)] = '0';

    for (int i = 0; i < strlen(temp) - n + 1;) {
        if (temp[i] == '1')
            for (int j = 0; j < n; j++)
                temp[i+j] = (temp[i+j] == poly[j]) ? '0' : '1';
        i++;
    }
    char* crc = malloc(5);
    strncpy(crc, temp + strlen(temp) - 4, 4);
    crc[4] = '\0';
    return crc;
}

void func(SOCKET sockfd) {
    char buff[MAX];
    char frame[200];

    for (;;) {
        printf("Enter binary msg: ");
        fgets(buff, MAX, stdin);
        buff[strcspn(buff, "\n")] = 0;
        if (strlen(buff) == 0) continue;

        // Construire frame
        strcpy(frame, FLAG SRC DST);
        strcat(frame, buff);
        strcat(frame, computeCRC(buff));
        strcat(frame, FLAG);

        send(sockfd, frame, strlen(frame), 0);

        if (strncmp(buff, "exit", 4) == 0) {
            printf("Client Exit...\n");
            break;
        }

        memset(buff, 0, MAX);
        int bytes = recv(sockfd, buff, MAX-1, 0);
        if (bytes > 0) {
            buff[bytes] = '\0';
            printf("From Server: %s\n", buff);
        }
    }
}

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in servaddr;

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
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("Connection failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }
    printf("Connected to server\n");

    func(sockfd);

    closesocket(sockfd);
    WSACleanup();
    return 0;
}