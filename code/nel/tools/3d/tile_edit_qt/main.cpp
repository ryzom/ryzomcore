#include "common.h"

#include "tile_edit_dlg.h"

#ifdef QT_STATICPLUGIN

#include <QtPlugin>

#if defined(Q_OS_WIN32)
	Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#elif defined(Q_OS_MAC)
	Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#elif defined(Q_OS_UNIX)
	Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

#endif

#include "../shared_widgets/common.h"

int main(int argc, char *argv[])
{

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need.
	NLMISC::CApplicationContext myApplicationContext;

	NLQT::preApplication();
	Q_INIT_RESOURCE(tile_edit_qt);
	QApplication app(argc, argv);

	CTile_edit_dlg tileEdit;
	tileEdit.show();

	return app.exec();
}