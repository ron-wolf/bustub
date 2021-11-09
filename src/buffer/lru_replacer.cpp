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

#include <cstdint> // intptr_t
#include <mutex>   // scoped_lock<...MutexTypes>, defer_lock_t defer_lock

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages)
  : locs_(num_pages)
  , shr_lk_(rw_lk_, std::defer_lock)
  , exc_lk_(rw_lk_, std::defer_lock) {}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  frame_id_t frame_id_v;
  {
    std::scoped_lock lk((exc_lk_));
    
    if (queue_.empty()) { return false; }
    
    frame_id_v = queue_.front();
    queue_.pop_front();
    
    (void) locs_.erase(frame_id_v);
  }
  *frame_id = frame_id_v;
  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  std::scoped_lock lk((exc_lk_));
  
  if (locs_.find(frame_id) == locs_.end()) { return; }
  
  auto frame_pos = locs_[frame_id];
  (void) queue_.erase(frame_pos);
  locs_.erase(frame_id);
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  std::scoped_lock lk((exc_lk_));
  
  if (locs_.find(frame_id) != locs_.end()) {
    (void) queue_.erase(locs_[frame_id]);
  }
  locs_[frame_id] = queue_.insert(queue_.end(), frame_id);
}

size_t LRUReplacer::Size() {
  std::scoped_lock lk((shr_lk_));
  
  return queue_.size();
}

}  // namespace bustub
