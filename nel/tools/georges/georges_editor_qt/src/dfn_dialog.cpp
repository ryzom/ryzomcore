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

#include "dfn_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QFileDialog>

#include "georges_editor_doc.h"

#include <nel/georges/form_dfn.h>

DfnDialog::DfnDialog(QWidget *parent)
	: QWidget(parent), _doc(nullptr)
{
	setupUi();
}

DfnDialog::~DfnDialog()
{
}

void DfnDialog::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	// Parents group
	QGroupBox *parentsGroup = new QGroupBox(tr("Parents"), this);
	QVBoxLayout *parentsLayout = new QVBoxLayout(parentsGroup);

	_parentsTable = new QTableWidget(0, 1, parentsGroup);
	_parentsTable->setHorizontalHeaderLabels(QStringList() << tr("Parent DFN"));
	_parentsTable->horizontalHeader()->setStretchLastSection(true);
	parentsLayout->addWidget(_parentsTable);

	QHBoxLayout *parentsBtnLayout = new QHBoxLayout();
	_addParentBtn = new QPushButton(tr("Add"), parentsGroup);
	_removeParentBtn = new QPushButton(tr("Remove"), parentsGroup);
	QPushButton *browseParentBtn = new QPushButton(tr("Browse..."), parentsGroup);
	parentsBtnLayout->addWidget(_addParentBtn);
	parentsBtnLayout->addWidget(_removeParentBtn);
	parentsBtnLayout->addWidget(browseParentBtn);
	parentsBtnLayout->addStretch();
	parentsLayout->addLayout(parentsBtnLayout);

	mainLayout->addWidget(parentsGroup);

	// Structure group
	QGroupBox *structGroup = new QGroupBox(tr("Structure"), this);
	QVBoxLayout *structLayout = new QVBoxLayout(structGroup);

	_structTable = new QTableWidget(0, 5, structGroup);
	_structTable->setHorizontalHeaderLabels(QStringList()
		<< tr("Name") << tr("Type") << tr("Type Name") << tr("Default/Filename") << tr("FilenameExt"));
	_structTable->horizontalHeader()->setStretchLastSection(true);
	structLayout->addWidget(_structTable);

	QHBoxLayout *structBtnLayout = new QHBoxLayout();
	_addStructBtn = new QPushButton(tr("Add"), structGroup);
	_removeStructBtn = new QPushButton(tr("Remove"), structGroup);
	QPushButton *browseTypeBtn = new QPushButton(tr("Browse Type..."), structGroup);
	structBtnLayout->addWidget(_addStructBtn);
	structBtnLayout->addWidget(_removeStructBtn);
	structBtnLayout->addWidget(browseTypeBtn);
	structBtnLayout->addStretch();
	structLayout->addLayout(structBtnLayout);

	mainLayout->addWidget(structGroup);

	// Connections
	connect(_parentsTable, &QTableWidget::cellChanged, this, &DfnDialog::onParentsCellChanged);
	connect(_structTable, &QTableWidget::cellChanged, this, &DfnDialog::onStructureCellChanged);
	connect(_addParentBtn, &QPushButton::clicked, this, &DfnDialog::onAddParent);
	connect(_removeParentBtn, &QPushButton::clicked, this, &DfnDialog::onRemoveParent);
	connect(_addStructBtn, &QPushButton::clicked, this, &DfnDialog::onAddStructEntry);
	connect(_removeStructBtn, &QPushButton::clicked, this, &DfnDialog::onRemoveStructEntry);
	connect(browseParentBtn, &QPushButton::clicked, this, &DfnDialog::onBrowseParent);
	connect(browseTypeBtn, &QPushButton::clicked, this, &DfnDialog::onBrowseTypeName);
}

void DfnDialog::setDocument(GeorgesEditorDoc *doc)
{
	_doc = doc;
	updateFromDocument();
}

void DfnDialog::updateFromDocument()
{
	if (!_doc || !_doc->isDfn() || !_doc->getDfnPtr())
		return;

	NLGEORGES::CFormDfn *dfn = _doc->getDfnPtr();

	_parentsTable->blockSignals(true);
	_structTable->blockSignals(true);

	// Parents
	_parentsTable->setRowCount((int)dfn->getNumParents());
	for (uint i = 0; i < dfn->getNumParents(); i++)
	{
		std::string name;
		dfn->getParentFilename(i, name);
		_parentsTable->setItem(i, 0, new QTableWidgetItem(QString::fromUtf8(name.c_str())));
	}

	// Structure entries
	_structTable->setRowCount((int)dfn->getNumEntry());
	for (uint i = 0; i < dfn->getNumEntry(); i++)
	{
		std::string name;
		dfn->getEntryName(i, name);
		_structTable->setItem(i, 0, new QTableWidgetItem(QString::fromUtf8(name.c_str())));

		std::string typeName;
		dfn->getEntryFilename(i, typeName);

		// Type column (Type/Dfn/Virtual Dfn)
		bool isArray;
		NLGEORGES::UFormDfn::TEntryType entryType;
		dfn->getEntryType(i, entryType, isArray);

		QString typeStr;
		switch (entryType)
		{
		case NLGEORGES::UFormDfn::EntryType:
			typeStr = "Type";
			break;
		case NLGEORGES::UFormDfn::EntryDfn:
			typeStr = "Dfn";
			break;
		case NLGEORGES::UFormDfn::EntryVirtualDfn:
			typeStr = "Virtual Dfn";
			break;
		}
		_structTable->setItem(i, 1, new QTableWidgetItem(typeStr));
		_structTable->setItem(i, 2, new QTableWidgetItem(QString::fromUtf8(typeName.c_str())));

		std::string dflt;
		dfn->getEntryFilenameExt(i, dflt);
		_structTable->setItem(i, 3, new QTableWidgetItem(""));
		_structTable->setItem(i, 4, new QTableWidgetItem(QString::fromUtf8(dflt.c_str())));
	}

	_parentsTable->blockSignals(false);
	_structTable->blockSignals(false);
}

void DfnDialog::onParentsCellChanged(int row, int column)
{
	Q_UNUSED(row);
	Q_UNUSED(column);
	if (_doc)
		_doc->setModified(true);
}

void DfnDialog::onStructureCellChanged(int row, int column)
{
	Q_UNUSED(row);
	Q_UNUSED(column);
	if (_doc)
		_doc->setModified(true);
}

void DfnDialog::onAddParent()
{
	int row = _parentsTable->rowCount();
	_parentsTable->setRowCount(row + 1);
	_parentsTable->setItem(row, 0, new QTableWidgetItem(""));
	if (_doc)
		_doc->setModified(true);
}

void DfnDialog::onRemoveParent()
{
	int row = _parentsTable->currentRow();
	if (row >= 0)
	{
		_parentsTable->removeRow(row);
		if (_doc)
			_doc->setModified(true);
	}
}

void DfnDialog::onAddStructEntry()
{
	int row = _structTable->rowCount();
	_structTable->setRowCount(row + 1);
	_structTable->setItem(row, 0, new QTableWidgetItem("NewEntry"));
	_structTable->setItem(row, 1, new QTableWidgetItem("Type"));
	_structTable->setItem(row, 2, new QTableWidgetItem(""));
	_structTable->setItem(row, 3, new QTableWidgetItem(""));
	_structTable->setItem(row, 4, new QTableWidgetItem(""));
	if (_doc)
		_doc->setModified(true);
}

void DfnDialog::onRemoveStructEntry()
{
	int row = _structTable->currentRow();
	if (row >= 0)
	{
		_structTable->removeRow(row);
		if (_doc)
			_doc->setModified(true);
	}
}

void DfnDialog::onBrowseParent()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Select Parent DFN"),
		QString(),
		tr("DFN Files (*.dfn);;All Files (*.*)"));
	if (fileName.isEmpty())
		return;

	int row = _parentsTable->currentRow();
	if (row < 0)
	{
		row = _parentsTable->rowCount();
		_parentsTable->setRowCount(row + 1);
	}
	_parentsTable->setItem(row, 0, new QTableWidgetItem(fileName));
	if (_doc)
		_doc->setModified(true);
}

void DfnDialog::onBrowseTypeName()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Select Type or DFN"),
		QString(),
		tr("Type Files (*.typ);;DFN Files (*.dfn);;All Files (*.*)"));
	if (fileName.isEmpty())
		return;

	int row = _structTable->currentRow();
	if (row >= 0)
	{
		_structTable->setItem(row, 2, new QTableWidgetItem(fileName));
		if (_doc)
			_doc->setModified(true);
	}
}
