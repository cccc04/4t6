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
#include <map>
#include <list>
//using namespace std;
//Server side
std::vector<std::string> tmp(50);
std::vector<std::string> tmp1(50);
sockaddr_in6 tsk[50];
int newSd[50];
std::thread t[50];
int serverSd;
std::vector<std::list<std::vector<char>>> gmsg(50);
std::mutex mutexpi, mutexpo;
std::vector<std::future<bool>> pingt(50), pongt(50);
std::vector<std::promise<void>> exitSignalPing(50), exitSignalPong(50);
std::vector<std::future<void>> futureObjPing(50), futureObjPong(50);



bool checkalive(int i, int j) {
    if (tmp[i].empty() && tmp[j].empty()) {
        return false;
    }
    else if (tmp[i].empty()) {
        std::cout << "broken" << std::endl;
        exit(0);
    }
    bool b1, b2;
    b1 = pongt[j].wait_for(std::chrono::microseconds(1)) == std::future_status::ready;
    b2 = pongt[i].wait_for(std::chrono::microseconds(1)) == std::future_status::ready;
    if (b1 || b2) {

        if (b1 && b2) {
            std::cout << "cl:" << i << " & " << j << "discnted" << std::endl;
        }
        else if (b1) {
            std::cout << "cl: " << j << "discnted" << std::endl;
        }
        else {
            std::cout << "cl: " << i << "discnted" << std::endl;
        }
        if (futureObjPing[i].wait_for(std::chrono::microseconds(1)) == std::future_status::timeout) {
            exitSignalPing[i].set_value();
            futureObjPing[i].wait();
        }
        if (futureObjPong[i].wait_for(std::chrono::microseconds(1)) == std::future_status::timeout) {
            exitSignalPong[i].set_value();
            futureObjPong[i].wait();
        }
        if (futureObjPing[j].wait_for(std::chrono::microseconds(1)) == std::future_status::timeout) {
            exitSignalPing[j].set_value();
            futureObjPing[j].wait();
        }
        if (futureObjPong[j].wait_for(std::chrono::microseconds(1)) == std::future_status::timeout) {
            exitSignalPong[j].set_value();
            futureObjPong[j].wait();
        }

        std::cout << "479: waiting for pinger" << std::endl;
        pongt[j].wait();
        pongt[i].wait();
        pingt[j].wait();
        pingt[i].wait();
        exitSignalPing[i] = std::promise<void>{};
        exitSignalPong[i] = std::promise<void>{};
        exitSignalPing[j] = std::promise<void>{};
        exitSignalPong[j] = std::promise<void>{};
        futureObjPing[i] = exitSignalPing[i].get_future();
        futureObjPong[i] = exitSignalPong[i].get_future();
        futureObjPing[j] = exitSignalPing[j].get_future();
        futureObjPong[j] = exitSignalPong[j].get_future();

        close(newSd[i]);
        close(newSd[j]);
        tmp[i].clear();
        tmp1[i].clear();
        tmp[j].clear();
        tmp1[j].clear();
        return false;
    }
    else {
        return true;
    }
}

bool gsend(std::string data, int buffersize, int i, int j) {

    //only taking received string variables
    int a;
    int l = 5;
    char msg[buffersize], msg1[6];
    std::string s = "gs:rtk";
    size_t size = data.size();
    data = "gs:" + data;
    memset(&msg, 0, sizeof(msg));
    memset(&msg1, 0, sizeof(msg1));
    strcpy(msg, data.c_str());
    strcpy(msg1, s.c_str());
    while (!gmsg[i].empty()) {
        gmsg[i].pop_front();
    }
    while (1) {
        if (gmsg[i].empty()) {
            if (!checkalive(i, j)) {
                return false;
            }
            if (l >= 5) {
                l = 0;
                std::lock_guard<std::mutex> guard(mutexpi);
                a = send(newSd[i], (char*)msg, sizeof(msg), 0);
                std::cout << "130: " << msg << "(bytes:" << a << ")" << std::endl;
                if (a < 0) {
                    std::cout << "132: error: " << errno << std::endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            l++;
        }
        else {
            std::lock_guard<std::mutex> guard(mutexpo);
            std::string lcs;
            for (auto it : gmsg[i].front()) {
                if (it == '\0') {
                    break;
                }
                lcs.push_back(it);
            }
            std::string msize;
            char cmsize[128];
            memset(&cmsize, 0, sizeof(cmsize));
            snprintf(cmsize, sizeof(cmsize), "%zu", size);
            for (int k = 0; k < strlen(cmsize); k++) {
                msize.push_back(cmsize[k]);
            }
            std::cout << "154: lcs: " << lcs << " msize: " << msize << std::endl;
            gmsg[i].pop_front();
            if (lcs != msize) {
                std::cout << "ajofivseiof" << std::endl;
            }
            else {
                std::lock_guard<std::mutex> guard(mutexpi);
                send(newSd[i], (char*)msg1, sizeof(msg1), 0);
                return true;
            }
        }
    }

}

void rl(int i, int j) {
    int sd;
    while (1) {

        char msg[1500];
        std::string data;
        while (gmsg[i].empty()) {
            if (!checkalive(i, j)) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        if (checkalive(i, j)) {
            std::lock_guard<std::mutex> guard(mutexpo);
            int l = 1;
            memset(&msg, 0, sizeof(msg));
            for (auto it : gmsg[i].front()) {
                if (l == 1501) {
                    l++;
                    break;
                }
                msg[l-1] = it;
                l++;
            }
            if (l > 1501) {
                std::cout << "194: >1501" << std::endl;
                gmsg[i].front().erase(gmsg[i].front().begin(), gmsg[i].front().begin() + 1500);
            }
            else {
                gmsg[i].pop_front();
            }
        }
        else {
            std::cout << "dscnected r" << std::endl;
            memset(&msg, 0, sizeof(msg));//clear the buffer
            data = "Z exit";
            strcpy(msg, data.c_str());
            std::lock_guard<std::mutex> guard(mutexpi);
            send(newSd[j], (char*)&msg, sizeof(msg), 0);
            break;
        }

        std::lock_guard<std::mutex> guard(mutexpi);
        std::cout << "212: msg snd: " << msg << std::endl;
        sd = send(newSd[j], (char*)&msg, sizeof(msg), 0);
        if (sd < 0) {
            std::cout << "cant snd" << std::endl;
            break;
        }
        if (!strcmp(msg, "Z exit"))
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

bool cmn(int i, int j) {

    char msg[150];
    std::string sr;
    while (gmsg[i].empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    for (int k = 0; k < 20;) {
        if (gmsg[i].empty()) {
            if (!checkalive(i, j)) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            k++;
            continue;
        }
        k = 0;
        if (1) {
            std::lock_guard<std::mutex> guard(mutexpo);
            for (auto it : gmsg[i].front()) {
                sr.push_back(it);
            }
            gmsg[i].pop_front();
        }
        if (sr.find("pcr") == 0) {
            break;
        }
        else if (sr.find("oc") == 0) {
            std::cout << "266: fd" << std::endl;
            gsend("sv", 6, i, j);
            gsend("cl", 6, j, i);
        }
        else {
            sr.clear();
        }
    }


    if (sr.find("punchedthrough") != std::string::npos)
    {
        std::cout << "w" << std::endl;
        return true;
    }
    else if (sr.find("punchedfail") != std::string::npos)
    {
        std::cout << "f" << std::endl;
        return false;
    }

    std::cout << "ept" << std::endl;
    return true;

}

void idp(int i, int j) {

    bool cmn1 = cmn(i,j);
    bool cmn2 = cmn(j,i);
    std::string s;
    std::cout << cmn1 << cmn2 << std::endl;


    if (cmn1 == true && cmn2 == true) {

        std::cout << "128: good" << std::endl;
        if (checkalive(i, j) == true) {
            exitSignalPong[i].set_value();
            pongt[i].wait();
            bool al = checkalive(i, j);
            std::cout << "135: " << al << std::endl;
        }
        tmp[i].clear();
        tmp[j].clear();
        bzero((char*)&tsk[j], sizeof(tsk[j]));
        bzero((char*)&tsk[i], sizeof(tsk[i]));
        close(newSd[i]);
        close(newSd[j]);
    }
    else if(cmn1 == false && cmn2 == false){

        std::string data = "connected";
        char msg[150];
        strcpy(msg, data.c_str());
        if (checkalive(i,j) == true) {
            std::thread tr(rl, i, j);
            std::thread tr2(rl, j, i);
            tr.join();
            tr2.join();
            if (checkalive(i, j) == true) {
                exitSignalPong[i].set_value();
                pongt[i].wait();
                bool al = checkalive(i, j);
                std::cout << "135: " << al << std::endl;
            }
            std::cout << "ok" << std::endl;
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

        if (checkalive(i, j) == true) {
            exitSignalPong[i].set_value();
            pongt[i].wait();
            bool al = checkalive(i, j);
            std::cout << "135: " << al << std::endl;
        }
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

    if (j > 50 || j < 0) {
        std::cout << "197: lsn invalid" << std::endl;
        return -1;
    }

    char msg[50], msg6[51];
    memset(&msg, 0, sizeof(msg));

    if (recv(newSd[j], (char*)&msg, sizeof(msg), 0) > 0) {

        
        std::cout << "Client: " << msg << std::endl;
        std::string sto, sto1, sto2;
        for (int i = 0; i < sizeof(msg); i++) {

            sto.push_back(msg[i]);

        }
        /*memset(&msg6, 0, sizeof(msg6));//clear the buffer
        if (recv(newSd[j], (char*)&msg6, sizeof(msg6), 0) <= 0) {
            std::cout << "cantown" << std::endl;
        }
        for (int i = 0; i < sizeof(msg6); i++) {
            sto1.push_back(msg6[i]);
        }
        std::cout << sto1 << std::endl;*/

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

bool ping(int j) {
    char msg[4];
    std::string es = "PING";
    strcpy(msg, es.c_str());
    while (futureObjPing[j].wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::lock_guard<std::mutex> guard(mutexpi);
        if (send(newSd[j], (char*)&msg, sizeof(msg), 0) < 0) {
            std::cout << "264: f snd" << std::endl;
            return false;
        }
    }
    return true;
}

bool pong( int j) {
    char msg[1500];

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(newSd[j], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cout << "284: tmotf1" << std::endl;
        return false;
    }
    while (futureObjPong[j].wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
        std::string s;
        int i;
        memset(&msg, 0, sizeof(msg));
        i = recv(newSd[j], (char*)&msg, sizeof(msg), 0);
        if ( i <= 0) {
            std::cout << "290: pong rcv t/o" << std::endl;
            if (futureObjPing[j].wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) {
                exitSignalPing[j].set_value();
                pingt[j].wait();
            }
            return false;
        }
        for (int i = 0; i < strlen(msg); i++) {
            s.push_back(msg[i]);
        }
        if(s == "PONG") {
            continue;
        }
        else {
            size_t sz;
            if (s.find("xf") == 0 || s.find("Z ") == 0 || s.find("oc") == 0) {
                sz = strlen(msg);
                std::cout << "480: " << s << ": " << sz << std::endl;
            }
            else {
                std::cout << "484: " << s << std::endl;
                sz = sizeof(msg);
            }
            std::lock_guard<std::mutex> guard(mutexpo);
            std::vector<char> tpmsg;
            for (int i = 0; i < sz; i++) {
                tpmsg.push_back(msg[i]);
            }
            gmsg[j].push_back(tpmsg);
        }

    }
    return true;
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
    std::cout << inet_ntop(AF_INET6, &(newSockAddr.sin6_addr.s6_addr), str, INET6_ADDRSTRLEN) << ":" << ntohs(newSockAddr.sin6_port) << std::endl;

    int yes = 1;
    if (setsockopt(newSd[j], SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) == -1) {
        std::cout << "KAL fail" << std::endl;
    }

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

    for (int i = 0; i < 50; i++) {
        futureObjPing[i] = exitSignalPing[i].get_future();
        futureObjPong[i] = exitSignalPong[i].get_future();
    }

    //grab the port number
    int port = atoi(argv[1]);
    //buffer to send and receive messages with
    char msg[64], msg1[10], msg2[10], msg3[64], msg4[10], msg5[10];

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
    int j = 0;
    std::vector<std::future<int>> fut(51);


    //listen for up to 5 requests at a time
    listen(serverSd, 5);

    while (j < 50) {
        for (int k = 0; k < 50; k++) {
            if (fut[k].valid() && fut[k].wait_for(std::chrono::microseconds(100)) != std::future_status::timeout) {                

                bool flg1 = false;
                std::string data1, data2, datac1, datac2, oip;
                char str1[INET6_ADDRSTRLEN], str2[INET6_ADDRSTRLEN];
                oip = "221.223.91.112";
                j = fut[k].get();
                pingt[j] = std::async(ping, j);
                pongt[j] = std::async(pong, j);

                if (j >= 0) {
                    for (int i = 0; i < 50; i++) {
                        if (tmp[j] == tmp[i] && i != j) {
                            std::cout << "i: " << i << std::endl;
                            std::cout << tmp[j] << std::endl;
                            std::cout << tmp[i] << std::endl;
                            tmp[i] = "anviouajsdfija7129408489uhnaidf";
                            tmp[j] = "anviouajsdfija7129408489uhnaidf";

                            if (!checkalive(i,j)) {
                                flg1 = true;
                                break;
                            }

                            memset(&msg3, 0, sizeof(msg3));//clear the buffer
                            memset(&msg4, 0, sizeof(msg4));//clear the buffer
                            memset(&msg5, 0, sizeof(msg5));//clear the buffer
                            memset(&msg, 0, sizeof(msg));//clear the buffer
                            memset(&msg1, 0, sizeof(msg1));//clear the buffer
                            memset(&msg2, 0, sizeof(msg2));//clear the buffer

                            data1 = std::to_string(ntohs(tsk[j].sin6_port));
                            data2 = std::to_string(ntohs(tsk[i].sin6_port));
                            datac1 = inet_ntop(AF_INET6, &(tsk[j].sin6_addr.s6_addr), str1, INET6_ADDRSTRLEN);
                            datac2 = inet_ntop(AF_INET6, &(tsk[i].sin6_addr.s6_addr), str2, INET6_ADDRSTRLEN);
                            if (datac1 == datac2) {
                                //strcpy(msg, tmp1[j].c_str());
                                std::cout << "lc : " << std::endl;
                            }

                            else if ((datac1.find("192.168.1.") != std::string::npos) && (datac2.find("192.168.1.") == std::string::npos)) {

                                strcpy(msg, oip.c_str());

                            }
                            else {

                                strcpy(msg, inet_ntop(AF_INET6, &(tsk[j].sin6_addr.s6_addr), str1, INET6_ADDRSTRLEN));

                            }

                            if (datac1 == datac2) {
                                //strcpy(msg3, tmp1[i].c_str());
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
                            /*int a, a1, a2, a3, a4, a5, a6;
                            thread aa, aaa;
                            aa = std::thread(syc, newSd[i], msg, msg1, msg2, tsk[i]);*/
                            bool bnb = false;
                            std::string data3 = inet_ntop(AF_INET6, &(tsk[i].sin6_addr.s6_addr), str2, INET6_ADDRSTRLEN);
                            std::string data = inet_ntop(AF_INET6, &(tsk[j].sin6_addr.s6_addr), str1, INET6_ADDRSTRLEN);
                            std::string data4 = "cl";
                            std::string data5 = "sv";
                            if (gsend(data, 64, i, j) != false) {
                                if (gsend(data3, 64, j, i) != false) {
                                    if (gsend(data1, 10, i, j) != false) {
                                        if (gsend(data2, 10, j, i) != false) {
                                            if (gsend(data2, 10, i, j) != false) {
                                                if (gsend(data1, 10, j, i) != false) {
                                                    if (gsend(data5, 6, i, j) != false) {
                                                        if (gsend(data4, 6, j, i) != false) {
                                                            bnb = true;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            if (bnb == false) {
                                break;
                            }
                            else {
                                /*exitSignalPong[i].set_value();
                                pongt[i].wait();
                                bool al = checkalive(i, j);
                                std::cout << "658: " << al << std::endl;
                                break;*/
                            }

                            if (t[j].joinable() == true) {

                                t[j].join();

                            }

                            tmp1[i].clear();
                            tmp1[j].clear();
                            t[j] = std::thread(&idp, i, j);
                            flg1 = true;
                            break;
                        }
                    }
                }
                else {
                    std::cout << "514: cl dscncted " << std::endl;
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
