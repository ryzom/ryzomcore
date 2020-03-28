// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/sound/complex_sound.h"
#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "nel/sound/audio_mixer_user.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND
{

bool CComplexSound::isDetailed() const
{
	return false;
}

void CComplexSound::parseSequence(const std::string &str, std::vector<uint32> &seq, uint scale)
{
	seq.clear();

	uint32 tmp;
	string	val;
	string::const_iterator first(str.begin()), last(str.end());

	for (; first != last; ++first)
	{
		if (*first != ';')
			val += *first;
		else
		{
			fromString(val, tmp);
			seq.push_back(tmp * scale);
			val.clear();
		}
	}

	// parse the last value
	if (!val.empty())
	{
		fromString(val, tmp);
		seq.push_back(tmp * scale);
	}

}

void CComplexSound::getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const
{
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	std::vector<NLMISC::TStringId>::const_iterator first(_Sounds.begin()), last(_Sounds.end());
	for (; first != last; ++first)
	{
		CSound *sound = mixer->getSoundId(*first);
		subsounds.push_back(make_pair(CStringMapper::unmap(*first), sound));
	}
}


uint32 CComplexSound::getDuration()
{
	// evaluate the duration of the sound...

	if (_DurationValid)
		return _Duration;

	// catch the duration of all sub sound.
	CAudioMixerUser *mixer = CAudioMixerUser::instance();

	vector<sint32>	durations;
	std::vector<NLMISC::TStringId>::iterator first(_Sounds.begin()), last(_Sounds.end());
	for (; first != last; ++first)
	{
		CSound *sound = mixer->getSoundId(*first);
		if (sound != NULL)
		{
			durations.push_back(sint32(sound->getDuration()));
		}
		else
			durations.push_back(0);

	}

	_Duration = 0;
	switch (_PatternMode)
	{
	case MODE_CHAINED:
		{
			// sum the duration minus the xfade time (this is an aproximation if sample are shorter than 2 xfade time)
			vector<uint32>::iterator first(_SoundSeq.begin()), last(_SoundSeq.end()), prev;
			for (; first != last; ++first)
			{
				if (first != _SoundSeq.begin() && !durations.empty())
				{
					// remove a xfade value
					_Duration -= minof<uint32>(uint32(_XFadeLength / _TicksPerSeconds), durations[*first % durations.size()] / 2, durations[*prev % durations.size()] /2);
				}
				if (!durations.empty())
					_Duration += durations[*first % durations.size()];
				prev = first;
			}
//			_Duration -= max(sint(0), sint(_XFadeLength * (_SoundSeq.size()-2)  ));
		}
		break;
	case MODE_SPARSE:
		{
			if (_SoundSeq.empty())
				_Duration = 0;
			else if (_DelaySeq.empty())
			{
				_Duration = durations[0];
			}
			else if (_DelaySeq.size() == 1)
			{
				_Duration = durations[0] + _DelaySeq[0];
			}
			else
			{
				uint soundIndex = 0;
				_Duration = 0; //durations[soundIndex++];

				std::vector<uint32>::iterator first(_DelaySeq.begin()), last(_DelaySeq.end());

				_Duration+= *first;
				++first;
				for (; first != last; ++first)
				{
					// add the sound length
					_Duration += durations[soundIndex++ % durations.size()];
					// add the delay
					_Duration += uint32(*first / _TicksPerSeconds);
				}
			}
		}
		break;
	case MODE_ALL_IN_ONE:
		// only find the longueur sound.
		if (!durations.empty())
			_Duration = *(std::max_element(durations.begin(), durations.end()));
		else
			_Duration = 0;
		break;
	default:
		return 0;
	}

	_DurationValid = true;
	return _Duration;

}


// ********************************************************

CComplexSound::CComplexSound() :
	_PatternMode(CComplexSound::MODE_UNDEFINED),
	_TicksPerSeconds(1.0f),
	_XFadeLength(3000),		// default to 3000 sec.
	_DoFadeIn(true),
	_DoFadeOut(true),
	_MaxDistValid(false),
	_Duration(0),
	_DurationValid(false)
{
}

// ********************************************************

CComplexSound::~CComplexSound()
{
/*	if (_VolumeEnvelope != 0)
	{
		delete _VolumeEnvelope;
	}

	if (_FreqModulation != 0)
	{
		delete _FreqModulation;
	}
*/
}

float CComplexSound::getMaxDistance() const
{
	if (!_MaxDistValid)
	{
		// compute the max distance by checking the max distance of all sounds.
		CAudioMixerUser *mixer = CAudioMixerUser::instance();

		// Hum, getMaxDistance is const, but we must compute the real max dist and update it !
		CComplexSound *This = const_cast<CComplexSound*>(this);

		This->_MaxDist = 0.0f;
		std::vector<NLMISC::TStringId>::const_iterator first(_Sounds.begin()), last(_Sounds.end());

		for (; first != last; ++first)
		{
			CSound *sound = mixer->getSoundId(*first);
			if( sound != NULL)
			{
				This->_MaxDist = max(_MaxDist, sound->getMaxDistance());
			}
		}
		// security check.
		if (_MaxDist == 0.0f)
			This->_MaxDist = 1000000.0f;
	}

	_MaxDistValid = true;
	return _MaxDist;
}

void	CComplexSound::serial(NLMISC::IStream &s)
{
	CSound::serial(s);
	s.serialEnum(_PatternMode);
	if (s.isReading())
	{
		uint32 nb;
		s.serial(nb);

		for (uint i=0; i<nb; ++i)
		{
			std::string name;
			s.serial(name);
			_Sounds.push_back(CStringMapper::map(name));
		}
	}
	else
	{
		uint32 nb = (uint32)_Sounds.size();
		s.serial(nb);
		for (uint i=0; i<nb; ++i)
		{
			std::string name = CStringMapper::unmap(_Sounds[i]);
			s.serial(name);
		}
	}
	s.serial(_TicksPerSeconds);
	s.serialCont(_SoundSeq);
	s.serialCont(_DelaySeq);
	s.serial(_XFadeLength);
	s.serial(_DoFadeIn);
	s.serial(_DoFadeOut);

	if (s.isReading())
		_DurationValid = false;
}


/// Load the sound parameters from georges' form
void	CComplexSound::importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot)
{
	NLGEORGES::UFormElm *psoundType;
	std::string dfnName;

	_DurationValid = false;

	// some basic checking.
	formRoot.getNodeByName(&psoundType, ".SoundType");
	nlassert(psoundType != NULL);
	psoundType->getDfnName(dfnName);
	nlassert(dfnName == "complex_sound.dfn");

	// Call the base class
	CSound::importForm(filename, formRoot);

	// Beat per second.
	formRoot.getValueByName(_TicksPerSeconds, ".SoundType.Beat");
	//beat can't be null or negative!
	if (_TicksPerSeconds <= 0.0f)
		_TicksPerSeconds = 1.0f;



	// List of sound int this pattern
	NLGEORGES::UFormElm	*psoundsArray;
	_Sounds.clear();
	formRoot.getNodeByName(&psoundsArray, ".SoundType.SoundList");

	if (psoundsArray != NULL)
	{
		uint size;
		psoundsArray->getArraySize(size);
		for (uint i=0; i<size; ++i)
		{
			string soundname;
			if (psoundsArray->getArrayValue(soundname, i))
			{
				soundname = CFile::getFilenameWithoutExtension(soundname);
				_Sounds.push_back(CStringMapper::map(soundname));
			}
		}
	}


	// Mode of the complex sound.
	string	mode;
	formRoot.getValueByName(mode, ".SoundType.Mode");

	if (mode == "Chained" || mode == "Sparse")
	{
		// XFade length
		if (!formRoot.getValueByName(_XFadeLength, ".SoundType.XFadeLength"))
			formRoot.getValueByName(_XFadeLength, ".SoundType.XFadeLenght"); // WORKAROUND: Typo in sound assets
		// Fade in/out flag.
		formRoot.getValueByName(_DoFadeIn, ".SoundType.DoFadeIn");
		formRoot.getValueByName(_DoFadeOut, ".SoundType.DoFadeOut");

		// convert xfade to millisec.
		_XFadeLength *= 1000;
		_PatternMode = MODE_CHAINED;
		// just read the sequence
		_SoundSeq.clear();

		string	str;
		formRoot.getValueByName(str, ".SoundType.SoundSeq");
		parseSequence(str, _SoundSeq);

		if (mode == "Sparse")
		{
			_PatternMode = MODE_SPARSE;
			// also read the delay sequence
			_DelaySeq.clear();

			string	str;
			formRoot.getValueByName(str, ".SoundType.DelaySeq");
			// parse the delay and premult by 1000 (for millisec).
			parseSequence(str, _DelaySeq, 1000);
		}
	}
	else if (mode == "AllInOne")
	{
		_PatternMode = MODE_ALL_IN_ONE;
		// nothing special to read.
	}
	else
		nlassertex(false, ("Unsupported mode : %s", mode.c_str()));

}

}
