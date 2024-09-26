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
#include "uninstalldialog.h"
#include "configfile.h"
#include "utils.h"

#include "nel/misc/system_info.h"
#include "nel/misc/common.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CUninstallDialog::CUninstallDialog(QWidget *parent):QDialog(parent), m_installerIndex(-1)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	setupUi(this);

	CConfigFile *config = CConfigFile::getInstance();

	int serverCount = config->getServersCount();

	QStandardItemModel *model = new QStandardItemModel(0, 2, this);

	QStringList columns;
	columns << tr("Component");
	columns << tr("Size");

	model->setHorizontalHeaderLabels(columns);

	QStandardItem *item = NULL;

	// clients
	for (int row = 0; row < serverCount; ++row)
	{
		const CServer &server = config->getServer(row);

		if (QFile::exists(server.getDirectory()))
		{
			m_serversIndices[server.id] = model->rowCount();

			item = new QStandardItem(tr("Client for %1").arg(server.name));
			item->setCheckable(true);
			model->appendRow(item);
		}
	}

	int profilesCount = config->getProfilesCount();

	// profiles
	for (int row = 0; row < profilesCount; ++row)
	{
		const CProfile &profile = config->getProfile(row);

		m_profilesIndices[profile.id] = model->rowCount();

		item = new QStandardItem(tr("Profile #%1: %2").arg(profile.id).arg(profile.name));
		item->setCheckable(true);
		model->appendRow(item);

	}

	// installer
	m_installerIndex = model->rowCount();

	item = new QStandardItem(tr("Installer"));
	item->setCheckable(true);
	model->appendRow(item);

	// downloaded files
	m_downloadedFilesIndex = model->rowCount();

	item = new QStandardItem(tr("Downloaded Files"));
	item->setCheckable(true);
	model->appendRow(item);

	componentsTreeView->setModel(model);
	componentsTreeView->resizeColumnToContents(0);

	// resize layout depending on content and constraints
	adjustSize();

	// fix height because to left bitmap
	setFixedHeight(height());

	// click signals
	connect(uninstallButton, SIGNAL(clicked()), SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
	connect(model, SIGNAL(itemChanged(QStandardItem *)), SLOT(onItemChanged(QStandardItem *)));

	// semi-hack to not update UI on another thread
	connect(this, SIGNAL(updateSize(int, QString)), SLOT(onUpdateSize(int, QString)));
	connect(this, SIGNAL(updateLayout()), SLOT(onUpdateLayout()));
}

CUninstallDialog::~CUninstallDialog()
{
}

void CUninstallDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	// update size of all components sizes in a thread to not block interface
	QtConcurrent::run(this, &CUninstallDialog::updateSizes);
}

void CUninstallDialog::setSelectedComponents(const SComponents &components)
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(componentsTreeView->model());
	if (model == NULL) return;

	QStandardItem *item = NULL;

	// servers
	IDIndicesMap::const_iterator it = m_serversIndices.begin(), iend = m_serversIndices.end();

	while (it != iend)
	{
		item = model->item(it.value());

		if (item) item->setCheckState(components.servers.indexOf(it.key()) > -1 ? Qt::Checked : Qt::Unchecked);

		++it;
	}

	// profiles
	it = m_profilesIndices.begin(), iend = m_profilesIndices.end();

	while (it != iend)
	{
		item = model->item(it.value());

		if (item) item->setCheckState(components.profiles.indexOf(it.key()) > -1 ? Qt::Checked : Qt::Unchecked);

		++it;
	}

	// installer
	item = model->item(m_installerIndex);
	if (item) item->setCheckState(components.installer ? Qt::Checked : Qt::Unchecked);

	// downloaded files
	item = model->item(m_downloadedFilesIndex);
	if (item) item->setCheckState(components.downloadedFiles ? Qt::Checked : Qt::Unchecked);
}

SComponents CUninstallDialog::getSelectedCompenents() const
{
	SComponents res;

	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(componentsTreeView->model());
	if (model == NULL) return res;

	QStandardItem *item = NULL;

	// servers
	IDIndicesMap::const_iterator it = m_serversIndices.begin(), iend = m_serversIndices.end();

	while (it != iend)
	{
		item = model->item(it.value());

		if (item && item->checkState() == Qt::Checked) res.servers << it.key();

		++it;
	}

	// profiles
	it = m_profilesIndices.begin(), iend = m_profilesIndices.end();

	while (it != iend)
	{
		item = model->item(it.value());

		if (item && item->checkState() == Qt::Checked) res.profiles << it.key();

		++it;
	}

	// installer
	item = model->item(m_installerIndex);
	res.installer = item && item->checkState() == Qt::Checked;

	// downloaded files
	item = model->item(m_downloadedFilesIndex);
	res.downloadedFiles = item && item->checkState() == Qt::Checked;

	return res;
}

void CUninstallDialog::accept()
{
	QDialog::accept();
}

void CUninstallDialog::onItemChanged(QStandardItem * /* item */)
{
	updateButtons();
}

void CUninstallDialog::onUpdateSize(int row, const QString &text)
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(componentsTreeView->model());
	if (model == NULL) return;

	// set size for a component
	QStandardItem *item = new QStandardItem(text);
	model->setItem(row, 1, item);
}

void CUninstallDialog::onUpdateLayout()
{
	componentsTreeView->resizeColumnToContents(1);

	updateButtons();
}

void CUninstallDialog::updateSizes()
{
	CConfigFile *config = CConfigFile::getInstance();

	// clients
	IDIndicesMap::const_iterator it = m_serversIndices.begin(), iend = m_serversIndices.end();

	while(it != iend)
	{
		const CServer &server = config->getServer(it.key());

		qint64 bytes = getDirectorySize(server.getDirectory(), true);

		emit updateSize(it.value(), qBytesToHumanReadable(bytes));

		++it;
	}

	// profiles
	it = m_profilesIndices.begin(), iend = m_profilesIndices.end();

	while (it != iend)
	{
		const CProfile &profile = config->getProfile(it.key());

		// wrong profile
		if (profile.id.isEmpty()) continue;

		qint64 bytes = getDirectorySize(profile.getDirectory(), true);

		emit updateSize(it.value(), qBytesToHumanReadable(bytes));

		++it;
	}

	// downloaded files
	qint64 bytes = 0;
	
	QDir dir(config->getInstallationDirectory());

	QStringList filters;

	filters << "*.log";
	filters << "*.7z";
	filters << "*.bnp";
	filters << "*.zip";
	filters << "*.part";

	QFileInfoList downloadedFiles = dir.entryInfoList(filters, QDir::Files);

	foreach(const QFileInfo &info, downloadedFiles)
	{
		bytes += info.size();
	}

	emit updateSize(m_downloadedFilesIndex, qBytesToHumanReadable(bytes));

	emit updateLayout();
}

void CUninstallDialog::updateButtons()
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(componentsTreeView->model());
	if (model == NULL) return;

	int checkedCount = 0;

	for (int i = 0; i < model->rowCount(); ++i)
	{
		if (model->item(i)->checkState() == Qt::Checked) ++checkedCount;
	}

	// Uninstall button should be enabled only if at least one component is checked
	uninstallButton->setEnabled(checkedCount > 0);
}
