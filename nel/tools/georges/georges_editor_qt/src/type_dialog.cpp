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

#include "type_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>

#include "georges_editor_doc.h"

#include <nel/georges/type.h>

TypeDialog::TypeDialog(QWidget *parent)
	: QWidget(parent), _doc(nullptr)
{
	setupUi();
}

TypeDialog::~TypeDialog()
{
}

void TypeDialog::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	// Type properties group
	QGroupBox *propsGroup = new QGroupBox(tr("Type Properties"), this);
	QFormLayout *formLayout = new QFormLayout(propsGroup);

	_typeCombo = new QComboBox(propsGroup);
	_typeCombo->addItem(tr("Unsigned Int"));
	_typeCombo->addItem(tr("Signed Int"));
	_typeCombo->addItem(tr("Double"));
	_typeCombo->addItem(tr("String"));
	_typeCombo->addItem(tr("Color"));
	formLayout->addRow(tr("Type:"), _typeCombo);

	_uiTypeCombo = new QComboBox(propsGroup);
	_uiTypeCombo->addItem(tr("Edit"));
	_uiTypeCombo->addItem(tr("EditSpin"));
	_uiTypeCombo->addItem(tr("NonEditable"));
	_uiTypeCombo->addItem(tr("FileBrowser"));
	_uiTypeCombo->addItem(tr("BigEdit"));
	_uiTypeCombo->addItem(tr("ColorEdit"));
	_uiTypeCombo->addItem(tr("IconWidget"));
	formLayout->addRow(tr("UI Type:"), _uiTypeCombo);

	_defaultCombo = new QComboBox(propsGroup);
	_defaultCombo->setEditable(true);
	formLayout->addRow(tr("Default:"), _defaultCombo);

	_minCombo = new QComboBox(propsGroup);
	_minCombo->setEditable(true);
	formLayout->addRow(tr("Min:"), _minCombo);

	_maxCombo = new QComboBox(propsGroup);
	_maxCombo->setEditable(true);
	formLayout->addRow(tr("Max:"), _maxCombo);

	_incrementCombo = new QComboBox(propsGroup);
	_incrementCombo->setEditable(true);
	formLayout->addRow(tr("Increment:"), _incrementCombo);

	mainLayout->addWidget(propsGroup);

	// Predefined values group
	QGroupBox *predefGroup = new QGroupBox(tr("Predefined Values"), this);
	QVBoxLayout *predefLayout = new QVBoxLayout(predefGroup);

	_predefTable = new QTableWidget(0, 2, predefGroup);
	_predefTable->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Value"));
	_predefTable->horizontalHeader()->setStretchLastSection(true);
	predefLayout->addWidget(_predefTable);

	QHBoxLayout *predefBtnLayout = new QHBoxLayout();
	QPushButton *addBtn = new QPushButton(tr("Add"), predefGroup);
	QPushButton *removeBtn = new QPushButton(tr("Remove"), predefGroup);
	predefBtnLayout->addWidget(addBtn);
	predefBtnLayout->addWidget(removeBtn);
	predefBtnLayout->addStretch();
	predefLayout->addLayout(predefBtnLayout);

	mainLayout->addWidget(predefGroup);

	// Connections
	connect(_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TypeDialog::onTypeChanged);
	connect(_uiTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TypeDialog::onUITypeChanged);
	connect(_defaultCombo, &QComboBox::currentTextChanged, this, &TypeDialog::onDefaultChanged);
	connect(_minCombo, &QComboBox::currentTextChanged, this, &TypeDialog::onMinChanged);
	connect(_maxCombo, &QComboBox::currentTextChanged, this, &TypeDialog::onMaxChanged);
	connect(_incrementCombo, &QComboBox::currentTextChanged, this, &TypeDialog::onIncrementChanged);
	connect(_predefTable, &QTableWidget::cellChanged, this, &TypeDialog::onPredefValueChanged);
	connect(addBtn, &QPushButton::clicked, this, &TypeDialog::onAddPredefValue);
	connect(removeBtn, &QPushButton::clicked, this, &TypeDialog::onRemovePredefValue);
}

void TypeDialog::setDocument(GeorgesEditorDoc *doc)
{
	_doc = doc;
	updateFromDocument();
}

void TypeDialog::updateFromDocument()
{
	if (!_doc || !_doc->isType() || !_doc->getTypePtr())
		return;

	NLGEORGES::CType *type = _doc->getTypePtr();

	// Block signals during update
	_typeCombo->blockSignals(true);
	_uiTypeCombo->blockSignals(true);
	_defaultCombo->blockSignals(true);
	_minCombo->blockSignals(true);
	_maxCombo->blockSignals(true);
	_incrementCombo->blockSignals(true);
	_predefTable->blockSignals(true);

	_typeCombo->setCurrentIndex((int)type->Type);
	_uiTypeCombo->setCurrentIndex((int)type->UIType);
	_defaultCombo->setEditText(QString::fromUtf8(type->Default.c_str()));
	_minCombo->setEditText(QString::fromUtf8(type->Min.c_str()));
	_maxCombo->setEditText(QString::fromUtf8(type->Max.c_str()));
	_incrementCombo->setEditText(QString::fromUtf8(type->Increment.c_str()));

	// Predefined values
	_predefTable->setRowCount((int)type->Definitions.size());
	for (uint i = 0; i < type->Definitions.size(); i++)
	{
		_predefTable->setItem(i, 0, new QTableWidgetItem(QString::fromUtf8(type->Definitions[i].Label.c_str())));
		_predefTable->setItem(i, 1, new QTableWidgetItem(QString::fromUtf8(type->Definitions[i].Value.c_str())));
	}

	_typeCombo->blockSignals(false);
	_uiTypeCombo->blockSignals(false);
	_defaultCombo->blockSignals(false);
	_minCombo->blockSignals(false);
	_maxCombo->blockSignals(false);
	_incrementCombo->blockSignals(false);
	_predefTable->blockSignals(false);
}

void TypeDialog::onTypeChanged(int index)
{
	if (!_doc || !_doc->getTypePtr())
		return;
	_doc->getTypePtr()->Type = (NLGEORGES::UType::TType)index;
	_doc->setModified(true);
}

void TypeDialog::onUITypeChanged(int index)
{
	if (!_doc || !_doc->getTypePtr())
		return;
	_doc->getTypePtr()->UIType = (NLGEORGES::CType::TUI)index;
	_doc->setModified(true);
}

void TypeDialog::onDefaultChanged(const QString &text)
{
	if (!_doc || !_doc->getTypePtr())
		return;
	_doc->getTypePtr()->Default = text.toUtf8().constData();
	_doc->setModified(true);
}

void TypeDialog::onMinChanged(const QString &text)
{
	if (!_doc || !_doc->getTypePtr())
		return;
	_doc->getTypePtr()->Min = text.toUtf8().constData();
	_doc->setModified(true);
}

void TypeDialog::onMaxChanged(const QString &text)
{
	if (!_doc || !_doc->getTypePtr())
		return;
	_doc->getTypePtr()->Max = text.toUtf8().constData();
	_doc->setModified(true);
}

void TypeDialog::onIncrementChanged(const QString &text)
{
	if (!_doc || !_doc->getTypePtr())
		return;
	_doc->getTypePtr()->Increment = text.toUtf8().constData();
	_doc->setModified(true);
}

void TypeDialog::onPredefValueChanged(int row, int column)
{
	if (!_doc || !_doc->getTypePtr())
		return;

	NLGEORGES::CType *type = _doc->getTypePtr();
	if (row >= (int)type->Definitions.size())
		return;

	QTableWidgetItem *item = _predefTable->item(row, column);
	if (!item)
		return;

	if (column == 0)
		type->Definitions[row].Label = item->text().toUtf8().constData();
	else
		type->Definitions[row].Value = item->text().toUtf8().constData();

	_doc->setModified(true);
}

void TypeDialog::onAddPredefValue()
{
	if (!_doc || !_doc->getTypePtr())
		return;

	NLGEORGES::CType *type = _doc->getTypePtr();
	NLGEORGES::CType::CDefinition def;
	def.Label = "NewValue";
	def.Value = "";
	type->Definitions.push_back(def);

	int row = _predefTable->rowCount();
	_predefTable->setRowCount(row + 1);
	_predefTable->setItem(row, 0, new QTableWidgetItem("NewValue"));
	_predefTable->setItem(row, 1, new QTableWidgetItem(""));
	_doc->setModified(true);
}

void TypeDialog::onRemovePredefValue()
{
	if (!_doc || !_doc->getTypePtr())
		return;

	int row = _predefTable->currentRow();
	if (row < 0)
		return;

	NLGEORGES::CType *type = _doc->getTypePtr();
	if (row < (int)type->Definitions.size())
	{
		type->Definitions.erase(type->Definitions.begin() + row);
	}
	_predefTable->removeRow(row);
	_doc->setModified(true);
}
