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

#include <nel/misc/types_nl.h>
#include "main_window.h"

// Qt includes
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QDockWidget>
#include <QCloseEvent>
#include <QSettings>
#include <QFileInfo>
#include <QStyleFactory>

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>

// Project includes
#include "georges_editor_qt_config.h"
#include "georges_editor_doc.h"
#include "georges_dock_widget.h"
#include "type_dialog.h"
#include "dfn_dialog.h"
#include "form_dialog.h"
#include "header_dialog.h"
#include "file_browser_dock.h"
#include "../../3d/shared_widgets/command_log.h"
#include "../../3d/shared_widgets/common.h"

using namespace NLQT;

// Find the project root by walking up from the current working directory
// looking for a ".nel" marker folder (same pattern as planar_reflection sample).
static std::string findProjectRoot()
{
	std::string rootPath = NLMISC::CPath::standardizePath(NLMISC::CPath::getCurrentPath(), false);

	while (!rootPath.empty())
	{
		if (NLMISC::CFile::isDirectory(rootPath + "/.nel"))
			return rootPath;
		std::string::size_type sep = NLMISC::CFile::getLastSeparator(rootPath);
		if (sep == std::string::npos)
			break;
		rootPath = rootPath.substr(0, sep);
	}
	return std::string();
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	_mdiArea(NULL),
	_fileBrowser(NULL),
	_commandLog(NULL), _commandLogDock(NULL)
{
	setObjectName("MainWindow");

	// Initialize NeL configuration (loads .cfg file, sets up search paths)
	m_Configuration.init(NLQT_CONFIG_FILE);

	// Discover project root by walking up from CWD looking for .nel marker
	// (same pattern as planar_reflection sample). Config paths are relative to this root.
	m_ProjectRoot = findProjectRoot();
	if (m_ProjectRoot.empty())
		nlwarning("Project root not found (no .nel marker folder in parent directories)");
	else
		nlinfo("Project root: %s", m_ProjectRoot.c_str());

	// Re-add SearchPaths relative to the project root.
	// CConfiguration::init() already tried them relative to CWD; re-add
	// relative to the discovered root so language data files are found.
	if (!m_ProjectRoot.empty())
	{
		NLMISC::CConfigFile::CVar *searchVar = m_Configuration.getConfigFile().getVarPtr("SearchPaths");
		if (searchVar)
		{
			for (uint i = 0; i < searchVar->size(); ++i)
			{
				std::string path = NLMISC::CPath::standardizePath(m_ProjectRoot + "/" + searchVar->asString(i), false);
				NLMISC::CPath::addSearchPath(path, true, false);
			}
		}
	}

	// Save original palette before any style changes
	m_OriginalPalette = QApplication::palette();

	// Register configuration callbacks (matching nel_qt pattern)
	m_Configuration.setAndCallback("QtStyle", CConfigCallback(this, &MainWindow::cfcbQtStyle));
	m_Configuration.setAndCallback("QtPalette", CConfigCallback(this, &MainWindow::cfcbQtPalette));
	m_Configuration.setAndCallback("RootSearchDirectory", CConfigCallback(this, &MainWindow::cfcbRootSearchDirectory));

	// Initialize internationalization
	m_Internationalization.init(&m_Configuration, NLQT_VERSION);
	m_Internationalization.enableCallback(CEmptyCallback(this, &MainWindow::incbLanguageCode));

	_mdiArea = new QMdiArea(this);
	_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setCentralWidget(_mdiArea);

	connect(_mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::onSubWindowActivated);

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	createDockWidgets();

	// Trigger initial language translation
	incbLanguageCode();

	updateMenus();
	updateRecentFileActions();

	// Restore window geometry and dock state if enabled
	if (m_Configuration.getValue("RememberWindowState", false))
	{
		QSettings settings("NeL", "Georges Editor Qt");
		restoreGeometry(settings.value("windowGeometry").toByteArray());
		restoreState(settings.value("windowState").toByteArray());
	}
}

MainWindow::~MainWindow()
{
	// Drop callbacks in reverse order of registration (matching nel_qt pattern)
	m_Internationalization.disableCallback(CEmptyCallback(this, &MainWindow::incbLanguageCode));
	m_Internationalization.release();
	m_Configuration.dropCallback("RootSearchDirectory");
	m_Configuration.dropCallback("QtPalette");
	m_Configuration.dropCallback("QtStyle");
	m_Configuration.release();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	// Save window geometry and dock state if enabled
	if (m_Configuration.getValue("RememberWindowState", false))
	{
		QSettings settings("NeL", "Georges Editor Qt");
		settings.setValue("windowGeometry", saveGeometry());
		settings.setValue("windowState", saveState());
	}
	event->accept();
}

// --- Configuration callbacks (matching nel_qt pattern) ---

void MainWindow::cfcbQtStyle(NLMISC::CConfigFile::CVar &var)
{
	QApplication::setStyle(QStyleFactory::create(var.asString().c_str()));
}

void MainWindow::cfcbQtPalette(NLMISC::CConfigFile::CVar &var)
{
	if (var.asBool())
		QApplication::setPalette(QApplication::style()->standardPalette());
	else
		QApplication::setPalette(m_OriginalPalette);
}

void MainWindow::cfcbRootSearchDirectory(NLMISC::CConfigFile::CVar &var)
{
	// Resolve RootSearchDirectory relative to the project root (the folder containing .nel).
	// Matches the planar_reflection sample pattern: config values are relative to project root,
	// not relative to the current working directory.
	NLMISC::CPath::removeAllAlternativeSearchPath();
	std::string rootDir = var.asString();
	if (!rootDir.empty() && !m_ProjectRoot.empty())
	{
		std::string fullPath = NLMISC::CPath::standardizePath(m_ProjectRoot + "/" + rootDir, false);
		nlinfo("Adding search path: %s (resolved from project root)", fullPath.c_str());
		NLMISC::CPath::addSearchPath(fullPath, true, true);
	}
	else if (!rootDir.empty())
	{
		// Fallback: no project root found, use path as-is (may be absolute or relative to CWD)
		nlwarning("No project root found, using RootSearchDirectory as-is: %s", rootDir.c_str());
		NLMISC::CPath::addSearchPath(rootDir, true, true);
	}
}

// --- Internationalization callback ---

void MainWindow::incbLanguageCode()
{
	setWindowTitle(nli18n("GeWindowTitle"));
	translateActions();
	translateMenus();
	translateToolBars();
	translateDockWindows();
}

// --- Translation methods ---

void MainWindow::translateActions()
{
	_newTypeAction->setText(nli18n("GeActionNewType"));
	_newDfnAction->setText(nli18n("GeActionNewDfn"));
	_newFormAction->setText(nli18n("GeActionNewForm"));
	_openAction->setText(nli18n("GeActionOpen"));
	_saveAction->setText(nli18n("GeActionSave"));
	_saveAllAction->setText(nli18n("GeActionSaveAll"));
	_closeAction->setText(nli18n("GeActionClose"));
	_closeAllAction->setText(nli18n("GeActionCloseAll"));
	_exitAction->setText(nli18n("GeActionExit"));

	_undoAction->setText(nli18n("GeActionUndo"));
	_redoAction->setText(nli18n("GeActionRedo"));
	_cutAction->setText(nli18n("GeActionCut"));
	_copyAction->setText(nli18n("GeActionCopy"));
	_pasteAction->setText(nli18n("GeActionPaste"));
	_insertAction->setText(nli18n("GeActionInsert"));
	_deleteAction->setText(nli18n("GeActionDelete"));
	_renameAction->setText(nli18n("GeActionRename"));
	_expandAllAction->setText(nli18n("GeActionExpandAll"));
	_collapseAllAction->setText(nli18n("GeActionCollapseAll"));

	_hold1Action->setText(nli18n("GeActionHold1"));
	_hold2Action->setText(nli18n("GeActionHold2"));
	_hold3Action->setText(nli18n("GeActionHold3"));
	_hold4Action->setText(nli18n("GeActionHold4"));
	_fetch1Action->setText(nli18n("GeActionFetch1"));
	_fetch2Action->setText(nli18n("GeActionFetch2"));
	_fetch3Action->setText(nli18n("GeActionFetch3"));
	_fetch4Action->setText(nli18n("GeActionFetch4"));

	_fileBrowserAction->setText(nli18n("GeActionFileBrowser"));
	_refreshAction->setText(nli18n("GeActionRefresh"));

	_aboutAction->setText(nli18n("GeActionAbout"));
}

void MainWindow::translateMenus()
{
	_fileMenu->setTitle(nli18n("GeMenuFile"));
	_editMenu->setTitle(nli18n("GeMenuEdit"));
	_viewMenu->setTitle(nli18n("GeMenuView"));
	_helpMenu->setTitle(nli18n("GeMenuHelp"));
	_recentMenu->setTitle(nli18n("GeMenuRecentFiles"));
}

void MainWindow::translateToolBars()
{
	_fileToolBar->setWindowTitle(nli18n("GeBarFile"));
	_editToolBar->setWindowTitle(nli18n("GeBarEdit"));
}

void MainWindow::translateDockWindows()
{
	_commandLogDock->setWindowTitle(nli18n("GeWidgetConsole"));
	_fileBrowser->setWindowTitle(nli18n("GeWidgetFileBrowser"));
}

void MainWindow::createActions()
{
	// File actions
	_newTypeAction = new QAction(this);
	_newTypeAction->setShortcut(QKeySequence("Ctrl+T"));
	connect(_newTypeAction, &QAction::triggered, this, &MainWindow::onNewType);

	_newDfnAction = new QAction(this);
	_newDfnAction->setShortcut(QKeySequence("Ctrl+D"));
	connect(_newDfnAction, &QAction::triggered, this, &MainWindow::onNewDfn);

	_newFormAction = new QAction(this);
	_newFormAction->setShortcut(QKeySequence("Ctrl+N"));
	connect(_newFormAction, &QAction::triggered, this, &MainWindow::onNewForm);

	_openAction = new QAction(this);
	_openAction->setShortcut(QKeySequence::Open);
	_openAction->setIcon(QIcon(":/icons/worldbuilder.ico"));
	connect(_openAction, &QAction::triggered, this, &MainWindow::onOpen);

	_saveAction = new QAction(this);
	_saveAction->setShortcut(QKeySequence::Save);
	connect(_saveAction, &QAction::triggered, this, &MainWindow::onSave);

	_saveAllAction = new QAction(this);
	connect(_saveAllAction, &QAction::triggered, this, &MainWindow::onSaveAll);

	_closeAction = new QAction(this);
	_closeAction->setShortcut(QKeySequence::Close);
	connect(_closeAction, &QAction::triggered, this, &MainWindow::onClose);

	_closeAllAction = new QAction(this);
	connect(_closeAllAction, &QAction::triggered, this, &MainWindow::onCloseAll);

	_exitAction = new QAction(this);
	_exitAction->setShortcut(QKeySequence::Quit);
	connect(_exitAction, &QAction::triggered, qApp, &QApplication::closeAllWindows);

	// Recent file actions
	for (int i = 0; i < MaxRecentFiles; ++i)
	{
		_recentFileActions[i] = new QAction(this);
		_recentFileActions[i]->setVisible(false);
		connect(_recentFileActions[i], &QAction::triggered, this, &MainWindow::onOpenRecentFile);
	}

	// Edit actions
	_undoAction = new QAction(this);
	_undoAction->setShortcut(QKeySequence::Undo);
	connect(_undoAction, &QAction::triggered, this, &MainWindow::onUndo);

	_redoAction = new QAction(this);
	_redoAction->setShortcut(QKeySequence::Redo);
	connect(_redoAction, &QAction::triggered, this, &MainWindow::onRedo);

	_cutAction = new QAction(this);
	_cutAction->setShortcut(QKeySequence::Cut);
	connect(_cutAction, &QAction::triggered, this, &MainWindow::onCut);

	_copyAction = new QAction(this);
	_copyAction->setShortcut(QKeySequence::Copy);
	connect(_copyAction, &QAction::triggered, this, &MainWindow::onCopy);

	_pasteAction = new QAction(this);
	_pasteAction->setShortcut(QKeySequence::Paste);
	connect(_pasteAction, &QAction::triggered, this, &MainWindow::onPaste);

	_insertAction = new QAction(this);
	_insertAction->setShortcut(QKeySequence("Ins"));
	connect(_insertAction, &QAction::triggered, this, &MainWindow::onInsert);

	_deleteAction = new QAction(this);
	_deleteAction->setShortcut(QKeySequence::Delete);
	connect(_deleteAction, &QAction::triggered, this, &MainWindow::onDelete);

	_renameAction = new QAction(this);
	_renameAction->setShortcut(QKeySequence("F2"));
	connect(_renameAction, &QAction::triggered, this, &MainWindow::onRename);

	// Hold/Fetch actions
	_hold1Action = new QAction(this);
	_hold2Action = new QAction(this);
	_hold3Action = new QAction(this);
	_hold4Action = new QAction(this);
	_fetch1Action = new QAction(this);
	_fetch2Action = new QAction(this);
	_fetch3Action = new QAction(this);
	_fetch4Action = new QAction(this);

	_expandAllAction = new QAction(this);
	connect(_expandAllAction, &QAction::triggered, this, &MainWindow::onExpandAll);

	_collapseAllAction = new QAction(this);
	connect(_collapseAllAction, &QAction::triggered, this, &MainWindow::onCollapseAll);

	// View actions
	_fileBrowserAction = new QAction(this);
	_fileBrowserAction->setCheckable(true);
	_fileBrowserAction->setChecked(true);
	connect(_fileBrowserAction, &QAction::triggered, this, &MainWindow::onToggleFileBrowser);

	_refreshAction = new QAction(this);
	_refreshAction->setShortcut(QKeySequence("F5"));
	connect(_refreshAction, &QAction::triggered, this, &MainWindow::onRefresh);

	// Help actions
	_aboutAction = new QAction(this);
	connect(_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createMenus()
{
	_fileMenu = menuBar()->addMenu("");
	_fileMenu->addAction(_newTypeAction);
	_fileMenu->addAction(_newDfnAction);
	_fileMenu->addAction(_newFormAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_openAction);

	// Recent files submenu
	_recentMenu = _fileMenu->addMenu("");
	for (int i = 0; i < MaxRecentFiles; ++i)
		_recentMenu->addAction(_recentFileActions[i]);

	_fileMenu->addSeparator();
	_fileMenu->addAction(_saveAction);
	_fileMenu->addAction(_saveAllAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_closeAction);
	_fileMenu->addAction(_closeAllAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_exitAction);

	_editMenu = menuBar()->addMenu("");
	_editMenu->addAction(_undoAction);
	_editMenu->addAction(_redoAction);
	_editMenu->addSeparator();
	_editMenu->addAction(_cutAction);
	_editMenu->addAction(_copyAction);
	_editMenu->addAction(_pasteAction);
	_editMenu->addSeparator();
	_editMenu->addAction(_insertAction);
	_editMenu->addAction(_deleteAction);
	_editMenu->addAction(_renameAction);
	_editMenu->addSeparator();
	_editMenu->addAction(_hold1Action);
	_editMenu->addAction(_hold2Action);
	_editMenu->addAction(_hold3Action);
	_editMenu->addAction(_hold4Action);
	_editMenu->addSeparator();
	_editMenu->addAction(_fetch1Action);
	_editMenu->addAction(_fetch2Action);
	_editMenu->addAction(_fetch3Action);
	_editMenu->addAction(_fetch4Action);
	_editMenu->addSeparator();
	_editMenu->addAction(_expandAllAction);
	_editMenu->addAction(_collapseAllAction);

	_viewMenu = menuBar()->addMenu("");
	_viewMenu->addAction(_fileBrowserAction);
	// Console toggle action is added by createDockWidgets()
	_viewMenu->addSeparator();
	_viewMenu->addAction(_refreshAction);

	_helpMenu = menuBar()->addMenu("");
	_helpMenu->addAction(_aboutAction);
}

void MainWindow::createToolBars()
{
	_fileToolBar = addToolBar("");
	_fileToolBar->addAction(_openAction);
	_fileToolBar->addAction(_saveAction);

	_editToolBar = addToolBar("");
	_editToolBar->addAction(_undoAction);
	_editToolBar->addAction(_redoAction);
	_editToolBar->addSeparator();
	_editToolBar->addAction(_cutAction);
	_editToolBar->addAction(_copyAction);
	_editToolBar->addAction(_pasteAction);
}

void MainWindow::createStatusBar()
{
	statusBar()->showMessage(nli18n("GeStatusReady"));
}

void MainWindow::createDockWidgets()
{
	_fileBrowser = new FileBrowserDock(this, this);
	addDockWidget(Qt::LeftDockWidgetArea, _fileBrowser);
	connect(_fileBrowser, &FileBrowserDock::fileDoubleClicked, this, &MainWindow::openDocument);

	// Console dock using shared CCommandLogDisplayer (same as Panoply Preview)
	{
		_commandLogDock = new QDockWidget(this);
		_commandLogDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		_commandLog = new NLQT::CCommandLogDisplayer(_commandLogDock);
		_commandLogDock->setWidget(_commandLog);
		addDockWidget(Qt::BottomDockWidgetArea, _commandLogDock);
		_viewMenu->addAction(_commandLogDock->toggleViewAction());
	}
}

void MainWindow::openDocument(const QString &path)
{
	// Check if already open
	QMdiSubWindow *existing = findMdiChild(path);
	if (existing)
	{
		_mdiArea->setActiveSubWindow(existing);
		return;
	}

	GeorgesEditorDoc *doc = new GeorgesEditorDoc();
	if (!doc->open(path))
	{
		nlwarning("Failed to open: %s", path.toUtf8().constData());
		delete doc;
		return;
	}

	// Create MDI child with splitter
	QSplitter *splitter = new QSplitter(Qt::Horizontal);

	GeorgesDockWidget *treeView = new GeorgesDockWidget(splitter);
	treeView->setDocument(doc);

	QStackedWidget *editorStack = new QStackedWidget(splitter);

	HeaderDialog *headerDlg = new HeaderDialog(editorStack);
	headerDlg->setDocument(doc);
	editorStack->addWidget(headerDlg);

	TypeDialog *typeDlg = new TypeDialog(editorStack);
	typeDlg->setDocument(doc);
	editorStack->addWidget(typeDlg);

	DfnDialog *dfnDlg = new DfnDialog(editorStack);
	dfnDlg->setDocument(doc);
	editorStack->addWidget(dfnDlg);

	FormDialog *formDlg = new FormDialog(editorStack);
	formDlg->setDocument(doc);
	editorStack->addWidget(formDlg);

	splitter->addWidget(treeView);
	splitter->addWidget(editorStack);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 2);

	// Show appropriate initial panel
	if (doc->isType())
		editorStack->setCurrentWidget(typeDlg);
	else if (doc->isDfn())
		editorStack->setCurrentWidget(dfnDlg);
	else if (doc->isForm())
		editorStack->setCurrentWidget(formDlg);

	// Connect tree selection to panel switching
	connect(treeView, &GeorgesDockWidget::itemSelected, [editorStack, headerDlg, typeDlg, dfnDlg, formDlg](CGeorgesEditDocSub *sub)
	{
		if (!sub)
			return;
		switch (sub->getType())
		{
		case CGeorgesEditDocSub::Header:
			headerDlg->updateFromDocument();
			editorStack->setCurrentWidget(headerDlg);
			break;
		case CGeorgesEditDocSub::Type:
			typeDlg->updateFromDocument();
			editorStack->setCurrentWidget(typeDlg);
			break;
		case CGeorgesEditDocSub::Dfn:
			dfnDlg->updateFromDocument();
			editorStack->setCurrentWidget(dfnDlg);
			break;
		case CGeorgesEditDocSub::Form:
			formDlg->updateFromDocument(sub);
			editorStack->setCurrentWidget(formDlg);
			break;
		default:
			break;
		}
	});

	QMdiSubWindow *subWindow = _mdiArea->addSubWindow(splitter);
	subWindow->setWindowTitle(doc->getFileName());
	subWindow->setProperty("documentPath", path);
	subWindow->setProperty("documentPtr", QVariant::fromValue(static_cast<void *>(doc)));
	subWindow->show();

	nlinfo("Opened: %s", path.toUtf8().constData());
	addRecentFile(path);
	updateMenus();
}

void MainWindow::createNewType()
{
	GeorgesEditorDoc *doc = new GeorgesEditorDoc();
	QSplitter *splitter = new QSplitter(Qt::Horizontal);
	GeorgesDockWidget *treeView = new GeorgesDockWidget(splitter);
	treeView->setDocument(doc);

	QStackedWidget *editorStack = new QStackedWidget(splitter);
	TypeDialog *typeDlg = new TypeDialog(editorStack);
	typeDlg->setDocument(doc);
	editorStack->addWidget(typeDlg);
	editorStack->setCurrentWidget(typeDlg);

	splitter->addWidget(treeView);
	splitter->addWidget(editorStack);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 2);

	QMdiSubWindow *subWindow = _mdiArea->addSubWindow(splitter);
	subWindow->setWindowTitle(nli18n("GeUntitledTyp"));
	subWindow->setProperty("documentPtr", QVariant::fromValue(static_cast<void *>(doc)));
	subWindow->show();
}

void MainWindow::createNewDfn()
{
	GeorgesEditorDoc *doc = new GeorgesEditorDoc();
	QSplitter *splitter = new QSplitter(Qt::Horizontal);
	GeorgesDockWidget *treeView = new GeorgesDockWidget(splitter);
	treeView->setDocument(doc);

	QStackedWidget *editorStack = new QStackedWidget(splitter);
	DfnDialog *dfnDlg = new DfnDialog(editorStack);
	dfnDlg->setDocument(doc);
	editorStack->addWidget(dfnDlg);
	editorStack->setCurrentWidget(dfnDlg);

	splitter->addWidget(treeView);
	splitter->addWidget(editorStack);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 2);

	QMdiSubWindow *subWindow = _mdiArea->addSubWindow(splitter);
	subWindow->setWindowTitle(nli18n("GeUntitledDfn"));
	subWindow->setProperty("documentPtr", QVariant::fromValue(static_cast<void *>(doc)));
	subWindow->show();
}

void MainWindow::createNewForm(const QString &dfnName)
{
	Q_UNUSED(dfnName);
	GeorgesEditorDoc *doc = new GeorgesEditorDoc();
	QSplitter *splitter = new QSplitter(Qt::Horizontal);
	GeorgesDockWidget *treeView = new GeorgesDockWidget(splitter);
	treeView->setDocument(doc);

	QStackedWidget *editorStack = new QStackedWidget(splitter);
	FormDialog *formDlg = new FormDialog(editorStack);
	formDlg->setDocument(doc);
	editorStack->addWidget(formDlg);
	editorStack->setCurrentWidget(formDlg);

	splitter->addWidget(treeView);
	splitter->addWidget(editorStack);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 2);

	QMdiSubWindow *subWindow = _mdiArea->addSubWindow(splitter);
	subWindow->setWindowTitle(nli18n("GeUntitledForm"));
	subWindow->setProperty("documentPtr", QVariant::fromValue(static_cast<void *>(doc)));
	subWindow->show();
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName) const
{
	for (QMdiSubWindow *window : _mdiArea->subWindowList())
	{
		if (window->property("documentPath").toString() == fileName)
			return window;
	}
	return nullptr;
}

void MainWindow::onNewType()
{
	createNewType();
}

void MainWindow::onNewDfn()
{
	createNewDfn();
}

void MainWindow::onNewForm()
{
	createNewForm();
}

void MainWindow::onOpen()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		nli18n("GeOpenFileTitle"),
		QString(),
		nli18n("GeOpenFileFilter"));
	if (!fileName.isEmpty())
		openDocument(fileName);
}

void MainWindow::onSave()
{
	QMdiSubWindow *active = _mdiArea->activeSubWindow();
	if (!active)
		return;

	GeorgesEditorDoc *doc = static_cast<GeorgesEditorDoc *>(active->property("documentPtr").value<void *>());
	if (doc)
	{
		if (doc->getFilePath().isEmpty())
		{
			QString fileName = QFileDialog::getSaveFileName(this,
				nli18n("GeSaveFileTitle"),
				QString(),
				nli18n("GeSaveFileFilter"));
			if (fileName.isEmpty())
				return;
			doc->save(fileName);
		}
		else
		{
			doc->save();
		}
		nlinfo("Saved: %s", doc->getFilePath().toUtf8().constData());
	}
}

void MainWindow::onSaveAll()
{
	for (QMdiSubWindow *window : _mdiArea->subWindowList())
	{
		GeorgesEditorDoc *doc = static_cast<GeorgesEditorDoc *>(window->property("documentPtr").value<void *>());
		if (doc && doc->isModified())
			doc->save();
	}
}

void MainWindow::onClose()
{
	_mdiArea->closeActiveSubWindow();
}

void MainWindow::onCloseAll()
{
	_mdiArea->closeAllSubWindows();
}

void MainWindow::onUndo()
{
	QMdiSubWindow *active = _mdiArea->activeSubWindow();
	if (!active)
		return;
	GeorgesEditorDoc *doc = static_cast<GeorgesEditorDoc *>(active->property("documentPtr").value<void *>());
	if (doc)
		doc->undo();
}

void MainWindow::onRedo()
{
	QMdiSubWindow *active = _mdiArea->activeSubWindow();
	if (!active)
		return;
	GeorgesEditorDoc *doc = static_cast<GeorgesEditorDoc *>(active->property("documentPtr").value<void *>());
	if (doc)
		doc->redo();
}

void MainWindow::onCut()
{
	// Stub: forward to active MDI child's tree view
}

void MainWindow::onCopy()
{
	// Stub: forward to active MDI child's tree view
}

void MainWindow::onPaste()
{
	// Stub: forward to active MDI child's tree view
}

void MainWindow::onInsert()
{
	// Stub: forward to active MDI child's tree view
}

void MainWindow::onDelete()
{
	// Stub: forward to active MDI child's tree view
}

void MainWindow::onRename()
{
	// Stub: forward to active MDI child's tree view
}

void MainWindow::onExpandAll()
{
	// Stub: forward to active MDI child's tree view
}

void MainWindow::onCollapseAll()
{
	// Stub: forward to active MDI child's tree view
}

void MainWindow::onToggleFileBrowser()
{
	_fileBrowser->setVisible(_fileBrowserAction->isChecked());
}

void MainWindow::onToggleOutputConsole()
{
	_commandLogDock->setVisible(!_commandLogDock->isVisible());
}

void MainWindow::onRefresh()
{
	_fileBrowser->refresh();
}

void MainWindow::onAbout()
{
	QMessageBox::about(this, nli18n("GeAboutTitle"), nli18n("GeAboutText"));
}

void MainWindow::onSubWindowActivated(QMdiSubWindow *window)
{
	Q_UNUSED(window);
	updateMenus();
}

void MainWindow::updateMenus()
{
	bool hasMdiChild = (_mdiArea->activeSubWindow() != nullptr);
	_saveAction->setEnabled(hasMdiChild);
	_saveAllAction->setEnabled(hasMdiChild);
	_closeAction->setEnabled(hasMdiChild);
	_closeAllAction->setEnabled(hasMdiChild);
	_undoAction->setEnabled(hasMdiChild);
	_redoAction->setEnabled(hasMdiChild);
	_cutAction->setEnabled(hasMdiChild);
	_copyAction->setEnabled(hasMdiChild);
	_pasteAction->setEnabled(hasMdiChild);
	_insertAction->setEnabled(hasMdiChild);
	_deleteAction->setEnabled(hasMdiChild);
	_renameAction->setEnabled(hasMdiChild);
	_expandAllAction->setEnabled(hasMdiChild);
	_collapseAllAction->setEnabled(hasMdiChild);
}

void MainWindow::addRecentFile(const QString &filePath)
{
	QSettings settings("NeL", "Georges Editor Qt");
	QStringList files = settings.value("recentFiles").toStringList();

	files.removeAll(filePath);
	files.prepend(filePath);

	int maxRecent = m_Configuration.getValue("MaxRecentFiles", (int)MaxRecentFiles);
	while (files.size() > maxRecent)
		files.removeLast();

	settings.setValue("recentFiles", files);
	updateRecentFileActions();
}

void MainWindow::updateRecentFileActions()
{
	QSettings settings("NeL", "Georges Editor Qt");
	QStringList files = settings.value("recentFiles").toStringList();

	int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

	for (int i = 0; i < numRecentFiles; ++i)
	{
		QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
		_recentFileActions[i]->setText(text);
		_recentFileActions[i]->setData(files[i]);
		_recentFileActions[i]->setToolTip(files[i]);
		_recentFileActions[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
		_recentFileActions[j]->setVisible(false);

	_recentMenu->setEnabled(numRecentFiles > 0);
}

void MainWindow::onOpenRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		openDocument(action->data().toString());
}
