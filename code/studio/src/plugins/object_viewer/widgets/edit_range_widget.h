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

#ifndef EDIT_RANGE_WIDGET_H
#define EDIT_RANGE_WIDGET_H

#include "ui_edit_range_float_form.h"
#include "ui_edit_range_uint_form.h"

// Qt includes

// Project includes
#include "ps_wrapper.h"

namespace NLQT
{

/**
@class CEditRangeUIntWidget
@brief The widget provides a horizontal slider and 3 QSpinBox(to set start/end value range and current value from this range.).
@details Slider sets the uint32 value within a specified range (start/end QSpinBox).
The values range can be set through the class methods: setRange() or setRangeMin(), setRangeMax().
Or the user input values in the widgets start/end QSpinBox.
Also the range of start/end values can be restricted through the class methods: enableLowerBound(), enableUpperBound()
this widget can be used by a two ways: Qt Q_SIGNAL/SLOT or wrapper.

1. Using the Qt Q_SIGNAL/SLOT current value can be set by class methods setValue() and
changes in current value(QSlider or QSpinBox) the signal valueChanged() will be emitted.

2. Using wrapper, create wpapper struct, example:
@code
struct CMaxNbParticlesWrapper : public IPSWrapperUInt
{
	NL3D::CPSLocated *Located;
	uint32 get(void) const { return Located->getMaxSize(); }
	void set(const uint32 &v) { Located->setMaxSize(v); }
} _MaxNbParticlesWrapper;
@endcode
to set the current values,it is need to call class methods updateUi();
*/

class CEditRangeUIntWidget: public QWidget
{
	Q_OBJECT

public:
	/// Constructor, sets 0 default current value
	CEditRangeUIntWidget(QWidget *parent = 0);
	~CEditRangeUIntWidget();

	/// Set an interface of a wrapper  to read / write values in the particle system
	/// NB : The 'OwnerNode' field of the wrapper
	void setWrapper(IPSWrapperUInt *wrapper);

	/// Convenience function to set the minimum, and maximum values with a single function call
	void setRange(uint32 minValue, uint32 maxValue);

	/// Set the minimum value that can take range(slider)
	void setRangeMin(uint32 minValue);

	/// Set the maximum value that can take range(slider)
	void setRangeMax(uint32 maxValue);

	/// Enable upper bound use (e.g. value must be < or <= upper bound )
	/// @param upperBound - maximum value of the range
	/// @param upperBoundExcluded - if true then the test is <, otherwise its <=
	void enableUpperBound(uint32 upperBound, bool upperBoundExcluded);

	/// Enable lower bound use (e.g. value must be < or <= lower bound )
	/// @param lowerBound - minimum value of the range
	/// @param lowerBoundExcluded - if true then the test is <, otherwise its <=
	void enableLowerBound(uint32 lowerBound, bool lowerBoundExcluded);

	/// Disable upper bound usage
	void disableUpperBound(void);

	/// Disable lower bound usage
	void disableLowerBound(void);

	/// With changes wrapper to be called for the installation of new range values
	void updateUi();

Q_SIGNALS:
	void valueChanged(uint32 value);

public Q_SLOTS:
	/// Set current value
	/// @param value - current value
	/// @param emit - will emit valueChanged() if the new value is different from the old one and param emit = true
	void setValue(uint32 value, bool emit = true);

private Q_SLOTS:
	void setMaximum(int value);
	void setMinimum(int value);
	void changeSlider(int value);

private:

	IPSWrapperUInt *_Wrapper;
	bool _emit;
	Ui::CEditRangeUIntWidget _ui;
}; /* class CEditRangeUIntWidget */


/**
@class CEditRangeIntWidget
@brief The widget provides a horizontal slider and 3 QSpinBox(to set start/end value range and current value from this range.).
@details Slider sets the sint32 value within a specified range (start/end QSpinBox).
The values range can be set through the class methods: setRange() or setRangeMin(), setRangeMax().
Or the user input values in the widgets start/end QSpinBox.
Also the range of start/end values can be restricted through the class methods: enableLowerBound(), enableUpperBound()
this widget can be used by a two ways: Qt Q_SIGNAL/SLOT or wrapper.

1. Using the Qt Q_SIGNAL/SLOT current value can be set by class methods setValue() and
changes in current value(QSlider or QSpinBox) the signal valueChanged() will be emitted.

2. Using wrapper, create wpapper struct, example:
@code
struct CRadialViscosityWrapper : public IPSWrapperFloat
{
	NL3D::CPSCylindricVortex *V;
	sint32 get(void) const { return V->getRadialViscosity(); }
	void set(const sint32 &value) { V->setRadialViscosity(value); }
} _RadialViscosityWrapper;
@endcode
to set the current values,it is need to call class methods updateUi();
*/

class CEditRangeIntWidget: public QWidget
{
	Q_OBJECT

public:
	/// Constructor, sets 0 default current value
	CEditRangeIntWidget(QWidget *parent = 0);
	~CEditRangeIntWidget();

	/// set an interface of a wrapper  to read / write values in the particle system
	/// NB : The 'OwnerNode' field of the wrapper
	void setWrapper(IPSWrapperInt *wrapper);

	/// Convenience function to set the minimum, and maximum values with a single function call
	void setRange(sint32 minValue, sint32 maxValue);

	/// Set the minimum value that can take range(slider)
	void setRangeMin(sint32 minValue);

	/// Set the maximum value that can take range(slider)
	void setRangeMax(sint32 maxValue);

	/// Enable upper bound use (e.g. value must be < or <= upper bound )
	/// @param upperBound - maximum value of the range
	/// @param upperBoundExcluded - if true then the test is <, otherwise its <=
	void enableUpperBound(sint32 upperBound, bool upperBoundExcluded);

	/// Enable lower bound use (e.g. value must be < or <= lower bound )
	/// @param lowerBound - minimum value of the range
	/// @param lowerBoundExcluded - if true then the test is <, otherwise its <=
	void enableLowerBound(sint32 lowerBound, bool lowerBoundExcluded);

	/// Disable upper bound usage
	void disableUpperBound(void);

	/// Disable lower bound usage
	void disableLowerBound(void);

	/// With changes wrapper to be called for the installation of new range values
	void updateUi();

Q_SIGNALS:
	void valueChanged(sint32 value);

public Q_SLOTS:
	/// Set current value
	/// @param value - current value
	/// @param emit - will emit valueChanged() if the new value is different from the old one and param emit = true
	void setValue(sint32 value, bool emit = true);

private Q_SLOTS:
	void setMaximum(int value);
	void setMinimum(int value);
	void changeSlider(int value);

private:

	IPSWrapperInt *_Wrapper;
	bool _emit;
	Ui::CEditRangeUIntWidget _ui;
}; /* class CEditRangeIntWidget */


/**
@class CEditRangeFloatWidget
@brief The widget provides a horizontal slider and 3 QDoubleSpinBox(to set start/end value range and current value from this range.).
@details Slider sets the float value within a specified range (start/end QDoubleSpinBox).
The values range can be set through the class methods: setRange() or setRangeMin(), setRangeMax().
Or the user input values in the widgets start/end QDoubleSpinBox.
Also the range of start/end values can be restricted through the class methods: enableLowerBound(), enableUpperBound()
this widget can be used by a two ways: Qt Q_SIGNAL/SLOT or wrapper.

1. Using the Qt Q_SIGNAL/SLOT current value can be set by class methods setValue() and
changes in current value(only QSlider) the signal valueChanged()will be emitted.

2. Using wrapper, create wpapper struct, example:
@code
struct CTangentialViscosityWrapper : public IPSWrapperFloat
{
	NL3D::CPSCylindricVortex *V;
	float get(void) const { return V->getTangentialViscosity(); }
	void set(const float &value) { V->setTangentialViscosity(value); }
} _TangentialViscosityWrapper;
@endcode
to set the current values,it is need to call class methods updateUi();
*/

class CEditRangeFloatWidget: public QWidget
{
	Q_OBJECT
public:
	/// Constructor, sets 0 default current value
	CEditRangeFloatWidget(QWidget *parent = 0);
	~CEditRangeFloatWidget();

	/// Set an interface of a wrapper  to read / write values in the particle system
	/// NB : The 'OwnerNode' field of the wrapper
	void setWrapper(IPSWrapperFloat *wrapper);

	/// Convenience function to set the minimum, and maximum values with a single function call
	void setRange(float minValue, float maxValue);

	/// Set the minimum value that can take range(slider)
	void setRangeMin(float minValue);

	/// Set the maximum value that can take range(slider)
	void setRangeMax(float maxValue);

	/// Enable upper bound use (e.g. value must be < or <= upper bound )
	/// @param upperBound - maximum value of the range
	/// @param upperBoundExcluded - if true then the test is <, otherwise its <=
	void enableUpperBound(float upperBound, bool upperBoundExcluded);

	/// Enable lower bound use (e.g. value must be < or <= lower bound )
	/// @param lowerBound - minimum value of the range
	/// @param lowerBoundExcluded - if true then the test is <, otherwise its <=
	void enableLowerBound(float lowerBound, bool lowerBoundExcluded);

	/// Disable upper bound usage
	void disableUpperBound(void);

	/// Disable lower bound usage
	void disableLowerBound(void);

	/// With changes wrapper to be called for the installation of new range values
	void updateUi();

Q_SIGNALS:
	void valueChanged(float value);

public Q_SLOTS:
	/// Set current value
	/// @param value - current value
	/// @param emit - will emit valueChanged() if the new value is different from the old one and param emit = true
	void setValue(float value, bool emit = true);

private Q_SLOTS:
	void changeRange();
	void changeSlider(int value);
	void changeValue(double value);

private:

	IPSWrapperFloat *_Wrapper;
	bool _emit;
	Ui::CEditRangeFloatWidget _ui;
}; /* class CEditRangeFloatWidget */

} /* namespace NLQT */

#endif // EDIT_RANGE_WIDGET_H
