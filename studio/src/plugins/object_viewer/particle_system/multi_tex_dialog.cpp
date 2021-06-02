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
#include "multi_tex_dialog.h"

// NeL includes
#include "nel/3d/ps_particle_basic.h"
#include "nel/3d/texture_bump.h"

// Project includes
#include "particle_node.h"

namespace NLQT
{

CMultiTexDialog::CMultiTexDialog(CWorkspaceNode *ownerNode, NL3D::CPSMultiTexturedParticle *mtp, QWidget *parent)
	: QDialog(parent),
	  _Node(ownerNode),
	  _MTP(mtp)
{
	_ui.setupUi(this);

	nlassert(_MTP);

	_ui.basicCapsCheckBox->setChecked(NL3D::CPSMultiTexturedParticle::areBasicCapsForced());
	_ui.useParticleDataCheckBox->setChecked(_MTP->getUseLocalDateAlt());
	_ui.useParticleDataCheckBox_2->setChecked(_MTP->getUseLocalDate());

	bool bEnvBumpMapUsed = _MTP->getMainTexOp() == NL3D::CPSMultiTexturedParticle::EnvBumpMap ? true : false;
	_ui.bumpFactorLabel->setEnabled(bEnvBumpMapUsed);
	_ui.bumpFactorDoubleSpinBox->setEnabled(bEnvBumpMapUsed);

	_TexWrapper.OwnerNode = _Node;
	_AlternateTexWrapper.OwnerNode = _Node;
	_TexWrapper.MTP = _MTP;
	_AlternateTexWrapper.MTP = _MTP;

	_ui.texWidget->setWrapper(&_TexWrapper);
	_ui.texWidget->updateUi();

	_ui.texWidget_2->setWrapper(&_AlternateTexWrapper);
	_ui.texWidget_2->updateUi();

	readValues();

	_ui.enableAlternateCheckBox->setChecked(_MTP->isAlternateTexEnabled());
	_ui.alternateTab->setEnabled(_MTP->isAlternateTexEnabled());

	_ui.texOpComboBox->setCurrentIndex(int(_MTP->getMainTexOp()));
	_ui.texOpComboBox_2->setCurrentIndex(int(_MTP->getAlternateTexOp()));

	connect(_ui.bumpFactorDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateValues()));
	connect(_ui.enableAlternateCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEnabledAlternate(bool)));
	connect(_ui.basicCapsCheckBox, SIGNAL(toggled(bool)), this, SLOT(setForceBasicCaps(bool)));
	connect(_ui.texOpComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setMainOp(int)));
	connect(_ui.texOpComboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(setAlternateOp(int)));
	connect(_ui.useParticleDataCheckBox, SIGNAL(toggled(bool)), this, SLOT(setUseParticleDate(bool)));
	connect(_ui.useParticleDataCheckBox_2, SIGNAL(toggled(bool)), this, SLOT(setUseParticleDateAlt(bool)));
	connect(_ui.uSpeed1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateValues()));
	connect(_ui.vSpeed1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateValues()));
	connect(_ui.uSpeed2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateValues()));
	connect(_ui.vSpeed2DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateValues()));
	connect(_ui.uSpeed1DoubleSpinBox_2, SIGNAL(valueChanged(double)), this, SLOT(updateValuesAlternate()));
	connect(_ui.vSpeed1DoubleSpinBox_2, SIGNAL(valueChanged(double)), this, SLOT(updateValuesAlternate()));
	connect(_ui.uSpeed2DoubleSpinBox_2, SIGNAL(valueChanged(double)), this, SLOT(updateValuesAlternate()));
	connect(_ui.vSpeed2DoubleSpinBox_2, SIGNAL(valueChanged(double)), this, SLOT(updateValuesAlternate()));
}

CMultiTexDialog::~CMultiTexDialog()
{
}

void CMultiTexDialog::setEnabledAlternate(bool state)
{
	if (state != _MTP->isAlternateTexEnabled())
	{
		_MTP->enableAlternateTex(state);
		updateModifiedFlag();
	}
	_ui.alternateTab->setEnabled(state);
}

void CMultiTexDialog::updateValues()
{
	NLMISC::CVector2f vs1, vs2;

	vs1.x = _ui.uSpeed1DoubleSpinBox->value();
	vs1.y = _ui.vSpeed1DoubleSpinBox->value();

	vs2.x = _ui.uSpeed2DoubleSpinBox->value();
	vs2.y = _ui.vSpeed2DoubleSpinBox->value();

	_MTP->setScrollSpeed(0, vs1);
	_MTP->setScrollSpeed(1, vs2);

	_MTP->setBumpFactor(_ui.bumpFactorDoubleSpinBox->value());

	updateModifiedFlag();
}

void CMultiTexDialog::updateValuesAlternate()
{
	NLMISC::CVector2f vs1, vs2;

	vs1.x = _ui.uSpeed1DoubleSpinBox_2->value();
	vs1.y = _ui.vSpeed1DoubleSpinBox_2->value();

	vs2.x = _ui.uSpeed2DoubleSpinBox_2->value();
	vs2.y = _ui.vSpeed2DoubleSpinBox_2->value();

	_MTP->setAlternateScrollSpeed(0, vs1);
	_MTP->setAlternateScrollSpeed(1, vs2);

	updateModifiedFlag();
}

void CMultiTexDialog::setAlternateOp(int index)
{
	_MTP->setAlternateTexOp((NL3D::CPSMultiTexturedParticle::TOperator) index);
	updateModifiedFlag();
}

void CMultiTexDialog::setMainOp(int index)
{
	_MTP->setMainTexOp((NL3D::CPSMultiTexturedParticle::TOperator) index);
	updateModifiedFlag();

	bool bEnvBumpMapUsed = _MTP->getMainTexOp() == NL3D::CPSMultiTexturedParticle::EnvBumpMap ? true : false;
	_ui.bumpFactorLabel->setEnabled(bEnvBumpMapUsed);
	_ui.bumpFactorDoubleSpinBox->setEnabled(bEnvBumpMapUsed);
}

void CMultiTexDialog::setForceBasicCaps(bool state)
{
	NL3D::CPSMultiTexturedParticle::forceBasicCaps(state);
}

void CMultiTexDialog::setUseParticleDate(bool state)
{
	_MTP->setUseLocalDate(state);
	updateModifiedFlag();
}

void CMultiTexDialog::setUseParticleDateAlt(bool state)
{
	_MTP->setUseLocalDateAlt(state);
	updateModifiedFlag();
}

void CMultiTexDialog::readValues()
{
	_ui.uSpeed1DoubleSpinBox->setValue(_MTP->getScrollSpeed(0).x);
	_ui.vSpeed1DoubleSpinBox->setValue(_MTP->getScrollSpeed(0).y);
	_ui.uSpeed2DoubleSpinBox->setValue(_MTP->getScrollSpeed(1).x);
	_ui.vSpeed2DoubleSpinBox->setValue(_MTP->getScrollSpeed(1).y);
	_ui.uSpeed1DoubleSpinBox_2->setValue(_MTP->getAlternateScrollSpeed(0).x);
	_ui.vSpeed1DoubleSpinBox_2->setValue(_MTP->getAlternateScrollSpeed(0).y);
	_ui.uSpeed2DoubleSpinBox_2->setValue(_MTP->getAlternateScrollSpeed(1).x);
	_ui.vSpeed2DoubleSpinBox_2->setValue(_MTP->getAlternateScrollSpeed(1).y);
	_ui.bumpFactorDoubleSpinBox->setValue(_MTP->getBumpFactor());
}

NL3D::ITexture *CMultiTexDialog::CMainTexWrapper::get(void)
{
	return MTP->getTexture2();
}

void CMultiTexDialog::CMainTexWrapper::set(NL3D::ITexture *tex)
{
	MTP->setTexture2(tex);
}

NL3D::ITexture *CMultiTexDialog::CAlternateTexWrapper::get(void)
{
	return MTP->getTexture2Alternate();
}

void CMultiTexDialog::CAlternateTexWrapper::set(NL3D::ITexture *tex)
{
	MTP->setTexture2Alternate(tex);
}

} /* namespace NLQT */