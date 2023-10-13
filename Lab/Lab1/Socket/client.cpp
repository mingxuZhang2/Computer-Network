#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ctime>    

#pragma comment(lib, "ws2_32.lib")

const int PORT = 8080;
const char* SERVER_IP = "127.0.0.1";

struct Message {
    int senderID;
    int targetID;
    char nickname[50];
    char content[512];
    char time[50];
};


void receiveMessages(SOCKET clientSocket) {
    Message msg;
    while (true) {
        int bytesRead = recv(clientSocket, (char*)&msg, sizeof(msg), 0);
        if (bytesRead <= 0) {
            std::cout << "Disconnected from server." << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            exit(0);
        }
        if(msg.targetID!=-1){
            std::cout<<"This is private message!";
        }
        std::cout << "[" << msg.time<<" - "<<msg.senderID << " - " << msg.nickname << "]: " << msg.content << std::endl;
    }
}


int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);

    connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    

    char nickname[550];
    std::cout << "Enter your nickname: ";
    std::cin.getline(nickname, sizeof(nickname));
    send(clientSocket, nickname, strlen(nickname) + 1, 0);

    int clientID;
    recv(clientSocket, (char*)&clientID, sizeof(clientID), 0);
    std::cout<<"Your ID is:"<<clientID<<std::endl;
    std::thread(receiveMessages, clientSocket).detach();

    char buffer[1024];
    Message msg;
    msg.senderID = clientID;
    strcpy(msg.nickname, nickname);
    while (true) {
        const char* endline = "Quit";
        const char* func = "private";
        std::cin.getline(msg.content, sizeof(msg.content));
        std::time_t now = std::time(nullptr);
        std::strftime(msg.time, sizeof(msg.time), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
        if (strcmp(msg.content, endline) == 0) {
            std::cout << "Thank you for using! The program will shutdown!" << std::endl;
            break;
        }
        if(strcmp(msg.content,func) == 0){
            std::cout << "You will send a private message! Please enter the clientID: ";
            std::cin >> msg.targetID;
            std::cin.ignore();  // To clear the newline left in the buffer
            std::cout << "Please enter the message: ";
            std::cin.getline(msg.content, sizeof(msg.content));
        } else {
            msg.targetID = -1;  // Broadcast
        }
        send(clientSocket, (char*)&msg, sizeof(msg), 0);
    }


    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
