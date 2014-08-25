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
#include "vegetable_appearance_page.h"

// Qt includes
#include <QtGui/QPainter>
#include <QtGui/QColorDialog>
#include <QtGui/QInputDialog>

// NeL includes
#include <nel/3d/vegetable.h>

// Projects include
#include "modules.h"

namespace NLQT
{

CVegetableApperancePage::CVegetableApperancePage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.bendPhaseGroupBox->setDefaultRangeAbs(NL_VEGETABLE_BENDPHASE_RANGE_MIN, NL_VEGETABLE_BENDPHASE_RANGE_MAX);
	_ui.bendPhaseGroupBox->setDefaultRangeRand(NL_VEGETABLE_BENDPHASE_RANGE_MIN, NL_VEGETABLE_BENDPHASE_RANGE_MAX);
	_ui.bendPhaseGroupBox->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);

	_ui.bendFactorGroupBox->setDefaultRangeAbs(NL_VEGETABLE_BENDFACTOR_RANGE_MIN, NL_VEGETABLE_BENDFACTOR_RANGE_MAX);
	_ui.bendFactorGroupBox->setDefaultRangeRand(NL_VEGETABLE_BENDFACTOR_RANGE_MIN, NL_VEGETABLE_BENDFACTOR_RANGE_MAX);
	_ui.bendFactorGroupBox->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);

	_ui.colorNoiseGroupBox->setDefaultRangeAbs(NL_VEGETABLE_COLOR_RANGE_MIN, NL_VEGETABLE_COLOR_RANGE_MAX);
	_ui.colorNoiseGroupBox->setDefaultRangeRand(NL_VEGETABLE_COLOR_RANGE_MIN, NL_VEGETABLE_COLOR_RANGE_MAX);
	_ui.colorNoiseGroupBox->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);

	connect(_ui.bendFactorGroupBox, SIGNAL(noiseValueChanged(NLMISC::CNoiseValue)), this, SLOT(setNoiseValueBendFactor(NLMISC::CNoiseValue)));
	connect(_ui.bendPhaseGroupBox, SIGNAL(noiseValueChanged(NLMISC::CNoiseValue)), this, SLOT(setNoiseValueBendPhase(NLMISC::CNoiseValue)));
	connect(_ui.colorNoiseGroupBox, SIGNAL(noiseValueChanged(NLMISC::CNoiseValue)), this, SLOT(setNoiseValueColor(NLMISC::CNoiseValue)));

	connect(_ui.addColorPushButton, SIGNAL(clicked()), this, SLOT(addNewColor()));
	connect(_ui.insColorPushButton, SIGNAL(clicked()), this, SLOT(insNewColor()));
	connect(_ui.removePushButton, SIGNAL(clicked()), this, SLOT(removeColor()));
	connect(_ui.getListPushButton, SIGNAL(clicked()), this, SLOT(getFromListColors()));

	connect(_ui.listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(browseColor(QListWidgetItem *)));
	setEnabled(false);
}

CVegetableApperancePage::~CVegetableApperancePage()
{
}

QIcon getRectColorIcon(QColor color)
{
	QPixmap pixmap(QSize(32, 32));
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(QBrush(color));
	painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));
	painter.drawRect(0, 0, pixmap.width() , pixmap.height());
	return QIcon(pixmap);
}

void CVegetableApperancePage::setVegetableToEdit(NL3D::CVegetable *vegetable)
{
	_Vegetable = vegetable;

	if(_Vegetable)
	{
		setEnabled(true);
		_ui.bendPhaseGroupBox->setNoiseValue(_Vegetable->BendPhase, false);
		_ui.bendFactorGroupBox->setNoiseValue(_Vegetable->BendFactor, false);
		_ui.colorNoiseGroupBox->setNoiseValue(_Vegetable->Color.NoiseValue, false);

		updateColorList();
	}
	else
		setEnabled(false);
}

void CVegetableApperancePage::setNoiseValueBendPhase(const NLMISC::CNoiseValue &value)
{
	_Vegetable->BendPhase = value;
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableApperancePage::setNoiseValueBendFactor(const NLMISC::CNoiseValue &value)
{
	_Vegetable->BendFactor = value;
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableApperancePage::setNoiseValueColor(const NLMISC::CNoiseValue &value)
{
	_Vegetable->Color.NoiseValue = value;
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableApperancePage::browseColor(QListWidgetItem *item)
{
	sint row = _ui.listWidget->currentRow();

	NLMISC::CRGBA oldColor = _Vegetable->Color.Gradients[row];
	QColor color = QColorDialog::getColor(QColor(oldColor.R, oldColor.G, oldColor.B));
	if (!color.isValid()) return;

	item->setIcon(getRectColorIcon(color));

	_Vegetable->Color.Gradients[row] = NLMISC::CRGBA(color.red(), color.green(), color.blue());

	Modules::veget().refreshVegetableDisplay();
}

void CVegetableApperancePage::addNewColor()
{
	sint row = _ui.listWidget->currentRow();

	// copy the current selected color
	NLMISC::CRGBA color(255, 255, 255);
	if(row != -1)
		color = _Vegetable->Color.Gradients[row];

	// update view and vegetable
	QListWidgetItem *item = new QListWidgetItem();

	item->setIcon(getRectColorIcon(QColor(color.R, color.G, color.B)));

	_ui.listWidget->addItem(item);

	_Vegetable->Color.Gradients.push_back(color);

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableApperancePage::insNewColor()
{
	sint row = _ui.listWidget->currentRow();

	// copy the current selected color
	NLMISC::CRGBA color(255, 255, 255);

	if(row != -1)
		color = _Vegetable->Color.Gradients[row];
	else
		row++;

	// update view and vegetable
	QListWidgetItem *item = new QListWidgetItem();

	item->setIcon(getRectColorIcon(QColor(color.R, color.G, color.B)));

	_ui.listWidget->insertItem(row, item);

	_Vegetable->Color.Gradients.insert(_Vegetable->Color.Gradients.begin() + row ,color);

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableApperancePage::removeColor()
{
	sint row = _ui.listWidget->currentRow();
	if (row == -1) return;

	// remove curSel from the list
	QListWidgetItem *item = _ui.listWidget->takeItem(row);
	delete item;
	_Vegetable->Color.Gradients.erase(_Vegetable->Color.Gradients.begin() + row);

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableApperancePage::getFromListColors()
{
	std::vector<std::string> listVegetables;
	Modules::veget().getListVegetables(listVegetables);
	if (listVegetables.empty())
		return;

	QStringList items;
	for(size_t i = 0; i < listVegetables.size(); ++i)
		items << QString(listVegetables[i].c_str());

	bool ok;
	QString item = QInputDialog::getItem(this, tr("Select on other vegetables"),
										 tr("Select the other vegetable to copy color."), items, 0, false, &ok);
	if (ok)
	{
		int i = items.indexOf(item);

		NL3D::CVegetable *otherVegetable = Modules::veget().getVegetable(i)->_vegetable;

		_Vegetable->Color.Gradients = otherVegetable->Color.Gradients;

		updateColorList();

		// update 3D view
		Modules::veget().refreshVegetableDisplay();
	}
}

void CVegetableApperancePage::updateColorList()
{
	// clear all
	_ui.listWidget->clear();
	// fill list.
	for(uint i = 0; i < _Vegetable->Color.Gradients.size(); i++)
	{
		NLMISC::CRGBA color = _Vegetable->Color.Gradients[i];
		QListWidgetItem *item = new QListWidgetItem();
		item->setIcon(getRectColorIcon(QColor(color.R, color.G, color.B)));
		_ui.listWidget->addItem(item);
	}
}

} /* namespace NLQT */