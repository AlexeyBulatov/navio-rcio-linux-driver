cmake_minimum_required(VERSION 2.8) 
project(navio-rcio-linux-driver CXX)

add_definitions("-Wall -std=c++11")

add_executable(rcio rcio.cpp)   
