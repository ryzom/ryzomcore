// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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
#include "../core/icore_listener.h"

// Nel includes
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QUndoStack>
#include <QtGui/QMainWindow>
#include <QtGui/QGridLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QMenu>
#include <QtGui/QMdiSubWindow>
#include <QtCore/QSignalMapper>



#include "ui_translation_manager_main_window.h"
#include <set>

class QWidget;

using namespace std;

namespace Plugin
{

struct CCelPos
{
        int col;
        int row;
};
    
class CMainWindow : public QMainWindow
{
	Q_OBJECT
public:
        CMainWindow(QWidget *parent = 0);
        virtual ~CMainWindow() {}
        QUndoStack *m_undoStack;
private:
        Ui::CMainWindow _ui;
        
        map<QMdiSubWindow*, list<CCelPos> > modifiedCells;
        // actions
        QAction *openAct;
        QAction *saveAct;
        QAction *saveAsAct;
        QMenu *windowMenu;
        QSignalMapper *windowMapper;
        // config
        map<string, list<string> > config_paths;
        list<string> languages;
        string ligo_path;
        string translation_path;
        string work_path;
private Q_SLOTS:
        void extractBotNames();
        void open();
        void save();
        void saveAs();
        void sheetEditorChanged(int, int);
        void setActiveSubWindow(QWidget *window);
        void activeSubWindowChanged();
private:
        void compareBotNames();
        bool verifySettings();
        void readSettings();
        void createMenus();
        void createToolbar();
        void updateWindowsList();
        list<string> convertQStringList(QStringList listq);
        
        
};

class CCoreListener : public Core::ICoreListener
{
	Q_OBJECT
public:
	CCoreListener(QObject *parent = 0): ICoreListener(parent) {}
	virtual ~CCoreListener() {}

	virtual bool closeMainWindow() const;
};

class CMdiSubWindow : public QMdiSubWindow 
{
   private:
       int window_type;
    public:
        int getWType()
        {
            return window_type;
        }
        void setWType(int nType)
        {
            window_type = nType;
        }
};

} // namespace Plugin

#endif // SIMPLE_VIEWER_H
