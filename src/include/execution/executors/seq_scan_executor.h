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

#include <iomanip> // std::quoted(char *)
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
      : AbstractExecutor(exec_ctx),
        table_data_ {exec_ctx_->GetCatalog()->GetTable(plan->GetTableOid())},
        in_schema_ {&table_data_->schema_},
        cur_itr_ {table_data_->table_->Begin(exec_ctx_->GetTransaction())},
        end_itr_ {table_data_->table_->End()},
        pred_ {plan->GetPredicate()},
        out_schema_ {plan->OutputSchema()} {}

  /** initializes the sequential scan executor. */
  void Init() override {}

  /** Determines the next tuple in the table.
   * @return true if tuple was found, false otherwise.
   */
  bool Next(Tuple *tuple) override {
    if (cur_itr_ == end_itr_) {
      // We are being called again for some reason; complain and identify table
      // Also give output schema, to aid in finding location of error
      std::ostringstream quoted_tabnam; // a sink for the quoted table name
      quoted_tabnam << std::quoted(table_data_->name_); // stream the quoted table name in
      LOG_ERROR("Iterator already reached end of table %s (oid=%u) -- %s", quoted_tabnam.str().c_str(), table_data_->oid_, out_schema_->ToString().c_str());
    }
    
    bool found = false;
    for (; cur_itr_ != end_itr_ && !(found); ++cur_itr_) {
      const Tuple *cand = TupPtrOf(cur_itr_);
      if (!pred_ || pred_->Evaluate(cand, in_schema_).GetAs<bool>()) {
        // We have found a matching tuple!
        // Collect all the fields into a Tuple corresponding to the output Schema
        std::vector<Value> fields; fields.reserve( out_schema_->GetColumnCount());
        for (const Column& out_col : out_schema_->GetColumns()) {
          fields.push_back(out_col.GetExpr()->Evaluate(cand, out_schema_));
        }
        *tuple = Tuple(fields, out_schema_);
        found = true;
      }
    }
    return found; // if true, there may be more; if false, the executor has stopped
  }

  const Schema *GetOutputSchema() override { return out_schema_; }

 private:
  TableMetadata *table_data_;
  const Schema *in_schema_;
  
  TableIterator cur_itr_;
  const TableIterator end_itr_;
  
  const AbstractExpression *pred_;
  const Schema *out_schema_;
  
  Tuple *TupPtrOf(TableIterator itr) { return itr.operator->(); }
};
}  // namespace bustub
