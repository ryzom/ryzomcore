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

/** 
 * This file defines the classes:
 * - CProfileOwner
 * - IAIProfileFactory
 * - CAIGenericProfileFactory
 * - CAIBaseProfile
 * - CProfilePtr
 */

#ifndef RYAI_AI_PROFILE_H
#define RYAI_AI_PROFILE_H


//#pragma warning (disable : 4355)	//	warning C4355: 'this' : used in base member initializer list

// This is the base class for defining NPC behaviour profiles
// The team infrastructure manages the allocation of AI profiles to bots

class CProfilePtr;
class IAIProfile;

//////////////////////////////////////////////////////////////////////////////
// CProfileOwner                                                            //
//////////////////////////////////////////////////////////////////////////////

class CProfileOwner
{
public:
	virtual ~CProfileOwner() { }	
};

//////////////////////////////////////////////////////////////////////////////
// IAIProfileFactory                                                       //
//////////////////////////////////////////////////////////////////////////////

class IAIProfileFactory
: public NLMISC::CDbgRefCount<IAIProfileFactory>
{
public:
	friend class CProfilePtr;
	virtual ~IAIProfileFactory() { }
	virtual	NLMISC::CSmartPtr<IAIProfile> createAIProfile(CProfileOwner *owner) = 0;
};

#define RYAI_DECLARE_PROFILE_FACTORY(ProfileClass) RYAI_DECLARE_FACTORY(IAIProfileFactory, ProfileClass, std::string);
#define RYAI_REGISTER_PROFILE_FACTORY(ProfileClass, KeyValue) RYAI_REGISTER_FACTORY(IAIProfileFactory, ProfileClass, std::string, std::string(KeyValue));

//////////////////////////////////////////////////////////////////////////////
// CAIGenericProfileFactory                                                 //
//////////////////////////////////////////////////////////////////////////////

template <class TProfile>
class CAIGenericProfileFactory
: public IAIProfileFactory
{
public:
	NLMISC::CSmartPtr<IAIProfile> createAIProfile(CProfileOwner* owner)
	{
		return new TProfile(owner);
	}
};

//////////////////////////////////////////////////////////////////////////////
// CAIBaseProfile                                                           //
//////////////////////////////////////////////////////////////////////////////

class IAIProfile
: public NLMISC::CRefCount
{
public:
	virtual ~IAIProfile() { }
	
	/// @name Virtual interface
	//@{
	// routine called when a profOwner starts to use a given profile
	// note that bots have a data member called 'void *aiProfileData' reserved for the use of the profile code
	// this data member should be setup here if it is to be used by the profile
	virtual void beginProfile() = 0;
	// routine called every time the profOwner is updated (frequency depends on player proximity, etc)
	virtual void updateProfile(uint ticksSinceLastUpdate) = 0;
	// routine called just before profOwner starts to use a new profile or when a profOwner dies
	virtual void endProfile() = 0;
	virtual AITYPES::TProfiles getAIProfileType() const = 0;
	virtual std::string getOneLineInfoString() const = 0;
	// routine called every time the profOwner's group changes state but profOwner maintains same ai profile
	virtual void stateChangeProfile() = 0;
	virtual void resumeProfile() = 0;
	//@}
};

//////////////////////////////////////////////////////////////////////////////
// CAIBaseProfile                                                           //
//////////////////////////////////////////////////////////////////////////////

class CAIBaseProfile
: public IAIProfile
{
public:
	virtual ~CAIBaseProfile() { }
	
	/// @name IAIProfile base implementation
	//@{
	virtual void stateChangeProfile() { beginProfile(); }
	virtual void resumeProfile() { }
	//@}
};

//////////////////////////////////////////////////////////////////////////////
// CProfilePtr                                                              //
//////////////////////////////////////////////////////////////////////////////

class CProfilePtr
{
public:
	enum TStartProfileType
	{
		START_BEGIN = 0,
		START_RESUME
	};
	
public:
	CProfilePtr();
	
	virtual ~CProfilePtr();
	
//	std::string buildProfileDebugString() const;
	virtual std::string getOneLineInfoString() const;
	
	AITYPES::TProfiles	getAIProfileType() const;
	
	template <class T>
	void setAIProfile(T* obj, IAIProfileFactory* profile, bool callStateChangedIfSame) const
	{
		if (profile)
		{
			setAIProfile(profile->createAIProfile(obj), callStateChangedIfSame);
		}
	}
	
	void setAIProfile(NLMISC::CSmartPtr<IAIProfile> profile, bool callStateChangedIfSame = false, TStartProfileType startType = START_BEGIN) const;
	
	void updateProfile(uint ticks) const;
	
	void mayUpdateProfile(uint ticks) const;
	
	IAIProfile* getAIProfile() const { return _AiProfile; }
	NLMISC::CSmartPtr<IAIProfile> const& getAISpawnProfile() const { return _AiProfile; }
	
private:
	mutable	NLMISC::CSmartPtr<IAIProfile>	_AiProfile;
	mutable	NLMISC::CSmartPtr<IAIProfile>	_NextAiProfile;
	mutable bool				_NextAiProfileCallStateChangedIfSame;
	
	mutable	TStartProfileType	_NextStartType;
	mutable	bool				_IsUpdating;
};

//////////////////////////////////////////////////////////////////////////////
// Global functions                                                         //
//////////////////////////////////////////////////////////////////////////////

/// the lookup routine that serves as a kind of repository interface
IAIProfileFactory* lookupAIGrpProfile(char const* name);

/****************************************************************************/
/* Inlined methods                                                          */
/****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
// CProfilePtr                                                              //
//////////////////////////////////////////////////////////////////////////////

inline
CProfilePtr::CProfilePtr()
: _AiProfile(NULL)
, _NextAiProfile(NULL)
, _NextStartType(START_BEGIN)
, _IsUpdating(false)
{
}

inline
CProfilePtr::~CProfilePtr()
{
	_NextAiProfile = NULL;
	_AiProfile = NULL;
}

inline
std::string CProfilePtr::getOneLineInfoString() const
{
	if (_AiProfile.isNull())
		return std::string("No Profile");
	return _AiProfile->getOneLineInfoString();
}

inline
AITYPES::TProfiles CProfilePtr::getAIProfileType() const
{
	if (!_AiProfile.isNull())
		return _AiProfile->getAIProfileType();
	return AITYPES::BAD_TYPE;	// unknown
}

inline
void CProfilePtr::setAIProfile(NLMISC::CSmartPtr<IAIProfile> profile, bool callStateChangedIfSame, TStartProfileType startType) const
{
	// :NOTE: profile can be NULL
	if (_IsUpdating)
	{
		_NextAiProfileCallStateChangedIfSame = callStateChangedIfSame;
		_NextAiProfile = profile;
		_NextStartType = startType;
		return;
	}
	
	if (!_AiProfile.isNull())
	{
		// we may use the == operator because it doesn't take account of parameters (which is bad) :(
		if (callStateChangedIfSame==true && _AiProfile->getAIProfileType ()==profile->getAIProfileType ())	// if we already have this profile, then call its stateChangeProfile method
		{
			_AiProfile->stateChangeProfile();
			return;
		}
		
		_AiProfile->endProfile();
		_AiProfile = NULL;
	}
	
	if (!profile.isNull())
	{
		_AiProfile = profile;
		if (startType==START_BEGIN)
			_AiProfile->beginProfile();
		else
			_AiProfile->resumeProfile();
	}
}

inline
void CProfilePtr::updateProfile(uint ticks) const
{
	BOMB_IF(_AiProfile.isNull(),"Attempting updateProfile() with _AiProfile.isNull()",return);

	_IsUpdating = true;
	_AiProfile->updateProfile(ticks);
	_IsUpdating = false;
	
	if	(!_NextAiProfile.isNull())
	{
		setAIProfile(_NextAiProfile, _NextAiProfileCallStateChangedIfSame, _NextStartType);
		_NextAiProfile = NULL;
	}
}

inline
void CProfilePtr::mayUpdateProfile(uint ticks) const
{
	if (_AiProfile)
		updateProfile(ticks);
}

#endif
