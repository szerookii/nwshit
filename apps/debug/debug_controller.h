#ifndef DEBUG_DEBUG_CONTROLLER_H
#define DEBUG_DEBUG_CONTROLLER_H

#include <escher.h>
#include "debug_view.h"

namespace Debug {

class DebugController : public ViewController {
public:
  DebugController();
  View* view() override;
  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;

private:
  DebugView m_debugView;
};

}

#endif

