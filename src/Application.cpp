//
// Created by skhan on 8/23/22.
//

#include "Application.h"
using namespace RuleTesting;

void Application::send(std::string msg, bool isRuled)
{
    if (isRuled)
        ruleMsg = msg;
    else
        orgMsg = msg;
}

Application& Application::getInstance()
{
    static Application* sInstance = new Application();
    return *sInstance;
}

void Application::run()
{
    std::unique_ptr<RuleManager> ruleManager = std::make_unique<RuleManager>();
    ruleManager->loadConfig();
    while(true)
    {

    }
}