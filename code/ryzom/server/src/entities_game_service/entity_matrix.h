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



#ifndef RY_ENTITY_MATRIX_H
#define RY_ENTITY_MATRIX_H

#include "entity_list_link.h"
#include "area_geometry.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "entity_manager/entity_base.h"


/***************************

TODO : 
This system was designed as a template to support special entities such as harvest deposit explosions,...
In fact, it is only used with CEntityBase. As special features had to be included in it, we use a lot of methods very specific to CEntityBase
So This class must be reworked when the new EGS classes will be designed.

trap 14.12.2004 : I have deleted the template argument of the CEntityMatrix class. This class is no more template.

****************************/



/// helper for coords conversion
/// convert world coords to matrix coords ( >> 14 is approximatively /16000 but faster )
inline static uint16 WorldtoMatrixDistance( uint32 dist )
{ 
	return uint16(dist >> 14); 
}
inline static uint8 WorldXtoMatrixX( sint32 x )
{ 
	return uint8(x >> 14); 
}
inline static uint8 WorldYtoMatrixY( sint32 y )
{ 
	return uint8((-y) >> 14); 
}

/*************************************************************

  MATRIX PATTERNS
  They represent an area of an entity matrix to be scanned
  Linear pattern tables use a better RAM access patern than random access tables and should be used whenever possible
  For template use, all pattern class must present the following method:
  - uint16 size() : return the size in rows of the pattern
  - uint16 runLength(uint row) : return the runLength of the specified row
  - sint16 startDx(uint row) : return a value to add to an X coord to go to the beginning of the current row from the end of the previous

**************************************************************/


/// This class uses a vector of horizontal run lengths to represent a pattern
/// The pattern represented by the data is assumed to be symetrical in both x and y.
/// it is based on linear tables. To avoid the generation of too many tables, the choice of the best iterator is done with only 1 parameter
/// So it is perfect for circles, but not for most other patterns ( rectangles for example are better selected by width and length )
class CEntityMatrixPatternSymetrical
{
public:
	/// ctor, used during init only
	inline CEntityMatrixPatternSymetrical(uint32 *runLengths,uint32 count)
	{
		uint32 lastRun=0;
		for(uint i=0;i<count;++i)
		{
			_Rows.push_back(CRow(runLengths[i],lastRun)); 
			lastRun=runLengths[i];
		}
	}
	
	static void initMatrixPatterns();
	/// find the best Disc pattern according to the distance chosen
	inline static CEntityMatrixPatternSymetrical* bestDiscPattern( uint16 distInMeters )
	{
#ifdef NL_DEBUG
		nlassert(distInMeters < _DiscPatterns.size());
#endif
		return _DiscPatterns[ distInMeters ];
	}
	

	inline uint16 size() const{ return (uint16)_Rows.size(); }
	inline sint16 startDx(uint row) const
	{ 
#ifdef NL_DEBUG
		nlassert( row < _Rows.size() );
#endif
		return _Rows[row].StartDx;
	}
	inline uint16 runLength(uint row) const
	{ 
#ifdef NL_DEBUG
		nlassert( row < _Rows.size() );
#endif
		return _Rows[row].RunLength;
	}	

private:
	/// struct representing a row in our pattern
	struct CRow
	{
		CRow()						 {}
		inline CRow(uint32 runLength, sint32 previousRunLength)	
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
		// length of a run in this row
		uint16 RunLength;
		// dx to add to the last x of the previous line to go to the beginning of this row
		sint16 StartDx;
	};
	
	/// the rows of the pattern
	std::vector<CRow> _Rows;

	/// the table of disc patterns
	static std::vector<CEntityMatrixPatternSymetrical*> _DiscPatterns;
};

/// This pattern represents a rectangle aligned with The X and Y axis
/// the width is the norm of the side on the X axis
/// the height is the norm of the side on the Y axis
/// These pattern are built "on the fly", no tables are needed
class CEntityMatrixPatternRect
{
public:
	inline CEntityMatrixPatternRect( uint32 width, uint32 height)
	{
		// first get the width in matrix coords
		_RunLength = WorldtoMatrixDistance( width );
		// if the runlength is even, add 3 ( 1 cell for the center cell and 2 because we dont know where the center is in the cell. Remove 1 because we just want the excess )
		if ( (_RunLength & 1) == 0 )
			_RunLength+= 2;
		// if the runlength is odd, add 2( 2 because we dont know where the center is in the cell. We alredy have a center. Remove 1 because we just want the excess )
		else
			_RunLength+= 1;

		// same for the size
		_Size = WorldtoMatrixDistance( height);
		if ( (_Size & 1) == 0 )
			_Size+= 3;
		else
			_Size+= 2;

		_StartDx = - (sint16)_RunLength;
	}
	
	
	inline uint16 size() const
	{ 
		return _Size;
	}
	inline sint16 startDx(uint row) const
	{
		return _StartDx;
	}
	inline uint16 runLength(uint row) const
	{ 
		return _RunLength;
	}
	
	
private:

	sint16 _StartDx;
	uint16 _RunLength;
	uint16 _Size;
};


/// This pattern represents a square border aligned with The X and Y axis :
/*
		*************
		*           *
		*           *
		*           *
		*           *
		*************
 */ 
/// the cellWidth is the width in cell of the border
/// These pattern are built "on the fly", no tables are needed
class CEntityMatrixPatternBorder
{
public:
	inline CEntityMatrixPatternBorder( uint8 cellSide )
	{
		_Side = cellSide;		
		_StartDx = sint16(1) - (sint16)_Side;
	}
	
	
	inline uint16 size() const
	{ 
		return _Side;
	}
	inline sint16 startDx(uint row) const
	{
		return _StartDx;
	}
	inline uint16 runLength(uint row) const
	{ 
		if ( row == 0 || row == uint ( _Side-1)  )
			return _Side - 1;
		return 1;
	}
	
	
private:
	uint16 _Side;
	sint16 _StartDx;
};


/**
 * Matrix used to dispatch all the entities in its entries
 * The matrix size is 256*256. Each entry contains a linked list of entities ( see CEntityListLink )
 * This way it is much faster to get all the entities which are within a specific distance from a point
 * Entities position in the matrix must be periodically updated
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CEntityMatrix
{
public:
	
	/// iterator used to iterate through an entity matrix cells using a specific pattern
	/// TEntity is the entity type
	/// TPattern is the used pattern type
	template <class TPattern >
	class CCellIterator
	{
	public:	
	
	/// ctor only used before operator = in STL-like for loops
	inline CCellIterator(){}
	/// real ctor
	/// matrix is the observed matrix, pattern is the pattern used to iterate
	/// x and y are the world coords of the center point
	inline CCellIterator( CEntityMatrix *matrix,TPattern *pattern,sint32 x, sint32 y )
		:_Matrix(matrix),_Pattern(pattern)
		{
		#ifdef NL_DEBUG
			nlassert(_Pattern!=NULL);
			nlassert(_Pattern->size()>0);
			nlassert(_Pattern->size()<32768);	// a numeric over-run limit
			nlassert(_Matrix!=NULL);
		#endif
			// setup the iterator to point to the start of the pattern and setup properties accordingly
			_IndexInPattern = 0;
			_RunLengthRemaining= _Pattern->runLength(0);
			_X = uint8( WorldXtoMatrixX(x) - (sint16)(pattern->runLength(0)/2) );
			_Y = uint8( WorldYtoMatrixY(y) - (sint16)(pattern->size()/2) );
		}
	
		///\return the en
		inline CEntityListLink<CEntityBase>*operator*() 
		{
			return & ( (*_Matrix)[_Y][_X] );	
		}
		inline CCellIterator &operator++() 
		{
		#ifdef NL_DEBUG
			// make sure we aren'TEntity trying to access an uninitialised iterator
			nlassert(_IndexInPattern < _Pattern->size());
		#endif
			// if we're not at the end of the current run continue run else move on to next line
			if (_RunLengthRemaining!=0)
			{
				--_RunLengthRemaining;
				++_X;
			}
			else
			{
				++_IndexInPattern;
				// check if end not reached
				if ( _IndexInPattern < _Pattern->size() )
				{
					_RunLengthRemaining= _Pattern->runLength(_IndexInPattern);
					_X= uint8( _X + _Pattern->startDx(_IndexInPattern) );
					++_Y;
				}
			}
			
			return *this;
		}
		
		inline bool end()	const
		{
			return _IndexInPattern >= _Pattern->size();
		}
	
	protected:
		/// coords of the current cells in the matrix
		uint8							_X;
		uint8							_Y;
		// matrix used by the iterator
		CEntityMatrix*					_Matrix;
		/// current pattern used
		TPattern*						_Pattern;
		/// iterator in the used pattern (used to see in which row of the pattern we are)
		uint32							_IndexInPattern;
		/// remaining run length
		uint32							_RunLengthRemaining;
	};

	template <class TPattern >
	class CCellIteratorBorder : public CCellIterator<TPattern>
	{
		public:	
			/// ctor only used before operator = in STL-like for loops
			inline CCellIteratorBorder(){}
			/// real ctor
			/// matrix is the observed matrix, pattern is the pattern used to iterate
			/// x and y are the world coords of the center point
			inline CCellIteratorBorder( CEntityMatrix *matrix,TPattern *pattern,sint32 x, sint32 y )
				:CCellIterator<TPattern>(matrix,pattern,x,y)
			{
		#ifdef NL_DEBUG
				nlassert(this->_Pattern!=NULL);
				nlassert(this->_Pattern->size()>0);
				nlassert(this->_Pattern->size()<32768);	// a numeric over-run limit
				nlassert(this->_Matrix!=NULL);
		#endif
				// setup the iterator to point to the start of the pattern and setup properties accordingly
				this->_IndexInPattern = 0;
				this->_RunLengthRemaining = this->_Pattern->runLength(0);
				this->_X = uint8( WorldXtoMatrixX(x) - (sint16)(pattern->runLength(0)/2) );
				this->_Y = uint8( WorldYtoMatrixY(y) - (sint16)(pattern->size()/2) );
			}
			inline CCellIteratorBorder &operator++() 
			{
#ifdef NL_DEBUG
				// make sure we aren'TEntity trying to access an uninitialised iterator
				nlassert(this->_IndexInPattern < this->_Pattern->size());
#endif
				
				if ( this->_RunLengthRemaining !=0 )
				{
					if ( this->_Pattern->runLength(this->_IndexInPattern) == 1 )
					{
						--this->_RunLengthRemaining;
						this->_X+= this->_Pattern->size() - 1;
					}
					else
					{
						--this->_RunLengthRemaining;
						++this->_X;
					}
				}
				else
				{
					++this->_IndexInPattern;
					// check if end not reached
					if ( this->_IndexInPattern < this->_Pattern->size() )
					{
						this->_RunLengthRemaining= this->_Pattern->runLength(this->_IndexInPattern);
						this->_X= uint8( this->_X + this->_Pattern->startDx(this->_IndexInPattern) );
						++this->_Y;
					}
				}
				return *this;
			}
				
	};
	



/*
-----------------------------------------------------------------------------------------------------
class CAIEntityMatrix<T>::CEntityIteratorTemplate
class CAIEntityMatrix<T>::CEntityIteratorLinear
class CAIEntityMatrix<T>::CEntityIteratorRandom

 This class provides an iterator for iterating across the entities listed in the cells of a matrix 
 '_matrix' following the pattern described by '_Pattern'
  
 The class is composed of a CCellIterator' _CellIt' responsible for iterating across the matrix
 and an entity pointer '_Entity' used for iterating over the entities in each matrix cell
	
 Note that unlinking, moving or deleting the entity refferenced by a CEntityIteratorLinear iterator
 invalidates it.
-----------------------------------------------------------------------------------------------------
*/
	template <class TPattern, class TCellIt>
	class CEntityIteratorTemplate
	{
	public:
		inline CEntityIteratorTemplate(){}
		inline CEntityIteratorTemplate(CEntityMatrix *matrix,
										TPattern *pattern,
										sint32 centerX,
										sint32 centerY) 
		: _CellIt(matrix,pattern,centerX,centerY),_CenterPosX(double(centerX)/1000.0),_CenterPosY(double(centerY)/1000.0)
		{
			// get a pointer to the list link
			_Entity=(*_CellIt);
		}
	
	
		inline CEntityBase &operator*()
		{
		#ifdef NL_DEBUG
			// make sure we aren't trying to access passed the end of list
			nlassert(!end());
		#endif
		CEntityBase * entity = dynamic_cast<CEntityBase *>(_Entity->entity());
		nlassert(entity);
		return *entity;
		}
	
		inline	const	CEntityIteratorTemplate	&operator++()
		{
		#ifdef NL_DEBUG
			// if you are on a breakpoint here it is because you've tried to do a ++ on an iterator
			nlassert(!_CellIt.end());
		#endif			
			// repeat the following loop until either we come to the end of the cell iterator or we find a valid entity 
			do
			{
				// try to get the next entity in the cell
				_Entity=_Entity->next();
				if (_Entity==*_CellIt)
				{
					// we're at the end of the entity list for this cell so try to find another cell with a valid entity
					do
					{
						++_CellIt;
					}
					while	(	!_CellIt.end()
						&& (*_CellIt)->unlinked());
					
					_Entity=(*_CellIt)->next();
				}
			}
			while	(	!_CellIt.end()
				&&	(	_Entity->unlinked()
				||	! testValidity(_Entity->entity()) ) );
			//	as cells are tiled over the world, we need to check if our entity have a real good match with the scanned position.
			return	*this;
		}
		// method for testing iterator for end of current sequence
		inline bool end()
		{
			// the following can only happen if there are no more entites in cell and no more deltas in cell iterator tbl
			return	_CellIt.end();
		}
		inline float getDistance(){return _Distance;}
	
	protected:
		/// test the validity of an entity
		virtual bool testValidity( CEntityBase* entity ) = 0;
		
		TCellIt								_CellIt;		// the cell iterator
		const CEntityListLink<CEntityBase>*	_Entity;		// which entity are we pointing at (within the cell)

		// center position
		double _CenterPosX;
		double _CenterPosY;
		// distance to center
		float  _Distance;
	};

	/// iterator used to get entities in a disc around a point
	class CEntityIteratorDisc : public CEntityIteratorTemplate<CEntityMatrixPatternSymetrical, CCellIterator<CEntityMatrixPatternSymetrical> >
	{
	public:
		inline CEntityIteratorDisc(){}
		inline CEntityIteratorDisc(CEntityMatrix *matrix,CEntityMatrixPatternSymetrical *pattern,
			sint32 posX, 
			sint32 posY,
			double radiusSquare)
			:CEntityIteratorTemplate<CEntityMatrixPatternSymetrical, CCellIterator<CEntityMatrixPatternSymetrical> >
			( matrix,
			  pattern,
			  posX,
			  posY
			  ),
			_RadiusSquare(radiusSquare){}

	protected:

		inline bool testValidity( CEntityBase* entity )
		{
			if( entity == 0 )
			{
				nlwarning("CEntityIteratorDisc::testValidity entity == 0 !!");
				return false;
			}
			double dx = double(entity->getState().X) /1000.0 -  _CenterPosX;
			double dy = double(entity->getState().Y) /1000.0 -  _CenterPosY;
			double distanceSquare = dx * dx + dy * dy;
			if  (  distanceSquare <= _RadiusSquare )
			{
				_Distance = (float)sqrt(distanceSquare);
				return true;
			}
			else
			{
				return false;
			}
		}
		double _RadiusSquare;
	};

	/// iterator used to get entities in a truncated cone ( a trapezoid in fact )
	class CEntityIteratorCone : public CEntityIteratorTemplate<CEntityMatrixPatternRect, CCellIterator<CEntityMatrixPatternRect> >
	{
	public:
		inline CEntityIteratorCone(){}
		inline CEntityIteratorCone(CEntityMatrix *matrix,
									CEntityMatrixPatternRect *pattern, 
									const CAreaQuad<sint32> * cone, 
									const CAreaCoords<sint32> & center, 
									const CEntityBase* mainTarget)
			:CEntityIteratorTemplate<CEntityMatrixPatternRect, CCellIterator<CEntityMatrixPatternRect> >
			( matrix,
			pattern,
			center.X,
			center.Y
			),
			_Cone(cone),
			_MainTarget(mainTarget)
		{
#ifdef NL_DEBUG
			nlassert(cone);
#endif			
		}		
	private:
		
		inline bool testValidity( CEntityBase* entity )
		{
			if( entity == 0 )
			{
				nlwarning("CEntityIteratorCone::testValidity entity == 0 !!");
				return false;
			}
			
			if ( entity != _MainTarget && _Cone->contains( CAreaCoords<sint32>(entity->getState().X,entity->getState().Y) ) )
			{
				double dx = double(entity->getState().X) /1000.0 -  _CenterPosX;
				double dy = double(entity->getState().Y) /1000.0 -  _CenterPosY;
				double distanceSquare = dx * dx + dy * dy;
				_Distance = (float)sqrt(distanceSquare);
				return true;
			}
			return false;
		}
		const CAreaQuad<sint32> * _Cone;
		const CEntityBase* _MainTarget;
	};

	/// iterator used to get entities in a disc around a point
	class CEntityIteratorChainCenter : public CEntityIteratorDisc
	{
	public:
		inline CEntityIteratorChainCenter(){}
		inline CEntityIteratorChainCenter(CEntityMatrix *matrix,
											CEntityMatrixPatternSymetrical *pattern,
											sint32 posX, 
											sint32 posY,
											double radiusSquare,
											const std::vector<CEntityBase*> * addedEntities,
											ACTNATURE::TActionNature nature,
											const TDataSetRow & actor,
											const CEntityBase * mainTarget
											)
			:CEntityIteratorDisc(matrix,pattern,posX,posY,radiusSquare),
			_AddedEntities(addedEntities),_ActorRowId(actor),_Nature(nature),_MainTarget(mainTarget){}
		
	private:
		
		inline bool testValidity( CEntityBase* entity )
		{
			if( entity == 0 )
			{
				nlwarning("CEntityIteratorChainCenter::testValidity entity == 0 !!");
				return false;
			}
			
			for ( uint i = 0; i < _AddedEntities->size(); i++ )
			{
				if ( (*_AddedEntities)[i] == entity )
					return false;
			}

			const bool mainTarget = (entity == _MainTarget);
			std::string dummy;

			switch (_Nature)
			{
			case ACTNATURE::FIGHT:
				if ( ! PHRASE_UTILITIES::testOffensiveActionAllowed(_ActorRowId, entity->getEntityRowId(), dummy, mainTarget) )
				{
					return false;
				}
				break;

			case ACTNATURE::OFFENSIVE_MAGIC:
			case ACTNATURE::CURATIVE_MAGIC:
				if ( ! PHRASE_UTILITIES::validateSpellTarget(_ActorRowId, entity->getEntityRowId(), _Nature, dummy, mainTarget) )
				{
					return false;
				}
				break;

			default:
				nlwarning("<CEntityIteratorChainCenter::testValidity> bad action nature: %s", ACTNATURE::toString(_Nature).c_str());
			}

			return CEntityIteratorDisc::testValidity( entity );
		}

		const std::vector<CEntityBase*>*	_AddedEntities;
		ACTNATURE::TActionNature			_Nature;
		TDataSetRow							_ActorRowId;
		const CEntityBase *					_MainTarget;
		
	};


	/// iterator used to get entities in a disc around a point
	class CEntityIteratorChainBorder : public CEntityIteratorTemplate<CEntityMatrixPatternBorder, CCellIteratorBorder<CEntityMatrixPatternBorder> >
	{
	public:
		inline CEntityIteratorChainBorder(){}
		inline CEntityIteratorChainBorder(CEntityMatrix *matrix,
											CEntityMatrixPatternBorder *pattern,
											sint32 posX, 
											sint32 posY,
											double radiusSquare, 
											const std::vector<CEntityBase*> * entities,
											ACTNATURE::TActionNature nature,
											const TDataSetRow & actor,
											const CEntityBase * mainTarget
											)
			:CEntityIteratorTemplate<CEntityMatrixPatternBorder, CCellIteratorBorder<CEntityMatrixPatternBorder> >
			( matrix,
			pattern,
			posX,
			posY
			),
			_RadiusSquare(radiusSquare),_AddedEntities(entities),_ActorRowId(actor),_Nature(nature),_MainTarget(mainTarget){}
		
	private:
		
		inline bool testValidity( CEntityBase* entity )
		{
			if( entity == 0 )
			{
				nlwarning("CEntityIteratorChainBorder::testValidity entity == 0 !!");
				return false;
			}
			
			for ( uint i = 0; i < _AddedEntities->size(); i++ )
			{
				if ( (*_AddedEntities)[i] == entity )
					return false;
			}

			const bool mainTarget = (entity == _MainTarget);
			std::string dummy;

			switch (_Nature)
			{
			case ACTNATURE::FIGHT:
				if ( ! PHRASE_UTILITIES::testOffensiveActionAllowed(_ActorRowId, entity->getEntityRowId(), dummy, mainTarget) )
				{
					return false;
				}
				break;

			case ACTNATURE::OFFENSIVE_MAGIC:
			case ACTNATURE::CURATIVE_MAGIC:
				if ( ! PHRASE_UTILITIES::validateSpellTarget(_ActorRowId, entity->getEntityRowId(), _Nature, dummy, mainTarget) )
				{
					return false;
				}
				break;

			default:
				nlwarning("<CEntityIteratorChainBorder::testValidity> bad action nature: %s", ACTNATURE::toString(_Nature).c_str());
			}

			double dx = double(entity->getState().X) /1000.0 -  _CenterPosX;
			double dy = double(entity->getState().Y) /1000.0 -  _CenterPosY;
			double distanceSquare = dx * dx + dy * dy;
			if  (  distanceSquare <= _RadiusSquare )
			{
				_Distance = (float)sqrt(distanceSquare);
				return true;
			}
			else
			{
				return false;
			}
		}

		double								_RadiusSquare;
		const std::vector<CEntityBase*> *	_AddedEntities;
		ACTNATURE::TActionNature			_Nature;
		TDataSetRow							_ActorRowId;
		const CEntityBase *					_MainTarget;
	};







	/// A line of the matrix
	class	CMatrixLine
	{
	public:
		/// one and only accessor
		inline CEntityListLink<CEntityBase> &operator[](uint8 x) { return Line[x]; }
	private:
		/// data
		CEntityListLink<CEntityBase> Line[256];
	};


	/// link an entity to the matrix
	inline void linkToMatrix(sint32 x, sint32 y, CEntityListLink<CEntityBase> & link)
	{
		// too slow H_AUTO(linkToMatrix);
		link.link(_Matrix [(uint8)WorldYtoMatrixY(y)] [(uint8)WorldXtoMatrixX(x)]);
	}

	
	inline CEntityIteratorDisc beginEntitiesInDisc(CEntityMatrixPatternSymetrical * pattern ,sint32 x, sint32 y,double radiusSquare)
	{
		CEntityIteratorDisc newIt(this,pattern,x,y,radiusSquare);
		++newIt;
		return newIt;
	}
	
	inline CEntityIteratorCone beginEntitiesInCone(CEntityMatrixPatternRect * pattern , const CAreaQuad<sint32> * cone, const CAreaCoords<sint32> & center, const CEntityBase * mainTarget)
	{
		CEntityIteratorCone newIt(this,pattern, cone, center, mainTarget);
		++newIt;
		return newIt;
	}

	inline CEntityIteratorChainCenter beginEntitiesInChainCenter(CEntityMatrixPatternSymetrical * pattern ,sint32 x, sint32 y,double radiusSquare, const std::vector<CEntityBase*> *addedEntities, ACTNATURE::TActionNature nature, const TDataSetRow & actor, const CEntityBase * mainTarget)
	{
		CEntityIteratorChainCenter newIt(this,pattern,x,y,radiusSquare,addedEntities, nature, actor, mainTarget);
		++newIt;
		return newIt;
	}

	inline CEntityIteratorChainBorder beginEntitiesInChainBorder(CEntityMatrixPatternBorder * pattern ,sint32 x, sint32 y,double radiusSquare, const std::vector<CEntityBase*> *addedEntities, ACTNATURE::TActionNature nature, const TDataSetRow & actor, const CEntityBase * mainTarget)
	{
		CEntityIteratorChainBorder newIt(this,pattern,x,y,radiusSquare,addedEntities, nature, actor, mainTarget);
		++newIt;
		return newIt;
	}


	// table lookup operator - should be used as myMatrix[y][x]
	inline CMatrixLine & operator[](uint8 y)
	{
		return _Matrix[y];
	}

private:
	/// the matrix data. WARNING acces is _Matrix[y][x]
	CMatrixLine _Matrix[256];
};

//the entity matrix
extern CEntityMatrix EntityMatrix;

#endif // RY_ENTITY_MATRIX_H

/* End of entity_matrix.h */
























