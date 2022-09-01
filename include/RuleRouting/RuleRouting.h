//
// Created by skhan on 8/23/22.
//

#ifndef RULETESTER_RULEROUTING_H
#define RULETESTER_RULEROUTING_H

#include <iostream>

class RuleRouting {
private:
    static RuleRouting *sInstance;
public:
    void send(std::string, bool);
    static RuleRouting& getInstance();
};


#endif //RULETESTER_RULEROUTING_H
