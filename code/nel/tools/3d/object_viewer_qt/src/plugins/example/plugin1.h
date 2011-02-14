#ifndef PLUGIN1_H
#define PLUGIN1_H

// Project includes
#include "../../extension_system/iplugin.h"
#include "simple_viewer.h"
#include "../core/iapp_page.h"

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

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();

	void setNelContext(NLMISC::INelContext *nelContext);

	QString name() const;
	QString version() const;
	QString vendor() const;
	QString description() const;
	QList<QString> dependencies() const;

	QObject *objectByName(const QString &name) const;
	ExtensionSystem::IPluginSpec *pluginByName(const QString &name) const;

protected:
	NLMISC::CLibraryContext *_LibContext;

private:
	ExtensionSystem::IPluginManager *_plugMan;

};

class CExampleAppPage: public QObject, public Core::IAppPage
{
	Q_OBJECT
	Q_INTERFACES(Core::IAppPage)
public:
	CExampleAppPage(QObject *parent = 0): QObject(parent) {}
	virtual ~CExampleAppPage() {}

	virtual QString id() const
	{
		return QLatin1String("ExampleAppPage");
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
