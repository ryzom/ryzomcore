#include <qglobal.h>

#ifdef Q_COMPILER_RVALUE_REFS
#undef Q_COMPILER_RVALUE_REFS
#endif

#include <QApplication>
#include <QFile>
#include <QMessageBox>

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

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	// build command line arguments string
	QString arguments;
	for(int i = 1 ; i <= argc ; i++)
	{
		arguments += argv[i];
		if (i < argc - 1)
			arguments += ' ';
	}

	// if command line starts with -f show file content
	if (arguments.startsWith("-f "))
	{
		QString fileName = arguments.remove(0, 3);

		QFile file(fileName);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			app.exit(-1);

		QString content;

		while (!file.atEnd())
		{
			QByteArray line = file.readLine();
			content.append(line);
		}

		QMessageBox::information(NULL, "message_box", content);
	}
	// else show arguments in message box content
	else
	{
		QMessageBox::information(NULL, "message_box", arguments);
	}

	app.exit(0);
}
