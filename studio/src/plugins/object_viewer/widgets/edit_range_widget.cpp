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
#include "edit_range_widget.h"

// NeL includes

// Project includes
#include "ps_wrapper.h"

using namespace NL3D;
using namespace NLMISC;

namespace NLQT
{

const int max_range = 9999;

CEditRangeUIntWidget::CEditRangeUIntWidget(QWidget *parent)
	: QWidget(parent),
	  _Wrapper(NULL),
	  _emit(true)
{
	_ui.setupUi(this);

	_ui.endSpinBox->setMinimum(0);
	_ui.startSpinBox->setMinimum(0);
	_ui.currentSpinBox->setMinimum(0);

	connect(_ui.startSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMinimum(int)));
	connect(_ui.endSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMaximum(int)));
	connect(_ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));

	setValue(0, false);
}

CEditRangeUIntWidget::~CEditRangeUIntWidget()
{
}

void CEditRangeUIntWidget::setWrapper(IPSWrapperUInt *wrapper)
{
	_Wrapper = wrapper;
}

void CEditRangeUIntWidget::setRange(uint32 minValue, uint32 maxValue)
{
	setRangeMin(minValue);
	setRangeMax(maxValue);
}

void CEditRangeUIntWidget::setRangeMin(uint32 minValue)
{
	_ui.startSpinBox->setValue(minValue);
}

void CEditRangeUIntWidget::setRangeMax(uint32 maxValue)
{
	_ui.endSpinBox->setValue(maxValue);
}

void CEditRangeUIntWidget::setValue(uint32 value, bool emit)
{
	if (value > uint32(_ui.endSpinBox->value()))
		_ui.endSpinBox->setValue(value);
	if (value < uint32(_ui.startSpinBox->value()))
		_ui.startSpinBox->setValue(value);

	_emit = emit;
	_ui.horizontalSlider->setValue(value);
	_emit = true;
}

void CEditRangeUIntWidget::enableUpperBound(uint32 upperBound, bool upperBoundExcluded)
{
	if (upperBoundExcluded)
		upperBound--;
	_ui.endSpinBox->setMaximum(upperBound);
	_ui.startSpinBox->setMaximum(upperBound);
}

void CEditRangeUIntWidget::enableLowerBound(uint32 lowerBound, bool lowerBoundExcluded)
{
	if (lowerBoundExcluded)
		lowerBound++;
	_ui.endSpinBox->setMinimum(lowerBound);
	_ui.startSpinBox->setMinimum(lowerBound);
}

void CEditRangeUIntWidget::disableUpperBound(void)
{
	_ui.endSpinBox->setMaximum(max_range);
	_ui.startSpinBox->setMaximum(max_range);
}

void CEditRangeUIntWidget::disableLowerBound(void)
{
	_ui.endSpinBox->setMinimum(0);
	_ui.startSpinBox->setMinimum(0);
}

void CEditRangeUIntWidget::setMaximum(int value)
{
	_ui.horizontalSlider->setMaximum(value);
	_ui.currentSpinBox->setMaximum(value);
}

void CEditRangeUIntWidget::setMinimum(int value)
{
	_ui.horizontalSlider->setMinimum(value);
	_ui.currentSpinBox->setMinimum(value);
}

void CEditRangeUIntWidget::changeSlider(int value)
{
	// NeL wrapper
	if ((_Wrapper != NULL) && (_Wrapper->get() != uint32(value)))
		_Wrapper->setAndUpdateModifiedFlag(value);

	if (_emit)
		Q_EMIT valueChanged(value);
}

void CEditRangeUIntWidget::updateUi()
{
	if (_Wrapper == NULL) return;
	setValue(_Wrapper->get());
}

CEditRangeIntWidget::CEditRangeIntWidget(QWidget *parent)
	: QWidget(parent), _Wrapper(NULL)
{
	_ui.setupUi(this);
	connect(_ui.startSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMinimum(int)));
	connect(_ui.endSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setMaximum(int)));
	connect(_ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));

	setValue(0, false);
}

CEditRangeIntWidget::~CEditRangeIntWidget()
{
}

void CEditRangeIntWidget::setWrapper(IPSWrapperInt *wrapper)
{
	_Wrapper = wrapper;
}

void CEditRangeIntWidget::setRange(sint32 minValue, sint32 maxValue)
{
	setRangeMin(minValue);
	setRangeMax(maxValue);
}

void CEditRangeIntWidget::setRangeMin(sint32 minValue)
{
	_ui.startSpinBox->setValue(minValue);
}

void CEditRangeIntWidget::setRangeMax(sint32 maxValue)
{
	_ui.endSpinBox->setValue(maxValue);
}

void CEditRangeIntWidget::setValue(sint32 value, bool emit)
{
	if (value > sint32(_ui.endSpinBox->value()))
		_ui.endSpinBox->setValue(value);
	if (value < sint32(_ui.startSpinBox->value()))
		_ui.startSpinBox->setValue(value);

	_emit = emit;
	_ui.horizontalSlider->setValue(value);
	_emit = true;
}

void CEditRangeIntWidget::enableUpperBound(sint32 upperBound, bool upperBoundExcluded)
{
	if (upperBoundExcluded)
		upperBound--;
	_ui.endSpinBox->setMaximum(upperBound);
	_ui.startSpinBox->setMaximum(upperBound);
}

void CEditRangeIntWidget::enableLowerBound(sint32 lowerBound, bool lowerBoundExcluded)
{
	if (lowerBoundExcluded)
		lowerBound++;
	_ui.endSpinBox->setMinimum(lowerBound);
	_ui.startSpinBox->setMinimum(lowerBound);
}

void CEditRangeIntWidget::disableUpperBound(void)
{
	_ui.endSpinBox->setMaximum(max_range);
	_ui.startSpinBox->setMaximum(max_range);
}

void CEditRangeIntWidget::disableLowerBound(void)
{
	_ui.endSpinBox->setMinimum(-max_range);
	_ui.startSpinBox->setMinimum(-max_range);
}

void CEditRangeIntWidget::setMaximum(int value)
{
	_ui.horizontalSlider->setMaximum(value);
	_ui.currentSpinBox->setMaximum(value);
}

void CEditRangeIntWidget::setMinimum(int value)
{
	_ui.horizontalSlider->setMinimum(value);
	_ui.currentSpinBox->setMinimum(value);
}

void CEditRangeIntWidget::changeSlider(int value)
{
	// NeL wrapper
	if ((_Wrapper != NULL) && (_Wrapper->get() != sint32(value)))
		_Wrapper->setAndUpdateModifiedFlag(value);

	if (_emit)
		Q_EMIT valueChanged(value);
}

void CEditRangeIntWidget::updateUi()
{
	if (_Wrapper == NULL) return;
	setValue(_Wrapper->get());
}


CEditRangeFloatWidget::CEditRangeFloatWidget(QWidget *parent )
	: QWidget(parent), _Wrapper(NULL), _emit(true)
{
	_ui.setupUi(this);

	connect(_ui.startSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeRange()));
	connect(_ui.endSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeRange()));
	connect(_ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeSlider(int)));
	connect(_ui.currentSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeValue(double)));

	setValue(0.0, false);
}

CEditRangeFloatWidget::~CEditRangeFloatWidget()
{
}

void CEditRangeFloatWidget::setWrapper(IPSWrapperFloat *wrapper)
{
	_Wrapper = wrapper;
}

void CEditRangeFloatWidget::setValue(float value, bool emit)
{
	if (value > _ui.endSpinBox->value())
		_ui.endSpinBox->setValue(value);
	if (value < _ui.startSpinBox->value())
		_ui.startSpinBox->setValue(value);

	float delta = _ui.endSpinBox->value() - _ui.startSpinBox->value();
	int deltaSlider = _ui.horizontalSlider->maximum() - _ui.horizontalSlider->minimum();
	int newValue = floor((deltaSlider / delta) * (value - _ui.startSpinBox->value()));
	_emit = emit;
	_ui.horizontalSlider->setSliderPosition(newValue);
	_emit = true;
}

void CEditRangeFloatWidget::setRange(float minValue, float maxValue)
{
	setRangeMax(maxValue);
	setRangeMin(minValue);
}

void CEditRangeFloatWidget::setRangeMin(float minValue)
{
	_ui.startSpinBox->setValue(minValue);
}

void CEditRangeFloatWidget::setRangeMax(float maxValue)
{
	_ui.endSpinBox->setValue(maxValue);
}

void CEditRangeFloatWidget::enableUpperBound(float upperBound, bool upperBoundExcluded)
{
	if (upperBoundExcluded)
		upperBound -= 0.001f;
	_ui.endSpinBox->setMaximum(upperBound);
	_ui.startSpinBox->setMaximum(upperBound);
	_ui.currentSpinBox->setMaximum(upperBound);
}

void CEditRangeFloatWidget::enableLowerBound(float lowerBound, bool lowerBoundExcluded)
{
	if (lowerBoundExcluded)
		lowerBound += 0.01f;
	_ui.endSpinBox->setMinimum(lowerBound);
	_ui.startSpinBox->setMinimum(lowerBound);
	_ui.currentSpinBox->setMinimum(lowerBound);
}

void CEditRangeFloatWidget::disableUpperBound(void)
{
	_ui.endSpinBox->setMaximum(max_range);
	_ui.startSpinBox->setMaximum(max_range);
}

void CEditRangeFloatWidget::disableLowerBound(void)
{
	_ui.endSpinBox->setMinimum(-max_range);
	_ui.startSpinBox->setMinimum(-max_range);
}

void CEditRangeFloatWidget::changeRange()
{
	if ((_ui.startSpinBox->value() < _ui.currentSpinBox->value()) &&
			(_ui.endSpinBox->value() > _ui.currentSpinBox->value()))
		setValue(_ui.currentSpinBox->value(), false);
}

void CEditRangeFloatWidget::changeSlider(int value)
{
	float delta = _ui.endSpinBox->value() - _ui.startSpinBox->value();
	int deltaSlider = _ui.horizontalSlider->maximum() - _ui.horizontalSlider->minimum();
	float newValue = _ui.startSpinBox->value() + ((delta / deltaSlider) * value);

	_ui.currentSpinBox->blockSignals(true);
	_ui.currentSpinBox->setValue(newValue);
	_ui.currentSpinBox->blockSignals(false);

	// NeL wrapper
	if ((_Wrapper != NULL) && (fabs(newValue - _Wrapper->get()) > 0.0001))
		_Wrapper->setAndUpdateModifiedFlag(newValue);

	if (_emit)
		Q_EMIT valueChanged(newValue);
}

void CEditRangeFloatWidget::changeValue(double value)
{
	setValue(value);
}

void CEditRangeFloatWidget::updateUi()
{
	if (_Wrapper == NULL) return;
	setValue(_Wrapper->get());
}

} /* namespace NLQT */
