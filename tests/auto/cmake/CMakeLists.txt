# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


cmake_minimum_required(VERSION 3.16)

project(qmake_cmake_files)

enable_testing()

find_package(Qt5Core REQUIRED)
set(Qt5_MODULE_TEST_DEPENDS Quick)

include("${_Qt5CTestMacros}")

test_module_includes(
  WaylandCompositor QWaylandBufferRef
)

# Can't test in `test_module_includes`, WaylandClient has no public headers
expect_pass(test_waylandclient)
