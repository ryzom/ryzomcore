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



#ifndef NL_ANIMAL_POSITION_STATE_H
#define NL_ANIMAL_POSITION_STATE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/class_registry.h"
#include "game_share/entity_types.h"


// ***************************************************************************
namespace NLMISC{
class CCDBNodeLeaf;
}

// ***************************************************************************
/**
 * A class used to retrieve position of some entities like animals or team mates
 *	The best position is returned: if entity in vision, its pos is returned, else take from Database
 * \author Matthieu Besson
 * \author Nevrax France
 * \date February 2003
 */
class CPositionState : public NLMISC::CRefCount, public NLMISC::IStreamable
{
public:
	virtual ~CPositionState() {}


	/** From DB/EntityMngr, get the accurate entity position (*1000).
	 *	0 and return false, if error or if the animal no more present
	 */
	bool			getPos(sint32 &px, sint32 &py);

	// serial supported ?
	virtual bool canSave() const = 0;

protected:
	virtual bool		 dbOk() = 0;
	// if an entity is available, get it for more precise localisation
	virtual class CEntityCL	*getEntity() = 0;
	// if no entity is available, then can retrieve less precise position from database
	virtual bool		 getDbPos(sint32 &px, sint32 &py) = 0;

	// helper to serial a CDBNodeLeaf, based on itsname
	void serialNodeLeaf(NLMISC::IStream &f, NLMISC::CCDBNodeLeaf *&dbNode);

};

// ***************************************************************************
/**
 * Implementation of CPositionState for an entity that is identified by its compressed uid
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2004
 */
class CUIDEntityPositionState : public CPositionState
{
public:
	CUIDEntityPositionState();
	virtual bool	dbOk() {return _DBPos && _Uid;}
	/// get the DB states related to the animal
	void			build(const std::string &baseDB);
	// try to retrieve current tracked pos
	bool			getPos(sint32 &px, sint32 &py);
	//
	virtual			void serial(NLMISC::IStream &f);
protected:
	// Database infos
	NLMISC::CCDBNodeLeaf				*_DBPos;
	NLMISC::CCDBNodeLeaf				*_Uid;
	// The slot of the entity that may be used to get more precise position
	CLFECOMMON::TCLEntityId		_EntitySlot;
	virtual CEntityCL	*getEntity();
	virtual bool		 getDbPos(sint32 &px, sint32 &py);
};


// ***************************************************************************
/**
 * Implementation of CPositionState for an entity that is identified by its name (encoded as a dynamic string id in db)
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2004
 */
class CNamedEntityPositionState : public CPositionState
{
public:
	NLMISC_DECLARE_CLASS(CNamedEntityPositionState)
	virtual bool	dbOk() {return _Name && _X && _Y;}
	void			build(NLMISC::CCDBNodeLeaf *name, NLMISC::CCDBNodeLeaf *x, NLMISC::CCDBNodeLeaf *y);
	NLMISC::CCDBNodeLeaf	*getNameNode() const { return _Name; }
	NLMISC::CCDBNodeLeaf	*getXNode() const { return _X; }
	NLMISC::CCDBNodeLeaf	*getYNode() const { return _X; }
	//
	virtual bool	canSave() const { return true; }
	virtual	void	serial(NLMISC::IStream &f);
protected:
	// Database infos
	NLMISC::CCDBNodeLeaf				*_Name;
	NLMISC::CCDBNodeLeaf				*_X;
	NLMISC::CCDBNodeLeaf				*_Y;
	virtual CEntityCL	*getEntity();
	virtual bool		 getDbPos(sint32 &px, sint32 &py);
};


// ***************************************************************************
/**
 * A class used to retrieve position of some entities like animals or team mates
 *	The best position is returned: if entity in vision, its pos is returned, else take from Database
 * \author Matthieu Besson
 * \author Nevrax France
 * \date February 2003
 */
class CTeammatePositionState : public CUIDEntityPositionState
{
public:
	NLMISC_DECLARE_CLASS(CTeammatePositionState)
	/// Constructor
	CTeammatePositionState();

	/// get the DB states related to the animal
	void			build(const std::string &baseDB);

	/** From DB/EntityMngr, get the accurate entity position (*1000).
	 *	0 and return false, if error or if the animal no more present
	 */
	bool			getPos(sint32 &px, sint32 &py);

	virtual bool	canSave() const { return false; }

	virtual	void	serial(NLMISC::IStream &/* f */) { nlassert(0); /* notsavable */ }
protected:
	// Database infos
	NLMISC::CCDBNodeLeaf				*_Present;
	// DB ok.
	bool						dbOk() {return _DBPos && _Present && _Uid;}

};

// ***************************************************************************
/**
 * A class used to retrieve position of the Steed or of a PackAnimal.
 *	The best position is returned: if entity in vision, its pos is returned, else take from Database
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2003
 */
class CAnimalPositionState : public CUIDEntityPositionState
{
public:
	NLMISC_DECLARE_CLASS(CAnimalPositionState)
	/// Constructor
	CAnimalPositionState();

	/// get the DB states related to the animal
	void			build(const std::string &baseDB);

	/** From DB/EntityMngr, get the accurate entity position (*1000).
	 *	0 and return false, if error or if the animal no more present
	 */
	bool			getPos(sint32 &px, sint32 &py);

	virtual bool	canSave() const { return true; }
	virtual	void	serial(NLMISC::IStream &f);

private:
	// Animal Database infos
	NLMISC::CCDBNodeLeaf				*_Status;
	// DB ok.
	bool						dbOk() {return _DBPos && _Status && _Uid;}

};


// ***************************************************************************
/**
 * A class used to retrieve position of some entities sent by server as "compass dialog"
 *	The best position is returned: if entity in vision, its pos is returned, else take from Database
 * \author Matthieu Besson
 * \author Nevrax France
 * \date February 2003
 */
class CDialogEntityPositionState : public CPositionState
{
public:
	NLMISC_DECLARE_CLASS(CDialogEntityPositionState)
	CDialogEntityPositionState(uint dialogIndex = 0)
		:CPositionState(),_DialogIndex(dialogIndex){}
	virtual bool	dbOk() {return true;}

	virtual bool canSave() const { return false; }

	virtual	void	serial(NLMISC::IStream &/* f */) { nlassert(0); /* notsavable */ }

protected:
	uint				_DialogIndex;
	virtual CEntityCL	*getEntity();
	virtual bool		 getDbPos(sint32 &px, sint32 &py);
};
#endif // NL_ANIMAL_POSITION_STATE_H

/* End of animal_position_state.h */
