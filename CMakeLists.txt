cmake_minimum_required(VERSION 3.19.6)
project(PCI_Scanner VERSION 1.16.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_library(ECNic STATIC src/ecnic.cpp)
add_library(ECM STATIC src/ecm.cpp)
add_executable(User src/user.cpp)

target_link_libraries(ECM ECNic pthread)
target_link_libraries(User ECM)
