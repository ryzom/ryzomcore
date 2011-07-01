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

#include "editor_worksheet.h"
#include <set>
// Qt includes
#include <QtGui/QErrorMessage>
#include <QtGui/QTableWidgetItem>
#include <QtCore/qfileinfo.h>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>

using namespace std;

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

namespace Plugin {



void CEditorWorksheet::open(QString filename)
{
             STRING_MANAGER::TWorksheet wk_file;          
             if(loadExcelSheet(filename.toStdString(), wk_file, true) == true)
             {
                 bool hasHashValue = false;
                 table_editor = new QTableWidget();  
                 if(wk_file.getData(0, 0) == ucstring("*HASH_VALUE"))
                 {
                      table_editor->setColumnCount(wk_file.ColCount - 1);
                      hasHashValue = true;
                 } else {
                      table_editor->setColumnCount(wk_file.ColCount);                   
                 }
                 table_editor->setRowCount(wk_file.size() - 1);
                 
                 // read columns name                
                 for(unsigned int i = 0; i < wk_file.ColCount; i++)
                 {
                     if(hasHashValue && i == 0)
                     {
                            // we don't show the column with hash value
                     } else {
                        QTableWidgetItem *col = new QTableWidgetItem();
                        ucstring col_name = wk_file.getData(0, i);
                        col->setText(tr(col_name.toString().c_str()));
                        if(hasHashValue)
                        {
                                table_editor->setHorizontalHeaderItem(i - 1, col);                        
                        } else {
                                table_editor->setHorizontalHeaderItem(i, col);
                        }
                     }
                 }
                 
                 // read rows
                 for(unsigned int i = 1; i < wk_file.size(); i++)
                 {
                     for(unsigned int j = 0; j < wk_file.ColCount; j++)
                     {
                        if(hasHashValue && j == 0)
                        {
                            // we don't show the column with hash value
                        } else {
                            QTableWidgetItem *row = new QTableWidgetItem();
                            ucstring row_value = wk_file.getData(i, j);
                            row->setText(tr(row_value.toString().c_str()));
                            if(hasHashValue)
                            {
                                table_editor->setItem(i - 1, j - 1, row);    
                            } else {
                                table_editor->setItem(i - 1, j, row); 
                            }
                        }
                    }
                }
                setCurrentFile(filename);
                setAttribute(Qt::WA_DeleteOnClose);
                setWidget(table_editor);
                table_editor->resizeColumnsToContents();
                table_editor->resizeRowsToContents(); 
                // set editor signals
                connect(table_editor, SIGNAL(cellChanged(int,int) ), this, SLOT(worksheetEditorChanged(int,int)));
             } else {
                QErrorMessage error;
                error.showMessage("This file is not a worksheet file.");
                error.exec();                             
             }
    
}

void CEditorWorksheet::activateWindow()
{
                showMaximized();
                   
}

void CEditorWorksheet::save()
{
               STRING_MANAGER::TWorksheet wk_file;   
               loadExcelSheet(current_file.toStdString(), wk_file, true);   
               uint rowIdx;
               uint colIdx = 0;
               bool hasHashValue = false;
               if(wk_file.getData(0, 0) == ucstring("*HASH_VALUE"))
               {
                   hasHashValue = true;
                   colIdx = 1;
               }
               for(int i = 0; i < table_editor->rowCount(); i++)
               {
                   // maybe extra rows ?
                   if((unsigned)table_editor->rowCount() > (wk_file.size() - 1))
                   {
                        rowIdx = wk_file.size();
                        wk_file.resize(rowIdx + table_editor->rowCount() - wk_file.size() + 1);
                   }
                   for(int j = 0; j < table_editor->columnCount(); j++)
                   {                       
                       ucstring tvalue;
                       ucstring colname;
                       uint rowIdf;
                       QString tvalueQt = table_editor->item(i, j)->text();
                       tvalue = ucstring(tvalueQt.toStdString());
                       colname = wk_file.getData(0, j + colIdx);
                       
                       rowIdf = uint(i + 1);
                       if(wk_file.findRow(j + colIdx, colname, rowIdf))
                       {
                           if(wk_file.getData(i + 1, j + colIdx) != tvalue)
                           {
                                wk_file.setData(i + 1, j + colIdx,  tvalue);
                           }
                       } else {
                           wk_file.setData(i + 1, j + colIdx,  tvalue);
                       }
                   }
               }
               if(hasHashValue)
               {
                        // rewrite the hash codes
                        makeHashCode(wk_file, true);
               }
               // write to file
               ucstring s = prepareExcelSheet(wk_file);            
               NLMISC::CI18N::writeTextFile(current_file.toStdString(), s, false);
               setCurrentFile(current_file);
}

void CEditorWorksheet::saveAs(QString filename)
{
                STRING_MANAGER::TWorksheet new_file, wk_file;  
                loadExcelSheet(current_file.toStdString(), wk_file, true);   
                // set columns
                new_file.resize(new_file.size() + 1);
                for(unsigned int i = 0; i < wk_file.ColCount; i++)
                {
                    ucstring col_name = wk_file.getData(0, i);
                    new_file.insertColumn(new_file.ColCount);
                    new_file.setData(0, new_file.ColCount - 1, col_name);
                } 
                // read all the rows from table
                uint rowIdx; 
                uint colIdx = 0;
                bool hasHashValue = false;
                if(wk_file.getData(0, 0) == ucstring("*HASH_VALUE"))
                {
                   hasHashValue = true;
                   colIdx = 1;
                }
                for(int i = 0; i < table_editor->rowCount(); i++)
                {
                     rowIdx = new_file.size();
                     new_file.resize(new_file.size() + 1);
                     for(int j = 0; j < table_editor->columnCount(); j++)
                     {
                         QTableWidgetItem* item = table_editor->item(i, j);
                         new_file.setData(rowIdx, j + colIdx, ucstring(item->text().toStdString()));
                     }   
                 } 
                if(hasHashValue)
                {
                        // rewrite the hash codes
                        makeHashCode(wk_file, true);
                }
                ucstring s = prepareExcelSheet(new_file);            
                NLMISC::CI18N::writeTextFile(filename.toStdString(), s, false); 
                setCurrentFile(filename);
}

void CEditorWorksheet::insertRow()
{
    int last_row = table_editor->rowCount();
    table_editor->setRowCount(last_row + 1);
    for(int j = 0; j < table_editor->columnCount(); j++)
    {
        QTableWidgetItem* item = new QTableWidgetItem();
        //item->setText(QString(" "));
        table_editor->setItem(last_row, j, item);
    }
}

void CEditorWorksheet::deleteRow()
{
 int selected_row = table_editor->currentRow();
 QMessageBox msgBox;
 msgBox.setText("The row will be deleted.");
 msgBox.setInformativeText("Do you want to delete the selected row ?");
 msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
 msgBox.setDefaultButton(QMessageBox::No);
 int ret = msgBox.exec(); 
 
 if(ret == QMessageBox::Yes)
 {
     table_editor->removeRow(selected_row);
 }

  table_editor->clearFocus();
  table_editor->clearSelection();  
  return;
}

void CEditorWorksheet::worksheetEditorChanged(int row, int column)
{
    
}

void CEditorWorksheet::extractBotNames()
{
    bool modified = false;
// get SimpleNames
                    {                       
                        map<string, TEntryInfo> SimpleNames =  getSimpleNames();
                        map<string, TEntryInfo>::iterator it(SimpleNames.begin()), last(SimpleNames.end());

                        for (; it != last; ++it)
                        {  
                            QList<QTableWidgetItem*> search_results = table_editor->findItems(tr(it->first.c_str()), Qt::MatchExactly);
                            if(search_results.size() == 0)
                            {
                                const int currentRow = table_editor->rowCount();                     
                                table_editor->setRowCount(currentRow + 1);
                                QTableWidgetItem *bot_name_row = new QTableWidgetItem();
                                bot_name_row->setText(tr(it->first.c_str()));    
                                bot_name_row->setBackgroundColor(QColor("#F75D59"));
                                table_editor ->setItem(currentRow, 0, bot_name_row);  
                                QTableWidgetItem *translation_name_row = new QTableWidgetItem();
                                translation_name_row->setBackgroundColor(QColor("#F75D59"));
                                translation_name_row->setText(tr(it->first.c_str()));
                                table_editor ->setItem(currentRow , 1, translation_name_row);               
                                QTableWidgetItem *sheet_name_row = new QTableWidgetItem();
                                sheet_name_row->setText(tr(it->second.SheetName.c_str()));      
                                sheet_name_row->setBackgroundColor(QColor("#F75D59"));
                                table_editor ->setItem(currentRow, 2, sheet_name_row); 
                                if(!modified) modified = true;
                            }
                        }  
                        cleanSimpleNames();
                    }
                    // get GenericNames
                    {
                        set<string> GenericNames = getGenericNames();                       
                        set<string>::iterator it(GenericNames.begin()), last(GenericNames.end());
                        for (; it != last; ++it)
                        {
                            string gnName = "gn_" + cleanupName(*it);
                            QList<QTableWidgetItem*> search_results = table_editor->findItems(tr((*it).c_str()), Qt::MatchExactly);
                            if(search_results.size() == 0)
                            {
                                const int currentRow = table_editor->rowCount();                     
                                table_editor->setRowCount(currentRow + 1);
                                QTableWidgetItem *bot_name_row = new QTableWidgetItem();
                                bot_name_row->setText(tr((*it).c_str()));    
                                bot_name_row->setBackgroundColor(QColor("#F75D59"));
                                table_editor ->setItem(currentRow, 0, bot_name_row);  
                                QTableWidgetItem *translation_name_row = new QTableWidgetItem();
                                translation_name_row->setBackgroundColor(QColor("#F75D59"));
                                translation_name_row->setText(tr(gnName.c_str()));
                                table_editor ->setItem(currentRow , 1, translation_name_row);               
                                QTableWidgetItem *sheet_name_row = new QTableWidgetItem();
                                sheet_name_row->setText(" ");      
                                sheet_name_row->setBackgroundColor(QColor("#F75D59"));
                                table_editor ->setItem(currentRow, 2, sheet_name_row);   
                                if(!modified) modified = true;
                            }                         
                        }
                        cleanGenericNames();
                    }  
                    if(modified)
                    {
                        setWindowModified(true);
                    }
           
}

void CEditorWorksheet::setCurrentFile(QString filename)
{
     QFileInfo *file = new QFileInfo(filename);
     current_file = file->canonicalFilePath();
     setWindowModified(false);
     setWindowTitle(file->fileName() + "[*]");  
     setWindowFilePath(current_file);
}

void CEditorWorksheet::closeEvent(QCloseEvent *event)
{
    close();
    event->accept();
    
}

bool CEditorWorksheet::isBotNamesTable()
{
    bool status = true;
   if(table_editor->horizontalHeaderItem(0)->text() != "bot name" 
           || table_editor->horizontalHeaderItem(1)->text() != "translated name" 
           || table_editor->horizontalHeaderItem(2)->text() != "sheet_name")
   {
       status = false;
   }
                 
    return status;
}

}


