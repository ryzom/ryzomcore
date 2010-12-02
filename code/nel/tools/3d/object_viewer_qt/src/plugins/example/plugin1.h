#ifndef PLUGIN1_H
#define PLUGIN1_H

#include "../../extension_system/iplugin.h"

#include "nel/misc/app_context.h"

#include <QtCore/QObject>

namespace NLMISC
{
class CLibraryContext;
}
namespace Plugin 
{

class MyPlugin : public QObject, public NLQT::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(NLQT::IPlugin)
public:

	bool initialize(NLQT::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();

	void setNelContext(NLMISC::INelContext *nelContext);

	QString name() const;
	QString version() const;
	QString vendor() const;
	QString description() const;

protected:
	NLMISC::CLibraryContext *_LibContext;

private:
	NLQT::IPluginManager *_plugMan;

};

} // namespace Plugin1

#endif // PLUGIN1_H
