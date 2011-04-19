#ifndef ZONE_PAINTER_PLUGIN_H
#define ZONE_PAINTER_PLUGIN_H

// Project includes
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"
#include "zone_painter_main_window.h"

// NeL includes
#include "nel/misc/app_context.h"
#include <nel/3d/landscape.h>
#include <nel/3d/patch.h>
#include <nel/3d/zone.h>

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

	class CZoneManager  {
	public:
		NL3D::CZone zone;

		std::string getZoneInfo() {
			NL3D::CZone zone;
			zone.getNumPatchs();

			return "";
		}
	};

class ZonePainterPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~ZonePainterPlugin();

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
	ZonePainterMainWindow *m_zonePainterMainWindow;
};

} // namespace Plugin

#endif // ZONE_PAINTER_PLUGIN_H
