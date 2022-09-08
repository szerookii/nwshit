#ifndef DEBUG_APP_H
#define DEBUG_APP_H

#include <escher.h>
#include "debug_controller.h"
#include "../shared/shared_app.h"

namespace Debug {

class App : public ::App {
public:
  class Descriptor : public ::App::Descriptor {
  public:
    I18n::Message name() override;
    I18n::Message upperName() override;
    const Image* icon() override;
  };

  class Snapshot : public ::App::Snapshot {
  public:
    App* unpack(Container* container) override;
    Descriptor* descriptor() override;
    void reset() override;
  };

private:
  App(Snapshot* snapshot);
  DebugController m_debugController;
};

}

#endif