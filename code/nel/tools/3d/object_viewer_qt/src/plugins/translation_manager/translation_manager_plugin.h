#ifndef TRANSLATION_MANAGER_PLUGIN_H
#define TRANSLATION_MANAGER_PLUGIN_H

// Project includes
#include "../../extension_system/iplugin.h"
#include "../core/icontext.h"
#include "simple_viewer.h"

// NeL includes
#include "nel/misc/app_context.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QIcon>

using namespace std;

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

class TranslationManagerPlugin : public QObject, public ExtensionSystem::IPlugin
{
	Q_OBJECT
	Q_INTERFACES(ExtensionSystem::IPlugin)
public:

	virtual ~TranslationManagerPlugin();

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
        list<string> ConvertQStringList(QStringList list);
        

private Q_SLOTS:
        void extractBotNames();
};

class CTranslationManagerContext: public Core::IContext
{
	Q_OBJECT
public:
	CTranslationManagerContext(QObject *parent = 0): IContext(parent)
	{
		m_simpleViewer = new CSimpleViewer();
	}

	virtual ~CTranslationManagerContext() {}

	virtual QString id() const
	{
		return QLatin1String("TranslationManagerContext");
	}
	virtual QString trName() const
	{
		return tr("Translation Manager");
	}
	virtual QIcon icon() const
	{
		return QIcon();
	}
	virtual QWidget *widget()
	{
		return m_simpleViewer;
	}

	CSimpleViewer *m_simpleViewer;
        
};

} // namespace Plugin

#endif // TRANSLATION_MANAGER_PLUGIN_H
