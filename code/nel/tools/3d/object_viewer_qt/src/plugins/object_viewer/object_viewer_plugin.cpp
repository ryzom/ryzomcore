// Project includes
#include "object_viewer_plugin.h"
#include "graphics_settings_page.h"
#include "sound_settings_page.h"
#include "vegetable_settings_page.h"
#include "modules.h"
#include "../core/core_constants.h"

// NeL includes
#include "nel/misc/debug.h"

// Qt includes
#include <QtCore/QObject>

namespace NLQT
{
ObjectViewerPlugin::~ObjectViewerPlugin()
{
	Q_FOREACH(QObject *obj, _autoReleaseObjects)
	{
		_plugMan->removeObject(obj);
	}
	qDeleteAll(_autoReleaseObjects);
	_autoReleaseObjects.clear();
	Modules::release();
}

bool ObjectViewerPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;
	Modules::init();
	addAutoReleasedObject(new CObjectViewerContext());
	addAutoReleasedObject(new GraphicsSettingsPage());
	addAutoReleasedObject(new SoundSettingsPage());
	addAutoReleasedObject(new VegetableSettingsPage());
	return true;
}

void ObjectViewerPlugin::extensionsInitialized()
{
}

void ObjectViewerPlugin::shutdown()
{
//	Modules::release();
}

void ObjectViewerPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

void ObjectViewerPlugin::addAutoReleasedObject(QObject *obj)
{
	_plugMan->addObject(obj);
	_autoReleaseObjects.prepend(obj);
}

void CObjectViewerContext::open()
{
	Modules::mainWin().open();
}

QUndoStack *CObjectViewerContext::undoStack()
{
	return Modules::mainWin().getUndoStack();
}

QWidget *CObjectViewerContext::widget()
{
	return &Modules::mainWin();
}

}

Q_EXPORT_PLUGIN(NLQT::ObjectViewerPlugin)