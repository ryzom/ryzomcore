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

// Project includes
#include "stdpch.h"
#include "scheme_bank_dialog.h"
#include "scheme_manager.h"
#include "modules.h"

// NeL includes
#include <nel/misc/file.h>

// Qt includes
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

namespace NLQT
{

CSchemeBankDialog::CSchemeBankDialog(CAttribWidget *attribWidget, QWidget *parent)
	: QDialog(parent)
{
	_ui.setupUi(this);
	_attribWidget = attribWidget;

	connect(_ui.createButton, SIGNAL(clicked()), this, SLOT(createScheme()));
	connect(_ui.currentButton, SIGNAL(clicked()), this, SLOT(setCurrentScheme()));
	connect(_ui.removeButton, SIGNAL(clicked()), this, SLOT(removeScheme()));
	connect(_ui.loadButton, SIGNAL(clicked()), this, SLOT(loadBank()));
	connect(_ui.saveButton, SIGNAL(clicked()), this, SLOT(saveBank()));
	connect(_ui.listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(enableButtons()));
	connect(_ui.listWidget, SIGNAL(itemChanged(QListWidgetItem *)), this, SLOT(changeNameScheme(QListWidgetItem *)));

	buildList();
}

CSchemeBankDialog::~CSchemeBankDialog()
{
}

void CSchemeBankDialog::createScheme()
{
	bool ok;
	QString text = QInputDialog::getText(this, tr("Insert new scheme"),
										 tr("Set name:"), QLineEdit::Normal,
										 "new scheme", &ok);
	if (ok && !text.isEmpty())
	{
		NL3D::CPSAttribMakerBase *attribMakerBase = _attribWidget->getCurrentSchemePtr()->clone();
		Modules::psEdit().getSchemeManager()->insertScheme(text.toUtf8().constData(), attribMakerBase);

		CSchemeItem *item = new CSchemeItem(text, _ui.listWidget);
		item->setUserData(attribMakerBase);
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}
}

void CSchemeBankDialog::setCurrentScheme()
{
	CSchemeItem *item = dynamic_cast<CSchemeItem *>(_ui.listWidget->currentItem());

	NL3D::CPSAttribMakerBase *attrib = item->getUserData();
	nlassert(attrib);

	_attribWidget->setCurrentSchemePtr(attrib->clone());
	_attribWidget->updateUi();
}

void CSchemeBankDialog::removeScheme()
{
	CSchemeItem *item = dynamic_cast<CSchemeItem *>(_ui.listWidget->currentItem());

	NL3D::CPSAttribMakerBase *attrib = item->getUserData();
	nlassert(attrib);
	Modules::psEdit().getSchemeManager()->remove(attrib);
	_ui.listWidget->removeItemWidget(item);
	delete item;

	if (_ui.listWidget->count() == 0)
	{
		_ui.currentButton->setEnabled(false);
		_ui.removeButton->setEnabled(false);
	}
}

void CSchemeBankDialog::saveBank()
{
	QString fileName = QFileDialog::getSaveFileName(this,
					   tr("Save scheme bank file"), ".",
					   tr("Scheme bank files (*.scb)"));

	if (!fileName.isEmpty())
	{
		try
		{
			NLMISC::COFile iF;
			iF.open(fileName.toUtf8().constData());
			NLQT::CSchemeManager *schemeManager = Modules::psEdit().getSchemeManager();
			iF.serial(*schemeManager);
		}
		catch (std::exception &e)
		{
			QMessageBox::critical(this, "Scheme manager", tr("Error saving scheme bank : %1").arg(e.what()));
			return;
		}
	}
}

void CSchemeBankDialog::loadBank()
{
	QString fileName = QFileDialog::getOpenFileName(this,
					   tr("Open scheme bank file"), ".",
					   tr("Scheme bank files (*.scb)"));

	if (!fileName.isEmpty())
	{
		NLQT::CSchemeManager sm;
		try
		{
			NLMISC::CIFile iF;
			iF.open(fileName.toUtf8().constData());
			iF.serial(sm);
			Modules::psEdit().getSchemeManager()->swap(sm);
		}
		catch (std::exception &e)
		{
			QMessageBox::critical(this, "Scheme manager", tr("Error loading scheme bank : %1").arg(e.what()));
			return;
		}
		buildList();
	}
}

void CSchemeBankDialog::changeNameScheme(QListWidgetItem *item)
{
	CSchemeItem *schemeItem = dynamic_cast<CSchemeItem *>(item);

	NL3D::CPSAttribMakerBase *attrib = schemeItem->getUserData();
	nlassert(attrib);

	Modules::psEdit().getSchemeManager()->rename(attrib, item->text().toUtf8().constData());
}

void CSchemeBankDialog::enableButtons()
{
	_ui.currentButton->setEnabled(true);
	_ui.removeButton->setEnabled(true);
}

void CSchemeBankDialog::buildList()
{
	_ui.listWidget->clear();
	typedef std::vector<NLQT::CSchemeManager::TSchemeInfo> TSchemeVect;
	static TSchemeVect schemes;
	Modules::psEdit().getSchemeManager()->getSchemes(_attribWidget->getCurrentSchemePtr()->getType(), schemes);
	for (TSchemeVect::const_iterator it = schemes.begin(); it != schemes.end(); ++it)
	{
		CSchemeItem *item = new CSchemeItem(it->first.c_str(), _ui.listWidget);
		item->setUserData(it->second);
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}
}

} /* namespace NLQT */