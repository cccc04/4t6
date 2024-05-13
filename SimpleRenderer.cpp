//
// This file is used by the template to render a basic scene using GL.
//
//#include "pch.h"

#include "SimpleRenderer.h"
//#include "MathHelper.h"

// These are used by the shader compilation methods.
#include <vector>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <thread>
#include <assert.h>
#include <chrono>
#include <stdexcept>
#include <poll.h>
//using namespace std;

SimpleRenderer::SimpleRenderer() {
    yyn = false;
    yon = false;
    td = "7hbgh@jj";
}

int SimpleRenderer::connect_with_timeout(int sockfd, const struct sockaddr* addr, socklen_t addrlen, unsigned int timeout_ms) {
    int rc = 0;
    // Set O_NONBLOCK
    int sockfd_flags_before;
    if ((sockfd_flags_before = fcntl(sockfd, F_GETFL, 0) < 0)) return -1;
    if (fcntl(sockfd, F_SETFL, sockfd_flags_before | O_NONBLOCK) < 0) return -1;
    // Start connecting (asynchronously)
    do {
        if (connect(sockfd, addr, addrlen) < 0) {
            // Did connect return an error? If so, we'll fail.
            if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS)) {
                rc = -1;
            }
            // Otherwise, we'll wait for it to complete.
            else {
                // Set a deadline timestamp 'timeout' ms from now (needed b/c poll can be interrupted)
                struct timespec now;
                if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) { std::cout << "ln53" << std::endl; rc = -1; break; }
                struct timespec deadline = { .tv_sec = now.tv_sec,
                                             .tv_nsec = now.tv_nsec + timeout_ms * 1000000l };
                // Wait for the connection to complete.
                do {
                    // Calculate how long until the deadline
                    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) { std::cout << "ln59" << std::endl; rc = -1; break; }
                    int ms_until_deadline = (int)((deadline.tv_sec - now.tv_sec) * 1000l
                        + (deadline.tv_nsec - now.tv_nsec) / 1000000l);
                    if (ms_until_deadline < 0) { std::cout << "ln62" << std::endl; rc = 0; break; }
                    // Wait for connect to complete (or for the timeout deadline)
                    struct pollfd pfds[] = { {.fd = sockfd, .events = POLLOUT } };
                    rc = poll(pfds, 1, ms_until_deadline);
                    // If poll 'succeeded', make sure it *really* succeeded
                    if (rc > 0) {
                        int error = 0; socklen_t len = sizeof(error);
                        int retval = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);
                        if (retval == 0) errno = error;
                        if (error != 0) rc = -1;
                    }
                }
                // If poll was interrupted, try again.
                while (rc == -1 && errno == EINTR);
                // Did poll timeout? If so, fail.
                if (rc == 0) {
                    errno = ETIMEDOUT;
                    rc = -1;
                }
            }
        }
    } while (0);
    // Restore original O_NONBLOCK state
    if (fcntl(sockfd, F_SETFL, sockfd_flags_before) < 0) {return -1; }
    // Success
    return rc;
}

void SimpleRenderer::punch(sockaddr_in sendSockAddr, std::future<void> futureObj) {

    char msg[10];
    int i;
    while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {

        //std::cout << "bang" << std::endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, "BANG");
        if (sendto(SimpleRenderer::udpSd, (char*)msg, sizeof(msg), 0, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == -1) {

            std::cout << "failed to punch" << std::endl;

        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    close(udpSd);
    close(tcpSd);
    //yyn = true;
    std::cout << "pover" << std::endl;

}

struct sockaddr_in SimpleRenderer::smt() {
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in loopback;

    if (sock == -1) {
        std::cerr << "Could not socket\n";
    }

    memset(&loopback, 0, sizeof(loopback));
    loopback.sin_family = AF_INET;
    loopback.sin_addr.s_addr = 1337;   // can be any IP address
    loopback.sin_port = htons(9999);      // using debug port

    if (connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback)) == -1) {
        close(sock);
        std::cerr << "Could not connect\n";
    }

    socklen_t addrlen = sizeof(loopback);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&loopback), &addrlen) == -1) {
        close(sock);
        std::cerr << "Could not getsockname\n";
    }

    close(sock);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &loopback.sin_addr, buf, INET_ADDRSTRLEN) == 0x0) {
        std::cerr << "Could not inet_ntop\n";
    }
    else {
        std::cout << "Local ip address: " << buf << "\n";
    }

    return loopback;

}

void SimpleRenderer::rcv(int clientSd) {
    char msg[1500];
    while (1)
    {
        //std::cout << "Awaiting server response..." << std::endl;
        memset(&msg, 0, sizeof(msg));//clear the buffer
        int a = recv(clientSd, (char*)&msg, sizeof(msg), 0);

        if (a == -1) {
            std::cout << "cant recv" << std::endl;
            std::cout << "lost connection" << std::endl;
            td = ".lost connection";
            yon = true;
            break;
        }
        if (a == 0) {

            std::cout << "lost connection" << std::endl;
            td = "lost connection";
            yon = true;
            break;

        }

        std::cout << ">someone: " << msg << std::endl;

        if (!strcmp(msg, "exit"))
        {
            std::cout << "someone has quit the session" << std::endl;
            td = "someone has quit the session";
            yon = true;
            break;
        }
        std::string sr, srr;
        for (int i = 0; i < strlen(msg); i++) {

            sr.push_back(msg[i]);

        }
        srr = ">someone: ";
        td = srr + sr;
        yon = true;
        if ((sr.find(".txt") != std::string::npos) || (sr.find(".doc") != std::string::npos) || (sr.find(".docx") != std::string::npos) ||
            (sr.find(".xlsx") != std::string::npos) || (sr.find(".cpp") != std::string::npos) || (sr.find(".c") != std::string::npos) || (sr.find(".pptx") != std::string::npos)
            || (sr.find(".pdf") != std::string::npos) || (sr.find(".png") != std::string::npos) || (sr.find(".jpg") != std::string::npos))
        {
            std::cout << "receiving file.." << std::endl;
            td = "receiving file..";
            yon = true;
            memset(&msg, 0, sizeof(msg));
            recv(clientSd, (char*)&msg, sizeof(msg), 0);
            std::cout << "size: " << msg << "bytes" << std::endl;
            int i = 0;
            char* buffer = new char[atoi(msg)];
            while (i < atoi(msg)) {
                const int l = recv(clientSd, &buffer[i], std::min(4096, atoi(msg) - i), 0);
                if (l < 0) { std::cout << "bs" << std::endl; } // this is an error
                i += l;
            }
            std::cout << "file received " << i << " bytes" << std::endl;
            td = "file received " + std::to_string(i) + " bytes";
            yon = true;
            std::ofstream file(sr, std::ios::binary);
            file.write(buffer, atoi(msg));
            delete[] buffer;
            file.close();
            std::cout << "yay" << std::endl;
            break;
        }
     }

}

void SimpleRenderer::pong() {

    char svmsg[15];
    recv(clientSd, (char*)&svmsg, sizeof(svmsg), 0);
    std::string sr;
    for (int i = 0; i < strlen(svmsg); i++) {

        sr.push_back(svmsg[i]);

    }
    if (sr.find("PING") != std::string::npos) {

        send(clientSd, (char*)&svmsg, sizeof(svmsg), 0);

    }
}

void SimpleRenderer::sts(const char* st) {
    SimpleRenderer::dt = std::string(st);
    std::cout << ">me: " << SimpleRenderer::dt << std::endl;
}

const char* SimpleRenderer::rts() {
    const char* a = SimpleRenderer::td.c_str();
    return a;
}

bool SimpleRenderer::yn() {
    bool a = SimpleRenderer::yon;
    return a;
}

void SimpleRenderer::ync() {
    SimpleRenderer::yon = false;
}

void SimpleRenderer::snd(int tcpSd1) {
    char msg[1500];
    std::string data, hd;
    while (1)
    {

        memset(&msg, 0, sizeof(msg));//clear the buffer

        if (data != SimpleRenderer::dt) {
            data = SimpleRenderer::dt;
            strcpy(msg, (data).c_str());
            if (data == "exit")
            {
                send(tcpSd1, (char*)&msg, strlen(msg), 0);
                break;
            }
            if ((data.find(".txt") != std::string::npos) || (data.find(".doc") != std::string::npos) || (data.find(".docx") != std::string::npos) ||
                (data.find(".xlsx") != std::string::npos) || (data.find(".cpp") != std::string::npos) || (data.find(".c") != std::string::npos) || (data.find(".jpg") != std::string::npos)
                || (data.find(".pptx") != std::string::npos) || (data.find(".pdf") != std::string::npos) || (data.find(".png") != std::string::npos))
            {
                std::ifstream f1;
                std::string drtry;
                while (1) {
                    std::cout << "Directory: ";
                    SimpleRenderer::td = "Directory: ";
                    SimpleRenderer::yon = true;
                    while (1) {
                        if (data == SimpleRenderer::dt) {
                            usleep(50000);
                        }
                        else {
                            break;
                        }
                    }
                    f1.open(SimpleRenderer::dt + data, std::ios::binary);
                    if (f1.is_open()) {
                        send(tcpSd1, (char*)&msg, strlen(msg), 0);
                        std::cout << "11" << std::endl;
                        f1.seekg(0, std::ios::end);
                        int s1 = f1.tellg();
                        f1.seekg(0, std::ios::beg);
                        char* buffer = new char[s1];
                        f1.read(buffer, s1);
                        f1.close();
                        memset(&msg, 0, sizeof(msg));
                        strcpy(msg, (std::to_string(s1)).c_str());
                        send(tcpSd1, (char*)&msg, strlen(msg), 0);
                        usleep(200000);
                        std::cout << "size: " << msg << std::endl;
                        int i = 0;
                        while (i < s1) {
                            const int l = send(tcpSd1, &buffer[i], std::min(4096, s1 - i), 0);
                            if (l < 0) { std::cout << "bs" << std::endl; } // this is an error
                            i += l;
                        }
                        delete[] buffer;
                        std::cout << "file sent " << i << " bytes" << std::endl;
                        break;
                    }
                    else {
                        std::cout << "No such file or directory  " << std::endl;
                        std::cout << "File name: ";
                        getline(std::cin, data);
                        if (data == "exit") {
                            break;
                        }
                    }

                }

            }
            else if (send(tcpSd1, (char*)&msg, strlen(msg), 0) == -1) {

                std::cout << "didn't send through" << std::endl;
                SimpleRenderer::td = "didn't send through";
                SimpleRenderer::yon = true;
            }

        }
        else {
            usleep(1000);
        }

    }

}

void SimpleRenderer::cnect(const char* ip) {

    const char* serverIp = ip; int svport = 11111;
    //setup a socket and connection tools 
    struct hostent* svhost = gethostbyname(serverIp);
    sockaddr_in svAddr;
    bzero((char*)&svAddr, sizeof(svAddr));
    svAddr.sin_family = AF_INET;
    svAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*svhost->h_addr_list));
    svAddr.sin_port = htons(svport);
    clientSd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSd == -1) {
        std::cout << "cantsocket" << std::endl;
    }

    if (connect(clientSd, (sockaddr*)&svAddr, sizeof(svAddr)) < 0) {
        std::cout << "cant connect to server, try again later maybe" << std::endl;
        td = "cant connect to server, try again later maybe";
        yon = true;
        return;
    }
    else {
        yyn = true;
    }

    td = "..waiting for server";
    yon = true;
}

void SimpleRenderer::SSS(const char* aa) {



    char svmsg[50], svmsg1[30], svmsg2[10], svmsg3[10], svmsg4[50];
    sockaddr_in sendSockAddr, myAddr;

    tcpSd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSd == -1) {
        std::cout << "canttcpsocket" << std::endl;
        return;
    }

    const int opt = 1;
    if (setsockopt(tcpSd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cout << "prblm" << std::endl;
    }

    const char* tgtip = aa; char abb[INET_ADDRSTRLEN];
    memset(&svmsg, 0, sizeof(svmsg));//clear the buffer
    strcpy(svmsg, tgtip);
    send(clientSd, (char*)&svmsg, strlen(svmsg), 0);
    memset(&svmsg4, 0, sizeof(svmsg4));
    sockaddr_in fm = smt();
    strcpy(svmsg4, inet_ntop(AF_INET, &(fm.sin_addr.s_addr), abb, INET_ADDRSTRLEN));
    sleep(1);
    send(clientSd, (char*)svmsg4, sizeof(svmsg4), 0);
    bzero((char*)&fm, sizeof(fm));
    int sport; int rport;
    memset(&svmsg1, 0, sizeof(svmsg1));
    memset(&svmsg2, 0, sizeof(svmsg2));
    memset(&svmsg3, 0, sizeof(svmsg3));
    pong();
    int f1, f2, f3;
    f1 = recv(clientSd, (char*)&svmsg1, sizeof(svmsg1), 0);
    if (f1 <= 0) {
        std::cout << "didntrcv" << std::endl;
    }
    const char* pt0 = svmsg1;
    std::cout << svmsg1 << "(bytes:" << f1 << ")" << std::endl;
    std::cout << pt0 << std::endl;


    f2 = recv(clientSd, (char*)&svmsg2, sizeof(svmsg2), 0);
    if (f2 <= 0) {
        std::cout << "didntrcv" << std::endl;
    }

    const char* pt = svmsg2;
    std::cout << svmsg2 << "(bytes:" << f2 << ")" << std::endl;
    std::cout << pt << std::endl;


    f3 = recv(clientSd, (char*)&svmsg3, sizeof(svmsg3), 0);
    if (f3 <= 0) {
        std::cout << "didntrcv" << std::endl;
        return;
    }

    const char* pt2 = svmsg3;
    std::cout << svmsg3 << "(bytes:" << f3 << ")" << std::endl;
    std::cout << pt2 << std::endl;

    //create a message buffer 
    char msg[1500]; sport = atoi(pt); rport = atoi(pt2);
    //setup a socket and connection tools 
    struct hostent* host = gethostbyname(pt0);

    socklen_t ssz = sizeof(sendSockAddr);
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(sport);

    bzero((char*)&myAddr, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(rport);

    udpSd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSd == -1) {
        std::cout << "cantsocket" << std::endl;
        return;
    }

    if (setsockopt(udpSd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cout << "prblm2" << std::endl;
    }

    if (bind(udpSd, (struct sockaddr*)&myAddr, sizeof(myAddr)) < 0) {
        std::cerr << "cantbind, maybe try another port" << std::endl;
        td = "cantbind, maybe try another port";
        yon = true;
        return;
    }

    std::promise<void> exitSignal1;
    std::future<void> futureObj1 = exitSignal1.get_future();

    std::thread t1;
    t1 = std::thread(&SimpleRenderer::punch, this, sendSockAddr, std::move(futureObj1));
    std::cout << "punching.." << std::endl;
    td = "..connecting";
    yon = true;
    bool flg1 = false;
    bool flg2 = false;
    /*while (1) {
        memset(&msg, 0, sizeof(msg));//clear the buffer
        if (recv(SimpleRenderer::udpSd, (char*)msg, sizeof(msg), 0) != -1) {
            std::cout << "the other side: " << msg << std::endl;
            if (!strcmp(msg, "BANG")) {
                std::cout << "THE HOLE's HERE, telling others.." << std::endl;
                sendto(SimpleRenderer::udpSd, "WE GOT THE HOLE", 15, 0, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
                flg1 = true;
            }
            if (!strcmp(msg, "WE GOT THE HOLE")) {
                std::cout << "punching done" << std::endl;
                exitSignal1.set_value();
                t1.join();
                flg2 = true;
            }
            if (flg1 == true && flg2 == true) {
                std::cout << "hole's ready" << std::endl;
                break;
            }
        }
        else {
            std::cout << "cant recv" << std::endl;
            exitSignal1.set_value();
            t1.join();
            break;
        }
    }*/

    std::thread t2;

    if (bind(tcpSd, (struct sockaddr*)&myAddr, sizeof(myAddr)) == -1) {
        std::cout << "cantbindtcp" << std::endl;
        td = "cantbindtcp";
        yon = true;
        return;
    }

    bool xc = false;

    if (connect(tcpSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)) == -1) {

        std::cout << errno << std::endl;
        close(tcpSd);

        std::cout << "cantconnect, retrying once.." << std::endl;
        tcptd[0] = socket(AF_INET, SOCK_STREAM, 0);
        if (tcptd[0] == -1) {
            std::cout << "canttcpsocket" << std::endl;
        }
        if (setsockopt(tcptd[0], SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cout << "prblm" << std::endl;
        }
        if (bind(tcptd[0], (struct sockaddr*)&myAddr, sizeof(myAddr)) == -1) {
            std::cout << "cantbindtcp" << std::endl;
            return;
        }
        if (connect_with_timeout(tcptd[0], (sockaddr*)&sendSockAddr, sizeof(sendSockAddr), 100) == -1){

            std::cout << errno << std::endl;
            std::cout << "cantconnect, retrying twice.." << std::endl;
            tcptd[1] = socket(AF_INET, SOCK_STREAM, 0);
            if (tcptd[1] == -1) {
                std::cout << "canttcpsocket" << std::endl;
            }
            if (setsockopt(tcptd[1], SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                std::cout << "prblm" << std::endl;
            }
            if (bind(tcptd[1], (struct sockaddr*)&myAddr, sizeof(myAddr)) == -1) {
                std::cout << "cantbindtcp" << std::endl;
                return;
            }
            if (connect_with_timeout(tcptd[1], (sockaddr*)&sendSockAddr, sizeof(sendSockAddr), 4000) != -1){
                xc = true;
            }
        }
        else {
            xc = true;
        }
        if(xc == false){

            std::cout << errno << std::endl;
            std::cout << "cantconnect, relay" << std::endl;
        }

    }

    exitSignal1.set_value();
    t1.join();

    if (xc == true) {

        std::cout << "hole's ready" << std::endl;
        std::cout << "connected" << std::endl;
        td = "connected";
        yon = true;
        std::string data = "punchedthrough";
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, (data).c_str());
        send(clientSd, (char*)&msg, strlen(msg), 0);
        memset(&msg, 0, sizeof(msg));//clear the buffer
        close(clientSd);

    }
    else {

        std::cout << "relaying" << std::endl;
        td = "relaying";
        yon = true;
        std::string data = "punchedfail";
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, (data).c_str());
        send(clientSd, (char*)&msg, strlen(msg), 0);
        memset(&msg, 0, sizeof(msg));//clear the buffer
        close(tcpSd);
        tcpSd = clientSd;

    }

    t2 = std::thread(&SimpleRenderer::rcv, this, tcpSd);
    snd(tcpSd);
    t2.join();

    close(tcpSd);

    close(udpSd);
    std::cout << "********Session********" << std::endl;
    std::cout << "Connection closed" << std::endl;
    td = "********Session********\nConnection closed";
    yon = true;
}
