#include "edit_helper.h"

#include <QVariantMap>

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
