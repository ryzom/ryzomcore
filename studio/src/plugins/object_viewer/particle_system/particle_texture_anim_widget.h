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

#ifndef PARTICLE_TEXTURE_ANIM_WIDGET_H
#define PARTICLE_TEXTURE_ANIM_WIDGET_H

#include "ui_particle_texture_anim_form.h"

// STL includes

// Qt includes

// NeL includes
#include "nel/3d/ps_particle.h"

// Project includes
#include "ps_wrapper.h"

namespace NL3D
{
class CPSTexturedParticle;
class CPSMultiTexturedParticle;
}

namespace NLQT
{

class CParticleTextureAnimWidget: public QWidget
{
	Q_OBJECT

public:
	CParticleTextureAnimWidget(QWidget *parent = 0);
	~CParticleTextureAnimWidget();

	void setCurrentTextureAnim(NL3D::CPSTexturedParticle *tp, NL3D::CPSMultiTexturedParticle *mtp,CWorkspaceNode *ownerNode);

private Q_SLOTS:
	void chooseGroupedTexture();
	void setEnabledTexAnim(bool state);
	void setMultitexturing(bool enabled);
	void editMultitexturing();

private:
	void updateTexAnimState(bool state);

	/// Wrapper for single texture
	struct CTextureWrapper : public IPSWrapperTexture
	{
		NL3D::CPSTexturedParticle *P;
		NL3D::ITexture *get(void)
		{
			return P->getTexture();
		}
		void set(NL3D::ITexture *t)
		{
			P->setTexture(t);
		}
	} _TextureWrapper;

	/// Wrapper for texture anim sequence
	struct CTextureIndexWrapper : public IPSWrapper<sint32>, IPSSchemeWrapper<sint32>
	{
		NL3D::CPSTexturedParticle *P;
		sint32 get(void) const
		{
			return P->getTextureIndex();
		}
		void set(const sint32 &v)
		{
			P->setTextureIndex(v);
		}
		scheme_type *getScheme(void) const
		{
			return P->getTextureIndexScheme();
		}
		void setScheme(scheme_type *s)
		{
			P->setTextureIndexScheme(s);
		}
	} _TextureIndexWrapper;

	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	NL3D::CPSTexturedParticle *_EditedParticle;

	NL3D::CPSMultiTexturedParticle *_MTP;

	CWorkspaceNode *_Node;

	Ui::CParticleTextureAnimWidget _ui;
}; /* class CParticleTextureAnimWidget */

} /* namespace NLQT */

#endif // PARTICLE_TEXTURE_ANIM_WIDGET_H
