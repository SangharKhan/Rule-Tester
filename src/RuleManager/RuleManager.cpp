//
// Created by skhan on 8/23/22.
//

#include "../../include/RuleManager/RuleManager.h"


void print(EditTags* e)
{
    std::cout << "SubType: " << e->subType << std::endl;
    for(auto t : e->msg)
    {
        std::cout << "Tag: " << t.first << std::endl;
        std::cout << "Value: " << t.second << std::endl;
    }
}

void print(CopyTags* c)
{
    for (auto tf : c->toFromPairs)
    {
        std::cout << "From: " << tf.first << std::endl;
        std::cout << "To: " << tf.second << std::endl;
    }
}

void print(SetValue* s)
{
    for(auto s : s->msg)
    {
        std::cout << "Tag: " << s.first << std::endl;
        std::cout << "Value: " << s.second << std::endl;
    }
}

void print(MapTagValue* m)
{
    for (auto t : m->msg)
    {
        typedef std::multimap<int, std::string>::iterator ITR;
        for (ITR itr1 = m->fromTagValue.begin(), itr2 = m->toTagValue.begin(); itr1 != m->fromTagValue.end() && itr2 != m->toTagValue.end(); itr1++,itr2++)
        {
            if (itr1->first == t.first)
            {
                std::cout << "If Tag: '" << itr1->first << "' = '" << itr1->second <<"', than ";
            }

            if(itr2->first == t.second)
            {
                std::cout << "Tag: '" << itr2->first << "' = '" << itr2->second <<"'" << std::endl;
            }
        }
    }
}

void print(Forward* f)
{
    for (auto f : f->clientTargetPairs)
    {
        std::cout << "Sender: '" << f.first << "' to Target: '" << f.second << "'" << std::endl;
    }
}

void print(RepeatingGroupRules* r)
{
    auto cn = r->CreateNew ? "True" : "False" ;
    std::cout << "GroupTag: " << r->groupTag << std::endl;
    std::cout << "StartTag: " << r->startTag << std::endl;
    std::cout << "CreateNew: " << cn << std::endl;
    std::cout << "LegNum: " << r->legNum << std::endl;

    std::cout << "Rules:" << std::endl;
    for (auto rl : r->rules)
    {
        std::cout << "RuleType: " << rl->Type << std::endl;
        if (rl->Type == "EditTags")
        {
            auto *edit = (EditTags*)rl;
            print(edit);
        }
        else if (rl->Type == "CopyTags")
        {
            auto *copy = (CopyTags*)rl;
            print(copy);
        }
        else if (rl->Type == "SetValue")
        {
            auto *set = (SetValue*)rl;
            print(set);
        }
        else if (rl->Type == "MapTagValue")
        {
            auto *_map = (MapTagValue*)rl;
            print(_map);
        }
        else if (rl->Type == "Forward")
        {
            auto *frwd = (Forward*)rl;
            print(frwd);
        }
        else if (rl->Type == "RepeatingGroupRules")
        {
            auto * rpg = (RepeatingGroupRules*)rl;
            print(rpg);
        }
    }

    for (auto l: r->legs)
    {
        std::cout << "LegNo: " << l.first << std::endl;
        for (auto vl : l.second)
        {
            std::cout << "LegRuleType: " << vl->Type << std::endl;
            if (vl->Type == "EditTags")
            {
                auto *edit = (EditTags*)vl;
                print(edit);
            }
            else if (vl->Type == "CopyTags")
            {
                auto *copy = (CopyTags*)vl;
                print(copy);
            }
            else if (vl->Type == "SetValue")
            {
                auto *set = (SetValue*)vl;
                print(set);
            }
            else if (vl->Type == "MapTagValue")
            {
                auto *_map = (MapTagValue*)vl;
                print(_map);
            }
            else if (vl->Type == "Forward")
            {
                auto *frwd = (Forward*)vl;
                print(frwd);
            }
            else if (vl->Type == "RepeatingGroupRules")
            {
                auto * rpg = (RepeatingGroupRules*)vl;
                print(rpg);
            }
        }
    }
}



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
    if (type == "EditTags")
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

        for (auto in : inComingRules)
        {
            std::cout << "SenderCompId: " << in.first << std::endl;
            for(auto v : in.second)
            {

                std::cout << "Type: " << v->Type << std::endl;
                for (auto m : v->condition)
                {
                    std::cout << "Condition: '" << m.first << "=" << m.second << "'" << std::endl;
                }

                if (v->Type == "EditTags")
                {
                    auto *edit = (EditTags*)v;
                    print(edit);
                }
                else if (v->Type == "CopyTags")
                {
                    auto *copy = (CopyTags*)v;
                    print(copy);
                }
                else if (v->Type == "SetValue")
                {
                    auto *set = (SetValue*)v;
                    print(set);
                }
                else if (v->Type == "MapTagValue")
                {
                    auto *_map = (MapTagValue*)v;
                    print(_map);
                }
                else if (v->Type == "Forward")
                {
                    auto *frwd = (Forward*)v;
                    print(frwd);
                }
                else if (v->Type == "RepeatingGroupRules")
                {
                    auto * rpg = (RepeatingGroupRules*)v;
                    print(rpg);
                }
            }
        }
    }
}

