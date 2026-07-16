#ifndef EDIT_HELPER_H
#define EDIT_HELPER_H

#include <QObject>
#include <QString>
#include <QVariant>

// Pure text math + undo for the editor (migration Phase A).
// QML still owns the on-screen TextEdit; this object is the typed brain.
class EditHelper : public QObject
{
    Q_OBJECT
public:
    explicit EditHelper(QObject *parent = nullptr);

    // Phase 0: unused by production QML; proves context-property wiring.
    Q_INVOKABLE QString ping() const;

    // Phase A1: pure string math (behavior-identical to former QML helpers).
    Q_INVOKABLE bool isSpaceChar(const QString &c) const;
    Q_INVOKABLE int lineStartPos(int pos, const QString &text) const;
    Q_INVOKABLE int lineEndPos(int pos, const QString &text) const;
    Q_INVOKABLE int lineCharCount(int lineStart, const QString &text) const;
    Q_INVOKABLE int wordLeftPos(int pos, const QString &text) const;
    Q_INVOKABLE int wordRightPos(int pos, const QString &text) const;
    Q_INVOKABLE int deleteWordLeftPos(int pos, const QString &text) const;
    Q_INVOKABLE int deleteLineLeftPos(int pos, const QString &text) const;
    Q_INVOKABLE int paragraphUpPos(int pos, const QString &text) const;
    Q_INVOKABLE int paragraphDownPos(int pos, const QString &text) const;
    // Returns {pos, len} or an invalid QVariant (undefined/null in QML).
    Q_INVOKABLE QVariant insertTextDelta(const QString &prevText, const QString &curText) const;
    // Insert index, or -1 if not a single-character insert.
    Q_INVOKABLE int isOneCharInsert(const QString &prevText, const QString &curText) const;
};

#endif
