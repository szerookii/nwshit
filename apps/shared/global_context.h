#ifndef APPS_SHARED_GLOBAL_CONTEXT_H
#define APPS_SHARED_GLOBAL_CONTEXT_H

#include <poincare/context.h>
#include <poincare/matrix.h>
#include <poincare/float.h>
#include <poincare/decimal.h>
#include <poincare/symbol.h>
#include <ion/storage.h>
#include <assert.h>
#include "sequence_store.h"

namespace Shared {

class GlobalContext final : public Poincare::Context {
public:
  static constexpr int k_numberOfExtensions = 3;
  static constexpr const char * k_extensions[] = {Ion::Storage::expExtension, Ion::Storage::funcExtension,  Ion::Storage::seqExtension};

  // Storage information
  static bool SymbolAbstractNameIsFree(const char * baseName);

  static const Poincare::Layout LayoutForRecord(Ion::Storage::Record record);

  // Destroy records
  static void DestroyRecordsBaseNamedWithoutExtension(const char * baseName, const char * extension);

  /* Expression for symbol
   * The expression recorded in global context is already an expression.
   * Otherwise, we would need the context and the angle unit to evaluate it */
  SymbolAbstractType expressionTypeForIdentifier(const char * identifier, int length) override;
  const Poincare::Expression expressionForSymbolAbstract(const Poincare::SymbolAbstract & symbol, bool clone, float unknownSymbolValue = NAN) override;
  void setExpressionForSymbolAbstract(const Poincare::Expression & expression, const Poincare::SymbolAbstract & symbol) override;
  static SequenceStore * sequenceStore();
private:
  // Expression getters
  static const Poincare::Expression ExpressionForSymbolAndRecord(const Poincare::SymbolAbstract & symbol, Ion::Storage::Record r, Context * ctx, float unknownSymbolValue = NAN);
  static const Poincare::Expression ExpressionForActualSymbol(Ion::Storage::Record r);
  static const Poincare::Expression ExpressionForFunction(const Poincare::SymbolAbstract & symbol, Ion::Storage::Record r);
  static const Poincare::Expression ExpressionForSequence(const Poincare::SymbolAbstract & symbol, Ion::Storage::Record r, Context * ctx, float unknownSymbolValue = NAN);
  // Expression setters
  static Ion::Storage::Record::ErrorStatus SetExpressionForActualSymbol(const Poincare::Expression & expression, const Poincare::SymbolAbstract & symbol, Ion::Storage::Record previousRecord);
  static Ion::Storage::Record::ErrorStatus SetExpressionForFunction(const Poincare::Expression & expression, const Poincare::SymbolAbstract & symbol, Ion::Storage::Record previousRecord);
  // Record getter
  static Ion::Storage::Record SymbolAbstractRecordWithBaseName(const char * name);

};

}

#endif
