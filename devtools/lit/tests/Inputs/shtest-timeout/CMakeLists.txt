# This source file is part of the polarphp.org open source project
#
# Copyright (c) 2017 - 2018 polarphp software foundation
# Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See http://polarphp.org/LICENSE.txt for license information
# See http://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
#
# Created by polarboy on 2018/11/16.

polar_setup_lit_cfg_setters(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}
   OUTPUT_NAME littestsshtesttimeout)

# need add depended ?

polar_add_lit_extra_test_executable(LittestShort Short.cpp)
polar_add_lit_extra_test_executable(LittestInfiniteLoop InfiniteLoop.cpp)

set(DEFS "")
list(APPEND DEFS "SHORT_GTEST_BIN=\"${POLAR_LIT_TEST_BIN_DIR}${DIR_SEPARATOR}LittestShort\"")
list(APPEND DEFS "INFINITE_LOOP_GTEST_BIN=\"${POLAR_LIT_TEST_BIN_DIR}${DIR_SEPARATOR}LittestInfiniteLoop\"")

set_target_properties(littestsshtesttimeout
   PROPERTIES
   COMPILE_DEFINITIONS "${DEFS}")

