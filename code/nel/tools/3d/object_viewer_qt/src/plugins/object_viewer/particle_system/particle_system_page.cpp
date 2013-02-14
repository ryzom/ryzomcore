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
#include "particle_system_page.h"

// Qt includes
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>
#include <QtGui/QErrorMessage>

// NeL includes
#include <nel/3d/particle_system.h>
#include <nel/3d/particle_system_model.h>
#include <nel/3d/ps_color.h>

// Project includes
#include "modules.h"
#include "auto_lod_dialog.h"

using namespace NL3D;
using namespace NLMISC;

namespace NLQT
{

static void chooseGlobalUserParam(uint userParam, NL3D::CParticleSystem *ps, QWidget *parent)
{
	nlassert(ps);
	bool ok;
	QString text = QInputDialog::getText(parent, "Choose Global User Param",
										 "User name:", QLineEdit::Normal,
										 QString(ps->getGlobalValueName(userParam).c_str()), &ok);

	if (ok)
		ps->bindGlobalValueToUserParam(text.toUtf8().constData(), userParam);
}

CParticleSystemPage::CParticleSystemPage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.timeThresholdWidget->setRange(0.005f, 0.3f);
	_ui.timeThresholdWidget->enableLowerBound(0, true);

	_ui.maxStepsWidget->setRange(0, 4);
	_ui.maxStepsWidget->enableLowerBound(0, true);

	_ui.userParamWidget_1->setRange(0, 1.0f);
	_ui.userParamWidget_1->enableLowerBound(0, false);
	_ui.userParamWidget_1->enableUpperBound(1, false);

	_ui.userParamWidget_2->setRange(0, 1.0f);
	_ui.userParamWidget_2->enableLowerBound(0, false);
	_ui.userParamWidget_2->enableUpperBound(1, false);

	_ui.userParamWidget_3->setRange(0, 1.0f);
	_ui.userParamWidget_3->enableLowerBound(0, false);
	_ui.userParamWidget_3->enableUpperBound(1, false);

	_ui.userParamWidget_4->setRange(0, 1.0f);
	_ui.userParamWidget_4->enableLowerBound(0, false);
	_ui.userParamWidget_4->enableUpperBound(1, false);

	_ui.maxViewDistWidget->setRange(0, 400.f);
	_ui.maxViewDistWidget->enableLowerBound(0, true);

	_ui.lodRatioWidget->setRange(0, 1.f);
	_ui.lodRatioWidget->enableLowerBound(0, true);
	_ui.lodRatioWidget->enableUpperBound(1, true);

	_ui.colorWidget->setSchemeWrapper(&_GlobalColorWrapper);
	_ui.colorWidget->enableMemoryScheme(false);
	_ui.colorWidget->enableNbCycles(false);
	_ui.colorWidget->enableSrcInput(false);
	_ui.colorWidget->setEnabledConstantValue(false);
	_ui.colorWidget->init();
	_ui.colorWidget->hide();

	// Integration tab
	connect(_ui.loadBalancingCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setLoadBalancing(bool)));
	connect(_ui.integrationCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setIntegration(bool)));
	connect(_ui.motionSlowDownCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setMotionSlowDown(bool)));
	connect(_ui.lockPushButton ,SIGNAL(toggled(bool)), this, SLOT(setLock(bool)));

	connect(_ui.timeThresholdWidget ,SIGNAL(valueChanged(float)), this, SLOT(setTimeThreshold(float)));
	connect(_ui.maxStepsWidget ,SIGNAL(valueChanged(uint32)), this, SLOT(setMaxSteps(uint32)));

	// User param tab
	connect(_ui.globalPushButton_1 ,SIGNAL(clicked()), this, SLOT(setGloabal1()));
	connect(_ui.globalPushButton_2 ,SIGNAL(clicked()), this, SLOT(setGloabal2()));
	connect(_ui.globalPushButton_3 ,SIGNAL(clicked()), this, SLOT(setGloabal3()));
	connect(_ui.globalPushButton_4 ,SIGNAL(clicked()), this, SLOT(setGloabal4()));

	connect(_ui.userParamWidget_1 ,SIGNAL(valueChanged(float)), this, SLOT(setUserParam1(float)));
	connect(_ui.userParamWidget_2 ,SIGNAL(valueChanged(float)), this, SLOT(setUserParam2(float)));
	connect(_ui.userParamWidget_3 ,SIGNAL(valueChanged(float)), this, SLOT(setUserParam3(float)));
	connect(_ui.userParamWidget_4 ,SIGNAL(valueChanged(float)), this, SLOT(setUserParam4(float)));

	// BBox tab
	connect(_ui.enablePBBCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setEnableBbox(bool)));
	connect(_ui.autoCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setAutoBbox(bool)));
	connect(_ui.resetPushButton ,SIGNAL(clicked()), this, SLOT(resetBbox()));
	connect(_ui.incBboxPushButton ,SIGNAL(clicked()), this, SLOT(incBbox()));
	connect(_ui.decBboxPushButton ,SIGNAL(clicked()), this, SLOT(decBbox()));
	connect(_ui.xDoubleSpinBox ,SIGNAL(valueChanged(double)), this, SLOT(setXBbox(double)));
	connect(_ui.yDoubleSpinBox ,SIGNAL(valueChanged(double)), this, SLOT(setYBbox(double)));
	connect(_ui.zDoubleSpinBox ,SIGNAL(valueChanged(double)), this, SLOT(setZBbox(double)));

	// LOD param tab
	connect(_ui.sharableCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setSharable(bool)));
	connect(_ui.autoLODCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setAutoLOD(bool)));
	connect(_ui.settingsPushButton ,SIGNAL(clicked()), this, SLOT(settings()));

	connect(_ui.maxViewDistWidget ,SIGNAL(valueChanged(float)), this, SLOT(setMaxViewDist(float)));
	connect(_ui.lodRatioWidget ,SIGNAL(valueChanged(float)), this, SLOT(setLodRatio(float)));

	// Global color tab
	connect(_ui.editGlobalColorCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setEditGlobalColor(bool)));
	connect(_ui.globalLightCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setGlobalLight(bool)));

	// Life mgt param tab
	connect(_ui.presetBehaviourComboBox ,SIGNAL(currentIndexChanged(int)), this, SLOT(setPresetBehaviour(int)));
	connect(_ui.modelRemovedCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setModelRemoved(bool)));
	connect(_ui.psResourceCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setPSResource(bool)));
	connect(_ui.lifeTimeUpdateCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setLifeTimeUpdate(bool)));
	connect(_ui.noMaxNBStepsCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setNoMaxNBSteps(bool)));
	connect(_ui.autoDelayCheckBox ,SIGNAL(toggled(bool)), this, SLOT(setAutoDelay(bool)));
	connect(_ui.animTypeComboBox ,SIGNAL(currentIndexChanged(int)), this, SLOT(setAnimType(int)));
	connect(_ui.dieComboBox ,SIGNAL(currentIndexChanged(int)), this, SLOT(setDie(int)));
	connect(_ui.afterDelaySpinBox ,SIGNAL(valueChanged(double)), this, SLOT(setAfterDelay(double)));

	enabledModifiedFlag = true;
}

CParticleSystemPage::~CParticleSystemPage()
{
}

void CParticleSystemPage::setEditedParticleSystem(CWorkspaceNode *node)
{
	_Node = node;
	enabledModifiedFlag = false;

	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	_Node->getPSPointer()->getAccurateIntegrationParams(t, max, csd, klt);

	// Update param Time threshold.
	_ui.timeThresholdWidget->setValue(t, false);

	// Update param Max steps.
	_ui.maxStepsWidget->setValue(max, false);

	// Update param User Param
	_ui.userParamWidget_1->setValue(_Node->getPSPointer()->getUserParam(0) , false);
	_ui.userParamWidget_2->setValue(_Node->getPSPointer()->getUserParam(1) , false);
	_ui.userParamWidget_3->setValue(_Node->getPSPointer()->getUserParam(2) , false);
	_ui.userParamWidget_4->setValue(_Node->getPSPointer()->getUserParam(3) , false);

	// Update param Max view dist.
	_ui.maxViewDistWidget->setValue(_Node->getPSPointer()->getMaxViewDist(), false);

	// Update param LOD Ratio.
	_ui.lodRatioWidget->setValue(_Node->getPSPointer()->getLODRatio(), false);

	// Integration
	_ui.integrationCheckBox->setChecked(_Node->getPSPointer()->isAccurateIntegrationEnabled());
	_ui.loadBalancingCheckBox->setChecked(_Node->getPSPointer()->isLoadBalancingEnabled());
	_ui.motionSlowDownCheckBox->setChecked(csd);

	// Precomputed Bbox
	_ui.enablePBBCheckBox->setChecked(!_Node->getPSPointer()->getAutoComputeBBox());

	// global color
	_ui.globalLightCheckBox->setChecked(_Node->getPSPointer()->getForceGlobalColorLightingFlag());

	_GlobalColorWrapper.PS = _Node->getPSPointer();
	_ui.colorWidget->setWorkspaceNode(_Node);
	int bGlobalColor = _Node->getPSPointer()->getColorAttenuationScheme() != NULL ?  1 : 0;
	_ui.editGlobalColorCheckBox->setChecked(bGlobalColor);
	if (bGlobalColor)
		_ui.colorWidget->updateUi();

	// Life mgt parameters
	_ui.presetBehaviourComboBox->setCurrentIndex(_Node->getPSPointer()->getBehaviourType());
	_ui.sharableCheckBox->setChecked(_Node->getPSPointer()->isSharingEnabled());
	_ui.autoLODCheckBox->setChecked(_Node->getPSPointer()->isAutoLODEnabled());

	_ui.modelRemovedCheckBox->setChecked(_Node->getPSPointer()->getDestroyModelWhenOutOfRange());
	_ui.psResourceCheckBox->setChecked(_Node->getPSPointer()->doesDestroyWhenOutOfFrustum());
	_ui.noMaxNBStepsCheckBox->setChecked(_Node->getPSPointer()->getBypassMaxNumIntegrationSteps());

	_ui.lifeTimeUpdateCheckBox->setChecked(klt);
	_ui.dieComboBox->setCurrentIndex(_Node->getPSPointer()->getDestroyCondition());
	_ui.animTypeComboBox->setCurrentIndex(_Node->getPSPointer()->getAnimType());
	_ui.autoDelayCheckBox->setChecked(_Node->getPSPointer()->getAutoComputeDelayBeforeDeathConditionTest());

	enabledModifiedFlag = true;
}

void CParticleSystemPage::updatePrecomputedBBoxParams()
{
	NLMISC::CAABBox b;
	_Node->getPSPointer()->computeBBox(b);
	_ui.xDoubleSpinBox->setValue(b.getHalfSize().x);
	_ui.yDoubleSpinBox->setValue(b.getHalfSize().y);
	_ui.zDoubleSpinBox->setValue(b.getHalfSize().z);
}

void CParticleSystemPage::updateDieOnEventParams()
{
	bool ew = _Node->getPSPointer()->getDestroyCondition() == NL3D::CParticleSystem::none ? false : true;
	_ui.autoDelayCheckBox->setEnabled(ew);
	bool autoDelay = _Node->getPSPointer()->getAutoComputeDelayBeforeDeathConditionTest();
	if (autoDelay)
		ew = false;
	_ui.afterDelaySpinBox->setEnabled(ew);
	_ui.afterDelaySpinBox->setValue(_Node->getPSPointer()->getDelayBeforeDeathConditionTest());
}

void CParticleSystemPage::updateLifeMgtPresets()
{
	bool bEnable =  _Node->getPSPointer()->getBehaviourType() == NL3D::CParticleSystem::UserBehaviour ? true :  false;

	_ui.modelRemovedCheckBox->setEnabled(bEnable);
	_ui.psResourceCheckBox->setEnabled(bEnable);
	_ui.lifeTimeUpdateCheckBox->setEnabled(bEnable);
	_ui.noMaxNBStepsCheckBox->setEnabled(bEnable);
	_ui.animTypeComboBox->setEnabled(bEnable);
	_ui.dieComboBox->setEnabled(bEnable);
	updateDieOnEventParams();
}

void CParticleSystemPage::setGlobalLight(bool state)
{
	if (state == _Node->getPSPointer()->getForceGlobalColorLightingFlag()) return;
	_Node->getPSPointer()->setForceGlobalColorLightingFlag(state);
	if (_Node && _Node->getPSModel())
	{
		_Node->getPSModel()->touchLightableState();
	}
	updateModifiedFlag();
}

void CParticleSystemPage::setLoadBalancing(bool state)
{
	if (state == _Node->getPSPointer()->isLoadBalancingEnabled()) return;
	if (state == false)
	{
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this, tr("Are you sure?"),
									  tr("Load balancing on/off"),
									  QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes)
			_Node->getPSPointer()->enableLoadBalancing(false);
		else
			_ui.loadBalancingCheckBox->setChecked(true);
	}
	else
	{
		_Node->getPSPointer()->enableLoadBalancing(true);
	}
	updateModifiedFlag();
}

void CParticleSystemPage::setIntegration(bool state)
{
	// enable/disable accurate integration.
	if (state != _Node->getPSPointer()->isAccurateIntegrationEnabled())
	{
		_Node->getPSPointer()->enableAccurateIntegration(state);
		updateModifiedFlag();
	}
	_ui.timeThresholdWidget->setEnabled(state);
	_ui.maxStepsWidget->setEnabled(state);
	_ui.motionSlowDownCheckBox->setEnabled(state);
}

void CParticleSystemPage::setMotionSlowDown(bool state)
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	_Node->getPSPointer()->getAccurateIntegrationParams(t, max, csd, klt);
	if (state == csd) return;
	_Node->getPSPointer()->setAccurateIntegrationParams(t, max, state, klt);
	updateModifiedFlag();
}

void CParticleSystemPage::setLock(bool checked)
{
	// Need frame delay dialog.
}

void CParticleSystemPage::setGloabal1()
{
	nlassert(_Node->getPSPointer());
	chooseGlobalUserParam(0, _Node->getPSPointer(), this);
	updateModifiedFlag();
}

void CParticleSystemPage::setGloabal2()
{
	chooseGlobalUserParam(1, _Node->getPSPointer(), this);
	updateModifiedFlag();
}

void CParticleSystemPage::setGloabal3()
{
	chooseGlobalUserParam(2, _Node->getPSPointer(), this);
	updateModifiedFlag();
}

void CParticleSystemPage::setGloabal4()
{
	chooseGlobalUserParam(3, _Node->getPSPointer(), this);
	updateModifiedFlag();
}

void CParticleSystemPage::setEnableBbox(bool state)
{
	if (state == _Node->getPSPointer()->getAutoComputeBBox())
	{
		_Node->getPSPointer()->setAutoComputeBBox(!state);
		updateModifiedFlag();
	}

	if (state)
		updatePrecomputedBBoxParams();
	else
		Modules::psEdit().setAutoBBox(false);
}

void CParticleSystemPage::setAutoBbox(bool state)
{
	Modules::psEdit().setAutoBBox(state);
	updateModifiedFlag();
}

void CParticleSystemPage::resetBbox()
{
	Modules::psEdit().resetAutoBBox();
	updateModifiedFlag();
}

void CParticleSystemPage::incBbox()
{
	NLMISC::CAABBox b;
	_Node->getPSPointer()->computeBBox(b);
	b.setHalfSize(1.1f * b.getHalfSize());
	_Node->getPSPointer()->setPrecomputedBBox(b);
	updatePrecomputedBBoxParams();
}

void CParticleSystemPage::decBbox()
{
	NLMISC::CAABBox b;
	_Node->getPSPointer()->computeBBox(b);
	b.setHalfSize(0.9f * b.getHalfSize());
	_Node->getPSPointer()->setPrecomputedBBox(b);
	updatePrecomputedBBoxParams();
}

void CParticleSystemPage::setXBbox(double value)
{
	NLMISC::CAABBox b;
	_Node->getPSPointer()->computeBBox(b);
	NLMISC::CVector h;
	h.x = value;
	h.y = b.getHalfSize().y;
	h.z = b.getHalfSize().z;
	b.setHalfSize(h);
	_Node->getPSPointer()->setPrecomputedBBox(b);
}

void CParticleSystemPage::setYBbox(double value)
{
	NLMISC::CAABBox b;
	_Node->getPSPointer()->computeBBox(b);
	NLMISC::CVector h;
	h.x = b.getHalfSize().x;
	h.y = value;
	h.z = b.getHalfSize().z;
	b.setHalfSize(h);
	_Node->getPSPointer()->setPrecomputedBBox(b);
}

void CParticleSystemPage::setZBbox(double value)
{
	NLMISC::CAABBox b;
	_Node->getPSPointer()->computeBBox(b);
	NLMISC::CVector h;
	h.x = b.getHalfSize().x;
	h.y = b.getHalfSize().y;
	h.z = value;
	b.setHalfSize(h);
	_Node->getPSPointer()->setPrecomputedBBox(b);
}

void CParticleSystemPage::setEditGlobalColor(bool state)
{
	bool bGlobalColor = _Node->getPSPointer()->getColorAttenuationScheme() != NULL ?  true : false;
	if (state != bGlobalColor)
	{
		// if the system hasn't a global color scheme, add one.
		if (_Node->getPSPointer()->getColorAttenuationScheme() == NULL)
		{
			static const NLMISC::CRGBA grad[] = { NLMISC::CRGBA::White, NLMISC::CRGBA::Black };
			_Node->getPSPointer()->setColorAttenuationScheme(new NL3D::CPSColorGradient(grad, 2, 64, 1.f));
			_ui.colorWidget->updateUi();
		}
		else
		{
			_Node->getPSPointer()->setColorAttenuationScheme(NULL);
		}
		updateModifiedFlag();
	}
	_ui.colorWidget->setVisible(state);
}

void CParticleSystemPage::setPresetBehaviour(int index)
{
	updateLifeMgtPresets();
	if (index == _Node->getPSPointer()->getBehaviourType()) return;
	if (index == NL3D::CParticleSystem::SpellFX ||
			index == NL3D::CParticleSystem::SpawnedEnvironmentFX)
	{
		NL3D::CPSLocatedBindable *lb;
		if (!_Node->getPSPointer()->canFinish(&lb))
		{
			_ui.presetBehaviourComboBox->setCurrentIndex(_Node->getPSPointer()->getBehaviourType());
			QErrorMessage *errorMessage = new QErrorMessage();
			errorMessage->setModal(true);
			if (!lb)
			{
				errorMessage->showMessage(tr("Can't perform operation : the system is flagged with 'No max nb steps' or uses the preset 'Spell FX', "
											 "and thus, should have a finite duration. Please remove that flag first."));
				errorMessage->exec();
			}
			else
			{
				errorMessage->showMessage(tr("The system must have a finite duration for this setting! Please check that the following object "
											 "doesn't live forever or doesn't create a loop in the system :") + QString(lb->getName().c_str()));
				errorMessage->exec();
			}
			delete errorMessage;
			return;
		}
	}
	_Node->getPSPointer()->activatePresetBehaviour((NL3D::CParticleSystem::TPresetBehaviour) index);
	updateLifeMgtPresets();
	updateModifiedFlag();
}

void CParticleSystemPage::setSharable(bool state)
{
	if (state == _Node->getPSPointer()->isSharingEnabled()) return;
	_Node->getPSPointer()->enableSharing(state);
	updateModifiedFlag();
}

void CParticleSystemPage::setAutoLOD(bool state)
{
	// performance warning
	if (state == _Node->getPSPointer()->isAutoLODEnabled()) return;
	_Node->getPSPointer()->enableAutoLOD(state);
	updateModifiedFlag();
}

void CParticleSystemPage::settings()
{
	CAutoLODDialog *dialog = new CAutoLODDialog(_Node, _Node->getPSPointer(), this);
	dialog->show();
	dialog->exec();
	delete dialog;
}

void CParticleSystemPage::setModelRemoved(bool state)
{
	if (state == _Node->getPSPointer()->getDestroyModelWhenOutOfRange()) return;
	_Node->getPSPointer()->setDestroyModelWhenOutOfRange(state);
	updateModifiedFlag();
}

void CParticleSystemPage::setPSResource(bool state)
{
	if (state != _Node->getPSPointer()->doesDestroyWhenOutOfFrustum())
	{
		_Node->getPSPointer()->destroyWhenOutOfFrustum(state);
		updateModifiedFlag();
	}
	_ui.animTypeComboBox->setEnabled(!state);
}

void CParticleSystemPage::setLifeTimeUpdate(bool state)
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	_Node->getPSPointer()->getAccurateIntegrationParams(t, max, csd, klt);
	if (klt == state) return;
	_Node->getPSPointer()->setAccurateIntegrationParams(t, max, csd, state);
	updateModifiedFlag();
}

void CParticleSystemPage::setNoMaxNBSteps(bool state)
{
	_ui.maxStepsWidget->setEnabled(!state);
	if (state == _Node->getPSPointer()->getBypassMaxNumIntegrationSteps()) return;
	if (state && !_Node->getPSPointer()->canFinish())
	{
		QErrorMessage *errorMessage = new QErrorMessage();
		errorMessage->setModal(true);
		errorMessage->showMessage(tr("The system must have a finite duration for this setting! Please check that."));
		errorMessage->exec();
		delete errorMessage;
		_ui.maxStepsWidget->setEnabled(state);
		_ui.noMaxNBStepsCheckBox->setChecked(!state);
		return;
	}
	_Node->getPSPointer()->setBypassMaxNumIntegrationSteps(state);
	updateModifiedFlag();
}

void CParticleSystemPage::setAutoDelay(bool state)
{
	_ui.afterDelaySpinBox->setEnabled(!state);
	if (state == _Node->getPSPointer()->getAutoComputeDelayBeforeDeathConditionTest()) return;
	_Node->getPSPointer()->setAutoComputeDelayBeforeDeathConditionTest(state);
	updateModifiedFlag();
}

void CParticleSystemPage::setAnimType(int index)
{
	if (index == _Node->getPSPointer()->getAnimType()) return;
	_Node->getPSPointer()->setAnimType((NL3D::CParticleSystem::TAnimType) index);
	updateModifiedFlag();
}

void CParticleSystemPage::setDie(int index)
{
	if (index != _Node->getPSPointer()->getDestroyCondition())
	{
		_Node->getPSPointer()->setDestroyCondition((NL3D::CParticleSystem::TDieCondition) index);
		updateModifiedFlag();
	}
	updateDieOnEventParams();
}

void CParticleSystemPage::setAfterDelay(double value)
{
	if (_Node->getPSPointer()->getDelayBeforeDeathConditionTest() != value)
	{
		_Node->getPSPointer()->setDelayBeforeDeathConditionTest(value);
		updateModifiedFlag();
	}
}


void CParticleSystemPage::setTimeThreshold(float value)
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	_Node->getPSPointer()->getAccurateIntegrationParams(t, max, csd, klt);
	_Node->getPSPointer()->setAccurateIntegrationParams(value, max, csd, klt);
	updateModifiedFlag();
}

void CParticleSystemPage::setMaxSteps(uint32 value)
{
	NL3D::TAnimationTime t;
	uint32 max;
	bool csd;
	bool klt;
	_Node->getPSPointer()->getAccurateIntegrationParams(t, max, csd, klt);
	_Node->getPSPointer()->setAccurateIntegrationParams(t, value, csd, klt);
	updateModifiedFlag();
}

void CParticleSystemPage::setUserParam1(float value)
{
	_Node->getPSPointer()->setUserParam(0, value);
}

void CParticleSystemPage::setUserParam2(float value)
{
	_Node->getPSPointer()->setUserParam(1, value);
}

void CParticleSystemPage::setUserParam3(float value)
{
	_Node->getPSPointer()->setUserParam(2, value);
}

void CParticleSystemPage::setUserParam4(float value)
{
	_Node->getPSPointer()->setUserParam(3, value);
}

void CParticleSystemPage::setMaxViewDist(float value)
{
	_Node->getPSPointer()->setMaxViewDist(value);
	updateModifiedFlag();
}

void CParticleSystemPage::setLodRatio(float value)
{
	_Node->getPSPointer()->setLODRatio(value);
	updateModifiedFlag();
}

CParticleSystemPage::CGlobalColorWrapper::scheme_type *CParticleSystemPage::CGlobalColorWrapper::getScheme(void) const
{
	nlassert(PS);
	return PS->getColorAttenuationScheme();
}

void CParticleSystemPage::CGlobalColorWrapper::setScheme(CParticleSystemPage::CGlobalColorWrapper::scheme_type *s)
{
	nlassert(PS);
	PS->setColorAttenuationScheme(s);
}

} /* namespace NLQT */
