#pragma once

#include <escher.h>

#include "debug_view.h"

namespace Debug {

class DebugController : public ViewController {
public:
  DebugController(Responder* parentResponder);
  View* view() override;
  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;

private:
  DebugView m_debugView;
};

}

