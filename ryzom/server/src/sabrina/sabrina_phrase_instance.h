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




#ifndef RY_SABRINA_PHRASE_INSTANCE_H
#define RY_SABRINA_PHRASE_INSTANCE_H

#include "nel/misc/types_nl.h"

#include "sabrina_phrase_model.h"
#include "sabrina_pointers.h"
#include "sabrina_enum.h"
#include "items/lockable.h"
 

//------------------------------------------------------------------------
// CSabrinaPhraseInstance class
//------------------------------------------------------------------------

class CSabrinaPhraseInstance : public NLMISC::CRefCount
{

public:
	enum TPhraseState
	{
		Inactive=0,
		PreActionDelay,
		PostActionDelay,
	};

public:
	// Constructors
	CSabrinaPhraseInstance(const ISabrinaActor* actor);
	CSabrinaPhraseInstance(const CSabrinaPhraseInstance& other);

	// Destructor
	virtual ~CSabrinaPhraseInstance();

	// method used to initialise a new phrase
	void beginPhrase(const ISabrinaPhraseModel* thePhrase);

	// update routine called on event triggers
	// returns true if _NextEventTime == current time (otherwise false)
	bool updatePhrase();

	// method used to prematurely abort execution of phrase
	void abortPhrase(SABRINA::TEventCode reason);

	// method used to prolong a sabrina action (eg prolong cast...)
	void addTimeBeforeNextEvent(uint32 ticks);

	// method for interrogating the state of the phrase instance - returns false if 'Inactive' otherwise true
	bool isActive();

	// read accessor - phrase state
	TPhraseState			getState() const;

	// read accessor - execution end date
	NLMISC::TGameCycle		getNextEventTime() const;

	// read accessor - pointer to the phrase model object
	ISabrinaPhraseModel*	getPhraseModel() const;

	// read accessor - pointer to the actor
	ISabrinaActor*			getActor() const;

	// read accessor - pointer to the target
	ISabrinaActor*			getTarget() const;


	// the following set of routines encapsulate the locked items container.
	// the locked item container is automatically cleared after the phrase terminates.

	// lock an item that is used by, and may be destroyed by, this phrase (eg crafting ingredients, ammo, etc)
	// returns false if the item was already locked by some other process (eg player exchange, bot gift, etc)
	bool lockItem(CLockable* item);

	// unlock an item that was locked by the above routine
	// returns false if the item was not already locked
	bool unlockItem(CLockable* item);

	// destroy all locked items (eg when a crafting action succeeds)
	void destroyLockedItems();

	// release all locked items (eg when crafting action is aborted)
	// this method is called automatically when a phrase has been terminated/ destroyed
	void unlockAllItems();

	// returns the size of the locked items container
	uint32 numLockedItems();

	// get the nth entry from the locked item container
	// returns NULL if 'idx' is >= the container size
	CLockable* getLockedItem(uint32 idx);

	// sets the nth entry in the locked item container
	// if need be the container is resized to accomodate the entry
	// an optional parameter sets the maximum value beyond which the routine asserts
	void setLockedItem(uint32 idx,CLockable* ptr,uint32 maxIdx=128);


private:
	// private method for closing down an active phrase cleanly
	void terminate();

private:
	// phrase state
	TPhraseState			_State;
	// execution end date
	NLMISC::TGameCycle		_NextEventTime;

	// pointer to the phrase model object
	ISabrinaPhraseModelPtr	_ThePhrase;

	// pointer to the actor
	ISabrinaActor*			_Actor;
	// pointer to the target
	ISabrinaActor*			_Target;

	// the vector of locked items (such as crafting materials, ammo, etc)
	// the vector is cleared after the phrase terminates.
	std::vector<CLock>		_Items;
};


//------------------------------------------------------------------------
// CSabrinaPhraseInstance inlines
//------------------------------------------------------------------------

// read accessor - phrase state
inline CSabrinaPhraseInstance::TPhraseState CSabrinaPhraseInstance::getState() const
{
	return _State;
}

// read accessor - execution end date
inline NLMISC::TGameCycle CSabrinaPhraseInstance::getNextEventTime() const
{
	return _NextEventTime;
}

// read accessor - pointer to the phrase model object
inline ISabrinaPhraseModel* CSabrinaPhraseInstance::getPhraseModel() const
{
	return _ThePhrase;
}

// read accessor - pointer to the actor
inline ISabrinaActor* CSabrinaPhraseInstance::getActor() const
{
	return _Actor;
}

// read accessor - pointer to the target
inline ISabrinaActor* CSabrinaPhraseInstance::getTarget() const
{
	return _Target;
}

// the following set of routines encapsulate the locked items container.
// the locked item container is automatically cleared after the phrase terminates.

// lock an item that is used by, and may be destroyed by, this phrase (eg crafting ingredients, ammo, etc)
// returns false if the item was already locked by some other process (eg player exchange, bot gift, etc)
inline bool CSabrinaPhraseInstance::lockItem(CLockable* item)
{
	// make sure the item didn't already exist
	for (uint32 i=_Items.size();i--;)
		if (item==_Items[i])
			return false;
	_Items.resize(_Items.size()+1,NULL);
	_Items[_Items.size()-1]=item;
	return true;
}

// unlock an item that was locked by the above routine
// returns false if the item was not already locked
inline bool CSabrinaPhraseInstance::unlockItem(CLockable* item)
{
	// find the item pointer in the vector
	for (uint32 i=_Items.size();i--;)
		if (item==_Items[i])
		{
			// unlock the item
			_Items[i]=NULL;
			return true;
		} 
	// the item didn't exist in the vector
	return false;
}


// destroy all locked items (eg when a crafting action succeeds)
inline void CSabrinaPhraseInstance::destroyLockedItems()
{
	nlerror("CSabrinaPhraseInstance::destroyLockedItems(): *** Not implemented yet ***");

	for (uint32 i=_Items.size();i--;)
		if (_Items[i]!=NULL)
		{
			// TODO: we need some way of destroying an item - either by calling the item's own 
			// 'destroySelf()' method or by some other means
			// note that a 'destroyed' item may still exist for a short while - it may just be
			// marked for deletion and treated later...

			// clear the lock (if the deletion didn't clear it already)
			// note that this operation is quite safe - if the item has realy been destroyed
			// then _Items[i] is already NULL and the re-assignment is harmless.
			_Items[i]=NULL;
		} 
}

// release all locked items (eg when crafting action is aborted)
// this method is called automatically when a phrase has been terminated/ destroyed
inline void CSabrinaPhraseInstance::unlockAllItems()
{
	_Items.clear();
}

// returns the size of the locked items container
inline uint32 CSabrinaPhraseInstance::numLockedItems()
{
	return _Items.size();
}

// get the nth entry from the locked item container
// returns NULL if 'idx' is >= the container size
inline CLockable* CSabrinaPhraseInstance::getLockedItem(uint32 idx)
{
	if (_Items.size()>idx)
		return _Items[idx];
	return NULL;
}

// sets the nth entry in the locked item container
// if need be the container is resized to accomodate the entry
// an optional parameter sets the maximum value beyond which the routine asserts
inline void CSabrinaPhraseInstance::setLockedItem(uint32 idx,CLockable* ptr,uint32 maxIdx)
{
	#ifdef NL_DEBUG
		nlassert(idx<maxIdx);
	#endif
	if (_Items.size()<=idx)
		_Items.resize(idx,NULL);
	_Items[idx]=ptr;
}


//------------------------------------------------------------------------
#endif
