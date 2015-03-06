/*
 * File:   ftp_selection.h
 * Author: cemycc
 *
 * Created on July 8, 2011, 4:03 PM
 */

#ifndef FTP_SELECTION_H
#define	FTP_SELECTION_H

#include "ui_ftp_selection.h"

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtGui/QDialog>
#include <QtCore/QString>
#include <QtGui/QWidget>
#include <QtCore/QFile>
#include <QtNetwork>

namespace TranslationManager
{

class CFtpSelection : public QDialog
{
	Q_OBJECT

public:
	CFtpSelection(QWidget *parent = 0);
	~CFtpSelection() {}
	bool status;
	QFile *file;

private Q_SLOTS:
	void cdToParent();
	void processItem(QTreeWidgetItem *,int);
	void ConnectButtonClicked();
	void DoneButtonClicked();
	void FtpCommandFinished(int, bool error);
	void AddToList(const QUrlInfo &urlInfo);

private:
	Ui::FtpSelectionDialog _ui;
	QFtp *conn;
	QHash<QString, bool> isDirectory;
	QString currentPath;
};
}

#endif	/* FTP_SELECTION_H */