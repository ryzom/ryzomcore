
#ifndef TRANSLATION_MANAGER_EDITOR_H
#define TRANSLATION_MANAGER_EDITOR_H

#include <QtCore/QObject>
#include <QtGui/QWidget>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>

namespace Plugin {
    
class CEditor : public QMdiSubWindow {
Q_OBJECT
protected:
    QString current_file;
    int editor_type;
public:
    CEditor(QMdiArea* parent) : QMdiSubWindow(parent) {}
    CEditor() : QMdiSubWindow() {}
    virtual void open(QString filename) =0;
    virtual void save() =0;
    virtual void saveAs(QString filename) =0;
    virtual void activateWindow() =0;
public:
    QString subWindowFilePath()
    {
        return current_file;
    }
};

}


#endif	/* TRANSLATION_MANAGER_EDITOR_H */

