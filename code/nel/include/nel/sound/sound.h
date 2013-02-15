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

#ifndef NL_SOUND_H
#define NL_SOUND_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/string_mapper.h"
#include "nel/sound/u_source.h"
#include "nel/georges/u_form_elm.h"
#include "nel/misc/sheet_id.h"
#include <string>

namespace NLSOUND {


class ISoundDriver;
class IBuffer;
class CSound;
class CGroupController;


/// Sound names hash map
//typedef std::hash_map<std::string, CSound*> TSoundMap;
typedef CHashMap<NLMISC::CSheetId, CSound*, NLMISC::CStringIdHashMapTraits> TSoundMap;

/// Sound names set (for ambiant sounds)
typedef std::set<CSound*> TSoundSet;

const double Sqrt12_2 = 1.0594630943592952645618252949463;  // 2^1/12

/**
 * A sound base class and its static properties
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2001
 */
class CSound
{
	friend class CAudioMixerUser;
public:
	/// Factory for specialized sound.
	static CSound *createSound(const std::string &name, NLGEORGES::UFormElm& formRoot);

	enum TSOUND_TYPE
	{
		SOUND_SIMPLE,
		SOUND_COMPLEX,
		SOUND_BACKGROUND,
		SOUND_CONTEXT,
		SOUND_MUSIC, // soon to be deprecated hopefully
		SOUND_STREAM,
		SOUND_STREAM_FILE
	};


	/// Constructor
	CSound();
	/// Destructor
	virtual ~CSound();

	/// Get the type of the sound.
	virtual TSOUND_TYPE getSoundType() =0;

	/// Load the sound parameters from georges' form
	virtual void		importForm(const std::string& filename, NLGEORGES::UFormElm& formRoot);

	/// Return the looping state
	bool				getLooping() const				{ return _Looping; }
	/// Return the gain
	float				getGain() const					{ return _Gain; }
	/// Return the pitch
	float				getPitch() const				{ return _Pitch; }
	/// Return the initial priority
	TSoundPriority		getPriority() const				{ return _Priority; }
	/// Return true if cone is meaningful
	// virtual bool		isDetailed() const = 0; // not used?
	/// Return the inner angle of the cone
	float				getConeInnerAngle() const			{ return _ConeInnerAngle; }
	/// Return the outer angle of the cone
	float				getConeOuterAngle() const			{ return _ConeOuterAngle; }
	/// Return the outer gain of the cone
	float				getConeOuterGain() const			{ return _ConeOuterGain; }
	/// Return the direction vector.
	const NLMISC::CVector &getDirectionVector()const		{ return _Direction;}
	/// Return the length of the sound in ms
	virtual uint32		getDuration() = 0;
	/// Return the name (must be unique)
	const NLMISC::CSheetId&	getName() const						{ return _Name; }

	/// Return the min distance (if detailed()) (default 1.0f if not implemented by sound type)
	virtual float		getMinDistance() const				{ return _MinDist; }
	/// Return the max distance (if detailed())
	virtual float		getMaxDistance() const				{ return _MaxDist; }

	inline CGroupController *getGroupController() const { return _GroupController; }

	/// Set looping
	void				setLooping( bool looping ) { _Looping = looping; }

	/// Used by the george sound plugin to check sound recursion (ie sound 'toto' use sound 'titi' witch also use sound 'toto' ...).
	virtual void		getSubSoundList(std::vector<std::pair<std::string, CSound*> > &subsounds) const =0;


	virtual void		serial(NLMISC::IStream &s);

	NLMISC::TStringId	getUserVarControler() { return _UserVarControler; }

	bool				operator<( const CSound& otherSound ) const
	{
		//return NLMISC::CStringMapper::unmap(_Name) < NLMISC::CStringMapper::unmap(otherSound._Name);
		return _Name.toString() < otherSound._Name.toString();
	}

protected:

	// Static properties
	float				_Gain;	// [0,1]
	float				_Pitch; // ]0,1]
	TSoundPriority		_Priority;
	float				_ConeInnerAngle, _ConeOuterAngle, _ConeOuterGain;
	NLMISC::CVector		_Direction;

	bool				_Looping;

	/// Distance to where the source is played at maximum volume. Used for stealing physical sources.
	/// Note: for compatibility reasons, _MinDist is not serial()'ized here in CSound. _MaxDist is.
	float				_MinDist;
	/// Clipping distance for complex or backgound sound.
	float				_MaxDist;

	// Sound name.
	NLMISC::CSheetId	_Name;
	/// An optional user var controler.
	NLMISC::TStringId	_UserVarControler;

	/// The group controller, always exists, owned by the audio mixer
	CGroupController	*_GroupController;

};


/**
 * ESoundFileNotFound
 */
class ESoundFileNotFound : public NLMISC::Exception
{
public:
	ESoundFileNotFound( const std::string filename ) :
	  NLMISC::Exception( (std::string("Sound file not found, or invalid file format: ")+filename).c_str() ) {}
};


} // NLSOUND


#endif // NL_SOUND_H

/* End of sound.h */

