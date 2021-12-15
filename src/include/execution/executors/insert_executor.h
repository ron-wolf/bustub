//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// insert_executor.h
//
// Identification: src/include/execution/executors/insert_executor.h
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <utility>

#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/insert_plan.h"
#include "storage/table/tuple.h"

namespace bustub {
/**
 * InsertExecutor executes an insert into a table.
 * Inserted values can either be embedded in the plan itself ("raw insert") or come from a child executor.
 */
class InsertExecutor : public AbstractExecutor {
 public:
  /**
   * Creates a new insert executor.
   * @param exec_ctx the executor context
   * @param plan the insert plan to be executed
   * @param child_executor the child executor to obtain insert values from, can be nullptr
   */
  InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                 std::unique_ptr<AbstractExecutor> &&child_executor)
      : AbstractExecutor(exec_ctx)
      , plan_ {plan}
      , table_data_ {exec_ctx_->GetCatalog()->GetTable(plan_->TableOid())}
      , in_schema_ {plan_->OutputSchema()}
      , out_schema_ {&table_data_->schema_}
  {
    if (! plan_->IsRawInsert()) {
      child_plan_ = plan_->GetChildPlan();
      child_exec_.swap(child_executor);
    }
  }
  
  ~InsertExecutor() = default;

  const Schema *GetOutputSchema() override { return plan_->OutputSchema(); }

  void Init() override {
    if (! plan_->IsRawInsert()) {
      child_exec_->Init();
    }
  }

  // Note that Insert does not make use of the tuple pointer being passed in.
  // We return false if the insert failed for any reason, and return true if all inserts succeeded.
  bool Next([[maybe_unused]] Tuple *tuple) override {
    bool success = true;
    if (! plan_->IsRawInsert()) {
      Tuple in_tup {};
      while (child_exec_->Next(&in_tup)) {
        const Tuple out_tup = GetAsInsertableTuple(in_tup);
        success |= table_data_->table_->InsertTuple(out_tup, new RID(), exec_ctx_->GetTransaction());
      }
    } else {
      for (const std::vector<Value>& in_vals : plan_->RawValues()) {
        const Tuple in_tup {in_vals, out_schema_};
        const Tuple out_tup = GetAsInsertableTuple(in_tup);
        RID rid {};
        success |= table_data_->table_->InsertTuple(out_tup, &rid, exec_ctx_->GetTransaction());
        if (success) {
          LOG_INFO("Inserted in page %d, slot %u: %s", rid.GetPageId(), rid.GetSlotNum(), out_tup.ToString(out_schema_).c_str());
        } else {
          LOG_ERROR("Failed to insert tuple: %s", out_tup.ToString(out_schema_).c_str());
        }
      }
    }
    return success;
  }

 private:
  const InsertPlanNode *plan_;
  const TableMetadata *table_data_;
  
  const AbstractPlanNode *child_plan_;
  std::unique_ptr<AbstractExecutor> child_exec_ {};
  
  const Schema *in_schema_, *out_schema_;
  
 private:
  // WARN: this method is untested in the presence of differing schemata;
  //       it may have to be tested again
  inline const Tuple GetAsInsertableTuple(const Tuple& in_tup) {
    
    if (!in_schema_ || !out_schema_) return in_tup;
    
    const std::string in_desc = in_schema_->ToString();
    const std::string out_desc = out_schema_->ToString();
    if (in_desc == out_desc) {
      LOG_INFO("Identical input/output schemata: %s", out_desc.c_str());
      return in_tup;
    }
    
    LOG_ERROR("Differing input/output schemata\n\tInput schema: %s\n\tOutput schema: %s", in_desc.c_str(), out_desc.c_str());
    static std::vector<Value> out_vals {};
    out_vals.reserve(out_schema_->GetColumnCount());
    
    for (const Column& out_col : out_schema_->GetColumns()) {
      
      bool found = false;
      for (uint32_t incol_pos = 0; incol_pos < out_schema_->GetColumnCount(); ++incol_pos) {
        Column in_col = out_schema_->GetColumn(incol_pos);
        if (in_col.GetName().compare(out_col.GetName()) == 0) {
          found = true;
          Value val = in_tup.GetValue(plan_->OutputSchema(), incol_pos);
          if (val.GetTypeId() != out_col.GetType()) {
            val = val.CastAs(out_col.GetType()); // this will throw an error every time
          }
          out_vals.push_back(val);
        }
      }
      if (! found) {
        out_vals.emplace_back(out_col.GetType()); // add a null field
      }
      
    }
    
    return Tuple(out_vals, &table_data_->schema_);
    
  }
};
}  // namespace bustub
