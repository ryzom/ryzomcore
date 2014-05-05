#ifndef PLUGIN1_H
#define PLUGIN1_H

// Project includes
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"
#include "simple_viewer.h"

// NeL includes
#include "nel/misc/app_context.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QIcon>

namespace NLMISC
{
class CLibraryContext;
}

namespace Plugin
{

class ExamplePlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:
	ExamplePlugin();
	virtual ~ExamplePlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();
	void setNelContext(NLMISC::INelContext *nelContext);

	void addAutoReleasedObject(QObject *obj);

protected:
	NLMISC::CLibraryContext *m_LibContext;

private:
	ExtensionSystem::IPluginManager *m_plugMan;
	QList<QObject *> m_autoReleaseObjects;
};

class ExampleContext: public Core::IContext
{
	Q_OBJECT
public:
	ExampleContext(QObject *parent = 0): IContext(parent)
	{
		m_simpleViewer = new SimpleViewer();
	}

	virtual ~ExampleContext() {}

	virtual QString id() const
	{
		return QLatin1String("ExampleContext");
	}
	virtual QString trName() const
	{
		return tr("SimpleViewer");
	}
	virtual QIcon icon() const
	{
		return QIcon();
	}
	virtual QWidget *widget()
	{
		return m_simpleViewer;
	}

	virtual QUndoStack *undoStack()
	{
		return m_simpleViewer->m_undoStack;
	}

	virtual void open()
	{
	}

	SimpleViewer *m_simpleViewer;
};

} // namespace Plugin

#endif // PLUGIN1_H
