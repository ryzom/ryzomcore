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
#include "main_window.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QApplication::setApplicationName("Service Dashboard");
	// Fusion draws the same frames on every platform and follows the system palette, where the
	// Windows 11 style leaves text edits without edges; QT_STYLE_OVERRIDE still wins
	if (qEnvironmentVariableIsEmpty("QT_STYLE_OVERRIDE"))
		QApplication::setStyle(QStyleFactory::create("Fusion"));

	QString iniPath = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QString();
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

	CDashboard dashboard(config);
	CMainWindow window(&dashboard);
	window.show();
	dashboard.start();
	return app.exec();
}
