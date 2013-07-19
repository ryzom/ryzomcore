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
#include "particle_force_page.h"

// Qt includes
#include <QtGui/QMessageBox>

// NeL includes
#include <nel/3d/particle_system.h>

// Project includes

namespace NLQT
{

CForcePage::CForcePage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.forceIntensityWidget->setRange(0, 10);
	_ui.forceIntensityWidget->setWrapper(&_ForceIntensityWrapper);
	_ui.forceIntensityWidget->setSchemeWrapper(&_ForceIntensityWrapper);
	_ui.forceIntensityWidget->init();

	_ui.parametricFactorWidget->setRange(0.0, 64.0);
	_ui.radialViscosityWidget->setRange(0.0, 1.0);
	_ui.tangentialViscosityWidget->setRange(0, 1);

	connect(_ui.toTargetsButton, SIGNAL(clicked()), this, SLOT(addTarget()));
	connect(_ui.toAvaibleTargetsButton, SIGNAL(clicked()), this, SLOT(removeTarget()));

	connect(_ui.parametricFactorWidget, SIGNAL(valueChanged(float)), this, SLOT(setFactorBrownianForce(float)));
	connect(_ui.radialViscosityWidget, SIGNAL(valueChanged(float)), this, SLOT(setRadialViscosity(float)));
	connect(_ui.tangentialViscosityWidget, SIGNAL(valueChanged(float)), this, SLOT(setTangentialViscosity(float)));
	connect(_ui.directionWidget, SIGNAL(valueChanged(NLMISC::CVector)), this, SLOT(setDir(NLMISC::CVector)));
	connect(_ui.directionWidget, SIGNAL(globalNameChanged(QString)), this, SLOT(setGlobalName(QString)));
}

CForcePage::~CForcePage()
{
}

void CForcePage::setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable)
{
	nlassert(locatedBindable);

	hideAdditionalWidget();
	_Node = ownerNode;
	_LBTarget = static_cast<NL3D::CPSTargetLocatedBindable *>(locatedBindable);
	updateTargets();

	// force with intensity case
	if (dynamic_cast<NL3D::CPSForceIntensity *>(_LBTarget))
	{
		_ForceIntensityWrapper.F = dynamic_cast<NL3D::CPSForceIntensity *>(_LBTarget);

		_ui.forceIntensityWidget->setWorkspaceNode(_Node);
		_ui.forceIntensityWidget->updateUi();
	}

	// vortex (to tune viscosity)
	NL3D::CPSCylindricVortex *cylindricVortex = dynamic_cast<NL3D::CPSCylindricVortex *>(_LBTarget);
	if (cylindricVortex)
	{
		_ui.radialViscosityWidget->setValue(cylindricVortex->getRadialViscosity(), false);
		_ui.radialViscosityLabel->show();
		_ui.radialViscosityWidget->show();

		_ui.tangentialViscosityWidget->setValue(cylindricVortex->getTangentialViscosity(), false);
		_ui.tangentialViscosityLabel->show();
		_ui.tangentialViscosityWidget->show();
	}

	// deals with emitters that have a direction
	NL3D::CPSDirection *direction = dynamic_cast<NL3D::CPSDirection *>(_LBTarget);
	if (direction)
	{
		_ui.directionWidget->setValue(direction->getDir(), false);
		_ui.directionWidget->enabledGlobalVariable(direction->supportGlobalVectorValue());
		_ui.directionWidget->setGlobalName(QString(direction->getGlobalVectorValueName().c_str()), false);
		_ui.directionWidget->show();
	}

	// Brownian (to tune parametric factor)
	NL3D::CPSBrownianForce *brownianForce = dynamic_cast<NL3D::CPSBrownianForce *>(_LBTarget);
	if (brownianForce)
	{
		_ui.parametricFactorWidget->setValue(brownianForce->getParametricFactor(), false);
		_ui.parametricFactorLabel->show();
		_ui.parametricFactorWidget->show();
	}
}

void CForcePage::addTarget()
{
	// TODO: multiple add items
	int totalCount = _ui.avaibleTargetsListWidget->count();
	if ((totalCount == 0) || (_ui.avaibleTargetsListWidget->currentRow() == -1))  return;

	CLocatedItem *item = dynamic_cast<CLocatedItem *>(_ui.avaibleTargetsListWidget->currentItem());

	NL3D::CPSLocated *loc = item->getUserData();
	nlassert(loc);

	// check that force isn't applied on a forever lasting object
	if (dynamic_cast<NL3D::CPSForce *>(_LBTarget))
	{
		if (loc->getLastForever())
		{
			int ret = QMessageBox::warning(this, tr("NeL particle system editor"),
										   tr("The target object last forever. Applying a force on such an object may result in instability in the system after a while. "
											  "Continue ? (clue : you've been warned..)"),
										   QMessageBox::Ok | QMessageBox::Cancel);

			if (ret == QMessageBox::Cancel)
				return;
		}

	}
	//
	_LBTarget->attachTarget(loc);

	_ui.avaibleTargetsListWidget->takeItem(_ui.avaibleTargetsListWidget->currentRow());
	_ui.targetsListWidget->addItem(item);

	updateModifiedFlag();
}

void CForcePage::removeTarget()
{
	// TODO: multiple remove items
	int totalCount = _ui.targetsListWidget->count();
	if ((totalCount == 0) || (_ui.targetsListWidget->currentRow() == -1))  return;

	CLocatedItem *item = dynamic_cast<CLocatedItem *>(_ui.targetsListWidget->takeItem(_ui.targetsListWidget->currentRow()));

	NL3D::CPSLocated *loc = item->getUserData();
	nlassert(loc);

	_LBTarget->detachTarget(loc);

	_ui.avaibleTargetsListWidget->addItem(item);
	updateModifiedFlag();
}
void CForcePage::setRadialViscosity(float value)
{
	nlassert(_LBTarget);
	dynamic_cast<NL3D::CPSCylindricVortex *>(_LBTarget)->setRadialViscosity(value);
	updateModifiedFlag();
}

void CForcePage::setTangentialViscosity(float value)
{
	nlassert(_LBTarget);
	dynamic_cast<NL3D::CPSCylindricVortex *>(_LBTarget)->setTangentialViscosity(value);
	updateModifiedFlag();
}

void CForcePage::setDir(const NLMISC::CVector &value)
{
	nlassert(_LBTarget);
	dynamic_cast<NL3D::CPSDirection *>(_LBTarget)->setDir(value);
	updateModifiedFlag();
}

void CForcePage::setGlobalName(const QString &globalName)
{
	nlassert(_LBTarget);
	dynamic_cast<NL3D::CPSDirection *>(_LBTarget)->enableGlobalVectorValue(globalName.toUtf8().constData());
	if (!globalName.isEmpty())
	{
		// take a non NULL value for the direction
		NL3D::CParticleSystem::setGlobalVectorValue(globalName.toUtf8().constData(), NLMISC::CVector::I);
	}
	updateModifiedFlag();
}

void CForcePage::setFactorBrownianForce(float value)
{
	nlassert(_LBTarget);
	dynamic_cast<NL3D::CPSBrownianForce *>(_LBTarget)->setParametricFactor(value);
	updateModifiedFlag();
}

void CForcePage::hideAdditionalWidget()
{
	_ui.directionWidget->hide();
	_ui.parametricFactorLabel->hide();
	_ui.parametricFactorWidget->hide();
	_ui.radialViscosityLabel->hide();
	_ui.radialViscosityWidget->hide();
	_ui.tangentialViscosityLabel->hide();
	_ui.tangentialViscosityWidget->hide();
}

void CForcePage::updateTargets()
{
	nlassert(_LBTarget);
	uint k;
	uint nbTarg = _LBTarget->getNbTargets();

	_ui.targetsListWidget->clear();

	std::set<NL3D::CPSLocated *> targetSet;

	// fill the box thta tells us what the target are
	for(k = 0; k < nbTarg; ++k)
	{
		CLocatedItem *item = new CLocatedItem(QString(_LBTarget->getTarget(k)->getName().c_str()),
											  _ui.targetsListWidget);
		item->setUserData(_LBTarget->getTarget(k));
		targetSet.insert(_LBTarget->getTarget(k));
	};

	// fill abox with the available targets
	NL3D::CParticleSystem  *ps = _LBTarget->getOwner()->getOwner();

	uint nbLocated = ps->getNbProcess();

	_ui.avaibleTargetsListWidget->clear();

	for (k = 0; k < nbLocated; ++k)
	{
		NL3D::CPSLocated *loc = dynamic_cast<NL3D::CPSLocated *>(ps->getProcess(k));
		if (loc)
		{
			if (targetSet.find(loc) == targetSet.end())
			{
				CLocatedItem *item = new CLocatedItem(QString(loc->getName().c_str()),
													  _ui.avaibleTargetsListWidget);
				item->setUserData(loc);
			}
		}
	}
}

} /* namespace NLQT */