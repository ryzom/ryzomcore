#include <libqtpropertybrowser/QtTextEditorFactory.h>
#include <libqtpropertybrowser/QtTextEditWidget.h>

#if defined(Q_CC_MSVC)
#pragma warning(disable : 4786) /* MS VS 6: truncating debug info after 255 characters */
#endif

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

QtTextEditorFactory::QtTextEditorFactory(QObject *parent)
    : QtAbstractEditorFactory(parent)
{
}

QtTextEditorFactory::~QtTextEditorFactory()
{
	qDeleteAll(editorToProperty.keys());
}

QWidget *QtTextEditorFactory::createEditor(QtTextPropertyManager *manager, QtProperty *property, QWidget *parent)
{
	auto *editor = new Editor(parent);
	auto it = createdEditors.find(property);
	if (it == createdEditors.end())
	{
		it = createdEditors.insert(property, EditorList());
	}
	it.value().append(editor);
	editorToProperty.insert(editor, property);

	editor->setValue(manager->value(property));
	editor->setStateResetButton(property->isModified());

	connect(editor, SIGNAL(resetProperty()), this, SLOT(slotResetProperty()));
	connect(editor, SIGNAL(valueChanged(QString)), this, SLOT(slotSetValue(QString)));
	connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
	return editor;
}

void QtTextEditorFactory::connectPropertyManager(QtTextPropertyManager *manager)
{
	connect(manager, SIGNAL(valueChanged(QtProperty *, const QString &)),
	    this, SLOT(slotPropertyChanged(QtProperty *, const QString &)));
}

void QtTextEditorFactory::disconnectPropertyManager(QtTextPropertyManager *manager)
{
	disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QString &)),
	    this, SLOT(slotPropertyChanged(QtProperty *, const QString &)));
}

void QtTextEditorFactory::slotPropertyChanged(QtProperty *property, const QString &value)
{
	const PropertyToEditorListMap::iterator it = createdEditors.find(property);
	if (it == createdEditors.end())
	{
		return;
	}
	QListIterator itEditor(it.value());

	while (itEditor.hasNext())
	{
		QtTextEditWidget *editor = itEditor.next();
		editor->setValue(value);
		editor->setStateResetButton(property->isModified());
	}
}

void QtTextEditorFactory::slotSetValue(const QString &value)
{
	QObject *object = sender();
	const EditorToPropertyMap::ConstIterator ecend = editorToProperty.constEnd();
	for (EditorToPropertyMap::ConstIterator itEditor = editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
		if (itEditor.key() == object)
		{
			QtProperty *property = itEditor.value();
			QtTextPropertyManager *manager = propertyManager(property);
			if (!manager)
				return;
			manager->setValue(property, value);
			return;
		}
}

void QtTextEditorFactory::slotResetProperty()
{
	QObject *object = sender();
	const EditorToPropertyMap::ConstIterator ecend = editorToProperty.constEnd();
	for (EditorToPropertyMap::ConstIterator itEditor = editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
		if (itEditor.key() == object)
		{
			QtProperty *property = itEditor.value();
			QtTextPropertyManager *manager = propertyManager(property);
			if (!manager)
				return;
			// not supported anymore
			// manager->emitResetProperty(property);
			return;
		}
}

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif
