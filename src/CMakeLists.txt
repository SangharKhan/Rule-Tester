set(SRCS
        Application.cpp
        main.cpp
        RuleManager/RuleManager.cpp
        RuleRouting/RuleRouting.cpp
        )

add_subdirectory(RuleRouting)
add_subdirectory(RuleManager)
add_executable(main ${SRCS})
target_link_libraries(main Poco::JSON ${Boost_LIBRARIES} -lboost_system ${QUICKERFIX_LIBRARIES} /usr/local/lib/libhiredis.so.1.0.3-dev /usr/local/lib/libredis++.so.1.3.3 /usr/local/lib64/libfmt.a)
