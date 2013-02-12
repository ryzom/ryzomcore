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

#include "stdsound.h"
#include "nel/sound/background_sound_manager.h"
#include "nel/sound/background_source.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND
{


CBackgroundSource::CBackgroundSource(CBackgroundSound *backgroundSound, bool spawn, TSpawnEndCallback cb, void *cbUserParam, NL3D::CCluster *cluster, CGroupController *groupController)
:	CSourceCommon(backgroundSound, spawn, cb, cbUserParam, cluster, groupController)
{
	_BackgroundSound = backgroundSound;
}

CBackgroundSource::~CBackgroundSource()
{
	if (_Playing)
		stop();
}

TSoundId CBackgroundSource::getSound()
{
	return NULL;
}

void CBackgroundSource::setGain( float gain )
{
	CSourceCommon::setGain(gain);

	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != 0)
		{
			first->Source->setGain(first->Source->getSound()->getGain() * gain);
			first->Source->setRelativeGain(_Gain);
		}
	}
}
void CBackgroundSource::setRelativeGain( float gain )
{
	CSourceCommon::setRelativeGain(gain);

	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != 0)
			first->Source->setRelativeGain(_Gain);
	}
}

void CBackgroundSource::setPos( const NLMISC::CVector& pos )
{
	CSourceCommon::setPos(pos);

	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != 0)
			first->Source->setPos(pos);
	}
}

void CBackgroundSource::setVelocity( const NLMISC::CVector& vel )
{
	CSourceCommon::setVelocity(vel);

	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != 0)
			first->Source->setVelocity(vel);
	}
}
void CBackgroundSource::setDirection( const NLMISC::CVector& dir )
{
	CSourceCommon::setDirection(dir);

	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		if (first->Source != 0)
			first->Source->setDirection(dir);
	}
}



void CBackgroundSource::play()
{
	if (_Playing)
		stop();

	CAudioMixerUser *mixer = CAudioMixerUser::instance();

	const vector<CBackgroundSound::TSoundInfo> &sounds = _BackgroundSound->getSounds();
	vector<CBackgroundSound::TSoundInfo>::const_iterator first(sounds.begin()), last(sounds.end());

	for (; first != last; ++first)
	{
		TSubSource subSource;
		subSource.Source = mixer->createSource(first->SoundName, false, 0, 0, _Cluster, NULL, _GroupController);
		if (subSource.Source != NULL)
			subSource.Source->setPriority(_Priority);
		subSource.Filter = first->Filter;
		subSource.Status = SUB_STATUS_STOP;
		_Sources.push_back(subSource);
	}

	updateFilterValues(mixer->getBackgroundSoundManager()->getFilterValues());

	CSourceCommon::play();
}

void CBackgroundSource::stop()
{
	if (_Playing)
	{
		while (!_Sources.empty())
		{
			TSubSource &subSource = _Sources.back();
			if (subSource.Source != NULL)
			{
				delete subSource.Source;
			}
			_Sources.pop_back();
		}
	}

	CSourceCommon::stop();

	CAudioMixerUser::instance()->unregisterUpdate(this);
}

void CBackgroundSource::updateFilterValues(const float *filterValues)
{
	bool needUpdate = false;
	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		TSubSource &ss = *first;
		if (ss.Source != 0)
		{
			float gain = 1.0f;
			for (uint i=0; i<UAudioMixer::TBackgroundFlags::NB_BACKGROUND_FLAGS; ++i)
			{
				if (ss.Filter.Flags[i])
				{
					// this filter is used
					gain *= filterValues[i];
				}
			}

			if (gain == 0 && ss.Status != SUB_STATUS_STOP)
			{
				// need to completely stop the sound
				if (ss.Source->isPlaying())
					ss.Source->stop();
				ss.Status = SUB_STATUS_STOP;
			}
			else if (gain > 0 && ss.Status != SUB_STATUS_PLAY)
			{
				// need to restard the sound
//				ss.Source->setRelativeGain(gain * _Gain);
				ss.Source->setGain(ss.Source->getSound()->getGain() * gain);
				ss.Source->setRelativeGain(_Gain);
				ss.Source->setPitch(ss.Source->getSound()->getPitch() * _Pitch);
				ss.Source->setPos(_Position);
				ss.Source->play();
				// some sub sound can be too far from the listener,
				// we must handle this in order to start them when the listener
				// is closer
				ss.Status = ss.Source->isPlaying() ? SUB_STATUS_PLAY : SUB_STATUS_PLAY_FAIL;

				needUpdate |= (ss.Status == SUB_STATUS_PLAY_FAIL);
			}
			else //if (ss.Status == SUB_STATUS_PLAY)
			{
				// just update the gain
				ss.Source->setGain(ss.Source->getSound()->getGain() * gain);
				ss.Source->setRelativeGain(_Gain);
			}
		}
	}

	// if some some sub sound fail to play...
	if (needUpdate)
		CAudioMixerUser::instance()->registerUpdate(this);

}

void CBackgroundSource::onUpdate()
{
	bool needUpdate = false;
	// Some sub source are distance clipped, so retry to start them.
	std::vector<TSubSource>::iterator first(_Sources.begin()), last(_Sources.end());
	for (; first != last; ++first)
	{
		TSubSource &ss = *first;
		if (ss.Status == SUB_STATUS_PLAY_FAIL)
		{
			ss.Source->play();
			// some sub sound can be too far from the listener,
			// we must handle this in order to start them when the listener
			// is closer
			ss.Status = ss.Source->isPlaying() ? SUB_STATUS_PLAY : SUB_STATUS_PLAY_FAIL;

			needUpdate |= (ss.Status == SUB_STATUS_PLAY_FAIL);
		}
	}

	// no more update needed ?
	if (!needUpdate)
		CAudioMixerUser::instance()->unregisterUpdate(this);
}

} // NLSOUND
