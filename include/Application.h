// Created by skhan on 8/23/22.
//

#ifndef RULETESTER_APPLICATION_H
#define RULETESTER_APPLICATION_H
#include <iostream>
#include "RuleManager/RuleManager.h"

namespace RuleTesting {
    class Application {
    private:
        static Application *sInstance;
        std::string orgMsg;
        std::string ruleMsg;
        Application () : orgMsg(""), ruleMsg(""){}
    public:
        void send(std::string msg, bool isRuled) ;
        void run();
        static Application& getInstance();
    };
}
#endif //RULETESTER_APPLICATION_H
