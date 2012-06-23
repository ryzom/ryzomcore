// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_PARTICLE_SYSTEM_MANAGER_H
#define NL_PARTICLE_SYSTEM_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/plane.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/particle_system_process.h"


#include <list>
#include <vector>

namespace NLMISC
{
	class CVector;
}

namespace NL3D {


class CParticleSystemModel;

/**
 * This class list all the particle systems that have resources allocated at a given time, so that
 * we can remove the resource of those who are too far from the viewer.
 * This has been added because objects that are in a cluster that is not
 * visible are not parsed during traversals, so their resources couldn't just be released
 * Only a few systems are parsed each time (because they can be numerous).
 * Call to that class should be made by CScene, so you don't need to use it
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2002
 */
class CParticleSystemManager
{
public:
	enum { NumProcessToRefresh  = 3 }; // the number of systems that are refreshed at each call

	/// default ctor
	CParticleSystemManager();

	// dtor
	~CParticleSystemManager();

	// release memory
	static void release();

	/// call this to refresh systems. (check those whose data should be released)
	void	refreshModels(const std::vector<NLMISC::CPlane>	&worldFrustumPyramid,  const NLMISC::CVector &viewerPos);

	/// perform animation on systems that should be animated even if not parsed (temporary spells for example)
	void	processAnimate(TAnimationTime deltaT);

	// stop sound for all particle systems in this manager
	void    stopSound();

	// reactivate sound for all particle systems in this manager
	void    reactivateSound();

	// stop sound for all particle systems in all managers
	static void    stopSoundForAllManagers();

	// reactivate sound for all particle systems in all managers
	static void    reactivateSoundForAllManagers();

private:
	friend class CParticleSystemModel;

	// info about a ps that is always animated
	class CAlwaysAnimatedPS
	{
	public:
		CParticleSystemModel *Model;
		CMatrix				 OldAncestorMatOrRelPos;	// last matrix of ancestor skeleton or relative matrix of ps to its ancestor (see flag below)
		bool				 IsRelMatrix;				// gives usage of the field OldAncestorMatOrRelPos
		bool				 HasAncestorSkeleton;		// has the system an ancestor skeleton ?
	public:
		// ctor
		CAlwaysAnimatedPS()
		{
			NL_PS_FUNC(CAlwaysAnimatedPS_CAlwaysAnimatedPS)
			Model = NULL;
			IsRelMatrix = false;
			HasAncestorSkeleton = false;
		}
	};

	typedef std::list<CParticleSystemModel *>   TModelList;
	typedef std::list<CAlwaysAnimatedPS>		TAlwaysAnimatedModelList;
	typedef std::list<CParticleSystemManager *> TManagerList;

	static TManagerList *ManagerList;

	struct TModelHandle
	{
		TModelList::iterator Iter;
		bool				 Valid;
		TModelHandle() : Valid(false) {}
	};

	struct TAlwaysAnimatedModelHandle
	{
		TAlwaysAnimatedModelList::iterator Iter;
		bool							   Valid;
		TAlwaysAnimatedModelHandle() : Valid(false) {}
	};


	/** Should be called when a new p.s. model has resources attached to it.
	  * \see removeSystemModel
	  */
	TModelHandle addSystemModel(CParticleSystemModel *);

	/// Should called when a p.s model has resources detached from it.
	void			removeSystemModel(TModelHandle &handle);

	/// Should be called to attach a system that must always be animated
	TAlwaysAnimatedModelHandle addPermanentlyAnimatedSystem(CParticleSystemModel *);

	/// Remove a permanenlty animated system
	void			removePermanentlyAnimatedSystem(TAlwaysAnimatedModelHandle &handle);

private:

	TModelList::iterator	_CurrListIterator; /// the current element being processed
	TModelList				_ModelList;
	TAlwaysAnimatedModelList _PermanentlyAnimatedModelList;
	uint					_NumModels;
	// access to list of currently instanciated managers
	static TManagerList     &getManagerList();
	// link into the global manager list for that manager
	TManagerList::iterator	_GlobalListHandle;
};


} // NL3D


#endif // NL_PARTICLE_SYSTEM_MANAGER_H

/* End of particle_system_manager.h */
