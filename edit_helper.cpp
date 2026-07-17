#include "edit_helper.h"

#include <QVariantMap>

EditHelper::EditState EditHelper::makeState(const QString &text, int cursor, int selStart, int selEnd)
{
    EditState st;
    st.text = text;
    st.cursor = cursor;
    st.selStart = selStart;
    st.selEnd = selEnd;
    return st;
}

QVariantMap EditHelper::stateToMap(const EditState &st)
{
    QVariantMap m;
    m.insert(QStringLiteral("text"), st.text);
    m.insert(QStringLiteral("cursor"), st.cursor);
    m.insert(QStringLiteral("selStart"), st.selStart);
    m.insert(QStringLiteral("selEnd"), st.selEnd);
    return m;
}

EditHelper::EditHelper(QObject *parent)
    : QObject(parent)
{
}

QString EditHelper::ping() const
{
    return QStringLiteral("editHelper");
}

bool EditHelper::isSpaceChar(const QString &c) const
{
    return c == QLatin1String(" ")
        || c == QLatin1String("\t")
        || c == QLatin1String("\n");
}

int EditHelper::lineStartPos(int pos, const QString &text) const
{
    // JS lastIndexOf(search, -1) clamps fromIndex to 0; Qt uses -1 as "from end".
    // Mirror QML: when pos <= 0 there is no prior newline to find.
    if (pos <= 0)
        return 0;
    const int prev = text.lastIndexOf(QLatin1Char('\n'), pos - 1);
    return prev == -1 ? 0 : prev + 1;
}

int EditHelper::lineEndPos(int pos, const QString &text) const
{
    const int nl = text.indexOf(QLatin1Char('\n'), pos);
    return nl == -1 ? text.length() : nl;
}

int EditHelper::lineCharCount(int lineStart, const QString &text) const
{
    return lineEndPos(lineStart, text) - lineStart;
}

int EditHelper::wordLeftPos(int pos, const QString &text) const
{
    if (pos <= 0)
        return 0;
    if (pos > text.length())
        pos = text.length();
    pos--;
    while (pos > 0 && isSpaceChar(text.mid(pos, 1)))
        pos--;
    while (pos > 0 && !isSpaceChar(text.mid(pos - 1, 1)))
        pos--;
    if (pos < text.length() && isSpaceChar(text.mid(pos, 1)))
        pos++;
    return pos;
}

int EditHelper::wordRightPos(int pos, const QString &text) const
{
    const int len = text.length();
    if (pos >= len)
        return len;
    while (pos < len && !isSpaceChar(text.mid(pos, 1)))
        pos++;
    while (pos < len && isSpaceChar(text.mid(pos, 1)))
        pos++;
    return pos;
}

int EditHelper::deleteWordLeftPos(int pos, const QString &text) const
{
    if (pos <= 0)
        return 0;
    int start = wordLeftPos(pos, text);
    // Swallow the preceding gap only when the whole word was consumed
    // (trailing-word Alt+BS). Mid-word must not eat the prior space.
    int wordEnd = start;
    while (wordEnd < text.length() && !isSpaceChar(text.mid(wordEnd, 1)))
        wordEnd++;
    if (pos >= wordEnd && start > 0) {
        int s = start - 1;
        while (s >= 0 && isSpaceChar(text.mid(s, 1)))
            s--;
        start = s + 1;
    }
    return start;
}

int EditHelper::deleteLineLeftPos(int pos, const QString &text) const
{
    if (pos <= 0)
        return 0;
    const int start = lineStartPos(pos, text);
    if (pos > start)
        return start;
    if (start == 0)
        return 0;
    return lineStartPos(start - 1, text);
}

int EditHelper::paragraphUpPos(int pos, const QString &text) const
{
    const int lineStart = lineStartPos(pos, text);
    if (lineStart == 0)
        return 0;
    int i = lineStart - 1;
    while (i > 0) {
        if (text.at(i) == QLatin1Char('\n') && text.at(i - 1) == QLatin1Char('\n'))
            return i + 1;
        i--;
    }
    return 0;
}

int EditHelper::paragraphDownPos(int pos, const QString &text) const
{
    const int len = text.length();
    const int lineEnd = lineEndPos(pos, text);
    if (lineEnd >= len)
        return len;
    for (int i = lineEnd; i < len - 1; i++) {
        if (text.at(i) == QLatin1Char('\n') && text.at(i + 1) == QLatin1Char('\n'))
            return i + 2;
    }
    return lineEnd + 1;
}

QVariant EditHelper::insertTextDelta(const QString &prevText, const QString &curText) const
{
    if (curText.length() <= prevText.length())
        return QVariant();
    const int extra = curText.length() - prevText.length();
    for (int p = 0; p <= prevText.length(); p++) {
        if (curText.left(p) == prevText.left(p)
                && curText.mid(p + extra) == prevText.mid(p)) {
            QVariantMap m;
            m.insert(QStringLiteral("pos"), p);
            m.insert(QStringLiteral("len"), extra);
            return m;
        }
    }
    return QVariant();
}

int EditHelper::isOneCharInsert(const QString &prevText, const QString &curText) const
{
    if (curText.length() != prevText.length() + 1)
        return -1;
    for (int i = 0; i < curText.length(); i++) {
        if (curText.left(i) + curText.mid(i + 1) == prevText)
            return i;
    }
    return -1;
}

void EditHelper::clearUndoStacks()
{
    m_undoStack.clear();
    m_redoStack.clear();
}

void EditHelper::syncUndoSnapshot(const QString &text, int cursor, int selStart, int selEnd)
{
    m_snapshot = makeState(text, cursor, selStart, selEnd);
}

void EditHelper::pushWithMerge(const EditState &prevState, const QString &curText, int curCursor)
{
    const int insPos = isOneCharInsert(prevState.text, curText);
    if (insPos >= 0 && !m_undoStack.isEmpty()) {
        const EditState &top = m_undoStack.last();
        const QVariant fromTop = insertTextDelta(top.text, curText);
        if (fromTop.isValid()) {
            const QVariantMap m = fromTop.toMap();
            const int pos = m.value(QStringLiteral("pos")).toInt();
            const int len = m.value(QStringLiteral("len")).toInt();
            if (pos == top.cursor && pos + len == curCursor) {
                m_redoStack.clear();
                return;
            }
        }
    }
    m_undoStack.append(prevState);
    m_redoStack.clear();
}

void EditHelper::beginTextEdit(const QString &text, int cursor, int selStart, int selEnd)
{
    if (m_undoCapture || m_skipTextUndoPush)
        return;
    pushWithMerge(makeState(text, cursor, selStart, selEnd), text, cursor);
    m_skipTextUndoPush = true;
}

void EditHelper::notifyTextChanged(const QString &text, int cursor, int selStart, int selEnd)
{
    if (m_skipTextUndoPush) {
        m_skipTextUndoPush = false;
        syncUndoSnapshot(text, cursor, selStart, selEnd);
        return;
    }
    if (!m_undoCapture && text.length() != m_snapshot.text.length()) {
        pushWithMerge(m_snapshot, text, cursor);
    }
    syncUndoSnapshot(text, cursor, selStart, selEnd);
}

QVariant EditHelper::undo(const QString &text, int cursor, int selStart, int selEnd)
{
    if (m_undoStack.isEmpty())
        return QVariant();
    const EditState prevState = m_undoStack.last();
    EditState redoState = makeState(text, cursor, selStart, selEnd);
    const QVariant ins = insertTextDelta(prevState.text, redoState.text);
    if (ins.isValid()) {
        const QVariantMap m = ins.toMap();
        const int pos = m.value(QStringLiteral("pos")).toInt();
        const int len = m.value(QStringLiteral("len")).toInt();
        if (pos == prevState.cursor) {
            redoState.cursor = pos + len;
            redoState.selStart = redoState.cursor;
            redoState.selEnd = redoState.cursor;
        }
    }
    m_redoStack.append(redoState);
    m_undoStack.removeLast();
    return stateToMap(prevState);
}

QVariant EditHelper::redo(const QString &text, int cursor, int selStart, int selEnd)
{
    if (m_redoStack.isEmpty())
        return QVariant();
    m_undoStack.append(makeState(text, cursor, selStart, selEnd));
    const EditState next = m_redoStack.takeLast();
    return stateToMap(next);
}

void EditHelper::beginRestore()
{
    m_undoCapture = true;
}

void EditHelper::endRestore(const QString &text, int cursor, int selStart, int selEnd)
{
    syncUndoSnapshot(text, cursor, selStart, selEnd);
    m_undoCapture = false;
}
