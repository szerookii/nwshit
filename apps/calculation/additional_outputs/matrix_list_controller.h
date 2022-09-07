#ifndef CALCULATION_ADDITIONAL_OUTPUTS_MATRIX_LIST_CONTROLLER_H
#define CALCULATION_ADDITIONAL_OUTPUTS_MATRIX_LIST_CONTROLLER_H

#include "expressions_list_controller.h"

namespace Calculation {

class MatrixListController : public ExpressionsListController {
public:
  MatrixListController(EditExpressionController * editExpressionController) :
    ExpressionsListController(editExpressionController) {}

  void setExpression(Poincare::Expression e) override;

private:
  I18n::Message messageAtIndex(int index) override;
  Poincare::Layout getLayoutFromExpression(Poincare::Expression e, Poincare::Context * context, Poincare::Preferences * preferences);
  // Map from cell index to message index
  constexpr static int k_maxNumberOfOutputRows = 5;
  int m_indexMessageMap[k_maxNumberOfOutputRows];
};

}

#endif


