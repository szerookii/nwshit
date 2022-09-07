#include "sequence.h"
#include "sequence_cache_context.h"
#include "sequence_store.h"
#include <poincare/layout_helper.h>
#include <poincare/serialization_helper.h>
#include <poincare/code_point_layout.h>
#include <poincare/sum.h>
#include <poincare/vertical_offset_layout.h>
#include <poincare/integer.h>
#include <poincare/rational.h>
#include <poincare/addition.h>
#include <poincare/zoom.h>
#include "../shared/poincare_helpers.h"
#include <string.h>
#include <apps/i18n.h>
#include <cmath>

using namespace Poincare;

namespace Shared {

I18n::Message Sequence::parameterMessageName() const {
  return I18n::Message::N;
}

int Sequence::nameWithArgument(char * buffer, size_t bufferSize) {
  int seqNameSize = name(buffer, bufferSize);
  assert(seqNameSize > 0);
  size_t result = seqNameSize;
  assert(result <= bufferSize);
  buffer[result++] = '(';
  assert(result <= bufferSize);
  assert(UTF8Decoder::CharSizeOfCodePoint(symbol()) <= 2);
  result += UTF8Decoder::CodePointToChars(symbol(), buffer+result, bufferSize-result);
  assert(result <= bufferSize);
  result += strlcpy(buffer+result, ")", bufferSize-result);
  return result;
}

int Sequence::nameWithArgumentAndType(char * buffer, size_t bufferSize) {
  int result = nameWithArgument(buffer, bufferSize);
  assert(result >= 1);
  int offset = result - 1;
  switch (type())
  {
  case Type::SingleRecurrence:
    result += strlcpy(buffer+offset, "+1)", bufferSize-offset);
    break;
  case Type::DoubleRecurrence:
    result += strlcpy(buffer+offset, "+2)", bufferSize-offset);
    break;
  default:
    break;
  }
  return result;
}

Sequence::Type Sequence::type() const {
  return recordData()->type();
}

int Sequence::initialRank() const {
  return recordData()->initialRank();
}

void Sequence::setType(Type t) {
  if (t == type()) {
    return;
  }
  if (type() == Type::Explicit) {
    setInitialRank(0);
  }
  recordData()->setType(t);
  m_definition.tidyName();
  tidy();
  /* Reset all contents */
  switch (t) {
    case Type::Explicit:
      setContent("", nullptr); // No context needed here
      break;
    case Type::SingleRecurrence:
    {
      char ex[5] = "u(n)";
      ex[0] = fullName()[0];
      setContent(ex, nullptr); // No context needed here
      break;
    }
    case Type::DoubleRecurrence:
    {
      char ex[12] = "u(n+1)+u(n)";
      char name = fullName()[0];
      ex[0] = name;
      ex[7] = name;
      setContent(ex, nullptr); // No context needed here
      break;
    }
  }
  setFirstInitialConditionContent("", nullptr); // No context needed here
  setSecondInitialConditionContent("", nullptr); // No context needed here
}

void Sequence::setInitialRank(int rank) {
  recordData()->setInitialRank(rank);
  m_firstInitialCondition.tidyName();
  m_secondInitialCondition.tidyName();
}

Poincare::Layout Sequence::nameLayout() {
  return HorizontalLayout::Builder(
      CodePointLayout::Builder(fullName()[0], KDFont::SmallFont),
      VerticalOffsetLayout::Builder(CodePointLayout::Builder(symbol(), KDFont::SmallFont), VerticalOffsetLayoutNode::Position::Subscript)
    );
}

bool Sequence::isDefined() {
  RecordDataBuffer * data = recordData();
  switch (type()) {
    case Type::Explicit:
      return value().size > metaDataSize();
    case Type::SingleRecurrence:
      return data->initialConditionSize(0) > 0 && value().size > metaDataSize() + data->initialConditionSize(0);
    default:
      return data->initialConditionSize(0) > 0 && data->initialConditionSize(1) > 0 && value().size > metaDataSize() + data->initialConditionSize(0) + data->initialConditionSize(1);
  }
}

bool Sequence::isEmpty() {
  RecordDataBuffer * data = recordData();
  Type type = data->type();
  return Function::isEmpty() &&
    (type == Type::Explicit ||
      (data->initialConditionSize(0) == 0 &&
        (type == Type::SingleRecurrence || data->initialConditionSize(1) == 0)));
}

bool Sequence::badlyReferencesItself(Context * context) {
  Expression e = expressionReduced(context);
  bool value = e.hasExpression([](Expression e, const void * sequencePointer) {
    if (e.type() != ExpressionNode::Type::Sequence) {
      return false;
    }
    Sequence * seq = (Sequence *)(sequencePointer);
    const char * symbolName = static_cast<Symbol&>(e).name();
    /* symbolName is either u, v or w while seq->fullName has the extention .seq
     * at the end. Therefore we cannot use strcmp on the two strings. We just
     * want to check if the first char are identical*/
    if (strncmp(symbolName, seq->fullName(), strlen(symbolName)) == 0) {
      /* The expression of the sequence contains a reference to itself.
       * We must check if the sequence can be calculated before continuing
       * If the sequence is of explicit type, it cannot reference itself.
       * If the sequence is of SingleRecurrent type, it can be defined by:
       * u(initialRank and u(n).
       * If the sequence is of DoubleRecurrent type, it can be defined by:
       * u(initialRank), u(initialRank+1), u(n) and u(n+1).
       * In any other case, the value of the sequence cannot be computed.
       * We therefore return NAN. */
      Expression rank = e.childAtIndex(0);
      if (seq->type() == Sequence::Type::Explicit ||
         (!(rank.isIdenticalTo(Rational::Builder(seq->initialRank())) || rank.isIdenticalTo(Symbol::Builder(UCodePointUnknown))) &&
         (seq->type() == Sequence::Type::SingleRecurrence || (seq->type() == Sequence::Type::DoubleRecurrence && !(rank.isIdenticalTo(Rational::Builder(seq->initialRank()+1)) || rank.isIdenticalTo(Addition::Builder(Symbol::Builder(UCodePointUnknown), Rational::Builder(1))))))))
         {
        return true;
      }
    }
      return false;
    }, reinterpret_cast<const void *>(this));
    return value;
}

template<typename T>
T Sequence::templatedApproximateAtAbscissa(T x, SequenceContext * sqctx) const {
  T n = std::round(x);
  int sequenceIndex = SequenceStore::sequenceIndexForName(fullName()[0]);
  if (sqctx->iterateUntilRank<T>(n)) {
    return sqctx->valueOfCommonRankSequenceAtPreviousRank<T>(sequenceIndex, 0);
  }
  return NAN;
}

template<typename T>
T Sequence::valueAtRank(int n, SequenceContext *sqctx) {
  if (n < 0 || badlyReferencesItself(sqctx)) {
    return NAN;
  }
  int sequenceIndex = SequenceStore::sequenceIndexForName(fullName()[0]);
  if (sqctx->independentSequenceRank<T>(sequenceIndex) > n || sqctx->independentSequenceRank<T>(sequenceIndex) < 0) {
    // Reset cache indexes and cache values
    sqctx->setIndependentSequenceRank<T>(-1, sequenceIndex);
    for (int i = 0 ; i < MaxRecurrenceDepth+1; i++) {
      sqctx->setIndependentSequenceValue<T>(NAN, sequenceIndex, i);
    }
  }
  while(sqctx->independentSequenceRank<T>(sequenceIndex) < n) {
    sqctx->stepSequenceAtIndex<T>(sequenceIndex);
  }
  /* In case we have sqctx->independentSequenceRank<T>(sequenceIndex) = n, we can return the
   * value */
  T value = sqctx->independentSequenceValue<T>(sequenceIndex, 0);
  return value;
}

template<typename T>
T Sequence::approximateToNextRank(int n, SequenceContext * sqctx, int sequenceIndex) const {
  if (n < initialRank() || n < 0) {
    return NAN;
  }

  constexpr int bufferSize = CodePoint::MaxCodePointCharLength + 1;
  char unknownN[bufferSize];
  Poincare::SerializationHelper::CodePoint(unknownN, bufferSize, UCodePointUnknown);

  SequenceCacheContext<T> ctx = SequenceCacheContext<T>(sqctx);
  // Hold values u(n), u(n-1), u(n-2), v(n), v(n-1), v(n-2)...
  T values[MaxNumberOfSequences][MaxRecurrenceDepth+1];

  /* In case we step only one sequence to the next step, the data stored in
   * values is not necessarily u(n), u(n-1).... Indeed, since the indexes are
   * independent, if the index for u is 3 but the one for v is 5, value will
   * hold u(3), u(2), u(1) | v(5), v(4), v(3). Therefore, the calculation will
   * be wrong if they relay on a symbol such as u(n). To prevent this, we align
   * all the values around the index of the sequence we are stepping. */
  int independentRank = sqctx->independentSequenceRank<T>(sequenceIndex);
  for (int i = 0; i < MaxNumberOfSequences; i++) {
    if (sequenceIndex != -1 && sqctx->independentSequenceRank<T>(i) != independentRank) {
      int offset = independentRank - sqctx->independentSequenceRank<T>(i);
      if (offset != 0) {
        for (int j = MaxRecurrenceDepth; j >= 0; j--) {
            values[i][j] = j-offset < 0 || j-offset > MaxRecurrenceDepth ? NAN : sqctx->independentSequenceValue<T>(i, j-offset);
        }
      }
    } else {
      for (int j = 0; j < MaxRecurrenceDepth+1; j++) {
        values[i][j] = sequenceIndex != -1 ? sqctx->independentSequenceValue<T>(i, j) : sqctx->valueOfCommonRankSequenceAtPreviousRank<T>(i, j);
      }
    }
  }
  // Hold symbols u(n), u(n+1), v(n), v(n+1), w(n), w(n+1)
  Poincare::Symbol symbols[MaxNumberOfSequences][MaxRecurrenceDepth];
  char name[MaxRecurrenceDepth][7] = {"0(n)","0(n+1)"};
  for (int i = 0; i < MaxNumberOfSequences; i++) {
    for (int j = 0; j < MaxRecurrenceDepth; j++) {
      name[j][0] = SequenceStore::k_sequenceNames[i][0];
      symbols[i][j] = Symbol::Builder(name[j], strlen(name[j]));
    }
  }
  switch (type()) {
    case Type::Explicit:
    {
      for (int i = 0; i < MaxNumberOfSequences; i++) {
        // Set in context u(n) = u(n) for all sequences
        ctx.setValueForSymbol(values[i][0], symbols[i][0]);
      }
      return PoincareHelpers::ApproximateWithValueForSymbol(expressionReduced(sqctx), unknownN, (T)n, &ctx);
    }
    case Type::SingleRecurrence:
    {
      if (n == initialRank()) {
        return PoincareHelpers::ApproximateWithValueForSymbol(firstInitialConditionExpressionReduced(sqctx), unknownN, (T)NAN, &ctx);
      }
      for (int i = 0; i < MaxNumberOfSequences; i++) {
        // Set in context u(n) = u(n-1) and u(n+1) = u(n) for all sequences
        ctx.setValueForSymbol(values[i][0], symbols[i][1]);
        ctx.setValueForSymbol(values[i][1], symbols[i][0]);
      }
      return PoincareHelpers::ApproximateWithValueForSymbol(expressionReduced(sqctx), unknownN, (T)(n-1), &ctx);
    }
    default:
    {
      if (n == initialRank()) {
        return PoincareHelpers::ApproximateWithValueForSymbol(firstInitialConditionExpressionReduced(sqctx), unknownN, (T)NAN, &ctx);
      }
      if (n == initialRank()+1) {
        return PoincareHelpers::ApproximateWithValueForSymbol(secondInitialConditionExpressionReduced(sqctx), unknownN, (T)NAN, &ctx);
      }
      for (int i = 0; i < MaxNumberOfSequences; i++) {
        // Set in context u(n) = u(n-2) and u(n+1) = u(n-1) for all sequences
        ctx.setValueForSymbol(values[i][1], symbols[i][1]);
        ctx.setValueForSymbol(values[i][2], symbols[i][0]);
      }
      return PoincareHelpers::ApproximateWithValueForSymbol(expressionReduced(sqctx), unknownN, (T)(n-2), &ctx);
    }
  }
}

Expression Sequence::sumBetweenBounds(double start, double end, Poincare::Context * context) const {
  /* Here, we cannot just create the expression sum(u(n), start, end) because
   * the approximation of u(n) is not handled by Poincare (but only by
   * Sequence). */
  double result = 0.0;
  if (end-start > ExpressionNode::k_maxNumberOfSteps || start + 1.0 == start) {
    return Float<double>::Builder(NAN);
  }
  start = std::round(start);
  end = std::round(end);
  for (double i = start; i <= end; i = i + 1.0) {
    /* When |start| >> 1.0, start + 1.0 = start. In that case, quit the
     * infinite loop. */
    if (i == i-1.0 || i == i+1.0) {
      return Float<double>::Builder(NAN);
    }
    result += evaluateXYAtParameter(i, context).x2();
  }
  return Float<double>::Builder(result);
}

void Sequence::rangeForDisplay(float * xMin, float * xMax, float * yMin, float * yMax, float targetRatio, Poincare::Context * context) const {
  Poincare::Zoom::ValueAtAbscissa evaluation = [](float x, Poincare::Context * context, const void * auxiliary) {
    return static_cast<float>(static_cast<const Shared::Sequence *>(auxiliary)->initialRank());
  };
  Poincare::Zoom::FullRange(evaluation, 0, 1, 1, xMin, xMax, context, this);
  *xMax += Poincare::Zoom::k_defaultHalfRange;
  protectedFullRangeForDisplay(*xMin, *xMax, 1.f, yMin, yMax, context, false);
}

Sequence::RecordDataBuffer * Sequence::recordData() const {
  assert(!isNull());
  Ion::Storage::Record::Data d = value();
  return reinterpret_cast<RecordDataBuffer *>(const_cast<void *>(d.buffer));
}

/* Sequence Model */

Poincare::Layout Sequence::SequenceModel::name(Sequence * sequence) {
  if (m_name.isUninitialized()) {
    buildName(sequence);
  }
  return m_name;
}

void Sequence::SequenceModel::updateNewDataWithExpression(Ion::Storage::Record * record, const Expression & expressionToStore, void * expressionAddress, size_t newExpressionSize, size_t previousExpressionSize) {
  Ion::Storage::Record::Data newData = record->value();
  // Translate expressions located downstream
  size_t sizeBeforeExpression = (char *)expressionAddress -(char *)newData.buffer;
  size_t remainingSize = newData.size - sizeBeforeExpression - previousExpressionSize;
  memmove((char *)expressionAddress + newExpressionSize, (char *)expressionAddress + previousExpressionSize, remainingSize);
  // Copy the expression
  if (!expressionToStore.isUninitialized()) {
    memmove(expressionAddress, expressionToStore.addressInPool(), newExpressionSize);
  }
  // Update meta data
  updateMetaData(record, newExpressionSize);
}

/* Definition Handle*/

void * Sequence::DefinitionModel::expressionAddress(const Ion::Storage::Record * record) const {
  return (char *)record->value().buffer+sizeof(RecordDataBuffer);
}

size_t Sequence::DefinitionModel::expressionSize(const Ion::Storage::Record * record) const {
  Ion::Storage::Record::Data data = record->value();
  RecordDataBuffer * dataBuffer = static_cast<const Sequence *>(record)->recordData();
  return data.size-sizeof(RecordDataBuffer) - dataBuffer->initialConditionSize(0) - dataBuffer->initialConditionSize(1);
}

void Sequence::DefinitionModel::buildName(Sequence * sequence) {
  char name = sequence->fullName()[0];
  if (sequence->type() == Type::Explicit) {
    m_name = HorizontalLayout::Builder(
        CodePointLayout::Builder(name, k_layoutFont),
        VerticalOffsetLayout::Builder(LayoutHelper::String("n", 1, k_layoutFont), VerticalOffsetLayoutNode::Position::Subscript));
  } else if (sequence->type() == Type::SingleRecurrence) {
    m_name = HorizontalLayout::Builder(
        CodePointLayout::Builder(name, k_layoutFont),
        VerticalOffsetLayout::Builder(LayoutHelper::String("n+1", 3, k_layoutFont), VerticalOffsetLayoutNode::Position::Subscript));
  } else {
    assert(sequence->type() == Type::DoubleRecurrence);
    m_name = HorizontalLayout::Builder(
        CodePointLayout::Builder(name, k_layoutFont),
        VerticalOffsetLayout::Builder(LayoutHelper::String("n+2", 3, k_layoutFont), VerticalOffsetLayoutNode::Position::Subscript));
  }
}

/* Initial Condition Handle*/

void * Sequence::InitialConditionModel::expressionAddress(const Ion::Storage::Record * record) const {
  Ion::Storage::Record::Data data = record->value();
  RecordDataBuffer * dataBuffer = static_cast<const Sequence *>(record)->recordData();
  size_t offset = conditionIndex() == 0 ? data.size - dataBuffer->initialConditionSize(0) - dataBuffer->initialConditionSize(1) : data.size - dataBuffer->initialConditionSize(1) ;
  return (char *)data.buffer+offset;
}

size_t Sequence::InitialConditionModel::expressionSize(const Ion::Storage::Record * record) const {
  return static_cast<const Sequence *>(record)->recordData()->initialConditionSize(conditionIndex());
}

void Sequence::InitialConditionModel::updateMetaData(const Ion::Storage::Record * record, size_t newSize) {
  static_cast<const Sequence *>(record)->recordData()->setInitialConditionSize(newSize, conditionIndex());
}

void Sequence::InitialConditionModel::buildName(Sequence * sequence) {
  assert((conditionIndex() == 0 && sequence->type() == Type::SingleRecurrence) || sequence->type() == Type::DoubleRecurrence);
  char buffer[k_initialRankNumberOfDigits+1];
  Integer(sequence->initialRank()+conditionIndex()).serialize(buffer, k_initialRankNumberOfDigits+1);
  Layout indexLayout = LayoutHelper::String(buffer, strlen(buffer), k_layoutFont);
  m_name = HorizontalLayout::Builder(
      CodePointLayout::Builder(sequence->fullName()[0], k_layoutFont),
      VerticalOffsetLayout::Builder(indexLayout, VerticalOffsetLayoutNode::Position::Subscript));
}

template double Sequence::templatedApproximateAtAbscissa<double>(double, SequenceContext*) const;
template float Sequence::templatedApproximateAtAbscissa<float>(float, SequenceContext*) const;
template double Sequence::approximateToNextRank<double>(int, SequenceContext*, int) const;
template float Sequence::approximateToNextRank<float>(int, SequenceContext*, int) const;
template double Sequence::valueAtRank<double>(int, SequenceContext *);
template float Sequence::valueAtRank<float>(int, SequenceContext *);

}
