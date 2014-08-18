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
#include "basic_edit_widget.h"

// Qt includes
#include <QtGui/QPainter>

// NeL includes
#include "nel/misc/matrix.h"
#include "nel/misc/vector.h"

namespace NLQT
{


// build an euler matrix
NLMISC::CMatrix  BuildEulerMatrix(float psi, float theta, float phi)
{
	float ca = cosf(psi), sa = sinf(psi)
						, cb = cosf(theta)
						, sb = sinf(theta)
						, cc = cosf(phi)
						, sc = sinf(phi);
	NLMISC::CMatrix m;
	m.identity();
	m.setRot(NLMISC::CVector(ca * cb * cc - sa * sc, -cc * sa - ca * cb *sc, ca * sb)
			 ,NLMISC::CVector(cb * cc * sa + ca * sc, ca * cc - cb * sa * sc, sa *sb)
			 ,NLMISC::CVector(-cc * sb, sb * sc, cb)
			);
	return m;
}

// get back the euler angles from a matrix
NLMISC::CVector GetEulerAngles(const NLMISC::CMatrix &mat)
{
	float m[3][3];
	// we got cos theta = m33
	NLMISC::CVector v[3];
	mat.getRot(v[0], v[1], v[2]);
	for (uint l = 0; l < 3; ++l)
	{
		m[0][l] = v[l].x;
		m[1][l] = v[l].y;
		m[2][l] = v[l].z;
	}

	// there are eight triplet that may satisfy the equation
	// we compute them all, and test them against the matrix

	float b0, b1, a0, a1, a2, a3, c0, c1, c2, c3;
	b0 = acosf(m[2][2]);
	b1 = (float) NLMISC::Pi - b0;
	float sb0 = sinf(b0), sb1 = sinf(b1);
	if (fabsf(sb0) > 10E-6)
	{
		a0 = m[2][0] / sb0;
		c0 = m[1][2] / sb0;
	}
	else
	{
		a0 = c0 = 1.f;
	}
	if (fabs(sb1) > 10E-6)
	{
		a1 = m[2][0] / sb1;
		c1 = m[1][2] / sb1;
	}
	else
	{
		a1 = c1 = 1.f;
	}


	a2 = (float) NLMISC::Pi - a0;
	a3 = (float) NLMISC::Pi - a1;


	c2 = (float) NLMISC::Pi - c0;
	c3 = (float) NLMISC::Pi - c1;

	NLMISC::CVector sol[] =
	{
		NLMISC::CVector(b0, a0, c0)
		,NLMISC::CVector(b0, a2, c0)
		,NLMISC::CVector(b0, a0, c2)
		,NLMISC::CVector(b0, a2, c2)
		,NLMISC::CVector(b1, a1, c1)
		,NLMISC::CVector(b1, a3, c1)
		,NLMISC::CVector(b1, a1, c3)
		,NLMISC::CVector(b1, a3, c3)
	};

	// now we take the triplet that fit best the 6 other equations

	float bestGap = 0.f;
	uint bestIndex;

	for (uint k = 0; k < 8; ++k)
	{
		float ca = cosf(sol[k].x), sa = sinf(sol[k].x)
								, cb = cosf(sol[k].y)
								, sb = sinf(sol[k].y)
								, cc = cosf(sol[k].z)
								, sc = sinf(sol[k].z);

		float gap = fabsf(m[0][0] - ca * cb * cc + sa * sc);
		gap += fabsf(m[1][0] + cc * sa + ca * cb *sc);
		gap += fabsf(m[0][1] - cb * cc * sa - ca * sc);
		gap += fabsf(m[0][1] - cb * cc * sa - ca * sc);
		gap += fabsf(m[1][1] - ca * cc + cb * sa * sc);
		gap += fabsf(m[2][1] - sb *ca);
		gap += fabsf(m[0][2] + cc * sb);

		if (k == 0 || gap < bestGap)
		{
			bestGap = gap;
			bestIndex  = k;

		}
	}
	return sol[bestIndex];
}

CBasicEditWidget::CBasicEditWidget(QWidget *parent)
	: QWidget(parent), 
	_Wrapper(NULL)
{
	_ui.setupUi(this);
	_ui.graphicsWidget->installEventFilter(this);

	connect(_ui.psiSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateGraphics()));
	connect(_ui.thetaSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateGraphics()));
	connect(_ui.phiSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateGraphics()));
}

CBasicEditWidget::~CBasicEditWidget()
{
}

void CBasicEditWidget::setWrapper(IPSWrapper<NL3D::CPlaneBasis> *wrapper)
{
	_Wrapper = wrapper;
}

void CBasicEditWidget::updateUi()
{
	if (_Wrapper == NULL) 
		return;
	NL3D::CPlaneBasis pb = _Wrapper->get();
	NLMISC::CMatrix mat;
	mat.setRot(pb.X, pb.Y, pb.X ^ pb.Y);
	NLMISC::CVector angles = GetEulerAngles(mat);

	_ui.psiSpinBox->blockSignals(true);
	_ui.thetaSpinBox->blockSignals(true);
	_ui.phiSpinBox->blockSignals(true);

	_ui.psiSpinBox->setValue(int(360.f * angles.x / (2.f * (float) NLMISC::Pi)));
	_ui.thetaSpinBox->setValue(int(360.f * angles.y / (2.f * (float) NLMISC::Pi)));
	_ui.phiSpinBox->setValue(int(360.f * angles.z / (2.f * (float) NLMISC::Pi)));

	_ui.psiSpinBox->blockSignals(false);
	_ui.thetaSpinBox->blockSignals(false);
	_ui.phiSpinBox->blockSignals(false);

	repaint();
}

void CBasicEditWidget::updateGraphics()
{
	if (_Wrapper == NULL) 
		return;

	NLMISC::CVector angles(2.f * (float) NLMISC::Pi * _ui.psiSpinBox->value() / 360.f
						   , 2.f * (float) NLMISC::Pi * _ui.thetaSpinBox->value() / 360.f
						   , 2.f * (float) NLMISC::Pi * _ui.phiSpinBox->value() / 360.f
						  );
	NLMISC::CMatrix mat = BuildEulerMatrix(angles.x, angles.y, angles.z);
	NL3D::CPlaneBasis pb;
	pb.X = mat.getI();
	pb.Y = mat.getJ();
	_Wrapper->setAndUpdateModifiedFlag(pb);
	repaint();
}

bool CBasicEditWidget::eventFilter(QObject *object, QEvent *event)
{
	if( event->type() == QEvent::Paint )
	{
		QPainter painter(_ui.graphicsWidget);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setBrush(QBrush(Qt::white));
		painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));
		painter.drawRoundedRect(QRect(3, 3, _ui.graphicsWidget->width() - 6, _ui.graphicsWidget->height() - 6), 3.0, 3.0);
	}
	return QWidget::eventFilter(object, event);
}
} /* namespace NLQT */
