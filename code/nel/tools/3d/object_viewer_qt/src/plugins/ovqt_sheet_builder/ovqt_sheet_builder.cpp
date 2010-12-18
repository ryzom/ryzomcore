// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "ovqt_sheet_builder.h"

#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

#include "../../extension_system/iplugin_spec.h"

#include "nel/misc/debug.h"
#include "sheetbuilderdialog.h"
#include "sheetbuilderconfgdialog.h"

using namespace Plugin;

bool SheetBuilderPlugin::initialize(NLQT::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	_plugMan = pluginManager;
	QMainWindow *wnd = qobject_cast<QMainWindow *>(objectByName("CMainWindow"));
	if (!wnd)
	{
		*errorString = tr("Not found MainWindow Object Viewer Qt.");
		return false;
	}
	QMenu *toolsMenu = qobject_cast<QMenu *>(objectByName("ovqt.Menu.Tools"));
	if (!toolsMenu)
	{
		*errorString = tr("Not found QMenu Tools.");
		return false;
	}
	return true;
}

void SheetBuilderPlugin::extensionsInitialized()
{
    QMenu *toolsMenu = qobject_cast<QMenu *>(objectByName("ovqt.Menu.Tools"));
    nlassert(toolsMenu);

    toolsMenu->addSeparator();

    QAction *actBuilder = toolsMenu->addAction("Sheet builder");
    connect(actBuilder, SIGNAL(triggered()), this, SLOT(execBuilderDialog()));
}

void SheetBuilderPlugin::execBuilderDialog()
{
    QMainWindow *wnd = qobject_cast<QMainWindow *>(objectByName("CMainWindow"));
    nlassert(wnd);

    SheetBuilderDialog dlg(wnd);
    dlg.exec();
}

void SheetBuilderPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
    // Ensure that a context doesn't exist yet.
    // This only applies to platforms without PIC, e.g. Windows.
    nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
    _LibContext = new NLMISC::CLibraryContext(*nelContext);
}

QString SheetBuilderPlugin::name() const
{
    return "Sheet builder";
}

QString SheetBuilderPlugin::version() const
{
    return "1.0";
}

QString SheetBuilderPlugin::vendor() const
{
    return "kharvd";
}

QString SheetBuilderPlugin::description() const
{
    return "make_sheet_id equivalent";
}

QObject* SheetBuilderPlugin::objectByName(const QString &name) const
{
    Q_FOREACH (QObject *qobj, _plugMan->allObjects())
            if (qobj->objectName() == name)
                return qobj;
    return 0;
}

NLQT::IPluginSpec *SheetBuilderPlugin::pluginByName(const QString &name) const
{
    Q_FOREACH (NLQT::IPluginSpec *spec, _plugMan->plugins())
            if (spec->name() == name)
                return spec;
    return 0;
}

Q_EXPORT_PLUGIN(SheetBuilderPlugin)
