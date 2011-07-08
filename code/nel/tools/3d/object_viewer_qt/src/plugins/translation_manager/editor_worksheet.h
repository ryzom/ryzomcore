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

#ifndef EDITOR_WORKSHEET_H
#define EDITOR_WORKSHEET_H

// Nel includes
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"
#include "nel/ligo/ligo_config.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QWidget>
#include <QtGui/QMdiArea>
#include <QtGui/QTableWidget>
#include <QtGui/QMdiSubWindow>

#include "translation_manager_editor.h"
#include "extract_new_sheet_names.h"

namespace Plugin {

class CEditorWorksheet : public CEditor
{
    Q_OBJECT
private:
    QTableWidget* table_editor;
public:
    CEditorWorksheet(QMdiArea* parent) : CEditor(parent) {}
    CEditorWorksheet() : CEditor() {}
    void open(QString filename);
    void save();
    void saveAs(QString filename);
    void activateWindow();
    void mergeWorksheetFile(QString filename);
    bool compareWorksheetFile(QString filename);
    void extractBotNames(list<string> filters, string level_design_path, NLLIGO::CLigoConfig ligoConfig);
    void extractWords(QString filename, QString columnId, IWordListBuilder &wordListBuilder);
    bool isBotNamesTable();
    bool isSheetTable(QString type);
    void closeEvent(QCloseEvent *event);
private Q_SLOTS:
    void worksheetEditorChanged(int,int);
    void insertRow();
    void deleteRow();
private:
    void setCurrentFile(QString filename);
    
};

};
#endif	/* EDITOR_WORKSHEET_H */

