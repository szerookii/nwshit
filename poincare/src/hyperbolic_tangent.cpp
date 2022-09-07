#include <poincare/hyperbolic_tangent.h>
#include <poincare/derivative.h>
#include <poincare/hyperbolic_cosine.h>
#include <poincare/layout_helper.h>
#include <poincare/power.h>
#include <poincare/serialization_helper.h>

namespace Poincare {

constexpr Expression::FunctionHelper HyperbolicTangent::s_functionHelper;

Layout HyperbolicTangentNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(HyperbolicTangent(this), floatDisplayMode, numberOfSignificantDigits, HyperbolicTangent::s_functionHelper.name());
}

int HyperbolicTangentNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, HyperbolicTangent::s_functionHelper.name());
}

template<typename T>
Complex<T> HyperbolicTangentNode::computeOnComplex(const std::complex<T> c, Preferences::ComplexFormat, Preferences::AngleUnit angleUnit) {
  return Complex<T>::Builder(ApproximationHelper::NeglectRealOrImaginaryPartIfNeglectable(std::tanh(c), c));
}

bool HyperbolicTangentNode::derivate(ReductionContext reductionContext, Expression symbol, Expression symbolValue) {
  return HyperbolicTangent(this).derivate(reductionContext, symbol, symbolValue);
}

Expression HyperbolicTangentNode::unaryFunctionDifferential(ReductionContext reductionContext) {
  return HyperbolicTangent(this).unaryFunctionDifferential(reductionContext);
}

bool HyperbolicTangent::derivate(ExpressionNode::ReductionContext reductionContext, Expression symbol, Expression symbolValue) {
  Derivative::DerivateUnaryFunction(*this, symbol, symbolValue, reductionContext);
  return true;
}

Expression HyperbolicTangent::unaryFunctionDifferential(ExpressionNode::ReductionContext reductionContext) {
  return Power::Builder(HyperbolicCosine::Builder(childAtIndex(0).clone()), Rational::Builder(-2));
}

template Complex<float> Poincare::HyperbolicTangentNode::computeOnComplex<float>(std::complex<float>, Preferences::ComplexFormat, Preferences::AngleUnit);
template Complex<double> Poincare::HyperbolicTangentNode::computeOnComplex<double>(std::complex<double>, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit);

}
