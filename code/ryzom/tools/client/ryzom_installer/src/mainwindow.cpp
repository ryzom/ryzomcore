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
#include "mainwindow.h"
#include "downloader.h"
#include "archive.h"
#include "wizarddialog.h"
#include "profilesdialog.h"
#include "configfile.h"
#include "config.h"

#include "seven_zip.h"

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
#include <QtWinExtras/QWinTaskbarProgress>
#include <QtWinExtras/QWinTaskbarButton>
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CMainWindow::CMainWindow():QMainWindow(), m_archive(NULL), m_statusLabel(NULL)
{
	setupUi(this);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button = new QWinTaskbarButton(this);
#endif

	connect(resumeButton, SIGNAL(clicked()), SLOT(onResumeClicked()));
	connect(stopButton, SIGNAL(clicked()), SLOT(onStopClicked()));

	// downloader
	m_downloader = new CDownloader(this);

	connect(m_downloader, SIGNAL(downloadPrepare()), SLOT(onDownloadPrepare()));
	connect(m_downloader, SIGNAL(downloadInit(qint64, qint64)), SLOT(onDownloadInit(qint64, qint64)));
	connect(m_downloader, SIGNAL(downloadStart()), SLOT(onDownloadStart()));
	connect(m_downloader, SIGNAL(downloadStop()), SLOT(onDownloadStop()));
	connect(m_downloader, SIGNAL(downloadProgress(qint64)), SLOT(onDownloadProgress(qint64)));
	connect(m_downloader, SIGNAL(downloadSuccess(qint64)), SLOT(onDownloadSuccess(qint64)));
	connect(m_downloader, SIGNAL(downloadFail(QString)), SLOT(onDownloadFail(QString)));
	connect(m_downloader, SIGNAL(htmlPageContent(QString)), SLOT(onHtmlPageContent(QString)));

	// archive
	m_archive = new CArchive(this);

	connect(m_archive, SIGNAL(extractPrepare()), SLOT(onExtractPrepare()));
	connect(m_archive, SIGNAL(extractInit(qint64, qint64)), SLOT(onExtractInit(qint64, qint64)));
	connect(m_archive, SIGNAL(extractStart()), SLOT(onExtractStart()));
	connect(m_archive, SIGNAL(extractStop()), SLOT(onExtractStop()));
	connect(m_archive, SIGNAL(extractProgress(qint64, QString)), SLOT(onExtractProgress(qint64, QString)));
	connect(m_archive, SIGNAL(extractSuccess(qint64)), SLOT(onExtractSuccess(qint64)));
	connect(m_archive, SIGNAL(extractFail(QString)), SLOT(onExtractFail(QString)));

	connect(actionProfiles, SIGNAL(triggered()), SLOT(onProfiles()));

	connect(actionAboutQt, SIGNAL(triggered()), SLOT(onAboutQt()));
	connect(actionAbout, SIGNAL(triggered()), SLOT(onAbout()));

	m_statusLabel = new QLabel();

	statusBar()->addWidget(m_statusLabel);

//	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}

CMainWindow::~CMainWindow()
{
}

void CMainWindow::processNextStep()
{
	CConfigFile *config = CConfigFile::getInstance();

	// default server
	const CServer &server = config->getServer();

	// default profile
	const CProfile &configuration = config->getProfile();

	switch(CConfigFile::getInstance()->getNextStep())
	{
		case CConfigFile::DisplayNoServerError:
		break;

		case CConfigFile::ShowWizard:
		break;

		case CConfigFile::DownloadData:
		displayProgressBar();
		m_downloader->prepareFile(config->expandVariables(server.dataDownloadUrl), config->getInstallationDirectory() + "/" + config->expandVariables(server.dataDownloadFilename) + ".part");
		break;

		case CConfigFile::ExtractDownloadedData:
		displayProgressBar();
		break;

		case CConfigFile::DownloadClient:
		displayProgressBar();
		m_downloader->prepareFile(config->expandVariables(server.clientDownloadUrl), config->getInstallationDirectory() + "/" + config->expandVariables(server.clientDownloadFilename) + ".part");
		break;

		case CConfigFile::ExtractDownloadedClient:
		displayProgressBar();
		break;

		case CConfigFile::CopyServerFiles:
		displayProgressBar();
		m_archive->copyServerFiles(config->getSrcServerDirectory(), config->getInstallationDirectory() + "/" + server.id);
		break;

		case CConfigFile::CopyProfileFiles:
		displayProgressBar();
		m_archive->copyProfileFiles(config->getSrcProfileDirectory(), config->getProfileDirectory() + "/0");
		break;

		case CConfigFile::ExtractBnpClient:
		displayProgressBar();
		m_archive->extract(config->getSrcServerClientBNPFullPath(), config->getInstallationDirectory() + "/" + server.id);
		break;

		case CConfigFile::CreateProfile:
		displayProgressBar();
		break;

		case CConfigFile::CreateShortcuts:
		displayProgressBar();
		break;

		default:
		// cases already managed in main.cpp
		displayConfigurationsChoices();
		break;
	}

	m_downloader->getHtmlPageContent(config->expandVariables(server.displayUrl));
}

void CMainWindow::displayProgressBar()
{
	downloadFrame->setVisible(true);
	configurationFrame->setVisible(false);

	resumeButton->setVisible(true);
	stopButton->setVisible(false);
}

void CMainWindow::displayConfigurationsChoices()
{
	downloadFrame->setVisible(false);
	configurationFrame->setVisible(true);
}

void CMainWindow::showEvent(QShowEvent *e)
{
#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->setWindow(windowHandle());
#endif

	e->accept();

	processNextStep();
}

void CMainWindow::closeEvent(QCloseEvent *e)
{
	hide();

	e->accept();
}

void CMainWindow::onResumeClicked()
{
	m_downloader->getFile();
}

void CMainWindow::onStopClicked()
{
	if (m_downloader->isDownloading())
	{
		if (!m_downloader->supportsResume())
		{
			QMessageBox::StandardButton res = QMessageBox::question(this, tr("Confirmation"), tr("Warning, this server doesn't support resume! If you stop download now, you won't be able to resume it later.\nAre you sure to abort download?"));

			if (res != QMessageBox::Yes) return;
		}

		m_downloader->stop();
	}
	else
	{
		m_archive->stop();
	}
}

void CMainWindow::onProfiles()
{
	CProfilesDialog dialog;

	dialog.exec();
}

void CMainWindow::onAbout()
{
	QString br("<br>");

	QMessageBox::about(this,
		tr("About %1").arg("Ryzom Installer"),
		QString("Ryzom Installer") + QApplication::applicationVersion() + br +
		tr("Program to install, download and manage Ryzom configurations.") +
		br+br+
		tr("Author: %1").arg("Cedric 'Kervala' OCHS") + br +
		tr("Copyright: %1").arg(COPYRIGHT) + br +
		tr("Support: %1").arg("<a href=\"https://bitbucket.org/ryzom/ryzomcore/issues?status=new&status=open\">Ryzom Core on Bitbucket</a>"));
}

void CMainWindow::onAboutQt()
{
	QMessageBox::aboutQt(this);
}

void CMainWindow::onDownloadPrepare()
{
	progressBar->setFormat(tr("%p% (%v/%m KiB)"));

	progressBar->setMinimum(0);
	progressBar->setMaximum(0);
	progressBar->setValue(0);

	resumeButton->setVisible(false);
	stopButton->setVisible(false);
}

void CMainWindow::onDownloadInit(qint64 current, qint64 total)
{
	resumeButton->setVisible(true);
	stopButton->setVisible(false);

	progressBar->setMinimum(0);
	progressBar->setMaximum(total / 1024);
	progressBar->setValue(current / 1024);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->setMinimum(0);
	m_button->progress()->setMaximum(total / 1024);
	m_button->progress()->setValue(current / 1024);
#endif
}

void CMainWindow::onDownloadStart()
{
	resumeButton->setVisible(false);
	stopButton->setVisible(true);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->show();
#endif
}

void CMainWindow::onDownloadStop()
{
	resumeButton->setVisible(true);
	stopButton->setVisible(false);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->hide();
#endif
}

void CMainWindow::onDownloadProgress(qint64 current)
{
	progressBar->setValue(current / 1024);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->setValue(current / 1024);
#endif
}

void CMainWindow::onDownloadSuccess(qint64 total)
{
	progressBar->setValue(total / 1024);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->hide();
#endif

	resumeButton->setVisible(false);
	stopButton->setVisible(false);
}

void CMainWindow::onDownloadFail(const QString &error)
{
	resumeButton->setVisible(true);
	stopButton->setVisible(false);
}

void CMainWindow::onHtmlPageContent(const QString &html)
{
	htmlTextEdit->setHtml(html);
}

void CMainWindow::onExtractPrepare()
{
	progressBar->setFormat("%p%");

	progressBar->setMinimum(0);
	progressBar->setMaximum(0);
	progressBar->setValue(0);

	resumeButton->setVisible(false);
	stopButton->setVisible(false);
}

void CMainWindow::onExtractInit(qint64 current, qint64 total)
{
	resumeButton->setVisible(true);
	stopButton->setVisible(false);

	progressBar->setMinimum(0);
	progressBar->setMaximum(total / 1024);
	progressBar->setValue(current / 1024);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->setMinimum(0);
	m_button->progress()->setMaximum(total / 1024);
	m_button->progress()->setValue(current / 1024);
#endif
}

void CMainWindow::onExtractStart()
{
	resumeButton->setVisible(false);
	stopButton->setVisible(true);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->show();
#endif
}

void CMainWindow::onExtractStop()
{
	resumeButton->setVisible(true);
	stopButton->setVisible(false);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->hide();
#endif
}

void CMainWindow::onExtractProgress(qint64 current, const QString &filename)
{
	m_statusLabel->setText(tr("Extracting %1...").arg(filename));

	progressBar->setValue(current / 1024);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->setValue(current / 1024);
#endif
}

void CMainWindow::onExtractSuccess(qint64 total)
{
	m_statusLabel->setText(tr("Extraction done"));

	progressBar->setValue(total / 1024);

#if defined(Q_OS_WIN32) && defined(QT_WINEXTRAS_LIB)
	m_button->progress()->hide();
#endif

	resumeButton->setVisible(false);
	stopButton->setVisible(false);

	processNextStep();
}

void CMainWindow::onExtractFail(const QString &error)
{
	resumeButton->setVisible(true);
	stopButton->setVisible(false);
}
