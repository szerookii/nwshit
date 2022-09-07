#include <poincare/vector_cross.h>
#include <poincare/division.h>
#include <poincare/layout_helper.h>
#include <poincare/matrix.h>
#include <poincare/serialization_helper.h>
#include <poincare/undefined.h>

namespace Poincare {

constexpr Expression::FunctionHelper VectorCross::s_functionHelper;

int VectorCrossNode::numberOfChildren() const { return VectorCross::s_functionHelper.numberOfChildren(); }

Expression VectorCrossNode::shallowReduce(ReductionContext reductionContext) {
  return VectorCross(this).shallowReduce(reductionContext);
}

Layout VectorCrossNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Prefix(VectorCross(this), floatDisplayMode, numberOfSignificantDigits, VectorCross::s_functionHelper.name());
}

int VectorCrossNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return SerializationHelper::Prefix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, VectorCross::s_functionHelper.name());
}

template<typename T>
Evaluation<T> VectorCrossNode::templatedApproximate(ApproximationContext approximationContext) const {
  Evaluation<T> input0 = childAtIndex(0)->approximate(T(), approximationContext);
  Evaluation<T> input1 = childAtIndex(1)->approximate(T(), approximationContext);
  return input0.cross(&input1);
}


Expression VectorCross::shallowReduce(ExpressionNode::ReductionContext reductionContext) {
  {
    Expression e = Expression::defaultShallowReduce();
    e = e.defaultHandleUnitsInChildren();
    if (e.isUndefined()) {
      return e;
    }
  }
  Expression c0 = childAtIndex(0);
  Expression c1 = childAtIndex(1);
  if (c0.type() == ExpressionNode::Type::Matrix && c1.type() == ExpressionNode::Type::Matrix) {
    Matrix matrixChild0 = static_cast<Matrix&>(c0);
    Matrix matrixChild1 = static_cast<Matrix&>(c1);
    // Cross product is defined between two vectors of same type and of size 3
    if (matrixChild0.vectorType() == Array::VectorType::None || matrixChild0.vectorType() != matrixChild1.vectorType() || matrixChild0.numberOfChildren() != 3 || matrixChild1.numberOfChildren() != 3) {
      return replaceWithUndefinedInPlace();
    }
    Expression a = matrixChild0.cross(&matrixChild1, reductionContext);
    replaceWithInPlace(a);
    return a.shallowReduce(reductionContext);
  }
  if (c0.deepIsMatrix(reductionContext.context()) && c1.deepIsMatrix(reductionContext.context())) {
    return *this;
  }
  return replaceWithUndefinedInPlace();
}

}
