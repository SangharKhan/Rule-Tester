#include <iostream>
#include "Application.h"
#include <thread>

void runThread()
{
    RuleTesting::Application::getInstance().run();
}

int main() {

    std::thread  th(runThread);
    th.join();

    return 0;
}
