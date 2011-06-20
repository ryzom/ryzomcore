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

#include "translation_manager_main_window.h"
// Project system includes
#include "../core/icore.h"
// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QLabel>
#include <QtGui/QTextEdit>
#include <QtCore/QSettings>
#include <QtGui/QErrorMessage>
#include <QtCore/QSignalMapper>
#include <QtGui/QTableWidget>
#include <QtGui/QListWidget>
#include <QtGui/QDockWidget>
#include <QtCore/QSize>
#include <QtGui/QGridLayout>
struct TEntryInfo
{
	string	SheetName;
};

set<string> getGenericNames();
map<string, TEntryInfo> getSimpleNames();
int extractBotNamesAll(map<string,list<string> > config_paths, string ligo_class_file, string trans_path, string work_path);

namespace Plugin
{

CMainWindow::CMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
         _ui.setupUi(this);
         
        _toolMenu = new QMenu(tr("Primitives"), _ui.toolBar);
        _ui.toolBar->addAction(_toolMenu->menuAction());
        
        QAction *extractBotNames = _toolMenu->addAction(tr("Extract bot names"));
        extractBotNames->setStatusTip(tr("Extract bot names from primitives"));
        connect(extractBotNames, SIGNAL(triggered()), this, SLOT(extractBotNames()));

        
        
         readSettings();
        m_undoStack = new QUndoStack(this);
}

void CMainWindow::readSettings()
{
            QSettings *settings = Core::ICore::instance()->settings();
            settings->beginGroup("translationmanager");

            list<string> paths = convertQStringList(settings->value("paths").toStringList()); /* paths */
            config_paths["paths"] = paths;
            list<string> pathsR = convertQStringList(settings->value("pathsR").toStringList()); /* pathsR */
            config_paths["pathsR"] = pathsR;
            list<string> georges = convertQStringList(settings->value("georges").toStringList()); /* georges */
            config_paths["georges"] = georges;
            list<string> filters = convertQStringList(settings->value("filters").toStringList()); /* filters */
            config_paths["filters"] = filters;
            
            languages = convertQStringList(settings->value("trlanguages").toStringList()); /* languages */
            ligo_path = settings->value("ligo").toString().toStdString();
            translation_path = settings->value("translation").toString().toStdString();
            work_path = settings->value("work").toString().toStdString();
            
            settings->endGroup();    
}

void CMainWindow::extractBotNames()
{
        if(verifySettings() == true) 
        {
           // int extract_bot_names = extractBotNamesAll(config_paths, ligo_path, translation_path, work_path);
 
            QGridLayout* mainLayout = new QGridLayout();
            
  
     
     //contentsWindow->setAllowedAreas(Qt::LeftDockWidgetArea);
 
    
     QListWidget *listWidget = new QListWidget(this);

       mainLayout->addWidget(QListWidget);
   
     
     
     QTableWidget *tableWidget = new QTableWidget(this);
  
     tableWidget->setRowCount(10);
     tableWidget->setColumnCount(5);   
     
     mainLayout->addWidget(QTableWidget);
     setCentralWidget(tableWidget);
        }    
}


bool CMainWindow::verifySettings()
{
        bool count_errors = false;
        
        QSettings *settings = Core::ICore::instance()->settings();
        settings->beginGroup("translationmanager");
        
        if(settings->value("paths").toList().count() == 0
                || settings->value("pathsR").toList().count() == 0
                || settings->value("georges").toList().count() == 0
                || settings->value("filters").toList().count() == 0)
        {
            QErrorMessage error_settings;
            error_settings.showMessage("Please write all the paths on the settings dialog.");
            error_settings.exec();
            count_errors = true;
        }
     
        if((settings->value("ligo").toString().isEmpty()
                || settings->value("translation").toString().isEmpty()
                || settings->value("work").toString().isEmpty()
                || settings->value("trlanguages").toList().count() == 0)
                && count_errors == false)
        {
            QErrorMessage error_settings;
            error_settings.showMessage("Please write the paths for ligo, translation and work files and the languages on the settings dialog." + settings->value("trlanguages").toString());
            error_settings.exec();    
            count_errors = true;
        }
        
        settings->endGroup();
        
        return !count_errors;
        
}

list<string> CMainWindow::convertQStringList(QStringList listq)
{       
        std::list<std::string> stdlist;
        
        Q_FOREACH(QString text, listq)
        {
                stdlist.push_back(text.toStdString());
        }
        
        return stdlist;
}

bool CCoreListener::closeMainWindow() const
{
    return true;
}

} /* namespace Plugin */