#ifndef SOLVER_EQUATION_h
#define SOLVER_EQUATION_h

#include "../shared/expression_model_handle.h"

namespace Solver {

class Equation : public Shared::ExpressionModelHandle {
public:
  Equation(Ion::Storage::Record record = Record()) : ExpressionModelHandle(record) {}
  Poincare::Expression standardForm(Poincare::Context * context, bool replaceFunctionsButNotSymbols, Poincare::ExpressionNode::ReductionTarget reductionTarget) const { return m_model.standardForm(this, context, replaceFunctionsButNotSymbols, reductionTarget); }
  bool containsIComplex(Poincare::Context * context) const;

private:
  class Model : public Shared::ExpressionModel {
  public:
    Poincare::Expression standardForm(const Ion::Storage::Record * record, Poincare::Context * context, bool replaceFunctionsButNotSymbols, Poincare::ExpressionNode::ReductionTarget reductionTarget) const;
  private:
    void * expressionAddress(const Ion::Storage::Record * record) const override;
    size_t expressionSize(const Ion::Storage::Record * record) const override;
  };
  size_t metaDataSize() const override { return 0; }
  const Shared::ExpressionModel * model() const override { return &m_model; }
  Model m_model;
};

}

#endif
