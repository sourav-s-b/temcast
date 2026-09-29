cmake_minimum_required(VERSION 3.15)
project(TemCast CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Boost REQUIRED COMPONENTS system)
find_package(spdlog REQUIRED)

file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(daemon ${SOURCES})

target_include_directories(daemon PRIVATE include)

target_link_libraries(daemon PRIVATE Boost::system pthread spdlog::spdlog)

