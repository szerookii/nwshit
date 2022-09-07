#include "histogram_parameter_controller.h"
#include "app.h"
#include <algorithm>
#include <assert.h>
#include <cmath>

using namespace Shared;

namespace Statistics {

HistogramParameterController::HistogramParameterController(Responder * parentResponder, InputEventHandlerDelegate * inputEventHandlerDelegate, Store * store) :
  FloatParameterController<double>(parentResponder),
  m_cells{},
  m_store(store),
  m_confirmPopUpController(Invocation([](void * context, void * sender) {
    Container::activeApp()->dismissModalViewController();
    ((HistogramParameterController *)context)->stackController()->pop();
    return true;
  }, this))
{
  for (int i = 0; i < k_numberOfCells; i++) {
    m_cells[i].setParentResponder(&m_selectableTableView);
    m_cells[i].textField()->setDelegates(inputEventHandlerDelegate, this);
  }
}

void HistogramParameterController::viewWillAppear() {
  // Initialize temporary parameters to the extracted value.
  /* setParameterAtIndex uses the value of the other parameter, so we need to
   * manually set the value of the second parameter before the first call. */
  double parameterAtIndex1 = extractParameterAtIndex(1);
  m_tempFirstDrawnBarAbscissa = parameterAtIndex1;
  setParameterAtIndex(0, extractParameterAtIndex(0));
  setParameterAtIndex(1, parameterAtIndex1);
  FloatParameterController::viewWillAppear();
}

const char * HistogramParameterController::title() {
  return I18n::translate(I18n::Message::HistogramSet);
}

void HistogramParameterController::willDisplayCellForIndex(HighlightCell * cell, int index) {
  if (index == numberOfRows()-1) {
    return;
  }
  MessageTableCellWithEditableText * myCell = (MessageTableCellWithEditableText *)cell;
  I18n::Message labels[k_numberOfCells] = {I18n::Message::RectangleWidth, I18n::Message::BarStart};
  myCell->setMessage(labels[index]);
  FloatParameterController::willDisplayCellForIndex(cell, index);
}

bool HistogramParameterController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Back && (extractParameterAtIndex(0) != parameterAtIndex(0) || extractParameterAtIndex(1) != parameterAtIndex(1))) {
    // Temporary values are different, open pop-up to confirm discarding values
    Container::activeApp()->displayModalViewController(&m_confirmPopUpController, 0.f, 0.f, Metric::ExamPopUpTopMargin, Metric::PopUpRightMargin, Metric::ExamPopUpBottomMargin, Metric::PopUpLeftMargin);
    return true;
  }
  return false;
}

double HistogramParameterController::extractParameterAtIndex(int index) {
  assert(index >= 0 && index < k_numberOfCells);
  return index == 0 ? m_store->barWidth() : m_store->firstDrawnBarAbscissa();
}

double HistogramParameterController::parameterAtIndex(int index) {
  assert(index >= 0 && index < k_numberOfCells);
  return index == 0 ? m_tempBarWidth : m_tempFirstDrawnBarAbscissa;
}

bool HistogramParameterController::confirmParameterAtIndex(int parameterIndex, double value) {
  assert(parameterIndex == 0 || parameterIndex == 1);
  if (parameterIndex == 0) {
    // Set the bar width
    m_store->setBarWidth(value);
  } else {
    m_store->setFirstDrawnBarAbscissa(value);
  }
  return true;
}

bool HistogramParameterController::setParameterAtIndex(int parameterIndex, double value) {
  assert(parameterIndex == 0 || parameterIndex == 1);
  const bool setBarWidth = parameterIndex == 0;

  if (setBarWidth && value <= 0.0) {
    // The bar width cannot be negative
    Container::activeApp()->displayWarning(I18n::Message::ForbiddenValue);
    return false;
  }

  const double nextFirstDrawnBarAbscissa = setBarWidth ? m_tempFirstDrawnBarAbscissa : value;
  const double nextBarWidth = setBarWidth ? value : m_tempBarWidth;

  // The number of bars cannot be above the max
  assert(DoublePairStore::k_numberOfSeries > 0);
  for (int i = 0; i < DoublePairStore::k_numberOfSeries; i++) {
    const double min = std::min(m_store->minValue(i), nextFirstDrawnBarAbscissa);
    double numberOfBars = std::ceil((m_store->maxValue(i) - min)/nextBarWidth);
    if (numberOfBars > Store::k_maxNumberOfBars) {
      Container::activeApp()->displayWarning(I18n::Message::ForbiddenValue);
      return false;
    }
  }

  if (setBarWidth) {
    // Set the bar width
    m_tempBarWidth = value;
  } else {
    m_tempFirstDrawnBarAbscissa = value;
  }
  return true;
}

HighlightCell * HistogramParameterController::reusableParameterCell(int index, int type) {
  assert(index >= 0 && index < k_numberOfCells);
  return &m_cells[index];
}

void HistogramParameterController::buttonAction() {
  // Update parameters values and proceed.
  if (confirmParameterAtIndex(0, m_tempBarWidth) && confirmParameterAtIndex(1, m_tempFirstDrawnBarAbscissa)) {
    FloatParameterController::buttonAction();
  }
}

}

