#include <QtGui/QApplication>
#include "nel/misc/app_context.h"
#include "tile_edit_dlg.h"


int main(int argc, char *argv[])
{

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need.
	NLMISC::CApplicationContext myApplicationContext;

	Q_INIT_RESOURCE(tile_edit_qt);
	QApplication app(argc, argv);

	CTile_edit_dlg *tileEdit = new CTile_edit_dlg;
	tileEdit->show();

	return app.exec();
}