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
#include "uninstallwizarddialog.h"
#include "configfile.h"
#include "utils.h"

#include "nel/misc/system_info.h"
#include "nel/misc/common.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CUninstallWizardDialog::CUninstallWizardDialog(QWidget *parent):QDialog(parent), m_installerIndex(-1)
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

		if (QFile::exists(config->getInstallationDirectory() + "/" + server.id))
		{
			m_serversIndices[row] = model->rowCount();

			item = new QStandardItem(tr("Client for %1").arg(server.name));
			item->setCheckable(true);
			model->appendRow(item);
		}
	}

	int profilesCount = config->getProfilesCount();

	// profiles
	for (int row = 0; row < profilesCount; ++row)
	{
		m_profilesIndices[row] = model->rowCount();

		const CProfile &profile = config->getProfile(row);

		item = new QStandardItem(tr("Profile #%1: %2").arg(profile.id).arg(profile.name));
		item->setCheckable(true);
		model->appendRow(item);

	}

	m_installerIndex = model->rowCount();

	item = new QStandardItem(tr("Ryzom Installer"));
	item->setCheckable(true);
	model->appendRow(item);

	componentsTreeView->setModel(model);
	componentsTreeView->resizeColumnToContents(0);

	connect(uninstallButton, SIGNAL(clicked()), SLOT(accept()));
	connect(quitButton, SIGNAL(clicked()), SLOT(reject()));
	connect(model, SIGNAL(itemChanged(QStandardItem *)), SLOT(onItemChanged(QStandardItem *)));

	adjustSize();

	QtConcurrent::run(this, &CUninstallWizardDialog::updateSizes);

	updateButtons();
}

CUninstallWizardDialog::~CUninstallWizardDialog()
{
}

void CUninstallWizardDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	QtConcurrent::run(this, &CUninstallWizardDialog::updateSizes);
}

void CUninstallWizardDialog::setSelectedComponents(const SUninstallComponents &components)
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(componentsTreeView->model());
	if (model == NULL) return;

	QStandardItem *item = NULL;

	// servers
	QMap<int, int>::const_iterator it = m_serversIndices.begin(), iend = m_serversIndices.end();

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
}

SUninstallComponents CUninstallWizardDialog::getSelectedCompenents() const
{
	SUninstallComponents res;

	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(componentsTreeView->model());
	if (model == NULL) return res;

	QStandardItem *item = NULL;

	// servers
	QMap<int, int>::const_iterator it = m_serversIndices.begin(), iend = m_serversIndices.end();

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

	return res;
}

void CUninstallWizardDialog::accept()
{
	QDialog::accept();
}

void CUninstallWizardDialog::onItemChanged(QStandardItem * /* item */)
{
	updateButtons();
}

void CUninstallWizardDialog::updateSizes()
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(componentsTreeView->model());

	CConfigFile *config = CConfigFile::getInstance();

	// clients
	QMap<int, int>::const_iterator it = m_serversIndices.begin(), iend = m_serversIndices.end();

	while(it != iend)
	{
		const CServer &server = config->getServer(it.key());

		qint64 bytes = getDirectorySize(config->getInstallationDirectory() + "/" + server.id);

		QStandardItem *item = new QStandardItem(qBytesToHumanReadable(bytes));
		model->setItem(it.value(), 1, item);

		++it;
	}

	// profiles
	it = m_profilesIndices.begin(), iend = m_profilesIndices.end();

	while (it != iend)
	{
		const CProfile &profile = config->getProfile(it.key());

		qint64 bytes = getDirectorySize(config->getProfileDirectory() + "/" + profile.id);

		QStandardItem *item = new QStandardItem(qBytesToHumanReadable(bytes));
		model->setItem(it.value(), 1, item);

		++it;
	}

	componentsTreeView->resizeColumnToContents(1);
}

void CUninstallWizardDialog::updateButtons()
{
	QStandardItemModel *model = qobject_cast<QStandardItemModel*>(componentsTreeView->model());

	int checkedCount = 0;

	// Uninstall button should be enabled only if at least one component is selected
	for (int i = 0; i < model->rowCount(); ++i)
	{
		if (model->item(i)->checkState() == Qt::Checked) ++checkedCount;
	}

	uninstallButton->setEnabled(checkedCount > 0);
}
