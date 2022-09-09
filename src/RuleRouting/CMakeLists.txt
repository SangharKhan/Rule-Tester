set(RoutingRuleSrcs
        RuleRouting.cpp
        )

set(RoutingRuleHeaders
        ${CMAKE_SOURCE_DIR}/include/RuleRouting/RuleRouting.h
        )

add_library(RuleRouting STATIC ${RoutingRuleSrcs} ${RoutingRuleHeaders} ${Boost_LIBRARIES})
#target_link_libraries(RuleRouting)