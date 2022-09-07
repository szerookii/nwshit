#ifndef SEQUENCE_CURVE_VIEW_RANGE_H
#define SEQUENCE_CURVE_VIEW_RANGE_H

#include "../../shared/interactive_curve_view_range.h"

namespace Sequence {

class CurveViewRange : public Shared::InteractiveCurveViewRange {
public:
  CurveViewRange(Shared::InteractiveCurveViewRangeDelegate * delegate = nullptr);
  void normalize(bool forceChangeY = false) override;
private:
  virtual bool hasDefaultRange() const override { return false; }
  constexpr static float k_displayLeftMarginRatio = 0.1f;
};

}

#endif
