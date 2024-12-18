#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 58901
#define SERVER_IP "127.0.0.1"

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

   
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

  
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Connection failed\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    printf("Connected to the Tic Tac Toe server.\n");

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            printf("Connection closed by server.\n");
            break;
        }
        printf("%s\n", buffer);

        if (strstr(buffer, "Your move")) {
            printf("Enter your move (0-8): ");
            fgets(buffer, sizeof(buffer), stdin);
            send(clientSocket, buffer, strlen(buffer), 0);
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

