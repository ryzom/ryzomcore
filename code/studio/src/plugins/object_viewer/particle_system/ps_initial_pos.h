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

#ifndef NL_PS_INITIAL_POS
#define NL_PS_INITIAL_POS

#include <nel/misc/types_nl.h>
//#include <nel/misc/common.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>

namespace NL3D
{
class CParticleSystem;
class CPSLocated;
class CPSLocatedBindable;
struct IPSMover;
}

namespace NLQT
{
/**
 @class CPSInitialPos
 This class helps to copy the position of initial instances in a particle
 system. This enable a system to run, and have its parameter modified.
 When the user press stop, he will find the system at t = 0, with the new parameters
*/
class CPSInitialPos
{
public:
	CPSInitialPos() : _PS(NULL) {}

	/// construct this by copying the datas of the system
	void copySystemInitialPos(NL3D::CParticleSystem *ps);

	/// reinitialize the system with its initial instances positions
	/// Works only once per copySystemInitialPos() call
	void restoreSystem();

	/// send back true when bbox display is enabled
	bool isBBoxDisplayEnabled();

	/// update data when a located in a particle system has been removed
	void removeLocated(NL3D::CPSLocated *loc);

	/// update data when a located bindable in a particle system has been removed
	void removeLocatedBindable(NL3D::CPSLocatedBindable *lb);

	/// initial position and speed of a located instance in a particle system
	struct CInitPSInstanceInfo
	{
		uint32 Index;
		NL3D::CPSLocated *Loc;
		NLMISC::CVector Speed;
		NLMISC::CVector Pos;
	};

	/// rotation and scale of an element
	struct CRotScaleInfo
	{
		uint32 Index;
		NL3D::CPSLocated *Loc;
		NL3D::CPSLocatedBindable *LB;
		NL3D::IPSMover *Psm;
		NLMISC::CMatrix Rot;
		NLMISC::CVector Scale;
	};
	NL3D::CParticleSystem *getPS()
	{
		return _PS;
	}
	const NL3D::CParticleSystem *getPS() const
	{
		return _PS;
	}
	bool isStateMemorized() const
	{
		return _PS != NULL;
	}

private:
	typedef std::vector<CInitPSInstanceInfo> TInitInfoVect;
	typedef std::vector<CRotScaleInfo> TRotScaleInfoVect;
	typedef std::vector< std::pair<NL3D::CPSLocated *, uint32> > TInitialLocatedSizeVect;
	TInitInfoVect _InitInfoVect;
	TRotScaleInfoVect _RotScaleInfoVect;

	/// initial number of instances for each located
	TInitialLocatedSizeVect  _InitialSizeVect;
	NL3D::CParticleSystem *_PS;

	/// reset all initial infos
	void reset();
};

} /* namespace NLQT */

#endif