cmake_minimum_required(VERSION 3.16)
project(RuleTester)

set(CMAKE_CXX_STANDARD 17)
include_directories(./include)
find_package(Poco CONFIG REQUIRED JSON)
find_package(Boost)

add_subdirectory(src)
