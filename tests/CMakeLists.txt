# This source file is part of the polarphp.org open source project
#
# Copyright (c) 2017 - 2018 polarphp software foundation
# Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See http://polarphp.org/LICENSE.txt for license information
# See http://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
#
# Created by polarboy on 2018/11/19.

polar_setup_lit_cfg_setters(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}
   OUTPUT_NAME tests
   SKIP_DIRS "Inputs")

set(DEFS "")

list(APPEND DEFS "LIT_TEST_BIN=\"${POLAR_RUNTIME_OUTPUT_INTDIR}${DIR_SEPARATOR}lit\"")
list(APPEND DEFS "POLAR_RUNTIME_OUTPUT_INTDIR=\"${POLAR_RUNTIME_OUTPUT_INTDIR}\"")

set_target_properties(tests
   PROPERTIES
   COMPILE_DEFINITIONS "${DEFS}")
