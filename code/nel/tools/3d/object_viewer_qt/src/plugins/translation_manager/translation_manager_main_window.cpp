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
#include "../core/core_constants.h"
#include "../core/imenu_manager.h"
#include "../../extension_system/iplugin_spec.h"
// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QLabel>
#include <QtGui/QTextEdit>
#include <QtCore/QSettings>
#include <QtGui/QErrorMessage>
#include <QtCore/QSignalMapper>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QListWidget>
#include <QtGui/QDockWidget>
#include <QtCore/QSize>
#include <QtGui/QGridLayout>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QFileDialog>

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
         
        
         readSettings();
         createToolbar();
        m_undoStack = new QUndoStack(this);
}

void CMainWindow::createToolbar()
{	
        // Tools menu
        Core::IMenuManager *menuManager = Core::ICore::instance()->menuManager();
        QMenu *translationManagerMenu = new QMenu("Translation Manager");
        QAction *extractBotNamesAct = translationManagerMenu->addAction("Extract bot names");
        extractBotNamesAct->setStatusTip(tr("Extract bot names from primitives"));
        QMenu *toolMenu = menuManager->menu(Core::Constants::M_TOOLS);
        toolMenu->addMenu(translationManagerMenu);

        
         // File menu
         //QAction *action = menuManager->action(Core::Constants::NEW);
        //_ui.toolBar->addAction(action);   
        openAct = menuManager->action(Core::Constants::OPEN);
        _ui.toolBar->addAction(openAct);
        connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
        
        saveAct = menuManager->action(Core::Constants::SAVE);
        _ui.toolBar->addAction(saveAct);
        connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
        //action = menuManager->action(Core::Constants::SAVE_AS);
        //_ui.toolBar->addAction(action);
     
}

void CMainWindow::open()
{
        QString file_name = QFileDialog::getOpenFileName(this);
         if (!file_name.isEmpty())
         {       
             STRING_MANAGER::TWorksheet wk_file;          
             if(loadExcelSheet(file_name.toStdString(), wk_file, true) == true)
             {
                 QTableWidget *wk_table = new QTableWidget();  
                 wk_table->setToolTip(file_name);
                 wk_table->setWindowFilePath(file_name);
                 wk_table->setColumnCount(wk_file.ColCount);
                 wk_table->setRowCount(wk_file.size() - 1);
                 // read columns name
                 for(unsigned int i = 0; i < wk_file.ColCount; i++)
                 {
                     QTableWidgetItem *col = new QTableWidgetItem();
                     ucstring col_name = wk_file.getData(0, i);
                     col->setText(tr(col_name.toString().c_str()));

                     wk_table->setHorizontalHeaderItem(i, col);
                 }
                 // read rows
                 for(unsigned int i = 1; i < wk_file.size(); i++)
                 {
                     for(unsigned int j = 0; j < wk_file.ColCount; j++)
                     {
                        QTableWidgetItem *row = new QTableWidgetItem();
                        ucstring row_value = wk_file.getData(i, j);
                        row->setText(tr(row_value.toString().c_str()));
                     
                        wk_table->setItem(i - 1, j, row);          
                     }
                 } 
                QMdiSubWindow *sub_window = new QMdiSubWindow(_ui.mdiArea);
                sub_window->setWidget(wk_table);
                wk_table->resizeColumnsToContents();
                wk_table->resizeRowsToContents(); 
                wk_table->showMaximized();
                sub_window->activateWindow();
                //_ui.mdiArea->addSubWindow(sub_window); 
                // set editor signals
                connect(wk_table, SIGNAL(cellChanged(int,int) ), this, SLOT(sheetEditorChanged(int,int)));
             }
         }
               
}

void CMainWindow::sheetEditorChanged(int, int)
{
    saveAct->setEnabled(true);
}

void CMainWindow::save()
{
    QMdiSubWindow *current_window = _ui.mdiArea->currentSubWindow();

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