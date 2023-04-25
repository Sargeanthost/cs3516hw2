cmake_minimum_required(VERSION 3.24)
project(cs3516hw2 C)

set(CMAKE_C_STANDARD 11)

include_directories(Project2)

add_executable(cs3516hw2
        Project2/project2.c
        Project2/student2.c)
