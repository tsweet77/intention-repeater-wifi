/*
Intention Repeater WiFi v0.14
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
int ErrCount = 0;
long long int seconds = 0;

const int NUM_THREADS = 8;
const int MAX_PACKET_LENGTH = 65500;
const int BUFFER_SIZE = 2048 * 2048;

vector<SOCKET> threadSocks(NUM_THREADS, INVALID_SOCKET);
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
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    // Use stringstream to format the string
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << minutes << ":"
       << std::setfill('0') << std::setw(2) << secs;

    return ss.str();
}

void broadcastMessage(const SOCKET &sock, const std::string &message, struct sockaddr_in &broadcastAddr)
{
    static bool errorReported = false; // Static flag to track if error has been reported.
    const char *buffer = message.c_str();
    int bufferSize = static_cast<int>(message.length());
    int bytesSent = 0;

    while (bytesSent < BUFFER_SIZE)
    {
        int bytesToSend = min(bufferSize - bytesSent, MAX_PACKET_LENGTH);
        int result = sendto(sock, buffer + bytesSent, bytesToSend, 0,
                            (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
        if (result == SOCKET_ERROR)
        {
            if (!errorReported) // Only report the error if it has not been reported before.
            {
                std::cerr << "\rBroadcasting: Waiting..." << string(40, ' ') << "\r" << flush;
                errorReported = true; // Set the flag to indicate that the error has been reported.
            }
            break; // Exit the loop after reporting an error.
        }
        else
        {
            errorReported = false; // Reset the flag if there was successful send after an error.
        }
        bytesSent += result;
    }
}

std::string display_suffix(std::string num, int power, std::string designator)
{
    std::string s;
    if (designator == "Iterations")
    {
        char iterations_suffix_array[] = {' ', 'k', 'M', 'B', 'T', 'q', 'Q', 's', 'S', 'O', 'N', 'D'};
        // cout << "Power: " << power << endl;
        s = iterations_suffix_array[int(power / 3)];
    }
    else // designator == "Frequency"
    {
        char frequency_suffix_array[] = {' ', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
        // cout << "Power: " << power << endl;
        s = frequency_suffix_array[int(power / 3)];
    }

    std::string str2 = num.substr(0, power % 3 + 1) + "." + num.substr(power % 3 + 1, 3) + s;

    return str2;
}

// Utility function to find the sum of two numbers represented as a string in CPP
std::string findsum(std::string a, std::string b)
{

    std::vector<int> finalsum; // Stores the final sum of two number

    int carry = 0; // Stores carry at each stage of calculation

    /* Step 1 starts here */

    int i = a.size() - 1,
        j = b.size() - 1; // Start adding from lowest significant bit
    while ((i >= 0) && (j >= 0))
    {                                                // Loop until either of number exhausts first
        int x = (a[i] - '0') + (b[j] - '0') + carry; // Calculate the sum of digit in final sum by adding
                                                     // respective digits with previous carry.
        finalsum.push_back(x % 10);                  // Store the respective digit of the final sum in a vector.
        carry = x / 10;                              // update the carry. The carry for next step is the
                                                     // remaining number after forming the digit of final sum.
        i--;                                         // Move one step towards the left in both the string(numbers)
        j--;
    }
    /*  Step 2 starts here */

    while (i >= 0)
    {                                 // If the number 1 was greater than number 2, then there must
                                      // be some digits to be taken care off.
        int x = (a[i] - '0') + carry; // Add the remaining digits to the carry one
                                      // by one and store the unit digit.
        finalsum.push_back(x % 10);
        carry = x / 10; // update the carry from each step.
        i--;
    }
    /* Step 3 starts here */

    while (j >= 0)
    {                                 // If the number 2 was greater than number 1, then there must
                                      // be some digits to be taken care off.
        int x = (b[j] - '0') + carry; // Add the remaining digits to the carry one
                                      // by one and store the unit digit.
        finalsum.push_back(x % 10);
        carry = x / 10; // update the carry from each step.
        j--;
    }
    /* Step 4 starts here */

    while (carry)
    {                                   // If after finishing addition of the two numbers, if there is
                                        // still carry/leftover then we need to take it into the final
                                        // sum.
        finalsum.push_back(carry % 10); // Store digit one by one.
        carry = carry / 10;             // Reduce carry
    }
    /* Step 5 starts here */
    std::stringstream final_iter;
    // Since vector pushes value at last, the most significant digits starts at
    // the end of the vector. Thus print reverse.

    std::copy(finalsum.rbegin(), finalsum.rend(), std::ostream_iterator<int>(final_iter, ""));

    return final_iter.str();
}

void broadcastThread(int threadIndex)
{
    SOCKET &sock = threadSocks[threadIndex];
    atomic<long long> &freq = threadFreq[threadIndex];

    while (running)
    {
        broadcastMessage(sock, intention_multiplied, broadcastAddr);
        ++freq;
    }
}

std::pair<std::string, long long int> multiplyIntention(const std::string &intention, const std::string &use_hashing, std::string::size_type bufferSize)
{
    std::string intention_multiplied = "", hashed_intention = "", hash_intention_multiplied = "";
    long long int multiplier = 0;
    long long int hash_multiplier = 1;

    while (intention_multiplied.length() + intention.length() <= bufferSize)
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

        while (hash_intention_multiplied.length() + hashed_intention.length() <= bufferSize)
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
    std::string iterations_string = "0", iterations_string_freq = "0";
    long long freq = 0;
    int digits = 0, freq_digits = 0;

    cout << "Intention Repeater WiFi Broadcaster v0.14" << endl;
    cout << "by Anthro Teacher, WebGPT and Claude 3 Opus" << endl;
    cout << "Note: May interrupt your internet connection when using." << endl
         << endl;

    while (true) { // Infinite loop
        std::cout << "Enter your Intention: ";
        std::getline(std::cin, intention);

        // Check if the intention string is not empty
        if (!intention.empty()) {
            break; // Exit the loop if an intention has been entered
        }

        // Optional: Inform the user that the intention cannot be empty
        std::cout << "The intention cannot be empty. Please try again.\n";
    }

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
        auto result = multiplyIntention(intention, use_hashing, BUFFER_SIZE);

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
        threadSocks[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (threadSocks[i] == INVALID_SOCKET)
        {
            cerr << "Socket creation failed for thread " << i << " with error: " << WSAGetLastError() << endl;
            WSACleanup();
            return 1;
        }

        if (setsockopt(threadSocks[i], SOL_SOCKET, SO_SNDBUF, (const char *)&BUFFER_SIZE, sizeof(BUFFER_SIZE)) == SOCKET_ERROR)
        {
            cerr << "Error in setting UDP buffer size with error: " << WSAGetLastError() << endl;
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
            if (chrono::duration_cast<chrono::milliseconds>(now - lastUpdate)
                    .count() >= 1000)
            {
                for (int i = 0; i < NUM_THREADS; ++i)
                {
                    freq += threadFreq[i].load();
                    threadFreq[i] = 0;
                }
                iterations_string_freq = to_string(freq * multiplier);
                iterations_string = findsum(iterations_string, iterations_string_freq);
                digits = iterations_string.length();
                freq_digits = iterations_string_freq.length();

                cout << "\rBroadcasting: [" << formatTime(seconds) << "] "
                     << display_suffix(iterations_string, digits - 1, "Iterations")
                     << " Repetitions ("
                     << display_suffix(iterations_string_freq, freq_digits - 1, "Frequency")
                     << "Hz)" << string(10, ' ') << "\r" << flush;

                lastUpdate = now;
                ++seconds;
                freq = 0;
                //std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        for (auto &thread : threads)
        {
            thread.join();
        }
    }
    else // Not Multithreaded
    {
        while (running)
        {
            broadcastMessage(threadSocks[0], intention_multiplied, broadcastAddr);
            ++freq;

            auto now = chrono::steady_clock::now();
            if (chrono::duration_cast<chrono::milliseconds>(now - lastUpdate).count() >= 1000)
            {
                iterations_string_freq = to_string(freq * multiplier);
                iterations_string = findsum(iterations_string, iterations_string_freq);
                digits = iterations_string.length();
                freq_digits = iterations_string_freq.length();
                cout << "\rBroadcasting: [" << formatTime(seconds) << "] "
                     << display_suffix(iterations_string, digits - 1, "Iterations")
                     << " Repetitions ("
                     << display_suffix(iterations_string_freq, freq_digits - 1, "Frequency")
                     << "Hz)" << string(10, ' ') << "\r" << flush;

                //std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Adjust based on required rate
                lastUpdate = now;
                freq = 0;
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