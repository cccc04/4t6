#pragma once

#include <string>
#include <stdlib.h>
#include <future>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <list>

#ifdef __APPLE__
#include <unistd.h>
#include <sys/resource.h>

/*#else // __ANDROID__ or _WIN32
#include <GLES2/gl2.h>*/
#endif
#ifndef SIMPLERENDERER_H
#define SIMPLERENDERER_H
#endif

class SimpleRenderer
{
public:
    void SSS(const char* aa);
    void sts(const char* st);
    const char* rts();
    bool yn();
    void ync();
    bool cnect(const char* ip, const char* port, int &sock, int tcud, bool cnnct);
    SimpleRenderer();
    bool yyn;
    int clientSd;
private:
    struct sockaddr_in6 smt();
    void punch(sockaddr sendSockAddr, std::future<void> futureObj);
    bool pong(int sock);
    void snd(int tcpSd1);
    void rcv(int clientSd);
    int connect_with_timeout(int sockfd, const struct sockaddr* addr, socklen_t addrlen, unsigned int timeout_ms);
    bool yon;
    int udpSd;
    int tcpSd;
    int tcptd[200];
    std::string dt;
    std::string td;
    std::list<std::string> gmsg;
    std::mutex mutexpo;
    std::future<bool> pongt;
    std::promise<void> exitSignalPong, exitSignalRtn;
    std::future<void> futureObjPong, futureObjRtn;
};
