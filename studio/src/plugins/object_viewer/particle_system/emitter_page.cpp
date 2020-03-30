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
#include "emitter_page.h"

// Qt includes
#include <QtGui/QMessageBox>

// NeL includes

// Project includes
#include "edit_range_widget.h"
#include "modules.h"

namespace NLQT
{


CEmitterPage::CEmitterPage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	// setup the dialog for the period of emission edition
	_ui.periodWidget->setRange(0.f, 2.f);
	_ui.periodWidget->setWrapper(&_PeriodWrapper);
	_ui.periodWidget->setSchemeWrapper(&_PeriodWrapper);
	_ui.periodWidget->init();

	// setup the dialog that helps tuning the number of particle being emitted at a time
	_ui.genNbWidget->setRange(1, 11);
	_ui.genNbWidget->setWrapper(&_GenNbWrapper);
	_ui.genNbWidget->setSchemeWrapper(&_GenNbWrapper);
	_ui.genNbWidget->init();

	// deals with emitters that have a direction
	_ui.strenghtModulateWidget->setRange(0, 10);
	_ui.strenghtModulateWidget->setWrapper(&_ModulatedStrenghtWrapper);
	_ui.strenghtModulateWidget->setSchemeWrapper(&_ModulatedStrenghtWrapper);
	_ui.strenghtModulateWidget->init();

	// SPEED_INHERITANCE_FACTOR
	_ui.speedInherFactorWidget->setRange(-1.f, 1.f);

	// DELAYED_EMISSION
	_ui.delayedEmissionWidget->setRange(0.f, 10.f);
	_ui.delayedEmissionWidget->enableLowerBound(0.f, false);

	// MAX_EMISSION_COUNT
	_ui.maxEmissionCountWidget->setRange(0, 100);
	_ui.maxEmissionCountWidget->enableUpperBound(256, false);

	// radius  for conic emitter
	_ui.radiusWidget->setRange(0.1f, 2.1f);

	connect(_ui.emittedTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEmittedType(int)));
	connect(_ui.typeEmissionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setTypeOfEmission(int)));
	connect(_ui.directionComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setDirectionMode(int)));
	connect(_ui.bypassAutoLODCheckBox, SIGNAL(toggled(bool)), this, SLOT(setBypassAutoLOD(bool)));
	connect(_ui.forceConsistentCheckBox, SIGNAL(toggled(bool)), this, SLOT(setConsistentEmission(bool)));

	connect(_ui.speedInherFactorWidget, SIGNAL(valueChanged(float)), this, SLOT(setSpeedInheritanceFactor(float)));
	connect(_ui.delayedEmissionWidget, SIGNAL(valueChanged(float)), this, SLOT(setEmitDelay(float)));
	connect(_ui.radiusWidget, SIGNAL(valueChanged(float)), this, SLOT(setConicEmitterRadius(float)));
	connect(_ui.maxEmissionCountWidget, SIGNAL(valueChanged(uint32)), this, SLOT(setMaxEmissionCount(uint32)));
	connect(_ui.directionWidget, SIGNAL(valueChanged(NLMISC::CVector)), this, SLOT(setDir(NLMISC::CVector)));
}

CEmitterPage::~CEmitterPage()
{
}

void CEmitterPage::setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable)
{
	_Emitter = static_cast<NL3D::CPSEmitter *>(locatedBindable);
	_Node = ownerNode;

	updateEmittedType();

	_ui.speedInherFactorWidget->setValue(_Emitter->getSpeedInheritanceFactor(), false);
	_ui.delayedEmissionWidget->setValue(_Emitter->getEmitDelay(), false);
	_ui.maxEmissionCountWidget->setValue(_Emitter->getMaxEmissionCount(), false);

	_PeriodWrapper.Node = _Node;
	_PeriodWrapper.E = _Emitter;
	_ui.periodWidget->setWorkspaceNode(_Node);
	_ui.periodWidget->updateUi();

	_GenNbWrapper.Node = _Node;
	_GenNbWrapper.E = _Emitter;
	_ui.genNbWidget->setWorkspaceNode(_Node);
	_ui.genNbWidget->updateUi();

	if (dynamic_cast<NL3D::CPSModulatedEmitter *>(_Emitter))
	{
		_ModulatedStrenghtWrapper.E = dynamic_cast<NL3D::CPSModulatedEmitter *>(_Emitter);
		_ui.strenghtModulateWidget->setWorkspaceNode(_Node);
		_ui.strenghtModulateWidget->updateUi();
		_ui.strenghtModulateWidget->show();
	}
	else
		_ui.strenghtModulateWidget->hide();


	// deals with emitters that have a direction
	if (dynamic_cast<NL3D::CPSDirection *>(_Emitter))
	{
		_ui.directionWidget->show();
		_ui.directionWidget->setValue(dynamic_cast<NL3D::CPSDirection *>(_Emitter)->getDir(), false);
	}
	else
		_ui.directionWidget->hide();


	// radius  for conic emitter
	if (dynamic_cast<NL3D::CPSEmitterConic *>(_Emitter))
	{
		_ui.radiusWidget->setValue(dynamic_cast<NL3D::CPSEmitterConic *>(_Emitter)->getRadius(),false);
		_ui.radiusWidget->show();
		_ui.radiusLabel->show();
	}
	else
	{
		_ui.radiusWidget->hide();
		_ui.radiusLabel->hide();
	}

	_ui.forceConsistentCheckBox->setChecked(_Emitter->isConsistentEmissionEnabled());

	_ui.directionComboBox->blockSignals(true);
	if (_Emitter->isSpeedBasisEmissionEnabled())
	{
		_ui.directionComboBox->setCurrentIndex(int(AlignOnEmitterDirection));
	}
	else if (!_Emitter->isUserMatrixModeForEmissionDirectionEnabled())
	{
		_ui.directionComboBox->setCurrentIndex(int(Default));
	}
	else if (_Emitter->getUserMatrixModeForEmissionDirection() == NL3D::PSFXWorldMatrix)
	{
		_ui.directionComboBox->setCurrentIndex(int(LocalToSystem));
	}
	else if (_Emitter->getUserMatrixModeForEmissionDirection() == NL3D::PSIdentityMatrix)
	{
		_ui.directionComboBox->setCurrentIndex(int(InWorld));
	}
	else if (_Emitter->getUserMatrixModeForEmissionDirection() == NL3D::PSUserMatrix)
	{
		_ui.directionComboBox->setCurrentIndex(int(LocalToFatherSkeleton));
	}
	else
	{
		nlassert(0);
	}
	_ui.directionComboBox->blockSignals(false);

	updatePeriodWidget();

	_ui.typeEmissionComboBox->setCurrentIndex(int(_Emitter->getEmissionType()));

	// bypass auto LOD
	nlassert(_Emitter->getOwner() && _Emitter->getOwner()->getOwner());
	NL3D::CParticleSystem &ps = *_Emitter->getOwner()->getOwner();
	if (ps.isAutoLODEnabled() && !ps.isSharingEnabled())
	{
		_ui.bypassAutoLODCheckBox->setEnabled(true);
		_ui.bypassAutoLODCheckBox->setChecked(_Emitter->getBypassAutoLOD());
	}
	else
		_ui.bypassAutoLODCheckBox->setEnabled(false);
}

void CEmitterPage::setEmittedType(int index)
{
	if (!_Emitter->setEmittedType(_LocatedList[index]))
	{
		if (_Emitter->getOwner()->getOwner()->getBehaviourType() == NL3D::CParticleSystem::SpellFX || _Emitter->getOwner()->getOwner()->getBypassMaxNumIntegrationSteps())
		{
			QMessageBox::critical(this, tr("NeL Particle Editor"),
								  tr("Can't perform operation : the system is flagged with 'No max nb steps' or uses the preset 'Spell FX',"
									 "and thus, should have a finite duration. This operation create a loop in the system, and so is forbidden."),
								  QMessageBox::Ok);
		}
		else
		{
			QMessageBox::critical(this, tr("NeL Particle Editor"),
								  tr("Loops with emitters are forbidden."),
								  QMessageBox::Ok);
		}
		updateEmittedType();
	}

	Modules::psEdit().resetAutoCount(_Node);
	updateModifiedFlag();
}

void CEmitterPage::setTypeOfEmission(int index)
{
	if (_Emitter->getEmissionType() == index) return;
	if (!_Emitter->setEmissionType((NL3D::CPSEmitter::TEmissionType) index))
	{
		QMessageBox::critical(this, tr("NeL Particle Editor"),
							  tr("Can't perform operation : the system is flagged with 'No max nb steps' or uses the preset 'Spell FX', "
								 "and thus, should have a finite duration. Please remove that flag first."),
							  QMessageBox::Ok);

		_ui.typeEmissionComboBox->setCurrentIndex(int(_Emitter->getEmissionType()));
	}

	updatePeriodWidget();

	Modules::psEdit().resetAutoCount(_Node);

	updateModifiedFlag();
}

void CEmitterPage::setConsistentEmission(bool state)
{
	if (_Emitter->isConsistentEmissionEnabled() == state) return;
	_Emitter->enableConsistenEmission(state);
	updateModifiedFlag();
}

void CEmitterPage::setBypassAutoLOD(bool state)
{
	if (_Emitter->getBypassAutoLOD() == state) return;
	_Emitter->setBypassAutoLOD(state);
	updateModifiedFlag();
}

void CEmitterPage::setDirectionMode(int index)
{
	nlassert(_Emitter);
	switch(index)
	{
	case Default:
		_Emitter->enableSpeedBasisEmission(false);
		_Emitter->enableUserMatrixModeForEmissionDirection(false);
		break;
	case AlignOnEmitterDirection:
		_Emitter->enableSpeedBasisEmission(true);
		_Emitter->enableUserMatrixModeForEmissionDirection(false);
		break;
	case InWorld:
		_Emitter->enableSpeedBasisEmission(false);
		_Emitter->enableUserMatrixModeForEmissionDirection(true);
		_Emitter->setUserMatrixModeForEmissionDirection(NL3D::PSIdentityMatrix);
		break;
	case LocalToSystem:
		_Emitter->enableSpeedBasisEmission(false);
		_Emitter->enableUserMatrixModeForEmissionDirection(true);
		_Emitter->setUserMatrixModeForEmissionDirection(NL3D::PSFXWorldMatrix);
		break;
	case LocalToFatherSkeleton:
		_Emitter->enableSpeedBasisEmission(false);
		_Emitter->enableUserMatrixModeForEmissionDirection(true);
		_Emitter->setUserMatrixModeForEmissionDirection(NL3D::PSUserMatrix);
		break;
	}
	updateModifiedFlag();
}

void CEmitterPage::setSpeedInheritanceFactor(float value)
{
	_Emitter->setSpeedInheritanceFactor(value);
	updateModifiedFlag();
}

void CEmitterPage::setConicEmitterRadius(float value)
{
	dynamic_cast<NL3D::CPSEmitterConic *>(_Emitter)->setRadius(value);
	updateModifiedFlag();
}

void CEmitterPage::setEmitDelay(float value)
{
	_Emitter->setEmitDelay(value);
	Modules::psEdit().resetAutoCount(_Node);
	updateModifiedFlag();
}

void CEmitterPage::setMaxEmissionCount(uint32 value)
{
	if (!_Emitter->setMaxEmissionCount((uint8)value))
	{

		QMessageBox::critical(this, tr("NeL Particle Editor"),
							  tr("Can't perform operation : the system is flagged with 'No max nb steps' or uses the preset 'Spell FX', "
								 "and thus, should have a finite duration. Please remove that flag first."),
							  QMessageBox::Ok);

		_ui.maxEmissionCountWidget->setValue((uint32)_Emitter->getMaxEmissionCount(), false);
		updateModifiedFlag();
	}
	Modules::psEdit().resetAutoCount(_Node);
}

void CEmitterPage::setDir(const NLMISC::CVector &value)
{
	dynamic_cast<NL3D::CPSDirection *>(_Emitter)->setDir(value);
	updateModifiedFlag();
}

void CEmitterPage::updatePeriodWidget()
{
	bool bEnable = _Emitter->getEmissionType() == NL3D::CPSEmitter::regular;

	_ui.periodWidget->setEnabled(bEnable);
	_ui.delayedEmissionWidget->setEnabled(bEnable);
	_ui.delayedEmissionLabel->setEnabled(bEnable);
	_ui.maxEmissionCountWidget->setEnabled(bEnable);
	_ui.maxEmissinCountLabel->setEnabled(bEnable);
}

void CEmitterPage::updateEmittedType()
{
	disconnect(_ui.emittedTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEmittedType(int)));

	_ui.emittedTypeComboBox->clear();
	_ui.emittedTypeComboBox->addItem(tr("no emission"));
	_LocatedList.clear();
	_LocatedList.push_back(NULL);
	NL3D::CParticleSystem *ps = _Emitter->getOwner()->getOwner();
	uint nbLocated = ps->getNbProcess();
	for (uint k = 0; k < nbLocated; ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k));
		if (loc) // is this a located
		{
			_ui.emittedTypeComboBox->addItem(QString(loc->getName().c_str()));
			_LocatedList.push_back(loc);
			if (loc == _Emitter->getEmittedType())
				_ui.emittedTypeComboBox->setCurrentIndex(k + 1);
		}
	}
	connect(_ui.emittedTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEmittedType(int)));
}

void CEmitterPage::CPeriodWrapper::set(const float &v)
{
	E->setPeriod(v);
	Modules::psEdit().resetAutoCount(Node);
}
void CEmitterPage::CPeriodWrapper::setScheme(scheme_type *s)
{
	E->setPeriodScheme(s);
	Modules::psEdit().resetAutoCount(Node);
}

void CEmitterPage::CGenNbWrapper::set(const uint32 &v)
{
	E->setGenNb(v);
	Modules::psEdit().resetAutoCount(Node);
}

void CEmitterPage::CGenNbWrapper::setScheme(scheme_type *s)
{
	E->setGenNbScheme(s);
	Modules::psEdit().resetAutoCount(Node);
}

} /* namespace NLQT */