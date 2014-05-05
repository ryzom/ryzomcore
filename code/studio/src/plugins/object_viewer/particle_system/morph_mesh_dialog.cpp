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
#include "morph_mesh_dialog.h"

// Qt include
#include <QtGui/QFileDialog>

// NeL includes
#include "nel/3d/ps_mesh.h"
#include "nel/3d/particle_system_model.h"

// Projects includes
#include "mesh_widget.h"

namespace NLQT
{

CMorphMeshDialog::CMorphMeshDialog(CWorkspaceNode *ownerNode, NL3D::CPSConstraintMesh *cm, QWidget *parent)
	: QDialog(parent),
	  _Node(ownerNode),
	  _CM(cm)
{
	_ui.setupUi(this);

	_ui.morphValueWidget->setRange(0, 10);
	_ui.morphValueWidget->setWrapper(&_MorphSchemeWrapper);
	_ui.morphValueWidget->setSchemeWrapper(&_MorphSchemeWrapper);
	_ui.morphValueWidget->init();

	_MorphSchemeWrapper.CM = _CM;
	_ui.morphValueWidget->setWorkspaceNode(_Node);
	_ui.morphValueWidget->updateUi();

	updateMeshList();

	_ui.infoLabel->setVisible(!_CM->isValidBuild());

	connect(_ui.addPushButton, SIGNAL(clicked()), this, SLOT(add()));
	connect(_ui.removePushButton, SIGNAL(clicked()), this, SLOT(remove()));
	connect(_ui.insertPushButton, SIGNAL(clicked()), this, SLOT(insert()));
	connect(_ui.changePushButton, SIGNAL(clicked()), this, SLOT(change()));
	connect(_ui.upPushButton, SIGNAL(clicked()), this, SLOT(up()));
	connect(_ui.downPushButton, SIGNAL(clicked()), this, SLOT(down()));
}

CMorphMeshDialog::~CMorphMeshDialog()
{
}

void CMorphMeshDialog::updateMeshList()
{
	nlassert(_CM);
	std::vector<sint> numVerts;
	_CM->getShapeNumVerts(numVerts);
	_ui.listWidget->clear();
	for (uint k = 0; k < _CM->getNumShapes(); ++k)
		_ui.listWidget->addItem(getShapeDescStr(k, numVerts[k]));

	_ui.listWidget->setCurrentRow(0);
	if (_CM->getNumShapes() < 2)
		_ui.removePushButton->setEnabled(false);
	else
		_ui.removePushButton->setEnabled(true);
}

QString CMorphMeshDialog::getShapeDescStr(uint shapeIndex, sint numVerts) const
{
	if (numVerts >= 0)
	{
		QString verts(tr("vertices"));
		QString msg = _CM->getShape(shapeIndex).c_str() + tr(" (%1 vertices)").arg(numVerts);
		return msg;
	}
	else
	{
		QString error = qobject_cast<CMeshWidget *>(QObject::parent())->getShapeErrorString(numVerts);
		QString result =  _CM->getShape(shapeIndex).c_str() + QString(" (%1)").arg(error);
		return result;
	}
}

void CMorphMeshDialog::add()
{
	QString fileName = QFileDialog::getOpenFileName(this,
					   tr("Open NeL data file"), ".",
					   tr("NeL shape files (*.shape)"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		std::vector<std::string> shapeNames;
		shapeNames.resize(_CM->getNumShapes() + 1);
		_CM->getShapesNames(&shapeNames[0]);
		uint index = (uint)shapeNames.size() - 1;
		shapeNames[index] = fileName.toUtf8().constData();
		_CM->setShapes(&shapeNames[0], (uint)shapeNames.size());
		std::vector<sint> numVerts;
		_CM->getShapeNumVerts(numVerts);
		_ui.listWidget->addItem(getShapeDescStr(index, numVerts[index]));
		_ui.removePushButton->setEnabled(true);
		touchPSState();
	}
	setCursor(Qt::ArrowCursor);
	_ui.infoLabel->setVisible(!_CM->isValidBuild());
}

void CMorphMeshDialog::remove()
{
	sint row = _ui.listWidget->currentRow();
	std::vector<std::string> shapeNames;
	shapeNames.resize(_CM->getNumShapes());
	_CM->getShapesNames(&shapeNames[0]);
	shapeNames.erase(shapeNames.begin() + row);
	_CM->setShapes(&shapeNames[0], (uint)shapeNames.size());

	if (_CM->getNumShapes() < 2)
		_ui.removePushButton->setEnabled(false);

	touchPSState();
	updateMeshList();
	_ui.infoLabel->setVisible(!_CM->isValidBuild());
}

void CMorphMeshDialog::insert()
{
	QString fileName = QFileDialog::getOpenFileName(this,
					   tr("Open NeL data file"), ".",
					   tr("NeL shape files (*.shape)"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		sint row = _ui.listWidget->currentRow();
		std::vector<std::string> shapeNames;
		shapeNames.resize(_CM->getNumShapes());
		_CM->getShapesNames(&shapeNames[0]);
		shapeNames.insert(shapeNames.begin() + row, fileName.toUtf8().constData());
		_CM->setShapes(&shapeNames[0], (uint)shapeNames.size());
		touchPSState();
		updateMeshList();
		_ui.listWidget->setCurrentRow(row);
	}
	setCursor(Qt::ArrowCursor);
	_ui.infoLabel->setVisible(!_CM->isValidBuild());
}

void CMorphMeshDialog::change()
{
	QString fileName = QFileDialog::getOpenFileName(this,
					   tr("Open NeL data file"), ".",
					   tr("NeL shape files (*.shape)"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		sint row = _ui.listWidget->currentRow();
		_CM->setShape(row, fileName.toUtf8().constData());
		updateMeshList();
		touchPSState();
	}
	setCursor(Qt::ArrowCursor);
	_ui.infoLabel->setVisible(!_CM->isValidBuild());
}

void CMorphMeshDialog::up()
{
	sint row = _ui.listWidget->currentRow();
	if (row == 0) return;
	std::vector<std::string> shapeNames;
	shapeNames.resize(_CM->getNumShapes());
	_CM->getShapesNames(&shapeNames[0]);
	std::swap(shapeNames[row - 1], shapeNames[row]);
	_CM->setShapes(&shapeNames[0], (uint)shapeNames.size());
	updateMeshList();
	_ui.listWidget->setCurrentRow(row - 1);
	_ui.infoLabel->setVisible(!_CM->isValidBuild());
}

void CMorphMeshDialog::down()
{
	sint row = _ui.listWidget->currentRow();
	if (row == (sint) (_CM->getNumShapes() - 1)) return;
	std::vector<std::string> shapeNames;
	shapeNames.resize(_CM->getNumShapes());
	_CM->getShapesNames(&shapeNames[0]);
	std::swap(shapeNames[row + 1], shapeNames[row]);
	_CM->setShapes(&shapeNames[0], (uint)shapeNames.size());
	updateMeshList();
	_ui.listWidget->setCurrentRow(row + 1);
	_ui.infoLabel->setVisible(!_CM->isValidBuild());
}

void CMorphMeshDialog::touchPSState()
{
	if (_Node && _Node->getPSModel())
	{
		_Node->getPSModel()->touchTransparencyState();
		_Node->getPSModel()->touchLightableState();
	}
}

float CMorphMeshDialog::CMorphSchemeWrapper::get(void) const
{
	nlassert(CM);
	return CM->getMorphValue();
}

void CMorphMeshDialog::CMorphSchemeWrapper::set(const float &v)
{
	nlassert(CM);
	CM->setMorphValue(v);
}

CMorphMeshDialog::CMorphSchemeWrapper::scheme_type *CMorphMeshDialog::CMorphSchemeWrapper::getScheme(void) const
{
	nlassert(CM);
	return CM->getMorphScheme();
}

void CMorphMeshDialog::CMorphSchemeWrapper::setScheme(scheme_type *s)
{
	nlassert(CM);
	CM->setMorphScheme(s);
}

} /* namespace NLQT */