// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

// Project includes
#include "disp_sheet_id_plugin.h"
#include "sheet_id_view.h"
#include "../core/icore.h"
#include "../core/menu_manager.h"
#include "../core/core_constants.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMenuBar>

// NeL includes
#include "nel/misc/debug.h"

using namespace SheetIdViewPlugin;

DispSheetIdPlugin::DispSheetIdPlugin()
{
}

DispSheetIdPlugin::~DispSheetIdPlugin()
{
	Core::MenuManager *menuManager = Core::ICore::instance()->menuManager();
	QAction *a = menuManager->action( "SheetIdView" );
	menuManager->unregisterAction( "SheetIdView" );
	delete a;

	delete m_LibContext;
	m_LibContext = NULL;
}

bool DispSheetIdPlugin::initialize(ExtensionSystem::IPluginManager *pluginManager, QString *errorString)
{
	Q_UNUSED(errorString);
	m_plugMan = pluginManager;
	return true;
}

void DispSheetIdPlugin::extensionsInitialized()
{
	Core::MenuManager *menuManager = Core::ICore::instance()->menuManager();

	QMenu *sheetMenu = menuManager->menu(Core::Constants::M_SHEET);
	QAction *sheetIdViewAction = sheetMenu->addAction(tr("Sheet id view"));
	menuManager->registerAction(sheetIdViewAction, "SheetIdView");
	connect(sheetIdViewAction, SIGNAL(triggered()), this, SLOT(execBuilderDialog()));

	connect(sheetIdViewAction, SIGNAL(triggered()), this, SLOT(execMessageBox()));
}

void DispSheetIdPlugin::execMessageBox()
{
	SheetIdView dialog;
	dialog.show();
	dialog.exec();
}

void DispSheetIdPlugin::setNelContext(NLMISC::INelContext *nelContext)
{
#ifdef NL_OS_WINDOWS
	// Ensure that a context doesn't exist yet.
	// This only applies to platforms without PIC, e.g. Windows.
	nlassert(!NLMISC::INelContext::isContextInitialised());
#endif // NL_OS_WINDOWS
	m_LibContext = new NLMISC::CLibraryContext(*nelContext);
}

Q_EXPORT_PLUGIN(DispSheetIdPlugin)
