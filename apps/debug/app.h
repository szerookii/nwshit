#pragma once

#include "escher/include/escher/app.h"
#include "../shared/shared_app.h"
#include "debug_controller.h"

namespace Debug {

class App : public ::App {
public:
  class Descriptor : public ::App::Descriptor {
  public:
    I18n::Message name() override;
    I18n::Message upperName() override;
  };

  class Snapshot : public ::SharedApp::Snapshot {
  public:
    App* unpack(Container* container) override;
    Descriptor * descriptor() override;
  };

  bool processEvent(Ion::Events::Event) override;

private:
  App(Snapshot* snapshot);
  DebugController m_debugController;
};
}