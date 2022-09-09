//
// Created by skhan on 8/23/22.
//

#include "../../include/RuleManager/RuleManager.h"

Poco::DynamicStruct RuleManager::parseJson(std::string str)
{
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var json = parser.parse(str);
    Poco::JSON::Object::Ptr jsonPtr = json.extract<Poco::JSON::Object::Ptr>();
    Poco::DynamicStruct strct = jsonPtr->makeStruct(jsonPtr);
    return strct;
}

void RuleManager::breakString(std::string orgStr, std::string& str1, std::string& str2)
{
    std::string str = "";
    for (auto c : orgStr)
    {
        if (c == '=')
        {
            str1 = str;
            str = "";
        }
        else {
            str = str + c;
        }
    }
    str2 = str;
}

std::string RuleManager::getFileData(std::string path)
{
    Poco::FileInputStream * fis = new Poco::FileInputStream(path, std::ios::in);
    std::stringstream ss;
    char buf[8096];
    int count = 0;
    while(!fis->eof())
    {
        fis->read((char*)&buf,8096);
        std::streamsize sz = fis->gcount();
        if(sz > 0)
        {
            buf[sz] = '\0';
            count+=sz;
            ss << buf;
        }
        else break;
    }
    fis->close();
    return ss.str();
}

void RuleManager::setCondition(std::map<int, std::string>* con, std::string* str)
{
    if (str != nullptr)
    {
        std::string temp = "";
        int tag = 0;

        for(auto s : *str)
        {
            if (s == '=')
            {
                tag = std::stoi(temp);
                temp = "";
            }
            else
            {
                temp = temp + s;
            }
        }

        if (temp != "" && tag != 0)
        {
            con->insert(std::pair<int, std::string>(tag,temp));
        }
    }
}


void RuleManager::getTagsValues(std::map<int, std::string>* msg, Poco::Dynamic::Var routingrules, std::string type)
{
    for (auto rule : routingrules)
    {
        Poco::DynamicStruct strct = parseJson(rule.toString());
        if (type == "EditTags" || type == "SetValue")
        {
            if (strct.contains("Tag") && strct.contains("Value"))
            {
                int tag = std::stoi(strct["Tag"].toString());
                std::string val(strct["Value"].toString());
                //msg[tag] = val;
                msg->insert(std::pair<int, std::string>(tag,val));
                //msg = &msg2;
            }
        }
    }
}

void RuleManager::MapValues(Poco::Dynamic::Var routingrules, Rules* rulePtr)
{
    auto *mapRule = (MapTagValue*)rulePtr;
    for (auto rule : routingrules)
    {
        Poco::DynamicStruct strct = parseJson(rule.toString());
        if (strct.contains("File") && strct.contains("From") &&(strct.contains("To") || strct.contains("ToHeader")))
        {
            std::string fileName = "/home/skhan/Repo/fix/FIXHub/config/";
            fileName = fileName + strct["File"].toString();
            int from = std::stoi(strct["From"].toString());
            int to = 0;
            if (strct.contains("To"))
            {
                to = std::stoi(strct["To"].toString());
            }
            else if (strct.contains("ToHeader"))
            {
                to = std::stoi(strct["ToHeader"].toString());
            }

            std::fstream file;
            file.open(fileName, std::ios::in);
            std::string str,str1,str2;
            while(!file.eof() && file.good())
            {
                std::getline(file,str);
                breakString(str,str1,str2);
                mapRule->fromTagValue.insert(std::pair<int, std::string>(from,str1));
                mapRule->toTagValue.insert(std::pair<int, std::string>(to,str2));
            }
            mapRule->msg.insert(std::pair<int, int>(from,to));
            file.close();
        }
    }
}

void RuleManager::getRules(Poco::Dynamic::Var routingrules, std::string type, Rules* rulePtr)
{
    if (type == "ClearTags")
    {
        auto *clearRule = (ClearTags*)rulePtr;
        for (auto rule : routingrules)
        {
            int tag = std::stoi(rule.toString());
            clearRule->tags.push_back(tag);
        }
    }
    else if (type == "EditTags")
    {
        auto *editRule = (EditTags*)rulePtr;
        for (auto rule : routingrules){
            Poco::DynamicStruct strct = parseJson(rule.toString());
            if (strct.contains("SubType"))
            {
                editRule->subType = std::stoi(strct["SubType"].toString());
                getTagsValues(&editRule->msg,strct["TagValues"],type);
            }
        }
    }
    else if (type == "CopyTags")
    {
        auto *copyRule = (CopyTags*)rulePtr;
        for (auto rule : routingrules){
            Poco::DynamicStruct strct = parseJson(rule.toString());
            int from,to;
            if (strct.contains("From"))
            {
                from = std::stoi(strct["From"].toString());
            }
            if (strct.contains("ToHeader"))
            {
                to = std::stoi(strct["ToHeader"].toString());
            }
            if (strct.contains("To"))
            {
                to = std::stoi(strct["To"].toString());
            }
            copyRule->toFromPairs.insert(std::pair<int, int>(from,to));
        }
    }
    else if (type == "SetValue")
    {
        auto *setRule = (SetValue*)rulePtr;
        getTagsValues(&setRule->msg,routingrules,type);
    }
    else if (type == "MapTagValue")
    {
        MapValues(routingrules, rulePtr);
    }
    else if (type == "Forward")
    {
        auto* frwd = (Forward*)rulePtr;
        for (auto rule : routingrules)
        {
            Poco::DynamicStruct strct = parseJson(rule.toString());
            if (strct.contains("sendercompid") && strct.contains("targetcompid"))
            {
                std::string sender = strct["sendercompid"].toString();
                std::string target = strct["targetcompid"].toString();
                frwd->clientTargetPairs.insert(std::pair<std::string, std::string>(sender, target));
            }
        }
    }
    else if (type == "RepeatingGroupRules")
    {
        auto* rpg = (RepeatingGroupRules*)rulePtr;
        Poco::DynamicStruct strct = parseJson(routingrules.toString());
        int gtag = 0, stag = 0;
        if (strct.contains("GroupTag") && strct.contains("StartTag"))
        {
            gtag = std::stoi(strct["GroupTag"].toString());
            stag = std::stoi(strct["StartTag"].toString());
        }
        bool createNew = strct.contains("CreateNew") ? true :false;
        int legNo = strct.contains("LegNum") ? std::stoi(strct["LegNum"].toString()) : 0;

        rpg->groupTag = gtag;
        rpg->startTag = stag;
        rpg->CreateNew = createNew;
        rpg->legNum = legNo;
        if (strct.contains("Rules"))
        {
            pRuleSet(strct["Rules"], &rpg->rules, nullptr);
        }

        if (strct.contains("Legs"))
        {
            for (auto rule : strct["Legs"])
            {
                Poco::DynamicStruct subStrct = parseJson(rule.toString());
                int lno = 0;
                if (subStrct.contains("LegNo"))
                {
                    lno = std::stoi(subStrct["LegNo"].toString());
                }
                std::vector<Rules*> rul;
                if (subStrct.contains("Rules"))
                {
                    pRuleSet(subStrct["Rules"],&rul, nullptr);
                }
                rpg->legs.insert(std::pair<int, std::vector<Rules*>>(lno, rul));
            }
        }
    }
}


void RuleManager::pRuleSet(Poco::Dynamic::Var routingrules,  std::vector<Rules*>* vRules = nullptr,std::string* condition = nullptr)
{
    for (auto rule : routingrules)
    {
        Poco::DynamicStruct strct = parseJson(rule.toString());
        if (strct.contains("targetcompid") && strct.contains("sendercompid"))
        {
            std::vector<Rules*> v_rule;
            pRuleSet(strct["RuleSet"],&v_rule);
            inComingRules.insert(std::pair<std::string, std::vector<Rules*>>(strct["sendercompid"].toString(),v_rule));
            outGoingRules.insert(std::pair<std::string, std::vector<Rules*>>(strct["targetcompid"].toString(),v_rule));
        }
        else if (strct.contains("sendercompid"))
        {
            std::vector<Rules*> v_rule;
            pRuleSet(strct["RuleSet"],&v_rule);
            inComingRules.insert(std::pair<std::string, std::vector<Rules*>>(strct["sendercompid"].toString(),v_rule));
        }
        else if (strct.contains("targetcompid"))
        {
            std::vector<Rules*> v_rule;
            pRuleSet(strct["RuleSet"],&v_rule);
            outGoingRules.insert(std::pair<std::string, std::vector<Rules*>>(strct["targetcompid"].toString(),v_rule));
        }
        else if (strct.contains("Condition"))
        {
            std::string con(strct["Condition"].toString());
            pRuleSet(strct["Rules"],vRules, &con);
        }
        else if (strct.contains("Type"))
        {

            if (strct["Type"].toString() == "EditTags")
            {
                EditTags* r = new EditTags();
                setCondition(&r->condition, condition);
                r->Type = strct["Type"].toString();
                getRules(strct["EditRules"], strct["Type"].toString(), r);
                if (vRules != nullptr)
                    vRules->push_back((Rules*)r);
            }
            else if (strct["Type"].toString() == "CopyTags")
            {
                CopyTags * r = new CopyTags ();
                setCondition(&r->condition, condition);
                r->Type = strct["Type"].toString();
                getRules(strct["Tags"], strct["Type"].toString(), r);
                if (vRules != nullptr)
                    vRules->push_back((Rules*)r);
            }
            else if (strct["Type"].toString() == "SetValue")
            {
                SetValue * r = new SetValue ();
                setCondition(&r->condition, condition);
                r->Type = strct["Type"].toString();
                getRules(strct["TagValues"], strct["Type"].toString(), r);
                if (vRules != nullptr)
                    vRules->push_back((Rules*)r);
            }
            else if (strct["Type"].toString() == "MapTagValue")
            {
                MapTagValue * r = new MapTagValue ();
                setCondition(&r->condition, condition);
                r->Type = strct["Type"].toString();
                getRules(strct["Tags"], strct["Type"].toString(), r);
                if (vRules != nullptr)
                    vRules->push_back((Rules*)r);
            }
            else if (strct["Type"].toString() == "Forward")
            {
                Forward * r = new Forward ();
                setCondition(&r->condition, condition);
                r->Type = strct["Type"].toString();
                getRules(strct["To"], strct["Type"].toString(), r);
                if (vRules != nullptr)
                    vRules->push_back((Rules*)r);
            }
            else if (strct["Type"].toString() == "RepeatingGroupRules")
            {
                RepeatingGroupRules * r = new RepeatingGroupRules();
                setCondition(&r->condition, condition);
                r->Type = strct["Type"].toString();
                getRules(rule, strct["Type"].toString(), r);
                if (vRules != nullptr)
                    vRules->push_back((Rules*)r);
            }
            else if (strct["Type"].toString() == "ClearTags")
            {
                ClearTags * ct = new ClearTags();
                setCondition(&ct->condition, condition);
                ct->Type = strct["Type"].toString();
                getRules(strct["Tags"], strct["Type"].toString(), ct);
                if (vRules != nullptr)
                    vRules->push_back((Rules*)ct);
            }
        }
    }
}


void RuleManager::loadConfig()
{
    std::string fileName(getFileData("../../config/ConfFilePath.txt"));
    std::string fileData(getFileData(fileName));

    Poco::DynamicStruct strct = parseJson(fileData);
    if (strct.contains("RoutingRuleSet"))
    {
        pRuleSet(strct["RoutingRuleSet"]);
    }
}

void RuleManager::validateEditTags(EditTags* e, bool isRepeation = false)
{
    int subType = e->subType;
    int i = 1;
    switch (subType)
    {
        case 1: {
            for (auto v : e->msg)
            {
                if(m_pOrgMsg->isSetField(v.first) && m_pRuleMsg->isSetField(v.first))
                {
                    fmt::print("\n");
                    fmt::print(fmt::emphasis::bold | fg(fmt::color::orange), "Rule No.: {}\n",i);
                    ++i;
                    std::string str = m_pOrgMsg->getField(v.first);
                    str = v.second + str;
                    if (str == m_pRuleMsg->getField(v.first))
                    {
                        fmt::print(fg(fmt::color::light_green), "Sub-Type: Prepend\n");
                        fmt::print(fg(fmt::color::light_green), "Tag: {0}\nOrginal Value: {1}\nEdit Value: {2}\n", v.first,m_pOrgMsg->getField(v.first),v.second);
                        fmt::print(fg(fmt::color::light_green), "Current Value: {0}\nExpected Value: {1}\n", m_pRuleMsg->getField(v.first), str);
                        fmt::print(fg(fmt::color::golden_rod), "Status: Passed\n");
                      //  std::cout << "Edit Tag Rules: Tag: " << v.first << ", Value: '" << m_pOrgMsg->getField(v.first) <<  "' to '" << str <<"'"<< std::endl;
                    }
                    else
                    {
                        fmt::print(fg(fmt::color::red), "Edit Tag Rule Failed with Wrong Value:\n");
                        fmt::print(fg(fmt::color::red), "Tag: {0}\nCurrent Value: {1}\nExpected Value: {2}\n",v.first,m_pRuleMsg->getField(v.first), str);
                        fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");

                    }
                }
                else
                {
                    fmt::print(fg(fmt::color::red),"No Edit Tag Rules Applied On Msg.\n");
                    fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
                }
            }
            break;
        }
        case 2: {
            for (auto v : e->msg)
            {
                if(m_pOrgMsg->isSetField(v.first) && m_pRuleMsg->isSetField(v.first))
                {
                    fmt::print("\n");
                    fmt::print(fmt::emphasis::bold | fg(fmt::color::orange), "Rule No.: {}\n",i);
                    ++i;
                    std::string str = m_pOrgMsg->getField(v.first);
                    str = str + v.second;
                    if (str == m_pRuleMsg->getField(v.first))
                    {
                        fmt::print(fg(fmt::color::light_green), "Sub-Type: Append\n");
                        fmt::print(fg(fmt::color::light_green), "Tag: {0}\nOrginal Value: {1}\nEdit Value: {2}\n", v.first,m_pOrgMsg->getField(v.first),v.second);
                        fmt::print(fg(fmt::color::light_green), "Current Value: {0}\nExpected Value: {1}\n", m_pRuleMsg->getField(v.first), str);
                        fmt::print(fg(fmt::color::golden_rod), "Status: Passed\n");
                    }
                    else
                    {
                        fmt::print(fg(fmt::color::red), "Edit Tag Rule Failed with Wrong Value:\n");
                        fmt::print(fg(fmt::color::red), "Tag: {0}\nCurrent Value: {1}\nExpected Value: {2}\n",v.first,m_pRuleMsg->getField(v.first), str);
                        fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
                    }
                }
                else
                {
                    fmt::print(fg(fmt::color::red),"No Edit Tag Rules Applied On Msg.\n");
                    fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
                }
            }
            break;
        }
        case 3: {
            break;
        }
        case 4: {
            break;
        }
        case 5: {
            break;
        }
        case 6: {
            break;
        }
        case 7: {
            break;
        }
        case 8: {
            break;
        }
        default: {
            std::cerr << "Wrong SubType." << std::endl;
            break;
        }
    }
}

void RuleManager::validateCopyTags(CopyTags* c, bool isRepeation = false)
{
    int i = 1;

    for (auto cft : c->toFromPairs)
    {
        if ( (m_pRuleMsg->isSetField(cft.first) || m_pRuleMsg->getHeader().isSetField(cft.first)) && (m_pRuleMsg->isSetField(cft.second) || m_pRuleMsg->getHeader().isSetField(cft.second)))
        {
            fmt::print("\n");
            fmt::print(fg(fmt::color::orange), "Rule No.: {}\n",i);
            ++i;
            std::string from;
            std::string to;

            if (m_pRuleMsg->isSetField(cft.first))
            {
                from = m_pRuleMsg->getField(cft.first);
            }
            else if(m_pRuleMsg->getHeader().isSetField(cft.first))
            {
                from = m_pRuleMsg->getHeader().getField(cft.first);
            }

            if (m_pRuleMsg->isSetField(cft.second))
            {
                to = m_pRuleMsg->getField(cft.second);
            }
            else if(m_pRuleMsg->getHeader().isSetField(cft.second))
            {
                to = m_pRuleMsg->getHeader().getField(cft.second);
            }

            if (from == to)
            {
                fmt::print(fg(fmt::color::light_green), "Copy Rules Value Copied.\n");
                fmt::print(fg(fmt::color::light_green), "From Tag: {0}\nValue: {1}\n",cft.first, from);
                fmt::print(fg(fmt::color::light_green), "To Tag: {0}\nValue: {1}\n",cft.second, to);
                fmt::print(fg(fmt::color::golden_rod), "Status: Passed");
               // std::cout << "CopyTag Rules: From Tag: " << cft.first <<" -----> To Tag: " << cft.second << std::endl;
            }
            else
            {
                fmt::print(fg(fmt::color::red), "Copy Rules Value Not Copied.\n");
                fmt::print(fg(fmt::color::red), "From Tag: {0}\nValue: {1}\n",cft.first, from);
                fmt::print(fg(fmt::color::red), "To Tag: {0}\nValue: {1}\n",cft.second, to);
                fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
//                std::cout << " Tag: " << cft.first << " to Tag: " << cft.second << std::endl;
            }

        }
        else
        {
            fmt::print(fg(fmt::color::red), "No Copy Tag Rules Applied On Msg.\n");
            fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
            //std::cout << " For Tag: " << cft.first << " to Tag: " << cft.second << std::endl;
        }
    }
}

void RuleManager::validateSetValue(SetValue* s, bool isRepeation = false)
{
    if (isRepeation)
    {
        fmt::print(fmt::emphasis::italic | fg(fmt::color::light_yellow), "Repeating Group: \n");
    }
    int i = 1;
    for (auto val : s->msg)
    {
        if (m_pRuleMsg->isSetField(val.first))
        {
            fmt::print("\n");
            fmt::print(fg(fmt::color::orange), "Rule No.: {}\n",i);
            ++i;
            if (m_pRuleMsg->getField(val.first) == val.second)
            {
                fmt::print(fg(fmt::color::light_green), "Tag: {0}\nCurrent Value: {1}\nExpected Value: {2}\n", val.first, m_pRuleMsg->getField(val.first) ,val.second);
                fmt::print(fg(fmt::color::golden_rod), "Status: Passed\n");
              //  std::cout << "Set Value Rules: Tag: " << val.first << ", Value: " << val.second << std::endl;
            }
            else
            {
                fmt::print(fg(fmt::color::red), "Set Value Rule: InCorrect Value.\n");
                fmt::print(fg(fmt::color::red), "Tag: {0}\nCurrent Value: {1}\nExpected Value: {2}\n", val.first, m_pRuleMsg->getField(val.first) ,val.second);
                fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
            }
        }
        else
        {
            fmt::print(fg(fmt::color::red), " Set Value Rules Applied On Msg.\n");
            fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
           // std::cout << "No Set Value Rules Applied On Msg For Tag: " << val.first << std::endl;
        }
    }
}

void RuleManager::validateMapTagValue(MapTagValue* m, bool isRepeation = false)
{
    int i = 1;
    for (auto v : m->msg)
    {
        fmt::print("\n");
        fmt::print(fg(fmt::color::orange), "Rule No.: {}\n",i);
        ++i;
        if (m_pRuleMsg->isSetField(v.first) && m_pRuleMsg->isSetField(v.second))
        {
            bool hasSymbol = false;
            bool hasValue = false;
            for(auto itr1 = m->fromTagValue.find(v.first); itr1 != m->fromTagValue.end(); itr1++)
            {
                if (m_pRuleMsg->getField(v.first) == itr1->second)
                {
                    for(auto itr2 = m->toTagValue.find(v.second); itr2 != m->toTagValue.end(); itr2++)
                    {
                        if (m_pRuleMsg->getField(v.second) == itr2->second)
                        {
                            fmt::print(fg(fmt::color::light_green), "From Tag: {0}\nTo Tag: {1}\n", v.first, v.second);
                            fmt::print(fg(fmt::color::light_green), "Current Value: {0}\nExpected Value: {1}\n", m_pRuleMsg->getField(v.second), itr2->second);
                            fmt::print(fg(fmt::color::golden_rod), "Status: Passed\n");
                            //std::cout << "Tag Value Map Rules: Tag: " << v.first << ", Value: " << itr1->second << "\tTag: " << v.second << ", Value: " << itr2->second << std::endl;
                            hasValue = true;
                        }
                    }
                    if (!hasValue)
                    {
                        fmt::print(fg(fmt::color::red),"Value Not Mapped.\n");
                        fmt::print(fg(fmt::color::red),"From Tag: {0}\nTo Tag: {1}\n", v.first, v.second);
                        fmt::print(fg(fmt::color::red), "Current Value: {}\n", m_pRuleMsg->getField(v.second));
                        fmt::print(fg(fmt::color::red),"Expected Values: \n");
                        int i = 1;
                        for (auto itr = m->toTagValue.find(v.second); itr != m->toTagValue.end(); itr++, i++)
                        {
                            fmt::print(fg(fmt::color::red),"{0}) {1}\n", i, itr->second);
                        }
                        fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
                    }
                    hasSymbol = true;
                }
            }

            if (!hasSymbol)
            {
                fmt::print(fg(fmt::color::red), "Wrong Value Of From Tag\n");
                fmt::print(fg(fmt::color::red), "From Tag: {}\n",v.first);
                fmt::print(fg(fmt::color::red), "Current Value: {}\n", m_pRuleMsg->getField(v.first));
                fmt::print(fg(fmt::color::red), "Expected Values:\n");
                int i = 1;
                for (auto itr = m->fromTagValue.find(v.second); itr != m->fromTagValue.end(); itr++, i++)
                {
                    fmt::print(fg(fmt::color::red),"{0}) {1}\n", i, itr->second);
                }
                fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
                //std::cout << ", Value For Tag: " << v.second << " is '" << m_pRuleMsg->getField(v.second) << "', Expected Value is '" << itr2->second << "'" << std::endl;
            }
        }
        else
        {
            fmt::print(fg(fmt::color::red), "No Map Tag Value Rule Applied On Msg.\n");
            fmt::print(fg(fmt::color::red), "From Tag: {}\nTo Tag: {}\n",v.first, v.second);
            fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
          //  std::cout << "N For Tag: " << v.first << ", On Tag: " << v.second << std::endl;
        }
    }
}

void RuleManager::validateForward(Forward* f, bool isRepeation = false)
{

}

void RuleManager::validateClearTag(ClearTags* ct,bool isRepeation = false)
{
    if (isRepeation)
    {

        fmt::print(fmt::emphasis::italic | fg(fmt::color::light_yellow), "Repeating Group: \n");
    }
    int i = 1;
    for (auto t : ct->tags)
    {
        fmt::print("\n");
        fmt::print(fg(fmt::color::orange), "Rule No.: {}\n",i);
        ++i;
        if (!m_pRuleMsg->isSetField(t))
        {
            fmt::print(fg(fmt::color::light_green), "Tag: {}\n", t);
            fmt::print(fg(fmt::color::golden_rod), "Status: Passed\n");
           // std::cout << "Clear Tag Rules: Tag: '" << t << "' could not be found, Rule Applied Successfully" << std::endl;
        }
        else
        {
            fmt::print(fg(fmt::color::red), "Tag: '{}' was founded in Msg, Rule Applied Unsuccessfully\n", t);
            fmt::print(fg(fmt::color::orange_red), "Status: Failed\n");
         //   std::cout << "Clear Tag Rules: Tag: '" << t << "' was founded, Rule Applied Unsuccessfully" << std::endl;
        }
    }
}

void RuleManager::validateRepeatingGroupRules(RepeatingGroupRules* r, bool isRepeation = false)
{
    if (m_pRuleMsg->isSetField(r->groupTag) && m_pRuleMsg->isSetField(r->startTag))
    {
        fmt::print(fg(fmt::color::green), "Group Tag: {0}\tValue: {1}\n", r->groupTag,m_pRuleMsg->getField(r->groupTag));
        fmt::print(fg(fmt::color::green), "Start Tag: {0}\tValue: {1}\n", r->startTag, m_pRuleMsg->getField(r->startTag));
    }
    int i = 1;
    for (auto rl : r->rules)
    {
        //std::cout << "RuleType: " << rl->Type << std::endl;
        fmt::print("\n");
        fmt::print(fg(fmt::color::orange), "Rule No.: {}",i);
        ++i;
        if (rl->Type == "EditTags")
        {
            fmt::print("\n\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "======= Edit Tag Rules =======");
            fmt::print("\n\n");
            auto *edit = (EditTags*)rl;
            validateEditTags(edit,true);
            fmt::print("\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "==============================");
            fmt::print("\n");
        }
        else if (rl->Type == "CopyTags")
        {
            fmt::print("\n\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "======= Copy Tag Rules =======");
            fmt::print("\n\n");
            auto *copy = (CopyTags*)rl;
            validateCopyTags(copy,true);
            fmt::print("\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "==============================");
            fmt::print("\n");
        }
        else if (rl->Type == "SetValue")
        {
            fmt::print("\n\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "======= SetValue Tag Rules =======");
            fmt::print("\n\n");
            auto *set = (SetValue*)rl;
            validateSetValue(set,true);
            fmt::print("\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "==============================");
            fmt::print("\n");
        }
        else if (rl->Type == "MapTagValue")
        {
            fmt::print("\n\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "======= Map Tag Rules =======");
            fmt::print("\n\n");
            auto *_map = (MapTagValue*)rl;
            validateMapTagValue(_map,true);
            fmt::print("\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "==============================");
            fmt::print("\n");
        }
        else if (rl->Type == "Forward")
        {
//            fmt::print("\n\n");
//            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "======= Forward Rules =======");
//            fmt::print("\n\n");
//            auto *frwd = (Forward*)rl;
//            validateForward(frwd,true);
//            fmt::print("\n");
//            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "==============================");
//            fmt::print("\n");
        }
        else if (rl->Type == "RepeatingGroupRules")
        {
            fmt::print("\n\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "======= Repeating Group Tag Rules =======");
            fmt::print("\n\n");
            auto * rpg = (RepeatingGroupRules*)rl;
            validateRepeatingGroupRules(rpg,true);
            fmt::print("\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "==============================");
            fmt::print("\n");
        }
        else if (rl->Type == "ClearTags")
        {
            fmt::print("\n\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "======= Clear Tag Rules =======");
            fmt::print("\n\n");
            auto *clear = (ClearTags*)rl;
            validateClearTag(clear,true);
            fmt::print("\n");
            fmt::print(bg(fmt::color::yellow) | fg(fmt::color::dark_blue), "==============================");
            fmt::print("\n");
        }
    }
    for (auto l: r->legs)
    {
        fmt::print("\n");
        fmt::print(fg(fmt::color::orange), "Leg No.: {}",l.first);
        for (auto vl : l.second)
        {
            if (vl->Type == "EditTags")
            {
                fmt::print("\n\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "======= Edit Tag Rules =======");
                fmt::print("\n\n");
                auto *edit = (EditTags*)vl;
                validateEditTags(edit,true);
                fmt::print("\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "==============================");
                fmt::print("\n");
            }
            else if (vl->Type == "CopyTags")
            {
                fmt::print("\n\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "======= Copy Tag Rules =======");
                fmt::print("\n\n");
                auto *copy = (CopyTags*)vl;
                validateCopyTags(copy,true);
                fmt::print("\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "==============================");
                fmt::print("\n");
            }
            else if (vl->Type == "SetValue")
            {
                fmt::print("\n\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "======= SetValue Tag Rules =======");
                fmt::print("\n\n");
                auto *set = (SetValue*)vl;
                validateSetValue(set,true);
                fmt::print("\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "==============================");
                fmt::print("\n");
            }
            else if (vl->Type == "MapTagValue")
            {
                fmt::print("\n\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "======= Map Tag Rules =======");
                fmt::print("\n\n");
                auto *_map = (MapTagValue*)vl;
                validateMapTagValue(_map,true);
                fmt::print("\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "==============================");
                fmt::print("\n");
            }
            else if (vl->Type == "Forward")
            {
                fmt::print("\n\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "======= Forward Rules =======");
                fmt::print("\n\n");
                auto *frwd = (Forward*)vl;
                validateForward(frwd,true);
                fmt::print("\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "==============================");
                fmt::print("\n");
            }
            else if (vl->Type == "RepeatingGroupRules")
            {
                fmt::print("\n\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "======= Repeating Group Tag Rules =======");
                fmt::print("\n\n");
                auto * rpg = (RepeatingGroupRules*)vl;
                validateRepeatingGroupRules(rpg,true);
                fmt::print("\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "==============================");
                fmt::print("\n");
            }
            else if (vl->Type == "ClearTags")
            {
                fmt::print("\n\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "======= Clear Tag Rules =======");
                fmt::print("\n\n");
                auto *clear = (ClearTags*)vl;
                validateClearTag(clear,true);
                fmt::print("\n");
                fmt::print(bg(fmt::color::yellow_green) | fg(fmt::color::dark_blue), "==============================");
                fmt::print("\n");
            }
        }
    }
}

void RuleManager::validateMsg() {
  //  std::cout << "49: " << m_pRuleMsg->getHeader().getField(49) << std::endl;
//    std::cout << "56: " << m_pRuleMsg->getHeader().getField(56) << std::endl;
    if (m_pRuleMsg->getHeader().isSetField(49)) {
        std::string sendercompid = m_pRuleMsg->getHeader().getField(49);
        fmt::print(fmt::emphasis::bold | fg(fmt::color::light_blue), "\n\nSenderCompId: {}\n", sendercompid);
        //std::cout << "SenderCompId: " << sendercompid << std::endl;
        if (inComingRules.count(sendercompid) > 0) {
            for (auto rule : inComingRules[sendercompid]) {
                bool hasCondition = false;
                for (auto con : rule->condition) {
                    if (m_pRuleMsg->getHeader().isSetField(con.first)) {
                        if (m_pRuleMsg->getHeader().getField(con.first) == con.second) {
                            hasCondition = true;
                            break;
                        }
                    }
                }

                if (hasCondition) {
                    if (rule->Type == "EditTags") {
                        auto *edit = (EditTags *) rule;
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green) | fg(fmt::color::gold), "===================== Edit Tag Rules ===================");
                        fmt::print("\n\n");
                        validateEditTags(edit);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    } else if (rule->Type == "CopyTags") {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== Copy Tag Rules ===================");
                        fmt::print("\n\n");
                        auto *copy = (CopyTags *) rule;
                        validateCopyTags(copy);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    } else if (rule->Type == "SetValue") {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== SetValue Tag Rules ===================");
                        fmt::print("\n\n");
                        auto *set = (SetValue *) rule;
                        validateSetValue(set);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    } else if (rule->Type == "MapTagValue") {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== Map Tag Value Rules ===================");
                        fmt::print("\n\n");
                        auto *_map = (MapTagValue *) rule;
                        validateMapTagValue(_map);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    } else if (rule->Type == "Forward") {
//                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== Forward Rules ===================");
//                        fmt::print("\n\n");
//                        auto *frwd = (Forward *) rule;
//                        validateForward(frwd);
//                        fmt::print("\n");
//                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
//                        fmt::print("\n\n\n");
                    } else if (rule->Type == "RepeatingGroupRules") {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== Repeating Group Rules ===================");
                        fmt::print("\n\n");
                        auto *rpg = (RepeatingGroupRules *) rule;
                        validateRepeatingGroupRules(rpg);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    }
                    else if (rule->Type == "ClearTags")
                    {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== Clear Tag Rules ===================");
                        fmt::print("\n\n");
                        auto *clear = (ClearTags*)rule;
                        validateClearTag(clear);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    }
                }
            }
        }
        else
        {
            fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "No Rule For Sender Comp ID Exists\n");
        }
    }
    if (m_pRuleMsg->getHeader().isSetField(56)) {
        std::string targetcompid = m_pRuleMsg->getHeader().getField(56);
        fmt::print(fmt::emphasis::bold | fg(fmt::color::light_blue), "TargetCompId: {}\n", targetcompid);
       // std::cout << "TargetCompId: " << targetcompid << std::endl;
        if (outGoingRules.count(targetcompid) > 0) {
            for (auto rule : outGoingRules[targetcompid]) {
                bool hasCondition = false;
                for (auto con : rule->condition) {
                    if (m_pRuleMsg->getHeader().isSetField(con.first)) {
                        if (m_pRuleMsg->getHeader().getField(con.first) == con.second) {
                            hasCondition = true;
                            break;
                        }
                    }
                }

                if (hasCondition) {
                    if (rule->Type == "EditTags") {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green) | fg(fmt::color::gold), "===================== Edit Tag Rules ===================");
                        fmt::print("\n\n");
                        auto *edit = (EditTags *) rule;
                        validateEditTags(edit);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    } else if (rule->Type == "CopyTags") {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== Copy Tag Rules ===================");
                        fmt::print("\n\n");
                        auto *copy = (CopyTags *) rule;
                        validateCopyTags(copy);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    } else if (rule->Type == "SetValue") {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== SetValue Tag Rules ===================");
                        fmt::print("\n\n");
                        auto *set = (SetValue *) rule;
                        validateSetValue(set);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    } else if (rule->Type == "MapTagValue") {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== Map Tag Value Rules ===================");
                        fmt::print("\n\n");
                        auto *_map = (MapTagValue *) rule;
                        validateMapTagValue(_map);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    } else if (rule->Type == "Forward") {
//                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== Forward Rules ===================");
//                        fmt::print("\n\n");
//                        auto *frwd = (Forward *) rule;
//                        validateForward(frwd);
//                        fmt::print("\n");
//                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
//                        fmt::print("\n\n\n");
                    } else if (rule->Type == "RepeatingGroupRules") {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== Repeating Group Rules ===================");
                        fmt::print("\n\n");
                        auto *rpg = (RepeatingGroupRules *) rule;
                        validateRepeatingGroupRules(rpg);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    }
                    else if (rule->Type == "ClearTags")
                    {
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "===================== Clear Tag Rules ===================");
                        fmt::print("\n\n");
                        auto *clear = (ClearTags*)rule;
                        validateClearTag(clear);
                        fmt::print("\n");
                        fmt::print(fmt::emphasis::bold | bg(fmt::color::green)| fg(fmt::color::gold), "=======================================================");
                        fmt::print("\n\n\n");
                    }
                }
            }
        }
        else
        {
            fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "No Rule For Target Comp ID Exists\n");
        }
    }
}
void RuleManager::setRuleMsg(std::string& str)
{
    if (m_pRuleMsg != nullptr)
    {
        delete m_pRuleMsg;
        m_pRuleMsg = nullptr;
    }
    m_pRuleMsg = new FIX::Message(str);
 //   std::cout << "RuleMsg in setRules: " << m_pRuleMsg->toString()<< std::endl;
}
void RuleManager::setOrgMsg(std::string& str)
{
    if (m_pOrgMsg != nullptr)
    {
        delete m_pOrgMsg;
        m_pOrgMsg = nullptr;
    }
    m_pOrgMsg = new FIX::Message(str);
   // std::cout << "OrgMsg in setRules: " << m_pOrgMsg->toString()<< std::endl;
}