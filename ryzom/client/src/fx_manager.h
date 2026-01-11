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



#ifndef CL_FX_MANAGER_H
#define CL_FX_MANAGER_H


/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
// 3D
#include "nel/3d/u_particle_system_instance.h"
// Std
#include <map>
#include <string>


///////////
// CLASS //
///////////
namespace NL3D
{
	class UParticleSystemInstance;
}

// when a fx has a timeout, it is
const float FX_MANAGER_DEFAULT_TIMEOUT = 12.f;

/**
 * Class to manage FXs.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2003
 */
class CFXManager
{
public:
	/// Constructor
	CFXManager();
	/// Destructor
	~CFXManager();

	// create and managed an fx (e.g destroy its model when it is finished)
	NL3D::UParticleSystemInstance instantFX(const std::string &fxName, float timeOut = FX_MANAGER_DEFAULT_TIMEOUT);
	// Add an fx that will be created some time later
	void deferFX(const std::string &fxName, const NLMISC::CMatrix &matrix, float delayInSeconds, float timeOut = FX_MANAGER_DEFAULT_TIMEOUT);
	// add an externally created fx to be managed
	void addFX(NL3D::UParticleSystemInstance fx, float timeOut = FX_MANAGER_DEFAULT_TIMEOUT, bool testNoMoreParticles = false);
	void fx2remove(NL3D::UParticleSystemInstance fx, float timeOut = FX_MANAGER_DEFAULT_TIMEOUT);
	void update();
	// instantly removes all fxs that need to
	void reset();
	// get the number of fx that are currently to be removed
	uint getNumFXtoRemove() const { return _NumFXToRemove; }
protected:
	class CFX2Remove
	{
	public:
		NL3D::UParticleSystemInstance Instance;
		float						  TimeOut;
		bool						  TestNoMoreParticles;
	public:
		CFX2Remove(NL3D::UParticleSystemInstance instance=NL3D::UParticleSystemInstance(), float timeOut = 0.f, bool testNoMoreParticles = false)
		{
			Instance = instance;
			TimeOut = timeOut;
			TestNoMoreParticles = testNoMoreParticles;
		}
	};
	// List of FXs to remove as soon as possible.
	std::list<CFX2Remove>	_FX2RemoveList;
	uint					_NumFXToRemove;


	struct CDeferredFX
	{
		NLMISC::CMatrix Matrix;
		std::string		FXName;
		float			TimeOut;
	};

	CHashMultiMap<uint64, CDeferredFX> _DeferredFXByDate;
};


////////////
// EXTERN //
////////////
// FX manager.
extern CFXManager	FXMngr;


#endif // CL_FX_MANAGER_H

/* End of fx_manager.h */
