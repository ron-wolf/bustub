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

#include "common/macros.h"
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
      : AbstractExecutor(exec_ctx)
      , table_data_ {exec_ctx_->GetCatalog()->GetTable(plan->GetTableOid())}
      , cur_itr_ {table_data_->table_->Begin(exec_ctx_->GetTransaction())}
      , end_itr_ {table_data_->table_->End()}
      , pred_ {plan->GetPredicate()}
      , in_schema_ {&table_data_->schema_}
      , out_schema_ {plan->OutputSchema()}
  {
    if (pred_ != nullptr) {
      BUSTUB_ASSERT(pred_->GetReturnType() == TypeId::BOOLEAN, "Plan predicates must return true or false");
    }
  }

  /** initializes the sequential scan executor. */
  void Init() override {}

  /** Determines the next tuple in the table.
   * @return true if tuple was found, false otherwise.
   */
  bool Next(Tuple *tuple) override {
    bool match = false;
    while (cur_itr_ != end_itr_) {
      const Tuple& cand = *cur_itr_;
      match = (pred_ == nullptr) || pred_->Evaluate(&cand, in_schema_).GetAs<bool>();
      
      if (match) {
        std::vector<Value> fields;
        fields.reserve(out_schema_->GetColumnCount());
        for (const Column& col : out_schema_->GetColumns()) {
          fields.push_back(col.GetExpr()->Evaluate(&cand, out_schema_));
        }
        *tuple = Tuple(fields, out_schema_);
      }
      ++cur_itr_;
      if (match) break;
    }
    return match;
  }

  const Schema *GetOutputSchema() override { return out_schema_; }

 private:
  TableMetadata *table_data_;
  TableIterator cur_itr_;
  const TableIterator end_itr_;
  const AbstractExpression *pred_;
  const Schema *in_schema_;
  const Schema *out_schema_;
};
}  // namespace bustub
