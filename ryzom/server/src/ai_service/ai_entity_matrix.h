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



#ifndef RYAI_ENTITY_MATRIX_H
#define RYAI_ENTITY_MATRIX_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/command.h"

#include <map>
#include <vector>
#include <string>

#include "ai_share/ai_entity_physical_list_link.h"
#include "ai_pos.h"

//--------------------------------------------------------------------------
// The CAIEntityMatrixIteratorTblLinear class
//--------------------------------------------------------------------------
// This class uses a vector of horizontal run lengths to represent
// the aea to be scanned. 
// The form represented by the data is assumed to be symetrical in both x and y.
// Linear tables use a better RAM access patern than random access tables
// and should be used whenever possible

class CAIEntityMatrixIteratorTblLinear
{
public:
	struct STblEntry
	{
		STblEntry()						 {}
		STblEntry(uint32 runLength, sint32 previousRunLength)	
		{
			RunLength=(uint16)(runLength-1);	// we assume all runs are at least 1 unit long - this is the excess
			StartDx=(sint16)(-(previousRunLength/2)-(((sint32)runLength)/2));
			
			#ifdef NL_DEBUG
			nlassert(runLength>0);			// runs must be at least 1 unit long
			nlassert(runLength<32768);		// this is the limit where StartDx runs out of bits
			nlassert(previousRunLength>=0);
			nlassert(previousRunLength<32768);
			#endif
		}

		uint16 RunLength;
		sint16 StartDx;
	};
	typedef std::vector<STblEntry> TTbl;
	typedef TTbl::iterator iterator;

	inline CAIEntityMatrixIteratorTblLinear();
	inline CAIEntityMatrixIteratorTblLinear(uint32 *runLengths,uint32 count);

	inline void push_back(uint32 runLength, sint32 previousRunLength);
	inline const STblEntry &operator[](uint32 idx) const;
	inline const iterator begin() const;
	inline const iterator end() const;
	inline uint32 size() const;

private:
	TTbl _tbl;
};


//--------------------------------------------------------------------------
// The CAIEntityMatrixIteratorTblRandom class
//--------------------------------------------------------------------------
// This class uses a vector of xy offsets to apply to the 'read position' ptr
// as it iterates across the grid.
// The first xy offset is applied at initalisation time in order to allow for
// the use of tables that do not scan the '0,0' cell.
// These tables use a less efficient RAM access pattern than Linear tables but
// benefit from a higher degree of control over the area scanned and the order
// of scanning. 

class CAIEntityMatrixIteratorTblRandom
{
public:
	struct STblEntry
	{
		STblEntry()					 {}
		STblEntry(sint32 dxdy)		 { dxdy=dxdy; }
		STblEntry(sint16 dx, sint16 dy) { dx=dx; dy=dy; }
		union
		{
			struct { sint16 dx, dy; };
			uint32 dxdy;
		};
	};
	typedef std::vector<STblEntry> TTbl;
	typedef TTbl::iterator iterator;

	inline CAIEntityMatrixIteratorTblRandom();

	inline void push_back(sint16 dx,sint16 dy);
	inline const STblEntry &operator[](uint32 idx) const;
	inline const iterator begin() const;
	inline const iterator end() const;
	inline uint32 size() const;

private:
	TTbl _tbl;
};


//--------------------------------------------------------------------------
// The CAIEntityMatrix class
//--------------------------------------------------------------------------

template	<class	T>
class CAIEntityMatrix :
	public NLMISC::CDbgRefCount<CAIEntityMatrix<T> >
{
public:
	/*
	-----------------------------------------------------------------------------------------------------
	class CAIEntityMatrix<T>::CCellTblIteratorRandom

	This class provides an iterator for iterating across the cells of a matrix '_matrix' following the 
	pattern described by a cell iteration table '_tbl'

	An iterator '_it' refferences the current position in '_tbl'

	A compact coordinate pair '_xy'	reffernces the current position in '_matrix'
    Note that because the matrix super imposes coordinates with the same low byte, the upper bytes of 
	the '_xy' coordinates is needed for position proximity tests
	-----------------------------------------------------------------------------------------------------
	*/
	class CCellTblIteratorRandom
	{
	public:

		inline CCellTblIteratorRandom():
		_matrix(NULL), _tbl(NULL), _x(0), _y(0) 
		{
		}
		
		inline CCellTblIteratorRandom(const CAIEntityMatrix<T> *matrix,const CAIEntityMatrixIteratorTblRandom *tbl,const	CAIVector	&pos):
		_matrix(matrix), _tbl(tbl), _x((uint16)pos.x().asInt16Meters()), _y((uint16)pos.y().asInt16Meters()) 
		{
			_it=_tbl->begin();
			// apply the first entry in the iterator table in order to setup the start position correctly
			++*this;
		}
		
		inline const CEntityListLink<T>	* operator*() 	const
		{
		  const typename CAIEntityMatrix<T>::SMatrixLine &line =(*_matrix)[_yl];
		  const CEntityListLink<T> &cellEntityListLink=line[_xl];
		  return &cellEntityListLink;
		}
		
		inline const CCellTblIteratorRandom &operator++() 
		{
#ifdef NL_DEBUG
			// make sure we aren't trying to access an uninitialised iterator
			nlassert(_it!=_tbl->end());
#endif
			
			_x+=(*_it).dx;
			_y+=(*_it).dy;
			++_it;
			
			return *this;
		}
				
		inline const CCellTblIteratorRandom &operator=(const CCellTblIteratorRandom &other)
		{
			_xy=	 other._xy;
			_matrix= other._matrix;
			_tbl=	 other._tbl;
			_it=	 other._it;
			
			return *this;
		}
		
		inline bool end()	const
		{
			return _it==_tbl->end();
		}
		
		inline const	uint32 &xy()	const	{ return _xy; }
		inline const	uint16 &x()		const	{ return  _x; }
		inline const	uint16 &y()		const	{ return  _y; }
		inline const	uint8  &xl()	const	{ return _xl; }
		inline const	uint8  &xh()	const	{ return _xh; }
		inline const	uint8  &yl()	const	{ return _yl; }
		inline const	uint8  &yh()	const	{ return _yh; }

		inline const CAIEntityMatrix<T> *matrix()	const
		{
			return _matrix;
		}

	private:
		union
		{
			uint32 _xy;
			struct { uint16 _x, _y;	};
			struct { uint8 _xl, _xh, _yl, _yh; };
		};
		const CAIEntityMatrix<T> *_matrix;
		const CAIEntityMatrixIteratorTblRandom *_tbl;
		CAIEntityMatrixIteratorTblRandom::iterator _it;
	};

	/*
	-----------------------------------------------------------------------------------------------------
	class CAIEntityMatrix<T>::CCellTblIteratorLinear

	This class provides an iterator for iterating across the cells of a matrix '_matrix' following the 
	pattern described by a cell iteration table '_tbl'

	An iterator '_it' refferences the current position in '_tbl'
	The _runLengthRemaining property is setup each time the iterator _it is advanced by the ++operator.
	Subsequent calls to operator++() will only increment _it if _runLengthRemaining==0

	A compact coordinate pair '_xy'	reffernces the current position in '_matrix'
    Note that because the matrix super imposes coordinates with the same low byte, the upper bytes of 
	the '_xy' coordinates is needed for position proximity tests
	-----------------------------------------------------------------------------------------------------
	*/
	class CCellTblIteratorLinear
	{
	public:
		inline CCellTblIteratorLinear():
		_x(0), _y(0), _matrix(NULL), _tbl(NULL), _runLengthRemaining(0) 
		{
		}
		
		inline CCellTblIteratorLinear(const CAIEntityMatrix<T> *matrix,const CAIEntityMatrixIteratorTblLinear *tbl,const	CAIVector	&pos):
		_x((uint16)pos.x().asInt16Meters()), _y((uint16)pos.y().asInt16Meters()), _matrix(matrix), _tbl(tbl) 
		{
#ifdef NL_DEBUG
			nlassert(_tbl!=NULL);
			nlassert(_tbl->size()>0);
			nlassert(_tbl->size()<32768);	// a numeric over-run limit
			nlassert(_matrix!=NULL);
#endif
			
			// setup the iterator to point to the strat of the iterator table and setup properties accordingly
			_it=_tbl->begin();
			_runLengthRemaining=(*_tbl)[0].RunLength;
			_x-=(sint16)(_runLengthRemaining/2);
			_y-=(sint16)(_tbl->size()/2);
		}
		
		inline const CEntityListLink<T> *operator*()	const
		{
			return &(*_matrix)[_yl][_xl];	
		}
		
		inline const CCellTblIteratorLinear &operator++() 
		{
#ifdef NL_DEBUG
			// make sure we aren't trying to access an uninitialised iterator
			nlassert(_it!=_tbl->end());
#endif
			
			// if we're not at the end of the current run continue run else move on to next line
			if (_runLengthRemaining!=0)
			{
				--_runLengthRemaining;
				++_x;
			}
			else
			{
				++_it;
				if (_it==_tbl->end())
					return *this;
					
				_runLengthRemaining=(*_it).RunLength;
				_x+=(*_it).StartDx;
				++_y;
			}
			
			return *this;
		}
		
		inline const CCellTblIteratorLinear &operator=(const CCellTblIteratorLinear &other)
		{
			_xy=	 other._xy;
			_matrix= other._matrix;
			_tbl=	 other._tbl;
			_it=	 other._it;
			_runLengthRemaining= other._runLengthRemaining;
			
			return *this;
		}
		
		inline bool end()	const
		{
			return _it==_tbl->end();
		}
		
		inline const uint32 &xy()	const	{ return _xy; }
		inline const uint16 &x()	const	{ return  _x; }
		inline const uint16 &y()	const	{ return  _y; }
		inline const uint8  &xl()	const	{ return _xl; }
		inline const uint8  &xh()	const	{ return _xh; }
		inline const uint8  &yl()	const	{ return _yl; }
		inline const uint8  &yh()	const	{ return _yh; }

		inline const CAIEntityMatrix<T> *matrix()	const
		{
			return _matrix;
		}

	private:
		union
		{
			uint32 _xy;
			struct { uint16 _x, _y;	};
			struct { uint8 _xl, _xh, _yl, _yh; };
		};
		const CAIEntityMatrix<T> *_matrix;
		const CAIEntityMatrixIteratorTblLinear *_tbl;
		CAIEntityMatrixIteratorTblLinear::iterator _it;
		uint32 _runLengthRemaining;
	};

	/*
	-----------------------------------------------------------------------------------------------------
	class CAIEntityMatrix<T>::CEntityIteratorTemplate
	class CAIEntityMatrix<T>::CEntityIteratorLinear
	class CAIEntityMatrix<T>::CEntityIteratorRandom

	This class provides an iterator for iterating across the entities listed in the cells of a matrix 
	'_matrix' following the pattern described by a cell iteration table '_tbl'

    The class is composed of a CCellTblIteratorLinear '_cellIt' responsible for iterating across the matrix
	and an entity pointer '_entity' used for iterating over the entities in each matrix cell

	Note that unlinking, moving or deleting the entity refferenced by a CEntityIteratorLinear iterator
	invalidates it.
	-----------------------------------------------------------------------------------------------------
	*/

	template <class CIt,class CTbl>
	class CEntityIteratorTemplate
	{
	public:
		
		inline CEntityIteratorTemplate() : _cellIt()
		{
			_entity=NULL;
		}
		
		inline CEntityIteratorTemplate(const CAIEntityMatrix<T> *matrix,const CTbl *tbl,const CAIVector &pos) : _cellIt(matrix,tbl,pos)
		{
			// get a pointer to the list link for the first entity in the cell
//			_entity=(*_cellIt)->next();
			
			_entity=(*_cellIt);
			++*this;

			// if the cell is empty then start iterating to find a non-empty cell
//			if (_entity->unlinked())
//				++*this;
		}
		
		inline CEntityIteratorTemplate(const CAIEntityMatrix<T> *matrix,const CTbl *tbl,const T *entity) : _cellIt(matrix,tbl,entity->pos())
		{
			// get a poniter to the list link for the first entity in the same cell as the given entity
//			_entity=(*_cellIt)->next();
			
			_entity=(*_cellIt);
			++*this;
		}
		
		inline T	&operator*()
		{
			#ifdef NL_DEBUG
				// make sure we aren't trying to access passed the end of list
				nlassert(!end());
			#endif

			return *NLMISC::safe_cast<T *>(_entity->entity());
		}
		
		inline	const	CEntityIteratorTemplate	&operator++()
		{
#ifdef NL_DEBUG
			
			// if you are on a breakpoint here it is because you've tried to do a ++ on an iterator
			// that has reached its end marker
			nlassert(!_cellIt.end());
			
			// make sure the entity is in the cell that the iterator reffers to
			// if you are on a breakpoint here it is because you've moved or removed an entity
			// and have invalidated this iterator
			//		nlassert(_entity->xyCellAsInt()==_cellIt.xy());
#endif			
			// repeat the following loop until either we come to the end of the cell iterator or we find an entity with
			// xy coords that match our own
			do
			{
				// try to get the next entity in the cell
				_entity=_entity->next();
				if (_entity==*_cellIt)
				{
					// we're at the end of the entity list for this cell so try to find another cell with a valid entity
					do
					{
						++_cellIt;
					}
					while	(	!_cellIt.end()
							&& (*_cellIt)->unlinked());

					if (_cellIt.end())
						return *this;
					_entity=(*_cellIt)->next();
				}

			}
			while	(	!_cellIt.end()
					&&	(	_entity->unlinked()
						||	!_entity->entity()->isAt16MetersPos(_cellIt.x(),_cellIt.y()))	);
			//	as cells are tiled over the world, we need to check if our entity have a real good match with the scanned position.
			return	*this;
		}
		
		inline const CEntityIteratorTemplate<CIt,CTbl>	&operator=(const CEntityIteratorTemplate<CIt,CTbl> & other)
		{
			_cellIt= other._cellIt;
			_entity= other._entity;
			return *this;
		}
		
		// method for testing iterator for end of current sequence
		inline bool end()
		{
			// the following can only happen if there are no more entites in cell and no more deltas in cell iterator tbl
			return	_cellIt.end();
		}
				
	private:
		CIt	_cellIt;		// the cell iterator
		const CEntityListLink<T>	*_entity;		// which entity are we pointing at (within the cell)
	};

	typedef CEntityIteratorTemplate<CCellTblIteratorLinear,CAIEntityMatrixIteratorTblLinear> CEntityIteratorLinear;
	typedef CEntityIteratorTemplate<CCellTblIteratorRandom,CAIEntityMatrixIteratorTblRandom> CEntityIteratorRandom;


	/*
	-----------------------------------------------------------------------------------------------------
	class CAIEntityMatrix<T>::SMatrixLine

	The class used to represent a matrix row
	-----------------------------------------------------------------------------------------------------
	*/

	struct	SMatrixLine
	{
		// data
		CEntityListLink<T> Line[256];

		// one and only accessor
		inline CEntityListLink<T> &operator[](uint8 x) const { return const_cast<CEntityListLink<T> &>(Line[x]); }
	};

	/*
	-----------------------------------------------------------------------------------------------------
	class CAIEntityMatrix methods
	-----------------------------------------------------------------------------------------------------
	*/

public:
	// ctor
	inline CAIEntityMatrix();

	// table lookup operator - should be used as myMatrix[y][x]
	inline SMatrixLine &operator[](uint8 y) const;

	// methods for initialising iterators
	inline const typename CAIEntityMatrix<T>::CCellTblIteratorRandom beginCells(const CAIEntityMatrixIteratorTblRandom *tbl,const	CAIVector	&pos) const;
	inline const typename CAIEntityMatrix<T>::CEntityIteratorRandom beginEntities(const CAIEntityMatrixIteratorTblRandom *tbl,const	CAIVector	&pos) const;

	inline const typename CAIEntityMatrix<T>::CCellTblIteratorLinear beginCells(const CAIEntityMatrixIteratorTblLinear *tbl,const	CAIVector	&pos) const;
	inline const typename CAIEntityMatrix<T>::CEntityIteratorLinear beginEntities(const CAIEntityMatrixIteratorTblLinear *tbl,const	CAIVector	&pos) const;


private:
	/*
	-----------------------------------------------------------------------------------------------------
	class CAIEntityMatrix data
	-----------------------------------------------------------------------------------------------------
	*/

	SMatrixLine _matrix[256];
};

//--------------------------------------------------------------------------
// The CAIEntityMatrix inlines
//--------------------------------------------------------------------------
template	<class T>
inline CAIEntityMatrix<T>::CAIEntityMatrix() 
{
}

template	<class T>
inline typename CAIEntityMatrix<T>::SMatrixLine &CAIEntityMatrix<T>::operator[](uint8 y) const
{
	return const_cast<typename CAIEntityMatrix<T>::SMatrixLine &>(_matrix[y]);
}

template	<class T>
inline const typename CAIEntityMatrix<T>::CCellTblIteratorRandom CAIEntityMatrix<T>::beginCells(const CAIEntityMatrixIteratorTblRandom *tbl,	const	CAIVector	&pos) const
{
	return CCellTblIteratorRandom(this,tbl,pos);
}

template	<class T>
inline const typename CAIEntityMatrix<T>::CEntityIteratorRandom CAIEntityMatrix<T>::beginEntities(const CAIEntityMatrixIteratorTblRandom *tbl,	const	CAIVector	&pos) const
{
	CEntityIteratorRandom newIt(this,tbl,pos);
	return newIt;
}

template	<class T>
inline const typename CAIEntityMatrix<T>::CCellTblIteratorLinear CAIEntityMatrix<T>::beginCells(const CAIEntityMatrixIteratorTblLinear *tbl,	const	CAIVector	&pos) const
{
	return CCellTblIteratorLinear(this,tbl,pos);
}

template	<class T>
inline const typename CAIEntityMatrix<T>::CEntityIteratorLinear CAIEntityMatrix<T>::beginEntities(const CAIEntityMatrixIteratorTblLinear *tbl,	const	CAIVector	&pos) const
{
	CEntityIteratorLinear newIt(this,tbl,pos);
	return newIt;
}


//--------------------------------------------------------------------------
// The CAIEntityMatrixIteratorTblRandom inlines
//--------------------------------------------------------------------------

inline CAIEntityMatrixIteratorTblRandom::CAIEntityMatrixIteratorTblRandom()	
{ 
}

inline void CAIEntityMatrixIteratorTblRandom::push_back(sint16 dx,sint16 dy)	
{ 
	_tbl.push_back(CAIEntityMatrixIteratorTblRandom::STblEntry(dx,dy)); 
}

inline const CAIEntityMatrixIteratorTblRandom::STblEntry &CAIEntityMatrixIteratorTblRandom::operator[](uint32 idx) const
{
	#ifdef NL_DEBUG
		nlassert(idx<_tbl.size());
	#endif

	return _tbl[idx];
}

inline const CAIEntityMatrixIteratorTblRandom::iterator CAIEntityMatrixIteratorTblRandom::begin() const
{
	return const_cast<TTbl&>(_tbl).begin();
}

inline const CAIEntityMatrixIteratorTblRandom::iterator CAIEntityMatrixIteratorTblRandom::end() const
{
	return const_cast<TTbl&>(_tbl).end();
}

inline uint32 CAIEntityMatrixIteratorTblRandom::size() const
{
	return (uint32)_tbl.size();
}


//--------------------------------------------------------------------------
// The CAIEntityMatrixIteratorTblLinear inlines
//--------------------------------------------------------------------------

inline CAIEntityMatrixIteratorTblLinear::CAIEntityMatrixIteratorTblLinear()	
{ 
}

inline CAIEntityMatrixIteratorTblLinear::CAIEntityMatrixIteratorTblLinear(uint32 *runLengths,uint32 count)	
{ 
	uint32 lastRun=0;
	for(uint i=0;i<count;++i)
	{
		push_back(runLengths[i],lastRun);
		lastRun=runLengths[i];
	}
}

inline void CAIEntityMatrixIteratorTblLinear::push_back(uint32 runLength, sint32 previousRunLength)	
{ 
	_tbl.push_back(CAIEntityMatrixIteratorTblLinear::STblEntry(runLength,previousRunLength)); 
}

inline const CAIEntityMatrixIteratorTblLinear::STblEntry &CAIEntityMatrixIteratorTblLinear::operator[](uint32 idx) const
{
	#ifdef NL_DEBUG
		nlassert(idx<_tbl.size());
	#endif

	return _tbl[idx];
}

inline const CAIEntityMatrixIteratorTblLinear::iterator CAIEntityMatrixIteratorTblLinear::begin() const
{
	return const_cast<TTbl&>(_tbl).begin();
}

inline const CAIEntityMatrixIteratorTblLinear::iterator CAIEntityMatrixIteratorTblLinear::end() const
{
	return const_cast<TTbl&>(_tbl).end();
}

inline uint32 CAIEntityMatrixIteratorTblLinear::size() const
{
	return (uint32)_tbl.size();
}


//--------------------------------------------------------------------------
#endif
