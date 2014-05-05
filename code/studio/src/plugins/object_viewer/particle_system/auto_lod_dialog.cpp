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
#include "auto_lod_dialog.h"

// STL includes

// Qt includes

// NeL includes

// Project includes

namespace NLQT
{

CAutoLODDialog::CAutoLODDialog(CWorkspaceNode *ownerNode, NL3D::CParticleSystem *ps, QWidget *parent)
	: QDialog(parent),
	  _Node(ownerNode),
	  _PS(ps)
{
	_ui.setupUi(this);

	setFixedHeight(sizeHint().height());

	// Edit the distance at which LOD starts
	_ui.startPercentDistWidget->setRange(0.f, 0.99f);
	_ui.startPercentDistWidget->enableUpperBound(1.f, true);
	_ui.startPercentDistWidget->enableLowerBound(0.f, false);
	_ui.startPercentDistWidget->setValue(_PS->getAutoLODStartDistPercent());

	// For non-shared systems only : Set the LOD bias at the max distance, so that some particles are still displayed
	_ui.maxDistBiasWidget->setRange(0.f, 1.0f);
	_ui.maxDistBiasWidget->enableUpperBound(1.f, false);
	_ui.maxDistBiasWidget->enableLowerBound(0.f, false);
	_ui.maxDistBiasWidget->setValue(_PS->getMaxDistLODBias());

	if (_PS->isSharingEnabled())
	{
		_ui.maxDistBiasWidget->setEnabled(false);
		_ui.maxDistBiasLabel->setEnabled(false);
	}
	else
		_ui.skipParticlesCheckBox->setEnabled(false);

	_ui.degrdExponentSpinBox->setValue(int(_PS->getAutoLODDegradationExponent()) - 1);
	_ui.skipParticlesCheckBox->setChecked(_PS->getAutoLODMode());

	connect(_ui.degrdExponentSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setDegradationExponent(int)));
	connect(_ui.skipParticlesCheckBox, SIGNAL(toggled(bool)), this, SLOT(setSkipParticles(bool)));
}

CAutoLODDialog::~CAutoLODDialog()
{
}

void CAutoLODDialog::setDegradationExponent(int value)
{
	_PS->setupAutoLOD(_PS->getAutoLODStartDistPercent(),value + 1);
}

void CAutoLODDialog::setSkipParticles(bool state)
{
	_PS->setAutoLODMode(state);
}

void CAutoLODDialog::setDistRatio(float value)
{
	_PS->setupAutoLOD(value, _PS->getAutoLODDegradationExponent());
	_Node->setModified(true);
}

void CAutoLODDialog::setMaxDistLODBias(float value)
{
	_PS->setMaxDistLODBias(value);
	_Node->setModified(true);
}

} /* namespace NLQT */