#ifndef ZONE_PAINTER_PLUGIN_H
#define ZONE_PAINTER_PLUGIN_H

// Project includes
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"
#include "zone_painter_main_window.h"

// NeL includes
#include <nel/misc/app_context.h>
#include <nel/misc/singleton.h>
#include <nel/3d/landscape.h>
#include <nel/3d/patch.h>
#include <nel/3d/zone.h>
#include <nel/3d/u_scene.h>

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

/*	class CZoneManager 
	{
		NLMISC_SAFE_SINGLETON_DECL(CZoneManager)
	public:
		//m_painterLandscape = static_cast<NL3D::CLandscapeModel *>
		
	private:
		NL3D::CLandscapeModel *m_painterLandscape;
		NL3D::CZone *m_currentZone;
	};
*/
class ZonePainterPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~ZonePainterPlugin();

	bool initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString);
	void extensionsInitialized();
	void setNelContext(NLMISC::INelContext *nelContext);

	void addAutoReleasedObject(QObject *obj);

public Q_SLOTS:
	void clickLoadZoneAction();
	void clickSaveZoneAction();
protected:
	NLMISC::CLibraryContext *m_LibContext;

private:
	ExtensionSystem::IPluginManager *m_plugMan;
	QList<QObject *> m_autoReleaseObjects;

	NL3D::CLandscapeModel *m_Landscape;
};

class CZonePainterContext: public Core::IContext
{
	Q_OBJECT
public:
	CZonePainterContext(QObject *parent = 0): IContext(parent)
	{
		m_zonePainterMainWindow = new ZonePainterMainWindow();
	}
	virtual ~CZonePainterContext() {}

	virtual QString id() const
	{
		return QLatin1String("ZonePainterContext");
	}
	virtual QString trName() const
	{
		return tr("Zone Painter");
	}
	virtual QIcon icon() const
	{
		return QIcon();
	}
	virtual QWidget *widget()
	{
		return m_zonePainterMainWindow;
	}

        virtual QUndoStack *undoStack()
        {
                return m_zonePainterMainWindow->getUndoStack();
        }
        virtual void open()
        {
        }


	ZonePainterMainWindow *m_zonePainterMainWindow;
};

} // namespace Plugin

#endif // ZONE_PAINTER_PLUGIN_H
