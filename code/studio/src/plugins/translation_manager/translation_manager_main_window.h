// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2011  Emanuel Costea <cemycc@gmail.com>
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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

// Project includes
#include "ui_translation_manager_main_window.h"
#include "translation_manager_editor.h"
#include "source_selection.h"
#include "editor_worksheet.h"
#include "editor_phrase.h"

// Project system includes
#include "../core/icore_listener.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QUndoStack>
#include <QtGui/QMainWindow>
#include <QtGui/QTableWidget>
#include <QtGui/QMenu>
#include <QtGui/QMdiSubWindow>
#include <QtCore/QSignalMapper>
#include <QtGui/QDialog>

// STL includes
#include <set>

// Nel includes
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"
#include "nel/ligo/ligo_config.h"

using namespace std;

namespace TranslationManager
{

class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CMainWindow(QWidget *parent = 0);
	~CMainWindow();
	QUndoStack *m_undoStack;

public:
	Ui::CMainWindow _ui;

private:
	// actions
	QAction *openAct;
	QAction *saveAct;
	QAction *saveAsAct;
	QMenu *windowMenu;
	QSignalMapper *windowMapper;
	// config
	QMap<string,bool> initialize_settings;
	QList<QString> filters;
	QList<QString> languages;
	QString level_design_path;
	QString primitives_path;
	QString translation_path;
	QString work_path;
	NLLIGO::CLigoConfig ligoConfig;

private Q_SLOTS:
	void extractBotNames();
	void extractWords(QString typeq);
	void open();
	void save();
	void saveAs();
	void setActiveSubWindow(QWidget *window);
	void updateWindowsList();
	void mergeSingleFile();

private:
	void openWorkFile(QString file);
	void updateToolbar(QMdiSubWindow *window);
	bool verifySettings();
	void readSettings();
	void removeMenus();
	void createMenus();
	void createToolbar();
	void initializeSettings(bool georges);
	std::list<std::string> convertQStringList(QStringList listq);
	CEditor *getEditorByWindowFilePath(const QString &fileName);
	// Worksheet specific functions
	CEditorWorksheet *getEditorByWorksheetType(const QString &type);
	bool isWorksheetEditor(QString filename);
	bool isPhraseEditor(QString filename);


	QMenu *menu;
};

class CCoreListener : public Core::ICoreListener
{
	Q_OBJECT

public:
	CCoreListener(CMainWindow *mainWindow, QObject *parent = 0): ICoreListener(parent)
	{
		m_MainWindow = mainWindow;
	}

	virtual ~CCoreListener() {}
	virtual bool closeMainWindow() const;

public:
	CMainWindow *m_MainWindow;
};

} // namespace TranslationManager

#endif
