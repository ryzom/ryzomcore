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

#ifndef PARTICLE_SYSTEM_PAGE_H
#define PARTICLE_SYSTEM_PAGE_H

#include <nel/misc/types_nl.h>
#include "ui_particle_system_form.h"

// STL includes

// NeL includes
#include <nel/misc/config_file.h>
#include <nel/3d/particle_system.h>
#include "ps_wrapper.h"

// Project includes
#include "edit_range_widget.h"

namespace NLQT
{
/**
@class CParticleSystemPage
@brief Page for QStackWidget, to edit workspace node in a particle system
*/
class CParticleSystemPage: public QWidget
{
	Q_OBJECT

public:
	CParticleSystemPage(QWidget *parent = 0);
	~CParticleSystemPage();

	/// Set the workspace node to edit.
	void setEditedParticleSystem(CWorkspaceNode *node);

private Q_SLOTS:
	// Integration tab
	void setLoadBalancing(bool state);
	void setIntegration(bool state);
	void setMotionSlowDown(bool state);
	void setLock(bool checked);

	void setTimeThreshold(float value);
	void setMaxSteps(uint32 value);

	// User param tab
	void setGloabal1();
	void setGloabal2();
	void setGloabal3();
	void setGloabal4();

	void setUserParam1(float value);
	void setUserParam2(float value);
	void setUserParam3(float value);
	void setUserParam4(float value);

	// BBox tab
	void setEnableBbox(bool state);
	void setAutoBbox(bool state);
	void resetBbox();
	void incBbox();
	void decBbox();
	void setXBbox(double value);
	void setYBbox(double value);
	void setZBbox(double value);

	// LOD Param
	void setSharable(bool state);
	void setAutoLOD(bool state);
	void settings();

	void setMaxViewDist(float value);
	void setLodRatio(float value);

	// Global color tab
	void setGlobalLight(bool state);
	void setEditGlobalColor(bool state);

	// Life mgt param
	void setPresetBehaviour(int index);
	void setModelRemoved(bool state);
	void setPSResource(bool state);
	void setLifeTimeUpdate(bool state);
	void setNoMaxNBSteps(bool state);
	void setAutoDelay(bool state);
	void setAnimType(int index);
	void setDie(int index);
	void setAfterDelay(double value);

private:
	void updatePrecomputedBBoxParams();
	void updateDieOnEventParams();
	void updateLifeMgtPresets();

	bool enabledModifiedFlag;

	void updateModifiedFlag()
	{
		if ((_Node) && (enabledModifiedFlag)) _Node->setModified(true);
	}

	CWorkspaceNode *_Node;

	struct CGlobalColorWrapper : public IPSSchemeWrapperRGBA
	{
		NL3D::CParticleSystem *PS;
		virtual scheme_type *getScheme(void) const;
		virtual void setScheme(scheme_type *s);
	}
	_GlobalColorWrapper;

	Ui::CParticleSystemPage _ui;

}; /* class CParticleSystemPage */

} /* namespace NLQT */

#endif // PARTICLE_SYSTEM_PAGE_H
