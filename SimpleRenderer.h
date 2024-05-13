#pragma once

#include <string>
#include <stdlib.h>
#include <future>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
    void cnect(const char* ip);
    SimpleRenderer();
    bool yyn;
private:
    struct sockaddr_in smt();
    void punch(sockaddr_in sendSockAddr, std::future<void> futureObj);
    void pong();
    void snd(int tcpSd1);
    void rcv(int clientSd);
    int connect_with_timeout(int sockfd, const struct sockaddr* addr, socklen_t addrlen, unsigned int timeout_ms);
    bool yon;
    int udpSd;
    int tcpSd;
    int clientSd;
    int tcptd[200];
    std::string dt;
    std::string td;
};
