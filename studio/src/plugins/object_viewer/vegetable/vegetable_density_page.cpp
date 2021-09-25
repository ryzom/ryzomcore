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
#include "vegetable_density_page.h"

// Qt includes
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

// NeL includes
#include <nel/3d/vegetable.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>

// Projects include
#include "modules.h"
#include "vegetable_dialog.h"

namespace NLQT
{

CVegetableDensityPage::CVegetableDensityPage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	// Init Density widget.
	_ui.densityGroupBox->setDefaultRangeAbs(NL_VEGETABLE_DENSITY_ABS_RANGE_MIN, NL_VEGETABLE_DENSITY_ABS_RANGE_MAX);
	_ui.densityGroupBox->setDefaultRangeRand(NL_VEGETABLE_DENSITY_RAND_RANGE_MIN, NL_VEGETABLE_DENSITY_RAND_RANGE_MAX);
	_ui.densityGroupBox->setDefaultRangeFreq(NL_VEGETABLE_FREQ_RANGE_MIN, NL_VEGETABLE_FREQ_RANGE_MAX);

	// Init MaxDensity widget.
	_ui.maxDensityWidget->setRange(0, NL_VEGETABLE_EDIT_DEFAULT_MAX_DENSITY);
	_ui.maxDensityWidget->enableLowerBound(0, false);

	connect(_ui.browseShapePushButton, SIGNAL(clicked()), this, SLOT(browseShapeVeget()));
	connect(_ui.distanceSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setDistanceOfCreat(int)));
	connect(_ui.densityGroupBox, SIGNAL(noiseValueChanged(NLMISC::CNoiseValue)), this, SLOT(setDensity(NLMISC::CNoiseValue)));
	connect(_ui.maxDensityCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEnabledMaxDensity(bool)));
	connect(_ui.maxDensityWidget, SIGNAL(valueChanged(float)), this, SLOT(setMaxDensity(float)));
	connect(_ui.floorRadioButton, SIGNAL(clicked()), this, SLOT(updateAngleMode()));
	connect(_ui.wallRadioButton, SIGNAL(clicked()), this, SLOT(updateAngleMode()));
	connect(_ui.ceilingRadioButton, SIGNAL(clicked()), this, SLOT(updateAngleMode()));
	connect(_ui.angleMinHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setAngleMinSlider(int)));
	connect(_ui.angleMaxHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setAngleMaxSlider(int)));

	setEnabled(false);
}

CVegetableDensityPage::~CVegetableDensityPage()
{
}

void CVegetableDensityPage::setVegetableToEdit(NL3D::CVegetable *vegetable)
{
	_Vegetable = vegetable;

	if(_Vegetable)
	{
		setEnabled(true);

		// ShapeName
		_ui.meshLineEdit->setText(QString(_Vegetable->ShapeName.c_str()));

		// Creation Distance.
		// normalize creation distance for this editor.
		_Vegetable->DistType = std::min(uint(_Vegetable->DistType), uint(_ui.distanceSpinBox->maximum()) );

		// set Creation Distance.
		_ui.distanceSpinBox->setValue(_Vegetable->DistType + 1);

		// set density widget
		_ui.densityGroupBox->setNoiseValue(_Vegetable->Density, false);

		// init MaxDensity
		if(_Vegetable->MaxDensity == -1)
		{
			// Disable the checkBox and the slider.
			_ui.maxDensityCheckBox->setChecked(false);

			_PrecMaxDensityValue = NL_VEGETABLE_EDIT_DEFAULT_MAX_DENSITY;
		}
		else
		{
			// Enable the checkBox and the slider
			_PrecMaxDensityValue = _Vegetable->MaxDensity;
			_ui.maxDensityCheckBox->setChecked(true);
			_ui.maxDensityWidget->setValue(_Vegetable->MaxDensity, false);
		}

		// init AngleSetup.
		// ----------
		NL3D::CVegetable::TAngleType angType = _Vegetable->getAngleType();

		// enable only the one of interest.
		_ui.floorRadioButton->blockSignals(true);
		_ui.wallRadioButton->blockSignals(true);
		_ui.ceilingRadioButton->blockSignals(true);
		switch(angType)
		{
		case NL3D::CVegetable::AngleGround:
			updateAngleMin();
			_ui.floorRadioButton->setChecked(true);
			break;
		case NL3D::CVegetable::AngleWall:
			updateAngleMin();
			updateAngleMax();
			_ui.wallRadioButton->setChecked(true);
			break;
		case NL3D::CVegetable::AngleCeiling:
			updateAngleMax();
			_ui.ceilingRadioButton->setChecked(true);
			break;
		}
		_ui.floorRadioButton->blockSignals(false);
		_ui.wallRadioButton->blockSignals(false);
		_ui.ceilingRadioButton->blockSignals(false);
	}
	else
		setEnabled(false);
}

void CVegetableDensityPage::browseShapeVeget()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Vegetable Shape"),
					   ".",
					   tr("veget files (*.veget);;"));
	if (!fileName.isEmpty())
	{
		// Add search path for the .veget
		NLMISC::CPath::addSearchPath (NLMISC::CFile::getPath(fileName.toUtf8().constData()));

		try
		{
			// update shapeName and view
			_Vegetable->ShapeName = NLMISC::CFile::getFilename(fileName.toUtf8().constData());
			_ui.meshLineEdit->setText(QString::fromUtf8(_Vegetable->ShapeName.c_str()));

			// update the name in the list-box
			Q_EMIT vegetNameChanged();

			// update 3D view
			Modules::veget().refreshVegetableDisplay();
		}
		catch (NLMISC::EPathNotFound &ep)
		{
			QMessageBox::critical(this, "Can't open file", QString(ep.what()), QMessageBox::Ok);
		}
	}
}

void CVegetableDensityPage::setDistanceOfCreat(int value)
{
	// Get the DistType, and just copy to vegetable.
	_Vegetable->DistType = value - 1;

	// Since used to display name in selection listBox, must update the name
	Q_EMIT vegetNameChanged();

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableDensityPage::setEnabledMaxDensity(bool state)
{
	if(state)
	{
		// check, restore maxDensity
		_Vegetable->MaxDensity = _PrecMaxDensityValue;
		// enable dlg.
		_ui.maxDensityWidget->setValue(_PrecMaxDensityValue);
		_ui.maxDensityWidget->setEnabled(true);
	}
	else
	{
		// uncheck, bkup maxDenstiy
		_PrecMaxDensityValue = _Vegetable->MaxDensity;

		_ui.maxDensityWidget->setEnabled(false);

		// and setup vegetable (disable MaxDensity).
		_Vegetable->MaxDensity= -1;
	}

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableDensityPage::updateAngleMode()
{
	if (_ui.floorRadioButton->isChecked())
	{
		_Vegetable->setAngleGround(0);
		updateAngleMin();
	}
	if (_ui.wallRadioButton->isChecked())
	{
		_Vegetable->setAngleWall(-1, 1);
		updateAngleMin();
		updateAngleMax();
	}
	if (_ui.ceilingRadioButton->isChecked())
	{
		_Vegetable->setAngleCeiling(0);
		updateAngleMax();
	}
	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableDensityPage::setDensity(const NLMISC::CNoiseValue &value)
{
	_Vegetable->Density = value;

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableDensityPage::setMaxDensity(float value)
{
	_Vegetable->MaxDensity = value;

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableDensityPage::setAngleMinSlider(int pos)
{
	float angle = 180 * float(pos) / float(abs(_ui.angleMinHorizontalSlider->maximum() - _ui.angleMinHorizontalSlider->minimum()));
	NLMISC::clamp(angle, -90, 90);
	// make a sinus, because 90 => 1, and -90 =>-1
	float cosAngleMin = float(sin(angle * NLMISC::Pi / 180.f));

	// setup vegetable.
	if(_Vegetable->getAngleType()== NL3D::CVegetable::AngleWall)
		_Vegetable->setAngleWall(cosAngleMin, _Vegetable->getCosAngleMax());
	else
		_Vegetable->setAngleGround(cosAngleMin);

	// update view
	_ui.angleMinDoubleSpinBox->setValue(angle);

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableDensityPage::setAngleMaxSlider(int pos)
{
	float angle = 180 * float(pos) / float(abs(_ui.angleMaxHorizontalSlider->maximum() - _ui.angleMaxHorizontalSlider->minimum()));
	NLMISC::clamp(angle, -90, 90);
	// make a sinus, because 90 => 1, and -90 =>-1
	float cosAngleMax = float(sin(angle * NLMISC::Pi / 180.f));

	// setup vegetable.
	if(_Vegetable->getAngleType() == NL3D::CVegetable::AngleWall)
		_Vegetable->setAngleWall(_Vegetable->getCosAngleMin(), cosAngleMax);
	else
		_Vegetable->setAngleCeiling(cosAngleMax);

	// update view
	_ui.angleMaxDoubleSpinBox->setValue(angle);

	// update 3D view
	Modules::veget().refreshVegetableDisplay();
}

void CVegetableDensityPage::updateAngleMin()
{
	double angle = _Vegetable->getCosAngleMin();
	NLMISC::clamp(angle, -1, 1);
	angle = asin(angle) * (180.f / NLMISC::Pi);
	sint pos = sint(angle * abs(_ui.angleMinHorizontalSlider->maximum() - _ui.angleMinHorizontalSlider->minimum()) / 180);
	NLMISC::clamp(pos, _ui.angleMinHorizontalSlider->minimum(), _ui.angleMinHorizontalSlider->maximum());
	_ui.angleMinHorizontalSlider->setSliderPosition(pos);
}

void CVegetableDensityPage::updateAngleMax()
{
	double angle = _Vegetable->getCosAngleMax();
	NLMISC::clamp(angle, -1, 1);
	angle = asin(angle) * (180.f / NLMISC::Pi);
	sint pos = sint(angle * abs(_ui.angleMaxHorizontalSlider->maximum() - _ui.angleMaxHorizontalSlider->minimum()) / 180);
	NLMISC::clamp(pos, _ui.angleMaxHorizontalSlider->minimum(), _ui.angleMaxHorizontalSlider->maximum());
	_ui.angleMaxHorizontalSlider->setSliderPosition(pos);
}

} /* namespace NLQT */