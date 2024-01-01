#ifndef LIBQTPROPERTYMANAGER_TEXT_EDITOR_FACTORY_H
#define LIBQTPROPERTYMANAGER_TEXT_EDITOR_FACTORY_H

#include <libqtpropertybrowser/QtTextEditWidget.h>
#include <libqtpropertybrowser/QtTextPropertyManager.h>

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QtTextEditorFactory : public QtAbstractEditorFactory<QtTextPropertyManager>
{
    Q_OBJECT

public:
	typedef QtTextEditWidget Editor;
	typedef QList<Editor *> EditorList;
	typedef QMap<QtProperty *, EditorList> PropertyToEditorListMap;
	typedef QMap<Editor *, QtProperty *> EditorToPropertyMap;

	QtTextEditorFactory(QObject *parent = nullptr);
    ~QtTextEditorFactory() override;

protected:
    QWidget *createEditor(QtTextPropertyManager *manager, QtProperty *property, QWidget *parent) override;
    void connectPropertyManager(QtTextPropertyManager *manager) override;
    void disconnectPropertyManager(QtTextPropertyManager *manager) override;

private Q_SLOTS:
	void slotPropertyChanged(QtProperty *property, const QString &value);
	void slotSetValue(const QString &value);
	void slotResetProperty();

private:
	PropertyToEditorListMap createdEditors;
	EditorToPropertyMap editorToProperty;
};

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#endif // LIBQTPROPERTYMANAGER_TEXT_EDITOR_FACTORY_H
