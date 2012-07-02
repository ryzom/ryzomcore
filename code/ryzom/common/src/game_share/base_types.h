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




#ifndef NL_BASE_TYPES_H
#define NL_BASE_TYPES_H


#include "nel/misc/types_nl.h"
#include "nel/net/unified_network.h"


/*
 * Types for options of CMirroredDataSet::declareProperty()
 *
 * \seealso CMirrorDataSet::declareProperty for more info
 */
typedef uint16 TPropSubscribingOptions;
enum TPropSubscribingOpt { PSOReadWrite=0, PSOReadOnly=1, PSOWriteOnly=2, PSONotifyChanges=4, PSOMonitorAssignment=8 };


/*
 * STORE_CHANGE_SERVICEIDS
 * Define this macro to enable the storing and checking of which service changes the value of a property
 * (in addition to storing the timestamp of the change).
 */
#define STORE_CHANGE_SERVICEIDS

/*
 * CHECK_DATASETROW_VALIDITY
 * Define this macro to enable checking if a datasetrow is still valid every time we access the mirror
 * with it. A datasetrow becomes invalid when the corresponding entity is removed.
 */
#define	CHECK_DATASETROW_VALIDITY

#ifndef FINAL_VERSION
#error "FINAL_VERSION missing"
#endif

/*
 * FAST_MIRROR
 * Define this macro to remove all runtime checks in mirror accessors.
 */
#if FINAL_VERSION
# ifndef FAST_MIRROR
#  define FAST_MIRROR
# endif
#else
# undef FAST_MIRROR
#endif

/*
 * MIRROR_INFO and MIRROR_DEBUG
 * These two macros are equivalent to nlinfo & nldebug respectively except that they allow logging to be dissabled via the variable VerboseMIRROR
 */

#define MIRROR_INFO if (!VerboseMIRROR.get()) {} else nlinfo
#define MIRROR_DEBUG if (!VerboseMIRROR.get()) {} else nldebug

extern NLMISC::CVariable<bool> VerboseMIRROR;


//#ifdef CHECK_DATASETROW_VALIDITY

const uint32 INVALID_DATASET_INDEX = 0x00FFFFFF;

// Always simple index
//class TDataSetIndex
//{
//public:
//
//	/// Default constructor (initializes to INVALID_DATASET_INDEX)
//	TDataSetIndex() : _Index(INVALID_DATASET_INDEX) {}
//
//	/// Assignment constructor
//	TDataSetIndex( uint32 srcValue ) : _Index(srcValue) {}
//
//	/// Constructor with no initialization
//	TDataSetIndex( bool doNotInitialize ) {}
//
//	/// Full access
//	operator uint32	()	const	{ return _Index; }
//
//	/// Assignment operator
//	TDataSetIndex& operator= ( uint32 src ) { _Index = src; }
//
////	uint32	operator	&(uint32 index)			const	{	return _Index&index; }
////	bool	operator	!=(const	TDataSetIndex	&other)	const	{	return _Index!=other._Index; }
////	bool	operator	==(const	TDataSetIndex	&other)	const	{	return _Index==other._Index; }
////	uint32	operator	>>(uint32 decal)		const	{	return _Index>>decal; }
////	bool	operator	<(TDataSetIndex other)	const	{	return _Index<other._Index; }
//
//	uint32	operator	++()	{	_Index++;	return _Index; }
//	uint32	operator	--()	{	_Index--;	return _Index; }
//	uint32	operator	+=(uint32 index)	{	_Index+=index;	return _Index; }
//	uint32	operator	-=(uint32 index)	{	_Index+=index;	return _Index; }
//	uint32	operator	|=(uint32 index)		{	_Index|=index;	return _Index; }
//
//private:
//	uint32			_Index;
//};

//	const TDataSetIndex NB_RESERVED_ROWS((uint32)1);

//	const TDataSetIndex	LAST_CHANGED((uint32)(INVALID_DATASET_INDEX-1));

typedef	uint32 TDataSetIndex;
const TDataSetIndex NB_RESERVED_ROWS=1;
const TDataSetIndex	LAST_CHANGED=INVALID_DATASET_INDEX-1;


/*
 * Version of TDataSetRow with embedded removal counter (to check validity)
 */

class TDataSetRow
{
public:
	/// Default constructor
	TDataSetRow() : _EntityIndexAndRemCounter( INVALID_DATASET_INDEX ) {}

	/// Copy constructor
	TDataSetRow( const TDataSetRow& src ) : _EntityIndexAndRemCounter( src._EntityIndexAndRemCounter ) {}

	/// Assignment
	TDataSetRow& operator= ( const TDataSetRow& src )
	{
		_EntityIndexAndRemCounter = src._EntityIndexAndRemCounter;
		return *this;
	}

	/// Explicit index
	TDataSetIndex		getIndex() const { return _EntityIndexAndRemCounter & 0xFFFFFF; }

	/**
	 * Get a compressed 20-bit index.
	 * - Assumes the max size of the corresponding dataset is lower than 2^20 (about 1 million rows)
	 * - Does not include the assignment counter of the dataset row (use with caution)
	 * - An invalid dataset row returns the compressed value 0xFFFFF.
	 * - To uncompress this value to a dataset row, use CMirrorDataSet::getCurrentDataSetRow().
	 *   Check the validity of the returned dataset row (!isValid()), as it will be invalid
	 *   if the index is not used anymore (but you will not be able to know if it has be
	 *   reassigned, so don't use this scheme if you don't check the uncompressed index
	 *   before the min reassignment time.
	 */
	TDataSetIndex		getCompressedIndex() const
	{
		//nlassert( datasetRow.getIndex() < 0xFFFFF );
		return _EntityIndexAndRemCounter & 0xFFFFF;
	}

//	!!! be careful, I removed operator= because TDataSetIndex is an int and there is permissious side effects with the use of CEntityId.
/// Assignment (no rem counter)
//	TDataSetRow& operator= ( TDataSetIndex srcIndex )
//	{
//		_EntityIndexAndRemCounter = srcIndex;
//		return *this;
//	}
//	/// Return the real index (without the packed counter)
//	operator TDataSetIndex () const { return getIndex(); } // 24 lower bits

	/// Prefix incrementation
	TDataSetRow& operator++ () { ++_EntityIndexAndRemCounter; return *this; }

	/// Prefix decrementation
	TDataSetRow& operator-- () { --_EntityIndexAndRemCounter; return *this; }

	/// Postfix incrementation
	TDataSetRow  operator++ ( int ) { TDataSetIndex value = _EntityIndexAndRemCounter; ++_EntityIndexAndRemCounter; return TDataSetRow(value); }

	/// Postfix decrementation
	TDataSetRow  operator-- ( int ) { TDataSetIndex value = _EntityIndexAndRemCounter; --_EntityIndexAndRemCounter; return TDataSetRow(value); }

	/// Add
	TDataSetRow& operator+= ( uint32 delta ) { _EntityIndexAndRemCounter += delta; return *this; }

	/// Sub
	TDataSetRow& operator-= ( uint32 delta ) { _EntityIndexAndRemCounter -= delta; return *this; }

	//	Used by Less STL template method. ( it must be slow .. :( )
	bool	operator	<(const	TDataSetRow	&other)		const	{	return	_EntityIndexAndRemCounter<other._EntityIndexAndRemCounter; 	}
	bool	operator	!=(const	TDataSetRow	&other)	const	{	return	_EntityIndexAndRemCounter!=other._EntityIndexAndRemCounter; }
	bool	operator	==(const	TDataSetRow	&other)	const	{	return	_EntityIndexAndRemCounter==other._EntityIndexAndRemCounter; }

	bool	operator	!=(TDataSetIndex	val)	{	return	getIndex()!=val; 	}
	bool	operator	==(TDataSetIndex	val)	{	return	getIndex()==val; 	}

	/// Return the entity binding counter
	uint8		counter() const { return (uint8)(_EntityIndexAndRemCounter >> 24); }


	/// Return true if the datasetrow is initialised to a valid value
	bool		isValid	()	const
	{
		return (getIndex()!=INVALID_DATASET_INDEX);
	}

	/// Return true if the datasetrow is not ininitialized
	bool		isNull() const
	{
		return (getIndex()==INVALID_DATASET_INDEX);
	}

	/// Serial (index and removal counter)
	void		serial( NLMISC::IStream& s )
	{
		s.serial( _EntityIndexAndRemCounter );
	}

	/// Return a string representing the datasetrow
	std::string	toString() const;

	/// Set to LAST_CHANGED
	void		initToLastChanged()
	{
		_EntityIndexAndRemCounter = LAST_CHANGED;
	}

	///
	class CHashCode
	{
	public:

		CHashCode() {}

		static const size_t bucket_size = 4;
		static const size_t min_buckets = 8;

		size_t	operator () ( const TDataSetRow &index ) const { return index.getHashCode(); }

		bool operator() (const TDataSetRow &index1, const TDataSetRow &index2) const { return index1.getHashCode() < index2.getHashCode(); }
	};

	/// Warning: method to avoid (use it only when using rows as a static array)
	static TDataSetRow createFromRawIndex( TDataSetIndex entityIndex ) { return TDataSetRow(entityIndex); }

private:
	friend class TDataSetRow::CHashCode;

	//	allows full access to ..
	friend	class	CMirror;
	friend	class	CMirrors;
	friend	class	CRangeList;
	friend	class	CDataSetMS;
	friend	class	CDeltaToMS;
	friend	class	CDataSetBase;
	friend	class	CMirrorService;
	friend	class	CMirroredDataSet;
	friend	class	CChangeTrackerBase;
	friend	class	CChangeTrackerClient;
	friend	class	CPropLocationUnpacked;
	friend	class	CPropLocationPacked1DS;

	friend	struct	monitorMirrorEntityClass;
	friend	struct	TClientServiceInfoOwnedRange;

	//	hide this type of constructors ( only use it in mirror methods ).
	/// Simple constructor (no rem counter)
	explicit		TDataSetRow( TDataSetIndex entityIndex ) : _EntityIndexAndRemCounter( entityIndex ) {}

	/// Constructor
	explicit		TDataSetRow( TDataSetIndex entityIndex, uint8 remCounter )
	{
//		_EntityIndexAndRemCounter = ((uint32)entityIndex) | ((uint32)remCounter << 24);
		_EntityIndexAndRemCounter=entityIndex;
		RemCounter=remCounter;
	}

	/// Set the index, no rem counter (yet)
	void			initFromIndex ( TDataSetIndex srcIndex )
	{
		_EntityIndexAndRemCounter = srcIndex;
	}

	/// Fill the entity binding counter
	void			setCounter( uint8 remCounter )
	{
//		_EntityIndexAndRemCounter |= ((uint32)remCounter << 24);
		RemCounter=remCounter;
	}

	/// Get Hash code
	uint32		getHashCode() const
	{
		return (uint32)(getIndex());
	}

	union
	{
		struct
		{
			TDataSetIndex	_EntityIndexAndRemCounter;
		};
		struct
		{
			uint8	dummy[3];
			uint8	RemCounter;
		};
	};
};

//#else
//// Row (which is mainly an index)
//typedef sint32 TDataSetRow;
//// Always simple index
//typedef sint32 TDataSetIndex;
//#endif


/*
 * Constants
 */
const TDataSetIndex INVALID_DATASET_ROW ((uint32)0x00FFFFFF); // leaving blank the most significant byte (see removal counter when CHECK_DATASETROW_VALIDITY is enabled)

/*
 * Short datasetrow used to reduce network overhead
 */
typedef uint16 TDataSetRow16;
const TDataSetRow16 INVALID_DATASET_ROW_16 = (TDataSetRow16)~0;

/*
 * Shorter datasetrow used to reduce network overhead
 */
typedef uint8 TDataSetRow8;
const TDataSetRow8 INVALID_DATASET_ROW_8 = (TDataSetRow8)~0;


/*
 * Range entry
 */
struct TRowRange
{
	TDataSetIndex				First;
	TDataSetIndex				Last;
	NLNET::TServiceId			OwnerServiceId;
	NLNET::TServiceId			MirrorServiceId;

	void serial( NLMISC::IStream& s )
	{
		s.serial( First );
		s.serial( Last );
		s.serial( OwnerServiceId );
		// No need of the MirrorServiceId, currently
	}
};


/*
 * Type for the index of property in a dataset
 */
typedef sint16 TPropertyIndex;
const TPropertyIndex INVALID_PROPERTY_INDEX = (TPropertyIndex)~0;


/*
 * Type for the row in a shared list associated to a property in a dataset
 */
#undef MIRROR_LIST_ROW_32BITS

#ifdef MIRROR_LIST_ROW_32BITS

typedef uint32 TSharedListRow;
const TSharedListRow INVALID_SHAREDLIST_ROW = ~0; //((uint16)~0)-1;
const uint NB_SHAREDLIST_CELLS = 500000; // property+list container footprint with data size of 32 bit and 500000 rows: 5.8 MB

#else

typedef uint16 TSharedListRow;
const TSharedListRow INVALID_SHAREDLIST_ROW = ((uint16)~0)-1;
const uint NB_SHAREDLIST_CELLS = INVALID_SHAREDLIST_ROW; // property+list container footprint with data size of 32 bit and 500000 rows: 1.3 MB

#endif

extern const char *ListRowSizeString;

/// Mirror Traffic Reduction Tag.
typedef sint8 TMTRTag;

/// Default MTR Tag for a service that want all data
const TMTRTag AllTag = 0;

/// Init MTR Tag
const TMTRTag IniTag = -1;

/// MTR Tag compatible with no other tag
const TMTRTag ExcludedTag = 127;

#endif
