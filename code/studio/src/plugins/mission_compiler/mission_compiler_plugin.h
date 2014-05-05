#ifndef MISSION_COMPILER_PLUGIN_H
#define MISSION_COMPILER_PLUGIN_H

// Project includes
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"
#include "mission_compiler_main_window.h"

// NeL includes
#include <nel/misc/app_context.h>
#include <nel/misc/singleton.h>

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

namespace MissionCompiler
{

class MissionCompilerPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~MissionCompilerPlugin();

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

class CMissionCompilerContext: public Core::IContext
{
	Q_OBJECT
public:
	CMissionCompilerContext(QObject *parent = 0): IContext(parent)
	{
		m_missionCompilerMainWindow = new MissionCompilerMainWindow();
	}
	virtual ~CMissionCompilerContext() {}

	virtual QString id() const
	{
		return QLatin1String("MissionCompilerContext");
	}
	virtual QString trName() const
	{
		return tr("Mission Compiler");
	}
	virtual QIcon icon() const
	{
		return QIcon();
	}
	virtual QWidget *widget()
	{
		return m_missionCompilerMainWindow;
	}

    virtual QUndoStack *undoStack()
    {
        return m_missionCompilerMainWindow->getUndoStack();
    }
    virtual void open() {}


	MissionCompilerMainWindow *m_missionCompilerMainWindow;
};

} // namespace MissionCompiler

#endif // MISSION_COMPILER_PLUGIN_H
