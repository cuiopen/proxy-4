#
#            Copyright (c) Marco Amorim 2017.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
project(proxy_manager)
cmake_minimum_required(VERSION 2.8)

aux_source_directory(
    src/modules SRC_LIST)

aux_source_directory(
    src/core SRC_LIST)

aux_source_directory(
    src/net SRC_LIST)

add_executable(
    ${PROJECT_NAME}
    ${SRC_LIST}
    LICENSE_1_0.txt
    README.md
    config/settings.xml
    config/log_settings.txt)

#set(Boost_DEBUG                 ON)
#set(Boost_USE_MULTITHREADED    OFF)
#set(Boost_USE_STATIC_LIBS       ON)
find_package(
    Boost 1.55.0 REQUIRED COMPONENTS
    log_setup
    log filesystem
    program_options
    thread
    random
    system)
find_package(
    Threads REQUIRED)

if(NOT Boost_USE_STATIC_LIBS)
    add_definitions(-DBOOST_LOG_DYN_LINK)
endif()

include_directories(
    ${Boost_INCLUDE_DIRS}
    src)

target_link_libraries(
    ${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})
