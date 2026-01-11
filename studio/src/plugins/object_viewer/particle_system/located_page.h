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

#ifndef LOCATED_PAGE_H
#define LOCATED_PAGE_H

#include <nel/misc/types_nl.h>
#include "ui_located_form.h"

// Qt includes

// STL includes

// NeL includes

// Project includes

namespace  NL3D
{
class CPSLocated;
}

namespace NLQT
{

/**
@class CLocatedPage
@brief Page for QStackWidget, to edit located in a particle system
*/
class CLocatedPage: public QWidget
{
	Q_OBJECT

public:
	CLocatedPage(QWidget *parent = 0);
	~CLocatedPage();

	/// Set the located to edit.
	void setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocated *located);

private Q_SLOTS:
	void setDisabledCountPS(bool state);
	void setLimitedLifeTime(bool state);
	void setDisgradeWithLod(bool state);
	void setParametricMotion(bool state);
	void editTriggerOnDeath();
	void setTriggerOnDeath(bool state);
	void setMatrixMode(int index);
	void setCurrentCount();

	void setNewMaxSize(uint32 value);

private:

	/// wrapper to tune the mass of particles
	struct CMassWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSLocated *Located;
		float get(void) const
		{
			return Located->getInitialMass();
		}
		void set(const float &v)
		{
			Located->setInitialMass(v);
		}
		virtual scheme_type *getScheme(void) const
		{
			return Located->getMassScheme();
		}
		virtual void setScheme(scheme_type *s)
		{
			Located->setMassScheme(s);
		}
	} _MassWrapper;

	struct CLifeWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSLocated *Located;
		CWorkspaceNode *Node;
		float get(void) const
		{
			return Located->getInitialLife();
		}
		void set(const float &v);
		virtual scheme_type *getScheme(void) const
		{
			return Located->getLifeScheme();
		}
		virtual void setScheme(scheme_type *s);
	} _LifeWrapper;

	/// the located this dialog is editing
	NL3D::CPSLocated *_Located;

	CWorkspaceNode *_Node;

	/// update the integrable check box
	void updateIntegrable(void);

	/// update the 'trigger on death' control
	void updateTriggerOnDeath(void);

	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	Ui::CLocatedPage _ui;
}; /* class CLocatedPage */

} /* namespace NLQT */


#endif // LOCATED_PAGE_H
