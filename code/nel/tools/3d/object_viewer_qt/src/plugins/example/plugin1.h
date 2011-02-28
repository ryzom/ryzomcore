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

namespace ExtensionSystem
{
class IPluginSpec;
}

namespace Plugin
{

class MyPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~MyPlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();

	void setNelContext(NLMISC::INelContext *nelContext);

	QString name() const;
	QString version() const;
	QString vendor() const;
	QString description() const;
	QStringList dependencies() const;

	void addAutoReleasedObject(QObject *obj);

	QObject *objectByName(const QString &name) const;
	ExtensionSystem::IPluginSpec *pluginByName(const QString &name) const;

protected:
	NLMISC::CLibraryContext *_LibContext;

private:
	ExtensionSystem::IPluginManager *_plugMan;
	QList<QObject *> _autoReleaseObjects;
};

class CExampleContext: public Core::IContext
{
	Q_OBJECT
public:
	CExampleContext(QObject *parent = 0): IContext(parent) {}
	virtual ~CExampleContext() {}

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
		return new CSimpleViewer();
	}
};

} // namespace Plugin

#endif // PLUGIN1_H
