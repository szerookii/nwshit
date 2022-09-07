#ifndef POINCARE_IMAGINARY_PART_H
#define POINCARE_IMAGINARY_PART_H

#include <poincare/approximation_helper.h>
#include <poincare/expression.h>

namespace Poincare {

class ImaginaryPartNode final : public ExpressionNode  {
public:

  // TreeNode
  size_t size() const override { return sizeof(ImaginaryPartNode); }
  int numberOfChildren() const override;
#if POINCARE_TREE_LOG
  void logNodeName(std::ostream & stream) const override {
    stream << "ImaginaryPart";
  }
#endif

  // Properties
  NullStatus nullStatus(Context * context) const override { return childAtIndex(0)->sign(context) != Sign::Unknown ? NullStatus::Null : NullStatus::Unknown; }
  Type type() const override { return Type::ImaginaryPart; }

private:
  // Layout
  Layout createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  // Simplification
  Expression shallowReduce(ReductionContext reductionContext) override;
  LayoutShape leftLayoutShape() const override { return LayoutShape::MoreLetters; };
  LayoutShape rightLayoutShape() const override { return LayoutShape::BoundaryPunctuation; }
  // Evaluation
  template<typename T> static Complex<T> computeOnComplex(const std::complex<T> c, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) {
    return Complex<T>::Builder(std::imag(c));
  }
  Evaluation<float> approximate(SinglePrecision p, ApproximationContext approximationContext) const override {
    return ApproximationHelper::Map<float>(this, approximationContext, computeOnComplex<float>);
  }
  Evaluation<double> approximate(DoublePrecision p, ApproximationContext approximationContext) const override {
    return ApproximationHelper::Map<double>(this, approximationContext, computeOnComplex<double>);
  }
};

class ImaginaryPart final : public Expression {
public:
  ImaginaryPart(const ImaginaryPartNode * n) : Expression(n) {}
  static ImaginaryPart Builder(Expression child) { return TreeHandle::FixedArityBuilder<ImaginaryPart, ImaginaryPartNode>({child}); }

  static constexpr Expression::FunctionHelper s_functionHelper = Expression::FunctionHelper("im", 1, &UntypedBuilderOneChild<ImaginaryPart>);

  Expression shallowReduce(ExpressionNode::ReductionContext reductionContext);
};

}

#endif
