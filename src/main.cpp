#include <iostream>
#include "Application.h"
#include <thread>
#include <fmt/color.h>

void runThread()
{
    RuleTesting::Application::getInstance().run();
}

int main() {

    fmt::print(fg(fmt::color::mint_cream), "Running Rule Tester");
    fmt::print("\n\n\n");
    std::thread  th(runThread);
    th.join();
    fmt::print("\n\n\n");
    fmt::print(fg(fmt::color::coral), "Exiting Rule Tester");
    return 0;
}
