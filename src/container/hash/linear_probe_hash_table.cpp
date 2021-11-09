//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// linear_probe_hash_table.cpp
//
// Identification: src/container/hash/linear_probe_hash_table.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "common/util/hash_util.h"
#include "container/hash/linear_probe_hash_table.h"

namespace bustub {

template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_TYPE::LinearProbeHashTable(const std::string &name,
                                      BufferPoolManager *buffer_pool_manager,
                                      const KeyComparator &comparator,
                                      size_t num_buckets,
                                      HashFunction<KeyType> hash_fn)
    : buffer_pool_manager_(buffer_pool_manager), comparator_(comparator),
      hash_fn_(std::move(hash_fn)) {
  header_page_id_ = INVALID_PAGE_ID;
  auto header_page = reinterpret_cast<HashTableHeaderPage *>(
      buffer_pool_manager_->NewPage(&header_page_id_, nullptr)->GetData());
  header_page->SetPageId(header_page_id_);

  // ceiling of num_buckets/BLOCK_ARRAY_SIZE
  size_t num_blocks = (num_buckets - 1) / BLOCK_ARRAY_SIZE + 1;

  header_page->SetSize(num_blocks * BLOCK_ARRAY_SIZE);

  CreateNewBlockPages(header_page, num_blocks);

  assert(buffer_pool_manager_->UnpinPage(header_page_id_, true, nullptr));
}

/*****************************************************************************
 * HELPERS
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
HashTableHeaderPage *HASH_TABLE_TYPE::GetHeaderPage() {
  Page *header = buffer_pool_manager_->FetchPage(header_page_id_, nullptr);
  return reinterpret_cast<HashTableHeaderPage *>(header->GetData());
}

template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_BLOCK_TYPE *HASH_TABLE_TYPE::GetBlockPage(page_id_t block_page_id) {
  Page *block = buffer_pool_manager_->FetchPage(block_page_id, nullptr);
  return reinterpret_cast<HASH_TABLE_BLOCK_TYPE *>(block->GetData());
}


/*****************************************************************************
 * SEARCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::GetValue(Transaction *transaction, const KeyType &key,
                               std::vector<ValueType> *result) {
  table_latch_.RLock();

  HashTableHeaderPage *header_page = GetHeaderPage();
  size_t num_blocks = header_page->NumBlocks();
  size_t size = header_page->GetSize();

  uint64_t hash = hash_fn_.GetHash(key);
  size_t bucket;
  bucket = hash % size;
  size_t block = bucket / BLOCK_ARRAY_SIZE;
  size_t bucket_ind = bucket % BLOCK_ARRAY_SIZE;

  size_t start_bucket_ind = bucket_ind;
  size_t start_block = block;

  page_id_t block_page_id = header_page->GetBlockPageId(block);
  HASH_TABLE_BLOCK_TYPE *block_page = GetBlockPage(block_page_id);

  (reinterpret_cast<Page *>(block_page))->RLatch();

  bool found = false;
  while (block_page->IsOccupied(bucket_ind)) {
    if (block_page->IsReadable(bucket_ind) && 0 == comparator_(key, block_page->KeyAt(bucket_ind))) {
      found = true;
      result->push_back(block_page->ValueAt(bucket_ind));
    }
    if (++bucket_ind >= BLOCK_ARRAY_SIZE) {
      (reinterpret_cast<Page *>(block_page))->RUnlatch();
      [[maybe_unused]] bool success = buffer_pool_manager_->UnpinPage(block_page_id, false, nullptr); assert(success);
      // Perform linear probe
      block = (block+1) % num_blocks;
      block_page_id = header_page->GetBlockPageId(block);
      block_page = GetBlockPage(block_page_id);
      bucket_ind = 0;
      (reinterpret_cast<Page *>(block_page))->RLatch();
    }
    if (block == start_block && bucket_ind == start_bucket_ind) {
      break;
    }
  }
  (reinterpret_cast<Page *>(block_page))->RUnlatch();
  assert(buffer_pool_manager_->UnpinPage(block_page_id, false, nullptr));
  assert(buffer_pool_manager_->UnpinPage(header_page_id_, false, nullptr));

  table_latch_.RUnlock();
  return found;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::GetValueLatchFree(Transaction *transaction,
                                        const KeyType &key,
                                        std::vector<ValueType> *result) {
  HashTableHeaderPage *header_page = GetHeaderPage();
  size_t num_blocks = header_page->NumBlocks();
  size_t size = header_page->GetSize();

  uint64_t hash = hash_fn_.GetHash(key);
  size_t bucket;
  bucket = hash % size;
  size_t block = bucket / BLOCK_ARRAY_SIZE;
  size_t bucket_ind = bucket % BLOCK_ARRAY_SIZE;

  size_t start_bucket_ind = bucket_ind;
  size_t start_block = block;

  page_id_t block_page_id = header_page->GetBlockPageId(block);
  HASH_TABLE_BLOCK_TYPE *block_page = GetBlockPage(block_page_id);

  bool found = false;
  while (block_page->IsOccupied(bucket_ind)) {
    if (block_page->IsReadable(bucket_ind) && 0 == comparator_(key, block_page->KeyAt(bucket_ind))) {
      found = true;
      result->push_back(block_page->ValueAt(bucket_ind));
    }
    if (++bucket_ind >= BLOCK_ARRAY_SIZE) {
      // At the end of the block, loading a new one in
      [[maybe_unused]] bool success = buffer_pool_manager_->UnpinPage(block_page_id, false, nullptr); assert(success);
      block = (block+1) % num_blocks;
      block_page_id = header_page->GetBlockPageId(block);
      block_page = GetBlockPage(block_page_id);
      bucket_ind = 0;
    }
    if (block == start_block && bucket_ind == start_bucket_ind) {
      break;
    }
  }
  assert(buffer_pool_manager_->UnpinPage(block_page_id, false, nullptr));
  assert(buffer_pool_manager_->UnpinPage(header_page_id_, false, nullptr));

  return found;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::Insert(Transaction *transaction, const KeyType &key,
                             const ValueType &value) {
  table_latch_.RLock();

  HashTableHeaderPage *header_page = GetHeaderPage();
  size_t num_blocks = header_page->NumBlocks();
  size_t size = header_page->GetSize();

  uint64_t hash = hash_fn_.GetHash(key);
  size_t bucket = hash % size;
  size_t block = bucket / BLOCK_ARRAY_SIZE;

  page_id_t block_page_id = header_page->GetBlockPageId(block);
  HASH_TABLE_BLOCK_TYPE *block_page = GetBlockPage(block_page_id);
  (reinterpret_cast<Page *>(block_page))->WLatch();

  std::vector<ValueType> vals;
  if (GetValueLatchFree(nullptr, key, &vals)) { // latch would require special-casing the first block
    if (std::find(vals.begin(), vals.end(), value) != vals.end()) {
      (reinterpret_cast<Page *>(block_page))->WUnlatch();
      table_latch_.RUnlock();
      return false;
    }
  }

  /*
   * complement here
   */

  size_t bucket_ind = bucket % BLOCK_ARRAY_SIZE;
  size_t start_bucket_ind = bucket_ind;
  size_t start_block = block;

  while (! block_page->Insert(bucket_ind, key, value)) {
    bucket_ind++;
    if (bucket_ind >= BLOCK_ARRAY_SIZE) {
      // At the end of the block, loading a new one in
      (reinterpret_cast<Page *>(block_page))->WUnlatch();
      [[maybe_unused]] bool success = buffer_pool_manager_->UnpinPage(block_page_id, false, nullptr); assert(success);
      block = (block+1) % num_blocks;
      block_page_id = header_page->GetBlockPageId(block);
      block_page = GetBlockPage(block_page_id);
      bucket_ind = 0;
      (reinterpret_cast<Page *>(block_page))->WLatch();

      if (GetValueLatchFree(nullptr, key, &vals)) {
        if (std::find(vals.begin(), vals.end(), value) != vals.end()) {
          (reinterpret_cast<Page *>(block_page))->WUnlatch();
          table_latch_.RUnlock();
          return false;
        }
      }
    }

    if (block == start_block && bucket_ind == start_bucket_ind) {
      [[maybe_unused]] bool success = false;
      
      (reinterpret_cast<Page *>(block_page))->WUnlatch();
      success = buffer_pool_manager_->UnpinPage(block_page_id, false, nullptr); assert(success);
      
      // Resize the table
      table_latch_.RUnlock();
      success = buffer_pool_manager_->UnpinPage(this->header_page_id_, false, nullptr); assert(success);
      Resize(size);
      table_latch_.RLock();
      
      // Retrieve new table metadata
      header_page = GetHeaderPage();
      num_blocks = header_page->NumBlocks();
      size = header_page->GetSize();
      
      // Recalculate position from original key-hash
      bucket = hash % size;
      block = bucket / BLOCK_ARRAY_SIZE;
      
      // Claim starting block page
      block_page_id = header_page->GetBlockPageId(block);
      block_page = GetBlockPage(block_page_id);
      (reinterpret_cast<Page *>(block_page))->WLatch();
      
      // Set index and wrap-around points
      bucket_ind = bucket % BLOCK_ARRAY_SIZE;
      start_bucket_ind = bucket_ind;
      start_block = block;
    }
  }
  (reinterpret_cast<Page *>(block_page))->WUnlatch();
  [[maybe_unused]] bool success;
  success = buffer_pool_manager_->UnpinPage(block_page_id, true, nullptr); assert(success);
  success = buffer_pool_manager_->UnpinPage(header_page_id_, false, nullptr); assert(success);

  table_latch_.RUnlock();

  // If this insert made the hash table full, resize

  if ((block * BLOCK_ARRAY_SIZE + bucket_ind + 1) % size == bucket) {
    Resize(size);
  }
  return true;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
bool HASH_TABLE_TYPE::Remove(Transaction *transaction, const KeyType &key,
                             const ValueType &value) {
  table_latch_.RLock();
  
  HashTableHeaderPage *header_page = GetHeaderPage();
  size_t num_blocks = header_page->NumBlocks();
  size_t size = header_page->GetSize();
  
  uint64_t hash = hash_fn_.GetHash(key);
  size_t bucket = hash % size;
  size_t block = bucket / BLOCK_ARRAY_SIZE;
  
  page_id_t block_page_id = header_page->GetBlockPageId(block);
  HASH_TABLE_BLOCK_TYPE *block_page = GetBlockPage(block_page_id);
  reinterpret_cast<Page *>(block_page)->WLatch();
  
  size_t bucket_ind = bucket % BLOCK_ARRAY_SIZE;
  size_t start_bucket_ind = bucket_ind;
  size_t start_block = block;
  
  bool removed = false;
  while (block_page->IsOccupied(bucket_ind)) {
    if (block_page->IsReadable(bucket_ind) && 0 == comparator_(block_page->KeyAt(bucket_ind), key)) {
      if (block_page->ValueAt(bucket_ind) == value) {
        block_page->Remove(bucket_ind);
        removed = true;
        break;
      }
    }
    
    // Not a success, so move on to the next bucket
    if (++bucket_ind >= BLOCK_ARRAY_SIZE) {
      // Move on to the next block as well
      reinterpret_cast<Page *>(block_page)->WUnlatch();
      [[maybe_unused]] bool success = buffer_pool_manager_->UnpinPage(block_page_id, false, nullptr); assert(success);
      block = (block+1) % num_blocks;
      block_page_id = header_page->GetBlockPageId(block);
      block_page = GetBlockPage(block_page_id);
      bucket_ind = 0;
      reinterpret_cast<Page *>(block_page)->WLatch();
    }
    
    if (block == start_block && bucket_ind == start_bucket_ind) {
      break;
    }
  }
  
  reinterpret_cast<Page *>(block_page)->WUnlatch();
  [[maybe_unused]] bool success;
  success = buffer_pool_manager_->UnpinPage(block_page_id, true, nullptr); assert(success);
  success = buffer_pool_manager_->UnpinPage(header_page_id_, false, nullptr); assert(success);
  
  table_latch_.RUnlock();
  return removed;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::Resize_Insert(HashTableHeaderPage *header_page,
                                    const KeyType &key,
                                    const ValueType &value) {
  // Table is write-locked before call
  uint64_t hash = hash_fn_.GetHash(key);
  size_t bucket = hash;
  bucket %= (header_page->GetSize());
  size_t block = bucket / BLOCK_ARRAY_SIZE;

  size_t num_blocks = header_page->NumBlocks();

  page_id_t block_page_id = header_page->GetBlockPageId(block);

  HASH_TABLE_BLOCK_TYPE *block_page = GetBlockPage(block_page_id);

  size_t bucket_ind = bucket % BLOCK_ARRAY_SIZE;

  while (! block_page->Insert(bucket_ind, key, value)) {
    if (++bucket_ind >= BLOCK_ARRAY_SIZE) {
      // At the end of the block, loading a new one in. Same as above.
      [[maybe_unused]] bool success = buffer_pool_manager_->UnpinPage(block_page_id, false, nullptr); assert(success);
      block = (block+1) % num_blocks;
      block_page_id = header_page->GetBlockPageId(block);
      block_page = GetBlockPage(block_page_id);
      bucket_ind = 0;
    }
  }
  reinterpret_cast<Page *>(block_page)->WUnlatch();
  assert(buffer_pool_manager_->UnpinPage(block_page_id, true, nullptr));
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::DeleteBlockPages(HashTableHeaderPage *old_header_page) {
  size_t num_blocks = old_header_page->NumBlocks();
  for (size_t i = 0; i < num_blocks; i++) {
    page_id_t block_page_id = old_header_page->GetBlockPageId(i);
    if (!(buffer_pool_manager_->DeletePage(block_page_id, nullptr))) {
      LOG_DEBUG("Block %zu is still pinned.", i);
    }
  }
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::CreateNewBlockPages(HashTableHeaderPage *header_page,
                                          size_t num_blocks) {
  // TODO: create all the necessary blocks, and store page_ids in the header file
  for (size_t i = 0; i < num_blocks; i++) {
    page_id_t new_block_page_id = INVALID_PAGE_ID;
    Page *page = buffer_pool_manager_->NewPage(&new_block_page_id, nullptr); assert(page != nullptr);
    GetHeaderPage()->AddBlockPageId(new_block_page_id);
    [[maybe_unused]] bool success = buffer_pool_manager_->UnpinPage(new_block_page_id, true, nullptr); assert(success);
  }
}

/*****************************************************************************
 * RESIZE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::Resize(size_t initial_size) {
  table_latch_.WLock();
  page_id_t old_header_page_id = header_page_id_;
  HashTableHeaderPage *old_header_page = GetHeaderPage();
  size_t size = old_header_page->GetSize();
  size_t old_num_blocks = old_header_page->NumBlocks();
  if (size != initial_size) {
    assert(buffer_pool_manager_->UnpinPage(header_page_id_, false, nullptr));
    table_latch_.WUnlock();
    return;
  }
  size *= 2;
  auto new_header_page = reinterpret_cast<HashTableHeaderPage *>(
      buffer_pool_manager_->NewPage(&this->header_page_id_, nullptr)->GetData());
  new_header_page->SetPageId(header_page_id_);
  new_header_page->SetSize(size);
  // no need to write-lock it, we're the only ones who know its page_id

  // ceiling of size/BLOCK_ARRAY_SIZE
  size_t num_blocks = (size - 1) / BLOCK_ARRAY_SIZE + 1;

  CreateNewBlockPages(new_header_page, num_blocks);
  page_id_t block_page_id = old_header_page->GetBlockPageId(0);
  HASH_TABLE_BLOCK_TYPE *block_page = GetBlockPage(block_page_id);

  size_t bucket_ind = 0;
  size_t block = 0;

  // run through every index of the old hash table
  for (size_t bucket = 0; bucket < initial_size; bucket++) {
    if (block_page->IsReadable(bucket_ind)) {
      Resize_Insert(new_header_page, block_page->KeyAt(bucket_ind), block_page->ValueAt(bucket_ind));
    }

    if (++bucket_ind >= BLOCK_ARRAY_SIZE) {
      // At the end of the block, loading a new one in. Same as above.
      block = (block+1) % old_num_blocks;
      block_page_id = old_header_page->GetBlockPageId(block);
      block_page = GetBlockPage(block_page_id);
      bucket_ind = 0;
    }
  }

  [[maybe_unused]] bool success;
  success = buffer_pool_manager_->UnpinPage(block_page_id, false, nullptr); assert(success);
  DeleteBlockPages(old_header_page);

  success = buffer_pool_manager_->UnpinPage(header_page_id_, true, nullptr); assert(success);

  success = buffer_pool_manager_->UnpinPage(old_header_page_id, false, nullptr); assert(success);
  success = buffer_pool_manager_->DeletePage(old_header_page_id, nullptr);
  if (! success) {
    LOG_DEBUG("Old header page still pinned, page_id:%d", old_header_page_id);
  }

  table_latch_.WUnlock();
}

/*****************************************************************************
 * GETSIZE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
size_t HASH_TABLE_TYPE::GetSize() {
  table_latch_.RLock();
  size_t size = 0;
  size = GetHeaderPage()->GetSize();
  [[maybe_unused]] bool success = buffer_pool_manager_->UnpinPage(header_page_id_, false, nullptr); assert(success);
  table_latch_.RUnlock();
  return size;
}

template class LinearProbeHashTable<int, int, IntComparator>;

template class LinearProbeHashTable<GenericKey<4>, RID, GenericComparator<4>>;
template class LinearProbeHashTable<GenericKey<8>, RID, GenericComparator<8>>;
template class LinearProbeHashTable<GenericKey<16>, RID, GenericComparator<16>>;
template class LinearProbeHashTable<GenericKey<32>, RID, GenericComparator<32>>;
template class LinearProbeHashTable<GenericKey<64>, RID, GenericComparator<64>>;

} // namespace bustub
