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

#ifndef OPERATIONDIALOG_H
#define OPERATIONDIALOG_H

#include "ui_operationdialog.h"
#include "operation.h"

class QWinTaskbarButton;
class CDownloader;
class CArchive;

/**
 * Main window
 *
 * \author Cedric 'Kervala' OCHS
 * \date 2016
 */
class COperationDialog : public QDialog, public Ui::OperationDialog, public IOperationProgressListener
{
	Q_OBJECT

public:
	COperationDialog(QWidget *parent = NULL);
	virtual ~COperationDialog();

	void setOperation(OperationType operation);
	void setUninstallComponents(const SComponents &components);
	void setCurrentServerId(const QString &serverId) { m_currentServerId = serverId; }

public slots:
	void onAbortClicked();

	void onDownloadPrepared();
	void onDownloadDone();

	void onProgressPrepare();
	void onProgressInit(qint64 current, qint64 total);
	void onProgressStart();
	void onProgressStop();
	void onProgressProgress(qint64 current, const QString &filename);
	void onProgressSuccess(qint64 total);
	void onProgressFail(const QString &error);
	void onDone();

signals:
	// emitted when requesting real URL
	void prepare();

	// emitted when we got the initial (local) and total (remote) size of file
	void init(qint64 current, qint64 total);

	// emitted when we begin to download
	void start();

	// emitted when the download stopped
	void stop();

	// emitted when extracting
	void progress(qint64 current, const QString &filename);

	// emitted when the whole file is downloaded
	void success(qint64 total);

	// emitted when an error occurs
	void fail(const QString &error);

	// emitted when done and should process next step
	void operationDone();

protected:
	void showEvent(QShowEvent *e);
	void closeEvent(QCloseEvent *e);

	void processNextStep();
	void processInstallNextStep();
	void processUninstallNextStep();
	void processUpdateProfilesNextStep();

	// operations
	void downloadData();
	void extractDownloadedData();
	void downloadClient();
	void extractDownloadedClient();
	void copyDataFiles();
	void copyProfileFiles();
	void cleanFiles();
	void extractBnpClient();
	void copyInstaller();
	void uninstallOldClient();
	bool createDefaultProfile();

	bool createProfileShortcuts(const QString &profileId);

	bool createAddRemoveEntry();
	bool updateAddRemoveEntry();
	bool deleteAddRemoveEntry();

	void addComponentsServers();
	void deleteComponentsServers();

	void addComponentsProfiles();
	void deleteComponentsProfiles();

	void deleteComponentsInstaller();
	void deleteComponentsDownloadedFiles();

	void updateAddRemoveComponents();

	// from CFilesCopier
	virtual void operationPrepare();
	virtual void operationInit(qint64 current, qint64 total);
	virtual void operationStart();
	virtual void operationStop();
	virtual void operationProgress(qint64 current, const QString &filename);
	virtual void operationSuccess(qint64 total);
	virtual void operationFail(const QString &error);
	virtual void operationContinue();

	virtual bool operationShouldStop();

	void renamePartFile();
	void launchUpgradeScript(const QString &directory, const QString &executable);

	// hacks to prevent an infinite loop
	void acceptDelayed();
	void rejectDelayed();

	QWinTaskbarButton *m_button;
	CDownloader *m_downloader;

	QString m_currentOperation;

	QMutex m_abortingMutex;
	bool m_aborting;

	OperationType m_operation;
	OperationStep m_operationStep;
	int m_operationStepCounter;
	SComponents m_addComponents;
	SComponents m_removeComponents;
	QString m_currentServerId;
};

#endif
