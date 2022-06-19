cmake_minimum_required(VERSION 3.16.3)
project(ProyectoFinal)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS -pthread)

find_package(SQLite3)

add_executable(client client.cpp)
add_executable(server server.cpp)
add_executable(node node.cpp)

include_directories(${SQLITE3_INCLUDE_DIRS})
target_link_libraries (node sqlite3)
target_link_libraries (server sqlite3)