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




#ifndef CL_STREAMABLE_ENTITY_H
#define CL_STREAMABLE_ENTITY_H

#include "nel/misc/vector.h"
#include "nel/misc/smart_ptr.h"


/** Base interface for streamable entities
  */
struct IStreamableEntity : public NLMISC::CRefCount
{
	/** Given a pos, test whether the entity needs to be loaded now.
	  * It it returns false, it means that the entity is too far or that asynchronous loading suffice.
	  * It it returns true, the next call to update will return only when the loading is completed.
	  */
	virtual bool needCompleteLoading(const NLMISC::CVector &pos) const = 0;
	/** Test that entity against the player position, and load / unload it (using synchronous or asynchronous loading)
	  */
	virtual void		 update(const NLMISC::CVector &pos) = 0;
	// The same as 'update', but force synchronous loading all the time
	virtual void		 forceUpdate(const NLMISC::CVector &pos, class NLMISC::IProgressCallback &progress) = 0;
	/// Force unloading of all entitie
	virtual void		 forceUnload() = 0;
	// dtor
	virtual	~IStreamableEntity() {}
};


/** Streamable entity, which uses distance to a point.
  * Such an entity is streamed when it is near the user, or its loading is forced when it is too near.
  * The derivers may choose what kind of object must be loaded
  */
class CStreamableEntity : public IStreamableEntity
{
public:
	//\name Object
	//@{
		// ctor
		CStreamableEntity();
		// dtor
		virtual ~CStreamableEntity() {}
	//@}

	//\name Attributes
	//@{
		// Get the position of the entity.
		const NLMISC::CVector &getPos() const				{  return _Pos; }
		// Get the radius (in meter) at which async loading must begin.
		float				   getLoadRadius() const		{ return _LoadRadius; }
		// Get the radius (in meter) at which loading is forced (user must wait)
		float				   getForceLoadRadius() const	{ return _ForceLoadRadius; }
		// Get the radius (in meter) at which the entity must be unloaded
		float				   getUnloadRadius() const		{ return _UnloadRadius; }
	//@}

	//\name From IStreamableEntity
	//@{
		/** Given a pos, test whether the entity needs to be loaded now.
		  * It it returns false, it means that the entity is too far or that asynchronous loading suffice.
		  * It it returns true, the next call to update will return only when the loading is completed.
		  */
		virtual bool needCompleteLoading(const NLMISC::CVector &pos) const
		{
			nlassert(_UnloadRadius >= _LoadRadius && _LoadRadius >= _ForceLoadRadius);
			return (pos - _Pos).norm() < _ForceLoadRadius;
		}
		/** Test that entity against the player position, and load / unload it (using synchronous or asynchronous loading)
		  */
		virtual void		 update(const NLMISC::CVector &pos);
		/** The same as 'update', but force synchronous update all the time
		  */
		virtual void		 forceUpdate(const NLMISC::CVector &pos, NLMISC::IProgressCallback &progress);
	//@}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
protected:
	// for derivers : set attributes.
	void					setPos(const NLMISC::CVector &pos)
	{
		_Pos = pos;
	}
	void					setLoadRadius(float loadRadius)
	{
		nlassert(loadRadius >= 0);
		_LoadRadius = loadRadius;
	}
	void					setForceLoadRadius(float forceLoadRadius)
	{
		nlassert(forceLoadRadius >= 0);
		_ForceLoadRadius = forceLoadRadius;
	}
	void					setUnloadRadius(float unloadRadius)
	{
		nlassert(unloadRadius >= 0);
		_UnloadRadius = unloadRadius;
	}

	//\name Derivers methods.
	//@{
		/** Force async loading of this object.
		  * Maybe called even if currently loading async
		  */
		virtual void			loadAsync() = 0;

		/** Force complete loading of that object. If an async loading was started, it must be finished.
		  */
		virtual void			load(NLMISC::IProgressCallback &progress) = 0;

		/** Unload that object.
		  * Maybe be called even if currently loading
		  */
		virtual void			unload() = 0;
	//@}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	NLMISC::CVector	_Pos;
	float			_LoadRadius;
	float			_ForceLoadRadius;
	float			_UnloadRadius;
};

#endif	// CL_STREAMABLE_ENTITY_H
