# This source file is part of the polarphp.org open source project
#
# Copyright (c) 2017 - 2018 polarphp software foundation
# Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See http://polarphp.org/LICENSE.txt for license information
# See http://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
#
# Created by polarboy on 2018/10/12.

add_custom_target(PolarBaseLibTests)
set_target_properties(PolarBaseLibTests PROPERTIES FOLDER "PolarBaseLibTests")

add_subdirectory(support)
add_subdirectory(basic/adt)
add_subdirectory(utils)
