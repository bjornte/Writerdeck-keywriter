#ifndef EDIT_HELPER_H
#define EDIT_HELPER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>

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

    // Phase A2: undo/redo stacks + one-char merge (behavior-identical).
    Q_INVOKABLE void clearUndoStacks();
    Q_INVOKABLE void syncUndoSnapshot(const QString &text, int cursor, int selStart, int selEnd);
    // Push current state (with merge), then skip the next onTextChanged push.
    // No-op while already capturing or skipping. Caller still gates mode/harness.
    Q_INVOKABLE void beginTextEdit(const QString &text, int cursor, int selStart, int selEnd);
    // TextEdit onTextChanged: skip/capture/merge push + snapshot sync.
    Q_INVOKABLE void notifyTextChanged(const QString &text, int cursor, int selStart, int selEnd);
    // Returns {text,cursor,selStart,selEnd} to apply, or invalid if stack empty.
    Q_INVOKABLE QVariant undo(const QString &text, int cursor, int selStart, int selEnd);
    Q_INVOKABLE QVariant redo(const QString &text, int cursor, int selStart, int selEnd);
    // Wrap QML restore so onTextChanged does not push.
    Q_INVOKABLE void beginRestore();
    Q_INVOKABLE void endRestore(const QString &text, int cursor, int selStart, int selEnd);

    // Phase B: chord -> action mapping (QML applies layout-dependent effects).
    // Returns {handled:bool, action:QString, ...}; handled=false when not matched.
    Q_INVOKABLE QVariantMap dispatchMacArrow(int key, int modifiers,
                                             const QString &text, int cursor,
                                             int selStart, int selEnd,
                                             int shiftAnchor, int shiftHead) const;
    Q_INVOKABLE QVariantMap dispatchMacBackspace(int key, int modifiers,
                                                 const QString &text, int cursor,
                                                 int selStart, int selEnd) const;
    Q_INVOKABLE QVariantMap dispatchMacEditKeys(int key, int modifiers,
                                                const QString &text, int cursor) const;

    // Phase C: visual-line math (layout via query TextEdit; QML keeps goalX + apply).
    // Q_INVOKABLE so QML can re-bind after load/harness prepare.
    Q_INVOKABLE void setQueryItem(QObject *queryItem);
    // gx < 0 means use the x at pos (same as omitted gx in QML).
    Q_INVOKABLE int visualLineDownPos(int pos, qreal gx = -1) const;
    Q_INVOKABLE int visualLineUpPos(int pos, qreal gx = -1) const;
    Q_INVOKABLE int visualLineStartPos(int pos) const;
    Q_INVOKABLE int visualLineEndPos(int pos) const;
    Q_INVOKABLE bool lineWrapsVisually(int pos, const QString &text) const;
    Q_INVOKABLE bool onWrappedLine(int pos, const QString &text) const;
    Q_INVOKABLE int macLineStartPos(int pos, const QString &text) const;
    Q_INVOKABLE int macLineEndPos(int pos, const QString &text) const;

private:
    struct QueryRect {
        qreal x = 0;
        qreal y = 0;
        qreal height = 0;
    };

    QueryRect queryRectAt(int pos) const;
    int queryTextLength() const;

    QObject *m_queryItem = nullptr;
    static int selectionExtendFrom(int key, int cursor, int selStart, int selEnd, int shiftHead);
    static QVariantMap notHandled();
    static QVariantMap handledAction(const QString &action);
    struct EditState {
        QString text;
        int cursor = 0;
        int selStart = 0;
        int selEnd = 0;
    };

    static EditState makeState(const QString &text, int cursor, int selStart, int selEnd);
    static QVariantMap stateToMap(const EditState &st);
    void pushWithMerge(const EditState &prevState, const QString &curText, int curCursor);

    QVector<EditState> m_undoStack;
    QVector<EditState> m_redoStack;
    bool m_undoCapture = false;
    bool m_skipTextUndoPush = false;
    EditState m_snapshot;
};

#endif
