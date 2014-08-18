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
#include "located_page.h"

// Qt includes
#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>

// NeL includes
#include <nel/3d/particle_system.h>

// Project includes
#include "modules.h"

namespace NLQT
{

CLocatedPage::CLocatedPage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.lifeWidget->setRange(0, 10);
	_ui.lifeWidget->setWrapper(&_LifeWrapper);
	_ui.lifeWidget->setSchemeWrapper(&_LifeWrapper);
	/// WARNING:// 0 is disallowed
	///_ui.lifeWidget->enableLowerBound(0, true);
	_ui.lifeWidget->enableMemoryScheme(false);
	_ui.lifeWidget->init();

	_ui.massWidget->setRange(0.001f, 10);
	_ui.massWidget->setWrapper(&_MassWrapper);
	_ui.massWidget->setSchemeWrapper(&_MassWrapper);
	/// WARNING:// 0 is disallowed
	///_ui.massWidget->enableLowerBound(0, true);
	_ui.massWidget->enableMemoryScheme(false);
	_ui.massWidget->init();

	_ui.maxNumParticleWidget->setRange(1, 501);
	_ui.maxNumParticleWidget->enableUpperBound(1 << 16, true);

	connect(_ui.coordSystemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setMatrixMode(int)));
	connect(_ui.disgradeWithLODCheckBox, SIGNAL(toggled(bool)), this, SLOT(setDisgradeWithLod(bool)));
	connect(_ui.parametricMotionCheckBox, SIGNAL(toggled(bool)), this, SLOT(setParametricMotion(bool)));
	connect(_ui.trigerOnDeathCheckBox, SIGNAL(toggled(bool)), this, SLOT(setTriggerOnDeath(bool)));
	connect(_ui.editPushButton, SIGNAL(clicked()), this, SLOT(editTriggerOnDeath()));
	connect(_ui.limitedLifeTimeCheckBox, SIGNAL(toggled(bool)), this, SLOT(setLimitedLifeTime(bool)));
	connect(_ui.setCurrentCountPushButton, SIGNAL(clicked()), this, SLOT(setCurrentCount()));

	connect(_ui.maxNumParticleWidget, SIGNAL(valueChanged(uint32)), this, SLOT(setNewMaxSize(uint32)));
}

CLocatedPage::~CLocatedPage()
{
}

void CLocatedPage::setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocated *located)
{
	_Located = located;
	_Node = ownerNode;

	_LifeWrapper.Located = _Located;
	_LifeWrapper.Node = _Node;
	_ui.lifeWidget->setWorkspaceNode(_Node);
	_ui.lifeWidget->updateUi();

	if (_Located->getLastForever())
		_ui.lifeWidget->setEnabled(false);


	_MassWrapper.Located = _Located;
	_ui.massWidget->setWorkspaceNode(_Node);
	_ui.massWidget->updateUi();

	if (_Located->getOwner())
		_ui.maxNumParticleWidget->setEnabled(!_Located->getOwner()->getAutoCountFlag());

	_ui.maxNumParticleWidget->setValue(_Located->getMaxSize(), false);

	_ui.coordSystemComboBox->setCurrentIndex(int(_Located->getMatrixMode()));
	_ui.limitedLifeTimeCheckBox->setChecked(!_Located->getLastForever());
	_ui.lifeWidget->setEnabled(!_Located->getLastForever());
	_ui.trigerOnDeathCheckBox->setChecked(_Located->isTriggerOnDeathEnabled());
	updateTriggerOnDeath();

	_ui.disgradeWithLODCheckBox->setChecked(_Located->hasLODDegradation());
	updateIntegrable();
	updateTriggerOnDeath();
}

void CLocatedPage::setDisabledCountPS(bool state)
{
	_ui.setCurrentCountPushButton->setEnabled(!state);
	_ui.maxNumParticleWidget->setEnabled(!state);
}

void CLocatedPage::setLimitedLifeTime(bool state)
{
	if (state != _Located->getLastForever()) return;
	if (!state)
	{
		bool forceApplied = false;
		// check that no force are applied on the located
		std::vector<NL3D::CPSTargetLocatedBindable *> targeters;
		_Located->getOwner()->getTargeters(_Located, targeters);
		for(uint k = 0; k < targeters.size(); ++k)
		{
			if (targeters[k]->getType() == NL3D::PSForce)
			{
				forceApplied = true;
				break;
			}
		}
		if (forceApplied)
		{
			int ret = QMessageBox::critical(this, tr("NeL particle system editor"),
											tr("The object has force(s) applied on it. If it last forever, "
											   "its motion can become instable after a while. Continue anyway ? (clue : you've been warned ..)"),
											QMessageBox::Ok | QMessageBox::Cancel);

			if (ret == QMessageBox::Cancel)
			{
				_ui.limitedLifeTimeCheckBox->setChecked(true);
				return;
			}
		}
		if (_Located->setLastForever())
		{
			_ui.lifeWidget->setEnabled(false);
		}
		else
		{
			QMessageBox::critical(this, tr("NeL particle system editor"),
								  tr("Can't perform operation : the system is flagged with 'No max nb steps' or uses the preset 'Spell FX', "
									 "and thus, should have a finite duration. Please remove that flag first."),
								  QMessageBox::Ok);
			_ui.limitedLifeTimeCheckBox->setChecked(true);
		}
	}
	else
	{
		_Located->setInitialLife(_Located->getInitialLife());
		_ui.lifeWidget->setEnabled(true);
	}
	updateTriggerOnDeath();
	Modules::psEdit().resetAutoCount(_Node);
	updateModifiedFlag();
}

void CLocatedPage::setDisgradeWithLod(bool state)
{
	if (state != _Located->hasLODDegradation())
	{
		_Located->forceLODDegradation(state);
		updateModifiedFlag();
	}
}

void CLocatedPage::setParametricMotion(bool state)
{
	if (state != _Located->isParametricMotionEnabled())
	{
		_Located->enableParametricMotion(state);
		updateModifiedFlag();
	}
}

void CLocatedPage::editTriggerOnDeath()
{
	bool ok;
	int i = QInputDialog::getInt(this, tr("Set the extern ID"),
								 tr("0 means no extern access."),
								 _Located->getTriggerEmitterID(), 0, 9999, 1, &ok);
	if (ok)
	{
		_Located->setTriggerEmitterID(uint32(i));
		updateModifiedFlag();
	}
}

void CLocatedPage::setTriggerOnDeath(bool state)
{
	if (state != _Located->isTriggerOnDeathEnabled())
	{
		_Located->enableTriggerOnDeath(state);
		updateTriggerOnDeath();
		updateModifiedFlag();
	}
}

void CLocatedPage::setMatrixMode(int index)
{
	nlassert(_Located);
	if (index != _Located->getMatrixMode())
	{
		_Located->setMatrixMode((NL3D::TPSMatrixMode) index);
		updateIntegrable();
		updateModifiedFlag();
	}
}

void CLocatedPage::setCurrentCount()
{
	// set new max size
	_ui.maxNumParticleWidget->setValue(_Located->getSize());
	updateModifiedFlag();
}

void CLocatedPage::setNewMaxSize(uint32 value)
{
	// if the max new size is lower than the current number of instance, we must suppress item
	// in the CParticleTreeCtrl
	if (value < _Located->getSize())
	{
		nlassert(_Node);
		/// WARNING:
		///TreeCtrl->suppressLocatedInstanceNbItem(*Node, v);
	}
	_Located->resize(value);

	updateModifiedFlag();
}

void CLocatedPage::updateIntegrable(void)
{
	_ui.parametricMotionCheckBox->setChecked(_Located->isParametricMotionEnabled());
	_ui.parametricMotionCheckBox->setEnabled(_Located->supportParametricMotion());
}

void CLocatedPage::updateTriggerOnDeath(void)
{
	nlassert(_Located);
	bool enable = !_Located->getLastForever();
	_ui.trigerOnDeathCheckBox->setEnabled(enable);
	_ui.editPushButton->setEnabled(enable && _Located->isTriggerOnDeathEnabled());
}

void CLocatedPage::CLifeWrapper::set(const float &v)
{
	Located->setInitialLife(v);
	Modules::psEdit().resetAutoCount(Node);
}
void CLocatedPage::CLifeWrapper::setScheme(scheme_type *s)
{
	Located->setLifeScheme(s);
	Modules::psEdit().resetAutoCount(Node);
}

} /* namespace NLQT */