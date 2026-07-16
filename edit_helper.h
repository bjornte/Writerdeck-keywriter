#ifndef EDIT_HELPER_H
#define EDIT_HELPER_H

#include <QObject>
#include <QString>

// Pure text math + undo for the editor (migration Phase A).
// QML still owns the on-screen TextEdit; this object is the typed brain.
class EditHelper : public QObject
{
    Q_OBJECT
public:
    explicit EditHelper(QObject *parent = nullptr);

    // Phase 0: unused by production QML; proves context-property wiring.
    Q_INVOKABLE QString ping() const;
};

#endif
