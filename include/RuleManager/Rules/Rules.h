//
// Created by skhan on 8/23/22.
//

#ifndef RULETESTER_RULES_H
#define RULETESTER_RULES_H
#include <iostream>
#include <map>
#include <vector>

struct Rules{
    std::string Type;
    std::map<int, std::string> condition;
    std::string getValue(int tag);

} typedef Rules;

struct ClearTags : Rules{
    std::vector<int> tags;
    std::string getValue(int tag){ return "";}
};

struct EditTags : Rules {
    int subType;
    std::map<int, std::string> msg;
    std::string getValue(int tag)
    {
        if (msg.count(tag) > 0)
        {
            return msg[tag];
        }
        return "";
    }
} typedef EditTags;

struct CopyTags : Rules {
    std::map<int, int> toFromPairs;
    std::string getValue(int tag)
    {
        if (toFromPairs.count(tag) > 0)
        {
            return std::to_string(toFromPairs[tag]);
        }
        return "";
    }
} typedef CopyTags;

struct SetValue : Rules {
    std::map<int, std::string> msg;
    std::string getValue(int tag)
    {
        if (msg.count(tag) > 0)
        {
            return msg[tag];
        }
        return "";
    }
} typedef SetValue;

struct MapTagValue : Rules {
    std::multimap<int, std::string> fromTagValue;
    std::multimap<int, std::string> toTagValue;
    std::map<int, int> msg;
    std::string getValue(int tag)
    {
        return "";
    }
} typedef MapTagValue;

struct Forward : Rules {
    std::map<std::string, std::string> clientTargetPairs;
    std::string getValue(int tag){  return "";  }
} typedef Forward;

struct RepeatingGroupRules : Rules {
    int groupTag;
    int startTag;
    bool CreateNew;
    int legNum;
    std::vector<Rules*> rules;
    typedef std::map<int, std::vector<Rules*>> LegMap;
    LegMap legs;
    std::string getValue(int tag){  return "";  }
} typedef RepeatingGroupRules;

typedef std::map<std::string, std::vector<Rules*>> ruleMap;
#endif //RULETESTER_RULES_H
