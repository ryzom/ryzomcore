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
#include "particle_control_dialog.h"

// Qt includes
#include <QtCore/QTimer>

// NeL includes
#include "nel/3d/scene.h"
#include <nel/3d/particle_system.h>

// Project includes
#include "skeleton_tree_model.h"
#include "particle_link_skeleton_dialog.h"
#include "modules.h"

namespace NLQT
{

CParticleControlDialog::CParticleControlDialog(CSkeletonTreeModel *model, QWidget *parent)
	: QDockWidget(parent)
{
	_ui.setupUi(this);

	_timer = new QTimer(this);

	_particleLinkDialog = new CParticleLinkDialog(model, parent);
	_particleLinkDialog->setVisible(false);

	_ui.numParticlesLabel->setMinimumWidth(_ui.numParticlesLabel->sizeHint().width());
	_ui.numWantedFacesLabel->setMinimumWidth(_ui.numWantedFacesLabel->sizeHint().width());
	_ui.systemTimesLabel->setMinimumWidth(_ui.systemTimesLabel->sizeHint().width());

	stop();

	connect(_timer, SIGNAL(timeout()), this, SLOT(updateCount()));

	connect(_ui.playToolButton, SIGNAL(clicked()), this, SLOT(play()));
	connect(_ui.stopToolButton, SIGNAL(clicked()), this, SLOT(stop()));
	connect(_ui.horizontalSlider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));
	connect(_ui.helpersCheckBox, SIGNAL(toggled(bool)), this, SLOT(displayHelpers(bool)));
	connect(_ui.displayBoxCheckBox, SIGNAL(toggled(bool)), this, SLOT(displayBBox(bool)));
	connect(_ui.loopCheckBox, SIGNAL(toggled(bool)), this, SLOT(autoRepeat(bool)));
	connect(_ui.autoCountCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEnabledAutoCount(bool)));
	connect(_ui.resetAutoCountPushButton, SIGNAL(clicked()), this, SLOT(resetAutoCount()));

	connect(_ui.linkSkelPushButton, SIGNAL(clicked()), this, SLOT(linkSceleton()));
	connect(_ui.unlinkSkelPushButton, SIGNAL(clicked()), this, SLOT(unlink()));
	connect(_ui.setAnimPushButton, SIGNAL(clicked()), this, SLOT(setAnim()));
	connect(_ui.clearAnimPushButton, SIGNAL(clicked()), this, SLOT(clearAnim()));
	connect(_ui.restickPushButton, SIGNAL(clicked()), this, SLOT(restickObjects()));

	connect(_ui.generalToolButton, SIGNAL(clicked()), this, SLOT(setGeneralWidget()));
	connect(_ui.additionalToolButton, SIGNAL(clicked()), this, SLOT(setAddtitionalWidget()));
}

CParticleControlDialog::~CParticleControlDialog()
{
}

void CParticleControlDialog::updateActiveNode()
{
	_ui.skelLineEdit->setText(Modules::psEdit().getActiveNode()->getParentSkelName().c_str());
	_ui.animLineEdit->setText(Modules::psEdit().getActiveNode()->getTriggerAnim().c_str());
	_ui.autoCountCheckBox->setChecked(Modules::psEdit().getActiveNode()->getPSPointer()->getAutoCountFlag());
}

void CParticleControlDialog::play()
{
	if (Modules::psEdit().getActiveNode() == NULL)
	{
		_ui.playToolButton->setChecked(false);
		return;
	}
	if (Modules::psEdit().isRunning())
		Modules::psEdit().pause();
	else if (_ui.multipleCheckBox->checkState() == Qt::Checked)
		Modules::psEdit().startMultiple();
	else
		Modules::psEdit().start();

	_timer->start(200);
	_ui.multipleCheckBox->setEnabled(false);
	Q_EMIT changeState();
}

void CParticleControlDialog::stop()
{
	_timer->stop();
	_ui.playToolButton->setChecked(false);
	Modules::psEdit().stop();
	_ui.multipleCheckBox->setEnabled(true);

	_ui.numParticlesLabel->setText(tr("Num particles:"));
	_ui.numWantedFacesLabel->setText(tr("Num wanted faces:"));
	_ui.systemTimesLabel->setText(tr("System time:"));

	Q_EMIT changeState();
}

void CParticleControlDialog::sliderMoved(int value)
{
	Modules::psEdit().setSpeed(float(value) / 100);
}

void CParticleControlDialog::displayHelpers(bool state)
{
	Modules::psEdit().setDisplayHelpers(state);
}

void CParticleControlDialog::displayBBox(bool state)
{
	Modules::psEdit().setDisplayBBox(state);
}

void CParticleControlDialog::autoRepeat(bool state)
{
	Modules::psEdit().setAutoRepeat(state);
}

void CParticleControlDialog::setEnabledAutoCount(bool state)
{
	resetAutoCount();
	Modules::psEdit().enableAutoCount(state);
	_ui.resetAutoCountPushButton->setEnabled(state);

	Q_EMIT changeAutoCount(state);
}

void CParticleControlDialog::resetAutoCount()
{
	stop();
	Modules::psEdit().resetAutoCount(Modules::psEdit().getActiveNode());
}

void CParticleControlDialog::updateCount()
{
	if (Modules::psEdit().getActiveNode() == NULL)
		return;

	NL3D::CParticleSystem *ps = Modules::psEdit().getActiveNode()->getPSPointer();
	sint currNumParticles = (sint) ps->getCurrNumParticles();

	// display number of particles for the currently active node
	_ui.numParticlesSpinBox->setValue(currNumParticles);

	// display max number of wanted faces
	NLMISC::CMatrix camMat = ps->getScene()->getCam()->getMatrix();
	sint numWantedFaces = (uint) ps->getWantedNumTris((ps->getSysMat().getPos() - camMat.getPos()).norm());
	_ui.numWantedFacesSpinBox->setValue(numWantedFaces);

	// display system date
	_ui.timeDoubleSpinBox->setValue(ps->getSystemDate());

	Q_EMIT changeCount();
}

void CParticleControlDialog::linkSceleton()
{
	_particleLinkDialog->show();
}

void CParticleControlDialog::unlink()
{
	CWorkspaceNode *node = Modules::psEdit().getActiveNode();
	if (node == NULL)
		return;

	node->unstickPSFromSkeleton();
}

void CParticleControlDialog::setAnim()
{
}

void CParticleControlDialog::clearAnim()
{
}

void CParticleControlDialog::restickObjects()
{
	CParticleWorkspace *pw = Modules::psEdit().getParticleWorkspace();
	if (pw == NULL)
		return;

	pw->restickAllObjects();
}

void CParticleControlDialog::setGeneralWidget()
{
	_ui.stackedWidget->setCurrentWidget(_ui.pageGeneral);
}

void CParticleControlDialog::setAddtitionalWidget()
{
	_ui.stackedWidget->setCurrentWidget(_ui.pageAdditional);
}

} /* namespace NLQT */