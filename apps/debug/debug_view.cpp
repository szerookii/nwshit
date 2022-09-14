#include "debug_view.h"
#include "apps/i18n.h"

#ifdef DEVICE
#include <ion/src/device/shared/drivers/timing.h>>
#endif

namespace Debug {

DebugView::DebugView() :
  View(),
  m_bufferTextView(KDFont::LargeFont, 0.5, 0.5, KDColorBlack),
  m_color(3),
  m_kdcolor(Palette::Cyan)
{
  m_bufferTextView.setText("Kalvin sale pd");
}

void DebugView::drawRect(KDContext * ctx, KDRect rect) const {
  ctx->fillRect(KDRect(0, 0, bounds().width(), bounds().height()), m_kdcolor);
}

void DebugView::reload() {
  switch (m_color) {
    case 0:
      m_kdcolor = Palette::Cyan;
      break;

    case 1:
      m_kdcolor = Palette::PurpleBright;
      break;

    case 2:
      m_kdcolor = Palette::YellowLight;
      break;
      
    default:
      m_kdcolor = Palette::Pink;
  }

  m_bufferTextView.setBackgroundColor(m_kdcolor);

  markRectAsDirty(bounds());
}

void DebugView::changeColor() {
  m_color++;

  if (m_color > 3)
    m_color = 0;

  reload();
}

int DebugView::numberOfSubviews() const {
  return 1;
}

View* DebugView::subviewAtIndex(int index) {
  return &m_bufferTextView;
}

void DebugView::layoutSubviews(bool force) {
  m_bufferTextView.setFrame(KDRect(0, 0, bounds().width(), bounds().height()), force);
}

}