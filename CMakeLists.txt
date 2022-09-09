cmake_minimum_required(VERSION 3.16)
project(RuleTester)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED)

include_directories(./include)
include_directories(./3rdpartyDirs/redis-plus-plus)
include_directories(./3rdpartyDirs/fmt/include/fmt)

find_package(Poco CONFIG REQUIRED JSON)
find_package(QuickerFix REQUIRED)
find_package(HiRedis REQUIRED)
find_package(redis++ REQUIRED)

add_subdirectory(src)
