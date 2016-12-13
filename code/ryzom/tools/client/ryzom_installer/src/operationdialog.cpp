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
#include "utils.h"
#include "nel/misc/path.h"

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

COperationDialog::COperationDialog(QWidget *parent):QDialog(parent), m_aborting(false), m_operation(OperationNone),
	m_operationStep(DisplayNoServerError), m_operationStepCounter(0)
{
	setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button = new QWinTaskbarButton(this);
#endif

	// downloader
	m_downloader = new CDownloader(this, this);

	connect(m_downloader, SIGNAL(downloadPrepared()), SLOT(onDownloadPrepared()));
	connect(m_downloader, SIGNAL(downloadDone()), SLOT(onDownloadDone()));

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

	adjustSize();

	// fix height
	setFixedHeight(height());

	raise();
}

COperationDialog::~COperationDialog()
{
}

void COperationDialog::setOperation(OperationType operation)
{
	m_operation = operation;
}

void COperationDialog::setUninstallComponents(const SComponents &components)
{
	m_removeComponents = components;
}

void COperationDialog::processNextStep()
{
	if (operationShouldStop())
	{
		rejectDelayed();
		return;
	}

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

	// long operations are done in a thread
	OperationStep step = config->getInstallNextStep();

	if (step == m_operationStep)
	{
		++m_operationStepCounter;
	}
	else
	{
		m_operationStep = step;
		m_operationStepCounter = 0;
	}

	if (m_operationStepCounter > 10)
	{
		nlwarning("Possible infinite loop, step %s %d times", Q2C(stepToString(m_operationStep)), m_operationStepCounter);
	}

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

		case CreateProfileShortcuts:
		createProfileShortcuts(0);
		break;

		case CreateAddRemoveEntry:
		createAddRemoveEntry();
		break;

		case Done:
		case LaunchInstalledInstaller:
		acceptDelayed();
		break;

		default:
		// cases already managed in main.cpp
		nlwarning("Shouldn't happen, step %s", Q2C(stepToString(step)));

		break;
	}
}

void COperationDialog::updateAddRemoveComponents()
{
	QStringList serversToUpdate;

	QStringList profilesToDelete;
	QStringList profilesToAdd;

	CConfigFile *config = CConfigFile::getInstance();

	foreach(const CProfile &profile, config->getProfiles())
	{
		// append all new profiles
		profilesToAdd << profile.id;
	}

	foreach(const CProfile &profile, config->getBackupProfiles())
	{
		// append all old profiles
		profilesToDelete << profile.id;

		// remove profiles that didn't exist
		profilesToAdd.removeAll(profile.id);

		// delete all shortcuts, we'll recreate them later
		profile.deleteShortcuts();
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

		// delete all shortcuts, they'll be recreated later
		profile.deleteShortcuts();
	}

	// update components to remove
	m_removeComponents.profiles << profilesToDelete;
	m_removeComponents.installer = false;
	m_removeComponents.downloadedFiles = false;

	// update components to add
	m_addComponents.profiles << profilesToAdd;
	m_addComponents.servers << serversToUpdate;
	m_addComponents.installer = false;
	m_addComponents.downloadedFiles = false;
}

void COperationDialog::processUpdateProfilesNextStep()
{
	m_currentOperation = tr("Updating profiles...");

	// for "update profiles" operations, we set installer to false when components are updated,
	// since we're not using this variable
	if (m_addComponents.installer && m_removeComponents.installer)
	{
		updateAddRemoveComponents();
	}

	// TODO: check all servers are downloaded
	// TODO: delete profiles directories that are not used anymore

	if (!m_removeComponents.profiles.isEmpty())
	{
		// delete profiles in another thread
		QtConcurrent::run(this, &COperationDialog::deleteComponentsProfiles);
		return;
	}

	if (!m_addComponents.profiles.isEmpty())
	{
		// add profiles in another thread
		QtConcurrent::run(this, &COperationDialog::addComponentsProfiles);
		return;
	}

	CConfigFile *config = CConfigFile::getInstance();

	if (!m_addComponents.servers.isEmpty())
	{
		const CServer &defaultServer = config->getServer();

		// servers files to download/update
		foreach(const QString &serverId, m_addComponents.servers)
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
					downloadData();
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
					{
						downloadData();
						return;
					}
				}
			}
			else
			{
				QString clientFile = config->getInstallationDirectory() + "/" + config->expandVariables(server.clientDownloadFilename);
			}
		}
	}

	// recreate all shortcuts
	foreach(const CProfile &profile, config->getProfiles())
	{
		profile.createShortcuts();
	}

	updateAddRemoveEntry();

	acceptDelayed();
}

void COperationDialog::processUninstallNextStep()
{
	CConfigFile *config = CConfigFile::getInstance();

	if (!m_removeComponents.servers.isEmpty())
	{
		QtConcurrent::run(this, &COperationDialog::deleteComponentsServers);
	}
	else if (!m_removeComponents.profiles.isEmpty())
	{
		QtConcurrent::run(this, &COperationDialog::deleteComponentsProfiles);
	}
	else if (m_removeComponents.downloadedFiles)
	{
		QtConcurrent::run(this, &COperationDialog::deleteComponentsDownloadedFiles);
	}
	else if (m_removeComponents.installer)
	{
		QtConcurrent::run(this, &COperationDialog::deleteComponentsInstaller);
	}
	else
	{
		// done
		acceptDelayed();
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

void COperationDialog::onDownloadPrepared()
{
	// actually download the file
	m_downloader->getFile();
}

void COperationDialog::onDownloadDone()
{
	renamePartFile();

	emit done();
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

	rejectDelayed();
}

void COperationDialog::onProgressProgress(qint64 current, const QString &filename)
{
	operationProgressLabel->setText(filename);

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

	{
		QMutexLocker locker(&m_abortingMutex);
		m_aborting = true;
	}

	processNextStep();
}

void COperationDialog::onDone()
{
	processNextStep();
}

void COperationDialog::downloadData()
{
	CConfigFile *config = CConfigFile::getInstance();

	const CServer &server = config->getServer(m_currentServerId);

	m_currentOperation = tr("Downloading data required by server %1...").arg(server.name);

	m_downloader->prepareFile(config->expandVariables(server.dataDownloadUrl), config->getInstallationDirectory() + "/" + config->expandVariables(server.dataDownloadFilename) + ".part");
}

void COperationDialog::extractDownloadedData()
{
	CConfigFile *config = CConfigFile::getInstance();

	const CServer &server = config->getServer(m_currentServerId);

	m_currentOperation = tr("Extracting data required by server %1...").arg(server.name);

	QString dest = server.getDirectory();

#ifdef Q_OS_MAC
	// under OS X, data should be uncompressed in Ryzom.app/Contents/Resources
	dest += "/Ryzom.app/Contents/Resources";
#endif

	CFilesExtractor extractor(this);
	extractor.setSourceFile(config->getInstallationDirectory() + "/" + server.dataDownloadFilename);
	extractor.setDestinationDirectory(dest);

	if (!extractor.exec()) return;

	emit done();
}

void COperationDialog::downloadClient()
{
	CConfigFile *config = CConfigFile::getInstance();

	const CServer &server = config->getServer(m_currentServerId);

	m_currentOperation = tr("Downloading client required by server %1...").arg(server.name);

	m_downloader->prepareFile(config->expandVariables(server.clientDownloadUrl), config->getInstallationDirectory() + "/" + config->expandVariables(server.clientDownloadFilename) + ".part");
}

void COperationDialog::extractDownloadedClient()
{
	CConfigFile *config = CConfigFile::getInstance();

	const CServer &server = config->getServer(m_currentServerId);

	m_currentOperation = tr("Extracting client required by server %1...").arg(server.name);

	QString destinationDirectory = server.getDirectory();

	CFilesExtractor extractor(this);
	extractor.setSourceFile(config->getInstallationDirectory() + "/" + config->expandVariables(server.clientDownloadFilename));
	extractor.setDestinationDirectory(destinationDirectory);

	if (!extractor.exec()) return;

	launchUpgradeScript(destinationDirectory, server.clientFilename);

	emit done();
}

void COperationDialog::copyDataFiles()
{
	CConfigFile *config = CConfigFile::getInstance();

	// default server
	const CServer &server = config->getServer(m_currentServerId);

	m_currentOperation = tr("Copying data required by server %1...").arg(server.name);

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

	if (!copier.exec()) return;

	emit done();
}

void COperationDialog::copyProfileFiles()
{
	CConfigFile *config = CConfigFile::getInstance();

	// default server
	const CServer &server = config->getServer();

	// default profile
	const CProfile &profile = config->getProfile();

	m_currentOperation = tr("Copying old profile to new location...");

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

	if (!copier.exec()) return;

	// correct path to client_default.cfg
	profile.createClientConfig();

	emit done();
}

void COperationDialog::extractBnpClient()
{
	CConfigFile *config = CConfigFile::getInstance();

	// default server
	const CServer &server = config->getServer();

	m_currentOperation = tr("Extracting client to new location...");

	QString destinationDirectory = server.getDirectory();

	CFilesExtractor extractor(this);
	extractor.setSourceFile(config->getSrcServerClientBNPFullPath());
	extractor.setDestinationDirectory(destinationDirectory);

	if (!extractor.exec()) return;

	launchUpgradeScript(destinationDirectory, server.clientFilename);

	emit done();
}

void COperationDialog::launchUpgradeScript(const QString &directory, const QString &executable)
{
	QString upgradeScript = directory + "/upgd_nl.";

#ifdef Q_OS_WIN
	upgradeScript += "bat";
#else
	upgradeScript += "sh";
#endif

	if (QFile::exists(upgradeScript))
	{
		QProcess process;

		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("RYZOM_CLIENT", QDir::toNativeSeparators(directory + "/" + executable));
		env.insert("UNPACKPATH", QDir::toNativeSeparators(directory + "/unpack"));
		env.insert("ROOTPATH", QDir::toNativeSeparators(directory));
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
			nlwarning("Unable to set executable flag to %s", Q2C(upgradeScript));
		}

		process.start(upgradeScript);

		while (process.waitForFinished())
		{
			nlwarning("Waiting end of %s", Q2C(upgradeScript));
		}
	}

}

void COperationDialog::copyInstaller()
{
	CConfigFile *config = CConfigFile::getInstance();

	m_currentOperation = tr("Copying installer to new location...");

	QString newInstallerFullPath = config->getInstallerInstalledFilePath();

	if (!newInstallerFullPath.isEmpty())
	{
		QString destinationDirectory = config->getInstallationDirectory();
		QString oldInstallerFullPath = config->getInstallerCurrentFilePath();
		QString srcDir = config->getInstallerCurrentDirPath();

		// always copy new installers
		CFilesCopier copier(this);
		copier.setIncludeFilter(config->getInstallerRequiredFiles());
#ifndef Q_OS_MAC
		copier.addFile(oldInstallerFullPath);
#endif
		copier.setSourceDirectory(srcDir);
		copier.setDestinationDirectory(config->getInstallationDirectory());

		if (!copier.exec()) return;

#ifndef Q_OS_MAC
		// copied file
		oldInstallerFullPath = config->getInstallationDirectory() + "/" + QFileInfo(oldInstallerFullPath).fileName();

		// rename old filename if different
		if (oldInstallerFullPath != newInstallerFullPath)
		{
			// delete previous installer
			QFile::remove(newInstallerFullPath);

			// rename new installer with final name
			QFile::rename(oldInstallerFullPath, newInstallerFullPath);
		}
#endif

		// create menu directory if defined
		QString path = config->getMenuDirectory();

		if (!path.isEmpty() && !QDir().mkpath(path))
		{
			nlwarning("Unable to create directory %s", Q2C(path));
		}

		// create installer link in menu
		QString executable = newInstallerFullPath;
		QString shortcut = config->getInstallerMenuShortcutFullPath();
		QString name = "Ryzom Installer";
		QString icon;

#ifdef Q_OS_WIN32
		// under Windows, icon is included in executable
		icon = executable;
#elif defined(Q_OS_MAC)
		// everything is in bundle
#else
		// icon is in the same directory as installer
		icon = config->getInstallationDirectory() + "/ryzom_installer.png";

		// create icon if not exists
		if (!QFile::exists(icon) && !writeResource(":/icons/ryzom.png", icon))
		{
			nlwarning("Unable to create icon %s", Q2C(icon));
		}
#endif

		createShortcut(shortcut, name, executable, "", icon, "");

		// installer already copied, don't need to copy it again
		config->setInstallerCopied(true);
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
				// to not ask twice
				CConfigFile::getInstance()->setUninstallingOldClient(true);

				// remember the choice
				CConfigFile::getInstance()->setShouldUninstallOldClient(true);

				// launch old uninstaller
				QDesktopServices::openUrl(QUrl::fromLocalFile(uninstaller));
			}
			else
			{
				// don't ask this question anymore
				CConfigFile::getInstance()->setShouldUninstallOldClient(false);
			}

			// save the choice
			CConfigFile::getInstance()->save();
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

	m_currentOperation = tr("Cleaning obsolete files...");

	CFilesCleaner cleaner(this);
	cleaner.setDirectory(server.getDirectory());
	cleaner.exec();

	emit done();
}

bool COperationDialog::createDefaultProfile()
{
	CConfigFile *config = CConfigFile::getInstance();

	CServer server = config->getServer();

	m_currentOperation = tr("Creating default profile...");

	CProfile profile;

	profile.id = "0";
	profile.name = QString("Ryzom (%1)").arg(server.name);
	profile.server = server.id;
	profile.comments = "Default profile created by Ryzom Installer";
	profile.desktopShortcut = false;
	profile.menuShortcut = false;

#ifdef Q_OS_WIN32
	QStringList paths;

	// desktop

	// Windows XP
	paths << "C:/Documents and Settings/All Users/Desktop";
	// since Windows Vista
	paths << "C:/Users/Public/Desktop";
	// new location
	paths << QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

	foreach(const QString &path, paths)
	{
		if (QFile::exists(path + "/Ryzom.lnk")) profile.desktopShortcut = true;
	}

	paths.clear();

	// start menu

	// Windows XP
	paths << "C:/Documents and Settings/All Users/Start Menu/Programs";
	// since Windows Vista
	paths << "C:/ProgramData/Microsoft/Windows/Start Menu/Programs";
	// new location
	paths << QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);

	foreach(const QString &path, paths)
	{
		if (QFile::exists(path + "/Ryzom/Ryzom.lnk")) profile.menuShortcut = true;
	}
#endif

	config->addProfile(profile);
	config->save();

	emit done();

	return true;
}

bool COperationDialog::createProfileShortcuts(const QString &profileId)
{
	CConfigFile *config = CConfigFile::getInstance();

	const CProfile &profile = config->getProfile(profileId);

	m_currentOperation = tr("Creating shortcuts for profile %1...").arg(profile.id);

	profile.createShortcuts();

	emit done();

	return true;
}

bool COperationDialog::createAddRemoveEntry()
{
	CConfigFile *config = CConfigFile::getInstance();

	QString newInstallerFullPath = config->getInstallerInstalledFilePath();

	if (!newInstallerFullPath.isEmpty() && QFile::exists(newInstallerFullPath))
	{
#ifdef Q_OS_WIN
		QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ryzom", QSettings::NativeFormat);

		QString nativeFullPath = QDir::toNativeSeparators(newInstallerFullPath);

		settings.setValue("Comments", config->getProductComments());
		settings.setValue("DisplayIcon", nativeFullPath + ",0");
		settings.setValue("DisplayName", QApplication::applicationName());
		settings.setValue("InstallDate", QDateTime::currentDateTime().toString("Ymd"));
		settings.setValue("InstallLocation", config->getInstallationDirectory());
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

	updateAddRemoveEntry();

	emit done();

	return true;
}

bool COperationDialog::updateAddRemoveEntry()
{
	CConfigFile *config = CConfigFile::getInstance();

	QString newInstallerFullPath = config->getInstallerInstalledFilePath();

	if (!newInstallerFullPath.isEmpty() && QFile::exists(newInstallerFullPath))
	{
		QString newInstallerFilename = config->getInstallerFilename();

#ifdef Q_OS_WIN
		QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ryzom", QSettings::NativeFormat);

		QString version = QApplication::applicationVersion();

		settings.setValue("DisplayVersion", version);
		settings.setValue("EstimatedSize", (quint32)(getDirectorySize(config->getInstallationDirectory(), true) / 1024)); // size if in KiB

		QStringList versionTokens = version.split('.');
		settings.setValue("MajorVersion", versionTokens[0].toInt());
		settings.setValue("MinorVersion", versionTokens[1].toInt());
#endif
	}

	return true;
}

bool COperationDialog::deleteAddRemoveEntry()
{
#ifdef Q_OS_WIN
	QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Ryzom", QSettings::NativeFormat);
	settings.remove("");
#endif

	return true;
}

void COperationDialog::deleteComponentsServers()
{
	m_currentOperation = tr("Deleting client...");

	emit prepare();
	emit init(0, m_removeComponents.servers.size());
	emit start();

	CConfigFile *config = CConfigFile::getInstance();

	int i = 0;

	foreach(const QString &serverId, m_removeComponents.servers)
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

		// delete all links to clients
		for (int i = 0; i < config->getProfilesCount(); ++i)
		{
			const CProfile &profile = config->getProfile(i);

			if (profile.server == serverId)
			{
				profile.deleteShortcuts();
			}
		}
	}

	emit success(m_removeComponents.servers.size());

	// clear list of all servers to uninstall
	m_removeComponents.servers.clear();

	// delete Ryzom directory if all files have been deleted
	if (isDirectoryEmpty(config->getInstallationDirectory(), true)) QDir(config->getInstallationDirectory()).removeRecursively();

	emit done();
}

void COperationDialog::addComponentsProfiles()
{
	m_currentOperation = tr("Adding profiles...");

	CConfigFile *config = CConfigFile::getInstance();

	foreach(const QString &profileId, m_addComponents.profiles)
	{
		const CProfile &profile = config->getProfile(profileId);

		profile.createShortcuts();
		profile.createClientConfig();
	}

	// clear list of all servers to uninstall
	m_addComponents.profiles.clear();

	emit done();
}

void COperationDialog::deleteComponentsProfiles()
{
	m_currentOperation = tr("Deleting profiles...");

	emit prepare();
	emit init(0, m_removeComponents.servers.size());

	CConfigFile *config = CConfigFile::getInstance();

	int i = 0;

	foreach(const QString &profileId, m_removeComponents.profiles)
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

		profile.deleteShortcuts();

		// delete profile
		config->removeProfile(profileId);
	}

	emit success(m_removeComponents.profiles.size());

	// clear list of all profiles to uninstall
	m_removeComponents.profiles.clear();

	// delete profiles directory if all files have been deleted
	if (isDirectoryEmpty(config->getProfileDirectory(), true)) QDir(config->getProfileDirectory()).removeRecursively();

	emit done();
}

void COperationDialog::deleteComponentsInstaller()
{
	m_currentOperation = tr("Deleting installer...");

	CConfigFile *config = CConfigFile::getInstance();

	deleteAddRemoveEntry();

	// delete menu
	QString path = config->getMenuDirectory();

	if (!path.isEmpty())
	{
		QDir dir(path);

		dir.removeRecursively();
	}

	path = config->getInstallerInstalledDirPath();
	QStringList files = config->getInstallerRequiredFiles();

	foreach(const QString &file, files)
	{
		QString fullPath = path + "/" + file;

		// delete file
		if (QFile::exists(fullPath) && !QFile::remove(fullPath))
		{
#ifdef Q_OS_WIN32
			// under Windows, a running executable is locked, so we need to delete it later
			MoveFileExW(qToWide(QDir::toNativeSeparators(fullPath)), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
#endif
		}
	}

	// delete installer shortcuts
	removeShortcut(config->getInstallerMenuShortcutFullPath());
	removeShortcut(config->getInstallerDesktopShortcutFullPath());

	// delete configuration file
	config->remove();

	// reset it once it's done
	m_removeComponents.installer = false;

	// delete Ryzom directory if all files have been deleted
	if (isDirectoryEmpty(config->getInstallationDirectory(), true)) QDir(config->getInstallationDirectory()).removeRecursively();

	emit success(1);
	emit done();
}

void COperationDialog::deleteComponentsDownloadedFiles()
{
	m_currentOperation = tr("Deleting downloaded files...");

	CConfigFile *config = CConfigFile::getInstance();

	QString path = config->getInstallationDirectory();

	QDir dir(path);

	QStringList filter;
	filter << "*.log";
	filter << "*.7z";
	filter << "*.bnp";
	filter << "*.zip";
	filter << "*.part";
	filter << "ryzom_installer_uninstalling_old_client";

	QStringList files = dir.entryList(filter, QDir::Files);

	foreach(const QString &file, files)
	{
		if (!QFile::remove(dir.filePath(file)))
		{
			nlwarning("Unable to delete file %s", Q2C(file));
		}
	}

	// reset it once it's done
	m_removeComponents.downloadedFiles = false;

	// delete Ryzom directory if all files have been deleted
	if (isDirectoryEmpty(config->getInstallationDirectory(), true)) QDir(config->getInstallationDirectory()).removeRecursively();

	emit success(1);
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

void COperationDialog::operationContinue()
{
	emit done();
}

bool COperationDialog::operationShouldStop()
{
	QMutexLocker locker(&m_abortingMutex);

	return m_aborting;
}

void COperationDialog::renamePartFile()
{
	QString partFile = m_downloader->getFileFullPath();

	QString finalFile = partFile;
	finalFile.remove(".part");

	if (partFile != finalFile)
	{
		QFile::rename(partFile, finalFile);
	}
}

void COperationDialog::acceptDelayed()
{
	// wait 500ms before to call accept()
	QTimer::singleShot(500, this, SLOT(accept()));
}

void COperationDialog::rejectDelayed()
{
	// wait 500ms before to call reject()
	QTimer::singleShot(500, this, SLOT(reject()));
}
