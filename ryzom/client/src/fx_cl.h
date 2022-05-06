// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.



#ifndef NL_FX_CL_H
#define NL_FX_CL_H

// Misc
#include "nel/misc/types_nl.h"
// Client
#include "entity_cl.h"
// 3D
#include "nel/3d/u_particle_system_instance.h"


class CFXSheet;

/**
 * <Class description>
 * \author David Fleury, Olivier Cado
 * \author Nevrax France
 * \date 2001, 2003
 */
class CFxCL : public CEntityCL
{
public:
	NLMISC_DECLARE_CLASS(CFxCL);

	/// Constructor
	CFxCL();
	/// Destructor
	virtual ~CFxCL();

	/// Build the entity from a sheet.
	virtual bool build( const CEntitySheet *sheet );

	/// Load a FX object (.ps file) (delete the previous object)
	bool setFx( const std::string &fileName );

protected:

	/// Initialize properties of the entity (according to the class).
	virtual void initProperties() { properties().selectable( false ); }

	/// Update the item position.
	virtual void updateVisualPropertyPos(const NLMISC::TGameCycle &gameCycle, const sint64 &prop, const NLMISC::TGameCycle &pI);

	// Update the position of the entity after the motion.
	//virtual void updatePos(const NLMISC::TTime &time, CEntityCL *target);

	// Method called each frame to manage the entity after the clipping test if the primitive is visible.
	//virtual void updateVisible(const NLMISC::TTime &time, CEntityCL *target);

	/// Draw the selection Box
	virtual void drawBox();

private:

	/// Init
	void init();

	const CFXSheet *_FXSheet;
	bool			_BadBuild;
};


#endif // NL_FX_CL_H

/* End of fx_cl.h */
