// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
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

// Project includes
#include "project_settings_dialog.h"
#include "world_editor_misc.h"

#include "../core/icore.h"
#include "../core/core_constants.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/ligo/ligo_config.h>

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QtGui/QFileDialog>

namespace WorldEditor
{

ProjectSettingsDialog::ProjectSettingsDialog(const QString &dataPath, QWidget *parent)
	: QDialog(parent)
{
	m_ui.setupUi(this);
	m_ui.pathLineEdit->setText(dataPath);
	m_ui.contextComboBox->addItem("empty");

	// Init the combo box
	const std::vector<std::string> &contexts = Utils::ligoConfig()->getContextString();
	for (uint i = 0; i < contexts.size(); i++)
		m_ui.contextComboBox->addItem(QString(contexts[i].c_str()));

	setFixedHeight(sizeHint().height());
	connect(m_ui.selectPathButton, SIGNAL(clicked()), this, SLOT(selectPath()));
}

ProjectSettingsDialog::~ProjectSettingsDialog()
{
}

QString ProjectSettingsDialog::dataPath() const
{
	return m_ui.pathLineEdit->text();
}

void ProjectSettingsDialog::selectPath()
{
	QString dataPath = QFileDialog::getExistingDirectory(this, tr("Select data path"), m_ui.pathLineEdit->text());
	if (!dataPath.isEmpty())
		m_ui.pathLineEdit->setText(dataPath);
}

} /* namespace WorldEditor */
