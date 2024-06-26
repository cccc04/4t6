#include "SimpleRenderer.h"
#include <iostream>

int main(int argc, char* argv[]) {
    //we need 2 things: ip address and port number, in that order
    if (argc != 3)
    {
        std::cerr << "Usage: ip_address port" << std::endl; exit(0);
    }
    SimpleRenderer *sp = new SimpleRenderer;
    std::thread t1, t2;
    std::promise<void> exitSignal;
    std::future<void> futureObj = exitSignal.get_future();
    t1 = std::thread([&] {while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) { if (sp->yn() == true) { sp->ync(); std::cout << "R: " << sp->rts() << std::endl; } }});
    bool cflg;
    std::string ts = "2409:8900:2dd1:d9bd:81c8:1000:1000:1000";
    if (sp->cnect(ts.c_str(), "11111", sp->clientSd, SOCK_STREAM, true, 1500) == true) {
        std::cout << "bad network, exiting" << std::endl;
        exitSignal.set_value();
        t1.join();
        return 0;
    }
    cflg = sp->cnect(argv[1], "11111", sp->clientSd, SOCK_STREAM, true);
    while ( cflg == false) {
        std::string ss;
        do {
            std::cout << "try again? (y/n)" << std::endl;
            getline(std::cin, ss);
        } while ((ss != "y" ) && (ss != "n"));
        if (ss == "n") {
            exit(0);
        }
        else {
            cflg = sp->cnect(argv[1], "11111", sp->clientSd, SOCK_STREAM, true);
        }
    }
    t2 = std::thread([&] { std::string ss;  do { getline(std::cin, ss); sp->sts(ss.c_str()); } while (ss != "exit"); });
    sp->SSS(argv[2]);
    exitSignal.set_value();
    t1.join();
    t2.join();
    return 0;
}