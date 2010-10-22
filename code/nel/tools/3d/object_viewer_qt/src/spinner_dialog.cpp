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
#include "spinner_dialog.h"

namespace NLQT {
  
CSpinnerDialog::CSpinnerDialog(NL3D::CPSBasisSpinner *sf, CWorkspaceNode *ownerNode, QWidget *parent)
      : QDialog(parent)
{
        _verticalLayout = new QVBoxLayout(this);
        _nbSamplesLabel = new QLabel(this);
        _verticalLayout->addWidget(_nbSamplesLabel);

        _nbSamplesWidget = new NLQT::CEditRangeUIntWidget(this);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(_nbSamplesWidget->sizePolicy().hasHeightForWidth());
        _nbSamplesWidget->setSizePolicy(sizePolicy);
        _verticalLayout->addWidget(_nbSamplesWidget);

        _dirWidget = new NLQT::CDirectionWidget(this);
        sizePolicy.setHeightForWidth(_dirWidget->sizePolicy().hasHeightForWidth());
        _dirWidget->setSizePolicy(sizePolicy);
        _verticalLayout->addWidget(_dirWidget);
	
	setWindowTitle(tr("Edit spinner"));
        _nbSamplesLabel->setText(tr("Nb samples:"));
        
	_AxisWrapper.OwnerNode = ownerNode;
	_NbSampleWrapper.OwnerNode = ownerNode;
	_NbSampleWrapper.S = sf;
	_AxisWrapper.S	   = sf;

	_nbSamplesWidget->setRange(1, 512);
	_nbSamplesWidget->setWrapper(&_NbSampleWrapper);
	_nbSamplesWidget->enableLowerBound(0, true);
	_nbSamplesWidget->updateUi();
	
	_dirWidget->setWrapper(&_AxisWrapper);
	_dirWidget->updateUi();

	setFixedHeight(sizeHint().height());
}

CSpinnerDialog::~CSpinnerDialog()
{
}

} /* namespace NLQT */