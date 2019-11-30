// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2015-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_CONTEXT_SOUND_H
#define NL_CONTEXT_SOUND_H

#include "nel/sound/sound.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/string_mapper.h"

namespace NLSOUND {

class ISoundDriver;
class IBuffer;
class CSound;


template <uint NbJoker, bool UseRandom, uint Shift = 5>
struct CContextMatcher
{
	// speudo constante
	enum
	{
		// Size of array : special case for 0 joker because we can't declare array of 0 elements
		JOKER_ARRAY_SIZE = (NbJoker == 0 ? 1 : NbJoker)
	};

	CContextMatcher(uint32 *jokersValues, uint32 randomValue)
		: HashValue(0)
	{
		uint i;
		for (i=0; i<NbJoker; ++i)
		{
			JokersValues[i] = jokersValues[i];

			uint leftShift = (5*i)&0x1f;
			HashValue ^= JokersValues[i] << leftShift;
			HashValue ^= JokersValues[i] >> (32-leftShift);
		}
		if (UseRandom)
		{
			RandomValue = randomValue;
			uint leftShift = (5*i)&0x1f;
			HashValue ^= randomValue << leftShift;
			HashValue ^= randomValue >> (32-leftShift);
		}
		else
			RandomValue = 0;
	}

	bool operator ==(const CContextMatcher &other) const
	{
		if (HashValue != other.HashValue)
			return false;
		else if (UseRandom)
			return RandomValue == other.RandomValue && memcmp(JokersValues, other.JokersValues, sizeof(uint32)*NbJoker) == 0;
		else
			return memcmp(JokersValues, other.JokersValues, sizeof(uint32)*NbJoker) == 0;
	}

	bool operator<(const CContextMatcher &other) const
	{
		if (UseRandom)
			if (RandomValue != other.RandomValue)
				return RandomValue < other.RandomValue;

		int cmp = memcmp(JokersValues, other.JokersValues, sizeof(uint32) * NbJoker);
		if (cmp != 0)
			return cmp < 0;

		return false;
	}

	size_t getHashValue() const
	{
		return size_t(HashValue);
	}

	uint32	HashValue;
	uint32	JokersValues[JOKER_ARRAY_SIZE];
	uint32	RandomValue;

	struct CHash : public std::unary_function<CContextMatcher, size_t>
	{
		enum { bucket_size = 4, min_buckets = 8, };
		size_t operator () (const CContextMatcher &patternMatcher) const
		{
			return patternMatcher.getHashValue();
		}
		bool operator() (const CContextMatcher &patternMatcher1, const CContextMatcher &patternMatcher2) const
		{
			return patternMatcher1 < patternMatcher2;
		}
	};
};


class IContextSoundContainer
{
public:
	virtual				~IContextSoundContainer() {}
	virtual void		init(uint *contextArgsIndex) =0;
	virtual void		addSound(CSound *sound, const std::string &baseName) =0;
	virtual CSound		*getSound(const CSoundContext &context, uint32 randomValue) =0;
	virtual void		getSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const =0;
	virtual float		getMaxDistance() const =0;
};

template <uint NbJoker, bool UseRandom, uint Shift = 5>
class CContextSoundContainer : public IContextSoundContainer
{
	// pseudo constants
	enum
	{
		// Size of array : special case for 0 joker because we can't declare array of 0 elements
		JOKER_ARRAY_SIZE = (NbJoker == 0 ? 1 : NbJoker)
	};

	typedef CHashMap<CContextMatcher<NbJoker, UseRandom, Shift>, CSound *, typename CContextMatcher<NbJoker, UseRandom, Shift>::CHash>	THashContextSound;

	virtual void		init(uint *contextArgsIndex)
	{
		_MaxDist = 0;
		NLMISC::CFastMem::memcpy(_ContextArgsIndex, contextArgsIndex, sizeof(uint) * NbJoker);
	}

	virtual float		getMaxDistance() const
	{
		return _MaxDist;
	}

	virtual void		addSound(CSound *sound, const std::string &baseName)
	{
		const std::string &patternName = NLMISC::CStringMapper::unmap(sound->getName());
		nlassert(patternName.size() >= baseName.size());

		std::string arg;
		uint32		args[JOKER_ARRAY_SIZE];

		_MaxDist = std::max(sound->getMaxDistance(), _MaxDist);

		// extract the context values
		std::string::const_iterator	first(patternName.begin() + baseName.size()), last(patternName.end());
//		std::string::const_iterator	first2(baseName.begin()), last2(baseName.end());
		// 1st, skip the base name
//		for (; first == first2; ++first, ++first2);

		// 2nd, read all the joker values
		uint i;
		for ( i=0; i<NbJoker && first != last; ++first)
		{
			if (isdigit(int(*first)))
			{
				arg += *first;
			}
			else if (!arg.empty())
			{
				// end of the argument.
				NLMISC::fromString(arg, args[i++]);
				arg.clear();
			}
		}
		// read the potential last arg.
		if (!arg.empty())
		{
			// end of the argument.
			NLMISC::fromString(arg, args[i++]);
			arg.clear();
		}

		if (i != NbJoker)
			return;
		nlassertex(i==NbJoker, ("Error while adding sound '%s' into context sound container", NLMISC::CStringMapper::unmap(sound->getName()).c_str()));

		sint randomValue = 0;
		if (UseRandom)
		{
			bool ok = false;
			// 3rd, read the random value (if any)
			while(first != last)
			{
				if (isdigit(int(*first)))
				{
					arg += *first;
				}
				else if (!arg.empty())
				{
					nlassertex (!ok, ("Error while adding sound '%s' into context sound container", NLMISC::CStringMapper::unmap(sound->getName()).c_str()));
					// end of the argument.
					NLMISC::fromString(arg, randomValue);
					arg.clear();
					ok = true;
				}

				++first;
			}
			// read the potential last arg.
			if (!arg.empty())
			{
				nlassertex (!ok, ("Error while adding sound '%s' into context sound container", NLMISC::CStringMapper::unmap(sound->getName()).c_str()));
				// end of the argument.
				NLMISC::fromString(arg, randomValue);
				arg.clear();
				ok = true;
			}
			nlassertex (ok, ("Error while adding sound '%s' into context sound container", NLMISC::CStringMapper::unmap(sound->getName()).c_str()));

		}
		else
		{
			randomValue = 0;
		}

		// ok, now create the key and store the sound.
		CContextMatcher<NbJoker, UseRandom, Shift>	cm(args, randomValue);

		std::pair<typename THashContextSound::iterator, bool>	ret;
		ret = _ContextSounds.insert(std::make_pair(cm, sound));
		if (!ret.second)
		{
			typename THashContextSound::iterator it = _ContextSounds.find(cm);
			nlassertex(it != _ContextSounds.end(), ("Error wile adding soudn '%s' into context sound container", NLMISC::CStringMapper::unmap(sound->getName()).c_str()));

			nlwarning("Sound %s has the same context matcher as the sound %s", NLMISC::CStringMapper::unmap(sound->getName()).c_str(), NLMISC::CStringMapper::unmap(it->second->getName()).c_str());
		}
	}

	virtual CSound		*getSound(const CSoundContext &context, uint32 randomValue)
	{
		// create a key
		uint32		args[JOKER_ARRAY_SIZE];
		for (uint i=0; i<NbJoker; ++i)
			args[i] = context.Args[_ContextArgsIndex[i]];

		CContextMatcher<NbJoker, UseRandom, Shift>	cm(args, randomValue);

		typename THashContextSound::iterator it = _ContextSounds.find(cm);

		if (it != _ContextSounds.end())
			return it->second;
		else
			return 0;
	}

	void getSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const
	{
		typename THashContextSound::const_iterator first(_ContextSounds.begin()), last(_ContextSounds.end());
		for (; first != last; ++first)
		{
			subsounds.push_back(std::make_pair(NLMISC::CStringMapper::unmap(first->second->getName()), first->second));
		}
	}

private:
	uint32				_ContextArgsIndex[JOKER_ARRAY_SIZE];
	THashContextSound	_ContextSounds;
	float				_MaxDist;
};

class CContextSound : public CSound
{
public:
	/// Constructor
	CContextSound();
	/// Destructor
	~CContextSound();


	TSOUND_TYPE			getSoundType()					{ return CSound::SOUND_CONTEXT; };

	/// Load the sound parameters from georges' form
	virtual void		importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);

	/// Return true if cone is meaningful
	virtual bool		isDetailed() const;
	/// Return the length of the sound in ms
	virtual uint32		getDuration();
	/// Used by the george sound plugin to check sound recursion (ie sound 'toto' use sound 'titi' witch also use sound 'toto' ...).
	virtual void		getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const;


	CSound				*getContextSound(CSoundContext &context);

	void				init();

	void				serial(NLMISC::IStream &s);

	float				getMaxDistance() const;



private:

	/// The context sound pattern name.
	std::string					_PatternName;
	/// The base name, that is the constante part of the name (before the first joker).
	std::string					_BaseName;

	/// The random length (0 mean no random)
	uint32						_Random;



	/// container for all the candidate sounds
	IContextSoundContainer		*_ContextSounds;

};

} // NLSOUND

#endif //NL_CONTEXT_SOUND_H
