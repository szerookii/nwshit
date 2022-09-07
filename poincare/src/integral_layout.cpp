#include <poincare/integral_layout.h>
#include <poincare/code_point_layout.h>
#include <poincare/horizontal_layout.h>
#include <poincare/serialization_helper.h>
#include <string.h>
#include <assert.h>
#include <algorithm>

namespace Poincare {

const uint8_t topSymbolPixel[IntegralLayoutNode::k_symbolHeight][IntegralLayoutNode::k_symbolWidth] = {
  {0xFF, 0xD5, 0x46, 0x09},
  {0xF1, 0x1A, 0x93, 0xF0},
  {0x98, 0x54, 0xFF, 0xFF},
  {0x53, 0xA7, 0xFF, 0xFF},
  {0x29, 0xCF, 0xFF, 0xFF},
  {0x14, 0xEA, 0xFF, 0xFF},
  {0x02, 0xFC, 0xFF, 0xFF},
  {0x00, 0xFF, 0xFF, 0xFF},
  {0x00, 0xFF, 0xFF, 0xFF},
};

const uint8_t bottomSymbolPixel[IntegralLayoutNode::k_symbolHeight][IntegralLayoutNode::k_symbolWidth] = {
  {0xFF, 0xFF, 0xFF, 0x00},
  {0xFF, 0xFF, 0xFF, 0x00},
  {0xFF, 0xFF, 0xFE, 0x03},
  {0xFF, 0xFF, 0xEA, 0x13},
  {0xFF, 0xFF, 0xCF, 0x29},
  {0xFF, 0xFF, 0xA5, 0x53},
  {0xFF, 0xFF, 0x54, 0x99},
  {0xF2, 0x95, 0x1A, 0xF1},
  {0x09, 0x46, 0xD5, 0xFF},
};

void IntegralLayoutNode::moveCursorLeft(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool forSelection) {
  if (cursor->layoutNode() == upperBoundLayout()
      || cursor->layoutNode() == lowerBoundLayout())
  {
    assert(cursor->position() == LayoutCursor::Position::Left);
    // Case: Left the upper or lower bound. Go Left of the integral.
    cursor->setLayoutNode(this);
    return;
  }
  if (cursor->layoutNode() == differentialLayout())
  {
    assert(cursor->position() == LayoutCursor::Position::Left);
    // Case: Left of the variable differential. Go Right of the integrand
    cursor->setLayoutNode(integrandLayout());
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  if (cursor->layoutNode() == integrandLayout())
  {
    assert(cursor->position() == LayoutCursor::Position::Left);
    // Case: Left the integrand. Go Right of the lower bound.
    cursor->setLayoutNode(lowerBoundLayout());
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  assert(cursor->layoutNode() == this);
  if (cursor->position() == LayoutCursor::Position::Right) {
    // Case: Right of the integral. Go to the differential.
    cursor->setLayoutNode(differentialLayout());
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  // Case: Left of the integral. Ask the parent.
  assert(cursor->position() == LayoutCursor::Position::Left);
  LayoutNode * parentNode = parent();
  if (parentNode != nullptr) {
    parentNode->moveCursorLeft(cursor, shouldRecomputeLayout);
  }
}

void IntegralLayoutNode::moveCursorRight(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool forSelection) {
  if (cursor->layoutNode() == upperBoundLayout()
      || cursor->layoutNode() == lowerBoundLayout())
  {
    assert(cursor->position() == LayoutCursor::Position::Right);
    // Case: Right the upper or lower bound. Go Left of the integrand.
    cursor->setLayoutNode(integrandLayout());
    cursor->setPosition(LayoutCursor::Position::Left);
    return;
  }
  if (cursor->layoutNode() == integrandLayout())
  {
    assert(cursor->position() == LayoutCursor::Position::Right);
    // Case: Right the differential. Go Right.
    cursor->setLayoutNode(this);
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  if (cursor->layoutNode() == differentialLayout())
  {
    assert(cursor->position() == LayoutCursor::Position::Right);
    // Case: Right the differential. Go Right.
    cursor->setLayoutNode(this);
    cursor->setPosition(LayoutCursor::Position::Right);
    return;
  }
  assert(cursor->layoutNode() == this);
  if (cursor->position() == LayoutCursor::Position::Left) {
    // Case: Left of the integral. Go to the upper bound.
    cursor->setLayoutNode(upperBoundLayout());
    cursor->setPosition(LayoutCursor::Position::Left);
    return;
  }
  // Case: Right. Ask the parent.
  assert(cursor->position() == LayoutCursor::Position::Right);
  LayoutNode * parentNode = parent();
  if (parentNode != nullptr) {
    parentNode->moveCursorRight(cursor, shouldRecomputeLayout);
  }
}

void IntegralLayoutNode::moveCursorUp(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool equivalentPositionVisited, bool forSelection) {
  if (cursor->layoutNode()->hasAncestor(lowerBoundLayout(), true)) {
    // If the cursor is inside the lower bound, move it to the upper bound.
    upperBoundLayout()->moveCursorUpInDescendants(cursor, shouldRecomputeLayout);
    return;
  }
  if (cursor->isEquivalentTo(LayoutCursor(integrandLayout(), LayoutCursor::Position::Left))) {
    // If the cursor is Left of the integrand, move it to the upper bound.
    upperBoundLayout()->moveCursorUpInDescendants(cursor, shouldRecomputeLayout);
    return;
  }
  LayoutNode::moveCursorUp(cursor, shouldRecomputeLayout, equivalentPositionVisited);
}

void IntegralLayoutNode::moveCursorDown(LayoutCursor * cursor, bool * shouldRecomputeLayout, bool equivalentPositionVisited, bool forSelection) {
  if (cursor->layoutNode()->hasAncestor(upperBoundLayout(), true)) {
    // If the cursor is inside the upper bound, move it to the lower bound.
    lowerBoundLayout()->moveCursorDownInDescendants(cursor, shouldRecomputeLayout);
    return;
  }
  // If the cursor is Left of the integrand, move it to the lower bound.
  if (cursor->isEquivalentTo(LayoutCursor(integrandLayout(), LayoutCursor::Position::Left))) {
    lowerBoundLayout()->moveCursorDownInDescendants(cursor, shouldRecomputeLayout);
    return;
  }
  LayoutNode::moveCursorDown(cursor, shouldRecomputeLayout, equivalentPositionVisited);
}

void IntegralLayoutNode::deleteBeforeCursor(LayoutCursor * cursor) {
  if (cursor->isEquivalentTo(LayoutCursor(integrandLayout(), LayoutCursor::Position::Left))) {
    // Case: Left of the integrand. Delete the layout, keep the integrand.
    Layout thisRef = Layout(this);
    Layout integrand = Layout(integrandLayout());
    thisRef.replaceChildWithGhostInPlace(integrand);
    // WARNING: Do not use "this" afterwards
    cursor->setLayout(thisRef.childAtIndex(0));
    cursor->setPosition(LayoutCursor::Position::Left);
    thisRef.replaceWith(integrand, cursor);
    return;
  }
  LayoutNode::deleteBeforeCursor(cursor);
}

int IntegralLayoutNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  if (bufferSize == 0) {
    return -1;
  }
  buffer[bufferSize-1] = 0;

  // Write the operator name
  int numberOfChar = strlcpy(buffer, "int", bufferSize);
  if (numberOfChar >= bufferSize-1) {
    return bufferSize-1;
  }

  /* TODO
   * For now, we serialize
   *    2
   *    ∫3dx as int{{3},{x},{1},{2}}
   *    1
   * To save space, we could serialize it as int{3}{x}{1}{2} and modify the
   * parser accordingly.
   * This could be done for other layouts too. */

  /* Add system parentheses to avoid serializing:
   *   2)+(1          2),1
   *    ∫    (5)dx or  ∫    (5)dx
   *    1             1+binomial(3
   */
  numberOfChar += SerializationHelper::CodePoint(buffer + numberOfChar, bufferSize - numberOfChar, UCodePointLeftSystemParenthesis);
  if (numberOfChar >= bufferSize-1) {
    return bufferSize-1;
  }

  LayoutNode * argLayouts[] = {
    const_cast<IntegralLayoutNode *>(this)->integrandLayout(),
    const_cast<IntegralLayoutNode *>(this)->differentialLayout(),
    const_cast<IntegralLayoutNode *>(this)->lowerBoundLayout(),
    const_cast<IntegralLayoutNode *>(this)->upperBoundLayout()};

  for (uint8_t i = 0; i < sizeof(argLayouts)/sizeof(argLayouts[0]); i++) {
    if (i != 0) {
      // Write the comma
      numberOfChar += SerializationHelper::CodePoint(buffer + numberOfChar, bufferSize - numberOfChar, ',');
      if (numberOfChar >= bufferSize-1) { return bufferSize-1; }
    }

    // Write the argument with system parentheses
    numberOfChar += SerializationHelper::CodePoint(buffer + numberOfChar, bufferSize - numberOfChar, UCodePointLeftSystemParenthesis);
    if (numberOfChar >= bufferSize-1) { return bufferSize-1; }
    numberOfChar += argLayouts[i]->serialize(buffer+numberOfChar, bufferSize-numberOfChar, floatDisplayMode, numberOfSignificantDigits);
    if (numberOfChar >= bufferSize-1) { return bufferSize-1; }
    numberOfChar += SerializationHelper::CodePoint(buffer + numberOfChar, bufferSize - numberOfChar, UCodePointRightSystemParenthesis);
    if (numberOfChar >= bufferSize-1) { return bufferSize-1; }
  }

  // Write the closing system parenthesis
  numberOfChar += SerializationHelper::CodePoint(buffer + numberOfChar, bufferSize - numberOfChar, UCodePointRightSystemParenthesis);
  return numberOfChar;
}

CodePoint IntegralLayoutNode::XNTCodePoint(int childIndex) const {
  return (childIndex == k_integrandLayoutIndex || childIndex == k_differentialLayoutIndex) ? CodePoint('x') : UCodePointNull;
}

// Return pointer to the first or the last integral from left to right (considering multiple integrals in a row)
IntegralLayoutNode * IntegralLayoutNode::mostNestedIntegral(NestedPosition position) {
  IntegralLayoutNode * p = this;
  IntegralLayoutNode * integral = p->nestedIntegral(position);
  while (integral != nullptr) {
    p = integral;
    integral = integral->nestedIntegral(position);
  }
  return p;
}

// Return pointer to the integral immediately on the right. If none is found, return nullptr
IntegralLayoutNode * IntegralLayoutNode::nextNestedIntegral() {
  LayoutNode * integrand = integrandLayout();
  if (integrand->type() == Type::IntegralLayout) {
    // Integral can be directly in the integrand
    return static_cast<IntegralLayoutNode *>(integrand);
  } else if (integrand->type() == Type::HorizontalLayout && integrand->numberOfChildren() == 1 && integrand->childAtIndex(0)->type() == Type::IntegralLayout) {
    // Or can be in a Horizontal layout that only contains an integral
    integrand = integrand->childAtIndex(0);
    return static_cast<IntegralLayoutNode *>(integrand);
  }
  return nullptr;
}

// Return pointer to the integral immediately on the left. If none is found, return nullptr
IntegralLayoutNode * IntegralLayoutNode::previousNestedIntegral() {
  assert(type() == Type::IntegralLayout);
  LayoutNode * p = parent();
  if (p == nullptr) {
    return nullptr;
  }
  if (p->type() == Type::IntegralLayout) {
    // Parent is an integral. Checking if the child is its integrand or not
    if (p->childAtIndex(0) == this) {
      return static_cast<IntegralLayoutNode *>(p);
    } else {
      // If this is not parent's integrand, it means that it is either a bound or differential
      return nullptr;
    }
  } else if (p->type() == Type::HorizontalLayout) {
    // Or can be a Horizontal layout himself contained in an integral
    LayoutNode * prev = p->parent();
    if (prev == nullptr) {
      return nullptr;
    }
    if (p->numberOfChildren() == 1 && prev->type() == Type::IntegralLayout) {
      // We can consider the integrals in a row only if the horizontal layout just contains an integral
      // The horizontal layout must be the previous integral's integrand
      if (prev->childAtIndex(0) == p) {
        return static_cast<IntegralLayoutNode *>(prev);
      }
    }
  }
  return nullptr;
}

/* Return the height of the tallest upper/lower bound amongst a row of integrals
 * If the integral is alone, will return its upper/lower bound height*/
KDCoordinate IntegralLayoutNode::boundMaxHeight(BoundPosition position) {
  IntegralLayoutNode * p = mostNestedIntegral(NestedPosition::Next);
  LayoutNode * bound = p->boundLayout(position);
  KDCoordinate max = bound->layoutSize().height();
  IntegralLayoutNode * first = mostNestedIntegral(NestedPosition::Previous);
  while (p != first) {
    p = p->previousNestedIntegral();
    bound = p->boundLayout(position);
    max = std::max(max, bound->layoutSize().height());
  }
  return max;
}

/*
Window configuration explained :
Vertical margins and offsets
+-----------------------------------------------------------------+
|                                                |                |
|                                        k_boundVerticalMargin    |
|                                                |                |
|                                       +------------------+      |
|                                       | upperBoundHeight |      |
|                                       +------------------+      |
|             +++       |                        |                |
|            +++   k_symbolHeight     k_integrandVerticalMargin   |
|           +++         |                        |                |
|                                                                 |
|           |||                                                   |
|           |||                                                   |
|           |||                                                   |
|           |||                                                   |
|           |||         centralArgumentHeight                     |
|           |||                                                   |
|           |||                                                   |
|           |||                                                   |
|           |||                                                   |
|           |||                                                   |
|                                                                 |
|           +++         |                      |                  |
|          +++    k_symbolHeight     k_integrandVerticalMargin    |
|         +++           |                      |                  |
|                                     +------------------+        |
|                                     | lowerBoundHeight |        |
|                                     +------------------+        |
|                                              |                  |
|                                      k_boundVerticalMargin      |
|                                              |                  |
+-----------------------------------------------------------------+

Horizontal margins and offsets
+-------------------------------------------------------------------------------------------------------+
|                                                                                                       |                                                                                                |
|           |             |                       |+---------------+|                           |       |
|           |k_symbolWidth|k_boundHorizontalMargin||upperBoundWidth||k_integrandHorizontalMargin|       |
|           |             |                       |+---------------+|                           |       |
|                      ***                                                                              |
|                    ***                                                                                |
|                  ***                                                                          |       |
|                ***                                                                                    |
|              ***                                                                                      |
|            ***                                                                                |       |
|            |||                                                                                        |
|            |||                                                                                        |
|            |||                                                                                |       |
|            |||                                                                                        |
|            |||                                                                                 x dx   |
|            |||                                                                                        |
|            |||                                                                                |       |
|            |||                                                                                        |
|            |||                                                                                        |
|            |||                                                                                |       |
|            |||                                                                                        |
|            |||                                                                                        |
|            ***                                                                                |       |
|          ***                                                                                          |
|        ***                                                                                            |
|      ***                                                                                      |       |
|    ***                                                                                                |
|  ***                                                                                                  |
| |             |         |                       |+---------------+|                           |       |
| |k_symbolWidth|    a    |k_boundHorizontalMargin||lowerBoundWidth||k_integrandHorizontalMargin|       |
| |             |         |                       |+---------------+|                           |       |
|                                                                                                       |
+-------------------------------------------------------------------------------------------------------+
||| = k_lineThickness
a = k_symbolWidth - k_lineThickness
*/

KDSize IntegralLayoutNode::computeSize() {
  KDSize dSize = k_font->stringSize("d");
  KDSize integrandSize = integrandLayout()->layoutSize();
  KDSize differentialSize = differentialLayout()->layoutSize();
  KDSize lowerBoundSize = lowerBoundLayout()->layoutSize();
  KDSize upperBoundSize = upperBoundLayout()->layoutSize();
  KDCoordinate width = k_symbolWidth+k_lineThickness+k_boundHorizontalMargin+std::max(lowerBoundSize.width(), upperBoundSize.width())+k_integrandHorizontalMargin+integrandSize.width()+k_differentialHorizontalMargin+dSize.width()+k_differentialHorizontalMargin+differentialSize.width();
  IntegralLayoutNode * last = mostNestedIntegral(NestedPosition::Next);
  KDCoordinate height;
  if (this == last) {
    height = k_boundVerticalMargin + boundMaxHeight(BoundPosition::UpperBound) + k_integrandVerticalMargin + centralArgumentHeight() + k_integrandVerticalMargin + boundMaxHeight(BoundPosition::LowerBound) + k_boundVerticalMargin;
  } else {
    height = last->layoutSize().height();
  }
  return KDSize(width, height);
}

KDCoordinate IntegralLayoutNode::computeBaseline() {
  IntegralLayoutNode * last = mostNestedIntegral(NestedPosition::Next);
  if (this == last) {
    return k_boundVerticalMargin + boundMaxHeight(BoundPosition::UpperBound) + k_integrandVerticalMargin + std::max(integrandLayout()->baseline(), differentialLayout()->baseline());
  } else {
    // If integrals are in a row, they must have the same baseline. Since the last integral has the lowest, we take this one for all the others
    return last->baseline();
  }
}

KDPoint IntegralLayoutNode::positionOfChild(LayoutNode * child) {
  KDSize lowerBoundSize = lowerBoundLayout()->layoutSize();
  KDSize upperBoundSize = upperBoundLayout()->layoutSize();
  KDCoordinate x = 0;
  KDCoordinate y = 0;
  KDCoordinate boundOffset = 2*k_symbolWidth - k_lineThickness + k_boundHorizontalMargin;
  if (child == lowerBoundLayout()) {
    x = boundOffset;
    y = computeSize().height() - k_boundVerticalMargin - boundMaxHeight(BoundPosition::LowerBound);
  } else if (child == upperBoundLayout()) {
    x = boundOffset;
    y = k_boundVerticalMargin + boundMaxHeight(BoundPosition::UpperBound) - upperBoundSize.height();
  } else if (child == integrandLayout()) {
    x = boundOffset + std::max(lowerBoundSize.width(), upperBoundSize.width()) + k_integrandHorizontalMargin;
    y = computeBaseline()-integrandLayout()->baseline();
  } else {
    assert(child == differentialLayout());
    x = computeSize().width() - differentialLayout()->layoutSize().width();
    y = computeBaseline()-differentialLayout()->baseline();
  }
  return KDPoint(x,y);
}

KDCoordinate IntegralLayoutNode::centralArgumentHeight() {
  // When integrals are in a row, the last one is the tallest. We take its central argument height to define the one of the others integrals
  IntegralLayoutNode * last = mostNestedIntegral(NestedPosition::Next);
  if (this == last) {
    KDCoordinate integrandHeight = integrandLayout()->layoutSize().height();
    KDCoordinate integrandBaseline = integrandLayout()->baseline();
    KDCoordinate differentialHeight = differentialLayout()->layoutSize().height();
    KDCoordinate differentialBaseline = differentialLayout()->baseline();
    return std::max(integrandBaseline, differentialBaseline) + std::max(integrandHeight-integrandBaseline, differentialHeight - differentialBaseline);
  } else {
    return last->centralArgumentHeight();
  }
}

void IntegralLayoutNode::render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor, Layout * selectionStart, Layout * selectionEnd, KDColor selectionColor) {
  KDSize integrandSize = integrandLayout()->layoutSize();
  KDCoordinate centralArgHeight = centralArgumentHeight();
  KDColor workingBuffer[k_symbolWidth*k_symbolHeight];


  // Render the integral symbol
  KDCoordinate offsetX = p.x() + k_symbolWidth;
  KDCoordinate offsetY = p.y() + k_boundVerticalMargin + boundMaxHeight(BoundPosition::UpperBound) + k_integrandVerticalMargin - k_symbolHeight;

  // Upper part
  KDRect topSymbolFrame(offsetX, offsetY, k_symbolWidth, k_symbolHeight);
  ctx->blendRectWithMask(topSymbolFrame, expressionColor, (const uint8_t *)topSymbolPixel, (KDColor *)workingBuffer);

  // Central bar
  offsetY = offsetY + k_symbolHeight;
  ctx->fillRect(KDRect(offsetX, offsetY, k_lineThickness, centralArgHeight), expressionColor);

  // Lower part
  offsetX = offsetX - k_symbolWidth + k_lineThickness;
  offsetY = offsetY + centralArgHeight;
  KDRect bottomSymbolFrame(offsetX,offsetY,k_symbolWidth, k_symbolHeight);
  ctx->blendRectWithMask(bottomSymbolFrame, expressionColor, (const uint8_t *)bottomSymbolPixel, (KDColor *)workingBuffer);

  // Render "d"
  KDPoint dPosition = p.translatedBy(positionOfChild(integrandLayout())).translatedBy(KDPoint(integrandSize.width() + k_differentialHorizontalMargin, integrandLayout()->baseline() - k_font->glyphSize().height()/2));
  ctx->drawString("d", dPosition, k_font, expressionColor, backgroundColor);
}

}
