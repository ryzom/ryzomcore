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

// Qt includes
#include <QtGui/QErrorMessage>
#include <QtGui/QTableWidgetItem>
#include <QtCore/qfileinfo.h>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>

// Project includes
#include "editor_worksheet.h"
#include "extract_bot_names.h"
#include "translation_manager_constants.h"
#include <set>

using namespace std;

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
                connect(table_editor, SIGNAL(itemChanged(QTableWidgetItem*) ), this, SLOT(worksheetEditorChanged(QTableWidgetItem*)));				
				connect(table_editor, SIGNAL(itemDoubleClicked(QTableWidgetItem*) ), this, SLOT(worksheetEditorCellEntered(QTableWidgetItem*)));
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
                       if(wk_file.findRow(j + colIdx, colname, rowIdf)) // search for the row
                       {
                           if(wk_file.getData(i + 1, j + colIdx) != tvalue) // verify the current value
                           {
                                wk_file.setData(i + 1, j + colIdx,  tvalue); // change the value
                           }
                       } else {
                           wk_file.setData(i + 1, j + colIdx,  tvalue); // insert the value
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
	current_stack->push(new CUndoWorksheetNewCommand(table_editor, last_row));
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
     current_stack->push(new CUndoWorksheetDeleteCommand(table_editor, selected_row));
 }

  table_editor->clearFocus();
  table_editor->clearSelection();  
  return;
}

void CEditorWorksheet::worksheetEditorCellEntered(QTableWidgetItem * item)
{
	temp_content = item->text();
	current_stack->push(new CUndoWorksheetCommand(table_editor, item, temp_content));
}

void CEditorWorksheet::worksheetEditorChanged(QTableWidgetItem * item)
{
	if(temp_content != item->text())
	{
		//current_stack->push(new CUndoWorksheetCommand(table_editor, item, temp_content));
	}

    if(!isWindowModified())
        setWindowModified(true);
}


void CEditorWorksheet::extractBotNames(list<string> filters, string level_design_path, NLLIGO::CLigoConfig ligoConfig)
{
    bool modified = false;
	QList<CTableWidgetItemStore> new_items;

    ExtractBotNames ebn;
    ebn.setRequiredSettings(filters, level_design_path);
    ebn.extractBotNamesFromPrimitives(ligoConfig);
                        // get SimpleNames
                    {                       
                        map<string, TEntryInfo> SimpleNames =  ebn.getSimpleNames();
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
								CTableWidgetItemStore bot_name_row_s(bot_name_row, currentRow, 0);
								new_items.push_back(bot_name_row_s);
								CTableWidgetItemStore translation_name_row_s(translation_name_row, currentRow, 1);
								new_items.push_back(translation_name_row_s);
								CTableWidgetItemStore sheet_name_row_s(sheet_name_row, currentRow, 2);
								new_items.push_back(sheet_name_row_s);								
                            }
                        }  
                        ebn.cleanSimpleNames();
                    }
                    // get GenericNames
                    {
                        set<string> GenericNames = ebn.getGenericNames();                       
                        set<string>::iterator it(GenericNames.begin()), last(GenericNames.end());
                        for (; it != last; ++it)
                        {
                            string gnName = "gn_" + ebn.cleanupName(*it);
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
								CTableWidgetItemStore bot_name_row_s(bot_name_row, currentRow, 0);
								new_items.push_back(bot_name_row_s);
								CTableWidgetItemStore translation_name_row_s(translation_name_row, currentRow, 1);
								new_items.push_back(translation_name_row_s);
								CTableWidgetItemStore sheet_name_row_s(sheet_name_row, currentRow, 2);
								new_items.push_back(sheet_name_row_s);	
                            }                         
                        }
                        ebn.cleanGenericNames();
                    } 
					
					current_stack->push(new CUndoWorksheetExtraction(new_items, table_editor));
                    if(modified)
                    {
                        setWindowModified(true);
                    }
           
}

void CEditorWorksheet::extractWords(QString filename, QString columnId, IWordListBuilder& wordListBuilder)
{
	uint	i;

	// **** Load the excel sheet
	// load
	TWorksheet		workSheet;
	if(!loadExcelSheet(filename.toStdString(), workSheet, true))
	{
		nlwarning("Error reading '%s'. Aborted", filename.toStdString().c_str());
		return;
	}
	// get the key column index
	uint	keyColIndex = 0;	
	if(!workSheet.findCol(columnId.toStdString(), keyColIndex))
	{
		nlwarning("Error: Don't find the column '%s'. '%s' Aborted", columnId.toStdString().c_str(), filename.toStdString().c_str());
		return;
	}
	// get the name column index
	uint	nameColIndex;
	if(!workSheet.findCol(ucstring("name"), nameColIndex))
	{
		nlwarning("Error: Don't find the column 'name'. '%s' Aborted", filename.toStdString().c_str());
		return;
	}    
    
	// **** List all words with the builder given
	std::vector<string>		allWords;
	if(!wordListBuilder.buildWordList(allWords, filename.toStdString()))
               {
		return;
                }
        bool modified = false;
		QList<CTableWidgetItemStore> new_items;
        for(i = 0; i < allWords.size(); i++)
        {                   
                string keyName = allWords[i];
                QList<QTableWidgetItem*> search_results = table_editor->findItems(tr(keyName.c_str()), Qt::MatchExactly);
                if(search_results.size() == 0)
                {
                   
                        int knPos = 0, nPos = 0;
                        if(workSheet.getData(0, 0) == ucstring("*HASH_VALUE"))
                        {
                            knPos = keyColIndex - 1;
                            nPos = nameColIndex - 1;
                        } else {
                            knPos = keyColIndex;
                            nPos = nameColIndex;
                        }                       
                        const int currentRow = table_editor->rowCount();                     
                        table_editor->setRowCount(currentRow + 1);
                        // keyName row
                        QTableWidgetItem *key_name_row = new QTableWidgetItem();
                        key_name_row->setText(tr(keyName.c_str()));    
                        key_name_row->setBackgroundColor(QColor("#F75D59"));               
                        table_editor ->setItem(currentRow, knPos, key_name_row); 
                        // nameColumn key
                        QTableWidgetItem *name_row = new QTableWidgetItem();
                        name_row->setText(QString("<GEN>") + tr(keyName.c_str()));    
                        name_row->setBackgroundColor(QColor("#F75D59"));                    
                        table_editor ->setItem(currentRow, nPos, name_row);   
                        if(!modified) modified = true;
						CTableWidgetItemStore key_name_row_s(key_name_row, currentRow, knPos);
						new_items.push_back(key_name_row_s);
						CTableWidgetItemStore name_row_s(name_row, currentRow, nPos);
						new_items.push_back(name_row_s);

                }            
        }
		current_stack->push(new CUndoWorksheetExtraction(new_items, table_editor));
        if(modified)
       {
              setWindowModified(true);
              table_editor->scrollToBottom();
       }
}

bool CEditorWorksheet::compareWorksheetFile(QString filename)
{
    STRING_MANAGER::TWorksheet wk_file; 
    int colIndex = 0;
    if(loadExcelSheet(filename.toStdString(), wk_file, true) == true)
    {
                 if(wk_file.getData(0, 0) == ucstring("*HASH_VALUE"))
                 {
                      colIndex = 1;
                 }   
                 if(wk_file.ColCount - colIndex != table_editor->columnCount())
                 {
                        return false;
                 }
                 for(int i = 0; i < table_editor->columnCount(); i++)
                 {
                     QString item = table_editor->horizontalHeaderItem(i)->text();
                     ucstring itemC = wk_file.getData(0, i+ colIndex);
                     if(item.toStdString() != itemC.toString())
                     {
                         nlwarning(item.toStdString().c_str());
                         nlwarning(itemC.toString().c_str());
                         return false;
                     }
                 }               
    } else {
        return false;
    }
    
    return true;
}

void CEditorWorksheet::mergeWorksheetFile(QString filename)
{
             STRING_MANAGER::TWorksheet wk_file;          
             if(loadExcelSheet(filename.toStdString(), wk_file, true) == true)
             {
                 bool hasHashValue = false;
                 int colIndex = 0;
                 if(wk_file.getData(0, 0) == ucstring("*HASH_VALUE"))
                 {
                      hasHashValue = true;
                      colIndex = 1;
                 }           
                 // read rows
                 for(unsigned int i = 1; i < wk_file.size(); i++)
                 {
                     // search with the first column
                     ucstring rowId = wk_file.getData(i,colIndex);
                     QList<QTableWidgetItem*> search_results = table_editor->findItems(tr(rowId.toString().c_str()), Qt::MatchExactly);
                     if(search_results.size() == 0)
                     {
                         const int lastRow = table_editor->rowCount();
                         table_editor->setRowCount(lastRow + 1);
                         for(unsigned int j = 0; j < table_editor->columnCount(); j++)
                         {
                             ucstring rowValue = wk_file.getData(i, j + colIndex); // get the value
                             QTableWidgetItem *row = new QTableWidgetItem();
                             row->setText(QString(rowValue.toString().c_str())); // set the value in table item
                             table_editor->setItem(lastRow, j, row);
                         }
                     }
                   }
                 } else {
                        QErrorMessage error;
                        error.showMessage("This file is not a worksheet file.");
                        error.exec();                             
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
    if(isWindowModified())
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        switch (ret) 
        {
                case QMessageBox::Save:
                    save();
                    event->accept();
                    close();
                    break;
                case QMessageBox::Discard:
                    event->accept();
                    close();
                    break;
                case QMessageBox::Cancel:
                    event->ignore();
                    break;
                default:
                    break;
        }
    } else {
        event->accept();
        close();
    }
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

bool CEditorWorksheet::isSheetTable(QString type)
{
    QString column_name;
    if(type.toAscii() == Constants::WK_ITEM)
    {
        column_name = "item ID";    
    } else if(type.toAscii() == Constants::WK_CREATURE) {
        column_name = "creature ID";   
    } else if(type.toAscii() == Constants::WK_SBRICK) {
        column_name = "sbrick ID";       
    } else if(type.toAscii() == Constants::WK_SPHRASE) {
        column_name = "sphrase ID";   
    } else if(type.toAscii() == Constants::WK_PLACE) {
        column_name = "placeId";
    }
   bool status = true;
   if(table_editor->horizontalHeaderItem(0)->text() != column_name
           || table_editor->horizontalHeaderItem(1)->text() != "name")
   {
       status = false;
   }
                 
   return status;    
}

}


