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
#include "particle_workspace_dialog.h"

// NeL includes
#include <nel/3d/particle_system_model.h>
#include <nel/3d/ps_located.h>
#include <nel/3d/ps_particle.h>
#include <nel/3d/ps_mesh.h>
#include <nel/3d/ps_force.h>
#include <nel/3d/ps_zone.h>
#include <nel/3d/ps_sound.h>
#include <nel/3d/ps_emitter.h>
#include <nel/3d/ps_light.h>
#include <nel/3d/ps_edit.h>

// Qt includes
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QMenu>

// Project includes
#include "dup_ps.h"
#include "modules.h"
#include "object_viewer_constants.h"

namespace NLQT
{

static const char *const LocatedBindable[] =
{
	QT_TR_NOOP("Point"),
	QT_TR_NOOP("LookAt"),
	QT_TR_NOOP("FanLight"),
	QT_TR_NOOP("Ribbon"),
	QT_TR_NOOP("TailDot"),
	QT_TR_NOOP("Mesh"),
	QT_TR_NOOP("ConstraintMesh"),
	QT_TR_NOOP("Face"),
	QT_TR_NOOP("ShockWave"),
	QT_TR_NOOP("Ribbon look at"),
	QT_TR_NOOP("Gravity"),
	QT_TR_NOOP("Directional force"),
	QT_TR_NOOP("Spring"),
	QT_TR_NOOP("Flyid friction"),
	QT_TR_NOOP("Central gravity"),
	QT_TR_NOOP("Cylindric vortex"),
	QT_TR_NOOP("Brownian move"),
	QT_TR_NOOP("Magnetic force"),
	QT_TR_NOOP("Plane"),
	QT_TR_NOOP("Sphere"),
	QT_TR_NOOP("Rectangle"),
	QT_TR_NOOP("Disc"),
	QT_TR_NOOP("Cylinder"),
	QT_TR_NOOP("Directional"),
	QT_TR_NOOP("Omni directional"),
	QT_TR_NOOP("Rectangle"),
	QT_TR_NOOP("Conic"),
	QT_TR_NOOP("Spherical"),
	QT_TR_NOOP("Radial"),
	QT_TR_NOOP("Bind sound"),
	QT_TR_NOOP("Bind light"),
	0
};

struct Action
{
	enum List
	{
		ParticlePoint = 0,
		ParticleLookAt,
		ParticleFanLight,
		ParticleRibbon,
		ParticleTailDot,
		ParticleMesh,
		ParticleConstraintMesh,
		ParticleFace,
		ParticleShockWave,
		ParticleRibbonLookAt,
		ForceGravity,
		ForceDirectional,
		ForceSpring,
		ForceFlyidFriction,
		ForceCentralGravity,
		ForceCylindricVortex,
		ForceBrownianMove,
		ForceMagnetic,
		ZonePlane,
		ZoneSphere,
		ZoneRectangle,
		ZoneDisc,
		ZoneCylinder,
		EmitterDirectional,
		EmitterOmniDirectional,
		EmitterRectangle,
		EmitterConic,
		EmitterSpherical,
		EmitterRadial,
		Sound,
		Light
	};
};

// this map is used to create increasing names
static std::map<std::string,  uint> _PSElementIdentifiers;

CParticleWorkspaceDialog::CParticleWorkspaceDialog(QWidget *parent)
	: QDockWidget(parent),
	  _currentItem(NULL)
{
	_ui.setupUi(this);

	connect(_ui.treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(clickedItem(QModelIndex)));
	connect(_ui.treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenu()));

	// Init tree model
	_treeModel = new CParticleTreeModel(this);
	_ui.treeView->setModel(_treeModel);
	_ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);

	_PropertyDialog = new CPropertyDialog(_treeModel, this);

	_signalMapper = new QSignalMapper(this);

	_setActivePSAction = new QAction(tr("Set as active particle system"), this);
	_savePSAction = new QAction(tr("Save"), this);
	_saveAsPSAction = new QAction(tr("Save as"), this);
	_clearContentAction = new QAction(tr("Clear content"), this);
	_removeFromWSAction = new QAction(tr("Remove from workspace"), this);
	_mergeAction = new QAction(tr("Merge"), this);

	_newLocatedAction = new QAction(tr("New located"), this);
	_newLocatedAction->setIcon(QIcon(Constants::ICON_LOCATED_ITEM_SMALL));
	_pasteLocatedAction = new QAction(tr("Paste located"), this);

	for(int i = 0; LocatedBindable[i]; ++i)
		_bindNewLocatedBindable[i] = new QAction(tr(LocatedBindable[i]), this);

	_bindNewLocatedBindable[Action::Sound]->setIcon(QIcon(Constants::ICON_SOUND_ITEM_SMALL));
	_bindNewLocatedBindable[Action::Light]->setIcon(QIcon(Constants::ICON_LIGHT_ITEM_SMALL));

	_forceZBiasAction = new QAction(tr("Force ZBias"), this);

	_instanciateAction = new QAction(tr("Instanciate"), this);
	_instanciateAction->setIcon(QIcon(Constants::ICON_INSTANCE_ITEM_SMALL));
	_copyLocatedAction = new QAction(tr("Copy located"), this);
	_copyBindableAction = new QAction(tr("Copy bindable"), this);
	_pasteBindableAction = new QAction(tr("Paste bindable"), this);
	_deleteAction = new QAction(tr("Delete"), this);

	_allLODAction = new QAction(tr("All LOD"), this);
	_allLODAction->setCheckable(true);
	_lod1Action = new QAction(tr("LOD 1"), this);
	_lod1Action->setCheckable(true);
	_lod2Action = new QAction(tr("LOD 2"), this);
	_lod2Action->setCheckable(true);
	_externIDAction = new QAction(tr("extern ID"), this);

	connect(_setActivePSAction, SIGNAL(triggered()), this, SLOT(setActiveNode()));
	connect(_savePSAction, SIGNAL(triggered()), this, SLOT(savePS()));
	connect(_saveAsPSAction, SIGNAL(triggered()), this, SLOT(saveAsPS()));
	connect(_clearContentAction, SIGNAL(triggered()), this, SLOT(clearContent()));
	connect(_removeFromWSAction, SIGNAL(triggered()), this, SLOT(removePS()));
	connect(_mergeAction, SIGNAL(triggered()), this, SLOT(mergePS()));
	connect(_newLocatedAction, SIGNAL(triggered()), this, SLOT(newLocated()));
	connect(_pasteLocatedAction, SIGNAL(triggered()), this, SLOT(pasteLocated()));

	connect(_signalMapper, SIGNAL(mapped(int)), this, SLOT(bindNewLocatedBindable(int)));
	for(int i = 0; LocatedBindable[i]; ++i)
	{
		_signalMapper->setMapping(_bindNewLocatedBindable[i], i);
		connect(_bindNewLocatedBindable[i], SIGNAL(triggered()), _signalMapper, SLOT(map()));
	}

	connect(_forceZBiasAction, SIGNAL(triggered()), this, SLOT(forceZBias()));

	connect(_copyLocatedAction, SIGNAL(triggered()), this, SLOT(copyLocated()));
	connect(_copyBindableAction, SIGNAL(triggered()), this, SLOT(copyBindable()));
	connect(_pasteBindableAction, SIGNAL(triggered()), this, SLOT(pasteBindable()));
	connect(_deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

	connect(_instanciateAction, SIGNAL(triggered()), this, SLOT(setInstanciate()));
	connect(_allLODAction, SIGNAL(triggered()), this, SLOT(setAllLOD()));
	connect(_lod1Action, SIGNAL(triggered()), this, SLOT(setLOD1()));
	connect(_lod2Action, SIGNAL(triggered()), this, SLOT(setLOD2()));
	connect(_externIDAction, SIGNAL(triggered()), this, SLOT(setExternID()));
}

CParticleWorkspaceDialog::~CParticleWorkspaceDialog()
{
}

void CParticleWorkspaceDialog::touchPSState(CParticleTreeItem *item)
{
	if (item == NULL) return;
	CWorkspaceNode *ownerNode = _treeModel->getOwnerNode(item);
	if (ownerNode && ownerNode->getPSModel())
	{
		ownerNode->getPSModel()->touchLightableState();
		ownerNode->getPSModel()->touchTransparencyState();
	}
}

void CParticleWorkspaceDialog::clickedItem(const QModelIndex &index)
{
	if (_currentItem != 0)
		_treeModel->getOwnerNode(_currentItem)->getPSPointer()->setCurrentEditedElement(NULL);

	_currentItem = static_cast<CParticleTreeItem *>(index.internalPointer());

	if (_currentItem == 0)
		return;

	if (index.flags() != Qt::NoItemFlags)
		_PropertyDialog->setCurrentEditedElement(_currentItem);

	if ((_currentItem->itemType() == ItemType::Workspace) ||
			(_currentItem->itemType() == ItemType::ParticleSystemNotLoaded))
		_currentItem = 0;
}

void CParticleWorkspaceDialog::customContextMenu()
{
	if (!Modules::psEdit().getParticleWorkspace()) return;
	clickedItem(_ui.treeView->currentIndex());
	if (_currentItem == 0) return;
	QMenu *popurMenu = new QMenu(this);
	switch (_currentItem->itemType())
	{
	case ItemType::ParticleSystem:
		popurMenu->addAction(_setActivePSAction);
		popurMenu->addAction(_savePSAction);
		popurMenu->addAction(_saveAsPSAction);
		popurMenu->addAction(_clearContentAction);
		popurMenu->addAction(_removeFromWSAction);
		popurMenu->addAction(_mergeAction);
		popurMenu->addSeparator();
		popurMenu->addAction(_newLocatedAction);
		popurMenu->addAction(_pasteLocatedAction);
		popurMenu->addSeparator();
		buildMenu(popurMenu);
		popurMenu->addSeparator();
		popurMenu->addAction(_forceZBiasAction);
		break;
	case ItemType::Located:
		popurMenu->addAction(_instanciateAction);
		popurMenu->addSeparator();
		buildMenu(popurMenu);
		popurMenu->addSeparator();
		popurMenu->addAction(_copyLocatedAction);
		popurMenu->addAction(_pasteBindableAction);
		popurMenu->addAction(_deleteAction);
		break;
	case ItemType::Force:
	case ItemType::Particle:
	case ItemType::Emitter:
	case ItemType::Light:
	case ItemType::CollisionZone:
	case ItemType::Sound:
		popurMenu->addAction(_copyBindableAction);
		popurMenu->addAction(_deleteAction);
		popurMenu->addSeparator();
		popurMenu->addAction(_allLODAction);
		popurMenu->addAction(_lod1Action);
		popurMenu->addAction(_lod2Action);
		popurMenu->addSeparator();
		popurMenu->addAction(_externIDAction);

		// check the menu to tell which lod is used for this located bindable
		if (_currentItem->getBind()->getLOD() == NL3D::PSLod1n2) _allLODAction->setChecked(true);
		else _allLODAction->setChecked(false);
		if (_currentItem->getBind()->getLOD() == NL3D::PSLod1) _lod1Action->setChecked(true);
		else _lod1Action->setChecked(false);
		if (_currentItem->getBind()->getLOD() == NL3D::PSLod2) _lod2Action->setChecked(true);
		else _lod2Action->setChecked(false);
		break;
	case ItemType::LocatedInstance:
		popurMenu->addAction(_deleteAction);
		break;
	}

	bool stopped = Modules::psEdit().getState() == CParticleEditor::State::Stopped ? true : false;
	_copyLocatedAction->setEnabled(stopped);
	_copyBindableAction->setEnabled(stopped);
	_pasteLocatedAction->setEnabled(stopped);
	_instanciateAction->setEnabled(stopped);
	_savePSAction->setEnabled(stopped);
	_saveAsPSAction->setEnabled(stopped);
	_removeFromWSAction->setEnabled(stopped);
	_clearContentAction->setEnabled(stopped);

	popurMenu->exec(QCursor::pos());
	delete popurMenu;
}

void CParticleWorkspaceDialog::setActiveNode()
{
	QModelIndex index = _ui.treeView->currentIndex();
	CParticleTreeItem *item = static_cast<CParticleTreeItem *>(index.internalPointer());
	nlassert(item->getNode());
	Modules::psEdit().setActiveNode(item->getNode());
	Q_EMIT changeActiveNode();
}

void CParticleWorkspaceDialog::savePS()
{
	_currentItem->getNode()->savePS();
	_currentItem->getNode()->setModified(false);
}

void CParticleWorkspaceDialog::saveAsPS()
{
	CWorkspaceNode *node = _treeModel->getOwnerNode(_currentItem);
	//if (nt->PS->getResetAutoCountFlag() && nt->PS->getPSPointer()->getAutoCountFlag())
	if (node->getPSPointer()->getAutoCountFlag())
	{
		QMessageBox::critical(this, tr("NeL particle system editor"),
							  QString(node->getFilename().c_str()) + tr(" uses auto count feature, and it has been modified. "
									  "You should run the system entirely at least once at full detail before saving so that the editor can compute the number of particles in the system. "
									  "If user params are used to modify system aspect, you should run the system for extreme cases before saving. "),
							  QMessageBox::Ok);
	}
	else
	{
		Modules::psEdit().stop();

		QString fileName = QFileDialog::getSaveFileName(this, tr("Save as ps file"),
						   ".",
						   tr("ps files (*.ps)"));
		// after check
		if (!fileName.isEmpty())
			node->savePSAs(fileName.toUtf8().constData());
	}
}

void CParticleWorkspaceDialog::clearContent()
{
	int ret = QMessageBox::question(this, tr("NeL particle system editor"),
									tr("Clear content ?"), QMessageBox::Yes | QMessageBox::No);

	if (ret == QMessageBox::Yes)
	{
		CWorkspaceNode *node = _treeModel->getOwnerNode(_currentItem);
		nlassert(node);
		node->setResetAutoCountFlag(false);
		while (_currentItem->childCount() != 0)
			_treeModel->removeRows(0, _ui.treeView->currentIndex());
		node->createEmptyPS();
		node->setModified(true);
	}
}

void CParticleWorkspaceDialog::removePS()
{
	CWorkspaceNode *node = _currentItem->getNode();
	if (node == Modules::psEdit().getActiveNode())
		Modules::psEdit().setActiveNode(NULL);

	QModelIndex index = _ui.treeView->currentIndex();
	_ui.treeView->setCurrentIndex(index.parent());
	clickedItem(index.parent());
	Modules::psEdit().getParticleWorkspace()->removeNode(node);
	_treeModel->removeRows(index.row(), index.parent());
}

void CParticleWorkspaceDialog::mergePS()
{
}

void CParticleWorkspaceDialog::newLocated()
{
	_treeModel->getOwnerNode(_currentItem)->setModified(true);
	createLocated(_treeModel->getOwnerNode(_currentItem)->getPSPointer());
}

void CParticleWorkspaceDialog::pasteLocated()
{
	nlassert(_currentItem->getNode());
	_treeModel->getOwnerNode(_currentItem)->setModified(true);

	Modules::psEdit().resetAutoCount(_treeModel->getOwnerNode(_currentItem));

	NL3D::CPSLocated *copy = dynamic_cast<NL3D::CPSLocated *>(::DupPSLocated(_LocatedCopy.get()));
	if (!copy) return;
	if (_currentItem->getNode()->getPSPointer()->attach(copy))
		_treeModel->insertRows(copy, _currentItem->childCount(), _ui.treeView->currentIndex());
	else
	{
		delete copy;
		QMessageBox::critical(this, tr("NeL particle system editor"),
							  tr("Can't perform operation : the system is flagged with 'No max nb steps' or uses the preset 'Spell FX', "
								 "and thus, should have a finite duration. Please remove that flag first."),
							  QMessageBox::Ok);
	}
}

void CParticleWorkspaceDialog::bindNewLocatedBindable(int id)
{
	NL3D::CPSLocatedBindable *toCreate = NULL;
	switch (id)
	{
	case Action::ParticlePoint:
		toCreate = new NL3D::CPSDot;
		break;
	case Action::ParticleLookAt:
		toCreate = new NL3D::CPSFaceLookAt;
		break;
	case Action::ParticleFanLight:
		toCreate = new NL3D::CPSFanLight;
		break;
	case Action::ParticleRibbon:
		toCreate = new NL3D::CPSRibbon;
		break;
	case Action::ParticleTailDot:
		toCreate = new NL3D::CPSTailDot;
		break;
	case Action::ParticleMesh:
		toCreate = new NL3D::CPSMesh;
		break;
	case Action::ParticleConstraintMesh:
		toCreate = new NL3D::CPSConstraintMesh;
		break;
	case Action::ParticleFace:
		toCreate = new NL3D::CPSFace;
		break;
	case Action::ParticleShockWave:
		toCreate = new NL3D::CPSShockWave;
		break;
	case Action::ParticleRibbonLookAt:
		toCreate = new NL3D::CPSRibbonLookAt;
		break;
	case Action::ForceGravity:
		toCreate = new NL3D::CPSGravity;
		break;
	case Action::ForceDirectional:
		toCreate = new NL3D::CPSDirectionnalForce;
		break;
	case Action::ForceSpring:
		toCreate = new NL3D::CPSSpring;
		break;
	case Action::ForceFlyidFriction:
		toCreate = new NL3D::CPSFluidFriction;
		break;
	case Action::ForceCentralGravity:
		toCreate = new NL3D::CPSCentralGravity;
		break;
	case Action::ForceCylindricVortex:
		toCreate = new NL3D::CPSCylindricVortex;
		break;
	case Action::ForceBrownianMove:
		toCreate = new NL3D::CPSBrownianForce;
		break;
	case Action::ForceMagnetic:
		toCreate = new NL3D::CPSMagneticForce;
		break;
	case Action::ZonePlane:
		toCreate = new NL3D::CPSZonePlane;
		break;
	case Action::ZoneSphere:
		toCreate = new NL3D::CPSZoneSphere;
		break;
	case Action::ZoneRectangle:
		toCreate = new NL3D::CPSZoneRectangle;
		break;
	case Action::ZoneDisc:
		toCreate = new NL3D::CPSZoneDisc;
		break;
	case Action::ZoneCylinder:
		toCreate = new NL3D::CPSZoneCylinder;
		break;
	case Action::EmitterDirectional:
		toCreate = new NL3D::CPSEmitterDirectionnal;
		break;
	case Action::EmitterOmniDirectional:
		toCreate = new NL3D::CPSEmitterOmni;
		break;
	case Action::EmitterRectangle:
		toCreate = new NL3D::CPSEmitterRectangle;
		break;
	case Action::EmitterConic:
		toCreate = new NL3D::CPSEmitterConic;
		break;
	case Action::EmitterSpherical:
		toCreate = new NL3D::CPSSphericalEmitter;
		break;
	case Action::EmitterRadial:
		toCreate = new NL3D::CPSRadialEmitter;
		break;
	case Action::Sound:
		toCreate = new NL3D::CPSSound;
		if (!Modules::psEdit().isRunning())
			(static_cast<NL3D::CPSSound *>(toCreate))->stopSound();
		break;
	case Action::Light:
		toCreate = new NL3D::CPSLight;
		break;
	}

	_treeModel->getOwnerNode(_currentItem)->setModified(true);

	NL3D::CPSLocated *loc;

	if (_currentItem->itemType() == ItemType::ParticleSystem)
	{
		loc = createLocated(_treeModel->getOwnerNode(_currentItem)->getPSPointer());
		if (_treeModel->getOwnerNode(_currentItem)->getPSPointer()->getBypassMaxNumIntegrationSteps())
		{
			if (toCreate->getType() == NL3D::PSParticle || toCreate->getType() == NL3D::PSEmitter)
				loc->setInitialLife(1.f);
			// object must have finite duration with that flag
		}
	}
	else
		loc = _currentItem->getLoc();

	if (!loc->bind(toCreate))
	{
		QMessageBox::critical(this, tr("NeL particle system editor"),
							  tr("The system is flagged with 'No max Nb steps',  or uses the preset 'Spell FX'."
								 "System must have finite duration. Can't add object. To solve this,  set a limited life time for the father."),
							  QMessageBox::Ok);
		delete toCreate;
		return;
	}

	// complete the name
	std::string name = toCreate->getName();
	if (_PSElementIdentifiers.count(name))
	{
		name += NLMISC::toString("%u", ++_PSElementIdentifiers[name]);
		toCreate->setName(name);
	}
	else
	{
		_PSElementIdentifiers[toCreate->getName()] = 0;
		toCreate->setName(name + "0");
	}

	touchPSState(_currentItem);

	Modules::psEdit().resetAutoCount(_treeModel->getOwnerNode(_currentItem));

	// update treeView
	if (_currentItem->itemType() == ItemType::ParticleSystem)
	{
		QModelIndex index = _treeModel->index(_currentItem->childCount() - 1, 0, _ui.treeView->currentIndex());
		_treeModel->insertRow(toCreate, 0, index);
	}
	else
		_treeModel->insertRow(toCreate, _currentItem->childCount(), _ui.treeView->currentIndex());
}

void CParticleWorkspaceDialog::forceZBias()
{
	bool ok;
	double d = QInputDialog::getDouble(this, tr("All object force ZBias"), tr(""), 0.0, -999999.0, 999999.0, 2, &ok);
	if (!ok) return;

	nlassert(_treeModel->getOwnerNode(_currentItem)->getPSPointer());

	_treeModel->getOwnerNode(_currentItem)->getPSPointer()->setZBias(-float(d));
	_treeModel->getOwnerNode(_currentItem)->setModified(true);
}

void CParticleWorkspaceDialog::copyLocated()
{
	nlassert(_currentItem->getLoc());
	_LocatedCopy.reset(NLMISC::safe_cast<NL3D::CPSLocated *>(::DupPSLocated(_currentItem->getLoc())));
}

void CParticleWorkspaceDialog::copyBindable()
{
	nlassert(_currentItem->getBind());
	_LocatedBindableCopy.reset(::DupPSLocatedBindable(_currentItem->getBind()));
}

void CParticleWorkspaceDialog::pasteBindable()
{
	nlassert(_currentItem->getLoc());
	_treeModel->getOwnerNode(_currentItem)->setModified(true);

	Modules::psEdit().resetAutoCount(_treeModel->getOwnerNode(_currentItem));

	NL3D::CPSLocatedBindable *copy = ::DupPSLocatedBindable(_LocatedBindableCopy.get());
	if (!copy) return;
	if (_currentItem->getLoc()->bind(copy))
		_treeModel->insertRow(copy, _currentItem->childCount(), _ui.treeView->currentIndex());
	else
	{
		delete copy;
		QMessageBox::critical(this, tr("NeL particle system editor"),
							  tr("Can't perform operation : the system is flagged with 'No max nb steps' or uses the preset 'Spell FX', "
								 "and thus, should have a finite duration. Please remove that flag first."),
							  QMessageBox::Ok);
	}
}

void CParticleWorkspaceDialog::deleteItem()
{
	_treeModel->getOwnerNode(_currentItem)->setModified(true);
	QModelIndex index = _ui.treeView->currentIndex();
	CParticleTreeItem *item = static_cast<CParticleTreeItem *>(index.internalPointer());
	CWorkspaceNode *ownerNode = _treeModel->getOwnerNode(item);
	nlassert(ownerNode);
	_ui.treeView->setCurrentIndex(index.parent());
	clickedItem(index.parent());
	switch(item->itemType())
	{
	case ItemType::Located:
	{
		NL3D::CPSLocated *loc = item->getLoc();
		touchPSState(item);
		ownerNode->setModified(true);
		// if the system is running,  we must destroy initial infos about the located,
		// as they won't need to be restored when the stop button will be pressed
		ownerNode->removeLocated(loc);

		Modules::psEdit().resetAutoCount(ownerNode);

		ownerNode->getPSPointer()->remove(loc);
		_treeModel->removeRows(index.row(), index.parent());
	}
	break;
	case ItemType::LocatedInstance:
	{
		Modules::psEdit().resetAutoCount(ownerNode);
		NL3D::CPSEmitter::setBypassEmitOnDeath(true);
		item->getLoc()->deleteElement(item->getLocatedInstanceIndex());
		NL3D::CPSEmitter::setBypassEmitOnDeath(false);
		_treeModel->removeRows(index.row(), index.parent());
		_treeModel->rebuildLocatedInstance(_ui.treeView->currentIndex());
	}
	break;
	case ItemType::Particle:
	case ItemType::Emitter:
	case ItemType::Force:
	case ItemType::Light:
	case ItemType::Sound:
	case ItemType::CollisionZone:
	{
		NL3D::CPSLocatedBindable *lb = item->getBind();
		touchPSState(item);
		// if the system is running,  we must destroy initial infos
		// that what saved about the located bindable,  when the start button was pressed,  as they won't need
		// to be restored
		ownerNode->removeLocatedBindable(lb);
		ownerNode->setModified(true);
		Modules::psEdit().resetAutoCount(ownerNode);
		lb->getOwner()->remove(lb);
		_treeModel->removeRows(index.row(), index.parent());
	}
	break;
	}
}

void CParticleWorkspaceDialog::setInstanciate()
{
	_treeModel->getOwnerNode(_currentItem)->setModified(true);

	Modules::psEdit().resetAutoCount(_treeModel->getOwnerNode(_currentItem));

	if (_currentItem->getLoc()->getSize() == _currentItem->getLoc()->getMaxSize())
		_currentItem->getLoc()->resize(_currentItem->getLoc()->getMaxSize() + 1);

	sint32 objIndex = _currentItem->getLoc()->newElement(NLMISC::CVector::Null, NLMISC::CVector::Null,
					  NULL, 0, _currentItem->getLoc()->getMatrixMode(), 0.f);

	_treeModel->insertRow(_currentItem->getLoc(), objIndex, _currentItem->childCount(), _ui.treeView->currentIndex());
}

void CParticleWorkspaceDialog::setAllLOD()
{
	_currentItem->getBind()->setLOD(NL3D::PSLod1n2);
	_treeModel->getOwnerNode(_currentItem)->setModified(true);
}

void CParticleWorkspaceDialog::setLOD1()
{
	_currentItem->getBind()->setLOD(NL3D::PSLod1);
	_treeModel->getOwnerNode(_currentItem)->setModified(true);
}

void CParticleWorkspaceDialog::setLOD2()
{
	_currentItem->getBind()->setLOD(NL3D::PSLod2);
	_treeModel->getOwnerNode(_currentItem)->setModified(true);
}

void CParticleWorkspaceDialog::setExternID()
{
	bool ok;
	int i = QInputDialog::getInt(this, tr("Set the extern ID"),
								 tr("0 means no extern access."),
								 _currentItem->getBind()->getExternID(), 0, 9999, 1, &ok);
	if (ok)
	{
		_currentItem->getBind()->setExternID(uint32(i));
		_treeModel->getOwnerNode(_currentItem)->setModified(true);
	}
}

void CParticleWorkspaceDialog::setNewState()
{
	if ((_currentItem != NULL) && (_currentItem->itemType() == ItemType::LocatedInstance))
	{
		QModelIndex index = _ui.treeView->currentIndex();
		_ui.treeView->setCurrentIndex(_ui.treeView->currentIndex().parent());
		clickedItem(index.parent());
	}
	_ui.treeView->update();
	_ui.treeView->repaint();
}

void CParticleWorkspaceDialog::updateTreeView()
{
	_ui.treeView->update();
	_ui.treeView->repaint();
}

void CParticleWorkspaceDialog::buildMenu(QMenu *menu)
{
	QMenu *bindParticleMenu = new QMenu(tr("Bind particle..."), menu);
	bindParticleMenu->setIcon(QIcon(Constants::ICON_PARTICLE_SYSTEM_SMALL));
	menu->addAction(bindParticleMenu->menuAction());
	for(int i = Action::ParticlePoint; i <= Action::ParticleRibbonLookAt; ++i)
		bindParticleMenu->addAction(_bindNewLocatedBindable[i]);

	QMenu *bindForceMenu = new QMenu(tr("Bind force..."), menu);
	bindForceMenu->setIcon(QIcon(Constants::ICON_FORCE_ITEM_SMALL));
	menu->addAction(bindForceMenu->menuAction());
	for(int i = Action::ForceGravity; i <= Action::ForceMagnetic; ++i)
		bindForceMenu->addAction(_bindNewLocatedBindable[i]);

	QMenu *bindZoneMenu = new QMenu(tr("Bind zone..."), menu);
	bindZoneMenu->setIcon(QIcon(Constants::ICON_COLLISION_ZONE_ITEM_SMALL));
	menu->addAction(bindZoneMenu->menuAction());
	for(int i = Action::ZonePlane; i <= Action::ZoneCylinder; ++i)
		bindZoneMenu->addAction(_bindNewLocatedBindable[i]);

	QMenu *bindEmitterMenu = new QMenu(tr("Bind emitter..."), menu);
	bindEmitterMenu->setIcon(QIcon(Constants::ICON_EMITTER_ITEM_SMALL));
	menu->addAction(bindEmitterMenu->menuAction());
	for(int i = Action::EmitterDirectional; i <= Action::EmitterRadial; ++i)
		bindEmitterMenu->addAction(_bindNewLocatedBindable[i]);

	menu->addAction(_bindNewLocatedBindable[Action::Sound]);
	menu->addAction(_bindNewLocatedBindable[Action::Light]);
}

NL3D::CPSLocated *CParticleWorkspaceDialog::createLocated(NL3D::CParticleSystem *ps)
{
	// build new name
	std::string name;
	if (_PSElementIdentifiers.count("located"))
	{
		name = NLMISC::toString("located %u", ++_PSElementIdentifiers["located"]);
	}
	else
	{
		name = "located 0";
		_PSElementIdentifiers["located"] = 0;
	}
	NL3D::CPSLocated *loc = new NL3D::CPSLocated;

	loc->setName(name);
	loc->setMatrixMode(NL3D::PSFXWorldMatrix);
	ps->attach(loc);

	touchPSState(_currentItem);

	// update treeView
	_treeModel->insertRows(loc, _currentItem->childCount(), _ui.treeView->currentIndex());

	return loc;
}

} /* namespace NLQT */