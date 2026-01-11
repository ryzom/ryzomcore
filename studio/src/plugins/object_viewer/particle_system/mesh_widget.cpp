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

// Qt include
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>

// NeL include
#include <nel/3d/ps_particle.h>
#include <nel/3d/ps_mesh.h>
#include <nel/3d/particle_system_model.h>
#include <nel/misc/path.h>

// Project includes
#include "mesh_widget.h"
#include "morph_mesh_dialog.h"

namespace NLQT
{

CMeshWidget::CMeshWidget(QWidget *parent)
	: QGroupBox(parent)
{
	_ui.setupUi(this);

	connect(_ui.browsePushButton, SIGNAL(clicked(bool)), this, SLOT(browseShape()));
	connect(_ui.editPushButton, SIGNAL(clicked(bool)), this, SLOT(editMorph()));
	connect(_ui.morphCheckBox, SIGNAL(toggled(bool)), this, SLOT(setMorphMesh(bool)));
}

CMeshWidget::~CMeshWidget()
{
}

void CMeshWidget::setCurrentShape(CWorkspaceNode *ownerNode, NL3D::CPSShapeParticle *sp)
{
	_Node = ownerNode;
	_ShapeParticle = sp;

	if (!dynamic_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle))
	{
		_ui.groupBox->hide();
		_ui.browsePushButton->setEnabled(true);
		_ui.meshLineEdit->setEnabled(true);
		_ui.label->setEnabled(true);
		_ui.meshLineEdit->setText(_ShapeParticle->getShape().c_str());
	}
	else
	{
		_ui.groupBox->show();
		NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
		if (cm->getNumShapes() > 1)
			_ui.morphCheckBox->setChecked(true);
		else
			_ui.morphCheckBox->setChecked(false);

		updateForMorph();
	}
}

void CMeshWidget::browseShape()
{
	QString fileName = QFileDialog::getOpenFileName(this,
					   tr("Open NeL data file"), ".",
					   tr("NeL shape file (*.shape)"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		NLMISC::CPath::addSearchPath(NLMISC::CFile::getPath(fileName.toUtf8().constData()));
		try
		{
			std::string shapeName = NLMISC::CFile::getFilename(fileName.toUtf8().constData());
			_ShapeParticle->setShape(shapeName);
			_ui.meshLineEdit->setText(shapeName.c_str());
			touchPSState();
		}
		catch (NLMISC::Exception &e)
		{
			QMessageBox::critical(this, tr("Shape loading error"), e.what(),  QMessageBox::Ok);
		}
		updateMeshErrorString();
	}
	setCursor(Qt::ArrowCursor);
}

void CMeshWidget::setMorphMesh(bool state)
{
	NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	if (!cm) return;
	if (state != (cm->getNumShapes() > 1))
	{
		if (state)
		{
			// morphing enabled
			std::string currName[2] = { cm->getShape(), cm->getShape() };
			cm->setShapes(currName, 2);
		}
		else
		{
			// morphing disabled
			std::string currName = cm->getShape(0);
			cm->setShape(currName);
		}
		updateModifiedFlag();
	}
	updateForMorph();
}

void CMeshWidget::editMorph()
{
	NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	CMorphMeshDialog *dialog = new CMorphMeshDialog(_Node, cm, this);
	dialog->show();
	dialog->exec();
	delete dialog;
	updateMeshErrorString();
}

void CMeshWidget::updateForMorph()
{
	NL3D::CPSConstraintMesh *cm = NLMISC::safe_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	if (cm)
	{
		bool enable = cm->getNumShapes() > 1;
		_ui.browsePushButton->setEnabled(!enable);
		_ui.meshLineEdit->setEnabled(!enable);
		_ui.label->setEnabled(!enable);
		if (!enable)
			_ui.meshLineEdit->setText(cm->getShape().c_str());
		else
			_ui.meshLineEdit->setText("");
	}
	updateMeshErrorString();
}

void CMeshWidget::updateMeshErrorString()
{
	_ui.infoLabel->setText("");
	NL3D::CPSConstraintMesh *cm = dynamic_cast<NL3D::CPSConstraintMesh *>(_ShapeParticle);
	if (!cm) return;
	// Update mesh error label
	std::vector<sint> numVerts;
	cm->getShapeNumVerts(numVerts);
	if (numVerts.empty()) return;
	if (numVerts.size() == 1)
		_ui.infoLabel->setText(getShapeErrorString(numVerts[0]));

	else
	{
		// display error msg for morphed meshs
		bool hasError = false;
		for(uint k = 0; k < numVerts.size(); ++k)
		{
			if (numVerts[k] < 0)
			{
				hasError = true;
				break;
			}
		}
		if (hasError)
			_ui.infoLabel->setText(tr("Error in morph meshes"));
	}
}

QString CMeshWidget::getShapeErrorString(sint errorCode)
{
	switch(errorCode)
	{
	case NL3D::CPSConstraintMesh::ShapeFileIsNotAMesh:
		return tr("Not a mesh");
	case NL3D::CPSConstraintMesh::ShapeFileNotLoaded:
		return tr("Shape not loaded");
	case NL3D::CPSConstraintMesh::ShapeHasTooMuchVertices:
		return tr("Too much vertices");
	default:
		break;
	};
	return QString();
}

void CMeshWidget::touchPSState()
{
	if (_Node && _Node->getPSModel())
	{
		_Node->getPSModel()->touchTransparencyState();
		_Node->getPSModel()->touchLightableState();
	}
}

} /* namespace NLQT */
