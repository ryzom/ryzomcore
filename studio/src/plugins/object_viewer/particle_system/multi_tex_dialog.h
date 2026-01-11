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

#ifndef MULTI_TEX_DIALOG_H
#define MULTI_TEX_DIALOG_H

#include "ui_multi_tex_form.h"

// STL includes

// Qt includes

// NeL includes

// Project includes
#include "ps_wrapper.h"

namespace NL3D
{
class CPSMultiTexturedParticle;
}

namespace NLQT
{

class CWorkspaceNode;

class CMultiTexDialog: public QDialog
{
	Q_OBJECT

public:
	CMultiTexDialog(CWorkspaceNode *ownerNode, NL3D::CPSMultiTexturedParticle *mtp, QWidget *parent = 0);
	~CMultiTexDialog();

private Q_SLOTS:
	void setEnabledAlternate(bool state);
	void updateValues();
	void updateValuesAlternate();
	void setAlternateOp(int index);
	void setMainOp(int index);
	void setForceBasicCaps(bool state);
	void setUseParticleDate(bool state);
	void setUseParticleDateAlt(bool state);

private:
	struct CMainTexWrapper : IPSWrapperTexture
	{
		NL3D::CPSMultiTexturedParticle *MTP;
		virtual NL3D::ITexture *get(void);
		virtual void set(NL3D::ITexture *);
	} _TexWrapper;

	struct CAlternateTexWrapper : IPSWrapperTexture
	{
		NL3D::CPSMultiTexturedParticle *MTP;
		virtual NL3D::ITexture *get(void);
		virtual void set(NL3D::ITexture *);
	} _AlternateTexWrapper;

	CWorkspaceNode *_Node;

	NL3D::CPSMultiTexturedParticle *_MTP;

	void readValues();
	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	Ui::CMultiTexDialog _ui;
}; /* class CMultiTexDialog */

} /* namespace NLQT */

#endif // MULTI_TEX_DIALOG_H
