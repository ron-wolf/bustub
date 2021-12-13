//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager.h"

namespace bustub {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  delete[] pages_;
  delete replacer_;
}

Page *BufferPoolManager::FetchPageImpl(page_id_t page_id) {
  mutex_.lock();
  frame_id_t frame_id;
  Page *page;
  
  if (page_table_.find(page_id) != page_table_.end()) {
    // Just grab the existing page
    frame_id = page_table_[page_id];
    page = &pages_[frame_id];
    
  } else {
    // Get a page we can overwrite
    page = GetUnpinnedPage(&frame_id);
    if (page == nullptr) { 
      mutex_.unlock();
      return nullptr; }
    
    // Complete any pending writes on the page before wiping it
    if (page->is_dirty_) {
      disk_manager_->WritePage(page->page_id_, page->data_);
      page->is_dirty_ = false;
    }
    page->ResetMemory();
    
    // Give the page a new identity, and publish it in the page table
    page->page_id_ = page_id;
    disk_manager_->ReadPage(page->page_id_, page->data_);
    page_table_[page->page_id_] = frame_id;
  }
  
  // Pin the page
  replacer_->Pin(frame_id);
  ++page->pin_count_;
  mutex_.unlock();
  return page;
}

bool BufferPoolManager::UnpinPageImpl(page_id_t page_id, bool is_dirty) {
  mutex_.lock();
  // Get the page and ensure its pin count can be reduced
  auto frame_id = page_table_[page_id];
  Page *page = &pages_[frame_id];
  page->is_dirty_ |= is_dirty;
  if (page->pin_count_-1 < 0) { 
    mutex_.unlock();
    return false; }
  
  // Reduce its pin count and, if non-positive, unpin it in the replacer
  if (--page->pin_count_ <= 0) {
    replacer_->Unpin(frame_id);
  }
  mutex_.unlock();
  return true;
}

bool BufferPoolManager::FlushPageImpl(page_id_t page_id) {
  mutex_.lock();
  // Get the page
  if (page_table_.find(page_id) == page_table_.end()) { return false; }
  auto frame_id = page_table_[page_id];
  Page *page = &pages_[frame_id];
  BUSTUB_ASSERT(page->page_id_ == page_id, "the page table must record a frame's correct page-id");
  
  // Flush the page
  disk_manager_->WritePage(page->page_id_, page->data_);
  page->is_dirty_ = false;
  mutex_.unlock();
  return true;
}

Page *BufferPoolManager::NewPageImpl(page_id_t *page_id) {
  mutex_.lock();
  // Get a page we can overwrite
  frame_id_t frame_id;
  Page *page = GetUnpinnedPage(&frame_id);
  if (page == nullptr) { 
    mutex_.unlock();
    return nullptr; }
  
  // Complete any pending writes on the page before wiping it
  if (page->is_dirty_) {
    disk_manager_->WritePage(page->page_id_, page->data_);
    page->is_dirty_ = false;
  }
  page->ResetMemory();
  
  // Give the page a new identity, and publish it in the page table
  page->page_id_ = disk_manager_->AllocatePage();
  page_table_[page->page_id_] = frame_id;
  page->pin_count_ = 0;
  page->is_dirty_ = false;
  page->ResetMemory();


  // Pin the page
  ++page->pin_count_;
  *page_id = page->page_id_;
  mutex_.unlock();
  return page;
}

bool BufferPoolManager::DeletePageImpl(page_id_t page_id) {
  // Get the page
  mutex_.lock();
  if (page_table_.find(page_id) == page_table_.end()) { 
    mutex_.unlock();
    return true; }
  auto frame_id = page_table_[page_id];
  Page *page = &pages_[frame_id];
  if(page->pin_count_ != 0) {
    mutex_.unlock();
    return false;
  }
  replacer_->Pin(frame_id);
  page_table_.erase(page_id);
  page->page_id_ = INVALID_PAGE_ID;
  page->ResetMemory();
  page->is_dirty_ = false;
  free_list_.push_front(frame_id);
  disk_manager_->DeallocatePage(page_id);
  mutex_.unlock();
  return true;
}

void BufferPoolManager::FlushAllPagesImpl() {
  mutex_.lock();
  for (auto kv : page_table_) {
    auto [page_id, frame_id] = kv;
    Page *page = &pages_[frame_id];
    BUSTUB_ASSERT(page->page_id_ == page_id, "The page table must correctly record page IDs");
    if (page->is_dirty_) {
      // It needs to be flushed
      disk_manager_->WritePage(page->page_id_, page->data_);
      page->is_dirty_ = false;
    }
  }
  mutex_.unlock();
}

Page *BufferPoolManager::GetUnpinnedPage(frame_id_t *frame_id) {
  Page *page = nullptr;
  if (! free_list_.empty()) {
    // Pop it off the front of the free list
    *frame_id = free_list_.front();
    page = &pages_[*frame_id];
    BUSTUB_ASSERT(page->pin_count_ <= 0, "Pages on the free list may no longer be pinned");
    free_list_.pop_front();
    
  } else if (replacer_->Victim(frame_id)) {
    // Remove it from the page table
    page = &pages_[*frame_id];
    BUSTUB_ASSERT(page->pin_count_ <= 0, "Pinned pages are not to be evicted");
    (void) page_table_.erase(page->page_id_);
  }
  BUSTUB_ASSERT(page == nullptr || page->pin_count_ >= 0, "a valid page has a whole number of pins");
  return page;
}

}  // namespace bustub
