#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <winsock2.h>
#include <ctime>  
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>


#pragma comment(lib, "ws2_32.lib")

const int PORT = 8080;


int globalID = 1;
std::map<int, std::string> clientNicknames;

struct Message {
    int senderID;
    int targetID;
    char nickname[50];
    char content[512];
    char time[50];  
};

struct ClientInfo {
    SOCKET clientSocket;
    int clientID;
};
std::vector<ClientInfo> clients;

void logMessage(const std::string& message) {
    std::time_t now = std::time(nullptr);
    char date[11];
    std::strftime(date, sizeof(date), "%Y-%m-%d", std::localtime(&now));
    
    std::string filename = std::string(date) + "_log.txt";
    std::ofstream logFile(filename, std::ios::app); 
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "Failed to open log file." << std::endl;
    }
}


void handleClient(SOCKET clientSocket, int clientID) {
    while (true) {
        Message msg;
        int bytesRead = recv(clientSocket, (char*)&msg, sizeof(msg), 0);
        if (bytesRead <= 0) {
            std::cout << "Client disconnected." << std::endl;
            closesocket(clientSocket);
            clients.erase(std::remove_if(clients.begin(), clients.end(),
                [clientID](const ClientInfo& c) { return c.clientID == clientID; }),
                clients.end());
            return;
        }

        std::cout << "["<< msg.targetID <<" - "<<msg.time<<" - " << msg.senderID << " - " << msg.nickname << "]: " << msg.content << std::endl;
        std::stringstream logStream;
        logStream << "[" << msg.targetID <<" - "<< msg.time << " - " << msg.senderID << " - " << msg.nickname << "]: " << msg.content;
        logMessage(logStream.str());

        // Check if the message is a private message
        if (msg.targetID != -1) {
            for (auto& client : clients) {
                std::cout<<client.clientID<<std::endl;
                if (client.clientID == msg.targetID) { // If the client is the target
                    send(client.clientSocket, (char*)&msg, sizeof(msg), 0);
                }
            }
        } else { // Broadcast the message
            for (auto& client : clients) {
                
                if (client.clientID != msg.senderID) {
                    send(client.clientSocket, (char*)&msg, sizeof(msg), 0);
                }
            }
        }
    }
}



int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    std::cout << "Server started and listening on port " << PORT << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        int clientID = globalID++;
        clients.push_back({ clientSocket, clientID });
        char buffer[1024];
        recv(clientSocket, buffer, sizeof(buffer), 0);  // Receive Nickname
        std::cout<<buffer<<std::endl;
        send(clientSocket, (char*)&clientID, sizeof(clientID), 0);
        clientNicknames[clientID] = buffer;
        std::thread(handleClient, clientSocket, std::ref(clientID)).detach();

    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
