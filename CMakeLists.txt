cmake_minimum_required(VERSION 3.26)
project(jvmcpp)

set(CMAKE_CXX_STANDARD 23)

add_executable(jvmcpp main.cpp
        parser.cpp
        parser.hpp
        cpool.hpp
        klass.cpp
        cpool.cpp
        vm.hpp
        vm.cpp)
