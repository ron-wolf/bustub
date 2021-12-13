//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.h
//
// Identification: src/include/execution/executors/seq_scan_executor.h
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>

#include "common/logger.h"
#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/seq_scan_plan.h"
#include "storage/table/tuple.h"

namespace bustub {

/**
 * SeqScanExecutor executes a sequential scan over a table.
 */
class SeqScanExecutor : public AbstractExecutor {
 public:
  /**
   * Creates a new sequential scan executor.
   * @param exec_ctx the executor context
   * @param plan the sequential scan plan to be executed
   */
  SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan)
      : AbstractExecutor{exec_ctx},
        plan_{plan},
        tabledata_{exec_ctx_->GetCatalog()->GetTable(plan_->GetTableOid())},
        table_{tabledata_->table_.get()},
        pred_{plan_->GetPredicate()} {
    BUSTUB_ASSERT(plan->GetPredicate()->GetReturnType() == TypeId::BOOLEAN, "sequential scans are intended for boolean predicates only");
    
    /*
     * plan :: PlanNode
     * * predicate :: Expr
     * * * children :: [Expr]
     * * table_oid :: u32
     * exec_ctx :: ExecutorContext
     * * catalog :: Catalog
     * * * tables :: u32 -> TableMetadata
     * * * * schema :: Schema
     * * * * table :: TableHeap // <--
     * * bpm :: BufferPoolManager
     * * * NewPage :: IO Page
     * * * FetchPage :: i32 -> IO Page
     * * transaction :: Transaction
     */
  }

  void Init() override {
    itr_ = table_->Begin(exec_ctx_->GetTransaction());
  }

  bool Next(Tuple *tuple) override {
    // Process the next tuple in the pipeline
    Tuple *tuple_ptr = itr_.operator->();
    Value match_bool = plan_->GetPredicate()->Evaluate(tuple_ptr, &tabledata_->schema_);
    if (! match_bool.CheckComparable(true_bool)) {
      LOG_ERROR("Non boolean-comparable value resulted from predicate; erroring out");
      match_bool = match_bool.CastAs(TypeId::BOOLEAN); // this will throw an error every tme
    }
    itr_++;
    
    // Return a result to the caller
    bool match = IsTruthy(match_bool);
    if (match) {
      *tuple = *tuple_ptr;
    }
    return match;
  }

  const Schema *GetOutputSchema() override { return plan_->OutputSchema(); }

 private:
  /** The sequential scan plan node to be executed. */
  const SeqScanPlanNode *plan_;
  
  TableMetadata *tabledata_;
  TableHeap *table_;
  TableIterator itr_ {nullptr, RID(), nullptr};
  const AbstractExpression *pred_;
  
  static inline const Value true_bool {TypeId::BOOLEAN, true};
  
  static inline bool IsTruthy(Value &value) {
    return value.CompareEquals(true_bool) == CmpBool::CmpTrue; // what a rat's nest
  }
};
}  // namespace bustub
