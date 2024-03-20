/*
Intention Repeater WiFi v0.1
Created by Anthro Teacher and WebGPT.
To compile: g++ -O3 -static -Wall Intention_Repeater_WiFi.cpp -o Intention_Repeater_WiFi.exe -lws2_32
*/

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cmath>
#include <winsock2.h> // Make sure to include the Winsock library
#include <Ws2tcpip.h> // Required for inet_addr()

using namespace std;

// Global variable for socket to be accessible in signal handler
SOCKET globalSock = INVALID_SOCKET;
volatile bool running = true; // Global flag to control program execution

// Signal handler for graceful shutdown
void signalHandler(int signum) {
    cout << "\nShutting down..." << endl;
	running = false; // Signal to the main loop to stop
    exit(signum);
}

void broadcastMessage(const SOCKET& sock, const string& message, struct sockaddr_in& broadcastAddr) {
    if (sendto(sock, message.c_str(), static_cast<int>(message.length()), 0, 
               (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) == SOCKET_ERROR) {
		std::cerr << "Error in sending broadcast message with error: " << WSAGetLastError() << std::endl;
    }
}

string formatNumber(long long count) {
    double number = static_cast<double>(count);  // Convert count to double for formatting
    std::stringstream stream;

    if (count < 1000) {
        // Format numbers less than 1000 to three decimal places
        stream << std::fixed << std::setprecision(3) << number;
        return stream.str();
    } else {
        // Determine the suffix and format accordingly
        const char* suffixes[] = {"k", "M", "B", "T", "Q"};
        size_t suffixIndex = static_cast<size_t>(std::floor(std::log10(count) / 3)) - 1; // Determine the index based on the number's magnitude
        double formattedNumber = number / std::pow(1000.0, suffixIndex + 1);
        
        stream << std::fixed << std::setprecision(3) << formattedNumber << suffixes[suffixIndex];
        return stream.str();
    }
}

string formatFreq(long long count) {
    double number = static_cast<double>(count);  // Convert count to double for formatting
    std::stringstream stream;

    if (count < 1000) {
        // Format numbers less than 1000 to three decimal places
        stream << std::fixed << std::setprecision(3) << number << "Hz";
        return stream.str();
    } else {
        // Determine the suffix and format accordingly
        const char* suffixes[] = {"kHz", "MHz", "GHz", "THz", "EHz"};
        size_t suffixIndex = static_cast<size_t>(std::floor(std::log10(count) / 3)) - 1; // Determine the index based on the number's magnitude
        double formattedNumber = number / std::pow(1000.0, suffixIndex + 1);
        
        stream << std::fixed << std::setprecision(3) << formattedNumber << suffixes[suffixIndex];
        return stream.str();
    }
}

int main() {
    // Register signal handler for SIGINT
    signal(SIGINT, signalHandler);

    cout << "Intention Repeater WiFi v0.1" << endl;
	cout << "by Anthro Teacher and WebGPT" << endl << endl;
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
    broadcastAddr.sin_port = htons(12345);

    long long count = 0, freq = 0;
    auto lastUpdate = chrono::steady_clock::now();

    while (true) {
        broadcastMessage(globalSock, intention, broadcastAddr);
        ++count;
        ++freq;

        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(now - lastUpdate).count() >= 1) {
            cout << "\rBroadcasting: " << formatNumber(count) << " Repetitions [" << formatFreq(freq) << "]" << string(5, ' ') << "\r";
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
