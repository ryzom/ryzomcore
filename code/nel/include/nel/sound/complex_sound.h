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

#ifndef NL_COMPLEX_SOUND_H
#define NL_COMPLEX_SOUND_H

#include "nel/misc/types_nl.h"
#include "nel/misc/string_mapper.h"
#include "nel/sound/sound.h"
#include <string>

namespace NLSOUND
{

class ISoundController;


typedef std::basic_string<uint16>	uint16_string;


class CComplexSound : public CSound
{
public:

	enum TPATTERN_MODE
	{
		MODE_UNDEFINED,
		MODE_CHAINED,
		MODE_ALL_IN_ONE,
		MODE_SPARSE
	};

	bool							isDetailed() const;
	uint32							getDuration();


	TPATTERN_MODE					getPatternMode()							{ return _PatternMode;}
	void							setPatternMode(TPATTERN_MODE patternMode)	{ _PatternMode = patternMode;}

	const std::vector<uint32>		&getSoundSeq() const						{ return _SoundSeq;}
	const std::vector<uint32>		&getDelaySeq() const						{ return _DelaySeq;}
	NLMISC::CSheetId				getSound(uint index) const					{ return !_Sounds.empty() ? _Sounds[index%_Sounds.size()]:NLMISC::CSheetId::Unknown;}
	const std::vector<NLMISC::CSheetId>	&getSounds() const					{ return _Sounds;}

	uint32							getFadeLength() const						{ return _XFadeLength;}

	/** Constructor */
	CComplexSound();

	/** Destructor */
	virtual ~CComplexSound();

	/// Load the sound parameters from georges' form
	virtual void					importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);

	/// \name Tempo
	//@{
	virtual float					getTicksPerSecond()							{ return _TicksPerSeconds; }
	virtual void					setTicksPerSecond(float ticks)				{ _TicksPerSeconds = ticks; }
	//@}

	TSOUND_TYPE						getSoundType() {return SOUND_COMPLEX;};

	void							getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const;
	bool							doFadeIn()								{ return _DoFadeIn; }
	bool							doFadeOut()								{ return _DoFadeOut; }

	void							serial(NLMISC::IStream &s);


private:

	void							parseSequence(const std::string &str, std::vector<uint32> &seq, uint scale = 1);
	virtual float					getMaxDistance() const;

	TPATTERN_MODE					_PatternMode;
	std::vector<NLMISC::CSheetId>	_Sounds;
	float							_TicksPerSeconds;
	std::vector<uint32>				_SoundSeq;
	/// Sequence of delay in millisec.
	std::vector<uint32>			_DelaySeq;

	/// Duration of xfade in millisec.
	uint32						_XFadeLength;
	/// Flag for fade in
	bool						_DoFadeIn;
	/// Flag for fade out (only on normal termination, not explicit stop).
	bool						_DoFadeOut;

	mutable bool				_MaxDistValid;

	// Duration of sound.
	uint32						_Duration;
	// flag for validity of duration (after first evaluation).
	bool						_DurationValid;
};

} // namespace

#endif // NL_COMPLEX_SOUND_H

