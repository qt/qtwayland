# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qtwayland_cmake_tests)

enable_testing()

find_package(Qt6Core REQUIRED)
set(Qt6_MODULE_TEST_DEPENDS Quick)

include("${_Qt6CTestMacros}")

_qt_internal_test_module_includes(
  WaylandCompositor QWaylandBufferRef
)

# Can't test in `_qt_internal_test_module_includes`, WaylandClient has no public headers
_qt_internal_test_expect_pass(test_waylandclient)
