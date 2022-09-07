#ifndef POINCARE_COSINE_H
#define POINCARE_COSINE_H

#include <poincare/approximation_helper.h>
#include <poincare/expression.h>
#include <poincare/trigonometry.h>

namespace Poincare {

class CosineNode final : public ExpressionNode  {
public:

  // TreeNode
  size_t size() const override { return sizeof(CosineNode); }
  int numberOfChildren() const override;
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "Cosine";
  }
#endif

  // Properties
  Type type() const override { return Type::Cosine; }

  template<typename T> static Complex<T> computeOnComplex(const std::complex<T> c, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit = Preferences::AngleUnit::Radian);

private:
  // Layout
  Layout createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  // Simplication
  Expression shallowReduce(ReductionContext reductionContext) override;
  LayoutShape leftLayoutShape() const override { return LayoutShape::MoreLetters; };
  LayoutShape rightLayoutShape() const override { return LayoutShape::BoundaryPunctuation; }

  // Derivation
  bool derivate(ReductionContext reductionContext, Expression symbol, Expression symbolValue) override;
  Expression unaryFunctionDifferential(ReductionContext reductionContext) override;

  // Evaluation
  Evaluation<float> approximate(SinglePrecision p, ApproximationContext approximationContext) const override {
    return ApproximationHelper::Map<float>(this, approximationContext, computeOnComplex<float>);
  }
  Evaluation<double> approximate(DoublePrecision p, ApproximationContext approximationContext) const override {
    return ApproximationHelper::Map<double>(this, approximationContext, computeOnComplex<double>);
  }
};

class Cosine final : public Expression {
public:
  Cosine(const CosineNode * n) : Expression(n) {}
  static Cosine Builder(Expression child) { return TreeHandle::FixedArityBuilder<Cosine, CosineNode>({child}); }

  static constexpr Expression::FunctionHelper s_functionHelper = Expression::FunctionHelper("cos", 1, &UntypedBuilderOneChild<Cosine>);

  Expression shallowReduce(ExpressionNode::ReductionContext reductionContext);

  bool derivate(ExpressionNode::ReductionContext reductionContext, Expression symbol, Expression symbolValue);
  Expression unaryFunctionDifferential(ExpressionNode::ReductionContext reductionContext);
};

}

#endif
