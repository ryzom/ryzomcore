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

#ifndef NL_SOUND_PATTERN_H
#define NL_SOUND_PATTERN_H

#include "nel/misc/types_nl.h"

namespace NLSOUND
{

class ISoundController;


typedef std::basic_string<uint16>	uint16_string;


class CSoundPattern
{
public:

	class Iterator
	{
	public:
		uint16 _Max;
		uint16 _Value;
		CSoundPattern* _Pattern;

		Iterator(CSoundPattern* pattern, uint16 max) : _Pattern(pattern), _Value(0), _Max(max) {}

		Iterator& operator++() {
			_Value++;
			if (_Value >= _Max) {
				_Value -= _Max;
			}
		}
	};

	class PatternIterator : public Iterator
	{
	public:
		PatternIterator() : Iterator(0, 0) {}
		PatternIterator(CSoundPattern* pattern, uint16 max) : Iterator(pattern, max) {}

		std::string& operator*() {
			return _Pattern->getSound(this);
		}
	};

	class IntervalIterator : public Iterator
	{
	public:
		IntervalIterator() : Iterator(0, 0) {}
		IntervalIterator(CSoundPattern* pattern, uint16 max) : Iterator(pattern, max) {}

		uint32 operator*() {
			return _Pattern->getInterval(this);
		}
	};

	/** Constructor */
	CSoundPattern();

	/** Destructor */
	virtual ~CSoundPattern();


	/// \name Sounds list management
	//@{

	/** \def addSound(name)
	 *  Add a sound to the list of sounds used in this pattern
	 */

	/** \def removeSound(name)
	 *  Remove a sound from the list of sounds used in this pattern
	 */

	/** \def getSounds(sounds)
	 *  Get the list of all sounds in this pattern
	 */

	virtual void				addSound(std::string& name)					{ _Sounds.push_back(name); }
	virtual void				removeSound(const NLMISC::TStringId& name);
	virtual void				getSounds(std::vector<std::string>& sounds);
	//@}


	/// \name Iterating through the sound pattern
	//@{

	/** \def beginSoundPattern()
	 *  Get the beginning of the pattern. The pattern has infinite length. The code to run
	 *  through the list of elements ressembles the iteration in STL classes:
	 *
	 *\code

	 	PatternIterator iterator = pattern.beginSoundPattern();

	 	while (true)
	 	{
	 		string& sound = *iterator;
	 		iterator++;
	 	}

	 *\endcode
	 */

	/** \def getSound(iterator)
	 *  Returns the sound corresponding to the iterator value in the sound pattern.
	 */

	virtual PatternIterator		beginSoundPattern()							{ return PatternIterator(this, _SoundPattern.size()); }
	virtual std::string&		getSound(PatternIterator* iterator);
	//@}


	/// \name Iterating through the sound pattern
	//@{

	/** \def beginIntervalPattern()
	 *  Get the beginning of the interval pattern. The pattern has infinite length. The code
	 *  to run through the list of elements ressembles the iteration in STL classes. See
	 *  code example in beginSoundPattern().
  	 * \see beginSoundPattern
	 */

	/** \def getInterval(iterator)
	 *  Returns the interval corresponding to the iterator value in the sound pattern.
	 */

	virtual IntervalIterator	beginIntervalPattern()						{ return IntervalIterator(this, _Intervals.size()); }
	virtual uint16				getInterval(IntervalIterator* iterator);
	//@}

	/// \name Sound pattern editing and generation
	//@{
	/** \def setSoundPattern(list)
	 *  Set the pattern of the intervals. The list should be a string of numbers
	 *  separated by a comma. For example, "100,120,100,5,10,50"
	 */

	/** \def getIntervals(list)
	 *  Returns the pattern of the intervals as a string. The returned list has
	 *  the same format as expected by the setIntervals function.
	 *  \see setIntervals
	 */

	virtual void				setSoundPattern(std::string& list);
	virtual void				getSoundPattern(std::string& list);
	virtual void				generateRandomPattern(uint length);
	virtual void				generateRandomMin1Pattern(uint length);
	//@}

	/// \name Interval pattern editing and generation
	//@{

	/** \def setIntervals(list)
	 *  Set the pattern of the intervals. The list should be a string of numbers
	 *  separated by a comma. For example, "100,120,100,5,10,50"
	 */

	/** \def getIntervals(list)
	 *  Returns the pattern of the intervals as a string. The returned list has
	 *  the same format as expected by the setIntervals function.
	 *  \see setIntervals
	 */

	virtual void				setIntervals(std::string& list);
	virtual void				getIntervals(std::string& list);
	virtual void				generateRandomIntervals(uint length, uint16 min, uint16 max);
	virtual void				generateRandomMin1Intervals(uint length, uint16 min, uint16 max);
	//@}

	/// \name Tempo
	//@{
	virtual float				getTicksPerSecond()							{ return _TicksPerSeconds; }
	virtual void				setTicksPerSecond(float ticks)				{ _TicksPerSeconds = ticks; }
	//@}

	/// \name Spawning
	//@{
	virtual void				setSpawn(bool v)							{ _Spawn = v; }
	virtual bool				getSpawn()									{ return _Spawn; }
	//@}

	/// \name Continuous controllers
	//@{
	virtual void				setVolumeEnvelope(ISoundController* env)	{ _VolumeEnvelope = env; }
	virtual ISoundController*	getVolumeEnvelope()							{ return _VolumeEnvelope; }
	virtual void				setFreqModulation(ISoundController* mod)	{ _FreqModulation = mod; }
	virtual ISoundController*	getFreqModulation()							{ return _FreqModulation; }
	//@}



private:

	virtual void				parsePattern(std::string& list, uint16_string& pattern);
	virtual void				concatenatePattern(std::string& list, uint16_string& pattern);
	virtual void				generateRandomPattern(uint16_string& pattern, uint length, uint16 min, uint16 max);
	virtual void				generateRandomMin1Pattern(uint16_string& pattern, uint length, uint16 min, uint16 max);
	virtual void				expandString(std::string& s, std::string& buffer);

	std::vector<std::string>	_Sounds;
	float						_TicksPerSeconds;
	uint16_string				_SoundPattern;
	uint16_string				_Intervals;
	bool						_Spawn;
	ISoundController			*_VolumeEnvelope;
	ISoundController			*_FreqModulation;
	std::string					_StringBuffer;
};

} // namespace

#endif // NL_SOUND_PATTERN_H
