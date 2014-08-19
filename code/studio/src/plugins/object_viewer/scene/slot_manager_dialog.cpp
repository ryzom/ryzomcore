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
#include "slot_manager_dialog.h"

// Qt includes
#include <QtGui/QInputDialog>

// NeL includes

// Project includes
#include "modules.h"

namespace NLQT
{

CSlotGroupBox::CSlotGroupBox(QWidget *parent)
	: QGroupBox(parent),
	  _animName("empty"),
	  _skelName("empty"),
	  _numSlot(0)
{
	_ui.setupUi(this);

	connect(_ui.animPushButton, SIGNAL(clicked()), this, SLOT(selectAnim()));
	connect(_ui.skelPushButton, SIGNAL(clicked()), this, SLOT(selectSkel()));
	connect(_ui.alignBlendPushButton, SIGNAL(clicked()), this, SLOT(alignAblend()));
	connect(_ui.clampRadioButton, SIGNAL(clicked()), this, SLOT(saveSlotInfo()));
	connect(_ui.repeatRadioButton, SIGNAL(clicked()), this, SLOT(saveSlotInfo()));
	connect(_ui.disableRadioButton, SIGNAL(clicked()), this, SLOT(saveSlotInfo()));
	connect(_ui.invSkelWeightCheckBox, SIGNAL(clicked()), this, SLOT(saveSlotInfo()));
	connect(_ui.endBlendSpinBox, SIGNAL(valueChanged(int)), this, SLOT(saveSlotInfo()));
	connect(_ui.endFrameSpinBox, SIGNAL(valueChanged(int)), this, SLOT(saveSlotInfo()));
	connect(_ui.offsetSpinBox, SIGNAL(valueChanged(int)), this, SLOT(saveSlotInfo()));
	connect(_ui.smoothSpinBox, SIGNAL(valueChanged(int)), this, SLOT(saveSlotInfo()));
	connect(_ui.speedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(saveSlotInfo()));
	connect(_ui.startBlendSpinBox, SIGNAL(valueChanged(int)), this, SLOT(saveSlotInfo()));
	connect(_ui.startFrameSpinBox, SIGNAL(valueChanged(int)), this, SLOT(saveSlotInfo()));
	connect(_ui.enableCheckBox, SIGNAL(clicked()), this, SLOT(saveSlotInfo()));
}

CSlotGroupBox::~CSlotGroupBox()
{
}

void CSlotGroupBox::updateUi()
{
	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
	{
		resetUi();
		return;
	}
	CEntity &entity = Modules::objView().getEntity(curObj);
	CSlotInfo slotInfo = entity.getSlotInfo(_numSlot);

	_ui.offsetSpinBox->setValue(slotInfo.Offset * Modules::mainWin().getFrameRate());
	_ui.startBlendSpinBox->setValue(int(slotInfo.StartBlend));
	_ui.endBlendSpinBox->setValue(int(slotInfo.EndBlend));
	_ui.startFrameSpinBox->setValue(int(slotInfo.StartTime * Modules::mainWin().getFrameRate()));
	_ui.endFrameSpinBox->setValue(int(slotInfo.EndTime * Modules::mainWin().getFrameRate()));
	_ui.speedDoubleSpinBox->setValue(slotInfo.SpeedFactor);
	_ui.smoothSpinBox->setValue(int(slotInfo.Smoothness));

	if (slotInfo.SkeletonInverted)
		_ui.invSkelWeightCheckBox->setCheckState(Qt::Checked);
	else
		_ui.invSkelWeightCheckBox->setCheckState(Qt::Unchecked);

	if (slotInfo.Enable)
		_ui.enableCheckBox->setCheckState(Qt::Checked);
	else
		_ui.enableCheckBox->setCheckState(Qt::Unchecked);

	switch (slotInfo.ClampMode)
	{
	case 0:
		_ui.clampRadioButton->setChecked(true);
		break;
	case 1:
		_ui.repeatRadioButton->setChecked(true);
		break;
	case 2:
		_ui.disableRadioButton->setChecked(true);
		break;
	}

	_animName = QString(slotInfo.Animation.c_str());
	_skelName = QString(slotInfo.Skeleton.c_str());
	QString title = tr("Slot %1 : ").arg(_numSlot) + _animName + " : " + _skelName;
	this->setTitle(title);
}

void CSlotGroupBox::saveSlotInfo()
{
	CSlotInfo slotInfo;
	slotInfo.Animation = _animName.toUtf8().constData();
	slotInfo.Skeleton = _skelName.toUtf8().constData();
	slotInfo.EndBlend = _ui.endBlendSpinBox->value();
	slotInfo.EndTime = float(_ui.endFrameSpinBox->value()) / Modules::mainWin().getFrameRate();
	slotInfo.Offset = float(_ui.offsetSpinBox->value()) / Modules::mainWin().getFrameRate();
	slotInfo.Smoothness = _ui.smoothSpinBox->value();
	slotInfo.SpeedFactor = _ui.speedDoubleSpinBox->value();
	slotInfo.StartBlend = _ui.startBlendSpinBox->value();
	slotInfo.StartTime = float(_ui.startFrameSpinBox->value()) / Modules::mainWin().getFrameRate();

	if (_ui.invSkelWeightCheckBox->checkState() == Qt::Checked)
		slotInfo.SkeletonInverted = true;
	else
		slotInfo.SkeletonInverted = false;

	if (_ui.enableCheckBox->checkState() == Qt::Checked)
		slotInfo.Enable = true;
	else
		slotInfo.Enable = false;

	if (_ui.clampRadioButton->isChecked())
		slotInfo.ClampMode = 0;
	else if (_ui.repeatRadioButton->isChecked())
		slotInfo.ClampMode = 1;
	else
		slotInfo.ClampMode = 2;

	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;
	CEntity	&entity = Modules::objView().getEntity(curObj);
	entity.setSlotInfo(_numSlot, slotInfo);
}

void CSlotGroupBox::resetUi()
{
	_ui.offsetSpinBox->setValue(0);
	_ui.startBlendSpinBox->setValue(0);
	_ui.endBlendSpinBox->setValue(0);
	_ui.startFrameSpinBox->setValue(0);
	_ui.endFrameSpinBox->setValue(0);
	_ui.speedDoubleSpinBox->setValue(0);
	_ui.smoothSpinBox->setValue(0);
	_ui.disableRadioButton->setChecked(true);
	_ui.invSkelWeightCheckBox->setCheckState(Qt::Unchecked);
	_ui.enableCheckBox->setCheckState(Qt::Unchecked);
}

void CSlotGroupBox::selectAnim()
{
	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;
	CEntity	&entity = Modules::objView().getEntity(curObj);
	std::vector<std::string>& animationList = entity.getAnimationList();

	if (animationList.empty()) return;

	QStringList items;
	items << tr("empty");
	for(size_t i = 0; i < animationList.size(); ++i)
		items << QString(animationList[i].c_str());

	bool ok;
	QString item = QInputDialog::getItem(this, tr("Select your anim"),
										 tr("Animation:"), items, 0, false, &ok);
	if (ok)
	{
		_animName = item;
		QString title = tr("Slot %1 : ").arg(_numSlot) + _animName + " : " + _skelName;
		this->setTitle(title);
		_ui.endFrameSpinBox->setValue(int(entity.getAnimLength(_animName.toUtf8().constData()) * Modules::mainWin().getFrameRate()));
		saveSlotInfo();
	}
}

void CSlotGroupBox::selectSkel()
{
	std::string curObj = Modules::objView().getCurrentObject();
	if (curObj.empty())
		return;
	CEntity	&entity = Modules::objView().getEntity(curObj);
	std::vector<std::string>& swtList = entity.getSWTList();

	if (swtList.empty()) return;

	QStringList items;
	items << tr("empty");
	for(size_t i = 0; i < swtList.size(); ++i)
		items << QString(swtList[i].c_str());

	bool ok;
	QString item = QInputDialog::getItem(this, tr("Select your skel"),
										 tr("Skeleton weight template:"), items, 0, false, &ok);
	if (ok)
	{
		_skelName = item;
		QString title = tr("Slot %1 : ").arg(_numSlot) + _animName + " : " + _skelName;
		this->setTitle(title);
		saveSlotInfo();
	}
}

void CSlotGroupBox::alignAblend()
{
	float deltaTime = _ui.endFrameSpinBox->value() - _ui.startFrameSpinBox->value();
	_ui.startFrameSpinBox->setValue(_ui.offsetSpinBox->value());
	_ui.endFrameSpinBox->setValue(int(float(_ui.offsetSpinBox->value()) + deltaTime / _ui.speedDoubleSpinBox->value()));
}

CSlotManagerDialog::CSlotManagerDialog(QWidget *parent)
	: QDockWidget(parent)
{
	setObjectName(QString::fromUtf8("CSlotManagerDialog"));
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/images/mixer.png"), QSize(), QIcon::Normal, QIcon::Off);
	setWindowIcon(icon);
	setMinimumSize(QSize(310, 100));
	_dockWidgetContents = new QWidget();
	_gridLayout = new QGridLayout(_dockWidgetContents);
	_scrollArea = new QScrollArea(_dockWidgetContents);
	_scrollArea->setWidgetResizable(true);
	_scrollAreaWidgetContents = new QWidget();

	_slotGridLayout = new QGridLayout(_scrollAreaWidgetContents);

	_tabWidget = new QTabWidget(_scrollAreaWidgetContents);
	_tabWidget->setObjectName(QString::fromUtf8("_tabWidget"));
	_tabWidget->setTabPosition(QTabWidget::East);

	for (int i = 0; i < NL3D::CChannelMixer::NumAnimationSlot; ++i)
	{
		_tabs[i] = new QWidget();
		_tabs[i]->setObjectName(QString::fromUtf8("_tab%1").arg(i));
		_gridLayouts[i] = new QGridLayout(_tabs[i]);
		_gridLayouts[i]->setObjectName(QString::fromUtf8("_gridLayouts%1").arg(i));

		_slotGroupBoxs[i] = new CSlotGroupBox(_tabs[i]);
		_slotGroupBoxs[i]->setTitle(tr("Slot %1 : empty : empty").arg(i));
		_slotGroupBoxs[i]->_numSlot = i;
		_gridLayouts[i]->addWidget(_slotGroupBoxs[i], 0, 0, 1, 1);
		_verticalSpacers[i] = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

		_gridLayouts[i]->addItem(_verticalSpacers[i], 1, 0, 1, 1);

		_tabWidget->addTab(_tabs[i], QString());
		_tabWidget->setTabText(i, tr("Slot %1").arg(i));


	}

	_slotGridLayout->addWidget(_tabWidget);

	_scrollArea->setWidget(_scrollAreaWidgetContents);

	_gridLayout->addWidget(_scrollArea, 0, 0, 1, 1);

	setWidget(_dockWidgetContents);
	setWindowTitle(tr("Slot manager"));
}

CSlotManagerDialog::~CSlotManagerDialog()
{
}

void CSlotManagerDialog::updateUiSlots()
{
	for (int i = 0; i < NL3D::CChannelMixer::NumAnimationSlot; ++i)
	{
		_slotGroupBoxs[i]->updateUi();
	}
}

} /* namespace NLQT */