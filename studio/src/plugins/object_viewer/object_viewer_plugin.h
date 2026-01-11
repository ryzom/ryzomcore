#ifndef OBJECT_VIEWER_PLUGIN_H
#define OBJECT_VIEWER_PLUGIN_H

// Project includes
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"

// NeL includes
#include "nel/misc/app_context.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QIcon>

namespace NLMISC
{
class CLibraryContext;
}

namespace ExtensionSystem
{
class IPluginSpec;
}

namespace NLQT
{

class ObjectViewerPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~ObjectViewerPlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();
	void shutdown();
	void setNelContext(NLMISC::INelContext *nelContext);

	void addAutoReleasedObject(QObject *obj);

protected:
	NLMISC::CLibraryContext *_LibContext;

private:
	ExtensionSystem::IPluginManager *_plugMan;
	QList<QObject *> _autoReleaseObjects;
};

class CObjectViewerContext: public Core::IContext
{
	Q_OBJECT
public:
	CObjectViewerContext(QObject *parent = 0): IContext(parent) {}
	virtual ~CObjectViewerContext() {}

	virtual QString id() const
	{
		return QLatin1String("ObjectViewer");
	}

	virtual QString trName() const
	{
		return tr("Object Viewer");
	}

	virtual QIcon icon() const
	{
		return QIcon(":/icons/ic_nel_pill.png");
	}

	virtual QUndoStack *undoStack();

	virtual void open();

	virtual QWidget *widget();
};

} // namespace NLQT

#endif // OBJECT_VIEWER_PLUGIN_H
