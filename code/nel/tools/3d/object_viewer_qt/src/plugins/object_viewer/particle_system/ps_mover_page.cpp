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
#include "ps_mover_page.h"

// Qt includes

// NeL includes

// Project includes
#include "modules.h"

namespace NLQT
{

const float epsilon = 10E-3f;

CPSMoverPage::CPSMoverPage(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.scaleWidget->setRange(0.f, 4.f);
	_ui.scaleWidget->setWrapper(&_UniformScaleWrapper);

	_ui.scaleXWidget->setRange(0.f, 4.f);
	_ui.scaleXWidget->setWrapper(&_XScaleWrapper);

	_ui.scaleYWidget->setRange(0.f, 4.f);
	_ui.scaleYWidget->setWrapper(&_YScaleWrapper);

	_ui.scaleZWidget->setRange(0.f, 4.f);
	_ui.scaleZWidget->setWrapper(&_ZScaleWrapper);

	//_ui.directionWidget->setWrapper(&_DirectionWrapper);

	connect(_ui.xDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setXPosition(double)));
	connect(_ui.yDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setYPosition(double)));
	connect(_ui.zDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setZPosition(double)));

	connect(_ui.directionWidget, SIGNAL(valueChanged(NLMISC::CVector)), this, SLOT(setDir(NLMISC::CVector)));

	connect(_ui.listWidget, SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
			this, SLOT(changeSubComponent()));
}

CPSMoverPage::~CPSMoverPage()
{
}

void CPSMoverPage::setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocated *located, uint32 editedLocatedIndex)
{
	_Node = ownerNode;
	_EditedLocated = located;
	_EditedLocatedIndex = editedLocatedIndex;

	updatePosition();

	_ui.listWidget->clear();
	hideAdditionalWidget();

	uint numBound = _EditedLocated->getNbBoundObjects();

	uint nbCandidates = 0;

	for (uint k = 0; k < numBound; ++k)
	{
		if (dynamic_cast<NL3D::IPSMover *>(_EditedLocated->getBoundObject(k)))
		{
			CLocatedBindableItem *item = new CLocatedBindableItem(QString(_EditedLocated->getBoundObject(k)->getName().c_str()),
					_ui.listWidget);
			item->setUserData(_EditedLocated->getBoundObject(k));
			++nbCandidates;
		}
	}
	if (nbCandidates > 0)
		_ui.listWidget->setCurrentRow(0);
}

void CPSMoverPage::updatePosition(void)
{
	const NLMISC::CVector &pos = _EditedLocated->getPos()[_EditedLocatedIndex];

	_ui.xDoubleSpinBox->setValue(pos.x);
	_ui.yDoubleSpinBox->setValue(pos.y);
	_ui.zDoubleSpinBox->setValue(pos.z);

}

void CPSMoverPage::hideAdditionalWidget()
{
	_ui.scaleLabel->hide();
	_ui.scaleXLabel->hide();
	_ui.scaleYLabel->hide();
	_ui.scaleZLabel->hide();
	_ui.scaleWidget->hide();
	_ui.scaleXWidget->hide();
	_ui.scaleYWidget->hide();
	_ui.scaleZWidget->hide();
	_ui.directionWidget->hide();
}

void CPSMoverPage::updateListener(void)
{
	/*	if(_ParticleDlg->MainFrame->isMoveElement())
		{
			const NLMISC::CVector &pos = _EditedLocated->getPos()[_EditedLocatedIndex];
			NLMISC::CMatrix m;
			m = _MouseListener->getModelMatrix();
			m.setPos(pos);
			_MouseListener->setModelMatrix(m);
			_Node->setModified(true);
		}*/
}

void CPSMoverPage::setXPosition(double value)
{
	NLMISC::CVector &pos = _EditedLocated->getPos()[_EditedLocatedIndex];
	if (fabs(pos.x - _ui.xDoubleSpinBox->value()) > epsilon)
	{
		pos.x = value;
		updateListener();
		_Node->setModified(true);
	}
}

void CPSMoverPage::setYPosition(double value)
{
	NLMISC::CVector &pos = _EditedLocated->getPos()[_EditedLocatedIndex];
	if (fabs(pos.y - _ui.yDoubleSpinBox->value()) > epsilon)
	{
		pos.y = value;
		updateListener();
		_Node->setModified(true);
	}
}

void CPSMoverPage::setZPosition(double value)
{
	NLMISC::CVector &pos = _EditedLocated->getPos()[_EditedLocatedIndex];
	if (fabs(pos.z - _ui.zDoubleSpinBox->value()) > epsilon)
	{
		pos.z = value;
		updateListener();
		_Node->setModified(true);
	}
}

void CPSMoverPage::changeSubComponent()
{
	hideAdditionalWidget();
	NL3D::IPSMover *m = getMoverInterface();
	if (!m) return;

	_Node->getPSPointer()->setCurrentEditedElement(NULL);
	_Node->getPSPointer()->setCurrentEditedElement(_EditedLocated, _EditedLocatedIndex, getLocatedBindable());


	if (m->supportUniformScaling() && ! m->supportNonUniformScaling() )
	{
		_UniformScaleWrapper.OwnerNode = _Node;
		_UniformScaleWrapper.M = m;
		_UniformScaleWrapper.Index = _EditedLocatedIndex;

		_ui.scaleWidget->updateUi();
		_ui.scaleLabel->show();
		_ui.scaleWidget->show();
	}
	else if (m->supportNonUniformScaling())
	{
		// dialog for edition of x scale
		_XScaleWrapper.OwnerNode = _Node;
		_XScaleWrapper.M = m;
		_XScaleWrapper.Index = _EditedLocatedIndex;

		_ui.scaleXWidget->updateUi();
		_ui.scaleXLabel->show();
		_ui.scaleXWidget->show();

		// dialog for edition of y scale
		_YScaleWrapper.OwnerNode = _Node;
		_YScaleWrapper.M = m;
		_YScaleWrapper.Index = _EditedLocatedIndex;

		_ui.scaleYWidget->updateUi();
		_ui.scaleYLabel->show();
		_ui.scaleYWidget->show();

		// dialog for edition of x scale
		_ZScaleWrapper.OwnerNode = _Node;
		_ZScaleWrapper.M = m;
		_ZScaleWrapper.Index = _EditedLocatedIndex;

		_ui.scaleZWidget->updateUi();
		_ui.scaleZLabel->show();
		_ui.scaleZWidget->show();
	}


	if (m->onlyStoreNormal())
	{
		_ui.directionWidget->setValue(getMoverInterface()->getNormal(getLocatedIndex()), false);
		_ui.directionWidget->show();
	}
}

void CPSMoverPage::setDir(const NLMISC::CVector &value)
{
	getMoverInterface()->setNormal(getLocatedIndex(), value);
	updateModifiedFlag();
}

NL3D::IPSMover *CPSMoverPage::getMoverInterface(void)
{
	nlassert(_EditedLocated);
	sint currIndex = _ui.listWidget->currentRow();
	if (currIndex == -1) return NULL;

	CLocatedBindableItem *item = dynamic_cast<CLocatedBindableItem *>(_ui.listWidget->currentItem());
	return dynamic_cast<NL3D::IPSMover *>(item->getUserData());
}

NL3D::CPSLocatedBindable *CPSMoverPage::getLocatedBindable(void)
{
	nlassert(_EditedLocated);
	sint currIndex = _ui.listWidget->currentRow();
	if (currIndex == -1) return NULL;

	CLocatedBindableItem *item = dynamic_cast<CLocatedBindableItem *>(_ui.listWidget->currentItem());
	return item->getUserData();
}

} /* namespace NLQT */