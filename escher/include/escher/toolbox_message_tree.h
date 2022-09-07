#ifndef ESCHER_TOOLBOX_MESSAGE_TREE_H
#define ESCHER_TOOLBOX_MESSAGE_TREE_H

#include <escher/message_tree.h>

class ToolboxMessageTree : public MessageTree {
public:
  constexpr static ToolboxMessageTree Leaf(I18n::Message label, I18n::Message text = (I18n::Message)0, bool stripInsertedText = true, I18n::Message insertedText = (I18n::Message)0, bool multiLine = false, uint8_t numberOfLines = 1) {
    return ToolboxMessageTree(
        label,
        text,
        (insertedText == (I18n::Message)0) ? label : insertedText,
        static_cast<ToolboxMessageTree *>(0),
        0,
        stripInsertedText,
        multiLine,
        numberOfLines);
  };
  template <int N>
  constexpr static ToolboxMessageTree Node(I18n::Message label, const ToolboxMessageTree (&children)[N], bool fork = false) {
    return ToolboxMessageTree(
        label,
        (I18n::Message)0,
        (I18n::Message)0,
        children,
        fork ? -N : N,
        true);
  }
  template <int N>
  constexpr static ToolboxMessageTree Node(I18n::Message label, const ToolboxMessageTree * (&children)[N], bool fork = false) {
    return ToolboxMessageTree(
        label,
        (I18n::Message)0,
        (I18n::Message)0,
        children,
        fork ? -N : N,
        true);
  }
  const MessageTree * childAtIndex(int index) const override {
    return m_childrenConsecutive ? m_children.m_direct + index : m_children.m_indirect[index];
  }
  I18n::Message text() const { return m_text; }
  I18n::Message insertedText() const { return m_insertedText; }
  bool stripInsertedText() const { return m_stripInsertedText; }
  bool isFork() const { return numberOfChildren() < 0; }
  bool isMultiLine() const { return m_multiLine; }
  uint8_t numberOfLines() const { return m_numberOfLines; }
private:
  constexpr ToolboxMessageTree(I18n::Message label, I18n::Message text, I18n::Message insertedText, const ToolboxMessageTree * children, int numberOfChildren, bool stripInsertedText, bool multiLine = false, uint8_t numberOfLines = 1) :
    MessageTree(label, numberOfChildren),
    m_children(children),
    m_text(text),
    m_insertedText(insertedText),
    m_stripInsertedText(stripInsertedText),
    m_childrenConsecutive(true),
    m_multiLine(multiLine),
    m_numberOfLines(numberOfLines)
  {}
  constexpr ToolboxMessageTree(I18n::Message label, I18n::Message text, I18n::Message insertedText, const ToolboxMessageTree ** children, int numberOfChildren, bool stripInsertedText, bool multiLine = false, uint8_t numberOfLines = 1) :
    MessageTree(label, numberOfChildren),
    m_children(children),
    m_text(text),
    m_insertedText(insertedText),
    m_stripInsertedText(stripInsertedText),
    m_childrenConsecutive(false),
    m_multiLine(multiLine),
    m_numberOfLines(numberOfLines)
  {}

  union Children {
  public:
    constexpr Children(const ToolboxMessageTree * children) : m_direct(children) {}
    constexpr Children(const ToolboxMessageTree ** children) : m_indirect(children) {}
    const ToolboxMessageTree * m_direct;
    const ToolboxMessageTree ** m_indirect;
  };
  const Children m_children;
  I18n::Message m_text;
  I18n::Message m_insertedText;
  bool m_stripInsertedText;
  const bool m_childrenConsecutive;
  bool m_multiLine;
  uint8_t m_numberOfLines;
};

#endif

