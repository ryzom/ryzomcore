#include "plugin1.h"

#include <QtCore/QObject>
#include <QtGui/QMessageBox>

using namespace Plugin;

bool MyPlugin::initialize(NLQT::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	QString str;
	QList<NLQT::CPluginSpec *>  listPlug = pluginManager->plugins();

	Q_FOREACH (NLQT::CPluginSpec *plugSpec, listPlug)
		str += plugSpec->name();

	QMessageBox msgBox;
	msgBox.setText(str);
	msgBox.exec();
	return true;
}

void MyPlugin::extensionsInitialized()
{
	QMessageBox msgBox;
	msgBox.setText("extensionsInitialize Example Plugin.");
	msgBox.exec();
}

QString MyPlugin::name() const
{
	return "ExamplePlugin";
}

QString MyPlugin::version() const
{
	return "0.1";
}

QString MyPlugin::vendor() const
{
	return "dnk";
}

QString MyPlugin::description() const
{
	return "Example plugin";
}

Q_EXPORT_PLUGIN(MyPlugin)
