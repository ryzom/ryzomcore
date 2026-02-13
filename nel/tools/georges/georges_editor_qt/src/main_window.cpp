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

#include "main_window.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QCloseEvent>
#include <QSettings>

#include "georges_editor_doc.h"
#include "georges_dock_widget.h"
#include "type_dialog.h"
#include "dfn_dialog.h"
#include "form_dialog.h"
#include "header_dialog.h"
#include "file_browser_dock.h"
#include "output_console_dock.h"
#include "settings_dialog.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle("Georges Editor Qt");
	resize(1024, 768);

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

	updateMenus();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
	// File actions
	_newTypeAction = new QAction(tr("New &Type"), this);
	_newTypeAction->setShortcut(QKeySequence("Ctrl+T"));
	connect(_newTypeAction, &QAction::triggered, this, &MainWindow::onNewType);

	_newDfnAction = new QAction(tr("New &DFN"), this);
	_newDfnAction->setShortcut(QKeySequence("Ctrl+D"));
	connect(_newDfnAction, &QAction::triggered, this, &MainWindow::onNewDfn);

	_newFormAction = new QAction(tr("New &Form"), this);
	_newFormAction->setShortcut(QKeySequence("Ctrl+N"));
	connect(_newFormAction, &QAction::triggered, this, &MainWindow::onNewForm);

	_openAction = new QAction(tr("&Open..."), this);
	_openAction->setShortcut(QKeySequence::Open);
	_openAction->setIcon(QIcon(":/icons/worldbuilder.ico"));
	connect(_openAction, &QAction::triggered, this, &MainWindow::onOpen);

	_saveAction = new QAction(tr("&Save"), this);
	_saveAction->setShortcut(QKeySequence::Save);
	connect(_saveAction, &QAction::triggered, this, &MainWindow::onSave);

	_saveAllAction = new QAction(tr("Save A&ll"), this);
	connect(_saveAllAction, &QAction::triggered, this, &MainWindow::onSaveAll);

	_closeAction = new QAction(tr("&Close"), this);
	_closeAction->setShortcut(QKeySequence::Close);
	connect(_closeAction, &QAction::triggered, this, &MainWindow::onClose);

	_closeAllAction = new QAction(tr("Close All"), this);
	connect(_closeAllAction, &QAction::triggered, this, &MainWindow::onCloseAll);

	_exitAction = new QAction(tr("E&xit"), this);
	_exitAction->setShortcut(QKeySequence::Quit);
	connect(_exitAction, &QAction::triggered, qApp, &QApplication::closeAllWindows);

	// Edit actions
	_undoAction = new QAction(tr("&Undo"), this);
	_undoAction->setShortcut(QKeySequence::Undo);
	connect(_undoAction, &QAction::triggered, this, &MainWindow::onUndo);

	_redoAction = new QAction(tr("&Redo"), this);
	_redoAction->setShortcut(QKeySequence::Redo);
	connect(_redoAction, &QAction::triggered, this, &MainWindow::onRedo);

	_cutAction = new QAction(tr("Cu&t"), this);
	_cutAction->setShortcut(QKeySequence::Cut);
	connect(_cutAction, &QAction::triggered, this, &MainWindow::onCut);

	_copyAction = new QAction(tr("&Copy"), this);
	_copyAction->setShortcut(QKeySequence::Copy);
	connect(_copyAction, &QAction::triggered, this, &MainWindow::onCopy);

	_pasteAction = new QAction(tr("&Paste"), this);
	_pasteAction->setShortcut(QKeySequence::Paste);
	connect(_pasteAction, &QAction::triggered, this, &MainWindow::onPaste);

	_insertAction = new QAction(tr("&Insert"), this);
	_insertAction->setShortcut(QKeySequence("Ins"));
	connect(_insertAction, &QAction::triggered, this, &MainWindow::onInsert);

	_deleteAction = new QAction(tr("&Delete"), this);
	_deleteAction->setShortcut(QKeySequence::Delete);
	connect(_deleteAction, &QAction::triggered, this, &MainWindow::onDelete);

	_renameAction = new QAction(tr("Re&name"), this);
	_renameAction->setShortcut(QKeySequence("F2"));
	connect(_renameAction, &QAction::triggered, this, &MainWindow::onRename);

	// Hold/Fetch actions
	_hold1Action = new QAction(tr("Hold 1"), this);
	_hold2Action = new QAction(tr("Hold 2"), this);
	_hold3Action = new QAction(tr("Hold 3"), this);
	_hold4Action = new QAction(tr("Hold 4"), this);
	_fetch1Action = new QAction(tr("Fetch 1"), this);
	_fetch2Action = new QAction(tr("Fetch 2"), this);
	_fetch3Action = new QAction(tr("Fetch 3"), this);
	_fetch4Action = new QAction(tr("Fetch 4"), this);

	_expandAllAction = new QAction(tr("&Expand All"), this);
	connect(_expandAllAction, &QAction::triggered, this, &MainWindow::onExpandAll);

	_collapseAllAction = new QAction(tr("Co&llapse All"), this);
	connect(_collapseAllAction, &QAction::triggered, this, &MainWindow::onCollapseAll);

	// View actions
	_fileBrowserAction = new QAction(tr("&File Browser"), this);
	_fileBrowserAction->setCheckable(true);
	_fileBrowserAction->setChecked(true);
	connect(_fileBrowserAction, &QAction::triggered, this, &MainWindow::onToggleFileBrowser);

	_outputConsoleAction = new QAction(tr("&Output Console"), this);
	_outputConsoleAction->setCheckable(true);
	_outputConsoleAction->setChecked(true);
	connect(_outputConsoleAction, &QAction::triggered, this, &MainWindow::onToggleOutputConsole);

	_refreshAction = new QAction(tr("&Refresh"), this);
	_refreshAction->setShortcut(QKeySequence("F5"));
	connect(_refreshAction, &QAction::triggered, this, &MainWindow::onRefresh);

	_settingsAction = new QAction(tr("&Settings..."), this);
	connect(_settingsAction, &QAction::triggered, this, &MainWindow::onSettings);

	// Help actions
	_aboutAction = new QAction(tr("&About"), this);
	connect(_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createMenus()
{
	_fileMenu = menuBar()->addMenu(tr("&File"));
	_fileMenu->addAction(_newTypeAction);
	_fileMenu->addAction(_newDfnAction);
	_fileMenu->addAction(_newFormAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_openAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_saveAction);
	_fileMenu->addAction(_saveAllAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_closeAction);
	_fileMenu->addAction(_closeAllAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_exitAction);

	_editMenu = menuBar()->addMenu(tr("&Edit"));
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

	_viewMenu = menuBar()->addMenu(tr("&View"));
	_viewMenu->addAction(_fileBrowserAction);
	_viewMenu->addAction(_outputConsoleAction);
	_viewMenu->addSeparator();
	_viewMenu->addAction(_refreshAction);
	_viewMenu->addAction(_settingsAction);

	_helpMenu = menuBar()->addMenu(tr("&Help"));
	_helpMenu->addAction(_aboutAction);
}

void MainWindow::createToolBars()
{
	_fileToolBar = addToolBar(tr("File"));
	_fileToolBar->addAction(_openAction);
	_fileToolBar->addAction(_saveAction);

	_editToolBar = addToolBar(tr("Edit"));
	_editToolBar->addAction(_undoAction);
	_editToolBar->addAction(_redoAction);
	_editToolBar->addSeparator();
	_editToolBar->addAction(_cutAction);
	_editToolBar->addAction(_copyAction);
	_editToolBar->addAction(_pasteAction);
}

void MainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createDockWidgets()
{
	_fileBrowser = new FileBrowserDock(this, this);
	addDockWidget(Qt::LeftDockWidgetArea, _fileBrowser);
	connect(_fileBrowser, &FileBrowserDock::fileDoubleClicked, this, &MainWindow::openDocument);

	_outputConsole = new OutputConsoleDock(this);
	addDockWidget(Qt::BottomDockWidgetArea, _outputConsole);
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
		_outputConsole->outputString(tr("Failed to open: %1").arg(path));
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

	_outputConsole->outputString(tr("Opened: %1").arg(path));
	updateMenus();
}

void MainWindow::createNewType()
{
	GeorgesEditorDoc *doc = new GeorgesEditorDoc();
	// Create empty type document (stub)
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
	subWindow->setWindowTitle(tr("Untitled.typ"));
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
	subWindow->setWindowTitle(tr("Untitled.dfn"));
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
	subWindow->setWindowTitle(tr("Untitled Form"));
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
		tr("Open Georges File"),
		QString(),
		tr("All Georges Files (*.typ *.dfn *.*);;Type Files (*.typ);;DFN Files (*.dfn);;All Files (*.*)"));
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
				tr("Save Georges File"),
				QString(),
				tr("All Georges Files (*.typ *.dfn *.*);;Type Files (*.typ);;DFN Files (*.dfn);;All Files (*.*)"));
			if (fileName.isEmpty())
				return;
			doc->save(fileName);
		}
		else
		{
			doc->save();
		}
		_outputConsole->outputString(tr("Saved: %1").arg(doc->getFilePath()));
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
	_outputConsole->setVisible(_outputConsoleAction->isChecked());
}

void MainWindow::onRefresh()
{
	_fileBrowser->refresh();
}

void MainWindow::onSettings()
{
	SettingsDialog dlg(this);
	dlg.loadSettings();
	if (dlg.exec() == QDialog::Accepted)
	{
		dlg.saveSettings();
		_fileBrowser->refresh();
	}
}

void MainWindow::onAbout()
{
	QMessageBox::about(this, tr("About Georges Editor Qt"),
		tr("Georges Editor Qt\n\nA Qt-based editor for NeL Georges files (.typ, .dfn, forms).\n\nPart of the NeL MMORPG Framework."));
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
