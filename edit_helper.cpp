#include "edit_helper.h"

EditHelper::EditHelper(QObject *parent)
    : QObject(parent)
{
}

QString EditHelper::ping() const
{
    return QStringLiteral("editHelper");
}
