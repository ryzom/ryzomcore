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



#ifndef NL_ENVIRONMENTAL_EFFECT_H
#define NL_ENVIRONMENTAL_EFFECT_H

#include "nel/misc/vector_2f.h"
#include "game_share/base_types.h"
#include "egs_mirror.h"

#include "phrase_manager/simple_entity_manager.h"

/**
 * Environmental Effect
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CEnvironmentalEffect
{
	NL_INSTANCE_COUNTER_DECL(CEnvironmentalEffect);
public:

	inline CEnvironmentalEffect() { _TimeToLive = 0; _Radius = 0.0f; }

	/// Virtual destructor (required for proper NL_INSTANCE_COUNTER_DECL count of derived classes)
	virtual ~CEnvironmentalEffect() {}

	/// Init
	inline void			init( const NLMISC::CVector& pos, float radius, NLMISC::TGameCycle lifetime=DefaultLifetime ) { _Pos = pos; _Radius = radius; _TimeToLive = lifetime; }

	/**
	 * Spawn the effect as an entity in mirror. Return false in case of failure.
	 * \param sheet sheet id of the effect (for visual FX)
	 */
	virtual bool		spawn( const NLMISC::CSheetId &sheet ) = 0;

	/// Tick update. Return false if the source's life has ended.
	virtual bool		update();

	// Accessors
	inline const TDataSetRow&		rowId() const { return _DataSetRow; }
	inline const NLMISC::CVector2f&	pos() const { return _Pos; }
	inline const float&				radius() const { return _Radius; }
	inline const NLMISC::TGameCycle&timeToLive() const { return _TimeToLive; }
	
	static NLMISC::TGameCycle	DefaultLifetime;

protected:
	/// Despawn the toxic cloud in mirror
	void				despawn();

protected:
	/// Entity representing the effect, valid after spawn()
	TDataSetRow			_DataSetRow;

	/// Position (coordinates in meters)
	NLMISC::CVector2f	_Pos;

	/// Radius
	float				_Radius;

	/// Toxic cloud remaining Time To Live (in ticks, decremented by every update)
	NLMISC::TGameCycle	_TimeToLive;
};


/**
 * Environmental effect manager class
 */
class CEnvironmentalEffectManager : public CSimpleEntityManager<CEnvironmentalEffect>
{
	NL_INSTANCE_COUNTER_DECL(CEnvironmentalEffectManager);
public:

	/// Singleton access
	static CEnvironmentalEffectManager *getInstance();

	/// Release
	static void release();
};




#endif // NL_ENVIRONMENTAL_EFFECT_H

/* End of environmental_effect.h */
