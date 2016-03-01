// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

class QWinTaskbarButton;
class CDownloader;
class CArchive;

/**
 * Main window
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class CMainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT

public:
	CMainWindow();
	virtual ~CMainWindow();

public slots:
	void onResumeClicked();
	void onStopClicked();

	void onProfiles();
	void onAbout();
	void onAboutQt();

	void onDownloadPrepare();
	void onDownloadInit(qint64 current, qint64 total);
	void onDownloadStart();
	void onDownloadStop();
	void onDownloadProgress(qint64 current);
	void onDownloadSuccess(qint64 total);
	void onDownloadFail(const QString &error);

	void onHtmlPageContent(const QString &html);

	void onExtractPrepare();
	void onExtractInit(qint64 current, qint64 total);
	void onExtractStart();
	void onExtractStop();
	void onExtractProgress(qint64 current, const QString &filename);
	void onExtractSuccess(qint64 total);
	void onExtractFail(const QString &error);

protected:
	void showEvent(QShowEvent *e);
	void closeEvent(QCloseEvent *e);

	void processNextStep();

	void displayProgressBar();
	void displayConfigurationsChoices();

	QWinTaskbarButton *m_button;
	CDownloader *m_downloader;
	CArchive *m_archive;

	QLabel *m_statusLabel;
};

#endif
