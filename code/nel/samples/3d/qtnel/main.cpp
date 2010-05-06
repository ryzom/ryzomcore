#include <QApplication>
#include <nel/misc/app_context.h>
#include "qnelwindow.h"

int main(int argc, char *argv[])
{
	NLMISC::CApplicationContext myApplicationContext;

	QApplication app(argc, argv);

	QNelWindow window;
	window.show();

	return app.exec();
}

