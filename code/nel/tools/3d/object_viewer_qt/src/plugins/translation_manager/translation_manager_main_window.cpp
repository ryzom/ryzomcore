// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

// Project system includes
#include "../core/icore.h"
#include "../core/core_constants.h"
#include "../core/imenu_manager.h"
#include "../../extension_system/iplugin_spec.h"

// Qt includes
#include <QtGui/QWidget>
#include <QtGui/QMessageBox>
#include <QtCore/QSettings>
#include <QtGui/QErrorMessage>
#include <QtCore/QSignalMapper>
#include <QtGui/QTableWidget>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QFileDialog>
#include <QtCore/QResource>
#include <QtGui/QMenuBar>
#include <QtCore/QFileInfo>
#include <QtCore/QEvent>
#include <QtGui/QCloseEvent>

// Plugin includes
#include "translation_manager_main_window.h"
#include "translation_manager_constants.h"
#include "ftp_selection.h"


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
         
         initialize_settings["georges"] = false;
         initialize_settings["ligo"] = false;

		 connect(Core::ICore::instance(), SIGNAL(changeSettings()), this, SLOT(readSettings()));
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
        // extract bot names
        QAction *extractBotNamesAct = wordsExtractionMenu->addAction("&Extract bot names...");
        extractBotNamesAct->setStatusTip(tr("Extract bot names from primitives."));
        connect(extractBotNamesAct, SIGNAL(triggered()), this, SLOT(extractBotNames()));       
        // Words extraction
        // -----------------------------
        // signal mapper for extraction words
        QSignalMapper *wordsExtractionMapper = new QSignalMapper(this);
        connect(wordsExtractionMapper, SIGNAL(mapped(QString)), this, SLOT(extractWords(QString)));
        // extract item words
        QAction *extractItemWordsAct = wordsExtractionMenu->addAction("&Extract item words...");
        extractItemWordsAct->setStatusTip(tr("Extract item words"));
        connect(extractItemWordsAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));             
        wordsExtractionMapper->setMapping(extractItemWordsAct, tr(Constants::WK_ITEM));              
        // extract creature words
        QAction *extractCreatureWordsAct = wordsExtractionMenu->addAction("&Extract creature words...");
        extractCreatureWordsAct->setStatusTip(tr("Extract creature words"));
        connect(extractCreatureWordsAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));             
        wordsExtractionMapper->setMapping(extractCreatureWordsAct, tr(Constants::WK_CREATURE));      
        // extract sbrick words
        QAction *extractSbrickWordsAct = wordsExtractionMenu->addAction("&Extract sbrick words...");
        extractSbrickWordsAct->setStatusTip(tr("Extract sbrick words"));  
        connect(extractSbrickWordsAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));             
        wordsExtractionMapper->setMapping(extractSbrickWordsAct, tr(Constants::WK_SBRICK));   
        // extract sphrase words
        QAction *extractSphraseWordsAct = wordsExtractionMenu->addAction("&Extract sphrase words...");
        extractSphraseWordsAct->setStatusTip(tr("Extract sphrase words"));
        connect(extractSphraseWordsAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));             
        wordsExtractionMapper->setMapping(extractSphraseWordsAct, tr(Constants::WK_SPHRASE));   
        // extract place and region names
        QAction *extractPlaceNamesAct = wordsExtractionMenu->addAction("&Extract place names...");
        extractPlaceNamesAct->setStatusTip(tr("Extract place names from primitives"));
        connect(extractPlaceNamesAct, SIGNAL(triggered()), wordsExtractionMapper, SLOT(map()));             
        wordsExtractionMapper->setMapping(extractPlaceNamesAct, tr(Constants::WK_PLACE));   
        // Merge options
        // -----------------------------
        QAction *mergeSingleFileAct = wordsExtractionMenu->addAction("&Merge worksheet file...");
        mergeSingleFileAct->setStatusTip(tr("Merge worksheet file from local or remote directory"));
        connect(mergeSingleFileAct, SIGNAL(triggered()), this, SLOT(mergeSingleFile()));
        // Windows menu
        windowMenu = new QMenu(tr("&Windows..."), _ui.toolBar);
        windowMenu->setIcon(QIcon(Core::Constants::ICON_PILL));     
        updateWindowsList();
        _ui.toolBar->addAction(windowMenu->menuAction());
        connect(windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowsList()));

		// Undo, Redo actions
		// -----------------------------
		Core::ICore *core = Core::ICore::instance();
		Core::IMenuManager *menuManager = core->menuManager();
		QAction* undoAction = menuManager->action(Core::Constants::UNDO);
        if (undoAction != 0)
                _ui.toolBar->addAction(undoAction);
 
        QAction* redoAction = menuManager->action(Core::Constants::REDO);
        if (redoAction != 0)
                _ui.toolBar->addAction(redoAction);
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
	//TODO: nothing to be done here atm
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
			CEditor *editor = getEditorByWindowFilePath(file_name);
			if(editor != NULL)
			{
				editor->activateWindow(); 
				return;
			}
            if(isWorksheetEditor(file_name))
            {
                 CEditorWorksheet *new_window = new CEditorWorksheet(_ui.mdiArea); 
				 new_window->setUndoStack(m_undoStack);
                 new_window->open(file_name);  
                 new_window->activateWindow();
            }
         }
               
}

void CMainWindow::openWorkFile(QString file)
{
    QFileInfo* file_path = new QFileInfo(QString("%1/%2").arg(work_path).arg(file));
    if(file_path->exists())
    {
             if(isWorksheetEditor(file_path->filePath()))
             {
                 CEditorWorksheet *new_window = new CEditorWorksheet(_ui.mdiArea);                 
                 new_window->open(file_path->filePath());  
                 new_window->activateWindow();
             }          
    } else {
            QErrorMessage error;
            error.showMessage(QString("The %1 file don't exists.").arg(file_path->fileName()));
            error.exec();        
    }
  
}

void CMainWindow::save()
{
    if(_ui.mdiArea->subWindowList().size() > 0)
    {
        CEditor* current_window = qobject_cast<CEditor*>(_ui.mdiArea->currentSubWindow());
    
        if(QString(current_window->widget()->metaObject()->className()) == "QTableWidget") // Sheet Editor
        {
                current_window->save();
        }
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

void CMainWindow::initializeSettings(bool georges = false)
{   
    if(georges == true && initialize_settings["georges"] == false)
    {
		CPath::addSearchPath(level_design_path.toStdString() + "/DFN", true, false);
        CPath::addSearchPath(level_design_path.toStdString() + "/Game_elem/Creature", true, false);
        initialize_settings["georges"] = true;
    }

    if(initialize_settings["ligo"] == false)
    {
        //-------------------------------------------------------------------
        // init ligo config      
        string ligoPath = CPath::lookup("world_editor_classes.xml", true, true);
        ligoConfig.readPrimitiveClass(ligoPath.c_str(), false);
        NLLIGO::Register();
        NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig = &ligoConfig;
        initialize_settings["ligo"] = true;
    }
}

void CMainWindow::extractWords(QString typeq)
{
        if(verifySettings() == true) 
        {
			CEditorWorksheet* editor_window = getEditorByWorksheetType(typeq);
			if(editor_window != NULL)
			{
                editor_window->activateWindow();
				QString file_path = editor_window->windowFilePath(); 
			} else {
				openWorkFile(typeq);
				editor_window = getEditorByWorksheetType(typeq);
				if(editor_window != NULL)
				{
					editor_window->activateWindow();
					QString file_path = editor_window->windowFilePath(); 
				} else return;
			}

            QString column_name;
            // Sheet extraction
            CSheetWordListBuilder	builderS;
            // Primitives extraction
            CRegionPrimWordListBuilder builderP;
            bool isSheet = false;
            if(typeq.toAscii() == Constants::WK_ITEM) {
                column_name = "item ID";
                builderS.SheetExt = "sitem";
				builderS.SheetPath = level_design_path.append("/game_element/sitem").toStdString();  
                isSheet = true;
            } else if(typeq.toAscii() == Constants::WK_CREATURE) {
                column_name = "creature ID";
                builderS.SheetExt = "creature";
				builderS.SheetPath = level_design_path.append("/Game_elem/Creature/fauna").toStdString();    
                isSheet = true;
            } else if(typeq.toAscii() == Constants::WK_SBRICK) {
                column_name = "sbrick ID";
                builderS.SheetExt = "sbrick";
                builderS.SheetPath = level_design_path.append("/game_element/sbrick").toStdString();     
                isSheet = true;
            } else if(typeq.toAscii() == Constants::WK_SPHRASE) {
                column_name = "sphrase ID";
                builderS.SheetExt = "sphrase";
                builderS.SheetPath = level_design_path.append("/game_element/sphrase").toStdString();  
                isSheet = true;
            } else if(typeq.toAscii() == Constants::WK_PLACE) {
                column_name = "placeId";
				builderP.PrimPath = primitives_path.toStdString();
                builderP.PrimFilter.push_back("region_*.primitive");
                builderP.PrimFilter.push_back("indoors_*.primitive");
                isSheet = false;
            }
     
            if(isSheet)
            {
                editor_window->extractWords(editor_window->windowFilePath(), column_name, builderS);
            } else {
                initializeSettings(false);
                editor_window->extractWords(editor_window->windowFilePath(), column_name, builderP);
            }       
       }     
   
}

void CMainWindow::extractBotNames()
{
        if(verifySettings() == true) 
        {
			CEditorWorksheet* editor_window = getEditorByWorksheetType(NULL);
			if(editor_window != NULL)
			{
                editor_window->activateWindow();
				QString file_path = editor_window->windowFilePath(); 
			} else {
				openWorkFile(Constants::WK_BOTNAMES);
				editor_window = getEditorByWorksheetType(NULL);
				if(editor_window != NULL)
				{
					editor_window->activateWindow();
					QString file_path = editor_window->windowFilePath(); 
				} else return;
			}
            initializeSettings(true);
			editor_window->extractBotNames(convertQStringList(filters), level_design_path.toStdString(), ligoConfig);  
        }    
}

void CMainWindow::mergeSingleFile()
{
    CEditor* editor_window = qobject_cast<CEditor*>(_ui.mdiArea->currentSubWindow());
    CSourceDialog *dialog = new CSourceDialog(this);
    map<QListWidgetItem*, int> methods;
    // create items
    QListWidgetItem* local_item = new QListWidgetItem();
    local_item->setText("Local directory");
    methods[local_item] = 0;
    QListWidgetItem* ftp_item = new QListWidgetItem();
    ftp_item->setText("From a FTP server");
    methods[ftp_item] = 1;
    
    dialog->setSourceOptions(methods);
    dialog->show();
    dialog->exec();
    if(dialog->selected_item == local_item) // Local directory
    {
        QString file_name;
        if (_ui.mdiArea->subWindowList().size() > 0)
        {
            file_name = QFileDialog::getOpenFileName(this);
        } else {
            return;
        }    
        
        if(QString(editor_window->widget()->metaObject()->className()) == "QTableWidget") // Sheet Editor
        {
            editor_window->activateWindow();
            CEditorWorksheet* current_window = qobject_cast<CEditorWorksheet*>(editor_window);
            if(current_window->windowFilePath() == file_name)
                return;
            if(current_window->compareWorksheetFile(file_name))
            {
                current_window->mergeWorksheetFile(file_name);
            } else  {
                QErrorMessage error;
                error.showMessage(QString("The file: %1 has different columns from the current file in editor.").arg(file_name));
                error.exec();                
            }
        }
    } else if(dialog->selected_item == ftp_item)   { // Ftp directory
        CFtpSelection* ftp_dialog = new CFtpSelection(this);
        ftp_dialog->show();
        ftp_dialog->exec();
    } else {
        return;
    }
        
       
}

void CMainWindow::readSettings()
{
            QSettings *settings = Core::ICore::instance()->settings();
			// translation manager settings
            settings->beginGroup("translationmanager");
			filters = settings->value("filters").toStringList();        
            languages = settings->value("trlanguages").toStringList();  
            translation_path = settings->value("translation").toString();
            work_path = settings->value("work").toString();
            settings->endGroup(); 
			// core settings
            settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
            level_design_path = settings->value(Core::Constants::LEVELDESIGN_PATH).toString();
            primitives_path = QString(Core::Constants::PRIMITIVES_PATH); //TODO
            settings->endGroup();              
}

bool CMainWindow::verifySettings()
{
        bool count_errors = false;
        
		if(level_design_path.isNull() || primitives_path.isNull() || work_path.isNull())
        {
            QErrorMessage error_settings;
            error_settings.showMessage("Please write all the paths on the settings dialog.");
            error_settings.exec();
            count_errors = true;
        }
        
        return !count_errors;
        
}

CEditor *CMainWindow::getEditorByWindowFilePath(const QString &fileName)
{
    Q_FOREACH(QMdiSubWindow *subWindow, _ui.mdiArea->subWindowList())
    {
        CEditor *currentEditor = qobject_cast<CEditor *>(subWindow);
        if(currentEditor->subWindowFilePath() == fileName)
            return currentEditor;
    }
    return NULL;
}

CEditorWorksheet *CMainWindow::getEditorByWorksheetType(const QString &type)
{
    Q_FOREACH(QMdiSubWindow *subWindow, _ui.mdiArea->subWindowList())
    {
        CEditor *currentEditor = qobject_cast<CEditor*>(subWindow);
		if(QString(currentEditor->widget()->metaObject()->className()) == "QTableWidget")
		{
			CEditorWorksheet *editor = qobject_cast<CEditorWorksheet *>(currentEditor);
			if(type != NULL) {
				if(editor->isSheetTable(type))
				{
					return editor;
				}
			} else {
				if(editor->isBotNamesTable())
				{
					return editor;
				}
			}
		}
    }
    return NULL;
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


