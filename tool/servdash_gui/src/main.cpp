// Ryzom Core - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// servdash_gui: the service dashboard of the quick start package as a Qt application, for
// development shards on Windows, Linux and macOS. It reads the same ini files as the Win32
// servdash and servdash_cli, shows NeL services through their console channel (--ctrl), and
// shuts the shard down when the window closes, like the original.

#include "dashboard.h"
#include "icons.h"
#include "shard_lock.h"
#include "main_window.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStyleFactory>
#include <QTimer>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QApplication::setApplicationName("Service Dashboard");
	QApplication::setWindowIcon(Icons::app());
	// Fusion draws the same frames on every platform and follows the system palette, where the
	// Windows 11 style leaves text edits without edges; QT_STYLE_OVERRIDE still wins. Qt 6.12 or
	// later: the Fusion of Qt 6.2 drops one pixel lines at a fractional display scale.
	if (qEnvironmentVariableIsEmpty("QT_STYLE_OVERRIDE"))
		QApplication::setStyle(QStyleFactory::create("Fusion"));

	// --screenshot <file.png> renders the window after the services came up and closes it again;
	// with QT_QPA_PLATFORM=offscreen this checks the layout without a desktop
	QString iniPath;
	QString screenshotPath;
	QString windowSize;
	QString iconDir;
	for (int i = 1; i < argc; ++i)
	{
		QString arg = QString::fromLocal8Bit(argv[i]);
		if (arg == "--screenshot" && i + 1 < argc)
			screenshotPath = QString::fromLocal8Bit(argv[++i]);
		else if (arg == "--size" && i + 1 < argc)
			windowSize = QString::fromLocal8Bit(argv[++i]);
		else if (arg == "--write-icon" && i + 1 < argc)
			iconDir = QString::fromLocal8Bit(argv[++i]);
		else if (iniPath.isEmpty())
			iniPath = arg;
	}
	if (!iconDir.isEmpty())
	{
		// --write-icon <dir> writes the application icon as PNG files, one per size, from which
		// the .ico of the Windows executable is packed
		for (int size : { 16, 20, 24, 32, 40, 48, 64, 128, 256 })
			Icons::appPixmap(size).save(QDir(iconDir).filePath(QString("servdash_gui_%1.png").arg(size)));
		return 0;
	}
	if (iniPath.isEmpty())
		iniPath = QFileDialog::getOpenFileName(nullptr, "Open a dashboard", QString(), "Dashboard files (*.ini);;All files (*)");
	if (iniPath.isEmpty())
		return 1;

	// the ini directory is the working directory, as with the Win32 tool
	QFileInfo info(iniPath);
	if (!info.exists())
	{
		QMessageBox::critical(nullptr, "Service Dashboard", QString("Cannot find '%1'.").arg(iniPath));
		return 1;
	}
	QDir::setCurrent(info.absolutePath());

	CDashboardConfig config;
	QString error;
	if (!CDashboardConfig::read(info.absoluteFilePath(), config, error))
	{
		QMessageBox::critical(nullptr, "Service Dashboard", error);
		return 1;
	}

	// one dashboard per shard, whichever tool it is
	CShardLock lock;
	QString holder;
	if (!lock.acquire(info.absoluteFilePath(), holder))
	{
		QMessageBox::critical(nullptr, "Service Dashboard", QString("'%1' is already open in %2.").arg(info.fileName(), holder));
		return 1;
	}

	CDashboard dashboard(config);
	CMainWindow window(&dashboard);
	if (!windowSize.isEmpty())
	{
		// --size WxH for the screenshot mode
		QStringList parts = windowSize.split('x');
		if (parts.size() == 2)
			window.resize(parts[0].toInt(), parts[1].toInt());
	}
	window.show();
	dashboard.start();
	if (!screenshotPath.isEmpty())
	{
		QTimer::singleShot(5000, &window, [&window, screenshotPath]() {
			window.grab().save(screenshotPath);
			window.close();
		});
	}
	return app.exec();
}
