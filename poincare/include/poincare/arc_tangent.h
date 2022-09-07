#ifndef POINCARE_ARC_TANGENT_H
#define POINCARE_ARC_TANGENT_H

#include <poincare/approximation_helper.h>
#include <poincare/expression.h>
#include <poincare/trigonometry.h>

namespace Poincare {

class ArcTangentNode final : public ExpressionNode {
public:

  // TreeNode
  size_t size() const override { return sizeof(ArcTangentNode); }
  int numberOfChildren() const override;
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "ArcTangent";
  }
#endif

  // Properties
  Sign sign(Context * context) const override { return childAtIndex(0)->sign(context); }
  NullStatus nullStatus(Context * context) const override { return childAtIndex(0)->nullStatus(context); }
  Type type() const override { return Type::ArcTangent; }

private:
  // Layout
  Layout createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  // Simplification
  Expression shallowReduce(ReductionContext reductionContext) override;
  LayoutShape leftLayoutShape() const override { return LayoutShape::MoreLetters; };
  LayoutShape rightLayoutShape() const override { return LayoutShape::BoundaryPunctuation; }

  //Evaluation
  template<typename T> static Complex<T> computeOnComplex(const std::complex<T> c, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit);
  Evaluation<float> approximate(SinglePrecision p, ApproximationContext approximationContext) const override {
    return ApproximationHelper::Map<float>(this, approximationContext, computeOnComplex<float>);
  }
  Evaluation<double> approximate(DoublePrecision p, ApproximationContext approximationContext) const override {
    return ApproximationHelper::Map<double>(this, approximationContext, computeOnComplex<double>);
  }
};

class ArcTangent final : public Expression {
public:
  ArcTangent(const ArcTangentNode * n) : Expression(n) {}
  static ArcTangent Builder(Expression child) { return TreeHandle::FixedArityBuilder<ArcTangent, ArcTangentNode>({child}); }

  static constexpr Expression::FunctionHelper s_functionHelper = Expression::FunctionHelper("atan", 1, &UntypedBuilderOneChild<ArcTangent>);

  Expression shallowReduce(ExpressionNode::ReductionContext reductionContext);
};

}

#endif
