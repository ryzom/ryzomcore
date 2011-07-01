
#ifndef EDITOR_WORKSHEET_H
#define EDITOR_WORKSHEET_H

// Nel includes
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QWidget>
#include <QtGui/QMdiArea>
#include <QtGui/QTableWidget>
#include <QtGui/QMdiSubWindow>

#include "translation_manager_editor.h"

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
    void extractBotNames();
    bool isBotNamesTable();
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

