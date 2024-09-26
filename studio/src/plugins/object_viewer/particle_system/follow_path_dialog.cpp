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
#include "follow_path_dialog.h"

// NeL include
#include <nel/3d/ps_plane_basis_maker.h>

// Project includes
#include "particle_node.h"

namespace NLQT
{

CFollowPathDialog::CFollowPathDialog(NL3D::CPSPlaneBasisFollowSpeed *pbfs, CWorkspaceNode *ownerNode, QWidget *parent)
	: QDialog(parent),
	  _FollowPath(pbfs),
	  _Node(ownerNode)
{
	resize(270, 90);
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
	setSizePolicy(sizePolicy);
	setMinimumSize(QSize(0, 90));
	setMaximumSize(QSize(16777215, 90));
	gridLayout = new QGridLayout(this);
	label = new QLabel(this);
	gridLayout->addWidget(label, 0, 0, 1, 1);
	comboBox = new QComboBox(this);
	gridLayout->addWidget(comboBox, 1, 0, 1, 2);
	horizontalSpacer = new QSpacerItem(207, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	gridLayout->addItem(horizontalSpacer, 2, 0, 1, 1);
	pushButton = new QPushButton(this);
	gridLayout->addWidget(pushButton, 2, 1, 1, 1);

	setWindowTitle(tr("Follow path param"));
	label->setText(tr("Projection plane:"));
	comboBox->clear();
	comboBox->insertItems(0, QStringList()
						  << tr("No projection")
						  << tr("XY plane")
						  << tr("XZ plane")
						  << tr("YZ plane"));
	pushButton->setText(("Ok"));

	comboBox->setCurrentIndex(_FollowPath->getProjectionPlane());

	setFixedHeight(sizeHint().height());

	connect(pushButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setProjectionMode(int)));
}

CFollowPathDialog::~CFollowPathDialog()
{
}

void CFollowPathDialog::setProjectionMode(int index)
{
	nlassert(_FollowPath);
	_FollowPath->setProjectionPlane((NL3D::CPSPlaneBasisFollowSpeed::TProjectionPlane) index);
	if (_Node) _Node->setModified(true);
}

} /* namespace NLQT */