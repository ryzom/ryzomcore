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
#include "constraint_mesh_widget.h"

// NeL includes
#include "nel/3d/ps_mesh.h"

namespace NLQT
{

CConstraintMeshWidget::CConstraintMeshWidget(QWidget *parent )
	: QWidget(parent)
{
	_ui.setupUi(this);

	connect(_ui.stageCheckBox_0, SIGNAL(clicked(bool)), this, SLOT(setForceStage0(bool)));
	connect(_ui.stageCheckBox_1, SIGNAL(clicked(bool)), this, SLOT(setForceStage1(bool)));
	connect(_ui.stageCheckBox_2, SIGNAL(clicked(bool)), this, SLOT(setForceStage2(bool)));
	connect(_ui.stageCheckBox_3, SIGNAL(clicked(bool)), this, SLOT(setForceStage3(bool)));
	connect(_ui.vertexColorLightingCheckBox, SIGNAL(clicked(bool)), this, SLOT(setForceVertexColorLighting(bool)));

	connect(_ui.texAnimTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setTexAnimType(int)));
	connect(_ui.reinitCheckBox, SIGNAL(toggled(bool)), this, SLOT(setReinitWhenNewElementIsCreated(bool)));
	connect(_ui.stageSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setStage(int)));

	connectGlobalTexAnim();
}

CConstraintMeshWidget::~CConstraintMeshWidget()
{
}

void CConstraintMeshWidget::connectGlobalTexAnim()
{
	connect(_ui.transUStartDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.transVStartDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.transUSpeedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.transVSpeedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.transUAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.transVAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));

	connect(_ui.scaleUStartDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.scaleVStartDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.scaleUSpeedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.scaleVSpeedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.scaleUAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.scaleVAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));

	connect(_ui.rotSpeedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	connect(_ui.rotAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
}

void CConstraintMeshWidget::disconnectGlobalTexAnim()
{
	disconnect(_ui.transUStartDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.transVStartDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.transUSpeedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.transVSpeedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.transUAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.transVAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));

	disconnect(_ui.scaleUStartDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.scaleVStartDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.scaleUSpeedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.scaleVSpeedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.scaleUAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.scaleVAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));

	disconnect(_ui.rotSpeedDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
	disconnect(_ui.rotAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlobalTexAnimValue()));
}

void CConstraintMeshWidget::setCurrentConstraintMesh(CWorkspaceNode *ownerNode, NL3D::CPSConstraintMesh *cm)
{
	_Node = ownerNode;
	_CM = cm;

	_ui.stageCheckBox_0->setChecked(_CM->isStageModulationForced(0));
	_ui.stageCheckBox_1->setChecked(_CM->isStageModulationForced(1));
	_ui.stageCheckBox_2->setChecked(_CM->isStageModulationForced(2));
	_ui.stageCheckBox_3->setChecked(_CM->isStageModulationForced(3));
	_ui.vertexColorLightingCheckBox->setChecked(_CM->isVertexColorLightingForced());

	_ui.texAnimTypeComboBox->setCurrentIndex(_CM->getTexAnimType());
	if (_CM->getTexAnimType() == NL3D::CPSConstraintMesh::GlobalAnim)
	{
		_ui.reinitCheckBox->setChecked(_CM->isGlobalAnimTimeResetOnNewElementForced());
		updateGlobalTexAnim(0);
	}
}

void CConstraintMeshWidget::setForceStage0(bool state)
{
	_CM->forceStageModulationByColor(0, state);
}

void CConstraintMeshWidget::setForceStage1(bool state)
{
	_CM->forceStageModulationByColor(1, state);
}

void CConstraintMeshWidget::setForceStage2(bool state)
{
	_CM->forceStageModulationByColor(2, state);
}

void CConstraintMeshWidget::setForceStage3(bool state)
{
	_CM->forceStageModulationByColor(3, state);
}

void CConstraintMeshWidget::setForceVertexColorLighting(bool state)
{
	_CM->forceVertexColorLighting(state);
}

void CConstraintMeshWidget::setTexAnimType(int index)
{
	switch(index)
	{
	case 0: // no anim
		_CM->setTexAnimType(NL3D::CPSConstraintMesh::NoAnim);
		_ui.stageSpinBox->hide();
		_ui.stageLabel->hide();
		_ui.tabWidget->hide();
		_ui.reinitCheckBox->hide();
		break;
	case 1: // global anim
		_CM->setTexAnimType(NL3D::CPSConstraintMesh::GlobalAnim);
		_ui.stageSpinBox->show();
		_ui.stageLabel->show();
		_ui.tabWidget->show();
		_ui.reinitCheckBox->show();
		_ui.stageSpinBox->setValue(0);
		_ui.reinitCheckBox->setChecked(_CM->isGlobalAnimTimeResetOnNewElementForced());
		break;
	default:
		nlstop;
		break;
	}
}

void CConstraintMeshWidget::setReinitWhenNewElementIsCreated(bool state)
{
	_CM->forceGlobalAnimTimeResetOnNewElement(state);
}

void CConstraintMeshWidget::setStage(int value)
{
	updateGlobalTexAnim(value);
}

void CConstraintMeshWidget::updateGlobalTexAnim(int value)
{
	disconnectGlobalTexAnim();

	const NL3D::CPSConstraintMesh::CGlobalTexAnim &gta = _CM->getGlobalTexAnim(value);

	_ui.transUStartDoubleSpinBox->setValue(gta.TransOffset.x);
	_ui.transVStartDoubleSpinBox->setValue(gta.TransOffset.y);
	_ui.transUSpeedDoubleSpinBox->setValue(gta.TransSpeed.x);
	_ui.transVSpeedDoubleSpinBox->setValue(gta.TransSpeed.y);
	_ui.transUAccelDoubleSpinBox->setValue(gta.TransAccel.x);
	_ui.transVAccelDoubleSpinBox->setValue(gta.TransAccel.y);

	_ui.scaleUStartDoubleSpinBox->setValue(gta.ScaleStart.x);
	_ui.scaleVStartDoubleSpinBox->setValue(gta.ScaleStart.y);
	_ui.scaleUSpeedDoubleSpinBox->setValue(gta.ScaleSpeed.x);
	_ui.scaleVSpeedDoubleSpinBox->setValue(gta.ScaleSpeed.y);
	_ui.scaleUAccelDoubleSpinBox->setValue(gta.ScaleAccel.x);
	_ui.scaleVAccelDoubleSpinBox->setValue(gta.ScaleAccel.y);

	_ui.rotSpeedDoubleSpinBox->setValue(gta.WRotSpeed);
	_ui.rotAccelDoubleSpinBox->setValue(gta.WRotAccel);

	connectGlobalTexAnim();
}

void CConstraintMeshWidget::setGlobalTexAnimValue()
{
	NL3D::CPSConstraintMesh::CGlobalTexAnim gta;

	gta.TransOffset.x = _ui.transUStartDoubleSpinBox->value();
	gta.TransOffset.y = _ui.transVStartDoubleSpinBox->value();
	gta.TransSpeed.x = _ui.transUSpeedDoubleSpinBox->value();
	gta.TransSpeed.y = _ui.transVSpeedDoubleSpinBox->value();
	gta.TransAccel.x = _ui.transUAccelDoubleSpinBox->value();
	gta.TransAccel.y = _ui.transVAccelDoubleSpinBox->value();

	gta.ScaleStart.x = _ui.scaleUStartDoubleSpinBox->value();
	gta.ScaleStart.y = _ui.scaleVStartDoubleSpinBox->value();
	gta.ScaleSpeed.x = _ui.scaleUSpeedDoubleSpinBox->value();
	gta.ScaleSpeed.y = _ui.scaleVSpeedDoubleSpinBox->value();
	gta.ScaleAccel.x = _ui.scaleUAccelDoubleSpinBox->value();
	gta.ScaleAccel.y = _ui.scaleVAccelDoubleSpinBox->value();

	gta.WRotSpeed = _ui.rotSpeedDoubleSpinBox->value();
	gta.WRotAccel = _ui.rotAccelDoubleSpinBox->value();

	_CM->setGlobalTexAnim(_ui.stageSpinBox->value(), gta);
}

} /* namespace NLQT */