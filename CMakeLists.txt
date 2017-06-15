cmake_minimum_required(VERSION 3.7)

project(myshell)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp builtin.cpp builtin.h interpreter.cpp interpreter.h)
set(MKDIR_SOURCE_FILES mkdir.cpp)
set(CP_SOURCE_FILES cp.cpp)
set(MV_SOURCE_FILES mv.cpp)
set(RM_SOURCE_FILES rm.cpp)
set(LS_SOURCE_FILES ls.cpp)
set(GREP_SOURCE_FILES grep.cpp)

add_executable(myshell ${SOURCE_FILES})
add_executable(mkdir ${MKDIR_SOURCE_FILES})
add_executable(cp ${CP_SOURCE_FILES})
add_executable(mv ${MV_SOURCE_FILES})
add_executable(rm ${RM_SOURCE_FILES})
add_executable(ls ${LS_SOURCE_FILES})
add_executable(grep ${GREP_SOURCE_FILES})