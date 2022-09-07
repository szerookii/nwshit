#include <poincare/sine.h>
#include <poincare/complex.h>
#include <poincare/cosine.h>
#include <poincare/derivative.h>
#include <poincare/layout_helper.h>
#include <poincare/multiplication.h>
#include <poincare/serialization_helper.h>

#include <cmath>

namespace Poincare {

constexpr Expression::FunctionHelper Sine::s_functionHelper;

int SineNode::numberOfChildren() const { return Sine::s_functionHelper.numberOfChildren(); }

template<typename T>
Complex<T> SineNode::computeOnComplex(const std::complex<T> c, Preferences::ComplexFormat, Preferences::AngleUnit angleUnit) {
  std::complex<T> angleInput = Trigonometry::ConvertToRadian(c, angleUnit);
  std::complex<T> res = std::sin(angleInput);
  return Complex<T>::Builder(ApproximationHelper::NeglectRealOrImaginaryPartIfNeglectable(res, angleInput));
}

Layout SineNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(Sine(this), floatDisplayMode, numberOfSignificantDigits, Sine::s_functionHelper.name());
}

int SineNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, Sine::s_functionHelper.name());
}

Expression SineNode::shallowReduce(ReductionContext reductionContext) {
  return Sine(this).shallowReduce(reductionContext);
}

bool SineNode::derivate(ReductionContext reductionContext, Expression symbol, Expression symbolValue) {
  return Sine(this).derivate(reductionContext, symbol, symbolValue);
}

Expression SineNode::unaryFunctionDifferential(ReductionContext reductionContext) {
  return Sine(this).unaryFunctionDifferential(reductionContext);
}

Expression Sine::shallowReduce(ExpressionNode::ReductionContext reductionContext) {
  {
    Expression e = Expression::defaultShallowReduce();
    e = e.defaultHandleUnitsInChildren();
    if (e.isUndefined()) {
      return e;
    }
  }
  return Trigonometry::shallowReduceDirectFunction(*this, reductionContext);
}

bool Sine::derivate(ExpressionNode::ReductionContext reductionContext, Expression symbol, Expression symbolValue) {
  Derivative::DerivateUnaryFunction(*this, symbol, symbolValue, reductionContext);
  return true;
}

Expression Sine::unaryFunctionDifferential(ExpressionNode::ReductionContext reductionContext) {
  return Multiplication::Builder(Trigonometry::UnitConversionFactor(reductionContext.angleUnit(), Preferences::AngleUnit::Radian), Cosine::Builder(childAtIndex(0).clone()));
}

}
