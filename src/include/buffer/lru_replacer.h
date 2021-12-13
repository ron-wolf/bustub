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
   * Destroys the LRUReplacer.llll
   */
  ~LRUReplacer() override;

  bool Victim(frame_id_t *frame_id) override;

  void Pin(frame_id_t frame_id) override;

  void Unpin(frame_id_t frame_id) override;

  size_t Size() override;

 private:
  // TODO: turn these into documentation comments
  
  std::list<frame_id_t> frame_queue_;
//  std::list<std::pair<frame_id_t, bool>> frame_queue_; // a list of frames, ordered by usage time
  std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> queue_indices_; // a map from frames to queue positions

  std::mutex data_mutex_; // a mutex for read/write access to the above data structures
                                 // this will be the core of the concurrency control. it is important
                                 // to have just one mutex, since this will ensure we never forget
                                 // to lock for reading when we lock for writing
  
  size_t max_pages_; //maximum size of the buffer pool
};

}  // namespace bustub
