/*
Intention Repeater WiFi v0.12
Created by Anthro Teacher, WebGPT and Claude 3 Opus.
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
#include "picosha2.h"

using namespace std;

volatile bool running = true;
long long ErrCount = 0;
long long seconds = 0;

const int NUM_THREADS = 8;
const int MAX_PACKET_LENGTH = 65500;

vector<SOCKET> threadSocks(NUM_THREADS, INVALID_SOCKET);
vector<atomic<long long>> threadIterations(NUM_THREADS);
vector<atomic<long long>> threadFreq(NUM_THREADS);

string intention = "", intention_multiplied = "";
long long int multiplier = 0;

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
    const char *buffer = message.c_str();
    int bufferSize = static_cast<int>(message.length());
    int bytesSent = 0;

    while (bytesSent < bufferSize)
    {
        int bytesToSend = min(bufferSize - bytesSent, MAX_PACKET_LENGTH);
        int result = sendto(sock, buffer + bytesSent, bytesToSend, 0,
                            (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
        if (result == SOCKET_ERROR)
        {
            ErrCount++;
            std::cerr << ErrCount << ": Error in sending broadcast message with error: " << WSAGetLastError() << std::endl;
            break;
        }
        bytesSent += result;
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

void broadcastThread(int threadIndex)
{
    SOCKET &sock = threadSocks[threadIndex];
    atomic<long long> &iterations = threadIterations[threadIndex];
    atomic<long long> &freq = threadFreq[threadIndex];

    while (running)
    {
        broadcastMessage(sock, intention_multiplied, broadcastAddr);
        ++iterations;
        ++freq;
    }
}

std::pair<std::string, long long int> multiplyIntention(const std::string &intention, const std::string &use_hashing, std::string::size_type buffer_size)
{
    std::string intention_multiplied = "", hashed_intention = "", hash_intention_multiplied = "";
    long long int multiplier = 0;
    long long int hash_multiplier = 1;

    while (intention_multiplied.length() + intention.length() <= buffer_size)
    {
        intention_multiplied += intention;
        ++multiplier; // Increment the counter each time the intention is appended
    }

    if (use_hashing == "y")
    {
        // Hash the value and multiply it.
        hashed_intention = picosha2::hash256_hex_string(intention_multiplied);
        hash_intention_multiplied = "";
        hash_multiplier = 0;

        while (hash_intention_multiplied.length() + hashed_intention.length() <= buffer_size)
        {
            hash_intention_multiplied += hashed_intention;
            ++hash_multiplier;
        }
        intention_multiplied = hash_intention_multiplied;
    }

    // Return the multiplied intention and the multiplier
    return std::make_pair(intention_multiplied, multiplier * hash_multiplier);
}

int main()
{
    signal(SIGINT, signalHandler);

    cout << "Intention Repeater WiFi Broadcaster v0.12" << endl;
    cout << "by Anthro Teacher, WebGPT and Claude 3 Opus" << endl
         << endl;

    cout << "Enter your Intention: ";
    getline(cin, intention);

    string multithreaded;
    cout << "MultiThreaded (y/N): ";
    getline(cin, multithreaded);
    transform(multithreaded.begin(), multithreaded.end(), multithreaded.begin(), ::tolower);

    string use_multiplying;
    cout << "Use Multiplying (y/N): ";
    getline(cin, use_multiplying);
    transform(use_multiplying.begin(), use_multiplying.end(), use_multiplying.begin(), ::tolower);

    if (use_multiplying != "y" && use_multiplying != "yes")
    {
        use_multiplying = "n";
    }

    string use_hashing;

    if (use_multiplying == "y")
    {
        cout << "Use Hashing (y/N): ";
        getline(cin, use_hashing);
        transform(use_hashing.begin(), use_hashing.end(), use_hashing.begin(), ::tolower);

        if (use_hashing != "y" && use_hashing != "yes")
        {
            use_hashing = "n";
        }
    }
    else
    {
        use_hashing = "n";
    }

    if (use_multiplying == "y")
    {
        int buffer_size = 2048 * 2048;
        auto result = multiplyIntention(intention, use_hashing, buffer_size);

        intention_multiplied = result.first;
        multiplier = result.second;
    }
    else
    {
        intention_multiplied = intention;
        multiplier = 1;
    }

    WSADATA wsaData;
    int wsaresult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaresult != NO_ERROR)
    {
        cerr << "WSAStartup failed with error: " << wsaresult << endl;
        return 1;
    }

    for (int i = 0; i < NUM_THREADS; ++i)
    {
        threadSocks[i] = socket(AF_INET, SOCK_DGRAM, 0);
        if (threadSocks[i] == INVALID_SOCKET)
        {
            cerr << "Socket creation failed for thread " << i << " with error: " << WSAGetLastError() << endl;
            WSACleanup();
            return 1;
        }

        const int broadcast = 1;
        if (setsockopt(threadSocks[i], SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast, sizeof(broadcast)) == SOCKET_ERROR)
        {
            cerr << "Error in setting Broadcast option for thread " << i << " with error: " << WSAGetLastError() << endl;
            closesocket(threadSocks[i]);
            WSACleanup();
            return 1;
        }
    }

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    broadcastAddr.sin_port = htons(11111);

    vector<thread> threads;
    auto lastUpdate = chrono::steady_clock::now();

    if (multithreaded == "y")
    {
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            threads.emplace_back(std::thread(broadcastThread, i));
        }

        while (running)
        {
            auto now = chrono::steady_clock::now();
            if (chrono::duration_cast<chrono::milliseconds>(now - lastUpdate).count() >= 1000)
            {
                long long totalIterations = 0;
                long long totalFreq = 0;
                for (int i = 0; i < NUM_THREADS; ++i)
                {
                    totalIterations += threadIterations[i].load();
                    totalFreq += threadFreq[i].load();
                    threadFreq[i] = 0;
                }
                cout << "\rBroadcasting: [" << formatTime(seconds) << "] " << formatNumber(totalIterations * multiplier) << " Repetitions (" << formatFreq(totalFreq * multiplier) << ")" << string(10, ' ') << "\r";
                cout.flush();
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Adjust based on required rate
                lastUpdate = now;
                ++seconds;
            }
        }

        for (auto &thread : threads)
        {
            thread.join();
        }
    }
    else
    {
        while (running)
        {
            broadcastMessage(threadSocks[0], intention_multiplied, broadcastAddr);
            ++threadIterations[0];
            ++threadFreq[0];

            auto now = chrono::steady_clock::now();
            if (chrono::duration_cast<chrono::milliseconds>(now - lastUpdate).count() >= 1000)
            {
                cout << "\rBroadcasting: [" << formatTime(seconds) << "] " << formatNumber(threadIterations[0].load() * multiplier) << " Repetitions (" << formatFreq(threadFreq[0].load() * multiplier) << ")" << string(10, ' ') << "\r";
                cout.flush();
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Adjust based on required rate
                lastUpdate = now;
                threadFreq[0] = 0;
                ++seconds;
            }
        }
    }

    for (auto &sock : threadSocks)
    {
        if (sock != INVALID_SOCKET)
        {
            closesocket(sock);
        }
    }
    WSACleanup();

    return 0;
}