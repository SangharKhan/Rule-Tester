set(RuleManagerSrcs
        RuleManager.cpp
        )

set(RuleManagerHeaders
        ../../include/RuleManager/RuleManager.h
        )
add_library(RuleManager STATIC ${RuleManagerSrcs} ${RuleManagerHeaders})