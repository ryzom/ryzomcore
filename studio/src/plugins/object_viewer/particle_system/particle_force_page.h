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

#ifndef PARTICLE_FORCE_PAGE_H
#define PARTICLE_FORCE_PAGE_H

#include <nel/misc/types_nl.h>
#include "ui_particle_force_form.h"

// Qt includes

// NeL includes
#include <nel/3d/ps_located.h>
#include <nel/3d/ps_force.h>

// Project includes
#include "particle_node.h"
#include "ps_wrapper.h"

namespace NLQT
{

/**
@class CLocatedItem
@brief Contain pointer to CPSLocated.
*/
class CLocatedItem: public QListWidgetItem
{
public:
	CLocatedItem ( const QString &text, QListWidget *parent = 0, int type = UserType ):
		QListWidgetItem(text, parent, type), _loc(NULL) {}

	void setUserData(NL3D::CPSLocated *loc)
	{
		_loc = loc;
	}
	NL3D::CPSLocated *getUserData() const
	{
		return _loc;
	}

private:

	NL3D::CPSLocated *_loc;
}; /* class CLocatedItem */

/**
@class CForcePage
@brief Page for QStackWidget, to edit forces in a particle system
*/
class CForcePage: public QWidget
{
	Q_OBJECT

public:
	CForcePage(QWidget *parent = 0);
	virtual ~CForcePage();

	/// Set the force to edit.
	void setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable);

private Q_SLOTS:
	void addTarget();
	void removeTarget();

	void setRadialViscosity(float value);
	void setTangentialViscosity(float value);
	void setDir(const NLMISC::CVector &value);
	void setGlobalName(const QString &globalName);
	void setFactorBrownianForce(float value);

private:

	/// wrapper to tune the intensity of a force
	struct CForceIntensityWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSForceIntensity *F;
		float get(void) const
		{
			return F->getIntensity();
		}
		void set(const float &value)
		{
			F->setIntensity(value);
		}
		scheme_type *getScheme(void) const
		{
			return F->getIntensityScheme();
		}
		void setScheme(scheme_type *s)
		{
			F->setIntensityScheme(s);
		}
	} _ForceIntensityWrapper;

	void hideAdditionalWidget();

	void updateTargets();

	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	// the target we're focusing on
	NL3D::CPSTargetLocatedBindable *_LBTarget;

	CWorkspaceNode *_Node;

	Ui::CForcePage _ui;

}; /* class CForcePage */

} /* namespace NLQT */

#endif // PARTICLE_FORCE_PAGE_H
