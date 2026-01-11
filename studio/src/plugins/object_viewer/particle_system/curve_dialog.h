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

#ifndef CURVE_DIALOG_H
#define CURVE_DIALOG_H

#include <nel/misc/types_nl.h>
#include "ui_curve_form.h"

// NeL includes
#include <nel/3d/ps_float.h>

// Qt includes
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

// Project includes
#include "hoverpoints.h"
#include "ps_wrapper.h"

namespace NLQT
{
/**
@class CurveEditDialog
@brief Dialogue editing graphics curve.
@details Adding / removing control points, scaling and moving graphics curve.
The choice of the interpolation algorithm.
*/
class CurveEditDialog : public QDialog
{
	Q_OBJECT

public:
	CurveEditDialog(NL3D::CPSFloatCurveFunctor *curve, CWorkspaceNode *ownerNode, QWidget *parent = 0);
	~CurveEditDialog();

private Q_SLOTS:
	void setSmoothing(bool state);
	void setDisplayInterpolation(bool state);
	void curveChanged(const QPolygonF &points);
	void setScale(int value);
	void setPosition(int value);
	void setCenter();
	void setLastTiFirst();
	void setFirstToLast();
	void setNumSamples(uint32 value);

protected:
	void buildPoints();

	float _scale;
	float _pos;

	CWorkspaceNode *_Node;

	/// the scheme being edited
	NL3D::CPSFloatCurveFunctor *_Curve;

	HoverPoints *_hoverPoints;
	Ui::CurveEditDialog _ui;
}; /* CurveEditDialog */

} /* namespace NLQT */

#endif // CURVE_DIALOG_H
