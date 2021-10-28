//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// macros.h
//
// Identification: src/include/common/macros.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <stdexcept>

namespace bustub {

#define BUSTUB_EXPECT(expr, message) assert((expr) && (message))

#define BUSTUB_ASSERT(expr, message)   \
  do {                                 \
    if (! (expr)) {                    \
      throw std::logic_error(message); \
    }                                  \
  } while (false)

#define UNREACHABLE(message) throw std::logic_error(message)

// Macros to disable copying and moving
#define DISALLOW_COPY(cname)                             \
  cname(const cname &) = delete;            /* NOLINT */ \
  cname &operator=(const cname &) = delete; /* NOLINT */

#define DISALLOW_MOVE(cname)                        \
  cname(cname &&) = delete;            /* NOLINT */ \
  cname &operator=(cname &&) = delete; /* NOLINT */

#define DISALLOW_COPY_AND_MOVE(cname) \
  DISALLOW_COPY(cname);               \
  DISALLOW_MOVE(cname);

}  // namespace bustub
