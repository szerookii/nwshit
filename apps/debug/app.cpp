#include "app.h"
#include "apps/apps_container.h"
#include <apps/i18n.h>
#include <assert.h>
#include "debug_icon.h"

namespace Debug {

I18n::Message App::Descriptor::name() {
  return upperName();
}

I18n::Message App::Descriptor::upperName() {
  return I18n::Message::DebugAppCapital;
}

const Image* App::Descriptor::icon() {
  return ImageStore::DebugIcon;
}

App::Descriptor* App::Snapshot::descriptor() {
  static Descriptor descriptor;
  return &descriptor;
}

App* App::Snapshot::unpack(Container* container) {
  return new (container->currentAppBuffer()) App(this);
}

void App::Snapshot::reset() {
}

App::App(Snapshot* snapshot) :
  ::App(snapshot, &m_debugController)
{
}

}