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

namespace NLQT {

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
	
private:

	/// period of emission
	struct CPeriodWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
	{
		CWorkspaceNode *Node;
		NL3D::CPSEmitter *E;
		float get(void) const { return E->getPeriod(); }
		void set(const float &v);
		scheme_type *getScheme(void) const { return E->getPeriodScheme(); }
		void setScheme(scheme_type *s);
	} _PeriodWrapper;

	/// number of particle to generate each time
	struct CGenNbWrapper : public IPSWrapperUInt, IPSSchemeWrapperUInt 
	{
		CWorkspaceNode *Node;
		NL3D::CPSEmitter *E;
		uint32 get(void) const { return E->getGenNb(); }
		void set(const uint32 &v);
		 scheme_type *getScheme(void) const { return E->getGenNbScheme(); }
		void setScheme(scheme_type *s);
	} _GenNbWrapper;

	/// wrappers to emitters that have strenght modulation
	struct CModulateStrenghtWrapper : public IPSWrapperFloat, IPSSchemeWrapperFloat 
	{
		NL3D::CPSModulatedEmitter *E;
		float get(void) const { return E->getEmitteeSpeed(); }
		void set(const float &v) { E->setEmitteeSpeed(v); }
		scheme_type *getScheme(void) const { return E->getEmitteeSpeedScheme(); }
		void setScheme(scheme_type *s) { E->setEmitteeSpeedScheme(s); }
	} _ModulatedStrenghtWrapper;

	/// wrappers to set the speed inheritance factor
	struct CSpeedInheritanceFactorWrapper : public IPSWrapperFloat
	{
		NL3D::CPSEmitter *E;
		float get(void) const { return E->getSpeedInheritanceFactor(); }
		void set(const float &f) { E->setSpeedInheritanceFactor(f); }
	} _SpeedInheritanceFactorWrapper;

	/// wrappers to tune the direction of emitters
	struct CDirectionWrapper : public IPSWrapper<NLMISC::CVector>
	{
		NL3D::CPSDirection *E;
		NLMISC::CVector get(void) const { return E->getDir(); }
		void set(const NLMISC::CVector &d){ E->setDir(d); }
	} _DirectionWrapper;

	/// wrapper to tune the radius of an emitter
	struct CConicEmitterRadiusWrapper : public IPSWrapperFloat
	{
		NL3D::CPSEmitterConic *E;
		float get(void) const { return E->getRadius(); }
		void set(const float &f) { E->setRadius(f); }
	} _ConicEmitterRadiusWrapper;

	/// wrapper to tune delayed emission
	struct CDelayedEmissionWrapper : public IPSWrapperFloat
	{
		CWorkspaceNode *Node;
		NL3D::CPSEmitter *E;
		float get(void) const { return E->getEmitDelay(); }
		void set(const float &f);
	} _DelayedEmissionWrapper;

	/// wrapper to tune max number of emissions
	struct CMaxEmissionCountWrapper : public IPSWrapperUInt
	{
		CWorkspaceNode *Node;
		CEditRangeUIntWidget   *widget;
		QWidget 		*parent;
		NL3D::CPSEmitter *E;
		uint32 get(void) const { return E->getMaxEmissionCount(); }
		void set(const uint32 &count);
	} _MaxEmissionCountWrapper;


	// the emitter being edited
	NL3D::CPSEmitter	 *_Emitter;

	// contains pointers to the located
	std::vector<NL3D::CPSLocated *> _LocatedList;
	
	void updateEmittedType();
	
	void updatePeriodWidget();
	
	void updateModifiedFlag() { if (_Node) _Node->setModified(true); }

	CWorkspaceNode *_Node;

	Ui::CEmitterPage _ui;
}; /* class CEmitterPage */

} /* namespace NLQT */

#endif // EMITTER_PAGE_H
