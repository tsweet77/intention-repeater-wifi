/*
Intention Repeater WiFi v0.3
Created by Anthro Teacher and WebGPT.
To compile: g++ -O3 -static -Wall Intention_Repeater_WiFi.cpp -o Intention_Repeater_WiFi.exe -lws2_32
*/

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cmath>
#include <winsock2.h>
#include <Ws2tcpip.h>

using namespace std;

SOCKET globalSock = INVALID_SOCKET;
volatile bool running = true;

void signalHandler(int signum) {
    cout << "\nShutting down..." << endl;
    running = false;
    exit(signum);
}

void broadcastMessage(const SOCKET& sock, const string& message, struct sockaddr_in& broadcastAddr) {
    if (sendto(sock, message.c_str(), static_cast<int>(message.length()), 0, 
               (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) == SOCKET_ERROR) {
        std::cerr << "Error in sending broadcast message with error: " << WSAGetLastError() << std::endl;
    }
}

string formatNumber(long long count) {
    if (count < 1000) {
        return to_string(count);
    } else {
        const char* suffixes[] = {"k", "M", "B", "T", "Q"};
        size_t suffixIndex = static_cast<size_t>(log10(count) / 3) - 1;
        double formattedNumber = count / pow(1000.0, suffixIndex + 1);
        
        char buffer[20];
        sprintf(buffer, "%.3f%s", formattedNumber, suffixes[suffixIndex]);
        return string(buffer);
    }
}

string formatFreq(long long count) {
    if (count < 1000) {
        return to_string(count) + "Hz";
    } else {
        const char* suffixes[] = {"kHz", "MHz", "GHz", "THz", "EHz"};
        size_t suffixIndex = static_cast<size_t>(log10(count) / 3) - 1;
        double formattedNumber = count / pow(1000.0, suffixIndex + 1);
        
        char buffer[20];
        sprintf(buffer, "%.3f%s", formattedNumber, suffixes[suffixIndex]);
        return string(buffer);
    }
}

int main() {
    signal(SIGINT, signalHandler);

    cout << "Intention Repeater WiFi v0.3" << endl;
    cout << "by Anthro Teacher and WebGPT and Claude 3 Opus" << endl << endl;
    cout << "Enter your Intention: ";
    string intention;
    getline(cin, intention);

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != NO_ERROR) {
        cerr << "WSAStartup failed with error: " << result << endl;
        return 1;
    }

    globalSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (globalSock == INVALID_SOCKET) {
        cerr << "Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    const int broadcast = 1;
    if (setsockopt(globalSock, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof(broadcast)) == SOCKET_ERROR) {
        cerr << "Error in setting Broadcast option with error: " << WSAGetLastError() << endl;
        closesocket(globalSock);
        WSACleanup();
        return 1;
    }

    struct sockaddr_in broadcastAddr;
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    broadcastAddr.sin_port = htons(11111);

    long long count = 0, freq = 0;
    auto lastUpdate = chrono::steady_clock::now();

    while (running) {
        broadcastMessage(globalSock, intention, broadcastAddr);
        ++count;
        ++freq;

        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - lastUpdate).count() >= 1000) {
            cout << "\rBroadcasting: " << formatNumber(count) << " Repetitions [" << formatFreq(freq * 10) << "]" << string(5, ' ') << "\r";
            cout.flush();
            lastUpdate = now;
            freq = 0;
        }
    }

    if (globalSock != INVALID_SOCKET) {
        closesocket(globalSock);
        WSACleanup();
    }

    return 0;
}