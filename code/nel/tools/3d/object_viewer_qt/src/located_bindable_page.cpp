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
#include "located_bindable_page.h"

// Qt includes

// NeL includes
#include <nel/3d/particle_system_model.h>
#include "nel/3d/ps_mesh.h"

// Project includes
#include "modules.h"

namespace NLQT
{

CLocatedBindablePage::CLocatedBindablePage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	// Particle color
	_ui.colorWidget->setWrapper(&_ColorWrapper);
	_ui.colorWidget->setSchemeWrapper(&_ColorWrapper);
	_ui.colorWidget->init();

	// Particle angle 2d
	_ui.angle2DWidget->setRange(0.f, 256.f);
	_ui.angle2DWidget->setWrapper(&_Angle2DWrapper);
	_ui.angle2DWidget->setSchemeWrapper(&_Angle2DWrapper);
	_ui.angle2DWidget->init();

	// Particle size
	_ui.sizeWidget->setRange(0.0, 1.0);
	_ui.sizeWidget->setWrapper(&_SizeWrapper);
	_ui.sizeWidget->setSchemeWrapper(&_SizeWrapper);
	_ui.sizeWidget->init();

	// Particle plane bias
	_ui.particlePlaneBasicWidget->setWrapper(&_PlaneBasisWrapper);
	_ui.particlePlaneBasicWidget->setSchemeWrapper(&_PlaneBasisWrapper);
	_ui.particlePlaneBasicWidget->init();

	// Fake motion blur coeff
	_ui.blurCoeffWidget->setRange(0.0, 5.0);
	_ui.blurCoeffWidget->setWrapper(&_MotionBlurCoeffWrapper);

	// Fake motion blur threshold
	_ui.blurTresholdWidget->setRange(0.0, 5.0);
	_ui.blurTresholdWidget->setWrapper(&_MotionBlurThresholdWrapper);

	// Shock wave radius cut
	_ui.radiusCutWidget->setRange(0, 1);
	_ui.radiusCutWidget->setWrapper(&_RadiusCutWrapper);

	// Shock wave number segment
	_ui.shockWaveNbSegWidget->setRange(3, 24);
	_ui.shockWaveNbSegWidget->enableLowerBound(3, false);
	_ui.shockWaveNbSegWidget->setWrapper(&_ShockWaveNbSegWrapper);

	// Shock wave texture U factor
	_ui.shockWaveUFactorWidget->setRange(0, 5);
	_ui.shockWaveUFactorWidget->setWrapper(&_ShockWaveUFactorWrapper);

	// Num fan light
	_ui.nbFanLightWidget->setRange(3, 127);
	_ui.nbFanLightWidget->enableLowerBound(3, false);
	_ui.nbFanLightWidget->enableUpperBound(128, true);
	_ui.nbFanLightWidget->setWrapper(&_FanLightWrapper);

	// Fan light phase smoothnes
	_ui.phaseSmoothnesWidget->setRange(0, 31);
	_ui.phaseSmoothnesWidget->enableUpperBound(32, true);
	_ui.phaseSmoothnesWidget->setWrapper(&_FanLightSmoothnessWrapper);

	// Fan light speed
	_ui.fanLightSpeedWidget->setRange(0, 4.f);
	_ui.fanLightSpeedWidget->setWrapper(&_FanLightPhaseWrapper);

	// Fan light intensity
	_ui.fanLightIntensityWidget->setRange(0, 4.f);
	_ui.fanLightIntensityWidget->setWrapper(&_FanLightIntensityWrapper);

	// Tail number segs / Look At ribbon tail nb segs
	_ui.tailNbSegsWidget->setRange(2, 16);
	_ui.tailNbSegsWidget->enableLowerBound(1, true);
	_ui.tailNbSegsWidget->setWrapper(&_TailParticleWrapper);

	// Ribbon texture U factor
	_ui.ribbonTexUfactorWidget->setRange(0, 5);
	_ui.ribbonTexUfactorWidget->setWrapper(&_RibbonUFactorWrapper);

	// Ribbon texture V factor
	_ui.ribbonTexVfactorWidget->setRange(0, 5);
	_ui.ribbonTexVfactorWidget->setWrapper(&_RibbonVFactorWrapper);

	// Ribbon segment duration
	_ui.segDurationWidget->setRange(0.05f, 0.5f);
	_ui.segDurationWidget->enableLowerBound(0, true);
	_ui.segDurationWidget->setWrapper(&_SegDurationWrapper);

	// Length
	_ui.ribbonLengthWidget->setRange(0.1f, 10.1f);
	_ui.ribbonLengthWidget->setWrapper(&_RibbonLengthWrapper);
	_ui.ribbonLengthWidget->enableLowerBound(0.f, true);

	// Lod degradation
	_ui.lodDegradationWidget->setRange(0.f, 1.f);
	_ui.lodDegradationWidget->setWrapper(&_LODDegradationWrapper);
	_ui.lodDegradationWidget->enableLowerBound(0.f, false);
	_ui.lodDegradationWidget->enableUpperBound(1.f, false);

	_ui.particleTextureWidget->setWrapper(&_TextureNoAnimWrapper);
	_ui.particleTextureWidget->enableRemoveButton(true);

	hideAllWidget();

	connect(_ui.autoLodCheckBox, SIGNAL(toggled(bool)), this, SLOT(setAutoLOD(bool)));
	connect(_ui.globalColorLightingCheckBox, SIGNAL(toggled(bool)), this, SLOT(setGlobalColorLight(bool)));
	connect(_ui.independentSizeCheckBox, SIGNAL(toggled(bool)), this, SLOT(setIndependantSize(bool)));
	connect(_ui.alignCheckBox, SIGNAL(toggled(bool)), this, SLOT(setAlignOnMotion(bool)));
	connect(_ui.ztestCheckBox, SIGNAL(toggled(bool)), this, SLOT(setZTest(bool)));
	connect(_ui.zalignCheckBox, SIGNAL(toggled(bool)), this, SLOT(setZAlign(bool)));
	connect(_ui.rotationPCCheckBox, SIGNAL(toggled(bool)), this, SLOT(setHint(bool)));
	connect(_ui.blendModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setBlendMode(int)));
	connect(_ui.zbiasDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setZBias(double)));
	connect(_ui.widthRadioButton, SIGNAL(clicked()), this, SLOT(setSizeWidth()));
	connect(_ui.heightRadioButton, SIGNAL(clicked()), this, SLOT(setSizeWidth()));
	connect(_ui.rotSpeedMaxDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setRotSpeedMax(double)));
	connect(_ui.rotSpeedMinDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setRotSpeedMin(double)));
	connect(_ui.numModelsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setNumModels(int)));
	connect(_ui.useHermitteCheckBox, SIGNAL(toggled(bool)), this, SLOT(setUseHermitteInterpolation(bool)));
	connect(_ui.constantLengthCheckBox, SIGNAL(toggled(bool)), this, SLOT(setConstantLength(bool)));
	connect(_ui.coordSystemComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setTrailCoordSystem(int)));
}

CLocatedBindablePage::~CLocatedBindablePage()
{
}

void CLocatedBindablePage::setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable)
{
	_Node = ownerNode;
	_Bindable = locatedBindable;

	hideAllWidget();

	// No Auto LOD
	NL3D::CParticleSystem *ps = _Bindable->getOwner()->getOwner();
	if (ps->isSharingEnabled())
	{
		_ui.autoLodCheckBox->show();
		if (ps->isAutoLODEnabled() == false)
			_ui.autoLodCheckBox->setChecked(false);
		else
			_ui.autoLodCheckBox->setChecked(NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable)->isAutoLODDisabled());
	}
	else
		_ui.autoLodCheckBox->hide();

	// has the particle a material ?
	bool isMaterial = dynamic_cast<NL3D::CPSMaterial *>(_Bindable) != NULL;

	_ui.blendModeComboBox->setVisible(isMaterial);
	_ui.blendModeLabel->setVisible(isMaterial);
	_ui.zbiasDoubleSpinBox->setVisible(isMaterial);
	_ui.zbiasLabel->setVisible(isMaterial);
	_ui.ztestCheckBox->setVisible(isMaterial);

	if (isMaterial)
	{
		NL3D::CPSMaterial *material = dynamic_cast<NL3D::CPSMaterial *>(_Bindable);
		// blending mode
		_ui.blendModeComboBox->setCurrentIndex(material->getBlendingMode());
		// z-test
		_ui.ztestCheckBox->setChecked(material->isZTestEnabled());
		// z-bias
		_ui.zbiasDoubleSpinBox->setValue(-material->getZBias());
	}

	if (dynamic_cast<NL3D::CPSParticle *>(_Bindable))
	{
		NL3D::CPSParticle *p = (NL3D::CPSParticle *) _Bindable;

		// check support for lighting
		if (p->supportGlobalColorLighting())
		{
			_ui.globalColorLightingCheckBox->show();
			// if global color lighting is forced for all objects, don't allow to modify
			_ui.globalColorLightingCheckBox->setEnabled(!ps->getForceGlobalColorLightingFlag());
			_ui.globalColorLightingCheckBox->setChecked(p->usesGlobalColorLighting());
		}
		else
			_ui.globalColorLightingCheckBox->hide();

		// check support for color
		if (dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable))
		{
			_ColorWrapper.S = dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable);
			_ui.colorWidget->setWorkspaceNode(_Node);
			_ui.colorWidget->updateUi();

			// Add material page in tabWidget
			_ui.tabWidget->addTab(_ui.materialPage, tr("Material"));
		}

		if (dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable))
		{
			updateSizeControl();

			// Size/Angle2D page in tabWidget
			_ui.tabWidget->addTab(_ui.sizeAnglePage, tr("Size/Angle 2D"));
		}

		// check support for angle 2D
		if (dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable))
		{
			_Angle2DWrapper.S = dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable);
			_ui.angle2DWidget->setWorkspaceNode(_Node);
			_ui.angle2DWidget->updateUi();
			_ui.angle2DWidget->show();
		}
		else
			_ui.angle2DWidget->hide();

		// check support for plane basis
		if (dynamic_cast<NL3D::CPSRotated3DPlaneParticle *>(_Bindable))
		{
			_PlaneBasisWrapper.S = dynamic_cast<NL3D::CPSRotated3DPlaneParticle *>(_Bindable);
			_ui.particlePlaneBasicWidget->setWorkspaceNode(_Node);
			_ui.particlePlaneBasicWidget->updateUi();
			_ui.particlePlaneBasicWidget->setEnabled(true);

			// Add material page in tabWidget
			_ui.tabWidget->addTab(_ui.rotatePage, tr("Rotations"));
		}

		// check support for precomputed rotations
		bool isHintParticle = dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable) != NULL;
		_ui.rotationPCCheckBox->setVisible(isHintParticle);
		_ui.rotGroupBox->setVisible(isHintParticle);
		if (isHintParticle)
		{
			NL3D::CPSHintParticleRotateTheSame *rotatedParticle = dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable);
			float minValue, maxValue;
			uint32 nbModels = rotatedParticle->checkHintRotateTheSame(minValue, maxValue);
			_ui.rotationPCCheckBox->setChecked(nbModels != 0);
		}

		// if we're dealing with a face look at, motion blur can be tuned
		if (dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable))
		{
			NL3D::CPSFaceLookAt *fla = static_cast<NL3D::CPSFaceLookAt *>(_Bindable);
			_MotionBlurCoeffWrapper.P = fla;
			_MotionBlurCoeffWrapper.OwnerNode = _Node;
			_ui.blurCoeffWidget->updateUi();

			_MotionBlurThresholdWrapper.P = fla;
			_MotionBlurThresholdWrapper.OwnerNode = _Node;
			_ui.blurTresholdWidget->updateUi();

			_ui.zalignCheckBox->show();
			_ui.alignCheckBox->show();
			_ui.alignCheckBox->setChecked(fla->getAlignOnMotion());
			_ui.zalignCheckBox->setChecked(fla->getAlignOnZAxis());

			// 'look at' independant sizes
			_ui.independentSizeCheckBox->setChecked(fla->hasIndependantSizes());

			_ui.independentSizeCheckBox->show();
			_ui.independentGroupBox->show();

			// Add Look at page in tabWidget
			_ui.tabWidget->addTab(_ui.lookAtPage, tr("Look At param"));
		}
		else
		{
			_ui.independentSizeCheckBox->hide();
			_ui.independentGroupBox->hide();
		}

		// Shock wave page setup
		if (dynamic_cast<NL3D::CPSShockWave *>(_Bindable))
		{
			NL3D::CPSShockWave *sw = static_cast<NL3D::CPSShockWave *>(_Bindable);

			_RadiusCutWrapper.OwnerNode = _Node;
			_RadiusCutWrapper.S = sw;
			_ui.radiusCutWidget->updateUi();

			_ShockWaveNbSegWrapper.OwnerNode = _Node;
			_ShockWaveNbSegWrapper.S = sw;
			_ui.shockWaveNbSegWidget->updateUi();

			_ShockWaveUFactorWrapper.OwnerNode = _Node;
			_ShockWaveUFactorWrapper.S = sw;
			_ui.shockWaveUFactorWidget->updateUi();

			// Add Shock wave page in tabWidget
			_ui.tabWidget->addTab(_ui.shockWavePage, tr("Shock wave param"));
		}

		// Fan Light page setup
		if (dynamic_cast<NL3D::CPSFanLight *>(_Bindable))
		{
			_FanLightWrapper.OwnerNode = _Node;
			_FanLightWrapper.P = dynamic_cast<NL3D::CPSFanLight *>(_Bindable);
			_ui.nbFanLightWidget->updateUi();

			_FanLightSmoothnessWrapper.OwnerNode = _Node;
			_FanLightSmoothnessWrapper.P = static_cast<NL3D::CPSFanLight *>(_Bindable);
			_ui.phaseSmoothnesWidget->updateUi();

			_FanLightPhaseWrapper.OwnerNode = _Node;
			_FanLightPhaseWrapper.P = static_cast<NL3D::CPSFanLight *>(_Bindable);
			_ui.fanLightSpeedWidget->updateUi();

			_FanLightIntensityWrapper.OwnerNode = _Node;
			_FanLightIntensityWrapper.P = static_cast<NL3D::CPSFanLight *>(_Bindable);
			_ui.fanLightIntensityWidget->updateUi();

			// Add Fan Light page in tabWidget
			_ui.tabWidget->addTab(_ui.fanLightPage, tr("Fan Light param"));
		}

		// tail particle
		if (dynamic_cast<NL3D::CPSTailParticle *>(_Bindable))
		{
			if (dynamic_cast<NL3D::CPSTailDot *>(_Bindable))
				_ui.tailNbSegsWidget->enableUpperBound(256, true);

			_TailParticleWrapper.OwnerNode = _Node;
			_TailParticleWrapper.P = dynamic_cast<NL3D::CPSTailParticle *>(_Bindable);
			_ui.tailNbSegsWidget->updateUi();

			_ui.tailWidget->setCurrentTailParticle(_Node, dynamic_cast<NL3D::CPSTailParticle *>(_Bindable));

			_ui.ribbonTexUfactorWidget->hide();
			_ui.ribbonTexVfactorWidget->hide();
			_ui.tailTexUflabel->hide();
			_ui.tailTexVflabel->hide();

			// Add tail page in tabWidget
			_ui.tabWidget->addTab(_ui.ribbonTailPage, tr("Tail param"));
		}

		// shape particle
		if (dynamic_cast<NL3D::CPSShapeParticle *>(_Bindable))
		{
			_ui.meshWidget->setCurrentShape(_Node, dynamic_cast<NL3D::CPSShapeParticle *>(_Bindable));

			// Add mesh page in tabWidget
			_ui.tabWidget->addTab(_ui.meshPage, tr("Mesh param"));
		}

		// constraint mesh particle
		if (dynamic_cast<NL3D::CPSConstraintMesh *>(_Bindable))
		{
			_ui.constraintMeshWidget->setCurrentConstraintMesh(_Node, dynamic_cast<NL3D::CPSConstraintMesh *>(_Bindable));
			_ui.constraintMeshWidget->show();
		}
		else
			_ui.constraintMeshWidget->hide();


		// check support for animated texture
		if (dynamic_cast<NL3D::CPSTexturedParticle *>(_Bindable))
		{
			_ui.texAnimWidget->setCurrentTextureAnim(dynamic_cast<NL3D::CPSTexturedParticle *>(_Bindable),
					dynamic_cast<NL3D::CPSMultiTexturedParticle *>(_Bindable),
					_Node);
			_ui.texAnimWidget->show();

			_ui.tabWidget->addTab(_ui.texturePage, tr("Texture param"));
		}
		else
			_ui.texAnimWidget->hide();

		// unanimated texture
		if (dynamic_cast<NL3D::CPSTexturedParticleNoAnim *>(_Bindable))
		{
			_TextureNoAnimWrapper.TP = dynamic_cast<NL3D::CPSTexturedParticleNoAnim *>(_Bindable);
			_TextureNoAnimWrapper.OwnerNode = _Node;

			_ui.particleTextureWidget->updateUi();
			_ui.particleTextureWidget->show();

			_ui.tabWidget->addTab(_ui.texturePage, tr("Texture param"));
		}
		else
			_ui.particleTextureWidget->hide();

		// ribbon texture (doesn't support texture animation for now)
		if (dynamic_cast<NL3D::CPSRibbon *>(_Bindable))
		{
			_RibbonUFactorWrapper.OwnerNode = _Node;
			_RibbonUFactorWrapper.R = static_cast<NL3D::CPSRibbon *>(_Bindable);
			_ui.ribbonTexUfactorWidget->updateUi();

			_RibbonVFactorWrapper.OwnerNode = _Node;
			_RibbonVFactorWrapper.R = static_cast<NL3D::CPSRibbon *>(_Bindable);
			_ui.ribbonTexVfactorWidget->updateUi();

			_ui.ribbonTexUfactorWidget->show();
			_ui.ribbonTexVfactorWidget->show();
			_ui.tailTexUflabel->show();
			_ui.tailTexVflabel->show();

		}

		if (dynamic_cast<NL3D::CPSRibbonBase *>(_Bindable))
		{
			NL3D::CPSRibbonBase *ribbon = static_cast<NL3D::CPSRibbonBase *>(_Bindable);

			_SegDurationWrapper.R = ribbon;
			_ui.segDurationWidget->updateUi();

			// Length
			_RibbonLengthWrapper.OwnerNode = _Node;
			_RibbonLengthWrapper.R = ribbon;
			_ui.ribbonLengthWidget->updateUi();

			// Lod degradation
			_LODDegradationWrapper.OwnerNode = _Node;
			_LODDegradationWrapper.R = ribbon;
			_ui.lodDegradationWidget->updateUi();

			// Coord system
			_ui.coordSystemComboBox->setCurrentIndex(ribbon->getMatrixMode());
			_ui.useHermitteCheckBox->setChecked(ribbon->getInterpolationMode() == NL3D::CPSRibbonBase::Hermitte);
			_ui.constantLengthCheckBox->setChecked(ribbon->getRibbonMode() == NL3D::CPSRibbonBase::FixedSize);
		}
	}
}

void CLocatedBindablePage::setAutoLOD(bool state)
{
	NL3D::CPSParticle *p = NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable);
	nlassert(p);
	if (state != p->isAutoLODDisabled())
	{
		p->disableAutoLOD(state);
		updateModifiedFlag();
	}
}

void CLocatedBindablePage::setBlendMode(int index)
{
	NL3D::CPSMaterial *m = dynamic_cast<NL3D::CPSMaterial *>(_Bindable);
	nlassert(m);
	if (index != m->getBlendingMode())
	{
		m->setBlendingMode( (NL3D::CPSMaterial::TBlendingMode)index);
		touchPSState();
		updateModifiedFlag();
	}
}

void CLocatedBindablePage::setGlobalColorLight(bool state)
{
	NL3D::CPSParticle *p = NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable);
	nlassert(p);
	if (state != p->usesGlobalColorLighting())
	{
		p->enableGlobalColorLighting(state);
		touchPSState();
		updateModifiedFlag();
	}
}

void CLocatedBindablePage::setIndependantSize(bool state)
{
	NL3D::CPSFaceLookAt *la = static_cast<NL3D::CPSFaceLookAt *>(_Bindable);
	nlassert(la);
	if (state != la->hasIndependantSizes())
	{
		la->setIndependantSizes(state);
		updateModifiedFlag();
	}
	_ui.independentGroupBox->setEnabled(state);
	updateSizeControl();
}

void CLocatedBindablePage::setAlignOnMotion(bool state)
{
	NL3D::CPSFaceLookAt *fla = NLMISC::safe_cast<NL3D::CPSFaceLookAt *>(_Bindable);
	nlassert(fla);
	if (state != fla->getAlignOnMotion())
	{
		fla->setAlignOnMotion(state);
		updateModifiedFlag();
	}
	updateValidWidgetForAlignOnMotion(!state);
}

void CLocatedBindablePage::setZTest(bool state)
{
	NL3D::CPSMaterial *mat = dynamic_cast<NL3D::CPSMaterial *>(_Bindable);
	nlassert(mat);
	if (state != mat->isZTestEnabled())
	{
		mat->enableZTest(state);
		updateModifiedFlag();
	}
}

void CLocatedBindablePage::setZAlign(bool state)
{
	NL3D::CPSFaceLookAt *fla = NLMISC::safe_cast<NL3D::CPSFaceLookAt *>(_Bindable);
	nlassert(fla);
	if (state != fla->getAlignOnZAxis())
	{
		fla->setAlignOnZAxis(state);
		updateModifiedFlag();
	}
}

void CLocatedBindablePage::setZBias(double value)
{
	NLMISC::safe_cast<NL3D::CPSParticle *>(_Bindable)->setZBias(-value);
}

void CLocatedBindablePage::setSizeWidth()
{
	updateSizeControl();
}

void CLocatedBindablePage::setHint(bool state)
{
	NL3D::CPSHintParticleRotateTheSame *rotatedParticle = dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable);
	float minVelocity, maxVelocity;
	uint32 nbModels = rotatedParticle->checkHintRotateTheSame(minVelocity, maxVelocity);
	nlassert(rotatedParticle);
	if ((nbModels != 0) != state)
	{
		if (state)
			_ui.numModelsSpinBox->setValue(32);
		else
			rotatedParticle->disableHintRotateTheSame();
		updateModifiedFlag();
	}
	_ui.rotGroupBox->setEnabled(state);
	_ui.particlePlaneBasicWidget->setEnabled(!state);
	rotatedParticle->checkHintRotateTheSame(minVelocity, maxVelocity);
	_ui.rotSpeedMaxDoubleSpinBox->setValue(maxVelocity);
	_ui.rotSpeedMinDoubleSpinBox->setValue(minVelocity);
}

void CLocatedBindablePage::setRotSpeedMax(double value)
{
	NL3D::CPSHintParticleRotateTheSame *rotatedParticle = dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable);
	nlassert(rotatedParticle);
	float valueMin, valueMax;
	uint32 nbModels = rotatedParticle->checkHintRotateTheSame(valueMin, valueMax);
	if (valueMax != value)
	{
		rotatedParticle->hintRotateTheSame(nbModels, valueMin, value);
		updateModifiedFlag();
	}
}

void CLocatedBindablePage::setRotSpeedMin(double value)
{
	NL3D::CPSHintParticleRotateTheSame *rotatedParticle = dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable);
	nlassert(rotatedParticle);
	float valueMin, valueMax;
	uint32 nbModels = rotatedParticle->checkHintRotateTheSame(valueMin, valueMax);
	if (valueMin != value)
	{
		rotatedParticle->hintRotateTheSame(nbModels, value, valueMax);
		updateModifiedFlag();
	}
}

void CLocatedBindablePage::setNumModels(int value)
{
	NL3D::CPSHintParticleRotateTheSame *rotatedParticle = dynamic_cast<NL3D::CPSHintParticleRotateTheSame *>(_Bindable);
	nlassert(rotatedParticle);
	float valueMin, valueMax;
	sint32 nbModels;

	nbModels = rotatedParticle->checkHintRotateTheSame(valueMin, valueMax);
	if (nbModels != value)
	{
		rotatedParticle->hintRotateTheSame((uint32) value, valueMin, valueMax);
		updateModifiedFlag();
	}
}

void CLocatedBindablePage::setUseHermitteInterpolation(bool state)
{
	NL3D::CPSRibbonBase *ribbon = dynamic_cast<NL3D::CPSRibbonBase *>(_Bindable);
	ribbon->setInterpolationMode(state ? NL3D::CPSRibbonBase::Hermitte : NL3D::CPSRibbonBase::Linear);
}

void CLocatedBindablePage::setConstantLength(bool state)
{
	NL3D::CPSRibbonBase *ribbon = dynamic_cast<NL3D::CPSRibbonBase *>(_Bindable);
	ribbon->setRibbonMode(state ? NL3D::CPSRibbonBase::FixedSize : NL3D::CPSRibbonBase::VariableSize);
	_ui.ribbonLengthWidget->setEnabled(state);
}

void CLocatedBindablePage::setTrailCoordSystem(int index)
{
	NL3D::CPSRibbonBase *ribbon = dynamic_cast<NL3D::CPSRibbonBase *>(_Bindable);
	if (index != ribbon->getMatrixMode())
	{
		ribbon->setMatrixMode((NL3D::CPSRibbonBase::TMatrixMode) index);
		updateModifiedFlag();
	}
}

void CLocatedBindablePage::updateValidWidgetForAlignOnMotion(bool align)
{
	_ui.blurCoeffLabel->setEnabled(align);
	_ui.blurCoeffWidget->setEnabled(align);
	_ui.blurTreshholdLabel->setEnabled(align);
	_ui.blurTresholdWidget->setEnabled(align);
	_ui.zalignCheckBox->setEnabled(align);
}

void CLocatedBindablePage::updateSizeControl()
{
	if (!dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable)) return;

	NL3D::CPSFaceLookAt *fla = dynamic_cast<NL3D::CPSFaceLookAt *>(_Bindable);
	// LookAt case
	if (fla && fla->hasIndependantSizes())
	{
		if (_ui.widthRadioButton->isChecked())
			_ui.sizeWidget->setTitle(tr("Width"));
		else
			_ui.sizeWidget->setTitle(tr("Height"));

		if (_ui.widthRadioButton->isChecked()) // wrap to the wanted size
			_SizeWrapper.S = fla;
		else
			_SizeWrapper.S = &fla->getSecondSize();
	}
	else // general case. Wrap to the size interface and the appropriate dialog
	{
		_SizeWrapper.S = dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable);
		_ui.sizeWidget->setTitle(tr("Size"));
	}

	_ui.sizeWidget->setWorkspaceNode(_Node);
	_ui.sizeWidget->updateUi();
}

void CLocatedBindablePage::hideAllWidget()
{
	for (int i = 0; i < _ui.tabWidget->count(); i++)
		_ui.tabWidget->widget(i)->hide();

	while (_ui.tabWidget->count() != 0)
		_ui.tabWidget->removeTab(_ui.tabWidget->count() - 1);
}

void CLocatedBindablePage::touchPSState()
{
	if (_Node && _Node->getPSModel())
	{
		_Node->getPSModel()->touchTransparencyState();
		_Node->getPSModel()->touchLightableState();
	}
}

} /* namespace NLQT */
