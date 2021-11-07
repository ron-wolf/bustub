//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"

#include <mutex>   // scoped_lock<...MutexTypes>, defer_lock_t defer_lock

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages)
  : locs_(num_pages)
  , shr_lk_(rw_lk_, std::defer_lock)
  , exc_lk_(rw_lk_, std::defer_lock) {}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) { return false; }

void LRUReplacer::Pin(frame_id_t frame_id) {}

void LRUReplacer::Unpin(frame_id_t frame_id) {}

size_t LRUReplacer::Size() { return 0; }

}  // namespace bustub
