#include "edit_helper.h"

#include <QMetaObject>
#include <QVariantMap>
#include <Qt>
#include <QtMath>

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

QVariantMap EditHelper::notHandled()
{
    QVariantMap m;
    m.insert(QStringLiteral("handled"), false);
    return m;
}

QVariantMap EditHelper::handledAction(const QString &action)
{
    QVariantMap m;
    m.insert(QStringLiteral("handled"), true);
    m.insert(QStringLiteral("action"), action);
    return m;
}

int EditHelper::selectionExtendFrom(int key, int cursor, int selStart, int selEnd, int shiftHead)
{
    if (shiftHead >= 0)
        return shiftHead;
    if (selStart == selEnd)
        return cursor;
    if (key == Qt::Key_Left || key == Qt::Key_Up)
        return qMin(selStart, selEnd);
    if (key == Qt::Key_Right || key == Qt::Key_Down)
        return qMax(selStart, selEnd);
    return cursor;
}

static QVariantMap moveToResult(int pos, bool extend)
{
    QVariantMap m;
    m.insert(QStringLiteral("handled"), true);
    m.insert(QStringLiteral("action"), QStringLiteral("moveTo"));
    m.insert(QStringLiteral("pos"), pos);
    m.insert(QStringLiteral("extend"), extend);
    return m;
}

static QVariantMap moveToResolvedResult(const QString &posKind, bool extend, int extendKey = 0)
{
    QVariantMap m;
    m.insert(QStringLiteral("handled"), true);
    m.insert(QStringLiteral("action"), QStringLiteral("moveToResolved"));
    m.insert(QStringLiteral("posKind"), posKind);
    m.insert(QStringLiteral("extend"), extend);
    if (extendKey != 0)
        m.insert(QStringLiteral("extendKey"), extendKey);
    return m;
}

QVariantMap EditHelper::dispatchMacArrow(int key, int modifiers,
                                         const QString &text, int cursor,
                                         int selStart, int selEnd,
                                         int shiftAnchor, int shiftHead) const
{
    Q_UNUSED(shiftAnchor)
    const int len = text.length();
    const bool shift = (modifiers & Qt::ShiftModifier) != 0;
    const bool cmd = (modifiers & Qt::ControlModifier) != 0;
    const bool alt = (modifiers & Qt::AltModifier) != 0;

    if (modifiers == Qt::NoModifier) {
        if (key == Qt::Key_Left || key == Qt::Key_Right) {
            if (selStart != selEnd) {
                QVariantMap m = handledAction(QStringLiteral("collapseSel"));
                m.insert(QStringLiteral("toMin"), key == Qt::Key_Left);
                return m;
            }
            const int newPos = (key == Qt::Key_Left)
                ? qMax(0, cursor - 1)
                : qMin(len, cursor + 1);
            return moveToResult(newPos, false);
        }
    }

    int newPos = cursor;

    if ((key == Qt::Key_Home || key == Qt::Key_End) && !(shift && !cmd && !alt)) {
        if (alt)
            return notHandled();
        if (key == Qt::Key_Home) {
            if (cmd)
                newPos = 0;
            else
                return moveToResolvedResult(QStringLiteral("macLineStartCursor"), shift);
        } else {
            if (cmd)
                newPos = len;
            else
                return moveToResolvedResult(QStringLiteral("macLineEndCursor"), shift);
        }
    } else if (shift && !cmd && !alt) {
        if (key == Qt::Key_Left || key == Qt::Key_Right) {
            QVariantMap m = handledAction(QStringLiteral("shiftHorizDelta"));
            m.insert(QStringLiteral("delta"), (key == Qt::Key_Left) ? -1 : 1);
            m.insert(QStringLiteral("eventKey"), key);
            return m;
        }
        if (key == Qt::Key_Up || key == Qt::Key_Down) {
            QVariantMap m = handledAction(QStringLiteral("shiftVert"));
            m.insert(QStringLiteral("down"), key == Qt::Key_Down);
            return m;
        }
        if (key == Qt::Key_Home) {
            QVariantMap m = handledAction(QStringLiteral("shiftHorizTo"));
            m.insert(QStringLiteral("posKind"), QStringLiteral("macLineStartShiftHead"));
            return m;
        }
        if (key == Qt::Key_End) {
            QVariantMap m = handledAction(QStringLiteral("shiftHorizTo"));
            m.insert(QStringLiteral("posKind"), QStringLiteral("macLineEndShiftHead"));
            return m;
        }
        return notHandled();
    } else if (!cmd && !alt && !shift) {
        if (key == Qt::Key_Up || key == Qt::Key_Down) {
            QVariantMap m = handledAction(QStringLiteral("moveVert"));
            m.insert(QStringLiteral("down"), key == Qt::Key_Down);
            return m;
        }
        if (key == Qt::Key_Left || key == Qt::Key_Right) {
            if (selStart != selEnd) {
                const int selLo = qMin(selStart, selEnd);
                const int selHi = qMax(selStart, selEnd);
                newPos = (key == Qt::Key_Left) ? selLo : selHi;
                QVariantMap m = moveToResult(newPos, false);
                m.insert(QStringLiteral("keepGoalColumn"), false);
                return m;
            }
        }
        return notHandled();
    } else if (key == Qt::Key_Left) {
        if (cmd && shift)
            return moveToResolvedResult(QStringLiteral("macLineStartExtend"), true, Qt::Key_Left);
        if (cmd)
            newPos = 0;
        else if (alt)
            newPos = wordLeftPos(selectionExtendFrom(Qt::Key_Left, cursor, selStart, selEnd, shiftHead), text);
        else
            newPos = lineStartPos(cursor, text);
    } else if (key == Qt::Key_Right) {
        if (cmd && shift)
            return moveToResolvedResult(QStringLiteral("macLineEndExtend"), true, Qt::Key_Right);
        if (cmd)
            newPos = len;
        else if (alt)
            newPos = wordRightPos(selectionExtendFrom(Qt::Key_Right, cursor, selStart, selEnd, shiftHead), text);
        else
            newPos = lineEndPos(cursor, text);
    } else if (key == Qt::Key_Up) {
        if (cmd)
            newPos = 0;
        else if (alt && shift)
            newPos = paragraphUpPos(selectionExtendFrom(Qt::Key_Up, cursor, selStart, selEnd, shiftHead), text);
        else
            newPos = paragraphUpPos(cursor, text);
    } else if (key == Qt::Key_Down) {
        if (cmd)
            newPos = len;
        else if (alt && shift)
            newPos = paragraphDownPos(selectionExtendFrom(Qt::Key_Down, cursor, selStart, selEnd, shiftHead), text);
        else
            newPos = paragraphDownPos(cursor, text);
    } else {
        return notHandled();
    }

    return moveToResult(newPos, shift);
}

QVariantMap EditHelper::dispatchMacBackspace(int key, int modifiers,
                                             const QString &text, int cursor,
                                             int selStart, int selEnd) const
{
    if (key != Qt::Key_Backspace && key != Qt::Key_Delete)
        return notHandled();

    const bool cmd = (modifiers & Qt::ControlModifier) != 0;
    const bool alt = (modifiers & Qt::AltModifier) != 0;
    const int len = text.length();

    if (!cmd && !alt) {
        QVariantMap m = handledAction(QStringLiteral("replaceText"));
        m.insert(QStringLiteral("beginEdit"), true);
        if (key == Qt::Key_Backspace) {
            if (selStart != selEnd) {
                const int a = qMin(selStart, selEnd);
                const int b = qMax(selStart, selEnd);
                m.insert(QStringLiteral("text"), text.left(a) + text.mid(b));
                m.insert(QStringLiteral("cursor"), a);
            } else if (cursor > 0) {
                m.insert(QStringLiteral("text"), text.left(cursor - 1) + text.mid(cursor));
                m.insert(QStringLiteral("cursor"), cursor - 1);
            } else {
                m.insert(QStringLiteral("text"), text);
                m.insert(QStringLiteral("cursor"), cursor);
            }
        } else {
            if (selStart != selEnd) {
                const int a = qMin(selStart, selEnd);
                const int b = qMax(selStart, selEnd);
                m.insert(QStringLiteral("text"), text.left(a) + text.mid(b));
                m.insert(QStringLiteral("cursor"), a);
            } else if (cursor < len) {
                m.insert(QStringLiteral("text"), text.left(cursor) + text.mid(cursor + 1));
                m.insert(QStringLiteral("cursor"), cursor);
            } else {
                m.insert(QStringLiteral("text"), text);
                m.insert(QStringLiteral("cursor"), cursor);
            }
        }
        return m;
    }

    if (cursor <= 0 && len == 0) {
        QVariantMap m = handledAction(QStringLiteral("noop"));
        return m;
    }

    if (selStart != selEnd) {
        const int selA = qMin(selStart, selEnd);
        const int selB = qMax(selStart, selEnd);
        QVariantMap m = handledAction(QStringLiteral("replaceText"));
        m.insert(QStringLiteral("beginEdit"), true);
        m.insert(QStringLiteral("text"), text.left(selA) + text.mid(selB));
        m.insert(QStringLiteral("cursor"), selA);
        return m;
    }

    const int start = cmd ? deleteLineLeftPos(cursor, text) : deleteWordLeftPos(cursor, text);
    const int end = cursor;
    if (start < end) {
        QVariantMap m = handledAction(QStringLiteral("replaceText"));
        m.insert(QStringLiteral("beginEdit"), true);
        m.insert(QStringLiteral("text"), text.left(start) + text.mid(end));
        m.insert(QStringLiteral("cursor"), start);
        return m;
    }

    QVariantMap m = handledAction(QStringLiteral("noop"));
    return m;
}

void EditHelper::setQueryItem(QObject *queryItem)
{
    m_queryItem = queryItem;
}

EditHelper::QueryRect EditHelper::queryRectAt(int pos) const
{
    QueryRect out;
    if (!m_queryItem)
        return out;
    QVariant ret;
    if (!QMetaObject::invokeMethod(m_queryItem, "positionToRectangle",
            Q_RETURN_ARG(QVariant, ret),
            Q_ARG(QVariant, pos)))
        return out;
    const QVariantMap m = ret.toMap();
    if (!m.isEmpty()) {
        out.x = m.value(QStringLiteral("x")).toReal();
        out.y = m.value(QStringLiteral("y")).toReal();
        out.height = m.value(QStringLiteral("height")).toReal();
        return out;
    }
    const QRectF rf = ret.toRectF();
    out.x = rf.x();
    out.y = rf.y();
    out.height = rf.height();
    return out;
}

int EditHelper::queryTextLength() const
{
    if (!m_queryItem)
        return 0;
    return m_queryItem->property("text").toString().length();
}

int EditHelper::visualLineDownPos(int pos, qreal gx) const
{
    const int len = queryTextLength();
    if (pos >= len)
        return len;
    const QueryRect curRect = queryRectAt(pos);
    const qreal goalXUse = (gx >= 0) ? gx : curRect.x;
    const qreal curY = curRect.y;
    const qreal h = curRect.height;
    int minGap = 3;
    if (h >= 40)
        minGap = 10;
    int best = -1;
    qreal bestDist = 1e12;
    qreal targetY = -1;
    for (int p = pos + 1; p < len; p++) {
        const QueryRect r = queryRectAt(p);
        if (r.y <= curY + minGap)
            continue;
        if (targetY < 0)
            targetY = r.y;
        if (qAbs(r.y - targetY) > 0.5)
            break;
        const qreal dist = qAbs(r.x - goalXUse);
        if (best < 0 || dist < bestDist) {
            best = p;
            bestDist = dist;
        }
    }
    return best >= 0 ? best : len;
}

int EditHelper::visualLineUpPos(int pos, qreal gx) const
{
    if (pos <= 0)
        return 0;
    const QueryRect curRect = queryRectAt(pos);
    const qreal goalXUse = (gx >= 0) ? gx : curRect.x;
    const qreal curY = curRect.y;
    const qreal h = curRect.height;
    int minGap = 3;
    if (h >= 40)
        minGap = 10;
    int best = -1;
    qreal bestDist = 1e12;
    qreal targetY = -1;
    for (int p = pos - 1; p >= 0; p--) {
        const QueryRect r = queryRectAt(p);
        if (r.y >= curY - minGap)
            continue;
        if (targetY < 0)
            targetY = r.y;
        if (qAbs(r.y - targetY) > 0.5)
            break;
        const qreal dist = qAbs(r.x - goalXUse);
        if (best < 0 || dist < bestDist) {
            best = p;
            bestDist = dist;
        }
    }
    return best >= 0 ? best : 0;
}

int EditHelper::visualLineStartPos(int pos) const
{
    if (pos <= 0)
        return 0;
    const qreal curY = queryRectAt(pos).y;
    int best = pos;
    for (int p = pos - 1; p >= 0; p--) {
        if (qAbs(queryRectAt(p).y - curY) < 0.5)
            best = p;
        else
            break;
    }
    return best;
}

int EditHelper::visualLineEndPos(int pos) const
{
    const int len = queryTextLength();
    if (pos >= len)
        return len;
    const qreal curY = queryRectAt(pos).y;
    int best = pos;
    for (int p = pos + 1; p <= len; p++) {
        if (qAbs(queryRectAt(p).y - curY) < 0.5)
            best = p;
        else
            break;
    }
    return best;
}

bool EditHelper::lineWrapsVisually(int pos, const QString &text) const
{
    const int s = lineStartPos(pos, text);
    const int e = lineEndPos(pos, text);
    if (e <= s)
        return false;
    return qAbs(queryRectAt(s).y - queryRectAt(e).y) > 0.5;
}

bool EditHelper::onWrappedLine(int pos, const QString &text) const
{
    const int s = lineStartPos(pos, text);
    const int nl = text.indexOf(QLatin1Char('\n'), s);
    return nl == -1 || nl > lineEndPos(pos, text);
}

int EditHelper::macLineStartPos(int pos, const QString &text) const
{
    return onWrappedLine(pos, text) ? visualLineStartPos(pos) : lineStartPos(pos, text);
}

int EditHelper::macLineEndPos(int pos, const QString &text) const
{
    return onWrappedLine(pos, text) ? visualLineEndPos(pos) : lineEndPos(pos, text);
}

QVariantMap EditHelper::dispatchMacEditKeys(int key, int modifiers, const QString &text, int cursor) const
{
    const bool cmd = (modifiers & Qt::ControlModifier) != 0;
    const bool alt = (modifiers & Qt::AltModifier) != 0;

    if (cmd && !alt && key == Qt::Key_A) {
        QVariantMap m = handledAction(QStringLiteral("selectAll"));
        m.insert(QStringLiteral("len"), text.length());
        return m;
    }

    if (key == Qt::Key_Return && modifiers == Qt::NoModifier) {
        QVariantMap m = handledAction(QStringLiteral("insertNewline"));
        m.insert(QStringLiteral("pos"), cursor);
        return m;
    }

    return notHandled();
}
