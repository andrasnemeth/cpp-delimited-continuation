cmake_minimum_required(VERSION 3.3)

project ("Delimited Continuation")

include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_executable(shiftreset shiftreset.cpp)
add_executable(runyield runyield.cpp)
set_target_properties(runyield PROPERTIES LINK_FLAGS "-lboost_context")

add_executable(simple example/lazy/simple.cpp)
set_target_properties(simple PROPERTIES COMPILE_FLAGS "-I ../include")
