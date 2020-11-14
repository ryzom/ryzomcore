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



#ifndef RY_GPM_UTILITIES_H
#define RY_GPM_UTILITIES_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/block_memory.h"
#include "nel/pacs/u_move_container.h"
#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_collision_desc.h"
#include "nel/pacs/u_global_retriever.h"
#include "nel/net/message.h"

#include "game_share/ryzom_entity_id.h"
#include "game_share/player_vision_delta.h"

#include <map>
#include <set>
#include <vector>
#include <list>

class CCell;
class CWorldEntity;
class CWorldPositionManager;
class ConstIteratorType;
class CPlayerInfos;


/*
 * Misc. data structures (essentially vision data)
 */

/**
 * Front end vision data to be sent
 */
class CFrontEndData
{
public:
	CFrontEndData() :	Message("",false),
						MessageHeaderSize(0),
						NumPlayers(0),
						CurrentVisionsAtTick(0),
						MaxVisionsPerTick(0),
						VisionIn(0),
						VisionOut(0),
						VisionReplace(0)
	{
	}

	NLNET::CMessage													Message;
	sint32															MessageHeaderSize;
	sint32															NumPlayers;
	sint32															CurrentVisionsAtTick;
	sint32															MaxVisionsPerTick;
	sint32															VisionIn;
	sint32															VisionOut;
	sint32															VisionReplace;
};

/**
 * Entities around vision data to be sent
 */
struct CServiceData
{
	CServiceData() : Message("",false) {} // always an output message
	NLNET::CMessage													Message;
	sint32															MessageHeaderSize;
};

typedef std::map<NLNET::TServiceId, CFrontEndData>					TMapFrontEndData;
typedef std::map<NLNET::TServiceId, CServiceData>					TMapServiceData;

typedef std::list< CPlayerInfos* >									TPlayerList;


/*
 * Utitility classes
 */

/**
 * A list of object that must have Next and Previous pointers
 * \param T the type of item
 * \param TPtr a pointer to T to use in list (useful for smartpointer)
 */
template<class T, class TPtr = T*>
class CObjectList
{
public:
	TPtr	Head;
	TPtr	Tail;

	CObjectList() : Head(NULL), Tail(NULL) {}

	void	insertAtHead(T *object)
	{
		nlassert(object->Next == NULL);
		nlassert(object->Previous == NULL);

		object->Next = Head;
		if (object->Next != NULL)
			object->Next->Previous = object;
		Head = object;
	}

	void	insertAtTail(T *object)
	{
		nlassert(object->Next == NULL);
		nlassert(object->Previous == NULL);

		object->Previous = Tail;
		if (object->Previous != NULL)
			object->Previous->Next = object;
		Tail = object;
	}

	void	remove(T *object)
	{
		// if object at head
		if (object->Previous == NULL)
			Head = object->Next;
		else
			object->Previous->Next = object->Next;

		// if object at tail
		if (object->Next == NULL)
			Tail = object->Previous;
		else
			object->Next->Previous = object->Previous;

		object->Previous = NULL;
		object->Next = NULL;
	}

	T			*getHead() { return (T*)Head; }
	T			*getTail() { return (T*)Tail; }
};

/**
 * A little stack implementation, for really fast push_back/pop_back
 */
template<class T, uint stackSize>
class CUnsafeConstantSizeStack
{
private:
	T			_Array[stackSize];
	T			*_Top;

public:
	CUnsafeConstantSizeStack()				{ _Top = _Array; }
	
	void		push_back(const T &o = T())	{ *(_Top++) = o; }
	void		pop_back()					{ nlassert(_Top > _Array); --_Top; }
	//T			&front()					{ return _Array[0]; }
	T			&back()						{ nlassert(_Top > _Array); return *(_Top-1); }
	uint		size()						{ return (uint)(_Top-_Array); }
	bool		empty()						{ return _Top == _Array; }
	void		clear()						{ _Top = _Array; }
};

/**
 * The same class, but uses stl vectors instead, and should be saffer
 */
template<class T, uint stackSize>
class CSafeConstantSizeStack : public std::vector<T>
{
public:
	CSafeConstantSizeStack()			{ this->reserve(stackSize); }
};




/**
 * Simple smart pointers, doesn't delete object when no reference
 * Instanciation class T must have a RefCounter attribute
 */
template<class T>
class CSimpleSmartPointer
{
private:
	T	*_Ptr;

public:
	CSimpleSmartPointer() : _Ptr(NULL) {}
	CSimpleSmartPointer(T* ptr) : _Ptr(NULL) { *this = ptr; }
	CSimpleSmartPointer(const CSimpleSmartPointer &ptr) : _Ptr(NULL) { *this = ptr; }
	~CSimpleSmartPointer() { *this = NULL; }

	T		* operator = (T *ptr)
	{
		if (ptr != NULL)
			++(ptr->RefCounter);
		if (_Ptr != NULL)
		{
			nlassert(_Ptr->RefCounter > 0);
			--(_Ptr->RefCounter);
		}
		_Ptr = ptr;
		return _Ptr;
	}

	T		* operator = (const CSimpleSmartPointer &ptr)
	{
		*this = ptr._Ptr;
		return _Ptr;
	}

	T		& operator * ()										{ return *_Ptr; }
	T		* operator -> ()									{ return _Ptr; }
	const T	& operator * () const								{ return *_Ptr; }
	const T	* operator -> () const								{ return _Ptr; }
	bool	operator == (const T *ptr) const					{ return ptr == _Ptr; }
	bool	operator != (const T *ptr) const					{ return ptr != _Ptr; }
	bool	operator == (const CSimpleSmartPointer &ptr) const	{ return ptr._Ptr == _Ptr; }
	bool	operator != (const CSimpleSmartPointer &ptr) const	{ return ptr._Ptr != _Ptr; }
	operator T* () const										{ return _Ptr; }
};



#endif // RY_GPM_UTILITIES_H

/* End of gpm_utilities.h */
