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

#include "form_dialog.h"

#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>

#include "georges_editor_doc.h"

#include <nel/georges/form.h>
#include <nel/georges/form_dfn.h>
#include <nel/georges/u_form_elm.h>

FormDialog::FormDialog(QWidget *parent)
	: QWidget(parent), _doc(nullptr), _formContainer(nullptr), _formLayout(nullptr)
{
	setupUi();
}

FormDialog::~FormDialog()
{
}

void FormDialog::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	_scrollArea = new QScrollArea(this);
	_scrollArea->setWidgetResizable(true);

	_formContainer = new QWidget(_scrollArea);
	_formLayout = new QVBoxLayout(_formContainer);
	_formLayout->setAlignment(Qt::AlignTop);

	_scrollArea->setWidget(_formContainer);
	mainLayout->addWidget(_scrollArea);
}

void FormDialog::setDocument(GeorgesEditorDoc *doc)
{
	_doc = doc;
}

void FormDialog::updateFromDocument(CGeorgesEditDocSub *sub)
{
	clearFormWidgets();

	if (!_doc || !_doc->isForm() || !_doc->getFormPtr())
		return;

	if (sub)
	{
		buildFormWidgets(sub);
	}
	else
	{
		// Build from root
		QLabel *label = new QLabel(tr("Select a form node in the tree to edit its properties."), _formContainer);
		_formLayout->addWidget(label);
	}
}

void FormDialog::clearFormWidgets()
{
	QLayoutItem *child;
	while ((child = _formLayout->takeAt(0)) != nullptr)
	{
		if (child->widget())
			delete child->widget();
		delete child;
	}
}

void FormDialog::buildFormWidgets(CGeorgesEditDocSub *sub)
{
	if (!sub || !_doc || !_doc->getFormPtr())
		return;

	QGroupBox *group = new QGroupBox(QString::fromUtf8(sub->getName().c_str()), _formContainer);
	QFormLayout *form = new QFormLayout(group);

	// For each child of this sub, create an editor widget
	for (uint i = 0; i < sub->getChildrenCount(); i++)
	{
		CGeorgesEditDocSub *child = sub->getChild(i);
		if (!child)
			continue;

		QString name = QString::fromUtf8(child->getName().c_str());
		QString formName = QString::fromUtf8(child->getFormName().c_str());

		if (child->getChildrenCount() > 0)
		{
			// Struct or array - show as sub group
			QLabel *structLabel = new QLabel(tr("[Struct: %1]").arg(name), group);
			form->addRow(name, structLabel);
		}
		else
		{
			// Leaf node - create editable combo
			QComboBox *combo = new QComboBox(group);
			combo->setEditable(true);

			// Try to get value from form
			std::string value;
			NLGEORGES::UFormElm::TWhereIsValue where = NLGEORGES::UFormElm::ValueForm;
			bool result = false;
			if (!formName.isEmpty())
			{
				const NLGEORGES::UFormElm &root = _doc->getFormPtr()->getRootNode();
				result = root.getValueByName(value, formName.toUtf8().constData(), NLGEORGES::UFormElm::NoEval, &where);
			}

			if (result)
				combo->setEditText(QString::fromUtf8(value.c_str()));

			// Color coding based on where the value comes from.
			// Colors chosen to be legible on both dark Fusion and native palettes.
			switch (where)
			{
			case NLGEORGES::UFormElm::ValueForm:
				combo->setStyleSheet("QComboBox { color: #e0e0e0; }");
				break;
			case NLGEORGES::UFormElm::ValueParentForm:
				combo->setStyleSheet("QComboBox { color: #909090; }");
				break;
			case NLGEORGES::UFormElm::ValueDefaultDfn:
				combo->setStyleSheet("QComboBox { color: #6699ff; }");
				break;
			case NLGEORGES::UFormElm::ValueDefaultType:
				combo->setStyleSheet("QComboBox { color: #66cc66; }");
				break;
			}

			connect(combo, &QComboBox::currentTextChanged, this, &FormDialog::onFieldChanged);
			form->addRow(name, combo);
		}
	}

	_formLayout->addWidget(group);
}

void FormDialog::onFieldChanged()
{
	if (_doc)
		_doc->setModified(true);
}
