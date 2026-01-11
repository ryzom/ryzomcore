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



#ifndef CL_STREAMABLE_ENTITY_COMPOSITE
#define CL_STREAMABLE_ENTITY_COMPOSITE

#include "streamable_entity.h"
#include <vector>

/** A group of streamable entity
  */
class CStreamableEntityComposite : public IStreamableEntity
{
public:
	// dtor
	~CStreamableEntityComposite();
	/// Add an entity. it is then owned by this obj.
	void			  add(IStreamableEntity *entity);
	/// optimisation : make room for further adds
	void			  reserve(uint size);
	/// Remove an entity (& delete it)
	void			  remove(IStreamableEntity *entity);
	/// Remove all entities (& elte them)
	void			  removeAll();
	/// Get the number of entity
	uint			  getNumEntities() const { return (uint)_Entities.size(); }
	/// Get an entity by its index
	IStreamableEntity *getEntity(uint index) const { return _Entities[index]; }
	//\name from IStreamableEntity
	//@{
		/** Given a pos, test whether one entity needs to be loaded now.
		  * It it returns true, the next call to update will return only when the loading of an entity is completed.
		  */
		virtual bool		needCompleteLoading(const NLMISC::CVector &pos) const;
		/// Load / Unload entity depeneding on the player position
		virtual void		 update(const NLMISC::CVector &pos);
		virtual	void		 forceUpdate(const NLMISC::CVector &pos, NLMISC::IProgressCallback &progress);
		/// Force unloading of all entitie
		virtual void		 forceUnload();
	//@}
private:
	typedef std::vector<IStreamableEntity *>	TStreambleEntities;
	TStreambleEntities	_Entities;
};








#endif
