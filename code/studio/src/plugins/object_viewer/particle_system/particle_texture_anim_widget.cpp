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
#include "particle_texture_anim_widget.h"

// NeL includes
#include <nel/3d/ps_particle.h>
#include <nel/3d/ps_particle_basic.h>
#include <nel/3d/texture_grouped.h>
#include <nel/3d/texture_file.h>
#include <nel/misc/smart_ptr.h>

// Projects includes
#include "value_gradient_dialog.h"
#include "multi_tex_dialog.h"

namespace NLQT
{

CParticleTextureAnimWidget::CParticleTextureAnimWidget(QWidget *parent)
	: QWidget(parent)
{
	_ui.setupUi(this);

	_ui.texIndexWidget->setRange(0, 1);
	_ui.texIndexWidget->setWrapper(&_TextureIndexWrapper );
	_ui.texIndexWidget->setSchemeWrapper(&_TextureIndexWrapper );
	_ui.texIndexWidget->init();

	_ui.texWidget->setWrapper(&_TextureWrapper);

	connect(_ui.texAnimCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEnabledTexAnim(bool)));
	connect(_ui.multitexturingCheckBox, SIGNAL(toggled(bool)), this, SLOT(setMultitexturing(bool)));
	connect(_ui.editPushButton, SIGNAL(clicked()), this, SLOT(editMultitexturing()));
	connect(_ui.textureGroupedPushButton, SIGNAL(clicked()), this, SLOT(chooseGroupedTexture()));
}

CParticleTextureAnimWidget::~CParticleTextureAnimWidget()
{
}

void CParticleTextureAnimWidget::setCurrentTextureAnim(NL3D::CPSTexturedParticle *tp, NL3D::CPSMultiTexturedParticle *mtp,CWorkspaceNode *ownerNode)
{
	_Node = ownerNode;
	_EditedParticle = tp;
	_MTP = mtp;

	disconnect(_ui.texAnimCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEnabledTexAnim(bool)));

	bool isAnimTex = _EditedParticle->getTextureGroup() ? true : false;
	_ui.texAnimCheckBox->setChecked(isAnimTex);
	updateTexAnimState(isAnimTex);

	if (_MTP)
	{
		_ui.multitexturingCheckBox->setChecked(_MTP->isMultiTextureEnabled());
		_ui.multitexturingGroupBox->show();
	}
	else
		_ui.multitexturingGroupBox->hide();

	connect(_ui.texAnimCheckBox, SIGNAL(toggled(bool)), this, SLOT(setEnabledTexAnim(bool)));
}

void CParticleTextureAnimWidget::setEnabledTexAnim(bool state)
{
	if (state)
	{
		if (_MTP)
			_ui.multitexturingCheckBox->setChecked(false);

		// When you try to load a dummy texture, remove alternative paths, an assertion is thrown otherwise
		NLMISC::CPath::removeAllAlternativeSearchPath();

		// put a dummy texture as a first texture
		NLMISC::CSmartPtr<NL3D::ITexture> tex = (NL3D::ITexture *) new NL3D::CTextureFile(std::string(""));
		NL3D::CTextureGrouped *tg = new NL3D::CTextureGrouped;
		tg->setTextures(&tex, 1);
		_EditedParticle->setTextureGroup(tg);
		_EditedParticle->setTextureIndex(0);
	}
	else
	{
		_EditedParticle->setTexture(NULL);
	}
	updateTexAnimState(state);
	updateModifiedFlag();
}

void CParticleTextureAnimWidget::chooseGroupedTexture()
{
	CTextureGradientInterface *texInterface = new CTextureGradientInterface(_EditedParticle, _Node);
	CGradientDialog *gd = new CGradientDialog(_Node, texInterface, true, false, 1, this);
	gd->setModal(true);
	gd->show();
	gd->exec();
	delete gd;
}

void CParticleTextureAnimWidget::setMultitexturing(bool enabled)
{
	if (_MTP->isMultiTextureEnabled() != enabled)
	{
		_MTP->enableMultiTexture(enabled);
		updateModifiedFlag();
	}
}

void CParticleTextureAnimWidget::editMultitexturing()
{
	CMultiTexDialog *multiTexDialog = new CMultiTexDialog(_Node, _MTP, this);
	multiTexDialog->show();
	multiTexDialog->exec();
	delete multiTexDialog;
}

void CParticleTextureAnimWidget::updateTexAnimState(bool state)
{
	if (state)
	{
		_ui.texIndexWidget->setRange( 0, _EditedParticle->getTextureGroup()->getNbTextures() - 1);
		_TextureIndexWrapper.P = _EditedParticle;
		_ui.texIndexWidget->setWorkspaceNode(_Node);
		_ui.texIndexWidget->updateUi();
	}
	else
	{
		_TextureWrapper.P = _EditedParticle;
		_TextureWrapper.OwnerNode = _Node;

		_ui.texWidget->updateUi();
	}
	_ui.texWidget->setVisible(!state);
	_ui.texIndexWidget->setVisible(state);
}

} /* namespace NLQT */