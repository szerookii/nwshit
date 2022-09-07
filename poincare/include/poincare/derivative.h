#ifndef POINCARE_DERIVATIVE_H
#define POINCARE_DERIVATIVE_H

#include <poincare/parametered_expression.h>
#include <poincare/symbol.h>
#include <poincare/variable_context.h>

namespace Poincare {

class DerivativeNode final : public ParameteredExpressionNode {
public:

  // TreeNode
  size_t size() const override { return sizeof(DerivativeNode); }
  int numberOfChildren() const override;
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "Derivative";
  }
#endif

  // Properties
  Type type() const override { return Type::Derivative; }
  int polynomialDegree(Context * context, const char * symbolName) const override;

private:
  // Layout
  Layout createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;

  // Simplification
  Expression shallowReduce(ReductionContext reductionContext) override;
  LayoutShape leftLayoutShape() const override { return LayoutShape::MoreLetters; };
  LayoutShape rightLayoutShape() const override { return LayoutShape::BoundaryPunctuation; }

  // Evaluation
  Evaluation<float> approximate(SinglePrecision p, ApproximationContext approximationContext) const override { return templatedApproximate<float>(approximationContext); }
  Evaluation<double> approximate(DoublePrecision p, ApproximationContext approximationContext) const override { return templatedApproximate<double>(approximationContext); }
  template<typename T> Evaluation<T> templatedApproximate(ApproximationContext approximationContext) const;
  template<typename T> T approximateWithArgument(T x, ApproximationContext approximationContext) const;
  template<typename T> T growthRateAroundAbscissa(T x, T h, ApproximationContext approximationContext) const;
  template<typename T> T riddersApproximation(ApproximationContext approximationContext, T x, T h, T * error) const;
  // TODO: Change coefficients?
  constexpr static double k_maxErrorRateOnApproximation = 0.001;
  constexpr static double k_minInitialRate = 0.01;
  constexpr static double k_rateStepSize = 1.4;
};

class Derivative final : public ParameteredExpression {
public:
  Derivative(const DerivativeNode * n) : ParameteredExpression(n) {}
  static Derivative Builder(Expression child0, Symbol child1, Expression child2) { return TreeHandle::FixedArityBuilder<Derivative, DerivativeNode>({child0, child1, child2}); }
  static Expression UntypedBuilder(Expression children);
  static constexpr Expression::FunctionHelper s_functionHelper = Expression::FunctionHelper("diff", 3, &UntypedBuilder);
  static void DerivateUnaryFunction(Expression function, Expression symbol, Expression symbolValue, ExpressionNode::ReductionContext reductionContext);

  Expression shallowReduce(ExpressionNode::ReductionContext context);
};

}

#endif
