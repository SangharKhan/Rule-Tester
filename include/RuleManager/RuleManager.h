// Created by skhan on 8/23/22.
//

#ifndef RULETESTER_RULEMANAGER_H
#define RULETESTER_RULEMANAGER_H

#include "Rules/Rules.h"
#include <cstring>
#include "Poco/FileStream.h"
#include "Poco/URI.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include <fstream>
#include <map>

using Poco::FileIOS;

class RuleManager {
private:
    ruleMap inComingRules;
    ruleMap outGoingRules;
    std::string getFileData(std::string);
    void breakString(std::string, std::string&, std::string&);
    Poco::DynamicStruct parseJson(std::string);
    void pRuleSet(Poco::Dynamic::Var, std::vector<Rules*>*, std::string*);
    void setCondition(std::map<int, std::string>*, std::string*);
    void getRules(Poco::Dynamic::Var , std::string , Rules* );
    void getTagsValues(std::map<int, std::string>*, Poco::Dynamic::Var, std::string);
    void MapValues(Poco::Dynamic::Var , Rules* );
public:
    void loadConfig();
};


#endif //RULETESTER_RULEMANAGER_H
