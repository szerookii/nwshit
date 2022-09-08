#ifndef DEBUG_DEBUG_VIEW_H
#define DEBUG_DEBUG_VIEW_H

#include <escher.h>

namespace Debug {

class DebugView : public View {
public:
  DebugView();
  void drawRect(KDContext * ctx, KDRect rect) const override;
  void reload();
  void changeColor();
  int numberOfSubviews() const override;
  View* subviewAtIndex(int index) override;

private:
  void layoutSubviews(bool force = false) override;
  BufferTextView m_bufferTextView;
  int m_color;
  KDColor m_kdcolor;
};

}

#endif