#ifndef PLUGIN1_H
#define PLUGIN1_H

#include "../../extension_system/iplugin.h"

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
private:
	NLQT::IPluginManager *_plugMan;

};

} // namespace Plugin1

#endif // PLUGIN1_H
