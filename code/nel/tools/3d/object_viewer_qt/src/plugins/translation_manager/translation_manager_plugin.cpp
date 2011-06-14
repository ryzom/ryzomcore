// Project includes
#include "translation_manager_plugin.h"
#include "translation_manager_settings_page.h"
#include "simple_viewer.h"
// Project system includes
#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/imenu_manager.h"
#include "../../extension_system/iplugin_spec.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

int extractBotNamesAll(map<string,list<string> > config_paths, string ligo_class_file, string trans_path, string work_path);

namespace Plugin
{
TranslationManagerPlugin::~TranslationManagerPlugin()
{
	Q_FOREACH(QObject *obj, _autoReleaseObjects)
	{
		_plugMan->removeObject(obj);
	}
	qDeleteAll(_autoReleaseObjects);
	_autoReleaseObjects.clear();
}

bool TranslationManagerPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;

	addAutoReleasedObject(new CTranslationManagerSettingsPage(this));
	addAutoReleasedObject(new CTranslationManagerContext(this));
	addAutoReleasedObject(new CCoreListener(this));
	return true;
}

void TranslationManagerPlugin::extensionsInitialized()
{
	Core::ICore *core = Core::ICore::instance();
	Core::IMenuManager *menuManager = core->menuManager();
	//menuManager = _plugMan->getObject<Core::IMenuManager>();
	// Menu Actions for plugin
	QAction *aboutTManPlugin = new QAction("Translation Manager", this);
	// Locations
	QMenu *helpMenu = menuManager->menu(Core::Constants::M_HELP);
	QAction *aboutQtAction = menuManager->action(Core::Constants::ABOUT_QT);
	helpMenu->addSeparator();
	helpMenu->insertAction(aboutQtAction, aboutTManPlugin);
	QMenu *transMenu = menuManager->menuBar()->addMenu("Translation Manager");       
    // Words extraction     
    QAction *botnamesAct = new QAction("Extract bot_names", this);
    connect(botnamesAct, SIGNAL(triggered()), this, SLOT(extractBotNames()));
    transMenu->addAction(botnamesAct);
}

void TranslationManagerPlugin::extractBotNames() 
{
        // prepare the config paths
        list<string> paths,pathsR, georges, filters, languages;
        string ligo, translation, work;
        map<string, list<string> > config_paths;
        
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("translationmanager");
        
	paths = ConvertQStringList(settings->value("paths").toStringList()); /* paths */
        config_paths["paths"] = paths;
        pathsR = ConvertQStringList(settings->value("pathsR").toStringList()); /* pathsR */
        config_paths["pathsR"] = pathsR;
        georges = ConvertQStringList(settings->value("georges").toStringList()); /* georges */
        config_paths["georges"] = georges;
        filters = ConvertQStringList(settings->value("filters").toStringList()); /* filters */
        config_paths["filters"] = filters;
        languages = ConvertQStringList(settings->value("languages").toStringList()); /* languages */
        ligo = settings->value("ligo").toString().toStdString();
        translation = settings->value("translation").toString().toStdString();
        work = settings->value("work").toString().toStdString();
	settings->endGroup();
        
        extractBotNamesAll(config_paths, ligo, translation, work);

}

void TranslationManagerPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

list<string> TranslationManagerPlugin::ConvertQStringList(QStringList listq)
{       
        std::list<std::string> stdlist;
    	Q_FOREACH(QString text, listq)
	{
            stdlist.push_back(text.toStdString());
        }
        
        return stdlist;
}

QString TranslationManagerPlugin::name() const
{
	return "Translation Manager";
}

QString TranslationManagerPlugin::version() const
{
	return "0.1";
}

QString TranslationManagerPlugin::vendor() const
{
	return "cemycc";
}

QString TranslationManagerPlugin::description() const
{
	return "OVQT plugin for translation files.";
}

QStringList TranslationManagerPlugin::dependencies() const
{
	QStringList list;
	list.append(Core::Constants::OVQT_CORE_PLUGIN);
	list.append("ObjectViewer");
	return list;
}

void TranslationManagerPlugin::addAutoReleasedObject(QObject *obj)
{
	_plugMan->addObject(obj);
	_autoReleaseObjects.prepend(obj);
}

QObject* TranslationManagerPlugin::objectByName(const QString &name) const
{
	Q_FOREACH (QObject *qobj, _plugMan->allObjects())
	if (qobj->objectName() == name)
		return qobj;
	return 0;
}

ExtensionSystem::IPluginSpec *TranslationManagerPlugin::pluginByName(const QString &name) const
{
	Q_FOREACH (ExtensionSystem::IPluginSpec *spec, _plugMan->plugins())
	if (spec->name() == name)
		return spec;
	return 0;
}

}

Q_EXPORT_PLUGIN(Plugin::TranslationManagerPlugin)
