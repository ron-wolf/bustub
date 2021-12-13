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

#include "common/config.h"  // frame_id_t
#include "common/logger.h"  // LOG_INFO(...)

//#include <algorithm>    // find_if(first,last,p)
#include <cassert>       // assert(condition)
#include <cstddef>       // size_t
#include <mutex>         // scoped_lock<...MutexTypes>, defer_lock_t defer_lock
#include <shared_mutex>  // shared_lock<Mutex>

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages) : max_pages_(num_pages) {}

LRUReplacer::~LRUReplacer() {
  frame_queue_.clear();
  queue_indices_.clear();
}

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  data_mutex_.lock();
  bool non_empty = !frame_queue_.empty();
  if (non_empty) {
    *frame_id = frame_queue_.front();
    frame_queue_.pop_front();
    queue_indices_.erase(*frame_id);

  } else {
    frame_id = nullptr;
    non_empty = false;
  }
  data_mutex_.unlock();
  return non_empty;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  data_mutex_.lock();

  decltype(queue_indices_)::const_iterator ext_pair = queue_indices_.find(frame_id);
  if (ext_pair == queue_indices_.cend()) {
    data_mutex_.unlock();

    return;
  }

  // decltype(frame_queue_)::const_iterator frame_pos = ext_pair->second;
  // frame_queue_.erase(frame_pos)
  // simply just use:
  frame_queue_.erase(ext_pair->second);

  queue_indices_.erase(ext_pair);
  data_mutex_.unlock();
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  data_mutex_.lock();

  if (static_cast<unsigned>(frame_id) >= max_pages_) {
    data_mutex_.unlock();
    return;
  }
  decltype(queue_indices_)::const_iterator kv_pair = queue_indices_.find(frame_id);  // this will be
  if (kv_pair != queue_indices_.cend()) {
    // if the page is already in the buffer pool, do nothing
    data_mutex_.unlock();
    return;
  }

  frame_queue_.push_back(frame_id);
  auto position = frame_queue_.end();
  position--;  // position to index
  queue_indices_[frame_id] = position;
  data_mutex_.unlock();
}

size_t LRUReplacer::Size() {
  data_mutex_.lock();
  size_t size = frame_queue_.size();
  data_mutex_.unlock();
  return size;
}

}  // namespace bustub
