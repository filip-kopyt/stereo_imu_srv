#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

using namespace std::string_literals;

static const uint16_t mg_port = 12000;
static const std::string mg_path = "/dev/stereo_imu"s;
// static const std::string mg_path = "f"s;
static bool mg_stop = false;

void signal_handler(int) { mg_stop = true; }

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);

    int srvFd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > srvFd) {
        std::cerr << "socket() failed" << std::endl;
        return -1;
    }

    struct sockaddr_in srvSockAddrIn = {.sin_family = AF_INET, .sin_port = htons(mg_port), .sin_addr = {.s_addr = INADDR_ANY}};
    if (0 != bind(srvFd, (struct sockaddr*) &srvSockAddrIn, sizeof(srvSockAddrIn))) {
        std::cerr << "bind() failed" << std::endl;
        return -1;
    }

    if (0 != listen(srvFd, 1)) {
        std::cerr << "listen() failed, errno=" << errno << " (" << strerror(errno) << ')' << std::endl;
        return -1;
    }

    fd_set fdSetRd, fdSetWr;
    struct timeval tv;
    int clientFd = -1, fileFd = -1, fdMax;
    uint8_t buffer[1024];
    uint32_t size = 0u, n = 0u;
    while (!mg_stop) {
        FD_ZERO(&fdSetRd);
        FD_ZERO(&fdSetWr);

        tv.tv_sec = 0;
        tv.tv_usec = 10000;

        if ((-1 == clientFd) || (-1 == fileFd)) {
            FD_SET(srvFd, &fdSetRd);
            fdMax = srvFd + 1;

        } else {
            FD_SET(clientFd, &fdSetRd);
            fdMax = clientFd + 1;

            if ((0u == size) || (n == size)) {   // && timeout
                FD_SET(fileFd, &fdSetRd);
                fdMax = std::max(clientFd, fileFd) + 1;
            } else {
                FD_SET(clientFd, &fdSetWr);
                fdMax = clientFd + 1;
            }
        }

        int result = select(fdMax, &fdSetRd, &fdSetWr, nullptr, &tv);
        if (0 < result) {
            if (FD_ISSET(srvFd, &fdSetRd)) {
                struct sockaddr_in clientAddress;
                socklen_t clientAddressLen = sizeof(clientAddress);

                clientFd = accept(srvFd, (struct sockaddr*) &clientAddress, &clientAddressLen);

                uint32_t ip = ntohl(clientAddress.sin_addr.s_addr);
                uint16_t port = ntohs(clientAddress.sin_port);
                std::cout << "accept() ok, peer=" << (0xFFu & (ip >> 24u)) << '.' << (0xFFu & (ip >> 16u)) << '.' << (0xFFu & (ip >> 8u))
                          << '.' << (0xFFu & (ip >> 0u)) << ':' << port << std::endl;

                fileFd = open(mg_path.c_str(), O_RDONLY);
                if (0 > fileFd) {
                    close(clientFd);
                    clientFd = -1;

                    std::cerr << "open() failed, errno=" << errno << " (" << strerror(errno) << ')' << std::endl;
                }
            }
            if (FD_ISSET(fileFd, &fdSetRd)) {
                result = read(fileFd, &buffer[0], sizeof(buffer) / sizeof(buffer[0]));

                if (0 < result) {
                    size = result;
                    n = 0;

                } else if (0 > result) {
                    size = 0;
                    n = 0;

                    close(fileFd);
                    fileFd = -1;

                    close(clientFd);
                    clientFd = -1;

                    std::cerr << "read(fileFd) failed" << std::endl;
                } else {
                    static int i = 0;
                    std::cerr << i++ << '\r' << std::flush;
                }
            }
            if (FD_ISSET(clientFd, &fdSetRd)) {
                char c;
                result = read(clientFd, &c, 1);

                if (0 >= result) {
                    size = 0;
                    n = 0;

                    close(fileFd);
                    fileFd = -1;

                    close(clientFd);
                    clientFd = -1;

                    std::cout << "close()" << std::endl;
                }
            }
            if (FD_ISSET(clientFd, &fdSetWr)) {
                result = send(clientFd, &buffer[n], size - n, MSG_NOSIGNAL);

                if (0 < result) {
                    n += result;

                } else {
                    size = 0;
                    n = 0;

                    close(fileFd);
                    fileFd = -1;

                    close(clientFd);
                    clientFd = -1;

                    std::cerr << "read(fileFd) failed" << std::endl;
                }
            }
        } else if (0 > result) {
            std::cerr << "Select error, result=" << result << ", errno=" << errno << " (" << strerror(errno) << ")" << std::endl;
        }
    }

    shutdown(srvFd, SHUT_RDWR);
    close(srvFd);
    std::cout << "Shutting down." << std::endl;

    return 0;
}

// std::cout << "Waiting for client.." << std::endl;
// clientFd = accept(srvFd, (struct sockaddr*) &clientAddress, &clientAddressLen);
// if (0 > clientFd) {
//     std::cerr << "Could not accept client (errno=" << errno << ", " << strerror(errno) << ")." << std::endl;
//     break;
// }

// uint32_t clientIp = ntohl(clientAddress.sin_addr.s_addr);
// uint16_t clientPort = ntohs(clientAddress.sin_port);
// std::cout << "Accepted client @ " << (0xFFu & (clientIp >> 24u)) << '.' << (0xFFu & (clientIp >> 16u)) << '.' << (0xFFu & (clientIp >>
// 8u))
//           << '.' << (0xFFu & (clientIp >> 0u)) << ':' << clientPort << std::endl;

// std::ifstream ifs(mg_path);
// if (!ifs) {
//     std::cerr << "Could not open file \"" << mg_path << "\"." << std::endl;
//     break;
// }

// std::string line;
// const char c = '\n';
// while (!mg_stop) {
//     std::getline(ifs, line);

//     const char* pData = line.c_str();
//     const size_t size = line.size();
//     if (0 != size) {
//         size_t n = 0;
//         do {
//             result = send(clientFd, pData + n, size - n, MSG_NOSIGNAL);
//             if (0 <= result) {
//                 n += result;
//             }
//         } while ((0 <= result) && (n < size));
//         if (0 > result) {
//             std::cerr << "Could not transmit all data (" << n << "/" << size << ")." << std::endl;
//             break;
//         }

//         do {
//             result = send(clientFd, &c, sizeof(c), MSG_NOSIGNAL);
//         } while (0 == result);
//         if (0 > result) {
//             std::cerr << "Could not transmit data ('\\n')." << std::endl;
//             break;
//         }

//     } else if (!ifs) {
//         ifs.clear();
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     }
// }

// ifs.close();
// close(clientFd);
// }

// std::cout << "Closing server.." << std::endl;
// close(srvFd);

// return 0;
// }