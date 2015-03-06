/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdpch.h"
#include "vegetable_dialog.h"

// Qt includes
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

// NeL includes
#include <nel/3d/vegetable.h>
#include <nel/3d/tile_vegetable_desc.h>
#include "nel/misc/file.h"

// Project includes
#include "modules.h"

namespace NLQT
{

CVegetableDialog::CVegetableDialog(QWidget *parent)
	: QDockWidget(parent)
{
	_ui.setupUi(this);

	connect(_ui.addtPushButton, SIGNAL(clicked()), this, SLOT(addVegetList()));
	connect(_ui.insPushButton, SIGNAL(clicked()), this, SLOT(insVegetList()));
	connect(_ui.delPushButton, SIGNAL(clicked()), this, SLOT(removeVegetList()));
	connect(_ui.clearPushButton, SIGNAL(clicked()), this, SLOT(clearVegetList()));
	connect(_ui.copyVegetPushButton, SIGNAL(clicked()), this, SLOT(copyVegetable()));
	connect(_ui.loadVegetdescPushButton, SIGNAL(clicked()), this, SLOT(loadVegetdesc()));
	connect(_ui.saveVegetdescPushButton, SIGNAL(clicked()), this, SLOT(saveVegetdesc()));
	connect(_ui.loadVegetsetPushButton, SIGNAL(clicked()), this, SLOT(loadVegetset()));
	connect(_ui.appendVegetsetPushButton, SIGNAL(clicked()), this, SLOT(appendVegetset()));
	connect(_ui.saveVegetsetPushButton, SIGNAL(clicked()), this, SLOT(saveVegetset()));
	connect(_ui.showVegetCheckBox, SIGNAL(toggled(bool)), this, SLOT(setVisibleVegetables(bool)));
	connect(_ui.listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentItem(int)));
}

CVegetableDialog::~CVegetableDialog()
{
}

void CVegetableDialog::loadVegetset()
{
	// ask name of the load new vegetset file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load a new vegetset file"),
					   ".",
					   tr("vegetset files (*.vegetset);;"));
	if (!fileName.isEmpty())
	{
		NL3D::CTileVegetableDesc vegetSet;
		// if succes to load the vegetSet
		if(Modules::veget().loadVegetableSet(vegetSet, fileName.toUtf8().constData()))
		{
			// Delete all vegetables.
			Modules::veget().clearVegetables();

			// build them from list.
			Modules::veget().appendVegetableSet(vegetSet);

			// update 3D view
			Modules::veget().refreshVegetableDisplay();

			updateVegetList();
		}
	}
}

void CVegetableDialog::appendVegetset()
{
	// ask name of the load new vegetset file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Append vegetset file"),
					   ".",
					   tr("vegetset files (*.vegetset);;"));
	if (!fileName.isEmpty())
	{
		NL3D::CTileVegetableDesc	vegetSet;
		// if succes to load the vegetSet
		if(Modules::veget().loadVegetableSet(vegetSet, fileName.toUtf8().constData()))
		{
			// Do not Delete any vegetables.
			// build them from list.
			Modules::veget().appendVegetableSet(vegetSet);

			// update 3D view
			Modules::veget().refreshVegetableDisplay();

			updateVegetList();
		}
	}
}

void CVegetableDialog::saveVegetset()
{
	NL3D::CTileVegetableDesc vegetSet;

	// first build the vegetSet.
	Modules::veget().buildVegetableSet(vegetSet);

	// Then try to save it.
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Vegetable Set"),
					   ".",
					   tr("VegetSetFiles (*.vegetset);;"));
	// after check
	if (!fileName.isEmpty())
	{
		NLMISC::COFile f;

		if( f.open(fileName.toUtf8().constData()) )
		{
			try
			{
				// save the vegetable set
				f.serial(vegetSet);
			}
			catch(NLMISC::EStream &e)
			{
				QMessageBox::critical(this, "Failed to save file!", QString(e.what()), QMessageBox::Ok);
			}
		}
		else
			QMessageBox::critical(this, "Failed to save file!", QString("Failed to open file for write!"), QMessageBox::Ok);
	}

}

void CVegetableDialog::addVegetList()
{
	// Add a new vegetable to the list.
	uint id = _ui.listWidget->count();

	Modules::veget().insEmptyVegetDesc(id);

	// update view
	QListWidgetItem *item = new QListWidgetItem(_ui.listWidget);
	item->setText(QString(Modules::veget().getVegetable(id)->_vegetableName.c_str()));

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableDialog::removeVegetList()
{
	sint id = _ui.listWidget->currentRow();
	if(id == -1) return;

	Modules::veget().delVegetDesc(id);

	QListWidgetItem *item = _ui.listWidget->takeItem(id);
	delete item;

	--id;

	_ui.listWidget->setCurrentRow(id);

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableDialog::insVegetList()
{
	sint id = _ui.listWidget->currentRow();
	if(id != -1)
	{
		// Add a new vegetable to the list.
		Modules::veget().insEmptyVegetDesc(id);

		// update view
		QListWidgetItem *item = new QListWidgetItem();
		item->setText(QString(Modules::veget().getVegetable(id)->_vegetableName.c_str()));
		_ui.listWidget->insertItem(id, item);

		// update 3D view
		Modules::veget().refreshVegetableDisplay();
	}
	else
	{
		// perform like an add.
		addVegetList();
	}
}

void CVegetableDialog::clearVegetList()
{
	if (_ui.listWidget->count() == 0) return;
	int ok = QMessageBox::question(this, "Clear List", QString("Clear all the list?"), QMessageBox::Yes | QMessageBox::No);
	if (ok == QMessageBox::Yes)
	{
		Modules::veget().clearVegetables();

		_ui.listWidget->clear();

		// update 3D view
		Modules::veget().refreshVegetableDisplay();
	}
}

void CVegetableDialog::copyVegetable()
{
}

void CVegetableDialog::loadVegetdesc()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Vegetable Descriptor"),
					   ".",
					   tr("vegetdesc files (*.vegetdesc);;"));
	if (!fileName.isEmpty())
	{
		NLMISC::CIFile f;

		if( f.open(fileName.toUtf8().constData()) )
		{
			NL3D::CVegetable veget;
			try
			{
				// read the vegetable
				f.serial(veget);

				// Add a new vegetable to the list.
				uint id = Modules::veget().addVegetDesc(veget);

				// update view
				QListWidgetItem *item = new QListWidgetItem(_ui.listWidget);
				item->setText(QString(Modules::veget().getVegetable(id)->_vegetableName.c_str()));

				// update 3D view
				Modules::veget().refreshVegetableDisplay();
			}
			catch(NLMISC::EStream &e)
			{
				QMessageBox::critical(this, "Failed to load file!", QString(e.what()), QMessageBox::Ok);
			}
		}
		else
			QMessageBox::critical(this, "Failed to open file!", QString("Failed to open file for read!"), QMessageBox::Ok);
	}
}

void CVegetableDialog::saveVegetdesc()
{
	sint id = _ui.listWidget->currentRow();
	if(id == -1) return;

	CVegetableNode *vegetNode = Modules::veget().getVegetable(id);

	QString oldFileName = QString(vegetNode->_vegetableName.c_str()) + ".vegetdesc";

	// Then try to save it.
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Vegetable Descriptor"),
					   oldFileName,
					   tr("VegetDescFiles (*.vegetdesc);;"));
	// after check
	if (!fileName.isEmpty())
	{
		NLMISC::COFile	f;

		if( f.open(fileName.toUtf8().constData()) )
		{
			try
			{
				// save the vegetable
				f.serial(*vegetNode->_vegetable);
			}
			catch(NLMISC::EStream &e)
			{
				QMessageBox::critical(this, "Failed to save file!", QString(e.what()), QMessageBox::Ok);
			}
		}
		else
			QMessageBox::critical(this, "Failed to save file!", QString("Failed to open file for write!"), QMessageBox::Ok);
	}
}

void CVegetableDialog::setVisibleVegetables(bool state)
{
	// update view.
	Modules::veget().enableLandscapeVegetable(state);
}

void CVegetableDialog::setCurrentItem(int row)
{
	NL3D::CVegetable *veget = NULL;
	if (row != -1)
		veget = Modules::veget().getVegetable(row)->_vegetable;
	_ui.densityPage->setVegetableToEdit(veget);
	_ui.appearancePage->setVegetableToEdit(veget);
	_ui.scalePage->setVegetableToEdit(veget);
	_ui.rotatePage->setVegetableToEdit(veget);
}

void CVegetableDialog::updateVegetList()
{
	std::vector<std::string> listVegetables;
	Modules::veget().getListVegetables(listVegetables);

	_ui.listWidget->clear();

	for (size_t i = 0; i < listVegetables.size(); i++)
	{
		QListWidgetItem *item = new QListWidgetItem(_ui.listWidget);
		item->setText(QString(listVegetables[i].c_str()));
	}
}

} /* namespace NLQT */