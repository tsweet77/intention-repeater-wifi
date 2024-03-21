/*
Intention Repeater WiFi v0.5
Created by Anthro Teacher and WebGPT.
To compile: g++ -O3 -static -Wall Intention_Repeater_WiFi.cpp -o Intention_Repeater_WiFi.exe -lws2_32
My Intention: I am cleared, healed, balanced and release what I do not need. I am a 5D Light Being. I am Love and Bliss.
*/

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cmath>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <vector>
#include <algorithm>
#include <atomic>

using namespace std;

SOCKET globalSock = INVALID_SOCKET;
volatile bool running = true;
long long ErrCount = 0;
long long seconds = 0;

std::atomic<long long> iterations(0);
std::atomic<long long> freq(0);

string intention;
struct sockaddr_in broadcastAddr;

void signalHandler(int signum)
{
    cout << "\nShutting down..." << endl;
    running = false;
    exit(signum);
}

std::string formatTime(long long seconds)
{
    // Calculate hours, minutes, and seconds
    long long hours = seconds / 3600;
    long long minutes = (seconds % 3600) / 60;
    long long secs = seconds % 60;

    // Use stringstream to format the string
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << minutes << ":"
       << std::setfill('0') << std::setw(2) << secs;

    return ss.str();
}

void broadcastMessage(const SOCKET &sock, const string &message, struct sockaddr_in &broadcastAddr)
{
    if (sendto(sock, message.c_str(), static_cast<int>(message.length()), 0,
               (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) == SOCKET_ERROR)
    {
        ErrCount++;
        std::cerr << ErrCount << ": Error in sending broadcast message with error: " << WSAGetLastError() << std::endl;
    }
}

string formatNumber(long long iterations)
{
    if (iterations < 1000)
    {
        return to_string(iterations);
    }
    else
    {
        const char *suffixes[] = {"k", "M", "B", "T", "Q"};
        size_t suffixIndex = static_cast<size_t>(log10(iterations) / 3) - 1;
        double formattedNumber = iterations / pow(1000.0, suffixIndex + 1);

        char buffer[20];
        sprintf(buffer, "%.3f%s", formattedNumber, suffixes[suffixIndex]);
        return string(buffer);
    }
}

string formatFreq(long long iterations)
{
    if (iterations < 1000)
    {
        return to_string(iterations) + "Hz";
    }
    else
    {
        const char *suffixes[] = {"kHz", "MHz", "GHz", "THz", "EHz"};
        size_t suffixIndex = static_cast<size_t>(log10(iterations) / 3) - 1;
        double formattedNumber = iterations / pow(1000.0, suffixIndex + 1);

        char buffer[20];
        sprintf(buffer, "%.3f%s", formattedNumber, suffixes[suffixIndex]);
        return string(buffer);
    }
}

void broadcastThread()
{
    while (running)
    {
        broadcastMessage(globalSock, intention, broadcastAddr);
        ++iterations;
        ++freq;
    }
}

int main()
{
    signal(SIGINT, signalHandler);

    cout << "Intention Repeater WiFi Broadcaster v0.5" << endl;
    cout << "by Anthro Teacher, WebGPT and Claude 3 Opus" << endl
         << endl;

    cout << "Enter your Intention: ";
    getline(cin, intention);

    string multithreaded;
    cout << "MultiThreaded (y/N): ";
    getline(cin, multithreaded);
    transform(multithreaded.begin(), multithreaded.end(), multithreaded.begin(), ::tolower);

    if (multithreaded != "y" && multithreaded != "yes")
    {
        multithreaded = "n";
    }

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != NO_ERROR)
    {
        cerr << "WSAStartup failed with error: " << result << endl;
        return 1;
    }

    globalSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (globalSock == INVALID_SOCKET)
    {
        cerr << "Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    const int broadcast = 1;
    if (setsockopt(globalSock, SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast, sizeof(broadcast)) == SOCKET_ERROR)
    {
        cerr << "Error in setting Broadcast option with error: " << WSAGetLastError() << endl;
        closesocket(globalSock);
        WSACleanup();
        return 1;
    }

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    broadcastAddr.sin_port = htons(11111);

    vector<thread> threads;
    auto lastUpdate = chrono::steady_clock::now();

    if (multithreaded == "y")
    {
        for (int i = 0; i < 8; ++i)
        {
            // threads.emplace_back(std::thread(broadcastThread, std::ref(iterations), std::ref(freq)));
            threads.emplace_back(std::thread(broadcastThread));
        }
    }

    while (running)
    {
        if (multithreaded != "y")
        {
            broadcastMessage(globalSock, intention, broadcastAddr);
            ++iterations;
            ++freq;
        }

        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - lastUpdate).count() >= 1000)
        {
            cout << "\rBroadcasting: [" << formatTime(seconds) << "] " << formatNumber(iterations) << " Repetitions (" << formatFreq(freq) << ")" << string(5, ' ') << "\r";
            cout.flush();
            lastUpdate = now;
            freq = 0;
            ++seconds;
        }
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    if (globalSock != INVALID_SOCKET)
    {
        closesocket(globalSock);
        WSACleanup();
    }

    return 0;
}