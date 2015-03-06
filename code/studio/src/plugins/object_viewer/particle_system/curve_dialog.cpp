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
#include "curve_dialog.h"

// Project includes
#include "particle_node.h"

namespace NLQT
{

CurveEditDialog::CurveEditDialog(NL3D::CPSFloatCurveFunctor *curve, CWorkspaceNode *ownerNode, QWidget *parent)
	: QDialog(parent),
	  _scale(1.0),
	  _pos(0.0),
	  _Node(ownerNode),
	  _Curve(curve)
{
	_ui.setupUi(this);

	show();
	_hoverPoints = new HoverPoints(_ui.curveWidget, HoverPoints::CircleShape);

	if (_Curve->hasSmoothing())
		_hoverPoints->setConnectionType(HoverPoints::CurveConnection);
	else
		_hoverPoints->setConnectionType(HoverPoints::LineConnection);

	_ui.smoothingCheckBox->setChecked(_Curve->hasSmoothing());

	buildPoints();

	_hoverPoints->setSortType(HoverPoints::XSort);
	_hoverPoints->setShapePen(QPen(QColor(0, 0, 100, 127), 1));
	_hoverPoints->setShapeBrush(QBrush(QColor(0, 0, 200, 127)));
	_hoverPoints->setConnectionPen(QPen(QColor(0, 0, 0, 127), 1));

	// FLOAT_CURVE_NB_SAMPLE
	_ui.numSamplesWidget->setRange(1, 512);
	_ui.numSamplesWidget->enableLowerBound(1, true);
	_ui.numSamplesWidget->setValue(_Curve->getNumSamples());

	connect(_ui.smoothingCheckBox, SIGNAL(toggled(bool)), this, SLOT(setSmoothing(bool)));
	connect(_hoverPoints, SIGNAL(pointsChanged(QPolygonF)), this, SLOT(curveChanged(QPolygonF)));
	connect(_ui.posVerticalSlider, SIGNAL(valueChanged(int)), this, SLOT(setPosition(int)));
	connect(_ui.sizeVerticalSlider, SIGNAL(valueChanged(int)), this, SLOT(setScale(int)));
	connect(_ui.firstLastPushButton, SIGNAL(clicked()), this, SLOT(setLastTiFirst()));
	connect(_ui.lastFirstPushButton, SIGNAL(clicked()), this, SLOT(setFirstToLast()));
	connect(_ui.centerToolButton, SIGNAL(clicked()), this, SLOT(setCenter()));
	connect(_ui.numSamplesWidget, SIGNAL(valueChanged(uint32)), this, SLOT(setNumSamples(uint32)));
	//setCenter();
}

CurveEditDialog::~CurveEditDialog()
{
}

void CurveEditDialog::setSmoothing(bool state)
{
	if (state)
		_hoverPoints->setConnectionType(HoverPoints::CurveConnection);
	else
		_hoverPoints->setConnectionType(HoverPoints::LineConnection);

	_Curve->enableSmoothing(state);

	_ui.curveWidget->update();
}

void CurveEditDialog::setDisplayInterpolation(bool state)
{
}

void CurveEditDialog::curveChanged(const QPolygonF &points)
{
	if (_Curve->getNumCtrlPoints() > uint(points.size()))
		_Curve->removeCtrlPoint(0);
	else if (_Curve->getNumCtrlPoints() < uint(points.size()))
		_Curve->addControlPoint(NL3D::CPSFloatCurveFunctor::CCtrlPoint(1, 0.5f));
	for (int i = 0; i < points.size(); ++i)
		_Curve->setCtrlPoint(uint(i), NL3D::CPSFloatCurveFunctor::CCtrlPoint(points.at(i).x() / _ui.curveWidget->width(),
							 (_ui.curveWidget->height() -  points.at(i).y() + _pos) / (_ui.curveWidget->height() * _scale)));
}

void CurveEditDialog::setScale(int value)
{
	_scale = value / float(_ui.sizeVerticalSlider->maximum() / 2);
	buildPoints();
	_ui.curveWidget->update();
}

void CurveEditDialog::setPosition(int value)
{
	_pos = (value - (_ui.posVerticalSlider->maximum() / 2));
	buildPoints();
	_ui.curveWidget->update();
}

void CurveEditDialog::setCenter()
{
	float maxValue = _Curve->getMaxValue();
	float minValue = _Curve->getMinValue();
	float fScale = (maxValue == minValue) ? 1.f : 1.f / (maxValue - minValue);
	int iScale = int(fScale * _ui.sizeVerticalSlider->maximum() / 2);
	_ui.sizeVerticalSlider->setValue(iScale);
	int pos = _ui.curveWidget->height() - (_scale * maxValue * _ui.curveWidget->height());
	_ui.posVerticalSlider->setValue((_ui.posVerticalSlider->maximum() / 2) - pos);
}

void CurveEditDialog::setLastTiFirst()
{
	NL3D::CPSFloatCurveFunctor::CCtrlPoint pt = _Curve->getControlPoint(_Curve->getNumCtrlPoints() - 1);
	pt.Date = _Curve->getControlPoint(0).Date;
	_Curve->setCtrlPoint(0, pt);
	buildPoints();
	_ui.curveWidget->update();
}

void CurveEditDialog::setFirstToLast()
{
	NL3D::CPSFloatCurveFunctor::CCtrlPoint pt = _Curve->getControlPoint(0);
	pt.Date = _Curve->getControlPoint(_Curve->getNumCtrlPoints() - 1).Date;
	_Curve->setCtrlPoint(_Curve->getNumCtrlPoints() - 1, pt);
	buildPoints();
	_ui.curveWidget->update();
}

void CurveEditDialog::setNumSamples(uint32 value)
{
	_Curve->setNumSamples(value);
}

void CurveEditDialog::buildPoints()
{
	QPolygonF points;
	for (uint i = 0; i < _Curve->getNumCtrlPoints(); ++i)
		points << QPointF((_Curve->getControlPoint(i).Date * _ui.curveWidget->width()),
						  _pos + _ui.curveWidget->height() - (_scale * _Curve->getControlPoint(i).Value * _ui.curveWidget->height()));

	_hoverPoints->setPoints(points);
	//_hoverPoints->setPointLock(0, HoverPoints::LockToLeft);
	//_hoverPoints->setPointLock(points.size() - 1, HoverPoints::LockToRight);

}

} /* namespace NLQT */