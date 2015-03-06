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
#include "attrib_widget.h"

// Qt includes
#include <QtGui/QInputDialog>

// NeL includes
#include <nel/3d/ps_attrib_maker.h>
#include <nel/3d/ps_float.h>
#include <nel/3d/ps_int.h>
#include <nel/3d/ps_color.h>
#include <nel/3d/ps_plane_basis.h>
#include <nel/3d/ps_plane_basis_maker.h>

// Projects includes
#include "value_blender_dialog.h"
#include "value_gradient_dialog.h"
#include "bin_op_dialog.h"
#include "curve_dialog.h"
#include "value_from_emitter_dialog.h"
#include "spinner_dialog.h"
#include "follow_path_dialog.h"
#include "scheme_bank_dialog.h"

namespace NLQT
{

CAttribWidget::CAttribWidget(QWidget *parent)
	: QGroupBox(parent),
	  _SrcInputEnabled(true),
	  _EnableConstantValue(true),
	  _DisableMemoryScheme(false),
	  _NbCycleEnabled(true),
	  _Node(NULL),
	  _SchemeWidget(NULL)
{
	_ui.setupUi(this);
	_ui.constRangeUIntWidget->hide();
	_ui.constRangeFloatWidget->hide();
	_ui.constAttribPlaneWidget->hide();
	_ui.constRangeIntWidget->hide();
	_ui.constRGBAWidget->hide();
	_ui.schemeWidget->hide();
	_ui.schemeEditWidget->hide();

	_ui.inMultiplierWidget->setRange(0.1f, 10.1f);
	_ui.inMultiplierWidget->enableLowerBound(0, true);;
	_ui.inMultiplierWidget->setWrapper(&_NbCyclesWrapper);
	_NbCyclesWrapper.widget = this;
}

CAttribWidget::~CAttribWidget()
{
}

void CAttribWidget::setEnabledConstantValue(bool enableConstantValue)
{
	_EnableConstantValue = enableConstantValue;
}

void CAttribWidget::init()
{
	connect(_ui.editPushButton, SIGNAL(clicked()), this, SLOT(clickedEdit()));
	connect(_ui.clampCheckBox, SIGNAL(toggled(bool)), this, SLOT(setClamp(bool)));
	connect(_ui.schemeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCurrentScheme(int)));
	connect(_ui.srcComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentSrc(int)));
	connect(_ui.userParamPushButton, SIGNAL(clicked()), this, SLOT(setUserIndex()));
	connect(_ui.bankButton, SIGNAL(clicked()), this, SLOT(openSchemeBankDialog()));
	connect(_ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeUseScheme(int)));
}

void CAttribWidget::updateUi()
{
	if  (!_EnableConstantValue)
		_ui.comboBox->hide();
	else
		_ui.comboBox->show();

	if (useScheme())
	{
		if (_ui.comboBox->currentIndex() == 1)
			schemeValueUpdate();
		else
			_ui.comboBox->setCurrentIndex(1);
	}
	else
	{

		nlassert(_EnableConstantValue);
		if (_ui.comboBox->currentIndex() == 0)
			cstValueUpdate();
		else
			_ui.comboBox->setCurrentIndex(0);
	}
}

void CAttribWidget::clickedEdit()
{
	QDialog *dialog = editScheme();
	if (dialog != NULL)
	{
		dialog->setModal(true);
		dialog->show();
		dialog->exec();
		delete dialog;
	}
}

void CAttribWidget::setClamp(bool state)
{
	// avoid performance warning
	if (state != isSchemeClamped())
		clampScheme(state);
}

void CAttribWidget::changeCurrentScheme(int index)
{
	if (getCurrentScheme() != index)
		setCurrentScheme(uint(index));
	schemeValueUpdate();
}

void CAttribWidget::setCurrentSrc(int index)
{
	NL3D::CPSInputType it;
	it.InputType = (NL3D::CPSInputType::TInputType) index;
	if (it.InputType != getSchemeInput().InputType)
	{
		if (it.InputType == NL3D::CPSInputType::attrUserParam)
		{
			it.UserParamNum = 0;
		}
		setSchemeInput(it);
	}
	inputValueUpdate();
}

void CAttribWidget::setUserIndex()
{
	bool ok;
	int i = QInputDialog::getInt(this, tr("Set user param"), tr(""), getSchemeInput().UserParamNum + 1, 1, 4, 1, &ok);
	if (ok)
	{
		NL3D::CPSInputType it =  getSchemeInput();
		it.UserParamNum = i - 1;
		setSchemeInput(it);
	}
	inputValueUpdate();
}

void CAttribWidget::changeUseScheme(int index)
{
	if (index == 0)
	{
		if (useScheme())
			resetCstValue(); // change constant
		cstValueUpdate(); // update ui
	}
	else
	{
		if (useScheme())
			changeCurrentScheme(getCurrentScheme()); // update ui
		else
			changeCurrentScheme(0); // change scheme
	}
}

void CAttribWidget::openSchemeBankDialog()
{
	CSchemeBankDialog dialog(this);
	dialog.setModal(true);
	dialog.show();
	dialog.exec();
	updateUi();
}

void CAttribWidget::inputValueUpdate(void)
{
	if (useScheme() && getSchemeInput().InputType == NL3D::CPSInputType::attrUserParam)
	{
		//_ui.userParamPushButton->setText(tr("User param: %1").arg(getSchemeInput().UserParamNum + 1));
		_ui.userParamPushButton->setEnabled(true);
	}
	else
	{
		//_ui.userParamPushButton->setText(tr("User param:"));
		_ui.userParamPushButton->setEnabled(false);
	}
}

void CAttribWidget::schemeValueUpdate()
{
	if (!useScheme()) return;

	_ui.constRangeUIntWidget->hide();
	_ui.constRangeFloatWidget->hide();
	_ui.constAttribPlaneWidget->hide();
	_ui.constRangeIntWidget->hide();
	_ui.constRGBAWidget->hide();

	_ui.schemeWidget->show();
	_ui.schemeEditWidget->show();
	sint k = getCurrentScheme();

	if (k == -1) // unknow scheme ...
	{
		_ui.schemeComboBox->setCurrentIndex(k);
		k = 0;
	}

	if (k != _ui.schemeComboBox->currentIndex())
		_ui.schemeComboBox->setCurrentIndex(k);

	if (hasSchemeCustomInput() && _SrcInputEnabled)
	{
		_ui.srcLabel->setEnabled(true);
		_ui.srcComboBox->setEnabled(true);
		_ui.srcComboBox->setCurrentIndex(int(getSchemeInput().InputType));

		_ui.clampCheckBox->setEnabled(isClampingSupported());
		_ui.inMultiplierWidget->setEnabled(isClampingSupported());
		_ui.inputLabel->setEnabled(isClampingSupported());
	}
	else
	{
		_ui.srcLabel->setEnabled(false);
		_ui.srcComboBox->setEnabled(false);
		inputValueUpdate();

		_ui.clampCheckBox->setEnabled(false);
		_ui.inMultiplierWidget->setEnabled(false);
		_ui.inputLabel->setEnabled(false);
	}

	if (_NbCycleEnabled)
	{
		_ui.inMultiplierWidget->updateUi();
		_ui.inMultiplierWidget->show();
		_ui.inputLabel->show();
		_ui.inMultiplierWidget->setEnabled(true);
		_ui.clampCheckBox->show();
	}
	else
	{
		_ui.inputLabel->hide();
		_ui.inMultiplierWidget->hide();
		_ui.clampCheckBox->hide();
	}

	if (isClampingSupported())
		_ui.clampCheckBox->setChecked(isSchemeClamped());
}

void CAttribWidget::enableMemoryScheme(bool enabled)
{
	_DisableMemoryScheme = !enabled;
	if (!enabled)
	{
		_ui.schemeComboBox->removeItem(_ui.schemeComboBox->count() - 1);
		_ui.schemeComboBox->removeItem(_ui.schemeComboBox->count() - 1);
	}
}

CAttribFloatWidget::CAttribFloatWidget(QWidget *parent)
	: CAttribWidgetT<float>(parent)
{
	_ui.schemeComboBox->addItem(tr("value blender"));
	_ui.schemeComboBox->addItem(tr("values gradient"));
	_ui.schemeComboBox->addItem(tr("curve"));
	_ui.schemeComboBox->addItem(tr("value computed from emitter"));
	_ui.schemeComboBox->addItem(tr("binary operator"));
}

CAttribFloatWidget::~CAttribFloatWidget()
{
}

void CAttribFloatWidget::setRange(float minValue, float maxValue)
{
	_MinRange = minValue;
	_MaxRange = maxValue;
	_ui.constRangeFloatWidget->setRange(_MinRange, _MaxRange);
}

void CAttribFloatWidget::setWrapper(IPSWrapper<float> *wrapper)
{
	nlassert(wrapper);
	_Wrapper = wrapper;
	_ui.constRangeFloatWidget->setWrapper(_Wrapper);
}

QDialog *CAttribFloatWidget::editScheme(void)
{
	NL3D::CPSAttribMaker<float> *scheme = _SchemeWrapper->getScheme();

	if (dynamic_cast<NL3D::CPSFloatBlender *>(scheme))
	{
		CFloatBlenderDialogClient *myInterface = new CFloatBlenderDialogClient();
		myInterface->MinRange = _MinRange;
		myInterface->MaxRange = _MaxRange;
		myInterface->SchemeFunc = & ((NL3D::CPSValueBlenderSample<float, 64> *) scheme)->_F;
		CValueBlenderDialog *vb = new CValueBlenderDialog(myInterface, _Node, true, this);
		return vb;
	}
	if (dynamic_cast<NL3D::CPSFloatGradient *>(scheme))
	{
		CFloatGradientWrapper *wrapper = new CFloatGradientWrapper;
		wrapper->m_minRange = _MinRange;
		wrapper->m_maxRange = _MaxRange;
		wrapper->m_scheme = &(((NL3D::CPSFloatGradient *) (_SchemeWrapper->getScheme()) )->_F);
		CGradientDialog *gd = new CGradientDialog(_Node, wrapper, true, true, 2, this);
		wrapper->m_defaultValue = 0.f;
		return gd;
	}
	if (dynamic_cast<NL3D::CPSFloatMemory *>(scheme))
	{
		CAttribFloatWidget *adf = new CAttribFloatWidget();
		adf->setRange(_MinRange, _MaxRange);
		CValueFromEmitterDialogT<float> *vfe = new CValueFromEmitterDialogT<float>( (NL3D::CPSFloatMemory *)(scheme),
				adf,
				this);
		vfe->init();
		adf->setWorkspaceNode(_Node);
		adf->updateUi();
		return vfe;
	}
	if (dynamic_cast<NL3D::CPSFloatBinOp *>(scheme))
	{
		CAttribFloatWidget *ad[2] = { NULL, NULL};
		for (uint k = 0; k <2; ++k)
		{
			ad[k] = new CAttribFloatWidget();
			ad[k]->setRange(_MinRange, _MaxRange);
		}
		CBinOpDialogT<float> *bod = new CBinOpDialogT<float>( (NL3D::CPSFloatBinOp *)(scheme),
				(CAttribWidgetT<float> **) ad,
				this);
		bod->init();
		for (uint k = 0; k <2; ++k)
		{
			ad[k]->setWorkspaceNode(_Node);
			ad[k]->updateUi();
		}
		return bod;
	}
	if (dynamic_cast<NL3D::CPSFloatCurve *>(scheme))
	{
		CurveEditDialog *curve = new CurveEditDialog(&(dynamic_cast<NL3D::CPSFloatCurve *>(scheme)->_F), _Node, this);
		return curve;
	}
	return NULL;
}

void CAttribFloatWidget::setCurrentScheme(uint index)
{
	nlassert(index < 5);
	NL3D::CPSAttribMaker<float> *scheme = NULL;

	switch (index)
	{
	case 0:
		scheme = new NL3D::CPSFloatBlender(_MinRange, _MaxRange);
		break;
	case 1:
	{
		static const float values[2] = { 0.1f, 1.f };
		scheme = new NL3D::CPSFloatGradient(values, 2, 16, 1.f);
	}
	break;
	case 2:
	{
		NL3D::CPSFloatCurve *curve = new NL3D::CPSFloatCurve;
		curve->_F.setNumSamples(128);
		curve->_F.addControlPoint(NL3D::CPSFloatCurveFunctor::CCtrlPoint(0, 0.5f));
		curve->_F.addControlPoint(NL3D::CPSFloatCurveFunctor::CCtrlPoint(1, 0.5f));
		scheme = curve;
	}
	break;
	case 3:
		scheme = new NL3D::CPSFloatMemory;
		((NL3D::CPSAttribMakerMemory<float> *) scheme)->setScheme(new NL3D::CPSFloatBlender(_MinRange, _MaxRange));
		break;
	case 4 :
		scheme = new NL3D::CPSFloatBinOp;
		((NL3D::CPSFloatBinOp *) scheme)->setArg(0, new NL3D::CPSFloatBlender);
		((NL3D::CPSFloatBinOp *) scheme)->setArg(1, new NL3D::CPSFloatBlender);
		break;
	default:
		break;
	}

	if (scheme)
	{
		_SchemeWrapper->setSchemeAndUpdateModifiedFlag(scheme);
	}
}

sint CAttribFloatWidget::getCurrentScheme(void) const
{
	const NL3D::CPSAttribMaker<float> *scheme = _SchemeWrapper->getScheme();

	if (dynamic_cast<const NL3D::CPSFloatBlender *>(scheme))
		return 0;
	if (dynamic_cast<const NL3D::CPSFloatGradient *>(scheme))
		return 1;
	if (dynamic_cast<const NL3D::CPSFloatCurve *>(scheme))
		return 2;
	if (dynamic_cast<const NL3D::CPSFloatMemory *>(scheme))
		return 3;
	if (dynamic_cast<const NL3D::CPSFloatBinOp *>(scheme))
		return 4;

	return -1;
}

void CAttribFloatWidget::cstValueUpdate()
{
	_ui.schemeWidget->hide();
	_ui.schemeEditWidget->hide();
	_ui.inMultiplierWidget->setEnabled(false);
	_ui.clampCheckBox->setEnabled(false);
	_ui.inputLabel->setEnabled(false);
	_ui.constRangeFloatWidget->show();
	_ui.constRangeFloatWidget->updateUi();
}

CAttribUIntWidget::CAttribUIntWidget(QWidget *parent)
	: CAttribWidgetT<uint32>(parent)
{
	_ui.schemeComboBox->addItem(tr("value blender"));
	_ui.schemeComboBox->addItem(tr("values gradient"));
	_ui.schemeComboBox->addItem(tr("value computed from emitter"));
	_ui.schemeComboBox->addItem(tr("binary operator"));
}

CAttribUIntWidget::~CAttribUIntWidget()
{
}

void CAttribUIntWidget::setRange(uint32 minValue, uint32 maxValue)
{
	_MinRange = minValue;
	_MaxRange = maxValue;
	_ui.constRangeUIntWidget->setRange(_MinRange, _MaxRange);
}

void CAttribUIntWidget::setWrapper(IPSWrapper<uint32> *wrapper)
{
	nlassert(wrapper);
	_Wrapper = wrapper;
	_ui.constRangeUIntWidget->setWrapper(_Wrapper);
}

QDialog *CAttribUIntWidget::editScheme(void)
{
	const NL3D::CPSAttribMaker<uint32> *scheme = _SchemeWrapper->getScheme();

	if (dynamic_cast<const NL3D::CPSUIntBlender *>(scheme))
	{
		CUIntBlenderDialogClient *myInterface = new CUIntBlenderDialogClient();
		myInterface->MinRange = _MinRange;
		myInterface->MaxRange = _MaxRange;
		myInterface->SchemeFunc = & ((NL3D::CPSValueBlenderSample<uint32, 64> *) scheme)->_F;
		CValueBlenderDialog *vb = new CValueBlenderDialog(myInterface, _Node, true, this);
		return vb;
	}
	if (dynamic_cast<const NL3D::CPSUIntGradient *>(scheme))
	{
		CUIntGradientWrapper *wrapper = new CUIntGradientWrapper;
		wrapper->m_minRange = _MinRange;
		wrapper->m_maxRange = _MaxRange;
		wrapper->m_scheme = &(((NL3D::CPSUIntGradient *) (_SchemeWrapper->getScheme()) )->_F);
		CGradientDialog *gd = new CGradientDialog(_Node, wrapper, true, true, 2, this);
		wrapper->m_defaultValue = 0;
		return gd;

	}
	if (dynamic_cast<const NL3D::CPSUIntMemory *>(scheme))
	{
		CAttribUIntWidget *adu = new CAttribUIntWidget();
		adu->setRange(_MinRange, _MaxRange);
		CValueFromEmitterDialogT<uint32> *vfe = new CValueFromEmitterDialogT<uint32>( (NL3D::CPSUIntMemory *)(scheme),
				adu,
				this);
		vfe->init();
		adu->setWorkspaceNode(_Node);
		adu->updateUi();
		return vfe;
	}
	if (dynamic_cast<const NL3D::CPSUIntBinOp *>(scheme))
	{
		CAttribUIntWidget *ad[2] = { NULL, NULL};
		for (uint k = 0; k <2; ++k)
		{
			ad[k] = new CAttribUIntWidget();
			ad[k]->setRange(_MinRange, _MaxRange);
		}
		CBinOpDialogT<uint32> *bod = new CBinOpDialogT<uint32>( (NL3D::CPSUIntBinOp *)(scheme),
				(CAttribWidgetT<uint32> **) ad,
				this);
		bod->init();
		for (uint k = 0; k <2; ++k)
		{
			ad[k]->setWorkspaceNode(_Node);
			ad[k]->updateUi();
		}
		return bod;
	}
	return NULL;
}

void CAttribUIntWidget::setCurrentScheme(uint index)
{
	nlassert(index < 4);
	NL3D::CPSAttribMaker<uint32> *scheme = NULL;

	switch (index)
	{
	case 0 :
		scheme = new NL3D::CPSUIntBlender(_MinRange, _MaxRange);
		break;
	case 1 :
		scheme = new NL3D::CPSUIntGradient;
		break;
	case 2 :
		scheme = new NL3D::CPSUIntMemory;
		((NL3D::CPSAttribMakerMemory<uint32> *) scheme)->setScheme(new NL3D::CPSUIntBlender(_MinRange, _MaxRange) );
		break;
	case 3 :
		scheme = new NL3D::CPSUIntBinOp;
		((NL3D::CPSUIntBinOp *) scheme)->setArg(0, new NL3D::CPSUIntBlender);
		((NL3D::CPSUIntBinOp *) scheme)->setArg(1, new NL3D::CPSUIntBlender);
		break;
	default:
		break;
	}
	if (scheme)
	{
		_SchemeWrapper->setSchemeAndUpdateModifiedFlag(scheme);
	}
}

sint CAttribUIntWidget::getCurrentScheme(void) const
{
	const NL3D::CPSAttribMaker<uint32> *scheme = _SchemeWrapper->getScheme();

	if (dynamic_cast<const NL3D::CPSUIntBlender *>(scheme))  return 0;
	if (dynamic_cast<const NL3D::CPSUIntGradient *>(scheme)) return 1;
	if (dynamic_cast<const NL3D::CPSUIntMemory *>(scheme))   return 2;
	if (dynamic_cast<const NL3D::CPSUIntBinOp *>(scheme))	 return 3;
	return -1;
}

void CAttribUIntWidget::cstValueUpdate()
{
	_ui.schemeWidget->hide();
	_ui.schemeEditWidget->hide();
	_ui.inMultiplierWidget->setEnabled(false);
	_ui.clampCheckBox->setEnabled(false);
	_ui.inputLabel->setEnabled(false);
	_ui.constRangeUIntWidget->show();
	_ui.constRangeUIntWidget->updateUi();
}

CAttribIntWidget::CAttribIntWidget(QWidget *parent)
	: CAttribWidgetT<sint32>(parent)
{
	_ui.schemeComboBox->addItem(tr("value exact blender"));
	_ui.schemeComboBox->addItem(tr("values gradient"));
	_ui.schemeComboBox->addItem(tr("value computed from emitter"));
	_ui.schemeComboBox->addItem(tr("binary operator"));
}

CAttribIntWidget::~CAttribIntWidget()
{
}

void CAttribIntWidget::setRange(sint32 minValue, sint32 maxValue)
{
	_MinRange = minValue;
	_MaxRange = maxValue;
	_ui.constRangeIntWidget->setRange(_MinRange, _MaxRange);
}

void CAttribIntWidget::setWrapper(IPSWrapper<sint32> *wrapper)
{
	nlassert(wrapper);
	_Wrapper = wrapper;
	_ui.constRangeIntWidget->setWrapper(_Wrapper);
}

QDialog *CAttribIntWidget::editScheme(void)
{
	const NL3D::CPSAttribMaker<sint32> *scheme = _SchemeWrapper->getScheme();

	if (dynamic_cast<const NL3D::CPSIntBlender *>(scheme))
	{
		CIntBlenderDialogClient *myInterface = new CIntBlenderDialogClient();
		myInterface->MinRange = _MinRange;
		myInterface->MaxRange = _MaxRange;
		myInterface->SchemeFunc = & ((NL3D::CPSValueBlenderSample<sint32, 64> *) scheme)->_F;
		CValueBlenderDialog *vb = new CValueBlenderDialog(myInterface, _Node, true, this);
		return vb;
	}
	if (dynamic_cast<const NL3D::CPSIntGradient *>(scheme))
	{
		CIntGradientWrapper *wrapper = new CIntGradientWrapper;
		wrapper->m_minRange = _MinRange;
		wrapper->m_maxRange = _MaxRange;
		wrapper->m_scheme = &(((NL3D::CPSIntGradient *) (_SchemeWrapper->getScheme()) )->_F);
		CGradientDialog *gd = new CGradientDialog(_Node, wrapper, true, true, 2, this);
		wrapper->m_defaultValue = 0;
		return gd;
	}
	if (dynamic_cast<const NL3D::CPSIntMemory *>(scheme))
	{
		CAttribIntWidget *adi = new CAttribIntWidget();
		adi->setRange(_MinRange, _MaxRange);
		CValueFromEmitterDialogT<sint32> *vfe = new CValueFromEmitterDialogT<sint32>((NL3D::CPSIntMemory *) _SchemeWrapper->getScheme(),
				adi, this);
		vfe->init();
		adi->setWorkspaceNode(_Node);
		adi->updateUi();
		return vfe;
	}
	if (dynamic_cast<const NL3D::CPSIntBinOp *>(scheme))
	{
		CAttribIntWidget *ad[2] = { NULL, NULL};
		for (uint k = 0; k <2; ++k)
		{
			ad[k] = new CAttribIntWidget();
			ad[k]->setRange(_MinRange, _MaxRange);
		}
		CBinOpDialogT<sint32> *bod = new CBinOpDialogT<sint32>( (NL3D::CPSIntBinOp *)(scheme),
				(CAttribWidgetT<sint32> **) ad,
				this);
		bod->init();
		for (uint k = 0; k <2; ++k)
		{
			ad[k]->setWorkspaceNode(_Node);
			ad[k]->updateUi();
		}
		return bod;
	}
	return NULL;
}

void CAttribIntWidget::setCurrentScheme(uint index)
{
	nlassert(index < 4);
	NL3D::CPSAttribMaker<sint32> *scheme = NULL;

	switch (index)
	{
	case 0 :
		scheme = new NL3D::CPSIntBlender;
		break;
	case 1 :
		scheme = new NL3D::CPSIntGradient;
		break;
	case 2 :
		scheme = new NL3D::CPSIntMemory;
		((NL3D::CPSAttribMakerMemory<sint32> *) scheme)->setScheme(new NL3D::CPSIntBlender(_MinRange, _MaxRange));
		break;
	case 3 :
		scheme = new NL3D::CPSIntBinOp;
		((NL3D::CPSIntBinOp *) scheme)->setArg(0, new NL3D::CPSIntBlender);
		((NL3D::CPSIntBinOp *) scheme)->setArg(1, new NL3D::CPSIntBlender);
		break;
	default:
		break;
	}
	if (scheme)
	{
		_SchemeWrapper->setSchemeAndUpdateModifiedFlag(scheme);
	}
}

sint CAttribIntWidget::getCurrentScheme(void) const
{
	const NL3D::CPSAttribMaker<sint32> *scheme = _SchemeWrapper->getScheme();

	if (dynamic_cast<const NL3D::CPSIntBlender *>(scheme)) return 0;
	if (dynamic_cast<const NL3D::CPSIntGradient *>(scheme)) return 1;
	if (dynamic_cast<const NL3D::CPSIntMemory *>(scheme)) return 2;
	if (dynamic_cast<const NL3D::CPSIntBinOp *>(scheme)) return 3;
	return -1;
}

void CAttribIntWidget::cstValueUpdate()
{
	_ui.schemeWidget->hide();
	_ui.schemeEditWidget->hide();
	_ui.inMultiplierWidget->setEnabled(false);
	_ui.clampCheckBox->setEnabled(false);
	_ui.inputLabel->setEnabled(false);
	_ui.constRangeIntWidget->show();
	//_ui.constRangeIntWidget->updateUi();
}

CAttribRGBAWidget::CAttribRGBAWidget(QWidget *parent)
	: CAttribWidgetT<NLMISC::CRGBA>(parent)
{
	_ui.schemeComboBox->addItem(tr("color sampled blender"));
	_ui.schemeComboBox->addItem(tr("color gradient"));
	_ui.schemeComboBox->addItem(tr("color exact blender"));
	_ui.schemeComboBox->addItem(tr("values computed from emitter"));
	_ui.schemeComboBox->addItem(tr("binary operator"));
}

CAttribRGBAWidget::~CAttribRGBAWidget()
{
}

void CAttribRGBAWidget::setWrapper(IPSWrapper<NLMISC::CRGBA> *wrapper)
{
	nlassert(wrapper);
	_Wrapper = wrapper;
	_ui.constRGBAWidget->setWrapper(_Wrapper);
}

QDialog *CAttribRGBAWidget::editScheme(void)
{
	const NL3D::CPSAttribMaker<NLMISC::CRGBA> *scheme = _SchemeWrapper->getScheme();

	if (dynamic_cast<const NL3D::CPSColorBlender *>(scheme))
	{
		CRGBABlenderDialogClient *myInterface = new CRGBABlenderDialogClient();
		myInterface->SchemeFunc = & ((NL3D::CPSValueBlenderSample<NLMISC::CRGBA, 64> *) scheme)->_F;
		CValueBlenderDialog *vb = new CValueBlenderDialog(myInterface, _Node, true, this);
		return vb;
	}
	if (dynamic_cast<const NL3D::CPSColorGradient *>(scheme))
	{
		CColorGradientWrapper *wrapper = new CColorGradientWrapper;
		wrapper->m_scheme = &(((NL3D::CPSColorGradient *) (_SchemeWrapper->getScheme()) )->_F);
		CGradientDialog *gd = new CGradientDialog(_Node, wrapper, true, true, 2, this);
		wrapper->m_defaultValue = NLMISC::CRGBA::White;
		return gd;
	}
	if (dynamic_cast<const NL3D::CPSColorBlenderExact *>(scheme))
	{
		return NULL;
	}
	if (dynamic_cast<const NL3D::CPSColorMemory *>(scheme))
	{
		CAttribRGBAWidget *ad = new CAttribRGBAWidget();
		CValueFromEmitterDialogT<NLMISC::CRGBA> *vfe = new CValueFromEmitterDialogT<NLMISC::CRGBA>( (NL3D::CPSColorMemory *)(scheme),
				ad,
				this);
		vfe->init();
		ad->setWorkspaceNode(_Node);
		ad->updateUi();
		return vfe;
	}
	if (dynamic_cast<const NL3D::CPSColorBinOp *>(scheme))
	{
		CAttribRGBAWidget *ad[2] = { NULL, NULL};
		for (uint k = 0; k <2; ++k)
		{
			ad[k] = new CAttribRGBAWidget();
		}
		CBinOpDialogT<NLMISC::CRGBA> *bod = new CBinOpDialogT<NLMISC::CRGBA>( (NL3D::CPSColorBinOp *)(scheme),
				(CAttribWidgetT<NLMISC::CRGBA> **) ad,
				this);
		bod->init();
		for (uint k = 0; k <2; ++k)
		{
			ad[k]->setWorkspaceNode(_Node);
			ad[k]->updateUi();
		}
		return bod;
	}
	return NULL;
}

void CAttribRGBAWidget::setCurrentScheme(uint index)
{
	nlassert(index < 5);
	NL3D::CPSAttribMaker<NLMISC::CRGBA> *scheme = NULL;

	switch (index)
	{
	case 0 :
		scheme = new NL3D::CPSColorBlender;
		break;
	case 1 :
		scheme = new NL3D::CPSColorGradient(NL3D::CPSColorGradient::_DefaultGradient, 2, 16, 1.f);
		break;
	case 2 :
		scheme = new NL3D::CPSColorBlenderExact;
		break;
	case 3 :
		scheme = new NL3D::CPSColorMemory;
		((NL3D::CPSAttribMakerMemory<NLMISC::CRGBA> *) scheme)->setScheme(new NL3D::CPSColorBlender);
		break;
	case 4 :
		scheme = new NL3D::CPSColorBinOp;
		((NL3D::CPSColorBinOp *) scheme)->setArg(0, new NL3D::CPSColorBlender);
		((NL3D::CPSColorBinOp *) scheme)->setArg(1, new NL3D::CPSColorBlender);
		break;
	default:
		break;
	}
	if (scheme)
	{
		_SchemeWrapper->setSchemeAndUpdateModifiedFlag(scheme);
	}
}

sint CAttribRGBAWidget::getCurrentScheme(void) const
{
	const NL3D::CPSAttribMaker<NLMISC::CRGBA> *scheme = _SchemeWrapper->getScheme();

	if (dynamic_cast<const NL3D::CPSColorBlender *>(scheme)) return 0;
	if (dynamic_cast<const NL3D::CPSColorGradient *>(scheme)) return 1;
	if (dynamic_cast<const NL3D::CPSColorBlenderExact *>(scheme)) return 2;
	if (dynamic_cast<const NL3D::CPSColorMemory *>(scheme)) return 3;
	if (dynamic_cast<const NL3D::CPSColorBinOp *>(scheme)) return 4;
	return -1;
}

void CAttribRGBAWidget::cstValueUpdate()
{
	_ui.schemeWidget->hide();
	_ui.schemeEditWidget->hide();
	_ui.inMultiplierWidget->setEnabled(false);
	_ui.clampCheckBox->setEnabled(false);
	_ui.inputLabel->setEnabled(false);
	_ui.constRGBAWidget->show();
	_ui.constRGBAWidget->updateUi();
}

CAttribPlaneBasisWidget::CAttribPlaneBasisWidget(QWidget *parent)
	: CAttribWidgetT<NL3D::CPlaneBasis>(parent)
{
	_ui.schemeComboBox->addItem(tr("basis gradient"));
	_ui.schemeComboBox->addItem(tr("follow path"));
	_ui.schemeComboBox->addItem(tr("spinner"));
	_ui.schemeComboBox->addItem(tr("values computed from emitter"));
	_ui.schemeComboBox->addItem(tr("binary operator"));
}

CAttribPlaneBasisWidget::~CAttribPlaneBasisWidget()
{
}

void CAttribPlaneBasisWidget::setWrapper(IPSWrapper<NL3D::CPlaneBasis> *wrapper)
{
	nlassert(wrapper);
	_Wrapper = wrapper;
	_ui.constAttribPlaneWidget->setWrapper(_Wrapper);
}

QDialog *CAttribPlaneBasisWidget::editScheme(void)
{
	NL3D::CPSAttribMaker<NL3D::CPlaneBasis> *scheme = _SchemeWrapper->getScheme();
	if (dynamic_cast<NL3D::CPSPlaneBasisGradient *>(scheme))
	{
		CPlaneBasisGradientWrapper *wrapper = new CPlaneBasisGradientWrapper;
		wrapper->m_scheme = &(((NL3D::CPSPlaneBasisGradient *) (_SchemeWrapper->getScheme()) )->_F);
		CGradientDialog *gd = new CGradientDialog(_Node, wrapper, true, true, 2, this);
		wrapper->m_defaultValue = NL3D::CPlaneBasis(NLMISC::CVector::K);
		return gd;
	}
	if (dynamic_cast<NL3D::CPSPlaneBasisFollowSpeed *>(scheme))
	{
		CFollowPathDialog *dialog = new CFollowPathDialog(dynamic_cast<NL3D::CPSPlaneBasisFollowSpeed *>(scheme), _Node, this);
		return dialog;
	}
	if (dynamic_cast<NL3D::CPSPlaneBasisMemory *>(scheme))
	{
		CAttribPlaneBasisWidget *ad = new CAttribPlaneBasisWidget();
		CValueFromEmitterDialogT<NL3D::CPlaneBasis> *vfe = new CValueFromEmitterDialogT<NL3D::CPlaneBasis>
		( (NL3D::CPSPlaneBasisMemory *)(scheme),
		  ad, this);
		vfe->init();
		ad->setWorkspaceNode(_Node);
		ad->updateUi();
		return vfe;
	}
	if (dynamic_cast<NL3D::CPSPlaneBasisBinOp *>(scheme))
	{
		CAttribPlaneBasisWidget *ad[2] = { NULL, NULL};
		for (uint k = 0; k <2; ++k)
		{
			ad[k] = new CAttribPlaneBasisWidget();
		}
		CBinOpDialogT<NL3D::CPlaneBasis> *bod = new CBinOpDialogT<NL3D::CPlaneBasis>( (NL3D::CPSPlaneBasisBinOp *)(scheme),
				(CAttribWidgetT<NL3D::CPlaneBasis> **) ad,
				this);
		bod->init();
		for (uint k = 0; k <2; ++k)
		{
			ad[k]->setWorkspaceNode(_Node);
			ad[k]->updateUi();
		}
		return bod;
	}
	if (dynamic_cast<NL3D::CPSBasisSpinner *>(scheme))
	{
		CSpinnerDialog *dialog = new CSpinnerDialog(dynamic_cast<NL3D::CPSBasisSpinner *>(scheme), _Node, this);
		return dialog;
	}
	return NULL;
}

void CAttribPlaneBasisWidget::setCurrentScheme(uint index)
{
	nlassert(index < 5);
	NL3D::CPSAttribMaker<NL3D::CPlaneBasis> *scheme = NULL;

	switch (index)
	{
	case 0:
		scheme = new NL3D::CPSPlaneBasisGradient;
		break;
	case 1:
		scheme = new NL3D::CPSPlaneBasisFollowSpeed;
		break;
	case 2:
		scheme = new NL3D::CPSBasisSpinner;
		static_cast<NL3D::CPSBasisSpinner *>(scheme)->_F.setNumSamples(16);
		break;
	case 3:
		scheme = new NL3D::CPSPlaneBasisMemory;
		((NL3D::CPSAttribMakerMemory<NL3D::CPlaneBasis> *) scheme)->setScheme(new NL3D::CPSPlaneBasisFollowSpeed);
		if (_Node)
		{
			_Node->setModified(true);
		}
		break;
	case 4 :
		scheme = new NL3D::CPSPlaneBasisBinOp;
		((NL3D::CPSPlaneBasisBinOp *) scheme)->setArg(0, new NL3D::CPSPlaneBasisFollowSpeed);
		((NL3D::CPSPlaneBasisBinOp *) scheme)->setArg(1, new NL3D::CPSPlaneBasisFollowSpeed);
		break;
	default:
		break;
	}

	if (scheme)
	{
		_SchemeWrapper->setSchemeAndUpdateModifiedFlag(scheme);
	}
}

sint CAttribPlaneBasisWidget::getCurrentScheme(void) const
{
	const NL3D::CPSAttribMaker<NL3D::CPlaneBasis> *scheme = _SchemeWrapper->getScheme();

	if (dynamic_cast<const NL3D::CPSPlaneBasisGradient *>(scheme)) return 0;
	if (dynamic_cast<const NL3D::CPSPlaneBasisFollowSpeed *>(scheme)) return 1;
	if (dynamic_cast<const NL3D::CPSBasisSpinner *>(scheme)) return 2;
	if (dynamic_cast<const NL3D::CPSPlaneBasisMemory *>(scheme)) return 3;
	if (dynamic_cast<const NL3D::CPSPlaneBasisBinOp *>(scheme)) return 4;

	return -1;
}

void CAttribPlaneBasisWidget::cstValueUpdate()
{
	_ui.schemeWidget->hide();
	_ui.schemeEditWidget->hide();
	_ui.inMultiplierWidget->setEnabled(false);
	_ui.clampCheckBox->setEnabled(false);
	_ui.inputLabel->setEnabled(false);
	_ui.constAttribPlaneWidget->show();
	_ui.constAttribPlaneWidget->updateUi();
}

} /* namespace NLQT */