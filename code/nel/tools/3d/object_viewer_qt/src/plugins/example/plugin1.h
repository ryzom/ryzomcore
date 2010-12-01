#ifndef PLUGIN1_H
#define PLUGIN1_H

#include "../../extension_system/iplugin.h"
#include "../../extension_system/plugin_spec.h"

#include <QtCore/QObject>

namespace Plugin 
{

class MyPlugin : public QObject, public NLQT::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(NLQT::IPlugin)
public:

	bool initialize(NLQT::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();

	QString name() const;
	QString version() const;
	QString vendor() const;
	QString description() const;
};

} // namespace Plugin1

#endif // PLUGIN1_H
