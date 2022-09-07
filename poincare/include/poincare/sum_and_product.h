#ifndef POINCARE_SUM_AND_PRODUCT_H
#define POINCARE_SUM_AND_PRODUCT_H

#include <poincare/parametered_expression.h>
#include <poincare/symbol.h>
#include <poincare/approximation_helper.h>

namespace Poincare {

class SumAndProductNode : public ParameteredExpressionNode {
public:
  int numberOfChildren() const override { return 4; }
  virtual void deepReduceChildren(ReductionContext reductionContext) override;
  virtual Expression deepReplaceReplaceableSymbols(Context * context, bool * didReplace, bool replaceFunctionsOnly, int parameteredAncestorsCount) override;
private:
  Layout createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  virtual Layout createSumAndProductLayout(Layout argumentLayout, Layout symbolLayout, Layout subscriptLayout, Layout superscriptLayout) const = 0;
  // Simplication
  Expression shallowReduce(ReductionContext reductionContext) override;
  LayoutShape leftLayoutShape() const override { return LayoutShape::BoundaryPunctuation; };
  /* Approximation */
  Evaluation<float> approximate(SinglePrecision p, ApproximationContext approximationContext) const override { return templatedApproximate<float>(approximationContext); }
  Evaluation<double> approximate(DoublePrecision p, ApproximationContext approximationContext) const override { return templatedApproximate<double>(approximationContext); }
 template<typename T> Evaluation<T> templatedApproximate(ApproximationContext approximationContext) const;
  virtual float emptySumAndProductValue() const = 0;
  virtual Evaluation<float> evaluateWithNextTerm(SinglePrecision p, Evaluation<float> a, Evaluation<float> b, Preferences::ComplexFormat complexFormat) const = 0;
  virtual Evaluation<double> evaluateWithNextTerm(DoublePrecision p, Evaluation<double> a, Evaluation<double> b, Preferences::ComplexFormat complexFormat) const = 0;
};

class SumAndProduct : public Expression {
public:
  SumAndProduct(const SumAndProductNode * n) : Expression(n) {}
  Expression deepReplaceReplaceableSymbols(Context * context, bool * didReplace, bool replaceFunctionsOnly, int parameteredAncestorsCount);
  Expression shallowReduce(Context * context);
};

}

#endif