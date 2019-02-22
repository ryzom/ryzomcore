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

#include "nel/sound/complex_source.h"
#include "nel/sound/complex_sound.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND
{

CComplexSource::CComplexSource	(CComplexSound *soundPattern, bool spawn, TSpawnEndCallback cb, void *cbUserParam, NL3D::CCluster *cluster, CGroupController *groupController)
:	CSourceCommon(soundPattern, spawn, cb, cbUserParam, cluster, groupController),
	_Source1(NULL),
	_Source2(NULL),
	_Muted(false)
{
	nlassert(soundPattern->getSoundType() == CSound::SOUND_COMPLEX);
	_PatternSound = static_cast<CComplexSound*>(soundPattern);

	// read original parameters
	_Gain = soundPattern->getGain();
	_Pitch = soundPattern->getPitch();
	_Looping = soundPattern->getLooping();
	_Priority = soundPattern->getPriority();
	_TickPerSecond = soundPattern->getTicksPerSecond();
}

CComplexSource::~CComplexSource()
{
//	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	// security
	CAudioMixerUser::instance()->unregisterUpdate(this);
	CAudioMixerUser::instance()->removeEvents(this);

	std::vector<USource	*>::iterator first(_AllSources.begin()), last(_AllSources.end());
	for (; first != last; ++first)
	{
		//mixer->removeSource(*first);
		delete *first;
	}
}


TSoundId CComplexSource::getSound()
{
	return _PatternSound;
}
/*
void CComplexSource::setPriority( TSoundPriority pr, bool redispatch)
{
}

void CComplexSource::	setLooping( bool l )
{
}
bool CComplexSource::getLooping() const
{
	return false;
}
*/

bool CComplexSource::isPlaying()
{
	return _Playing;
}

void CComplexSource::play()
{
	if (_Gain == 0)
	{
		_Muted = true;
	}
	else
	{
		playStuf();
	}
	CSourceCommon::play();
}

void CComplexSource::playStuf()
{
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	NLMISC::TTime now = NLMISC::CTime::getLocalTime();

	switch (_PatternSound->getPatternMode())
	{
	case CComplexSound::MODE_CHAINED:
		{
			_SoundSeqIndex = 0;
			_FadeFactor = 1.0f;
			const vector<uint32>	&soundSeq = _PatternSound->getSoundSeq();
			if (!soundSeq.empty())
			{
				CSound *sound = mixer->getSoundId(_PatternSound->getSound(soundSeq[_SoundSeqIndex++]));

				if (sound == 0)
					return;

				if (_PatternSound->doFadeIn())
					_FadeLength = min(uint32(_PatternSound->getFadeLength()/_TickPerSecond), sound->getDuration() /2);
				else
					_FadeLength = 0;

				_Source2 = mixer->createSource(sound, false, 0, 0, _Cluster, NULL, _GroupController);
				if (_Source2 == NULL)
					return;
				_Source2->setPriority(_Priority);
				_Source2->setRelativeGain(0);
				_Source2->setPos(_Position);
				_Source2->setPitch(_Source2->getSound()->getPitch() * _Pitch);
				_Source2->play();
				_StartTime2 = now;

				// register for fade in.
				mixer->registerUpdate(this);
			}
		}
		break;
	case CComplexSound::MODE_SPARSE:
		{
			// use Source1, sound sequence, delay sequence and event.
			_SoundSeqIndex = 0;
			_DelaySeqIndex = 0;

			const std::vector<uint32> &delaySeq = _PatternSound->getDelaySeq();

			if (!delaySeq.empty() && delaySeq[_DelaySeqIndex] != 0)
			{
				_LastSparseEvent = false;
				// begin with a delay
				mixer->addEvent(this, uint64(now + delaySeq[_DelaySeqIndex++]/_TickPerSecond));
			}
			else
			{
				if (!delaySeq.empty())
					_DelaySeqIndex = 1;
				const vector<uint32>	&soundSeq = _PatternSound->getSoundSeq();
				if (!soundSeq.empty())
				{
					CSound *sound = mixer->getSoundId(_PatternSound->getSound(soundSeq[_SoundSeqIndex++]));

					_Source1 = mixer->createSource(sound, false, 0, 0, _Cluster, NULL, _GroupController);
					if (_Source1 == NULL)
						return;
					_Source1->setPriority(_Priority);
					_Source1->setRelativeGain(_Gain*_Gain*_Gain);
					_Source1->setPos(_Position);
					_Source1->setPitch(_Source1->getSound()->getPitch() * _Pitch);
					_Source1->play();
					_StartTime1 = now;

					// register event for next sound.
					const std::vector<uint32> &delaySeq = _PatternSound->getDelaySeq();
					if (!delaySeq.empty() && _DelaySeqIndex < delaySeq.size())
					{
						// event for next sound.
						mixer->addEvent(this, uint64(now + sound->getDuration() + delaySeq[_DelaySeqIndex++]/_TickPerSecond));
						if (_DelaySeqIndex >= delaySeq.size() && !_Looping)
							_LastSparseEvent = true;
						else
							_LastSparseEvent = false;
					}
					else
					{
						_LastSparseEvent = true;
						// event for stop
						mixer->addEvent(this, now + sound->getDuration());
					}
				}
			}

		}
		break;
	case CComplexSound::MODE_ALL_IN_ONE:
		{
			// just spanw all the listed source.
			const std::vector<NLMISC::CSheetId> &sounds = _PatternSound->getSounds();

			std::vector<NLMISC::CSheetId>::const_iterator first(sounds.begin()), last(sounds.end());

			if (_AllSources.empty())
			{
				// create the sources
				for (; first != last; ++first)
				{
					CSound *sound = mixer->getSoundId(*first);
					if (sound != NULL)
					{
						USource *source = mixer->createSource(sound, false, 0, 0, _Cluster, NULL, _GroupController);
						if (source != NULL)
						{
							source->setPriority(_Priority);
							source->setRelativeGain(_Gain*_Gain*_Gain);
							source->setPos(_Position);
							source->setPitch(source->getSound()->getPitch() * _Pitch);
							source->play();

							_AllSources.push_back(source);
						}
					}
				}
			}
			else
			{
				// just replay the existing source.
				std::vector<USource	*>::iterator first(_AllSources.begin()), last(_AllSources.end());

				for (; first != last; ++first)
				{
					(*first)->setRelativeGain(_Gain*_Gain*_Gain);
					(*first)->setPos(_Position);
					(*first)->setPitch((*first)->getSound()->getPitch() * _Pitch);
					(*first)->play();
				}
			}

			if (!_Looping)
			{
				// event to stop the sound
				mixer->addEvent(this, NLMISC::CTime::getLocalTime() + _PatternSound->getDuration());
			}
		}
		break;
	default:
		nldebug("Unknow pattern mode. Can't play.");
	}

	_Muted = false;
}
void CComplexSource::stop()
{
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	if (_Source1)
	{
//		_Source1->stop();
//		mixer->removeSource(_Source1);
		delete _Source1;

		_Source1 = NULL;
	}
	if (_Source2)
	{
//		_Source2->stop();
//		mixer->removeSource(_Source2);
		delete _Source2;

		_Source2 = NULL;
	}

	std::vector<USource	*>::iterator first(_AllSources.begin()), last(_AllSources.end());
	for (; first != last; ++first)
	{
		if ((*first)->isPlaying())
			(*first)->stop();
		delete *first;
	}
	_AllSources.clear();

	switch (_PatternSound->getPatternMode())
	{
	case CComplexSound::MODE_CHAINED:
		mixer->unregisterUpdate(this);
		mixer->removeEvents(this);
		break;
	case CComplexSound::MODE_SPARSE:
	case CComplexSound::MODE_ALL_IN_ONE:
		mixer->removeEvents(this);
		break;
	case CComplexSound::MODE_UNDEFINED:
	default:
		break;
	}

	CSourceCommon::stop();
}

/*void CComplexSource::unregisterSpawnCallBack()
{
}
*/
void CComplexSource::setPos( const NLMISC::CVector& pos )
{
	CSourceCommon::setPos(pos);

	if (_Source1 != NULL)
		_Source1->setPos(pos);
	if (_Source2 != NULL)
		_Source2->setPos(pos);

	std::vector<USource	*>::iterator first(_AllSources.begin()), last(_AllSources.end());
	for (; first != last; ++first)
	{
		(*first)->setPos(pos);
	}
}

void CComplexSource::setVelocity( const NLMISC::CVector& vel )
{
	CSourceCommon::setVelocity(vel);

	if (_Source1 != NULL)
		_Source1->setVelocity(vel);
	if (_Source2 != NULL)
		_Source2->setVelocity(vel);

	std::vector<USource	*>::iterator first(_AllSources.begin()), last(_AllSources.end());
	for (; first != last; ++first)
	{
		(*first)->setVelocity(vel);
	}
}
/*void CComplexSource::getVelocity( NLMISC::CVector& vel ) const
{
}
*/
void CComplexSource::setDirection( const NLMISC::CVector& dir )
{
	CSourceCommon::setDirection(dir);

	if (_Source1 != NULL)
		_Source1->setDirection(dir);
	if (_Source2 != NULL)
		_Source2->setDirection(dir);

	std::vector<USource	*>::iterator first(_AllSources.begin()), last(_AllSources.end());
	for (; first != last; ++first)
	{
		(*first)->setDirection(dir);
	}
}
/*
void CComplexSource::getDirection( NLMISC::CVector& dir ) const
{
}
*/
void CComplexSource::setGain( float gain )
{
	CSourceCommon::setGain(gain);

	// update the gain of the played source.
	if (_PatternSound->getPatternMode() == CComplexSound::MODE_CHAINED)
	{
		// set sub source volume with fade value.
		if (_Source1 != NULL)
			_Source1->setRelativeGain((1.0f - _FadeFactor) * _Gain*_Gain*_Gain);
		if (_Source2 != NULL)
			_Source2->setRelativeGain(_FadeFactor * _Gain*_Gain*_Gain);

	}
	else
	{
		if (_Source1 != NULL)
			_Source1->setRelativeGain(_Gain*_Gain*_Gain);
		if (_Source2 != NULL)
			_Source2->setRelativeGain(_Gain*_Gain*_Gain);
	}

	std::vector<USource	*>::iterator first(_AllSources.begin()), last(_AllSources.end());
	for (; first != last; ++first)
	{
		(*first)->setGain(_Gain);
	}

	if (_Muted && _Playing)
		playStuf();
}

void CComplexSource::setRelativeGain( float gain )
{
	CSourceCommon::setRelativeGain(gain);

	// update the gain of the played source.
	if (_PatternSound->getPatternMode() == CComplexSound::MODE_CHAINED)
	{
		// set sub source volume with fade value.
		if (_Source1 != NULL)
			_Source1->setRelativeGain((1.0f - _FadeFactor) * _Gain*_Gain*_Gain);
		if (_Source2 != NULL)
			_Source2->setRelativeGain(_FadeFactor * _Gain*_Gain*_Gain);

	}
	else
	{
		if (_Source1 != NULL)
			_Source1->setRelativeGain(_Gain*_Gain*_Gain);
		if (_Source2 != NULL)
			_Source2->setRelativeGain(_Gain*_Gain*_Gain);
	}

	std::vector<USource	*>::iterator first(_AllSources.begin()), last(_AllSources.end());
	for (; first != last; ++first)
	{
		(*first)->setRelativeGain(_Gain);
	}

	if (_Muted && _Playing)
		playStuf();
}

/*
void CComplexSource::setPitch( float pitch )
{
}
float CComplexSource::getPitch() const
{
	return 0;
}
*/
/*
void CComplexSource::setSourceRelativeMode( bool mode )
{
}
bool CComplexSource::getSourceRelativeMode() const
{
	return false;
}
*/

uint32 CComplexSource::getTime()
{
	// evaluate the elapsed time.
	if (!_Playing || _PlayStart == 0)	// not started ?
		return 0;

	TTime now = NLMISC::CTime::getLocalTime();

	TTime delta = now - _PlayStart;

	return uint32(delta);
}


/// Mixer update implementation.
void CComplexSource::onUpdate()
{
	// do the cross fade :
	//	- lower sound1, louder sound2,
	//	- when max reach, stop the update, swap the sound, delete sound1 and set event for next fade.

	// can only occur for chained mode.
	nlassert(_PatternSound->getPatternMode() == CComplexSound::MODE_CHAINED);

	CAudioMixerUser *mixer = CAudioMixerUser::instance();

	// compute xfade factor.
	TTime now = NLMISC::CTime::getLocalTime();
	if (_FadeLength > 0)
	{
		_FadeFactor = float((double(now) - double(_StartTime2)) / double(_FadeLength)) ;
//		_FadeFactor = (_FadeFactor*_FadeFactor);
	}
	else
		_FadeFactor = 1.0f;

//	nldebug("Fade factor = %f", _FadeFactor);
	if (_FadeFactor >= 1.0)
	{
		_FadeFactor = 1.0f;
		// fade end !
		if (_Source1)
		{
//			_Source1->stop();
//			mixer->removeSource(_Source1);
			delete _Source1;
			_Source1 = NULL;
		}
		if (_Source2)
		{
			// set max volume
			_Source2->setRelativeGain(1.0f * _Gain);
			// 'swap' the source
			_Source1 = _Source2;
			_FadeFactor = 0.0f;
			_StartTime1 = _StartTime2;
			_Source2 = NULL;
			// if there is a next sound available, program an event for the next xfade.
			CSound	*sound2 = NULL;
//			_SoundSeqIndex++;
			const vector<uint32>	&soundSeq = _PatternSound->getSoundSeq();
			if (_SoundSeqIndex < soundSeq.size())
			{
				sound2 = mixer->getSoundId(_PatternSound->getSound(soundSeq[_SoundSeqIndex++]));
			}
			else if (_Looping)
			{
				// restart the sound sequence
				_SoundSeqIndex = 0;
				sound2 = mixer->getSoundId(_PatternSound->getSound(soundSeq[_SoundSeqIndex++]));
			}


			if (sound2 != NULL)
			{
				//nldebug("CS : Chaining to sound %s", CStringMapper::unmap(sound2->getName()).c_str());
				CAudioMixerUser	*mixer = CAudioMixerUser::instance();

				// determine the XFade length (if next sound is too short.
				_FadeLength = minof<uint32>(uint32(_PatternSound->getFadeLength()/_TickPerSecond), (sound2->getDuration()) / 2, (_Source1->getSound()->getDuration())/2);
				_Source2 = mixer->createSource(sound2, false, 0, 0, _Cluster, NULL, _GroupController);
				if (_Source2)
				{
					_Source2->setPriority(_Priority);
					// there is a next sound, add event for xfade.
					//nldebug("Seting event for sound %s in %u millisec (XFade = %u).", CStringMapper::unmap(_Source1->getSound()->getName()).c_str(), _Source1->getSound()->getDuration()-_FadeLength, _FadeLength);
					mixer->addEvent(this, _StartTime1 + _Source1->getSound()->getDuration() - _FadeLength);
				}
			}
			else
			{
				// no sound after, just set an event at end of current sound to stop the complex sound.
				nldebug("Setting last event for sound %s in %u millisec.", _Source1->getSound()->getName().toString().c_str()/*CStringMapper::unmap(_Source1->getSound()->getName()).c_str()*/, _Source1->getSound()->getDuration());
				if (_PatternSound->doFadeOut())
				{
					// set the event to begin fade out.
					mixer->addEvent(this, _StartTime1 + _Source1->getSound()->getDuration() - _PatternSound->getFadeLength());
				}
				else
				{
					// set the event at end of sound.
					mixer->addEvent(this, _StartTime1 + _Source1->getSound()->getDuration());
				}
			}
		}
		else
		{
			if (_PatternSound->doFadeOut())
			{
				// update is responsible for stoping the sound.
				_Playing = false;
			}
		}
		// remove from the update list
		mixer->unregisterUpdate(this);
	}
	else
	{
//		nldebug("XFade : %4.3f <=> %4.3f (Fade Len = %6.3f", (1.0f-_FadeFactor)*_Gain, _FadeFactor*_Gain, _FadeLength/1000.0f);

		// do the xfade
		if (_Source1)
		{
			// lower the sound 1.
			_Source1->setRelativeGain((1.0f - _FadeFactor) * _Gain);

		}
		if (_Source2)
		{
			// lower the sound 1.
//			_Source2->setRelativeGain(float(sqrt(_FadeFactor)) * _Gain);
			_Source2->setRelativeGain(_FadeFactor * _Gain);
		}
	}
}
/// Mixer event implementation.
void CComplexSource::onEvent()
{
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	NLMISC::TTime now = NLMISC::CTime::getLocalTime();

	switch (_PatternSound->getPatternMode())
	{
	case CComplexSound::MODE_CHAINED:
		{
			// either it's time to begin a new xfade, or to end this sound.
			if (_Source2 != NULL)
			{
				// start new cross fade.?
				_StartTime2 = now;
				// mute the source2
				_Source2->setRelativeGain(0);
				_Source2->setPitch(_Source2->getSound()->getPitch() * _Pitch);
				_Source2->setPos(_Position);
				// start the source 2
				_Source2->play();
				// register for update.
				mixer->registerUpdate(this);
			}
			else
			{
				if (_PatternSound->doFadeOut())
				{
					// set in update list for fade out.
					_StartTime2 = now;
					mixer->registerUpdate(this);
				}
				else
				{
					// end the sound.
//					_Source1->stop();
//					mixer->removeSource(_Source1);
					delete _Source1;
					_Source1 = NULL;
					_Playing = false;
				}
			}
		}
		break;
	case CComplexSound::MODE_SPARSE:
		{
			if (_Source1 != NULL)
			{
//				_Source1->stop();
//				mixer->removeSource(_Source1);
				delete _Source1;
				_Source1 = NULL;
			}

			const std::vector<uint32> &delaySeq = _PatternSound->getDelaySeq();
			const vector<uint32>	&soundSeq = _PatternSound->getSoundSeq();

			if (_Looping && _DelaySeqIndex >= delaySeq.size())
			{
				_DelaySeqIndex = 1;
/*				if (!delaySeq.empty() && delaySeq[0] == 0)
					_DelaySeqIndex = 1;
				else
					_DelaySeqIndex = 0;
*/			}

			if (!soundSeq.empty() && !_LastSparseEvent)
			{
				// wrap around sound sequence until there are delays...
				if (_SoundSeqIndex >= soundSeq.size())
					_SoundSeqIndex = 0;

				CSound *sound = mixer->getSoundId(_PatternSound->getSound(soundSeq[_SoundSeqIndex++]));

				_Source1 = mixer->createSource(sound, false, 0, 0, _Cluster, NULL, _GroupController);
				if (_Source1 == NULL)
				{
					stop();
					return;
				}
				_Source1->setPriority(_Priority);
				_Source1->setRelativeGain(_Gain*_Gain*_Gain);
				_Source1->setPitch(_Source1->getSound()->getPitch() * _Pitch);
				_Source1->setPos(_Position);
				_Source1->play();
				_StartTime1 = now;

				// register event for next sound.
				if (!delaySeq.empty() && _DelaySeqIndex < delaySeq.size())
				{
					// event for next sound.
					mixer->addEvent(this, uint64(now + sound->getDuration() + delaySeq[_DelaySeqIndex++]/_TickPerSecond));
					if (_DelaySeqIndex == delaySeq.size() && !_Looping)
						_LastSparseEvent = true;
				}
				else
				{
					// event for stop ?
					if (!_Looping)
						_LastSparseEvent = true;
					else
						_LastSparseEvent = false;
					mixer->addEvent(this, now + sound->getDuration());
				}
			}
			else
			{
				// this is the event for stop !
				stop();
			}
		}
		break;
	case CComplexSound::MODE_ALL_IN_ONE:
		// just call the stop method.
		stop();
		break;
	default:
		nlassert(false);
	}
}


void CComplexSource::checkup()
{
	if (_Muted)
		return;

	if (_Source1 != NULL && _Source1->getSound()->getLooping() && !_Source1->isPlaying())
		_Source1->play();
	if (_Source2 != NULL && _Source2->getSound()->getLooping() && !_Source2->isPlaying())
		_Source2->play();

	std::vector<USource	*>::iterator first(_AllSources.begin()), last(_AllSources.end());
	for (; first != last; ++first)
	{
		USource *source = *first;
		if (source == NULL)
			continue;
		if (source->getSound()->getLooping() && !source->isPlaying())
			source->play();
		if (source->getSound()->getSoundType() != CSound::SOUND_SIMPLE)
			static_cast<CSourceCommon*>(source)->checkup();
	}
}



} // NLSOUND
