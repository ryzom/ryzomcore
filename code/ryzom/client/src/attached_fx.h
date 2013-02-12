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

#ifndef CL_ATTACHED_FX_H
#define CL_ATTACHED_FX_H

#include "fx_manager.h"
#include "animation_fx.h"
#include "game_share/entity_types.h"
#include "client_sheets/fx_stick_mode.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/vector.h"
#include "nel/3d/animation_time.h"

namespace NL3D
{
	class UParticleSystemInstance;
}

class CCharacterCL;

// max number of anim a fx animation can span over.
const uint   MAX_FX_ANIM_COUNT = 2;


/** An fx linked to a character using a stick mode (replacement to former class CCharacterCL::CAnimFX)
  * \TODO nico The template sheet is named CAnimationFX. Maybe CAttachedFXTemplate would be a better name...
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2005
  */
class CAttachedFX : public NLMISC::CRefCount
{
public:
	friend class CCharacterCL;
	// FX targeter (could be the caster for a ray, or for range impact...)
	class CTargeterInfo
	{
	public:
		uint8			Slot;
		CFXStickMode	StickMode;
		NLMISC::CVector	StickOffset;
		NLMISC::CVector DefaultPos;  // Default pos used if slot isn't available
		CTargeterInfo()
		{
			Slot = CLFECOMMON::INVALID_SLOT;
			StickOffset = NLMISC::CVector::Null;
			DefaultPos = NLMISC::CVector::Null;
		}
	};
	// Description of fx to build
	class CBuildInfo
	{
	public:
		const CAnimationFX*		Sheet;
		const CFXStickMode*		StickMode;			// NULL to use default stick mode of the sheet
		NLMISC::CVector			StickOffset;
		const NLMISC::CMatrix*	StaticMatrix;		// Useful if stick mode is "StaticMatrix"
		uint					MaxNumAnimCount;	// Number of frame on which the fx can  overlap when it is being shutdown
		float					TimeOut;
		double					StartTime;
		float					DelayBeforeStart;
	public:
		CBuildInfo()
		{
			Sheet = NULL;
			StickMode = NULL;
			StickOffset = NLMISC::CVector::Null;
			StaticMatrix = NULL;
			MaxNumAnimCount = 0;
			TimeOut = FX_MANAGER_DEFAULT_TIMEOUT;
			StartTime = 0.0;
			DelayBeforeStart = 0.f;
		}
	};
	CAttachedFX();
	~CAttachedFX();
	// Creation from a sheet (sheet used in buildInfo)
	void create(CCharacterCL					&parent,
				const CBuildInfo				&buildInfo,
				const CTargeterInfo				&targeterInfo
			   );
	// Creation from an externally created FX (sheet is still used to specify track)
	void create(CCharacterCL					&parent,
				NL3D::UParticleSystemInstance   instance,
				const CBuildInfo				&buildInfo,
				const CTargeterInfo				&targeterInfo
			   );
	// reset that object
	void clear();

	/* old params
	  CAnimFX &dest,
	  NL3D::UParticleSystemInstance instance,
	  const CAnimationFX *sheet,
	  const CFXStickMode *stickMode,
	  float timeOut = FX_MANAGER_DEFAULT_TIMEOUT,
	  const NLMISC::CVector &stickOffset = NLMISC::CVector::Null,
	  uint maxNumAnimCount = 0,
	  uint8 targeterSlot = CLFECOMMON::INVALID_SLOT
	*/


	// update position
	void update(CCharacterCL &parent, const NLMISC::CMatrix &alignMatrix);
	// a smart pointer to that object
	typedef NLMISC::CSmartPtr<CAttachedFX> TSmartPtr;
public:
	NL3D::UParticleSystemInstance	FX;
	const CAnimationFX				*AniFX;

	NL3D::TGlobalAnimationTime		SpawnTime;			// used for track evaluation
	NLMISC::CVector					SpawnPos;			// spawn pos is in world in 'spawn' mode, or local for 'sticked' and 'follow' modes
	double							TimeOutDate;		// timeOut date for FXs to be removed. If the fx is in the active list, then the date is relative and is added to current date when the fx is shutdown
	uint							MaxAnimCount;		// max number of anim changes during which the fx is still visible while being shutdown (0 if no limit)
	//
	CFXStickMode::TStickMode		StickMode;			// The stick mode for that fx. If NULL, the default stickmode of the .animation_fx sheet is used instead
	uint8							UserBoneID;			// filled only when fx is sticked
	//
	CTargeterInfo					TargeterInfo;
	uint8							TargeterUserBoneID;	// filled at setup
	// eval pos from slot & stick mode
	void evalTargeterStickPos(NLMISC::CVector &dest) const;
};





#endif
