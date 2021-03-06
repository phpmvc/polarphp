// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarphp software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
//
// Created by softboy on 2018/06/09.

#ifndef POLARPHP_UTILS_REVERSE_ITERATION_H
#define POLARPHP_UTILS_REVERSE_ITERATION_H

#include "polarphp/global/Config.h"
#include "polarphp/utils/PointerLikeTypeTraits.h"

namespace polar {
namespace utils {

template<typename T = void *>
bool should_reverse_iterate()
{
#if POLAR_ENABLE_REVERSE_ITERATION
  return internal::IsPointerLike<T>::value;
#else
  return false;
#endif

}

} // utils
} // polar

#endif // POLARPHP_UTILS_REVERSE_ITERATION_H
