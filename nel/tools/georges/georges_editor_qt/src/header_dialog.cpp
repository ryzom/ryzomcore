// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "header_dialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>

#include "georges_editor_doc.h"

#include <nel/georges/header.h>

HeaderDialog::HeaderDialog(QWidget *parent)
	: QWidget(parent), _doc(nullptr)
{
	setupUi();
}

HeaderDialog::~HeaderDialog()
{
}

void HeaderDialog::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	QGroupBox *headerGroup = new QGroupBox(tr("Header"), this);
	QFormLayout *formLayout = new QFormLayout(headerGroup);

	// Version row with label and increment button
	QHBoxLayout *versionLayout = new QHBoxLayout();
	_versionLabel = new QLabel("0", headerGroup);
	_incrementBtn = new QPushButton(tr("Increment"), headerGroup);
	versionLayout->addWidget(_versionLabel);
	versionLayout->addWidget(_incrementBtn);
	versionLayout->addStretch();
	formLayout->addRow(tr("Version:"), versionLayout);

	// State combo
	_stateCombo = new QComboBox(headerGroup);
	_stateCombo->addItem(tr("Modified"));
	_stateCombo->addItem(tr("Checked"));
	formLayout->addRow(tr("State:"), _stateCombo);

	// Comments
	_commentsEdit = new QTextEdit(headerGroup);
	_commentsEdit->setMaximumHeight(100);
	formLayout->addRow(tr("Comments:"), _commentsEdit);

	// Log (read-only)
	_logEdit = new QTextEdit(headerGroup);
	_logEdit->setReadOnly(true);
	_logEdit->setMaximumHeight(150);
	formLayout->addRow(tr("Log:"), _logEdit);

	mainLayout->addWidget(headerGroup);
	mainLayout->addStretch();

	// Connections
	connect(_incrementBtn, &QPushButton::clicked, this, &HeaderDialog::onIncrementVersion);
	connect(_stateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HeaderDialog::onStateChanged);
	connect(_commentsEdit, &QTextEdit::textChanged, this, &HeaderDialog::onCommentsChanged);
}

void HeaderDialog::setDocument(GeorgesEditorDoc *doc)
{
	_doc = doc;
	updateFromDocument();
}

void HeaderDialog::updateFromDocument()
{
	if (!_doc)
		return;

	const NLGEORGES::CFileHeader *header = _doc->getHeaderPtr();
	if (!header)
		return;

	_versionLabel->blockSignals(true);
	_stateCombo->blockSignals(true);
	_commentsEdit->blockSignals(true);
	_logEdit->blockSignals(true);

	_versionLabel->setText(QString::number(header->MajorVersion) + "." + QString::number(header->MinorVersion));
	_stateCombo->setCurrentIndex((int)header->State);
	_commentsEdit->setPlainText(QString::fromUtf8(header->Comments.c_str()));
	_logEdit->setPlainText(QString::fromUtf8(header->Log.c_str()));

	_versionLabel->blockSignals(false);
	_stateCombo->blockSignals(false);
	_commentsEdit->blockSignals(false);
	_logEdit->blockSignals(false);
}

void HeaderDialog::onIncrementVersion()
{
	if (!_doc)
		return;

	NLGEORGES::CFileHeader *header = _doc->getHeaderPtr();
	if (!header)
		return;

	header->MinorVersion++;
	_versionLabel->setText(QString::number(header->MajorVersion) + "." + QString::number(header->MinorVersion));
	_doc->setModified(true);
}

void HeaderDialog::onStateChanged(int index)
{
	if (!_doc)
		return;

	NLGEORGES::CFileHeader *header = _doc->getHeaderPtr();
	if (!header)
		return;

	header->State = (NLGEORGES::CFileHeader::TState)index;
	_doc->setModified(true);
}

void HeaderDialog::onCommentsChanged()
{
	if (!_doc)
		return;

	NLGEORGES::CFileHeader *header = _doc->getHeaderPtr();
	if (!header)
		return;

	header->Comments = _commentsEdit->toPlainText().toUtf8().constData();
	_doc->setModified(true);
}
