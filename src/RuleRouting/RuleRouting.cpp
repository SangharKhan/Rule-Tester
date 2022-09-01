//
// Created by skhan on 8/23/22.
//

#include "RuleRouting/RuleRouting.h"
#include "Application.h"

RuleRouting& RuleRouting::getInstance()
{
    static RuleRouting* sInstance = new RuleRouting();
    return *sInstance;
}

void RuleRouting::send(std::string msg, bool isRuleApplied)
{
    RuleTesting::Application::getInstance().send(msg, isRuleApplied);
}
