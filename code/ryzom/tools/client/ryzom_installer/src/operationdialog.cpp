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

#include "stdpch.h"
#include "operationdialog.h"
#include "downloader.h"
#include "profilesdialog.h"
#include "configfile.h"
#include "config.h"
#include "profilesmodel.h"

#include "filescopier.h"
#include "filesextractor.h"
#include "filescleaner.h"

#include "seven_zip.h"

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
#include <QtWinExtras/QWinTaskbarProgress>
#include <QtWinExtras/QWinTaskbarButton>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

COperationDialog::COperationDialog(QWidget *parent):QDialog(parent), m_aborting(false), m_operation(OperationNone)
{
	setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button = new QWinTaskbarButton(this);
#endif

//	connect(resumeButton, SIGNAL(clicked()), SLOT(onResumeClicked()));
//	connect(stopButton, SIGNAL(clicked()), SLOT(onStopClicked()));

	// downloader
	m_downloader = new CDownloader(this, this);

	connect(operationButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onAbortClicked()));

	// operations
	connect(this, SIGNAL(prepare()), SLOT(onProgressPrepare()));
	connect(this, SIGNAL(init(qint64, qint64)), SLOT(onProgressInit(qint64, qint64)));
	connect(this, SIGNAL(start()), SLOT(onProgressStart()));
	connect(this, SIGNAL(stop()), SLOT(onProgressStop()));
	connect(this, SIGNAL(progress(qint64, QString)), SLOT(onProgressProgress(qint64, QString)));
	connect(this, SIGNAL(success(qint64)), SLOT(onProgressSuccess(qint64)));
	connect(this, SIGNAL(fail(QString)), SLOT(onProgressFail(QString)));
	connect(this, SIGNAL(done()), SLOT(onDone()));

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

COperationDialog::~COperationDialog()
{
}

void COperationDialog::setOperation(OperationType operation)
{
	m_operation = operation;
}

void COperationDialog::setUninstallComponents(const SUninstallComponents &components)
{
	m_components = components;
}

void COperationDialog::processNextStep()
{
	switch (m_operation)
	{
		case OperationMigrate:
		case OperationInstall:
			processInstallNextStep();
			break;

		case OperationUpdateProfiles:
			processUpdateProfilesNextStep();
			break;

		case OperationUninstall:
			processUninstallNextStep();
			break;

		default:
			break;
	}
}

void COperationDialog::processInstallNextStep()
{
	CConfigFile *config = CConfigFile::getInstance();

	// default server
	const CServer &server = config->getServer();

	// default profile
	const CProfile &configuration = config->getProfile();

	// long operations are done in a thread
	OperationStep step = config->getInstallNextStep();

	switch(step)
	{
		case DownloadData:
		downloadData();
		break;

		case ExtractDownloadedData:
		QtConcurrent::run(this, &COperationDialog::extractDownloadedData);
		break;

		case DownloadClient:
		downloadClient();
		break;

		case ExtractDownloadedClient:
		QtConcurrent::run(this, &COperationDialog::extractDownloadedClient);
		break;

		case CopyDataFiles:
		QtConcurrent::run(this, &COperationDialog::copyDataFiles);
		break;

		case CopyProfileFiles:
		QtConcurrent::run(this, &COperationDialog::copyProfileFiles);
		break;

		case CleanFiles:
		QtConcurrent::run(this, &COperationDialog::cleanFiles);
		break;

		case ExtractBnpClient:
		QtConcurrent::run(this, &COperationDialog::extractBnpClient);
		break;

		case CopyInstaller:
		QtConcurrent::run(this, &COperationDialog::copyInstaller);
		break;

		case UninstallOldClient:
		uninstallOldClient();
		break;

		case CreateProfile:
		createDefaultProfile();
		break;

		case CreateShortcuts:
		createDefaultShortcuts();
		break;

		case CreateAddRemoveEntry:
		createAddRemoveEntry();
		break;

		case Done:
		accept();
		break;

		default:
		// cases already managed in main.cpp
		qDebug() << "Shouldn't happen, step" << step;
		break;
	}
}

void COperationDialog::processUpdateProfilesNextStep()
{
	// TODO: check all servers are downloaded
	// TODO: delete profiles directories that are not used anymore
	// TODO: create shortcuts
}

	QStringList serversToUpdate;
	QStringList profilesToDelete;

	CConfigFile *config = CConfigFile::getInstance();

	// append all old profiles
	foreach(const CProfile &profile, config->getBackupProfiles())
	{
		if (QFile::exists(profile.getDirectory())) profilesToDelete << profile.id;
	}

	const CServer &defaultServer = config->getServer();

	foreach(const CProfile &profile, config->getProfiles())
	{
		const CServer &server = config->getServer(profile.server);

		QString serverDirectory = server.getDirectory();

		// check if Ryzom is installed in new server directory
		if (server.id != defaultServer.id && !config->isRyzomInstalledIn(serverDirectory) && serversToUpdate.indexOf(server.id) == -1)
		{
			serversToUpdate << server.id;
		}

		// remove profiles that still exist
		profilesToDelete.removeAll(profile.id);
	}

	if (!profilesToDelete.isEmpty())
	{
		m_components.profiles << profilesToDelete;

		// delete profiles in another thread
		QtConcurrent::run(this, &COperationDialog::deleteComponentsProfiles);

		return;
	}

	// servers files to download/update
	foreach(const QString &serverId, serversToUpdate)
	{
		const CServer &server = config->getServer(serverId);

		// data
		if (!config->areRyzomDataInstalledIn(server.getDirectory()))
		{
			QString dataFile = config->getInstallationDirectory() + "/" + server.dataDownloadFilename;

			// archive already downloaded
			if (QFile::exists(dataFile))
			{
				// make server current
				m_currentServerId = server.id;

				// uncompress it
				QtConcurrent::run(this, &COperationDialog::extractDownloadedData);
				return;
			}

			// data download URLs are different, can't copy data from default server
			if (server.dataDownloadUrl != defaultServer.dataDownloadUrl)
			{
				// download it
				// TODO

				return;
			}

			// same data used

			// copy them
			// TODO
			return;
		}

		// client
		if (!config->isRyzomClientInstalledIn(server.getDirectory()))
		{
			// client download URLs are different, can't copy client from default server
			if (server.clientDownloadUrl == defaultServer.clientDownloadUrl)
			{
				if (QFile::exists(""))
					downloadData();
				return;
			}
		}
		else
		{
			QString clientFile = config->getInstallationDirectory() + "/" + server.clientDownloadFilename;
		}
	}
}

void COperationDialog::processUninstallNextStep()
{
	CConfigFile *config = CConfigFile::getInstance();

	if (!m_components.servers.isEmpty())
	{
		QtConcurrent::run(this, &COperationDialog::deleteComponentsServers);
	}
	else if (!m_components.profiles.isEmpty())
	{
		QtConcurrent::run(this, &COperationDialog::deleteComponentsProfiles);
	}
	else if (m_components.installer)
	{
		QtConcurrent::run(this, &COperationDialog::deleteComponentsInstaller);
	}
	else
	{
		// done
		accept();
	}
}

void COperationDialog::showEvent(QShowEvent *e)
{
#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->setWindow(windowHandle());
#endif

	e->accept();

	processNextStep();
}

void COperationDialog::closeEvent(QCloseEvent *e)
{
	if (e->spontaneous())
	{
		e->ignore();

		onAbortClicked();
	}
}

void COperationDialog::onAbortClicked()
{
	if (m_downloader->isDownloading())
	{
		if (!m_downloader->supportsResume())
		{
			QMessageBox::StandardButton res = QMessageBox::question(this, tr("Confirmation"), tr("Warning, this server doesn't support resume! If you stop download now, you won't be able to resume it later.\nAre you sure to abort download?"));

			if (res != QMessageBox::Yes) return;
		}
	}

	QMutexLocker locker(&m_abortingMutex);
	m_aborting = true;
}

void COperationDialog::onProgressPrepare()
{
	operationProgressBar->setFormat(tr("%p% (%v/%m KiB)"));

	operationProgressBar->setMinimum(0);
	operationProgressBar->setMaximum(0);
	operationProgressBar->setValue(0);

	operationLabel->setText(m_currentOperation);
}

void COperationDialog::onProgressInit(qint64 current, qint64 total)
{
	operationProgressBar->setMinimum(0);
	operationProgressBar->setMaximum(total / 1024);
	operationProgressBar->setValue(current / 1024);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->setMinimum(0);
	m_button->progress()->setMaximum(total / 1024);
	m_button->progress()->setValue(current / 1024);
#endif
}

void COperationDialog::onProgressStart()
{
#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->show();
#endif
}

void COperationDialog::onProgressStop()
{
#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->hide();
#endif

	reject();
}

void COperationDialog::onProgressProgress(qint64 current, const QString &filename)
{
	operationProgressLabel->setText(m_currentOperationProgressFormat.arg(filename));

	operationProgressBar->setValue(current / 1024);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->setValue(current / 1024);
#endif
}

void COperationDialog::onProgressSuccess(qint64 total)
{
	operationProgressBar->setValue(total / 1024);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->hide();
#endif
}

void COperationDialog::onProgressFail(const QString &error)
{
	QMessageBox::critical(this, tr("Error"), error);
}

void COperationDialog::onDone()
{
	if (!operationShouldStop()) processNextStep();
}

void COperationDialog::downloadData()
{
	CConfigFile *config = CConfigFile::getInstance();

	const CServer &server = config->getServer(m_currentServerId);

	m_currentOperation = QApplication::tr("Download data required by server %1").arg(server.name);
	m_currentOperationProgressFormat = QApplication::tr("Downloading %1...");

	m_downloader->prepareFile(config->expandVariables(server.dataDownloadUrl), config->getInstallationDirectory() + "/" + config->expandVariables(server.dataDownloadFilename) + ".part");
}

void COperationDialog::extractDownloadedData()
{
	// TODO: implement
}

void COperationDialog::downloadClient()
{
	CConfigFile *config = CConfigFile::getInstance();

	const CServer &server = config->getServer(m_currentServerId);

	m_currentOperation = QApplication::tr("Download client required by server %1").arg(server.name);
	m_currentOperationProgressFormat = QApplication::tr("Downloading %1...");

	m_downloader->prepareFile(config->expandVariables(server.clientDownloadUrl), config->getInstallationDirectory() + "/" + config->expandVariables(server.clientDownloadFilename) + ".part");
}

void COperationDialog::extractDownloadedClient()
{
	CConfigFile *config = CConfigFile::getInstance();

	const CServer &server = config->getServer(m_currentServerId);

	m_currentOperation = QApplication::tr("Extract data files required by server %1").arg(server.name);
	m_currentOperationProgressFormat = QApplication::tr("Extracting %1...");

	CFilesExtractor extractor(this);
	extractor.setSourceFile(config->getInstallationDirectory() + "/" + server.clientDownloadFilename);
	extractor.setDestinationDirectory(server.getDirectory());

	if (extractor.exec())
	{
	}
	else
	{
	}

	emit done();
}

void COperationDialog::copyDataFiles()
{
	CConfigFile *config = CConfigFile::getInstance();

	// default server
	const CServer &server = config->getServer(m_currentServerId);

	m_currentOperation = QApplication::tr("Copy data files required by server %1").arg(server.name);
	m_currentOperationProgressFormat = QApplication::tr("Copying %1...");

	QStringList serverFiles;
	serverFiles << "cfg";
	serverFiles << "data";
	serverFiles << "examples";
	serverFiles << "patch";
	serverFiles << "unpack";

	CFilesCopier copier(this);
	copier.setSourceDirectory(config->getSrcServerDirectory());
	copier.setDestinationDirectory(server.getDirectory());
	copier.setIncludeFilter(serverFiles);

	if (copier.exec())
	{
	}
	else
	{
	}

	emit done();
}

void COperationDialog::copyProfileFiles()
{
	CConfigFile *config = CConfigFile::getInstance();

	// default server
	const CServer &server = config->getServer();

	// default profile
	const CProfile &profile = config->getProfile();

	m_currentOperation = QApplication::tr("Copy old profile to new location");
	m_currentOperationProgressFormat = QApplication::tr("Copying %1...");

	QStringList profileFiles;
	profileFiles << "cache";
	profileFiles << "save";
	profileFiles << "user";
	profileFiles << "screenshots";
	profileFiles << "client.cfg";
	profileFiles << "*.log";

	CFilesCopier copier(this);
	copier.setSourceDirectory(config->getSrcProfileDirectory());
	copier.setDestinationDirectory(profile.getDirectory());
	copier.setIncludeFilter(profileFiles);

	if (copier.exec())
	{
	}
	else
	{
	}

	emit done();
}

void COperationDialog::extractBnpClient()
{
	CConfigFile *config = CConfigFile::getInstance();

	// default server
	const CServer &server = config->getServer();

	m_currentOperation = QApplication::tr("Extract client to new location");
	m_currentOperationProgressFormat = QApplication::tr("Extracting %1...");

	QString destinationDirectory = server.getDirectory();

	CFilesExtractor extractor(this);
	extractor.setSourceFile(config->getSrcServerClientBNPFullPath());
	extractor.setDestinationDirectory(destinationDirectory);
	extractor.exec();

	QString upgradeScript = destinationDirectory + "/upgd_nl.";

#ifdef Q_OS_WIN
	upgradeScript += "bat";
#else
	upgradeScript += "sh";
#endif

	if (QFile::exists(upgradeScript))
	{
		QProcess process;

		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("RYZOM_CLIENT", QDir::toNativeSeparators(destinationDirectory + "/" + server.clientFilename));
		env.insert("UNPACKPATH", QDir::toNativeSeparators(destinationDirectory + "/unpack"));
		env.insert("ROOTPATH", QDir::toNativeSeparators(destinationDirectory));
		env.insert("STARTUPPATH", "");
		process.setProcessEnvironment(env);

		// permissions to execute script
		QFileDevice::Permissions permissions;
		permissions |= QFileDevice::ExeOther;
		permissions |= QFileDevice::ExeOwner;
		permissions |= QFileDevice::ExeUser;
		permissions |= QFileDevice::ReadOther;
		permissions |= QFileDevice::ReadOwner;
		permissions |= QFileDevice::ReadUser;
		permissions |= QFileDevice::WriteOwner;

		if (!QFile::setPermissions(upgradeScript, permissions))
		{
			qDebug() << "Unable to set executable flag to" << upgradeScript;
		}

		process.start(upgradeScript);

		while (process.waitForFinished())
		{
			qDebug() << "waiting";
		}
	}

	emit done();
}

void COperationDialog::copyInstaller()
{
	CConfigFile *config = CConfigFile::getInstance();

	// default server
	const CServer &server = config->getServer();

	m_currentOperation = QApplication::tr("Copy installer to new location");
	m_currentOperationProgressFormat = QApplication::tr("Copying %1...");

	QString destinationDirectory = config->getInstallationDirectory();

	// rename old client to installer
	QString newInstallerFilename = server.installerFilename;

	if (!newInstallerFilename.isEmpty())
	{
		QString oldInstallerFullPath = QApplication::applicationFilePath();
		QString newInstallerFullPath = config->getInstallationDirectory() + "/" + newInstallerFilename;

		if (!QFile::exists(newInstallerFullPath))
		{
			QStringList filter;
			filter << "msvcp100.dll";
			filter << "msvcr100.dll";

			CFilesCopier copier(this);
			copier.setIncludeFilter(filter);
			copier.addFile(oldInstallerFullPath);
			copier.setSourceDirectory(config->getSrcServerDirectory());
			copier.setDestinationDirectory(config->getInstallationDirectory());
			copier.exec();

			// copied file
			oldInstallerFullPath = config->getInstallationDirectory() + "/" + QFileInfo(oldInstallerFullPath).fileName();

			// rename old filename if different
			if (oldInstallerFullPath != newInstallerFullPath)
			{
				QFile::rename(oldInstallerFullPath, newInstallerFullPath);
			}
		}
	}

	emit done();
}

void COperationDialog::uninstallOldClient()
{
#ifdef Q_OS_WIN

#ifdef Q_OS_WIN64
	// use WOW6432Node in 64 bits (64 bits OS and 64 bits Installer) because Ryzom old installer was in 32 bits
	QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ryzom", QSettings::NativeFormat);
#else
	QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ryzom", QSettings::NativeFormat);
#endif

	// check if Ryzom 2.1.0 is installed
	if (settings.contains("UninstallString"))
	{
		QString uninstaller = settings.value("UninstallString").toString();

		if (!uninstaller.isEmpty() && QFile::exists(uninstaller))
		{
			QMessageBox::StandardButtons button = QMessageBox::question(this, tr("Uninstall old client"), tr("An old version of Ryzom has been detected on this system, would you like to uninstall it to save space disk?"));

			if (button == QMessageBox::Yes)
			{
				CConfigFile::getInstance()->setShouldUninstallOldClient(true);

				QDesktopServices::openUrl(QUrl::fromLocalFile(uninstaller));
			}
			else
			{
				// don't ask this question anymore
				CConfigFile::getInstance()->setShouldUninstallOldClient(false);
			}
		}
	}
#endif

	emit done();
}

void COperationDialog::cleanFiles()
{
	CConfigFile *config = CConfigFile::getInstance();

	// default server
	const CServer &server = config->getServer();

	m_currentOperation = QApplication::tr("Clean obsolete files");
	m_currentOperationProgressFormat = QApplication::tr("Deleting %1...");

	CFilesCleaner cleaner(this);
	cleaner.setDirectory(server.getDirectory());
	cleaner.exec();

	emit done();
}

bool COperationDialog::createDefaultProfile()
{
	CConfigFile *config = CConfigFile::getInstance();

	CServer server = config->getServer(config->getDefaultServerIndex());

	m_currentOperation = QApplication::tr("Create default profile");

	CProfile profile;

	profile.id = "0";
	profile.name = QString("Ryzom (%1)").arg(server.name);
	profile.server = server.id;
	profile.comments = "Default profile created by Ryzom Installer";

#ifdef Q_OS_WIN32
//	C:\Users\Public\Desktop
	profile.desktopShortcut = QFile::exists(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/Ryzom.lnk");
#endif

	// TODO
	// profile.menuShortcut

	config->addProfile(profile);
	config->save();

	emit done();

	return true;
}

bool COperationDialog::createDefaultShortcuts()
{
	emit done();

	return true;
}

bool COperationDialog::createAddRemoveEntry()
{
	CConfigFile *config = CConfigFile::getInstance();

	const CServer &server = config->getServer();

	QString oldInstallerFilename = server.clientFilenameOld;
	QString newInstallerFilename = server.installerFilename;

	if (!oldInstallerFilename.isEmpty() && !newInstallerFilename.isEmpty())
	{
		QString oldInstallerFullPath = config->getSrcServerDirectory() + "/" + oldInstallerFilename;
		QString newInstallerFullPath = config->getInstallationDirectory() + "/" + newInstallerFilename;

		if (QFile::exists(newInstallerFullPath))
		{
#ifdef Q_OS_WIN
			QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ryzom", QSettings::NativeFormat);

			QStringList versionTokens = QString(RYZOM_VERSION).split('.');
			QString nativeFullPath = QDir::toNativeSeparators(newInstallerFullPath);

			settings.setValue("Comments", "");
			settings.setValue("DisplayIcon", nativeFullPath + ",0");
			settings.setValue("DisplayName", "Ryzom");
			settings.setValue("DisplayVersion", RYZOM_VERSION);
			settings.setValue("EstimatedSize", 1500000); // TODO: compute real size
			settings.setValue("InstallDate", QDateTime::currentDateTime().toString("Ymd"));
			settings.setValue("InstallLocation", config->getInstallationDirectory());
			settings.setValue("MajorVersion", versionTokens[0].toInt());
			settings.setValue("MinorVersion", versionTokens[1].toInt());
			settings.setValue("NoModify", 0);
			settings.setValue("NoRemove", 0);
			settings.setValue("NoRepair", 0);
			if (!config->getProductPublisher().isEmpty()) settings.setValue("Publisher", config->getProductPublisher());
			settings.setValue("QuietUninstallString", nativeFullPath + " -u -s");
			settings.setValue("UninstallString", nativeFullPath + " -u");
			if (!config->getProductUpdateUrl().isEmpty()) settings.setValue("URLUpdateInfo", config->getProductUpdateUrl());
			if (!config->getProductAboutUrl().isEmpty()) settings.setValue("URLInfoAbout", config->getProductAboutUrl());
			if (!config->getProductHelpUrl().isEmpty()) settings.setValue("HelpLink", config->getProductHelpUrl());
			//	ModifyPath
#endif
		}
	}

	emit done();

	return true;
}

bool COperationDialog::deleteAddRemoveEntry()
{
#ifdef Q_OS_WIN
	QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ryzom", QSettings::NativeFormat);
	settings.remove("");
#endif

	emit done();

	return true;
}

void COperationDialog::deleteComponentsServers()
{
	m_currentOperation = QApplication::tr("Delete client files");
	m_currentOperationProgressFormat = QApplication::tr("Deleting %1...");

	emit prepare();
	emit init(0, m_components.servers.size());
	emit start();

	CConfigFile *config = CConfigFile::getInstance();

	int i = 0;

	foreach(const QString &serverId, m_components.servers)
	{
		if (operationShouldStop())
		{
			emit stop();
			return;
		}

		const CServer &server = config->getServer(serverId);

		emit progress(i++, server.name);

		QString path = server.getDirectory();

		if (!path.isEmpty())
		{
			QDir dir(path);

			if (dir.exists() && !dir.removeRecursively())
			{
				emit fail(tr("Unable to delete files for client %1").arg(server.name));
				return;
			}
		}
	}

	emit success(m_components.servers.size());
	emit done();
}

void COperationDialog::deleteComponentsProfiles()
{
	m_currentOperation = QApplication::tr("Delete profiles");
	m_currentOperationProgressFormat = QApplication::tr("Deleting profile %1...");

	emit prepare();
	emit init(0, m_components.servers.size());

	CConfigFile *config = CConfigFile::getInstance();

	int i = 0;

	foreach(const QString &profileId, m_components.profiles)
	{
		if (operationShouldStop())
		{
			emit stop();
			return;
		}

		const CProfile &profile = config->getProfile(profileId);

		emit progress(i++, profile.name);

		QString path = profile.getDirectory();

		if (!path.isEmpty())
		{
			QDir dir(path);

			if (dir.exists() && !dir.removeRecursively())
			{
				emit fail(tr("Unable to delete files for profile %1").arg(profile.name));
				return;
			}
		}

		// delete profile
		config->removeProfile(profileId);
	}

	// clear list of all profiles to uninstall
	m_components.profiles.clear();

	emit success(m_components.servers.size());
	emit done();
}

void COperationDialog::deleteComponentsInstaller()
{
	m_currentOperation = QApplication::tr("Delete installer");
	m_currentOperationProgressFormat = QApplication::tr("Deleting %1...");

	CConfigFile *config = CConfigFile::getInstance();

	// TODO: delete installer

	deleteAddRemoveEntry();

	emit onProgressSuccess(m_components.servers.size());
	emit done();
}

void COperationDialog::operationPrepare()
{
	emit prepare();
}

void COperationDialog::operationInit(qint64 current, qint64 total)
{
	emit init(current, total);
}

void COperationDialog::operationStart()
{
	emit start();
}

void COperationDialog::operationStop()
{
	emit stop();
}

void COperationDialog::operationProgress(qint64 current, const QString &filename)
{
	emit progress(current, filename);
}

void COperationDialog::operationSuccess(qint64 total)
{
	emit success(total);
}

void COperationDialog::operationFail(const QString &error)
{
	emit fail(error);
}

bool COperationDialog::operationShouldStop()
{
	QMutexLocker locker(&m_abortingMutex);

	return m_aborting;
}
