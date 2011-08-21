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
