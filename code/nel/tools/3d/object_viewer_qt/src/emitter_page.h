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

#ifndef EMITTER_PAGE_H
#define EMITTER_PAGE_H

#include <nel/misc/types_nl.h>
#include "ui_emitter_form.h"

// STL includes

// NeL includes
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_emitter.h"

// Project includes
#include "particle_node.h"
#include "ps_wrapper.h"

namespace NLQT
{

/**
@class CEmitterPage
@brief Page for QStackWidget, to edit emitters in a particle system
*/
class CEmitterPage: public QWidget
{
	Q_OBJECT

public:
	/// This enum match the option in the combo box that allow to choose how the direction of emission is computed.
	enum TDirectionMode
	{
		Default = 0,
		AlignOnEmitterDirection,
		InWorld,
		LocalToSystem,
		LocalToFatherSkeleton
	};

	CEmitterPage(QWidget *parent = 0);
	~CEmitterPage();

	/// Set the emitter to edit.
	void setEditedItem(CWorkspaceNode *ownerNode, NL3D::CPSLocatedBindable *locatedBindable);

private Q_SLOTS:
	void setEmittedType(int index);
	void setTypeOfEmission(int index);
	void setConsistentEmission(bool state);
	void setBypassAutoLOD(bool state);
	void setDirectionMode(int index);

	void setSpeedInheritanceFactor(float value);
	void setConicEmitterRadius(float value);
	void setEmitDelay(float value);
	void setMaxEmissionCount(uint32 value);
	void setDir(const NLMISC::CVector &value);

private:

	/// period of emission
	struct CPeriodWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		CWorkspaceNode *Node;
		NL3D::CPSEmitter *E;
		float get(void) const
		{
			return E->getPeriod();
		}
		void set(const float &v);
		scheme_type *getScheme(void) const
		{
			return E->getPeriodScheme();
		}
		void setScheme(scheme_type *s);
	} _PeriodWrapper;

	/// number of particle to generate each time
	struct CGenNbWrapper : public IPSWrapperUInt, IPSSchemeWrapperUInt
	{
		CWorkspaceNode *Node;
		NL3D::CPSEmitter *E;
		uint32 get(void) const
		{
			return E->getGenNb();
		}
		void set(const uint32 &v);
		scheme_type *getScheme(void) const
		{
			return E->getGenNbScheme();
		}
		void setScheme(scheme_type *s);
	} _GenNbWrapper;

	/// wrappers to emitters that have strenght modulation
	struct CModulateStrenghtWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat
	{
		NL3D::CPSModulatedEmitter *E;
		float get(void) const
		{
			return E->getEmitteeSpeed();
		}
		void set(const float &v)
		{
			E->setEmitteeSpeed(v);
		}
		scheme_type *getScheme(void) const
		{
			return E->getEmitteeSpeedScheme();
		}
		void setScheme(scheme_type *s)
		{
			E->setEmitteeSpeedScheme(s);
		}
	} _ModulatedStrenghtWrapper;

	// the emitter being edited
	NL3D::CPSEmitter *_Emitter;

	// contains pointers to the located
	std::vector<NL3D::CPSLocated *> _LocatedList;

	void updateEmittedType();

	void updatePeriodWidget();

	void updateModifiedFlag()
	{
		if (_Node) _Node->setModified(true);
	}

	CWorkspaceNode *_Node;

	Ui::CEmitterPage _ui;
}; /* class CEmitterPage */

} /* namespace NLQT */

#endif // EMITTER_PAGE_H
