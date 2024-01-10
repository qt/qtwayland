# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

project(test_plugins)

cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0056 NEW)

find_package(Qt6WaylandClient REQUIRED)
add_executable(test_waylandclient_exe main.cpp)
target_link_libraries(test_waylandclient_exe Qt6::WaylandClientPrivate)
