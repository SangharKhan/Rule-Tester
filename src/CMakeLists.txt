set(SRCS
        Application.cpp
        main.cpp
        RuleManager/RuleManager.cpp
        RuleRouting/RuleRouting.cpp
        )

add_subdirectory(RuleRouting)
add_subdirectory(RuleManager)
add_executable(main ${SRCS})
target_link_libraries(main Poco::JSON)