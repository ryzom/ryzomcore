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
#include "particle_zone_page.h"

// Qt includes

// NeL includes

// Project includes
#include "particle_force_page.h"
#include "modules.h"

namespace NLQT
{

CZonePage::CZonePage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.bounceFactorWidget->setRange(0.f, 1.f);

	connect(_ui.toTargetsToolButton, SIGNAL(clicked()), this, SLOT(addTarget()));
	connect(_ui.toAvaibleTargetsToolButton, SIGNAL(clicked()), this, SLOT(removeTarget()));
	connect(_ui.bounceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setBounce(int)));

	connect(_ui.bounceFactorWidget, SIGNAL(valueChanged(float)), this, SLOT(setBounceFactor(float)));
}

CZonePage::~CZonePage()
{
}

void CZonePage::setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable)
{
	_Node = ownerNode;
	_LBTarget = static_cast<NL3D::CPSTargetLocatedBindable *>(locatedBindable);
	_Zone = dynamic_cast<NL3D::CPSZone *>(_LBTarget);

	updateTargets();
	_ui.bounceFactorWidget->setValue(_Zone->getBounceFactor() ,false);
	_ui.bounceComboBox->setCurrentIndex( _Zone->getCollisionBehaviour());
}

void CZonePage::addTarget()
{
	// TODO: multiple add items
	int totalCount = _ui.avaibleTargetsListWidget->count();
	if ((totalCount == 0) || (_ui.avaibleTargetsListWidget->currentRow() == -1))  return;

	CLocatedItem *item = dynamic_cast<CLocatedItem *>(_ui.avaibleTargetsListWidget->currentItem());

	NL3D::CPSLocated *loc = item->getUserData();
	nlassert(loc);

	_LBTarget->attachTarget(loc);

	_ui.avaibleTargetsListWidget->takeItem(_ui.avaibleTargetsListWidget->currentRow());
	_ui.targetsListWidget->addItem(item);

	updateModifiedFlag();
}

void CZonePage::removeTarget()
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

void CZonePage::setBounce(int index)
{
	if (_Zone->getCollisionBehaviour() != index)
		_Zone->setCollisionBehaviour( (NL3D::CPSZone::TCollisionBehaviour) index);

	_ui.bounceFactorWidget->setEnabled(_Zone->getCollisionBehaviour() == NL3D::CPSZone::bounce ? true : false);
	Modules::psEdit().resetAutoCount(_Node);
}

void CZonePage::setBounceFactor(float value)
{
	_Zone->setBounceFactor(value);
	updateModifiedFlag();
}

void CZonePage::updateTargets()
{
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
	NL3D::CParticleSystem *ps = _LBTarget->getOwner()->getOwner();

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