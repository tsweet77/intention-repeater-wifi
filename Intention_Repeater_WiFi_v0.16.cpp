/*
Intention Repeater WiFi v0.16
Created by Anthro Teacher, WebGPT and Claude 3 Opus.
To compile: g++ -O3 -static -Wall main3.cpp -o main3.exe -lws2_32 -std=c++20 -lz
My Intention: I am cleared, healed, balanced and release what I do not need. I am a 5D Light Being. I am Love and Bliss.
*/

#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <cmath>
#include <conio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <vector>
#include <algorithm>
#include <atomic>
#include "picosha2.hPP"
#include <zlib.h>
#include <mutex>

using namespace std;
const std::string WindowTitle = "Nathan AND Anthro Teacher's Wifi Intention Repeater";
constexpr bool WriteDescriptionOfPerformanceToTitle = false;
int ErrCount = 0;
long long int seconds = 0, multiplier = 1, hash_multiplier = 1;

constexpr int NUM_THREADS = 8;
constexpr int MAX_PACKET_LENGTH = 1400;
constexpr int BUFFER_SIZE = 2048 * 2048;
constexpr int BROADCAST_PORT_BASE = 11111;

bool useMultiplying = false;
bool useHashing = false;
bool useRemoveSpaces = false;
bool useMultithreaded = false;
bool useZLIBCompression = false;

vector<SOCKET> threadSocks(NUM_THREADS + 1, INVALID_SOCKET);
vector<atomic<long long>> threadFreq(NUM_THREADS + 1);

atomic<bool> exitFlag(false);
std::mutex coutMutex;

string intention = "", intention_multiplied = "", param_duration = "X";

struct sockaddr_in broadcastAddr[NUM_THREADS + 1];

void introPrompt()
{
    const std::string introText = R"(
Intention Repeater WiFi Broadcaster v0.16
by Anthro Teacher, Nathan Myerscough, WebGPT and Claude 3 Opus
Note: May interrupt your internet connection when using.)";

    std::cout << introText << std::endl;
}

void print_help()
{
    const std::string helpText = R"(
Optional Flags:
 a) --compress or -c, example: --compress y
 b) --hashing or -s, example: --hashing y
 c) --file or -f, example: --file "myfile.txt"
 d) --dur or -d, example: --dur 00:01:00
 e) --multithread or -t, example: --multithread y
 f) --mult or -m, example: --mult y
 g) --remspaces or r, example: --remspaces y
 h) --help or -h, example: --help
 
 --compress = Use compression (y/n).
 --hashing = Use hashing (y/n).
 --file = Use file as input.
 --dur = Set duration of broadcast in HH:MM:SS format.
 --multithread = Set to use multithreading.
 --mult = Use intention multiplying.
 --remspaces = Remove spaces from intention.
 --help = Print this help message.)";

    introPrompt();
    std::cout << helpText << std::endl;
}

std::string compressMessage(const std::string &message)
{
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, Z_DEFAULT_COMPRESSION) != Z_OK)
    {
        return ""; // Compression initialization failed
    }

    zs.next_in = (Bytef *)message.data();
    zs.avail_in = message.size();

    std::string compressed;
    char outbuffer[32768]; // Output buffer
    int ret;
    do
    {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (compressed.size() < zs.total_out)
        {
            compressed.append(outbuffer, zs.total_out - compressed.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END)
    {
        return ""; // Compression failed
    }

    return compressed;
}

std::string removeSpaces(std::string str)
{
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    return str;
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

void broadcastMessage(const SOCKET &sock, const std::string &message, const struct sockaddr_in &broadcastAddr)
{
    const char *buffer = message.c_str();
    int bufferSize = static_cast<int>(message.length());
    int bytesSent = 0;
    int errorCode = 0; // Variable to store the error code
    bool errorReported = false;
    while (bytesSent < bufferSize)
    {
        int bytesToSend = std::min(bufferSize - bytesSent, MAX_PACKET_LENGTH);
        int result = sendto(sock, buffer + bytesSent, bytesToSend, 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
        if (result == SOCKET_ERROR)
        {
            if (!errorReported)
            {
                // Store the error code when it occurs for the first time
                errorCode = WSAGetLastError();
                errorReported = true;
            }
            break;
        }
        bytesSent += result;
    }
    if (errorReported)
    {
        // Construct the error message
        std::ostringstream errorMessage;
        errorMessage << "\rBroadcasting: Error sending data. Error code: " << errorCode << " - ";
        // Get the error message corresponding to the error code
        LPSTR errorBuffer = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorBuffer, 0, NULL);
        // Append the error message to the stream
        errorMessage << errorBuffer << std::endl;
        // Free the error buffer
        LocalFree(errorBuffer);
        // Output the error message
        std::cerr << errorMessage.str();
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

// Thread-safe cout function
void threadSafeCout(const std::string &message)
{
    // Lock the mutex before accessing std::cout
    std::lock_guard<std::mutex> lock(coutMutex);

    // Output the message
    std::cout << message << std::endl;

    // Automatically release the lock when lock_guard goes out of scope
}

void broadcastThread(int threadIndex)
{
    SOCKET &sock = threadSocks[threadIndex];
    atomic<long long> &freq = threadFreq[threadIndex];

    while (!exitFlag)
    {
        broadcastMessage(sock, intention_multiplied, broadcastAddr[threadIndex]);
        ++freq;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    threadSafeCout("BROADCASTING THREAD #" + std::to_string(threadIndex) + " EXITING");
}
#define WAITFOREVER 2000000000
void keyInputThreadFunction()
{
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD irInputRecord;
    DWORD dwEventsRead;

    while (!exitFlag)
    {
        if (WaitForSingleObject(hStdIn, WAITFOREVER) == WAIT_OBJECT_0)
        {
            ReadConsoleInput(hStdIn, &irInputRecord, 1, &dwEventsRead);
            if (irInputRecord.EventType == KEY_EVENT && irInputRecord.Event.KeyEvent.bKeyDown)
            {
                switch (irInputRecord.Event.KeyEvent.wVirtualKeyCode)
                {
                case VK_ESCAPE:
                    exitFlag = true;
                    threadSafeCout("Escape key detected. Setting exitFlag to true.");
                    break;
                }
            }
        }
    }
    threadSafeCout("KEY INPUT THREAD EXITING");
}
/// FUNCTION FOR TIME KEEPER DESCRIBER OF INENTION REPEATS THREAD ///
void timeKeeperThreadFunction()
{
    std::chrono::steady_clock::time_point lastUpdate = std::chrono::steady_clock::now();
    int seconds = 0;
    std::string iterations_string = "0", iterations_string_freq = "0";
    long long freq = 0;
    int digits = 0, freq_digits = 0;
    while (!exitFlag)
    {
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(now - lastUpdate).count() >= 1000)
        {
            for (int i = 0; i < NUM_THREADS + 1; ++i)
            {
                freq += threadFreq[i].load();
                threadFreq[i] = 0;
            }
            iterations_string_freq = to_string(freq * multiplier);
            iterations_string = findsum(iterations_string, iterations_string_freq);
            digits = iterations_string.length();
            freq_digits = iterations_string_freq.length();
            std::string fullDescription = "Broadcasting: [" + formatTime(seconds) + "] " + display_suffix(iterations_string, digits - 1, "Iterations") + " Repetitions (" +
                                          display_suffix(iterations_string_freq, freq_digits - 1, "Frequency") + "Hz) (ESC to exit)";
            if (WriteDescriptionOfPerformanceToTitle)
            {
                static const std::string WindowTitleWithSeperator = WindowTitle + ": ";
                std::string fullStringWithWindowTitle = WindowTitleWithSeperator + fullDescription;
                SetConsoleTitle(fullStringWithWindowTitle.c_str());
            }
            else
            {
                cout << "\r" << fullDescription << string(20, ' ') << "\r" << flush;
            }

            if (formatTime(seconds) == param_duration)
            {
                exitFlag = true;
                threadSafeCout("Time limit reached. Setting exitFlag to true.");
            }

            lastUpdate = now;
            freq = 0;
            ++seconds;
        }
        switch (std::chrono::milliseconds(chrono::duration_cast<chrono::milliseconds>(now - lastUpdate)).count())
        {
        case 0 ... 300:
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            break;
        case 301 ... 600:
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            break;
        case 601 ... 800:
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            break;
        case 801 ... 900:
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            break;
        case 901 ... 980:
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            break;
        }
    }
    threadSafeCout("TIME KEEPER PERFORMANCE DESCRIBER THREAD EXITING");
}

std::pair<std::string, long long int> multiplyIntention(const std::string &intention, std::string::size_type bufferSize)
{
    std::string intention_multiplied = "", hashed_intention = "", hash_intention_multiplied = "";
    multiplier = 0;

    while (intention_multiplied.length() + intention.length() <= bufferSize)
    {
        intention_multiplied += intention;
        ++multiplier; // Increment the counter each time the intention is appended
    }

    if (useHashing)
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
        multiplier *= hash_multiplier;
    }

    if (useZLIBCompression)
    {
        intention_multiplied = compressMessage(intention_multiplied);
    }

    // Return the multiplied intention and the multiplier
    return std::make_pair(intention_multiplied, multiplier);
}
/// QUERIES USER ABOUT THE SETTING AND SETS IT ACCORDINGLY WAITING FOR PROPER INPUT ///
void setBooleanWhileLoop(bool &theBoolean, const std::string &thePrompt)
{
    while (true)
    {
        string input;
        cout << thePrompt;
        getline(cin, input);
        transform(input.begin(), input.end(), input.begin(), ::tolower);
        if (input == "y" || input == "yes")
        {
            theBoolean = true;
            break;
        }
        else if (input == "n" || input == "no")
        {
            theBoolean = false;
            break;
        }
    }
}

void setInt(int &theInt, const std::string &thePrompt)
{
    string input;
    cout << thePrompt;
    getline(cin, input);
    theInt = std::stoi(input);
}

void setStringWhileLoop(std::string &theString, const std::string &thePrompt)
{
    while (true)
    { // Infinite loop
        std::cout << thePrompt;
        std::getline(std::cin, theString);

        // Trim leading and trailing whitespace
        std::string trimmedString = theString;
        trimmedString.erase(0, trimmedString.find_first_not_of(" \t\n\r"));
        trimmedString.erase(trimmedString.find_last_not_of(" \t\n\r") + 1);

        // Check if the intention string is empty or contains only whitespace
        if (theString.empty() || trimmedString.empty())
        {
            std::cout << "The intention cannot be empty or contain only whitespace. Please try again.\n";
        }
        else
        {
            break; // Exit the loop if an intention has been entered
        }
    }
}
/// INITIALIZES NECESARY THINGS FOR WINDOWS SOCKETS ///
int setupWindowsSockets()
{
    WSADATA wsaData;
    int wsaresult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaresult != NO_ERROR)
    {
        cerr << "WSAStartup failed with error: " << wsaresult << endl;
        return 1;
    }
    return 0;
}

void setupBroadCastAddress(const int i)
{
    memset(&broadcastAddr[i], 0, sizeof(broadcastAddr[i]));
    broadcastAddr[i].sin_family = AF_INET;
    broadcastAddr[i].sin_addr.s_addr = inet_addr("255.255.255.255");
    broadcastAddr[i].sin_port = htons(BROADCAST_PORT_BASE + i);
}
/// setup individual socket ///
// setup individual socket
int setupSocket(SOCKET &theSocket, const int i)
{
    // setWindowsSockets();

    theSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (theSocket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed for thread " << i << " with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    const int broadcast = 1; // Value to enable broadcast
    if (setsockopt(theSocket, SOL_SOCKET, SO_BROADCAST, (const char *)&broadcast, sizeof(broadcast)) == SOCKET_ERROR)
    {
        // Setting socket option for enabling broadcast
        cerr << "Error in setting Broadcast option with error: " << WSAGetLastError() << endl; // Printing error message if setting fails
        closesocket(theSocket);                                                                // Closing socket
        WSACleanup();                                                                          // Cleaning up Winsock resources
        return 1;                                                                              // Exiting program with error code
    }

    if (setsockopt(theSocket, SOL_SOCKET, SO_SNDBUF, (const char *)&BUFFER_SIZE, sizeof(BUFFER_SIZE)) == SOCKET_ERROR)
    {
        cerr << "Error in setting UDP buffer size with error: " << WSAGetLastError() << endl;
        closesocket(theSocket);
        WSACleanup();
        return 1;
    }

    return 0;
}
void socketCleanup()
{
    threadSafeCout("CLEANING UP SOCKETS");
    for (auto &sock : threadSocks)
    {
        if (sock != INVALID_SOCKET)
        {
            closesocket(sock);
        }
    }
    WSACleanup();
}

int main(int argc, char **argv)
{
    std::string param_intention = "X", param_hashing = "X", param_multithreaded = "X", param_multiplying = "X", param_remove_spaces = "X", param_compress = "X", param_file = "X";

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") || !strcmp(argv[i], "/?"))
        {
            print_help();
            std::exit(EXIT_SUCCESS);
        }
        else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dur"))
        {
            param_duration = argv[i + 1];
        }
        else if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--intent"))
        {
            param_intention = argv[i + 1];
        }
        else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file"))
        {
            param_file = argv[i + 1];
        }
        else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--multithread"))
        {
            param_multithreaded = argv[i + 1];
        }
        else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--compress"))
        {
            param_compress = argv[i + 1];
        }
        else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--hashing"))
        {
            param_hashing = argv[i + 1];
        }
        else if (!strcmp(argv[i], "-m") || !strcmp(argv[i], "--mult"))
        {
            param_multiplying = argv[i + 1];
        }
        else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--remspaces"))
        {
            param_remove_spaces = argv[i + 1];
        }
    }

    setupWindowsSockets();
    SetConsoleTitle(WindowTitle.c_str());
    introPrompt();
    if (param_file == "X")
    {
        if (param_intention == "X")
        {
            setStringWhileLoop(intention, "Enter your Intention: ");
        }
        else
        {
            intention = param_intention;
        }
    }
    else
    {
        // Read from param_file and store the full file contents in intention
        std::ifstream file(param_file);
        std::string file_contents;
        file_contents.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        intention = file_contents;
        file.close();
    }

    if (param_multithreaded == "X")
    {
        setBooleanWhileLoop(useMultithreaded, "Use Multithreading (y/N): ");
    }
    else
    {
        useMultithreaded = (param_multithreaded == "y" || param_multithreaded == "Y");
    }

    if (param_multiplying == "X")
    {
        setBooleanWhileLoop(useMultiplying, "Use Multiplying (y/N): ");
    }
    else
    {
        useMultiplying = (param_multiplying == "y" || param_multiplying == "Y");
    }

    if (param_remove_spaces == "X")
    {
        setBooleanWhileLoop(useRemoveSpaces, "Remove Empty Spaces? (y/n): ");
    }
    else
    {
        useRemoveSpaces = (param_remove_spaces == "y" || param_remove_spaces == "Y");
    }

    if (useRemoveSpaces)
    {
        intention = removeSpaces(intention);
    }
    if (useMultiplying)
    {
        if (param_hashing == "X")
        {
            setBooleanWhileLoop(useHashing, "Use Hashing? (y/n): ");
        }
        else
        {
            useHashing = (param_hashing == "y" || param_hashing == "Y");
        }
        if (param_compress == "X")
        {
            setBooleanWhileLoop(useZLIBCompression, "Use ZLIB Compression? (y/n): ");
        }
        else
        {
            useZLIBCompression = (param_compress == "y" || param_compress == "Y");
        }

        auto result = multiplyIntention(intention, BUFFER_SIZE);
        intention_multiplied = result.first;
        multiplier = result.second;
    }
    else
    {
        intention_multiplied = intention;
        multiplier = 1;
    }
    // std::cout << intention_multiplied << "\n";
    std::jthread timeKeeperThread(&timeKeeperThreadFunction);
    std::jthread keyInputThread(&keyInputThreadFunction);
    std::vector<std::jthread> BroadcastingThreads;
    if (useMultithreaded)
    {
        for (int i = 0; i < NUM_THREADS + 1; ++i)
        {
            setupBroadCastAddress(i);
            setupSocket(threadSocks[i], i);
            if (i < NUM_THREADS)
                BroadcastingThreads.emplace_back(&broadcastThread, i);
        }
    }
    else // Not Multithreaded
    {
        setupBroadCastAddress(8);
        setupSocket(threadSocks[8], 8);
    }
    while (!exitFlag)
    {
        broadcastMessage(threadSocks[8], intention_multiplied, broadcastAddr[8]);
        threadFreq[8]++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    threadSafeCout("MAIN THREAD EXITING");
    socketCleanup();
    return 0;
}
