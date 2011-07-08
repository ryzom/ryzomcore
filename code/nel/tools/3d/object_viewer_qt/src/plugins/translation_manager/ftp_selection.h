/* 
 * File:   ftp_selection.h
 * Author: cemycc
 *
 * Created on July 8, 2011, 4:03 PM
 */

#ifndef FTP_SELECTION_H
#define	FTP_SELECTION_H

#include <QtCore/QObject>
#include <QtGui/QDialog>
#include <QtCore/QString>
#include <QtGui/QWidget>
#include <QtNetwork>
#include <QtCore/QUrl>

#include "ui_ftp_selection.h"

using namespace std;

namespace Plugin {
    
    class CFtpSelection : public QDialog
    {
        Q_OBJECT
    private:
        Ui::FtpSelectionDialog _ui;
        QFtp *conn;
        QHash<QString, bool> isDirectory;
        QString currentPath;
    private Q_SLOTS:
        void processItem(QTreeWidgetItem*,int);
        void CancelButtonClicked();
        void ConnectButtonClicked();
        void DoneButtonClicked();
        void FtpCommandFinished(int, bool error);
        void AddToList(const QUrlInfo &urlInfo);
    public:
        CFtpSelection(QWidget* parent = 0);
        ~CFtpSelection() {}
    };
}

#endif	/* FTP_SELECTION_H */

