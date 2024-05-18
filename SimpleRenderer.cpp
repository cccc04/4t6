//
// This file is used by the template to render a basic scene using GL.
//
//#include "pch.h"

#include "SimpleRenderer.h"
//#include "MathHelper.h"

// These are used by the shader compilation methods.
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
    tcpSd = socket(AF_INET6, SOCK_STREAM, 0);
    if (tcpSd == -1) {
        std::cout << "canttcpsocket" << std::endl;
    }
    futureObjPong = exitSignalPong.get_future();
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

void SimpleRenderer::punch(sockaddr sendSockAddr, std::future<void> futureObj) {

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

sockaddr_in6 SimpleRenderer::smt() {
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    sockaddr_in6 loopback;

    if (sock == -1) {
        std::cerr << "Could not socket\n";
    }

    memset(&loopback, 0, sizeof(loopback));
    loopback.sin6_family = AF_INET6;
    std::fill_n(loopback.sin6_addr.s6_addr, 0, 16);  // can be any IP address
    loopback.sin6_port = htons(9999);      // using debug port

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

    char buf[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET6, &loopback.sin6_addr, buf, INET6_ADDRSTRLEN) == 0x0) {
        std::cerr << "Could not inet_ntop\n";
    }
    else {
        std::cout << "Local ip address: " << buf << "\n";
    }

    return loopback;

}

bool SimpleRenderer::checkalive() {
    if (!pongt.valid()) {
        return false;
    }
    if (pongt.wait_for(std::chrono::microseconds(1)) == std::future_status::ready) {
        if (futureObjPong.valid() && (futureObjPong.wait_for(std::chrono::microseconds(1)) == std::future_status::ready)) {
            exitSignalPong = std::promise<void>{};
            futureObjPong = exitSignalPong.get_future();
        }
        std::cout << "162: Pinger dead" << std::endl;
        td = ".lost connection";
        yon = true;
        return false;
    }
    return true;
}

void SimpleRenderer::rcv(int sock) {

    std::string sr, srr;
    while (1)
    {
        //std::cout << "Awaiting server response..." << std::endl;
        //memset(&msg, 0, sizeof(msg));//clear the buffer
        while (gmsg.empty()) {
            if (checkalive() == true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            else {
                return;
            }
        }

        if (1) {
            std::lock_guard<std::mutex> guard(mutexpo);
            sr = gmsg.front();
            gmsg.pop_front();
        }

        if (sr == "Z exit")
        {
            std::cout << "someone has quit the session" << std::endl;
            td = "someone has quit the session";
            yon = true;
            break;
        }
        size_t pos = 2;
        srr = ">someone: ";
        td = srr + sr.substr(pos, sr.size() - pos);
        yon = true;
     }

}

bool SimpleRenderer::pong(int sock, bool np, bool rd) {
    char msg[1500], msgp[5], msgr[128];


    std::string es = "PONG";
    strcpy(msgp, es.c_str());
    size_t asize = 2;
    std::string os;

    if (np == false) {
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            std::cout << "331: tmotf1" << std::endl;
            return false;
        }
        if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
            std::cout << "335: tmotf2" << std::endl;
            return false;
        }
        std::cout << "338: " << std::endl;
    }
    while (futureObjPong.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        std::string s, srr;
        int i;
        if (np == true && rd == true) {
            while (gmsg.empty()) {
                if (checkalive() == true) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
                else {
                    return false;
                }
            }

            if (1) {
                std::lock_guard<std::mutex> guard(mutexpo);
                s = gmsg.front();
                gmsg.pop_front();
            }

            td = srr + s.substr(asize, s.size() - asize);
            yon = true;
        }
        else if (rd == false || np == false) {

            memset(&msg, 0, sizeof(msg));
            i = recv(sock, (char*)&msg, sizeof(msg), 0);
            if (i <= 0) {
                std::cout << "251: ping rcv t/o" << std::endl;
                return false;
            }
            for (int j = 0; j < strlen(msg); j++) {
                s.push_back(msg[j]);
            }
            if (s == "PING") {
                //std::cout << "367: ping rcved" << std::endl;
                std::lock_guard<std::mutex> guard(mutexpi);
                if (send(sock, (char*)&msgp, sizeof(msgp), 0) < 0) {
                    std::cout << "257: f snd" << std::endl;
                    return false;
                }
                //std::cout << "372: ping sent" << std::endl;
            }
            else if (s.find("Z ") == 0) {
                //std::cout << "258: " << s << std::endl;
                if (np == true) {
                    if (s.find("Z exit") == 0)
                    {
                        std::cout << "someone has quit the session" << std::endl;
                        td = "someone has quit the session";
                        yon = true;
                        return true;
                    }
                    size_t pos = 2;
                    srr = ">someone: ";
                    td = srr + s.substr(pos, s.size() - pos);
                    yon = true;
                }
                else {
                    std::lock_guard<std::mutex> guard(mutexpo);
                    gmsg.push_back(s);
                }
            }
            else if (s.find("gs:") == 0) {
                std::string ss;
                for (int i = 3; i < strlen(msg); i++) {
                    ss.push_back(msg[i]);
                }
                std::cout << "267: " << ss << std::endl;
                if (ss != "rtk") {
                    os = ss;
                    memset(&msgr, 0, sizeof(msgr));
                    snprintf(msgr, sizeof(msgr), "%zu", ss.size());
                    std::lock_guard<std::mutex> guard(mutexpi);
                    if (send(sock, (char*)msgr, sizeof(msgr), 0) < 0) {
                        std::cout << "269: f snd" << std::endl;
                        return false;
                    }
                }
                else {
                    std::lock_guard<std::mutex> guard(mutexpo);
                    gmsg.push_back(os);
                }
            }
            else if (s.find("xf") == 0) {
                std::cout << "receiving file.." << std::endl;
                td = "receiving file..";
                yon = true;
                memset(&msg, 0, sizeof(msg));
                recv(sock, (char*)&msg, sizeof(msg), 0);
                std::cout << "size: " << msg << "bytes" << std::endl;
                int i = 0;
                char* buffer = new char[atoi(msg)];
                while (i < atoi(msg)) {
                    const int l = recv(sock, &buffer[i], std::min(4096, atoi(msg) - i), 0);
                    if (l < 0) { std::cout << "bs" << std::endl; } // this is an error
                    i += l;
                }
                std::cout << "file received " << i << " bytes" << std::endl;
                td = "file received " + std::to_string(i) + " bytes";
                yon = true;
                std::ofstream file(s.substr(asize, s.size() - asize), std::ios::binary);
                file.write(buffer, atoi(msg));
                delete[] buffer;
                file.close();
                std::cout << "yay" << std::endl;
            }
            else {
                std::cout << "282: " << s << std::endl;
            }        
        }
    }
    return true;
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
    std::cout << "462: called snd" << std::endl;
    char msg[1500];
    std::string data = "Z ";
    std::string adata;
    size_t size = 2;
    while (1)
    {

        memset(&msg, 0, sizeof(msg));//clear the buffer

        if (data != ("Z " + SimpleRenderer::dt)) {
            data = "Z " + SimpleRenderer::dt;
            adata = SimpleRenderer::dt;
            strcpy(msg, (data).c_str());
            if (data == "Z exit")
            {   
                std::lock_guard<std::mutex> guard(mutexpi);
                send(tcpSd1, (char*)&msg, strlen(msg), 0);
                break;
            }
            if ((data.find(".txt") != std::string::npos) || (data.find(".doc") != std::string::npos) || (data.find(".docx") != std::string::npos) ||
                (data.find(".xlsx") != std::string::npos) || (data.find(".cpp") != std::string::npos) || (data.find(".c") != std::string::npos) || 
                (data.find(".jpg") != std::string::npos)  || (data.find(".pptx") != std::string::npos) || (data.find(".pdf") != std::string::npos) || 
                (data.find(".png") != std::string::npos))
            {
                std::ifstream f1;
                std::string drtry;
                while (1) {
                    std::cout << "Directory: ";
                    SimpleRenderer::td = "Directory: ";
                    SimpleRenderer::yon = true;
                    while (1) {
                        if (adata == SimpleRenderer::dt) {
                            usleep(50000);
                        }
                        else {
                            break;
                        }
                    }
                    f1.open(SimpleRenderer::dt + adata, std::ios::binary);
                    if (f1.is_open()) {
                        memset(&msg, 0, sizeof(msg));
                        strcpy(msg, ("xf" + adata).c_str());
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
                        std::cout << "File name: " << std::endl;
                        adata = SimpleRenderer::dt;
                        while (1) {
                            if (adata == SimpleRenderer::dt) {
                                usleep(50000);
                            }
                            else {
                                adata = SimpleRenderer::dt;
                                break;
                            }
                        }
                        if (adata == "exit") {
                            break;
                        }
                    }

                }

            }
            else if ([&] {std::lock_guard<std::mutex> guard(mutexpi); bool b; b = (send(tcpSd1, (char*)&msg, strlen(msg), 0) == -1); return b;}()) {

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

bool SimpleRenderer::cnect(const char* ip, const char* port, int &sock, int tcud, bool cnnct, int timeoutms) {

    if (tcud != SOCK_DGRAM && tcud != SOCK_STREAM) {
        std::cout << "invalid tcud" << std::endl;
        return false;
    }
    //setup a socket and connection tools 
    struct addrinfo  hints;
    struct addrinfo* result;
    //sockaddr6_in svAddr;
    bzero((char*)&hints, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = tcud;
    //hints.ai_protocol = 0;
    if (cnnct == false){
        ip = NULL;
        hints.ai_canonname = NULL;
        //hints.ai_addr = NULL;
        hints.ai_next = NULL;
        hints.ai_flags = AI_PASSIVE;
    }
    if (getaddrinfo(ip, port, &hints, &result) != 0) {
        std::cout << "invalid ip/port.(only ipv6 accepted)" << std::endl;
        return false;
    }
    for (struct addrinfo* rp = result; rp != NULL; rp = rp->ai_next) {
        int i = socket(rp->ai_family, rp->ai_socktype, 0);
        if (i == -1)
            continue;

        if (cnnct == true) {
            sock = i;
            if (connect_with_timeout(sock, rp->ai_addr, rp->ai_addrlen, timeoutms) < 0) {
                std::cout << "cant connect to server, try again later maybe" << std::endl;
                td = "cant connect to server, try again later maybe";
                yon = true;
                return false;
            }
            else {
                td = "..waiting for server";
                yon = true;
                return true;
            }
        }
        else {
            if (bind(sock, rp->ai_addr, rp->ai_addrlen) < 0) {
                std::cerr << "393: cantbind, maybe try another port" << std::endl;
                td = "cantbind, maybe try another port";
                yon = true;
                return false;
            }
            else {
                return true;
            }
        }
        close(sock);
    }
    return false;

    /*svAddr.sin6_addr.s6_addr = inet_addr(inet_ntoa(*(struct in6_addr*)*svhost->h_addr_list));
    svAddr.sin6_port = htons(svport);
    clientSd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSd == -1) {
        std::cout << "cantsocket" << std::endl;
    }

    if (connect(clientSd, (sockaddr*)&svAddr, sizeof(svAddr)) < 0) {
        std::cout << "cant connect to server, try again later maybe" << std::endl;
        td = "cant connect to server, try again later maybe";
        yon = true;
        return false;
    }
    else {
        td = "..waiting for server";
        yon = true;
        return true;
    }*/
}

void SimpleRenderer::SSS(const char* aa) {



    char svmsg[50], svmsg1[64], svmsg2[10], svmsg3[10], svmsg4[64];
    //sockaddr_in sendSockAddr, myAddr;
    const char* tgtip = aa; char abb[INET6_ADDRSTRLEN];
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(clientSd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cout << "441: prblm2" << std::endl;
    }
    memset(&svmsg, 0, sizeof(svmsg));//clear the buffer
    strcpy(svmsg, tgtip);
    send(clientSd, (char*)&svmsg, sizeof(svmsg), 0);
    memset(&svmsg4, 0, sizeof(svmsg4));
    sockaddr_in6 fm = smt();
    /*strcpy(svmsg4, inet_ntop(AF_INET6, &(fm.sin6_addr.s6_addr), abb, INET6_ADDRSTRLEN));
    sleep(1);
    send(clientSd, (char*)svmsg4, sizeof(svmsg4), 0);
    bzero((char*)&fm, sizeof(fm));*/
    pongt = std::async(&SimpleRenderer::pong, this, clientSd, false, true);

    std::vector<std::string> pt1;
    /*while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }*/
    for (int i = 0; i < 4; i++) {
        while (gmsg.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        std::lock_guard<std::mutex> guard(mutexpo);
        pt1.push_back(gmsg.front());
        gmsg.pop_front();
        std::cout << pt1[i] << std::endl;
    }

    //create a message buffer 
    char msg[1500];
    //setup a socket and connection tools 
    /*socklen_t ssz = sizeof(sendSockAddr);
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin6_family = AF_INET;
    sendSockAddr.sin6_addr.s6_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(sport);*/

    struct addrinfo  hints1;
    struct addrinfo* sendAd;
    //sockaddr6_in svAddr;
    bzero((char*)&hints1, sizeof(hints1));
    hints1.ai_family = AF_INET6;
    hints1.ai_socktype = SOCK_STREAM;
    //hints.ai_protocol = 0;
    hints1.ai_next = NULL;
    if (getaddrinfo(pt1[0].c_str(), std::to_string(stoi(pt1[1]) - 100).c_str(), &hints1, &sendAd) != 0) {
        std::cout << "gai 505" << std::endl;
    }

    int udpSd1 = socket(AF_INET6, SOCK_DGRAM, 0);
    udpSd = socket(AF_INET6, SOCK_DGRAM, 0);

    const int opt = 1;

    if (setsockopt(udpSd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cout << "prblm2" << std::endl;
    }
    std::cout << "punching.." << std::endl;
    td = "..connecting";
    yon = true;
    int iii = 0;
    if (cnect(NULL, std::to_string(stoi(pt1[2]) - 100).c_str(), udpSd1, SOCK_DGRAM, false) == false) {
        std::cout << "dintbind" << std::endl;
    }
    while ( cnect(pt1[0].c_str(), std::to_string(stoi(pt1[1]) - 100).c_str(), udpSd, SOCK_DGRAM) == false || iii < 5) {
        std::cout << "cantsocket" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        iii++;
        //return;
    }

    struct sockaddr_storage peer_addr;
    socklen_t peer_addrlen;
    std::future<void> thread = std::async([&] {
        std::string string;
        while (string != "ACK")
        { 
            char mssg[5];
            string.clear();
            std::cout << "630: here" << std::endl;
            peer_addrlen = sizeof(peer_addr);
            if (recvfrom(udpSd1, (char*)&mssg, sizeof(mssg), 0, (sockaddr*)&peer_addr, &peer_addrlen) < 0)
            { 
                std::cout << "625: -1" << std::endl;
            }
            else {
                std::cout << "634: " << mssg << std::endl;
                for (int jj = 0; jj < strlen(mssg); jj++) {
                    string.push_back(mssg[jj]);
                }
            }
        }
        return;
    });
    std::string ack = "ACK";
    do {
        char mssg[5];
        strcpy(mssg, ack.c_str());
        int iiii = send(udpSd, (char*)mssg, sizeof(mssg), 0);
        if (iiii < 0) {
            std::cout << "643: not send" << std::endl;
        }
        else std::cout << "647: " << iiii << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    } while (thread.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout);

    /*std::promise<void> exitSignal1;
    std::future<void> futureObj1 = exitSignal1.get_future();

    std::thread t1;
    sockaddr sendSockAddr = *(sendAd->ai_addr);
    t1 = std::thread(&SimpleRenderer::punch, this, sendSockAddr, std::move(futureObj1));
    bool flg1 = false;
    bool flg2 = false;
    while (1) {
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





    int tcptd;
    bool xc;
    for (int q = 0; q < 2; q++) {
        xc = false;
        if (pt1[3] == "sv") {
            sockaddr_in6 servAddr;
            bzero((char*)&servAddr, sizeof(servAddr));
            int innt = stoi(pt1[2]) - 100;
            servAddr.sin6_family = AF_INET6;
            servAddr.sin6_addr = in6addr_any;
            servAddr.sin6_port = htons(innt);
            std::cout << "578: " << innt << std::endl;

            int serverSd = socket(AF_INET6, SOCK_STREAM, 0);
            if (serverSd < 0)
            {
                std::cerr << "Error establishing the server socket" << std::endl;
            }
            const int opt = 1;
            if (setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                std::cout << "prblm" << std::endl;
            }
            //bind the socket to its local address
            int bindStatus = bind(serverSd, (struct sockaddr*)&servAddr, sizeof(servAddr));
            if (bindStatus < 0)
            {
                std::cerr << "Error binding socket to local address" << std::endl;
            }
            else if(serverSd >= 0) {
                listen(serverSd, 1);

                sockaddr_in6 newSockAddr;
                bzero((char*)&newSockAddr, sizeof(newSockAddr));
                socklen_t newSockAddrSize = sizeof(newSockAddr);
                int newSd;
                auto acpt = [&] {
                    while (1) {
                        newSd = accept(serverSd, (sockaddr*)&newSockAddr, &newSockAddrSize);
                        if (newSd < 0)
                        {
                            std::cerr << "Error accepting request from client!" << std::endl;
                            return false;
                        }
                        char ccstr[INET6_ADDRSTRLEN];
                        std::string string = inet_ntop(AF_INET6, &(newSockAddr.sin6_addr.s6_addr), ccstr, INET6_ADDRSTRLEN);
                        std::cout << string << ":" << ntohs(newSockAddr.sin6_port) << std::endl;
                        if (string == pt1[0]) {
                            std::cout << "Connected with client!" << std::endl;
                            int yes = 1;
                            if (setsockopt(newSd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) == -1) {
                                std::cout << "KAL fail" << std::endl;
                            }
                            tcpSd = newSd;
                            return true;
                        }

                        //close(newSd);
                    }
                    return false;
                };
                std::future<bool> thread = std::async(acpt);
                int x = 0;
                while (gmsg.empty() && (thread.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout)) {
                    if (checkalive()) {
                        if (x == 5) {
                            x = 0;
                        }
                        x++;
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    }
                    else {
                        std::cout << "626: not alive" << std::endl;
                        xc = false;
                        close(serverSd);
                        if (thread.valid()) {
                            thread.wait();
                        }
                        break;
                    }
                }
                if (thread.wait_for(std::chrono::microseconds(1)) == std::future_status::ready && thread.get() == true) {
                        std::cout << "635: joinable" << std::endl;
                        tcpSd = newSd;
                        xc = true;
                        break;
                }
                else {
                    close(serverSd);
                    if (thread.valid()) {
                        thread.wait();
                    }
                }
            }
            if (q == 0 ) {
                    std::lock_guard<std::mutex> guard(mutexpo);
                    std::string srrr = gmsg.front();
                    gmsg.pop_front();
                    std::cout << "765: " << srrr << std::endl;
                    if (srrr == "cl") {
                        pt1[3] = srrr;
                        continue;
                    }
                    else {
                        std::cout << "649: no idea what happened here";
                    }
            }
            xc = false;
            break;
            

        }
        else if (pt1[3] == "cl") {
            //const int opt = 1;
            /**/if (setsockopt(tcpSd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                std::cout << "prblm" << std::endl;
            }

            /*if (cnect(pt1[0].c_str(), std::to_string((stoi(pt1[2]) - 100)).c_str(), tcpSd, SOCK_STREAM, false) == false) {
                std::cout << "cantbindtcp" << std::endl;
                td = "cantbindtcp";
                yon = true;
                return;
            }*/
            xc = cnect(pt1[0].c_str(), std::to_string((stoi(pt1[1]) - 100)).c_str(), tcpSd, SOCK_STREAM);
            if ( xc == false) {

                std::cout << errno << std::endl;
                close(tcpSd);

                std::cout << "cantconnect, retrying once.." << std::endl;
                tcptd = socket(AF_INET6, SOCK_STREAM, 0);
                /**/if (setsockopt(tcptd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                    std::cout << "prblm" << std::endl;
                }
                else {
                    xc = cnect(pt1[0].c_str(), std::to_string((stoi(pt1[1]) - 100)).c_str(), tcptd, SOCK_STREAM, true, 1000);
                }
                if ( xc == false) {
                    std::cout << errno << std::endl;
                    close(tcptd);
                    std::cout << "cantconnect" << std::endl;
                }
                else {
                    tcpSd = tcptd;
                }
            }
            if (xc == false) {
                if (q == 0) {
                    memset(&msg, 0, sizeof(msg));
                    std::string qs = "fu:";
                    strcpy(msg, qs.c_str());

                    if (1) {
                        std::lock_guard<std::mutex> guard(mutexpi);
                        send(clientSd, (char*)&msg, strlen(msg), 0);
                    }

                    while (gmsg.empty()) {
                        if (checkalive()) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        }
                        else {
                            xc = false;
                            break;
                        }
                    }
                    std::lock_guard<std::mutex> guard(mutexpo);
                    std::string srrr = gmsg.front();
                    gmsg.pop_front();
                    std::cout << "844: " << srrr << std::endl;
                    if (srrr == "sv") {
                        pt1[3] = srrr;
                        continue;
                    }
                    else {
                        std::cout << "649: no idea what happened here";
                    }
                }
            }
            else {
                break;
            }
        }
    }
    //exitSignal1.set_value();
    //t1.join();
    std::thread t2;
    if (xc == true) {

        std::cout << "hole's ready" << std::endl;
        std::cout << "connected" << std::endl;
        td = "connected";
        yon = true;
        std::string data = "pcr:punchedthrough";
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, (data).c_str());
        if (1) {
            std::lock_guard<std::mutex> guard(mutexpi);
            send(clientSd, (char*)&msg, strlen(msg), 0);
            memset(&msg, 0, sizeof(msg));//clear the buffer
        }
        if (futureObjPong.valid() && (futureObjPong.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout)) {
            exitSignalPong.set_value();
            while (checkalive()) {
                std::cout << "still alive" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            exitSignalPong = std::promise<void>{};
            futureObjPong = exitSignalPong.get_future();
        }
        close(clientSd);
        if (pongt.valid()) {
            pongt.wait();
        }
        t2 = std::thread(&SimpleRenderer::pong, this, tcpSd, true, false);
    }
    else {

        std::cout << "relaying" << std::endl;
        td = "relaying";
        yon = true;
        std::string data = "pcr:punchedfail";
        memset(&msg, 0, sizeof(msg));//clear the buffer
        strcpy(msg, (data).c_str());
        if (1) {
            std::lock_guard<std::mutex> guard(mutexpi);
            send(clientSd, (char*)&msg, strlen(msg), 0);
            memset(&msg, 0, sizeof(msg));//clear the buffer
        }
        close(tcpSd);
        tcpSd = clientSd;
        t2 = std::thread(&SimpleRenderer::rcv, this, tcpSd);
    }

     
    snd(tcpSd);
    t2.join();

    close(tcpSd);

    close(udpSd);
    std::cout << "********Session********" << std::endl;
    std::cout << "Connection closed" << std::endl;
    td = "********Session********\nConnection closed";
    yon = true;
}
