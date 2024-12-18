#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib") 

#define PORT 58901
#define BOARD_SIZE 9

char board[BOARD_SIZE]; 
int currentPlayer = 0;  
SOCKET playerSockets[2]; 

void initializeBoard();
void printBoard();
int hasWinner();
int isBoardFull();
void sendToPlayer(SOCKET playerSocket, const char* message);
void broadcastMessage(const char* message);

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char buffer[1024];

    
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }

  
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    
    if (listen(serverSocket, 2) == SOCKET_ERROR) {
        printf("Listen failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server started on port %d. Waiting for players...\n", PORT);
    initializeBoard();

    
    for (int i = 0; i < 2; i++) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            printf("Accept failed\n");
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
        playerSockets[i] = clientSocket;
        printf("Player %d connected.\n", i + 1);
    }

    printf("Both players connected. Game starting!\n");

   
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        sprintf(buffer, "Your move (player %d):", currentPlayer + 1);
        sendToPlayer(playerSockets[currentPlayer], buffer);

        
        int bytesReceived = recv(playerSockets[currentPlayer], buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            printf("Player %d disconnected. Game over.\n", currentPlayer + 1);
            break;
        }

        int move = atoi(buffer);
        if (move >= 0 && move < BOARD_SIZE && board[move] == ' ') {
            board[move] = (currentPlayer == 0) ? 'X' : 'O';
            printBoard();

            if (hasWinner()) {
                broadcastMessage("Game over: Player won!");
                break;
            } else if (isBoardFull()) {
                broadcastMessage("Game over: It's a tie!");
                break;
            }

            currentPlayer = 1 - currentPlayer; 
        } else {
            sendToPlayer(playerSockets[currentPlayer], "Invalid move. Try again.");
        }
    }

    
    for (int i = 0; i < 2; i++) {
        closesocket(playerSockets[i]);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

void initializeBoard() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        board[i] = ' ';
    }
}

void printBoard() {
    printf("\nCurrent Board:\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf(" %c ", board[i]);
        if (i % 3 != 2) printf("|");
        else if (i < 6) printf("\n---+---+---\n");
    }
    printf("\n");
}

int hasWinner() {
    int winCombos[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
        {0, 4, 8}, {2, 4, 6}
    };
    for (int i = 0; i < 8; i++) {
        if (board[winCombos[i][0]] != ' ' &&
            board[winCombos[i][0]] == board[winCombos[i][1]] &&
            board[winCombos[i][1]] == board[winCombos[i][2]]) {
            return 1;
        }
    }
    return 0;
}

int isBoardFull() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board[i] == ' ') return 0;
    }
    return 1;
}

void sendToPlayer(SOCKET playerSocket, const char* message) {
    send(playerSocket, message, strlen(message), 0);
}

void broadcastMessage(const char* message) {
    for (int i = 0; i < 2; i++) {
        sendToPlayer(playerSockets[i], message);
    }
}

