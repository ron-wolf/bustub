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
  // TODO: complement the below line
  header_page->SetPageId(/* here */);

  // ceiling of num_buckets/BLOCK_ARRAY_SIZE
  size_t num_blocks = (num_buckets - 1) / BLOCK_ARRAY_SIZE + 1;

  // TODO: complement the below line
  header_page->SetSize(/* here */);

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
  // TODO: finish the below empty
  while (/* enter here */) {
    if (/* enter here */) {
      found = true;
      result->push_back(block_page->ValueAt(bucket_ind));
    }
    if (++bucket_ind >= BLOCK_ARRAY_SIZE) {
      (reinterpret_cast<Page *>(block_page))->RUnlatch();
      /* complete here */
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
  // TODO: finish the below empty, the same as GetValue() above.
  while (/* enter here */) {
    if (/* enter here */) {
      found = true;
      result->push_back(block_page->ValueAt(bucket_ind));
    }
    if (++bucket_ind >= BLOCK_ARRAY_SIZE) {
      // At the end of the block, loading a new one in
      /* complete here */
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

  // TODO: check if (key, value) already exists.
  //       If already exists, return false.

  /*
   * complement here
   */

  size_t bucket_ind = bucket % BLOCK_ARRAY_SIZE;
  size_t start_bucket_ind = bucket_ind;
  size_t start_block = block;

  while (!block_page->Insert(bucket_ind, key, value)) {
    bucket_ind++;
    if (bucket_ind >= BLOCK_ARRAY_SIZE) {
      // At the end of the block, loading a new one in
      (reinterpret_cast<Page *>(block_page))->WUnlatch();
      /*
       * complement here
       */
      (reinterpret_cast<Page *>(block_page))->WLatch();

      // TODO: check if (key, value) already exists.
      //       If already exists, return false.
      //       Same as above

      /*
       * complement here
       */
    }

    // TODO: If back to the starting point, resize
    if (block == start_block && bucket_ind == start_bucket_ind) {
      (reinterpret_cast<Page *>(block_page))->WUnlatch();
      /*
       * complement here
       */
      (reinterpret_cast<Page *>(block_page))->WLatch();
    }
  }
  (reinterpret_cast<Page *>(block_page))->WUnlatch();
  assert(buffer_pool_manager_->UnpinPage(block_page_id, true, nullptr));
  assert(buffer_pool_manager_->UnpinPage(header_page_id_, false, nullptr));

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
  // TODO: finish the whole function refer to the Insert() and GetValue()
  // function above.
  /*
   * code here
   */

  return false;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::Resize_Insert(HashTableHeaderPage *header_page,
                                    const KeyType &key,
                                    const ValueType &value) {
  uint64_t hash = hash_fn_.GetHash(key);
  size_t bucket = hash;
  bucket %= (header_page->GetSize());
  size_t block = bucket / BLOCK_ARRAY_SIZE;

  size_t num_blocks = header_page->NumBlocks();

  page_id_t block_page_id = header_page->GetBlockPageId(block);

  HASH_TABLE_BLOCK_TYPE *block_page = GetBlockPage(block_page_id);

  size_t bucket_ind = bucket % BLOCK_ARRAY_SIZE;

  while (!block_page->Insert(bucket_ind, key, value)) {
    if (++bucket_ind >= BLOCK_ARRAY_SIZE) {
      // At the end of the block, loading a new one in. Same as above.
      /*
       * complement here
       */
    }
  }
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
  // TODO: create all the necessary blocks, and store page_ids in the header
  // file
  for (size_t i = 0; i < num_blocks; i++) {
    page_id_t new_block_page_id;
    /*
     * code here
     */
    assert(buffer_pool_manager_->UnpinPage(new_block_page_id, true, nullptr));
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
      buffer_pool_manager_->NewPage(&header_page_id_, nullptr)->GetData());
  new_header_page->SetPageId(header_page_id_);
  new_header_page->SetSize(size);

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
      // TODO: insert the new key,value using Resize_Insert().
      /*
       * complement here
       */
    }

    if (++bucket_ind >= BLOCK_ARRAY_SIZE) {
      // At the end of the block, loading a new one in. Same as above.
      /*
       * complement here
       */
    }
  }

  assert(buffer_pool_manager_->UnpinPage(block_page_id, false, nullptr));
  DeleteBlockPages(old_header_page);

  assert(buffer_pool_manager_->UnpinPage(header_page_id_, true, nullptr));

  assert(buffer_pool_manager_->UnpinPage(old_header_page_id, false, nullptr));
  if (!(buffer_pool_manager_->DeletePage(old_header_page_id, nullptr))) {
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
  // TODO: get header page size
  /*
   * complement here
   */
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
