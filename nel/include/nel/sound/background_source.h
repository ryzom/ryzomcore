// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_BACKGROUND_SOURCE_H
#define NL_BACKGROUND_SOURCE_H

#include "nel/misc/types_nl.h"
//#include "nel/sound/u_source.h"
#include "nel/sound/source_common.h"
#include "nel/sound/background_sound.h"


namespace NLSOUND {

class CBackgroundSound;

/** Implemetation class for background source.
 *	Complex source are source that use a CPatternSound object.
 * \author Boris Boucher.
 * \author Nevrax
 */
class CBackgroundSource : public CSourceCommon , public CAudioMixerUser::IMixerUpdate
{
public:
	/// Constructor
	CBackgroundSource	(CBackgroundSound *backgroundSound=NULL, bool spawn=false, TSpawnEndCallback cb=0, void *cbUserParam = 0, NL3D::CCluster *cluster = 0, CGroupController *groupController = NULL);
	/// Destructor
	~CBackgroundSource	();

	/// Return the sound binded to the source (or NULL if there is no sound)
	virtual TSoundId				getSound();

	virtual void					play();
	/// Stop playing
	virtual void					stop();

	TSOURCE_TYPE					getType() const								{return SOURCE_BACKGROUND;}

	void							setGain( float gain );
	void							setRelativeGain( float gain );

	void							setPos( const NLMISC::CVector& pos );
	void							setVelocity( const NLMISC::CVector& vel );
	void							setDirection( const NLMISC::CVector& dir );

	void							updateFilterValues(const float *filterValues);


private:

	/// Mixer update
	void onUpdate();

	/// Sub source possible status.
	enum TSubSourceStatus
	{
		/// The sub source is playing.
		SUB_STATUS_PLAY,
		/// The sub source is stopped : it is masked by environnemt status.
		SUB_STATUS_STOP,
		/// The sub source have fail to play, either because of distance or no available track...
		SUB_STATUS_PLAY_FAIL
	};

	/// Sub source info.
	struct TSubSource
	{
		/// Sub source instance.
		USource				*Source;
		/// Sub source status.
		TSubSourceStatus	Status;
		/// Sub source filter.
		UAudioMixer::TBackgroundFlags	Filter;
	};

	/// The sound static data.
	CBackgroundSound			*_BackgroundSound;

	/// The sub sources container.
	std::vector<TSubSource>		_Sources;
};



} // NLSOOUND

#endif // NL_BACKGROUND_SOURCE_H
