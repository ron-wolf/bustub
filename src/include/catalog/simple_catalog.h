#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "buffer/buffer_pool_manager.h"
#include "catalog/schema.h"
#include "storage/index/index.h"
#include "storage/table/table_heap.h"

namespace bustub {

/**
 * Typedefs
 */
using table_oid_t = uint32_t;
using column_oid_t = uint32_t;

/**
 * Metadata about a table.
 */
struct TableMetadata {
  TableMetadata(Schema schema, std::string name, std::unique_ptr<TableHeap> &&table, table_oid_t oid)
      : schema_(std::move(schema)), name_(std::move(name)), table_(std::move(table)), oid_(oid) {}
  Schema schema_;
  std::string name_;
  std::unique_ptr<TableHeap> table_;
  table_oid_t oid_;
};

/**
 * SimpleCatalog is a non-persistent catalog that is designed for the executor to use.
 * It handles table creation and table lookup.
 */
class SimpleCatalog {
 public:
  /**
   * Creates a new catalog object.
   * @param bpm the buffer pool manager backing tables created by this catalog
   * @param lock_manager the lock manager in use by the system
   * @param log_manager the log manager in use by the system
   */
  SimpleCatalog(BufferPoolManager *bpm, LockManager *lock_manager, LogManager *log_manager)
      : bpm_{bpm}, lock_manager_{lock_manager}, log_manager_{log_manager} {}

  /**
   * Create a new table and return its metadata.
   * @param txn the transaction in which the table is being created
   * @param table_name the name of the new table
   * @param schema the schema of the new table
   * @return a pointer to the metadata of the new table
   */
  TableMetadata *CreateTable(Transaction *txn, const std::string &table_name, const Schema &schema) {
    BUSTUB_ASSERT(names_.count(table_name) == 0, "Table names should be unique!");
    
    page_id_t first_page_id = INVALID_PAGE_ID;
    Page *p = bpm_->NewPage(&first_page_id, nullptr);
    BUSTUB_ASSERT(p != nullptr, "The buffer pool should not run out of space"); // TODO: find if there is a better error here
    
    table_oid_t table_oid = next_table_oid_.fetch_add(1);
    tables_.emplace(table_oid, std::make_unique<TableMetadata>(
      schema,
      table_name,
      std::make_unique<TableHeap>(bpm_, lock_manager_, log_manager_, first_page_id),
      table_oid
    ));
    names_.emplace(table_name, table_oid);
    
    TableMetadata *meta = GetTable(table_oid);
    return meta;
  }

  /** @return table metadata by name */
  TableMetadata *GetTable(const std::string &table_name) {
    table_oid_t table_oid = names_.at(table_name);
    return GetTable(table_oid);
  }

  /** @return table metadata by oid */
  TableMetadata *GetTable(table_oid_t table_oid) {
    TableMetadata *meta = tables_.at(table_oid).get();
    return meta;
  }

 private:
  BufferPoolManager *bpm_;
  LockManager *lock_manager_;
  LogManager *log_manager_;

  /** tables_ : table identifiers -> table metadata. Note that tables_ owns all table metadata. */
  std::unordered_map<table_oid_t, std::unique_ptr<TableMetadata>> tables_;
  /** names_ : table names -> table identifiers */
  std::unordered_map<std::string, table_oid_t> names_;
  /** The next table identifier to be used. */
  std::atomic<table_oid_t> next_table_oid_{0};
};
}  // namespace bustub
