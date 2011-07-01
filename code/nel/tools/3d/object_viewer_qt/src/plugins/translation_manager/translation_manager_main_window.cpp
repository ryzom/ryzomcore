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
#include "editor_worksheet.h"

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
#include <QtCore/QResource>
#include <QtGui/QMenuBar>
#include <QtCore/QFileInfo>
#include <QtCore/QEvent>
#include <QtGui/QCloseEvent>



struct TEntryInfo
{
	string	SheetName;
};

set<string> getGenericNames();
void cleanGenericNames();
map<string, TEntryInfo> getSimpleNames();
void cleanSimpleNames();
void setPathsForPrimitives(map<string,list<string> > config_paths, string ligo_class_file);
void extractBotNamesFromPrimitives();
string cleanupName(const std::string &name);
ucstring cleanupUcName(const ucstring &name);

namespace Plugin
{

CMainWindow::CMainWindow(QWidget *parent)
	: QMainWindow(parent)
{
         _ui.setupUi(this);
         
         _ui.mdiArea->closeAllSubWindows();
         connect(_ui.mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),this, SLOT(activeSubWindowChanged()));         
         windowMapper = new QSignalMapper(this);
         connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));
         
         // set extraction scripts counters
         execution_count["extract_bot_names"] = 0;
         
         readSettings();
         createToolbar();
        m_undoStack = new QUndoStack(this);
}

void CMainWindow::createToolbar()
{	
         // File menu        
        openAct = new QAction(QIcon(Core::Constants::ICON_OPEN), "&Open...", this);
        _ui.toolBar->addAction(openAct);
        connect(openAct, SIGNAL(triggered()), this, SLOT(open()));       
        saveAct = new QAction(QIcon(Core::Constants::ICON_SAVE), "&Save...", this);
        _ui.toolBar->addAction(saveAct);
        connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));       
        saveAsAct = new QAction(QIcon(Core::Constants::ICON_SAVE_AS), "&Save as...", this);
        _ui.toolBar->addAction(saveAsAct);
        connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

        // Tools menu
        QMenu *wordsExtractionMenu = new QMenu("&Words extraction...");
        wordsExtractionMenu->setIcon(QIcon(Core::Constants::ICON_SETTINGS));
        _ui.toolBar->addAction(wordsExtractionMenu->menuAction());
        QAction *extractBotNamesAct = wordsExtractionMenu->addAction("&Extract bot names...");
        extractBotNamesAct->setStatusTip(tr("Extract bot names from primitives."));
        connect(extractBotNamesAct, SIGNAL(triggered()), this, SLOT(extractBotNames()));
        
        // Windows menu
        windowMenu = new QMenu(tr("&Windows..."), _ui.toolBar);
        windowMenu->setIcon(QIcon(Core::Constants::ICON_PILL));     
        updateWindowsList();
        _ui.toolBar->addAction(windowMenu->menuAction());
        connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowsList()));
}

void CMainWindow::updateToolbar(QMdiSubWindow *window)
{
    if(_ui.mdiArea->subWindowList().size() > 0)
    if(QString(window->widget()->metaObject()->className()) == "QTableWidget") // Sheet Editor
    {
        QAction *insertRowAct = windowMenu->addAction("Insert new row");
        connect(insertRowAct, SIGNAL(triggered()), window, SLOT(insertRow())); 
        QAction *deleteRowAct = windowMenu->addAction("Delete row");
        connect(deleteRowAct, SIGNAL(triggered()), window, SLOT(deleteRow())); 
        
    }
}

void CMainWindow::setActiveSubWindow(QWidget* window)
{
        if (!window)
        {
                return;
        }
        QMdiSubWindow *cwindow = qobject_cast<QMdiSubWindow *>(window);
        _ui.mdiArea->setActiveSubWindow(cwindow);   
}

void CMainWindow::activeSubWindowChanged()
{
   
}

void CMainWindow::updateWindowsList()
{
        windowMenu->clear();
        QMdiSubWindow *current_window = _ui.mdiArea->activeSubWindow();     
        QList<QMdiSubWindow*> subWindows = _ui.mdiArea->subWindowList();      
        
        updateToolbar(current_window);
        
        for(int i = 0; i < subWindows.size(); ++i) 
        {
                QString window_file = QFileInfo(subWindows.at(i)->windowFilePath()).fileName();
                QString action_text;
                if (i < 9) {
                        action_text = tr("&%1 %2").arg(i + 1).arg(window_file);
                } else {
                        action_text = tr("%1 %2").arg(i + 1).arg(window_file);
                }
                QAction *action  = windowMenu->addAction(action_text);
                action->setCheckable(true);
                action->setChecked(subWindows.at(i) == current_window);
                connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));             
                windowMapper->setMapping(action, subWindows.at(i));
        }    
}

void CMainWindow::open()
{
        QString file_name = QFileDialog::getOpenFileName(this);
        if(!file_name.isEmpty())
        {
            list<CEditor*> subWindows = convertSubWindowList(_ui.mdiArea->subWindowList());
            list<CEditor*>::iterator it = subWindows.begin();
            CEditor* current_window = qobject_cast<CEditor*>(_ui.mdiArea->currentSubWindow());
            for(; it != subWindows.end(); ++it)
            {
                QString sw_file = (*it)->subWindowFilePath();
                if(file_name == sw_file)
                {
                    if((*it) != current_window)
                    {
                        (*it)->activateWindow();
                    }
                    return;
                }
            }          
             if(isWorksheetEditor(file_name))
             {
                 CEditorWorksheet *new_window = new CEditorWorksheet(_ui.mdiArea);                 
                 new_window->open(file_name);  
                 new_window->activateWindow();
             }
         }
               
}

void CMainWindow::save()
{
    CEditor* current_window = qobject_cast<CEditor*>(_ui.mdiArea->currentSubWindow());
    
    if(QString(current_window->widget()->metaObject()->className()) == "QTableWidget") // Sheet Editor
    {
        current_window->save();
    }
}

void CMainWindow::saveAs()
{
    QString file_name;
    if (_ui.mdiArea->isActiveWindow())
    {
        file_name = QFileDialog::getSaveFileName(this);
    }
    
    if (!file_name.isEmpty())
    {    
        CEditor* current_window = qobject_cast<CEditor*>(_ui.mdiArea->currentSubWindow());   
        if(QString(current_window->widget()->metaObject()->className()) == "QTableWidget") // Sheet Editor
        {
            current_window->saveAs(file_name);
        }             
             
    }    
}

void CMainWindow::extractBotNames()
{
        if(verifySettings() == true) 
        {
                CEditor* editor_window = qobject_cast<CEditor*>(_ui.mdiArea->currentSubWindow());
                if(QString(editor_window->widget()->metaObject()->className()) == "QTableWidget") // Sheet Editor
                {
                    CEditorWorksheet* current_window = qobject_cast<CEditorWorksheet*>(editor_window);
                    QString file_path = current_window->subWindowFilePath();
                    if(!current_window->isBotNamesTable())
                    {
                        list<CEditor*> subWindows =  convertSubWindowList(_ui.mdiArea->subWindowList());
                        list<CEditor*>::iterator it = subWindows.begin();
                        bool finded = false;
                        for(; it != subWindows.end(), finded != true; ++it)
                        {                           
                            current_window = qobject_cast<CEditorWorksheet*>((*it));
                            file_path = current_window->subWindowFilePath();
                            if(current_window->isBotNamesTable())
                            {
                                finded = true;
                                current_window->activateWindow();
                            }
                        }
                        if(!finded)
                        {
                            open();
                            current_window = qobject_cast<CEditorWorksheet*>(_ui.mdiArea->currentSubWindow());
                            file_path = current_window->windowFilePath();
                        }
                    }
                    if(execution_count["extract_bot_names"] == 0)
                        setPathsForPrimitives(config_paths, ligo_path);
                    extractBotNamesFromPrimitives();
                    execution_count["extract_bot_names"] = execution_count["extract_bot_names"]  + 1;
                    
                    current_window->extractBotNames();
                 //   if(current_window->isWindowModified())
                  //  {
                        
                 //   }    
                    
                }
        }    
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

void CMainWindow::debug(QString text)
{
            QErrorMessage error_settings;
            error_settings.showMessage(text);
            error_settings.exec();    
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

list<CEditor*> CMainWindow::convertSubWindowList(QList<QMdiSubWindow*> listq)
{
        list<CEditor*> subwindows;
        QList<QMdiSubWindow*>::iterator it = listq.begin();
        
        for(; it != listq.end(); ++it)
        {
                CEditor* current_window = qobject_cast<CEditor*>((*it));
                subwindows.push_back(current_window);
        }
        
        return subwindows;
}

bool CMainWindow::isWorksheetEditor(QString filename)
{
             STRING_MANAGER::TWorksheet wk_file;          
             if(loadExcelSheet(filename.toStdString(), wk_file, true) == true)
             {
                 return true;
             }  else {
                 return false;
             }
}

bool CCoreListener::closeMainWindow() const
{
    return true;
}

} /* namespace Plugin */


