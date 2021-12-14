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
      : bpm_{bpm}, lock_mgr_{lock_manager}, log_mgr_{log_manager} {}

  /**
   * Create a new table and return its metadata.
   * @param txn the transaction in which the table is being created
   * @param table_name the name of the new table
   * @param schema the schema of the new table
   * @return a pointer to the metadata of the new table
   */
  TableMetadata *CreateTable(Transaction *txn, const std::string& table_name, const Schema& schema) {
    BUSTUB_ASSERT(oid_of_name_.count(table_name) == 0, "Table names must be unique for querying to work");
    
    // Create another table
    table_oid_t table_oid = next_table_oid_++;
    const auto& [kv_pair, inserted] = table_of_oid_.emplace(table_oid, std::make_unique<TableMetadata>(
      schema,
      table_name,
      std::make_unique<TableHeap>(bpm_, lock_mgr_, log_mgr_, txn),
      table_oid
    ));
    BUSTUB_ASSERT(inserted, "The next_table_oid must always be unique, and should never wrap around");
    oid_of_name_.emplace(table_name, table_oid); // Map the table's name to its OID
    TableMetadata *meta = kv_pair->second.get(); // Retrieve the inserted table
    return meta;
  }

  /** @return table metadata by name */
  TableMetadata *GetTable(const std::string& table_name) { return GetTable(oid_of_name_.at(table_name)); }

  /** @return table metadata by oid */
  TableMetadata *GetTable(table_oid_t table_oid) { return table_of_oid_.at(table_oid).get(); }

 private:
  BufferPoolManager *bpm_;
  LockManager *lock_mgr_;
  LogManager *log_mgr_;

  /** table_of_oid_ : table identifiers -> table metadata. Note that table_of_oid_ owns all table metadata. */
  std::unordered_map<table_oid_t, std::unique_ptr<TableMetadata>> table_of_oid_;
  /** oid_of_name_ : table names -> table identifiers */
  std::unordered_map<std::string, table_oid_t> oid_of_name_;
  /** The next table identifier to be used. */
  std::atomic<table_oid_t> next_table_oid_{0};
};
}  // namespace bustub
