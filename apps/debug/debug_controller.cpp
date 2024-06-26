#include "debug_controller.h"
#include <apps/i18n.h>
#include "../global_preferences.h"

namespace Debug {

DebugController::DebugController() :
  ViewController(nullptr)
  {
  }

View* DebugController::view() {
  return &m_debugView;
}

void DebugController::didBecomeFirstResponder() {
}

bool DebugController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::OK || event == Ion::Events::EXE || event == Ion::Events::TimerFire || event == Ion::Events::None) {
    m_debugView.changeColor();
    return true;
  }

  return false;
}

}
