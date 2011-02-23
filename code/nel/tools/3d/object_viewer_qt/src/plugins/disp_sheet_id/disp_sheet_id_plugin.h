#ifndef PLUGIN1_H
#define PLUGIN1_H

#include "../../extension_system/iplugin.h"

#include "nel/misc/app_context.h"

#include <QtCore/QObject>

namespace NLMISC
{
class CLibraryContext;
}

namespace NLQT
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

private Q_SLOTS:
	void execMessageBox();

protected:
	NLMISC::CLibraryContext *_LibContext;

private:
	ExtensionSystem::IPluginManager *_plugMan;

};

} // namespace Plugin1

#endif // PLUGIN1_H
