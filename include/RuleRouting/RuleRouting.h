//
// Created by skhan on 8/23/22.
//

#ifndef RULETESTER_RULEROUTING_H
#define RULETESTER_RULEROUTING_H

#include <iostream>
#include <sw/redis++/redis++.h>
#include <fstream>
#include <map>

class RuleRouting {
private:
    sw::redis::Redis* m_pRedis = nullptr;
    std::string m_host = "";
    int m_port = 0;
    std::string addRedisData(const std::string& key, const std::string& id, const std::string& name, const std::string& value);
    std::string getRedisRange(const std::string& key, const std::string& start, const std::string& end);
    int delItems(std::string_view& );
    bool connect();
    bool setHostPort();
public:
    bool push(const std::string& name, const std::string& value);
    std::string getValue(std::string name);
    bool del(std::string_view key);
    bool keyExists(std::string key);
    RuleRouting(){}
    ~RuleRouting()
    {
        if (m_pRedis != nullptr)
        {
            delete m_pRedis;
        }
    }
};


#endif //RULETESTER_RULEROUTING_H
