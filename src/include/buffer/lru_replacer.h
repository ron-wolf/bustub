//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.h
//
// Identification: src/include/buffer/lru_replacer.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "buffer/replacer.h"
#include "common/config.h"

#include <cstddef>       // size_t
#include <list>          // list<T>
#include <shared_mutex>  // shared_mutex
#include <unordered_map> // unordered_map<Key,T>

namespace bustub {

/**
 * LRUReplacer implements the lru replacement policy, which approximates the Least Recently Used policy.
 */
class LRUReplacer : public Replacer {
 public:
  /**
   * Create a new LRUReplacer.
   * @param num_pages the maximum number of pages the LRUReplacer will be required to store
   */
  explicit LRUReplacer(size_t num_pages);

  /**
   * Destroys the LRUReplacer.
   */
  ~LRUReplacer() override;

  bool Victim(frame_id_t *frame_id) override;

  void Pin(frame_id_t frame_id) override;

  void Unpin(frame_id_t frame_id) override;

  size_t Size() override;

 private:
  /** a queue of unpinned frames, with eldest at the front */
  std::list<frame_id_t> queue_;
  /** a map from frames to queue positions */
  std::unordered_map<frame_id_t, decltype(queue_)::iterator> locs_;

  /** a mutex for read/write access to LRU data structures */
  std::shared_mutex rw_lk_;
  // this will be the core of the concurrency control. it is important
  // to have just one mutex, since this will ensure we never forget
  // to also lock for reading when we lock for writing
  
  /** a lock for shared, read-only access to LRU data structures */
  std::shared_lock<std::shared_mutex> shr_lk_;
  /** a lock for exclusive, read/write access to LRU data structures */
  std::unique_lock<std::shared_mutex> exc_lk_;
};

}  // namespace bustub
