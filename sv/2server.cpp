#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <future>
//using namespace std;
//Server side
std::vector<std::string> tmp(50);
std::vector<std::string> tmp1(50);
sockaddr_in6 tsk[50];
int newSd[50];
std::thread t[50];
int serverSd;


void rl(int newSd1, int newSd2) {
    int rc, sd;
    while (1) {

        char msg[1500];
        memset(&msg, 0, sizeof(msg));//clear the buffer
        rc = recv(newSd1, (char*)&msg, sizeof(msg), 0);
        if (rc < 0) {
            std::cout << "cant rcv1" << std::endl;
            break;
        }
        if (rc == 0) {
            std::cout << "dscnected r" << std::endl;
            memset(&msg, 0, sizeof(msg));//clear the buffer
            std::string data = "exit";
            strcpy(msg, data.c_str());
            send(newSd2, (char*)&msg, sizeof(msg), 0);
            break;
        }
        sd = send(newSd2, (char*)&msg, sizeof(msg), 0);
        if (sd < 0) {
            std::cout << "cant snd" << std::endl;
            break;
        }
        if (sd == 0) {
            std::cout << "dscnected s" << std::endl;
            break;
        }
        if (!strcmp(msg, "exit"))
        {
            std::cout << "Client has quit the session" << std::endl;
            break;
        }
 
    }
}

void input() {
    int ipu = 1;
    while (ipu != 0) {
        std::cin >> ipu;
    }
    close(serverSd);
    std::cout << "********Session********" << std::endl;
    std::cout << "Connection closed..." << std::endl;
    exit(1);
}

bool cmn(int newSd) {

    bool yn = false;
    char msg[150];
    memset(&msg, 0, sizeof(msg));//clear the buffer
    int aa = recv(newSd, (char*)&msg, sizeof(msg), 0);
    if ( aa < 0) {
        std::cout << "cant rcv" << std::endl;
    }
    std::string sr;
    for (int i = 0; i < strlen(msg); i++) {

        sr.push_back(msg[i]);

    }
    if ( aa > 0) {
            if (sr.find("punchedthrough") != std::string::npos)
            {
                std::cout << "w" << std::endl;
                yn = true;
            }
            if (sr.find("punchedfail") != std::string::npos)
            {
                std::cout << "f" << std::endl;
                yn = false;
            }
    }
    if ( aa == 0) {
            std::cout << "ept" << std::endl;
            yn = true;
    }

    return yn;
}

void idp(int i, int j) {

    bool cmn1 = cmn(newSd[i]);
    bool cmn2 = cmn(newSd[j]);
    std::cout << cmn1 << cmn2 << std::endl;
    if (cmn1 == true && cmn2 == true) {

        tmp[i].clear();
        tmp[j].clear();
        bzero((char*)&tsk[j], sizeof(tsk[j]));
        bzero((char*)&tsk[i], sizeof(tsk[i]));
        close(newSd[i]);
        close(newSd[j]);
        std::cout << "good" << std::endl;
    }
    else if(cmn1 == false && cmn2 == false){

        std::string data = "connected";
        char msg[150];
        strcpy(msg, data.c_str());
        if (send(newSd[i], (char*)&msg, sizeof(msg), 0) > 0) {
            if (send(newSd[j], (char*)&msg, sizeof(msg), 0) > 0) {

                std::thread tr(rl, newSd[i], newSd[j]);
                std::thread tr2(rl, newSd[j], newSd[i]);
                tr.join();
                tr2.join();
                std::cout << "ok" << std::endl;
            }
            else {
                std::cout << "client dscnimt" << std::endl;
            }
                
        }
        else {
            std::cout << "client dscnimt" << std::endl;
        }
        close(newSd[i]);
        close(newSd[j]);
        tmp[i].clear();
        tmp[j].clear();
        bzero((char*)&tsk[j], sizeof(tsk[j]));
        bzero((char*)&tsk[i], sizeof(tsk[i]));
        std::cout << "cler" << std::endl;

    }
    else {

        tmp[i].clear();
        tmp[j].clear();
        bzero((char*)&tsk[j], sizeof(tsk[j]));
        bzero((char*)&tsk[i], sizeof(tsk[i]));
        close(newSd[i]);
        close(newSd[j]);
        std::cerr << "shit happened" << std::endl;

    }

}

void syc(int newSd, char msg[], char* msg1[], char* msg2[], sockaddr_in tsk) {

    int a, a1, a2;
    do {
        a = sendto(newSd, (char*)msg, sizeof(msg), 0, (sockaddr*)&tsk, sizeof(tsk));
        std::cout << msg << "(bytes:" << a << ")" << std::endl;
    } while (a <= 5);
    usleep(300000);
    do {
        a1 = sendto(newSd, (char*)msg1, sizeof(msg1), 0, (sockaddr*)&tsk, sizeof(tsk));
        std::cout << msg1 << "(bytes:" << a1 << ")" << std::endl;
    } while (a1 <= 2);
    usleep(300000);
    do {
        a2 = sendto(newSd, (char*)msg2, sizeof(msg2), 0, (sockaddr*)&tsk, sizeof(tsk));
        std::cout << msg2 << "(bytes:" << a2 << ")" << std::endl;
    } while (a2 <= 2);

}

int lsn(int j, sockaddr_in6 newSockAddr) {

    char msg[50], msg6[51];
    memset(&msg, 0, sizeof(msg));

    if (recv(newSd[j], (char*)&msg, sizeof(msg), 0) > 0) {

        
        std::cout << "Client: " << msg << std::endl;
        std::string sto, sto1, sto2;
        for (int i = 0; i < sizeof(msg); i++) {

            sto.push_back(msg[i]);

        }
        memset(&msg6, 0, sizeof(msg6));//clear the buffer
        if (recv(newSd[j], (char*)&msg6, sizeof(msg6), 0) <= 0) {
            std::cout << "cantown" << std::endl;
        }
        for (int i = 0; i < sizeof(msg6); i++) {
            sto1.push_back(msg6[i]);
        }
        std::cout << sto1 << std::endl;

        if(sto.find("Relay845") != std::string::npos){
            memset(&msg6, 0, sizeof(msg6));//clear the buffer
            if (recv(newSd[j], (char*)&msg6, sizeof(msg6), 0) <= 0) {
                std::cout << "noname" << std::endl;
            }
            else {
                for (int i = 0; i < sizeof(msg6); i++) {
                    sto2.push_back(msg6[i]);
                }
            }
        }


 

        tmp[j] = sto;
        tsk[j] = newSockAddr;
        tmp1[j] = sto1;
        return j;

    }
    else {
        close(newSd[j]);
        return -1;
    }
}

int acpt() {
    sockaddr_in6 newSockAddr;
    bzero((char*)&newSockAddr, sizeof(newSockAddr));
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    int j;

    std::cout << "Waiting for clients to connect..." << std::endl;
    for (int i = 0; i < 50; i++) {

        if (tmp[i].empty()) {

            j = i;
            break;
        }
        else if (i == 49) {
            return i++;
        }

    }
    std::cout << j << std::endl;

    //receive a request from client using accept
    //we need a new address to connect with the client
    newSd[j] = accept(serverSd, (sockaddr*)&newSockAddr, &newSockAddrSize);
    if (newSd[j] < 0)
    {
        std::cerr << "Error accepting request from client!" << std::endl;
        exit(1);
    }
    char str[INET6_ADDRSTRLEN];
    std::cout << "Connected with client!" << std::endl;
    std::cout << inet_ntop(AF_INET6, &(newSockAddr.sin6_addr.s6_addr), str, INET6_ADDRSTRLEN) << ":" << newSockAddr.sin6_port << std::endl;

    tmp[j] = "aavavgaagggggggg";
    tsk[j] = newSockAddr;
    return j;
}

int main(int argc, char* argv[])
{
    //for the server, we only need to specify a port number
    if (argc != 2)
    {
        std::cerr << "Usage: port" << std::endl;
        exit(0);
    }
    //grab the port number
    int port = atoi(argv[1]);
    //buffer to send and receive messages with
    char msg[128], msg1[10], msg2[10], msg3[128], msg4[10], msg5[10];

    //setup a socket and connection tools
    sockaddr_in6 servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin6_family = AF_INET6;
    servAddr.sin6_addr= in6addr_any;
    servAddr.sin6_port = htons(port);

    //open stream oriented socket with internet address
    //also keep track of the socket descriptor
    serverSd = socket(AF_INET6, SOCK_STREAM, 0);
    if (serverSd < 0)
    {
        std::cerr << "Error establishing the server socket" << std::endl;
        exit(0);
    }
    //bind the socket to its local address
    int bindStatus = bind(serverSd, (struct sockaddr*)&servAddr, sizeof(servAddr));
    if (bindStatus < 0)
    {
        std::cerr << "Error binding socket to local address" << std::endl;
        exit(0);
    }
    std::thread iput(input);
    //accept, create a new socket descriptor to 
    //handle the new connection with client
    int j;
    std::vector<std::future<int>> fut(51);


    //listen for up to 5 requests at a time
    listen(serverSd, 5);

    while (j < 50) {
        for (int k = 0; k < 50; k++) {
            if (fut[k].valid()) {
                if (fut[k].wait_for(std::chrono::microseconds(100)) != std::future_status::timeout) {

                    bool flg1 = false;
                    std::string data1, data2, datac1, datac2, oip;
                    char str1[INET_ADDRSTRLEN], str2[INET_ADDRSTRLEN];
                    oip = "221.223.91.112";
                    j = fut[k].get();

                    if (j > 0) {
                        for (int i = 0; i < 50; i++) {
                            if (tmp[j] == tmp[i] && i != j) {
                                std::cout << "i: " << i << std::endl;
                                std::cout << tmp[j] << std::endl;
                                std::cout << tmp[i] << std::endl;
                                tmp[i] = "anviouajsdfija7129408489uhnaidf";
                                tmp[j] = "anviouajsdfija7129408489uhnaidf";
                                struct timeval timeout, timeout1;
                                timeout.tv_sec = 2;
                                timeout.tv_usec = 0;
                                if (setsockopt(newSd[i], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
                                    std::cout << "tmotf1" << std::endl;
                                }
                                if (setsockopt(newSd[j], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
                                    std::cout << "tmotf2" << std::endl;
                                }
                                memset(&msg2, 0, sizeof(msg2));
                                memset(&msg3, 0, sizeof(msg3));
                                std::string es = "PING";
                                strcpy(msg2, es.c_str());
                                send(newSd[i], (char*)&msg2, sizeof(msg2), 0);
                                send(newSd[j], (char*)&msg2, sizeof(msg2), 0);
                                memset(&msg2, 0, sizeof(msg2));
                                if (recv(newSd[i], (char*)&msg3, sizeof(msg3), 0) <= 0) {
                                    std::cout << "cl discnted" << std::endl;
                                    close(newSd[i]);
                                    close(newSd[j]);
                                    tmp[i].clear();
                                    tmp1[i].clear();
                                    tmp[j].clear();
                                    tmp1[j].clear();
                                    flg1 = true;
                                    break;
                                }
                                memset(&msg3, 0, sizeof(msg3));
                                if (recv(newSd[j], (char*)&msg3, sizeof(msg3), 0) <= 0) {
                                    std::cout << "cl discnted" << std::endl;
                                    close(newSd[i]);
                                    close(newSd[j]);
                                    tmp[i].clear();
                                    tmp1[i].clear();
                                    tmp[j].clear();
                                    tmp1[j].clear();
                                    flg1 = true;
                                    break;
                                }
                                timeout1.tv_sec = 1000000;
                                timeout1.tv_usec = 0;
                                if (setsockopt(newSd[i], SOL_SOCKET, SO_RCVTIMEO, &timeout1, sizeof(timeout1)) < 0) {
                                    std::cout << "tmotf1" << std::endl;
                                }
                                if (setsockopt(newSd[j], SOL_SOCKET, SO_RCVTIMEO, &timeout1, sizeof(timeout1)) < 0) {
                                    std::cout << "tmotf2" << std::endl;
                                }
                                memset(&msg3, 0, sizeof(msg3));//clear the buffer
                                memset(&msg4, 0, sizeof(msg4));//clear the buffer
                                memset(&msg5, 0, sizeof(msg5));//clear the buffer
                                memset(&msg, 0, sizeof(msg));//clear the buffer
                                memset(&msg1, 0, sizeof(msg1));//clear the buffer
                                memset(&msg2, 0, sizeof(msg2));//clear the buffer

                                data1 = tsk[j].sin6_port;
                                data2 = tsk[i].sin6_port;
                                datac1 = inet_ntop(AF_INET, &(tsk[j].sin6_addr.s6_addr), str1, INET6_ADDRSTRLEN);
                                datac2 = inet_ntop(AF_INET, &(tsk[i].sin6_addr.s6_addr), str2, INET6_ADDRSTRLEN);
                                if (datac1 == datac2) {
                                    strcpy(msg, tmp1[j].c_str());
                                    std::cout << "lc : " << std::endl;
                                }

                                else if ((datac1.find("192.168.1.") != std::string::npos) && (datac2.find("192.168.1.") == std::string::npos)) {

                                    strcpy(msg, oip.c_str());

                                }
                                else {

                                    strcpy(msg, inet_ntop(AF_INET6, &(tsk[j].sin6_addr.s6_addr), str1, INET6_ADDRSTRLEN));

                                }

                                if (datac1 == datac2) {
                                    strcpy(msg3, tmp1[i].c_str());
                                    std::cout << "lc : " << std::endl;
                                }
                                else if ((datac1.find("192.168.1.") == std::string::npos) && (datac2.find("192.168.1.") != std::string::npos)) {

                                    strcpy(msg3, oip.c_str());

                                }
                                else {

                                    strcpy(msg3, inet_ntop(AF_INET6, &(tsk[i].sin6_addr.s6_addr), str2, INET6_ADDRSTRLEN));

                                }
                                strcpy(msg1, data1.c_str());
                                strcpy(msg2, data2.c_str());
                                strcpy(msg4, data2.c_str());
                                strcpy(msg5, data1.c_str());
                                int a, a1, a2, a3, a4, a5, a6;
                                /*thread aa, aaa;
                                aa = std::thread(syc, newSd[i], msg, msg1, msg2, tsk[i]);*/
                                int fgd = 0;
                                usleep(20000);
                                do {
                                    a = sendto(newSd[i], (char*)msg, sizeof(msg), 0, (sockaddr*)&tsk[i], sizeof(tsk[i]));
                                    std::cout << msg << "(bytes:" << a << ")" << std::endl;
                                    fgd = errno;
                                } while (a <= 5);
                                if (fgd != 0) {
                                    std::cout << "cl discnted\nerror: " << fgd << std::endl;
                                    fgd = 0;
                                }
                                do {
                                    a3 = sendto(newSd[j], (char*)msg3, sizeof(msg3), 0, (sockaddr*)&tsk[j], sizeof(tsk[j]));
                                    std::cout << msg3 << "(bytes:" << a3 << ")" << std::endl;
                                    fgd = errno;
                                } while (a <= 5);
                                if (fgd != 0) {
                                    std::cout << "cl discnted\nerror: " << fgd << std::endl;
                                    fgd = 0;
                                }
                                usleep(30000);
                                do {
                                    a1 = sendto(newSd[i], (char*)msg1, sizeof(msg1), 0, (sockaddr*)&tsk[i], sizeof(tsk[i]));
                                    std::cout << msg1 << "(bytes:" << a1 << ")" << std::endl;
                                } while (a1 <= 2);
                                if (fgd != 0) {
                                    std::cout << "cl discnted\nerror: " << fgd << std::endl;
                                    fgd = 0;
                                }
                                do {
                                    a4 = sendto(newSd[j], (char*)msg4, sizeof(msg4), 0, (sockaddr*)&tsk[j], sizeof(tsk[j]));
                                    std::cout << msg4 << "(bytes:" << a4 << ")" << std::endl;
                                } while (a4 <= 2);
                                if (fgd != 0) {
                                    std::cout << "cl discnted\nerror: " << fgd << std::endl;
                                    fgd = 0;
                                }
                                usleep(30000);
                                do {
                                    a2 = sendto(newSd[i], (char*)msg2, sizeof(msg2), 0, (sockaddr*)&tsk[i], sizeof(tsk[i]));
                                    std::cout << msg2 << "(bytes:" << a2 << ")" << std::endl;
                                } while (a2 <= 2);
                                if (fgd != 0) {
                                    std::cout << "cl discnted\nerror: " << fgd << std::endl;
                                    fgd = 0;
                                }
                                do {
                                    a5 = sendto(newSd[j], (char*)msg5, sizeof(msg5), 0, (sockaddr*)&tsk[j], sizeof(tsk[j]));
                                    std::cout << msg5 << "(bytes:" << a5 << ")" << std::endl;
                                } while (a5 <= 2);
                                if (fgd != 0) {
                                    std::cout << "cl discnted\nerror: " << fgd << std::endl;
                                    fgd = 0;
                                }
                                //usleep(100000);

                                /*aaa = std::thread(syc, newSd[j], msg3, msg4, msg5, newSockAddr);*/

                                if (t[j].joinable() == true) {

                                    t[j].join();

                                }

                                tmp1[i].clear();
                                tmp1[j].clear();
                                t[j] = std::thread(idp, i, j);
                                flg1 = true;
                                break;
                            }
                        }
                    }
                }
            }
            else {
                usleep(1000);
            }
        }
        if (!fut[50].valid()) {
            fut[50] = std::async(acpt);
        }
        else if (fut[50].wait_for(std::chrono::microseconds(100)) != std::future_status::timeout) {
            j = fut[50].get();
            fut[j] = std::async(lsn, j, tsk[j]);
        }
    }

    std::cout << "full" << std::endl;
    if (iput.joinable()) {
        iput.join();
    }

    close(serverSd);
    return 0;
}
