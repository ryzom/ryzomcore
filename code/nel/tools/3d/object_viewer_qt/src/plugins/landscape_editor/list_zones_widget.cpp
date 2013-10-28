// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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
#include "list_zones_widget.h"
#include "list_zones_model.h"
#include "builder_zone.h"

// NeL includes
#include <nel/misc/debug.h>
#include <nel/ligo/zone_bank.h>
#include <nel/ligo/zone_region.h>

// STL includes
#include <vector>
#include <string>

// Qt includes
#include <QtGui/QIcon>
#include <QtCore/QModelIndex>

namespace LandscapeEditor
{

ListZonesWidget::ListZonesWidget(QWidget *parent)
	: QWidget(parent),
	  m_rotCycle(0),
	  m_flipCycle(0),
	  m_listZonesModel(0),
	  m_zoneBuilder(0)
{
	m_ui.setupUi(this);

	m_listZonesModel = new ListZonesModel(4, this);
	m_ui.listView->setModel(m_listZonesModel);

	m_ui.addFilterButton_1->setChecked(false);
	m_ui.addFilterButton_2->setChecked(false);
	m_ui.addFilterButton_3->setChecked(false);

	connect(m_ui.categoryTypeComboBox_1, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateFilters_1(QString)));
	connect(m_ui.categoryTypeComboBox_2, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateFilters_2(QString)));
	connect(m_ui.categoryTypeComboBox_3, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateFilters_3(QString)));
	connect(m_ui.categoryTypeComboBox_4, SIGNAL(currentIndexChanged(QString)), this, SLOT(updateFilters_4(QString)));
	connect(m_ui.categoryValueComboBox_1, SIGNAL(currentIndexChanged(int)), this, SLOT(updateListZones()));
	connect(m_ui.categoryValueComboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateListZones()));
	connect(m_ui.categoryValueComboBox_3, SIGNAL(currentIndexChanged(int)), this, SLOT(updateListZones()));
	connect(m_ui.categoryValueComboBox_4, SIGNAL(currentIndexChanged(int)), this, SLOT(updateListZones()));
	connect(m_ui.logicComboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateListZones()));
	connect(m_ui.logicComboBox_3, SIGNAL(currentIndexChanged(int)), this, SLOT(updateListZones()));
	connect(m_ui.logicComboBox_4, SIGNAL(currentIndexChanged(int)), this, SLOT(updateListZones()));
}

ListZonesWidget::~ListZonesWidget()
{
}

void ListZonesWidget::updateUi()
{
	if (m_zoneBuilder == 0)
		return;

	disableSignals(true);
	std::vector<std::string> listCategoryType;
	m_zoneBuilder->getZoneBank().getCategoriesType(listCategoryType);

	QStringList listCategories;

	listCategories << STRING_UNUSED;
	for (size_t i = 0; i < listCategoryType.size(); ++i)
		listCategories << QString(listCategoryType[i].c_str());

	m_ui.categoryTypeComboBox_1->clear();
	m_ui.categoryTypeComboBox_2->clear();
	m_ui.categoryTypeComboBox_3->clear();
	m_ui.categoryTypeComboBox_4->clear();
	m_ui.categoryValueComboBox_1->clear();
	m_ui.categoryValueComboBox_2->clear();
	m_ui.categoryValueComboBox_3->clear();
	m_ui.categoryValueComboBox_4->clear();

	m_ui.categoryTypeComboBox_1->addItems(listCategories);
	m_ui.categoryTypeComboBox_2->addItems(listCategories);
	m_ui.categoryTypeComboBox_3->addItems(listCategories);
	m_ui.categoryTypeComboBox_4->addItems(listCategories);

	disableSignals(false);

	m_listZonesModel->rebuildModel(m_zoneBuilder->pixmapDatabase());
}

QString ListZonesWidget::currentZoneName()
{
	QString zoneName = "";
	QModelIndex index = m_ui.listView->currentIndex();
	if (index.isValid())
		zoneName = index.data().toString();
	if (m_ui.zoneSelectComboBox->currentIndex() == 1)
	{
		// Random value
		if (m_listSelection.size() > 0)
		{
			uint32 randZone = uint32(NLMISC::frand(m_listSelection.size()));
			NLMISC::clamp(randZone, (uint32)0, uint32(m_listSelection.size() - 1));
			zoneName = m_listSelection[randZone];
		}
	}
	else if (m_ui.zoneSelectComboBox->currentIndex() == 2)
	{
		// Full cycle
		if (m_listSelection.size() > 0)
		{
			zoneName = m_listSelection[m_zoneNameCycle];
			m_zoneNameCycle++;
			m_zoneNameCycle = m_zoneNameCycle % m_listSelection.size();
		}
	}
	return zoneName;
}

int ListZonesWidget::currentRot()
{
	int rot = m_ui.rotComboBox->currentIndex();
	if (rot == 4)
	{
		// Random value
		uint32 randRot = uint32(NLMISC::frand(4.0));
		NLMISC::clamp(randRot, (uint32)0, (uint32)3);
		rot = int(randRot);
	}
	else if (rot == 5)
	{
		// Full cycle
		rot = m_rotCycle;
		m_rotCycle++;
		m_rotCycle = m_rotCycle % 4;
	}
	return rot;
}

int ListZonesWidget::currentFlip()
{
	int flip = m_ui.flipComboBox->currentIndex();
	if (flip == 2)
	{
		// Random value
		uint32 randFlip = uint32(NLMISC::frand(2.0));
		NLMISC::clamp (randFlip, (uint32)0, (uint32)1);
		flip = int(randFlip);
	}
	else if (flip == 3)
	{
		// Full cycle
		flip = m_flipCycle;
		m_flipCycle++;
		m_flipCycle = m_flipCycle % 2;
	}
	return flip;
}

bool ListZonesWidget::isNotPropogate() const
{
	return m_ui.propogateCheckBox->isChecked();
}

bool ListZonesWidget::isForce() const
{
	return m_ui.forceCheckBox->isChecked();
}

void ListZonesWidget::setZoneBuilder(ZoneBuilder *zoneBuilder)
{
	m_zoneBuilder = zoneBuilder;
}

void ListZonesWidget::updateFilters_1(const QString &value)
{
	disableSignals(true);
	std::vector<std::string> allCategoryValues;
	m_zoneBuilder->getZoneBank().getCategoryValues(value.toStdString(), allCategoryValues);
	m_ui.categoryValueComboBox_1->clear();
	for(size_t i = 0; i < allCategoryValues.size(); ++i)
		m_ui.categoryValueComboBox_1->addItem(QString(allCategoryValues[i].c_str()));

	disableSignals(false);
	updateListZones();
}

void ListZonesWidget::updateFilters_2(const QString &value)
{
	disableSignals(true);
	std::vector<std::string> allCategoryValues;
	m_zoneBuilder->getZoneBank().getCategoryValues(value.toStdString(), allCategoryValues);

	m_ui.categoryValueComboBox_2->clear();
	for(size_t i = 0; i < allCategoryValues.size(); ++i)
		m_ui.categoryValueComboBox_2->addItem(QString(allCategoryValues[i].c_str()));

	disableSignals(false);
	updateListZones();
}

void ListZonesWidget::updateFilters_3(const QString &value)
{
	disableSignals(true);
	std::vector<std::string> allCategoryValues;
	m_zoneBuilder->getZoneBank().getCategoryValues(value.toStdString(), allCategoryValues);

	m_ui.categoryValueComboBox_3->clear();
	for(size_t i = 0; i < allCategoryValues.size(); ++i)
		m_ui.categoryValueComboBox_3->addItem(QString(allCategoryValues[i].c_str()));

	disableSignals(false);
	updateListZones();
}

void ListZonesWidget::updateFilters_4(const QString &value)
{
	disableSignals(true);
	std::vector<std::string> allCategoryValues;
	m_zoneBuilder->getZoneBank().getCategoryValues(value.toStdString(), allCategoryValues);

	m_ui.categoryValueComboBox_4->clear();
	for(size_t i = 0; i < allCategoryValues.size(); ++i)
		m_ui.categoryValueComboBox_4->addItem(QString(allCategoryValues[i].c_str()));

	disableSignals(false);
	updateListZones();
}

void ListZonesWidget::updateListZones()
{
	// Execute the filter
	NLLIGO::CZoneBank &zoneBank = m_zoneBuilder->getZoneBank();
	zoneBank.resetSelection ();

	if(m_ui.categoryTypeComboBox_1->currentIndex() > 0 )
		zoneBank.addOrSwitch (m_ui.categoryTypeComboBox_1->currentText().toStdString()
							  , m_ui.categoryValueComboBox_1->currentText().toStdString());

	if(m_ui.categoryTypeComboBox_2->currentIndex() > 0 )
	{
		if (m_ui.logicComboBox_2->currentIndex() == 0) // AND switch wanted
			zoneBank.addAndSwitch(m_ui.categoryTypeComboBox_2->currentText().toStdString()
								  ,m_ui.categoryValueComboBox_2->currentText().toStdString());
		else // OR switch wanted
			zoneBank.addOrSwitch(m_ui.categoryTypeComboBox_2->currentText().toStdString()
								 ,m_ui.categoryValueComboBox_2->currentText().toStdString());
	}

	if(m_ui.categoryTypeComboBox_3->currentIndex() > 0 )
	{
		if (m_ui.logicComboBox_3->currentIndex() == 0) // AND switch wanted
			zoneBank.addAndSwitch(m_ui.categoryTypeComboBox_3->currentText().toStdString()
								  ,m_ui.categoryValueComboBox_3->currentText().toStdString());
		else // OR switch wanted
			zoneBank.addOrSwitch(m_ui.categoryTypeComboBox_3->currentText().toStdString()
								 ,m_ui.categoryValueComboBox_3->currentText().toStdString());
	}

	if(m_ui.categoryTypeComboBox_4->currentIndex() > 0 )
	{
		if (m_ui.logicComboBox_4->currentIndex() == 0) // AND switch wanted
			zoneBank.addAndSwitch(m_ui.categoryTypeComboBox_4->currentText().toStdString()
								  ,m_ui.categoryValueComboBox_4->currentText().toStdString());
		else // OR switch wanted
			zoneBank.addOrSwitch(m_ui.categoryTypeComboBox_4->currentText().toStdString()
								 ,m_ui.categoryValueComboBox_4->currentText().toStdString());
	}

	std::vector<NLLIGO::CZoneBankElement *> currentSelection;
	zoneBank.getSelection (currentSelection);

	m_listSelection.clear();
	m_zoneNameCycle = 0;
	for (size_t i = 0; i < currentSelection.size(); ++i)
		m_listSelection << currentSelection[i]->getName().c_str();

	m_listZonesModel->setListZones(m_listSelection);
}

void ListZonesWidget::disableSignals(bool block)
{
	m_ui.categoryTypeComboBox_1->blockSignals(block);
	m_ui.categoryTypeComboBox_2->blockSignals(block);
	m_ui.categoryTypeComboBox_3->blockSignals(block);
	m_ui.categoryTypeComboBox_4->blockSignals(block);
	m_ui.categoryValueComboBox_1->blockSignals(block);
	m_ui.categoryValueComboBox_2->blockSignals(block);
	m_ui.categoryValueComboBox_3->blockSignals(block);
	m_ui.categoryValueComboBox_4->blockSignals(block);
}

} /* namespace LandscapeEditor */
