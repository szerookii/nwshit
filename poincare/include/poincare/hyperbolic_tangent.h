#ifndef POINCARE_HYPERBOLIC_TANGENT_H
#define POINCARE_HYPERBOLIC_TANGENT_H

#include <poincare/approximation_helper.h>
#include <poincare/hyperbolic_trigonometric_function.h>

namespace Poincare {

class HyperbolicTangentNode final : public HyperbolicTrigonometricFunctionNode {
public:

  // TreeNode
  size_t size() const override { return sizeof(HyperbolicTangentNode); }
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "HyperbolicTangent";
  }
#endif

  // Properties
  Type type() const override { return Type::HyperbolicTangent; }
private:
  // Layout
  Layout createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  // Derivation
  bool derivate(ReductionContext reductionContext, Expression symbol, Expression symbolValue) override;
  Expression unaryFunctionDifferential(ReductionContext reductionContext) override;
  //Evaluation
  template<typename T> static Complex<T> computeOnComplex(const std::complex<T> c, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit);
  Evaluation<float> approximate(SinglePrecision p, ApproximationContext approximationContext) const override {
    return ApproximationHelper::Map<float>(this, approximationContext, computeOnComplex<float>);
  }
  Evaluation<double> approximate(DoublePrecision p, ApproximationContext approximationContext) const override {
    return ApproximationHelper::Map<double>(this, approximationContext, computeOnComplex<double>);
  }
};

class HyperbolicTangent final : public HyperbolicTrigonometricFunction {
public:
  HyperbolicTangent(const HyperbolicTangentNode * n) : HyperbolicTrigonometricFunction(n) {}
  static HyperbolicTangent Builder(Expression child) { return TreeHandle::FixedArityBuilder<HyperbolicTangent, HyperbolicTangentNode>({child}); }

  bool derivate(ExpressionNode::ReductionContext reductionContext, Expression symbol, Expression symbolValue);
  Expression unaryFunctionDifferential(ExpressionNode::ReductionContext reductionContext);

  static constexpr Expression::FunctionHelper s_functionHelper = Expression::FunctionHelper("tanh", 1, &UntypedBuilderOneChild<HyperbolicTangent>);
};

}

#endif
