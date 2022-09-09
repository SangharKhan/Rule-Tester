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
    bool hasOrg = false, hasRule = false;
    while(true)
    {
        RuleRouting* ruleRouting = new RuleRouting();
        if (ruleRouting->keyExists("OrgMsg") && !hasOrg)
        {
            std::string str = ruleRouting->getValue("OrgMsg");
            if (str != "")
            {
                fmt::print("Org Msg: ");
                fmt::print(fg(fmt::color::cyan), "{}\n", str);
               // std::cout << "OrgMsg = " << str << std::endl;
                ruleManager->setOrgMsg(str);
                hasOrg = true;
            }
            ruleRouting->del("OrgMsg");
        }

        if (ruleRouting->keyExists("RuleMsg") && !hasRule)
        {
            std::string str = ruleRouting->getValue("RuleMsg");
            if (str != "")
            {
                fmt::print("Rule Msg: ");
                fmt::print(fg(fmt::color::hot_pink) , "{}\n", str);
                //std::cout << "RuleMsg = " << str << std::endl;
                ruleManager->setRuleMsg(str);
                hasRule = true;
            }
            ruleRouting->del("RuleMsg");
        }
        delete ruleRouting;

        if(hasOrg && hasRule)
        {
            ruleManager->validateMsg();
            hasOrg = false;
            hasRule = false;
        }
    }

}