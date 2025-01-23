# Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.15)
project(axi_1wire_host)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES axi_1wire_host_selftest.c)
collect (PROJECT_LIB_SOURCES axi_1wire_host.c)
collect (PROJECT_LIB_HEADERS axi_1wire_host.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(axi_1wire_host STATIC ${_sources})
set_target_properties(axi_1wire_host PROPERTIES LINKER_LANGUAGE C)
