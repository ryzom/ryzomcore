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



/*
--------------------------------------------------------------------------------------------------------------

  INTRODUCTION
  ------------
  This file provides support for lockable objects and the related lock class.
  It was designed for use with inventory items that need to be locked while in use
  for specific operations. (eg while in use in crafting action, in use in player exchange, etc...)

  BREAKING LOCKS VIOLENTLY
  ------------------------
  It is possible to break the locks on an object when specific game rules demand (eg - if a game rule
  makes a player drop all of the objects that they are carrying, regardless of whether they are in use, 
  in an exchange, etc)
  In the case when a locked object is destroyed the lock is automatically broken.
  The CLock class can be specialised. The virtual cbLockBroken() method can be overloaded to provide the
  means to interrupt the game actions when the locks that they have setup are broken.

  LOCKS AS POINTERS
  -----------------
  CLock objects can be assigned values and de-refferenced as pointers
  CLock objects cannot be copied as it is not legal to have more than 1 lock for the same target object

  * VECTORS, MAPS & OTHER CONTAINERS *
  ------------------------------------
  Because it is illegal to copy a CLock (there is no copy constructor) you cannot use push_back() with a
  vector of CLock objects.

--------------------------------------------------------------------------------------------------------------
  EXAMPLE
  -------

  // A 'lockable' item class

  class CMyItem: public CLockable
  {
	...
  };

  ...
  // A specialisation of 'CLock' for crafting actions 

  class CCraftingLock: public CLock
  {
  public:
	  CCraftingLock(CCraftingAction* action,CLockable* target=NULL): CLock(target)
	  {
	      _TheAction= action;
	  }

  protected:
	  void cbLockBroken()
	  {
		  _TheAction->cancel();
	  }

  private:	
	  CCraftingAction* _TheAction;
  }

  ...
  // an object class that represents an on-going crafting action

  class CCraftingAction
  {
      // constructor - initialise the _CraftingTool object with a pointer to self
	  CCraftingAction(...): _CraftingTool(this)...
	  {
	  ...
	  }
	  ...
	  // setup the crafting tool
	  void setTool(CMyItem* tool)
	  {
	      _CraftingTool=tool;
	  }

	  // cancel the action
	  void cancel()
	  {
	      _CraftingTool=NULL;
		  _RawMaterials.clear();
	  }

	  ...
	  // the pointer to the crafting tool
	  CCraftingLock _CraftingTool;

	  // the vector of pointers to the raw materials
	  vector<CCraftingLock> _RawMaterials;
  };

--------------------------------------------------------------------------------------------------------------
*/

#ifndef RY_LOCKABLE_H
#define RY_LOCKABLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"


//------------------------------------------------------------------------
// Advanced class declarations
//------------------------------------------------------------------------

class CLock;
class CLockable;


//------------------------------------------------------------------------
// CLockable class
//------------------------------------------------------------------------

class CLockable 
{
public:
	// constructor - items are never locked at creation
	CLockable();

	// copy constructor
	// copy is legal because derived objects may be legaly copied
	// - copies of locked objects should not be locked themselves...
	CLockable(const CLockable&);

	// destructor - make sure that locks are broken cleanly when object is destroyed
	~CLockable();

	// query the state of the object (locked or not)
	bool isLocked()	const;

	// break a lock cleanly, informing locker that the lock has been broken
	void breakLock();

	// read accessors - primariyl for use in debug routines
	const CLock* getLock() const;
	CLock* getLock();

private:
	// the private data is manipulated by CLock objects
	friend CLock;
	CLock* _Lock;
};


//------------------------------------------------------------------------
// CLock class
//------------------------------------------------------------------------

class CLock
{
public:
	// constructor - construct with pointer to object to lock (NULL by default)
	CLock(CLockable* target=NULL);

	// destructor - mark the target as unlocked
	virtual ~CLock();

	// lock an object by assigning it to an CLock object
	CLock& operator=(CLockable* target);

	// read accessors (via operators)
	operator CLockable*();
	CLockable* operator->();

	// *** THIS ROUTINE IS ONLY ALLOWED FOR STL CONTAINER COMPATIBILITY
	// *** COPYING A CLock OBJECT IS STRICTLY FORBIDDEN (unless the value is NULL)
	// - it is not legal to have 2 locks refferencing the same object so you can't copy a lock
	CLock(const CLock& other)
	{
		nlassert(other._Target==NULL);
		_Target=NULL;
	}

private:
	// handy private method for performing 'lock' operation
	void _lock(CLockable* target);

	// handy private method for performing 'unlock' operation
	void _unlock();

protected:
	// private interface for CLockable (partner class)
	friend CLockable;

	// routine called by targeted object when the lock is broken violently
	// this is virtual so it can be overloaded in specialisations of CLock
	// the _Target property is valid when this routine is called.
	// _Target is set to NULL by caller after this routine returns.
	virtual void cbLockBroken();

private:
	// private data...
	CLockable* _Target;
};


//------------------------------------------------------------------------
// CLockable inlines
//------------------------------------------------------------------------

// constructor - items are never locked at creation
inline CLockable::CLockable()
{
	_Lock=NULL;
}

// copy constructor
// copy is legal because derived objects may be legaly copied
// - copies of locked objects should not be locked themselves...
inline CLockable::CLockable(const CLockable&)
{
	_Lock=NULL;
}

// destructor - make sure that locks are broken cleanly when object is destroyed
inline CLockable::~CLockable()
{
	breakLock();
}

// query the state of the object (locked or not)
inline bool CLockable::isLocked()	const
{
	return _Lock!=NULL;
}

// break a lock cleanly, informing locker that the lock has been broken
inline void CLockable::breakLock()
{
	if (_Lock!=NULL)
	{
		nlassert(_Lock->_Target==this);
		// inform the locker that the lock has been violently broken
		_Lock->cbLockBroken();
		// have the locker clear the lock cleanly
		(*_Lock)=NULL;
	}
	_Lock=NULL;
}

// read accessors - primariyl for use in debug routines
inline const CLock* CLockable::getLock() const
{
	return _Lock;
}
inline CLock* CLockable::getLock()
{
	return _Lock;
}


//------------------------------------------------------------------------
// CLock class
//------------------------------------------------------------------------

// constructor - construct with pointer to object to lock (NULL by default)
inline CLock::CLock(CLockable* target)
{
	_lock(target);
}

// destructor - mark the target as unlocked
inline CLock::~CLock()
{
	_unlock();
}

// lock an object by assigning it to an CLock object
inline CLock& CLock::operator=(CLockable* target)
{
	// if the new target and old target are the same then there's nothing to do...
	if (_Target==target)
		return *this;

	// if the lock had a different target previously then unlock the previous target
	_unlock();

	// assign the new target to the _Target property
	_lock(target);

	return *this;
}

// read accessors
inline CLock::operator CLockable*()
{
	return _Target;
}

inline CLockable* CLock::operator->()
{
	return _Target;
}

// handy private method for performing 'lock' operation
inline void CLock::_lock(CLockable* target)
{
	_Target= target;

	// if the new target is not null then inform the targeted object that it is now locked
	if (_Target!=NULL)
	{
		nlassert(_Target->_Lock==NULL);
		_Target->_Lock= this;
	}
}

// handy private method for performing 'unlock' operation
inline void CLock::_unlock()
{
	if (_Target!=NULL)
	{
		nlassert(_Target->_Lock==this);
		_Target->_Lock=NULL;
	}
	_Target=NULL;
}

// default callback does nothing...
inline void CLock::cbLockBroken()
{
}


//------------------------------------------------------------------------
#endif
