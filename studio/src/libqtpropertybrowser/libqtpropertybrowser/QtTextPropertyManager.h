#ifndef LIBQTPROPERTYMANAGER_TEXT_PROPERTY_MANAGER_H
#define LIBQTPROPERTYMANAGER_TEXT_PROPERTY_MANAGER_H

#include <qtpropertybrowser/QtStringPropertyManager>

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif


class QT_QTPROPERTYBROWSER_EXPORT QtTextPropertyManager : public QtStringPropertyManager
{
    Q_OBJECT
public:
    QtTextPropertyManager(QObject *parent = 0):QtStringPropertyManager(parent) {}

protected:
    virtual QString valueText(const QtProperty *property) const;
};

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif //  LIBQTPROPERTYMANAGER_TEXT_PROPERTY_MANAGER_H
