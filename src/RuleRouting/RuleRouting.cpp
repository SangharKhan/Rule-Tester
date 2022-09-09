//
// Created by skhan on 8/23/22.
//

#include "RuleRouting/RuleRouting.h"



bool RuleRouting::setHostPort()
{
    std::fstream confFile;
    confFile.open("../../config/connection.txt",std::ios::in);

    if (confFile.is_open())
    {
        std::string str;
        while (!confFile.eof())
        {
            std::getline(confFile,str);
            std::stringstream sstr(str);
            std::string tokn;
            std::getline(sstr,tokn,'=');
            if (tokn == "host")
            {
                std::getline(sstr,tokn,'=');
                m_host = tokn;
            }
            else if (tokn == "port")
            {
                std::getline(sstr,tokn,'=');
                m_port = std::stoi(tokn);
            }
        }

        if (m_port != 0 && m_host != "")
        {
            return true;
        } else{
            return false;
        }

    } else{
        std::cerr << "Failed to open file connection.txt" << std::endl;
        return false;
    }
}

bool RuleRouting::connect()
{

    if (m_pRedis != nullptr)
        return true;

    if (setHostPort())
    {
        sw::redis::ConnectionOptions connection_options;
        connection_options.host = m_host;  // Required.
        connection_options.port = m_port;
        connection_options.db = 0;
        connection_options.connect_timeout = std::chrono::milliseconds(10);
        sw::redis::ConnectionPoolOptions pool_options;
        pool_options.size = 2;
        connection_options.type = sw::redis::ConnectionType::TCP;
        m_pRedis = new sw::redis::Redis(connection_options, pool_options);

        if (m_pRedis != nullptr)
        {
            return true;
        } else{
            return false;
        }
    } else{
        std::cerr << "Could not get host or port" << std::endl;
        return false;
    }
}

std::string RuleRouting::addRedisData(const std::string& key, const std::string& id, const std::string& name, const std::string& value)
{
    std::vector<std::pair<std::string, std::string_view>> vecData;
    vecData.push_back(std::pair<std::string, std::string_view>(name, value));
    std::string retId = m_pRedis->xadd(key, id, vecData.begin(), vecData.end());
    return retId;

}

bool RuleRouting::push(const std::string& key, const std::string& value)
{
    if (connect())
    {
        addRedisData(key,"123-1","Msg",value);
        return true;
    } else{
        std::cerr << "Could Not Connect to redis" << std::endl;
        return false;
    }
}


std::string RuleRouting::getRedisRange(const std::string& key, const std::string& start, const std::string& end)
{
    typedef std::vector<std::pair<std::string, std::string>> VecStrPair ;
    using Item = std::pair<std::string, std::optional<VecStrPair>>;
    using ItemStream = std::vector<Item>;
    ItemStream stream;
    m_pRedis->xrange(key, start, end, std::back_inserter(stream));
    if (!stream.empty())
    {
        std::string retId = stream[0].first;
        std::string value ="";
        value = stream[0].second->at(0).second;
        return value;
    }

    return "";
}

int RuleRouting::delItems(std::string_view & key)
{
    int retInt = m_pRedis->del(key);
    return retInt;
}

bool RuleRouting::del(std::string_view key)
{
    if (connect())
    {
        int ret = delItems(key);
       // std::cout << "Item Deleted " << ret << std::endl;
        return true;
    } else{
        std::cerr << "Could Not Connect to redis" << std::endl;
        return false;
    }
}

std::string RuleRouting::getValue(std::string key)
{
    if (connect())
    {
        std::string val = getRedisRange(key,"123-1","123-1");
        return val;

    } else{
        std::cerr << "Could Not Connect to redis" << std::endl;
    }
    return "";
}

bool RuleRouting::keyExists(std::string key)
{
    if (connect())
    {
       int hasKey = m_pRedis->exists(key);
       return hasKey > 0 ? true : false;
    } else{
        std::cerr << "Could Not Connect to redis" << std::endl;
    }
    return false;
}
