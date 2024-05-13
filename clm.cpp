#include "SimpleRenderer.h"
#include <iostream>

int main(int argc, char* argv[]) {
    //we need 2 things: ip address and port number, in that order
    if (argc != 3)
    {
        std::cerr << "Usage: ip_address port" << std::endl; exit(0);
    }
    SimpleRenderer *sp = new SimpleRenderer;
    sp->yyn = false;
    sp->cnect(argv[1]);
    std::thread t1, t2;
    std::promise<void> exitSignal;
    std::future<void> futureObj = exitSignal.get_future();
    t1 = std::thread([&] {while (futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout) { if (sp->yn() == true) { sp->ync(); std::cout << "R: " << sp->rts() << std::endl; } }});
    t2 = std::thread([&] { std::string ss; getline(std::cin, ss); do { sp->sts(ss.c_str()); } while (ss != "exit"); });
    if (sp->yyn == true) {
        sp->SSS(argv[2]);
    }
    exitSignal.set_value();
    t1.join();
    t2.join();
    return 0;
}