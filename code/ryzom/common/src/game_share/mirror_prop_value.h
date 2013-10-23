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



#ifndef NL_MIRROR_PROP_VALUE_H
#define NL_MIRROR_PROP_VALUE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "mirrored_data_set.h"
#include <stddef.h>

/**
 * Read-only handler of a value of a property.
 *
 * This class provides a subset of the functionalities
 * of CMirrorPropValue. It does not allow to modify a
 * value in the mirror, but it stores only one pointer
 * and that's all (less memory overhead).
 *
 * Please use CMirrorPropValueRO instead of
 * CMirrorPropValueBase, because its name is
 * better and you can use it with a const
 * dataset argument.
 *
 * \seealso CMirrorPropValueBase
 * \seaalso CMirrorPropValueRO
 * \seealso CMirrorPropValue1DS
 * \seealso CMirrorPropValueUnpacked
 * \seealso CMirrorPropValueAlice
 * \seealso CMirrorPropValueAlice1DS
 * \seealso CMirrorPropValueAliceUnpacked
 * \seealso CMirrorPropValueCF
 * \seealso CMirrorPropValueCF1DS
 * \seealso CMirrorPropValueCFUnpacked
 * \seealso CMirrorPropValueBaseCF
 * \seealso CMirrorPropValueList
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
template <class T>
class CMirrorPropValueBase
{
public:
	/// Default constructor
	CMirrorPropValueBase() : _Pt(NULL) {}

	/// Constructor in mirror
	CMirrorPropValueBase( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow constructor in mirror
	CMirrorPropValueBase( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	/// Init (use only after default construction)
	void						init( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow init (use only after default construction)
	void						init( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	/// Return true if the CMirrorPropValue<T> object is initialized or contains a temporary value (i.e. operator () can be called)
	bool						isReadable() const;

	/// Get the value
	const T&					operator() () const;

	/// Get the value (method provided for compatibility with previous mirror system)
								operator const T& () const { return (*this)(); };

	/// Get the value (method provided for compatibility with previous mirror system)
	const T&					getValue() const { return (*this)(); }

	/// Return an human-readable string (callable if provided by class T)
	std::string					toString() const;

	/** Allocate and store a temporary value, NOT in the mirror. The CMirrorPropValue<T>
	 *  object must be UNitialized (i.e. the default constructor has been called but
	 *  not any other constructor or init()).
	 */
	void						tempStore( const T& srcValue );

	/// Same as tempStore but no assignment is done
	void						tempAllocate();

	/// Return the temporary value stored (you can use operator () instead)
	const T&					tempValue() const;

	/// Delete the temporary value stored if it becomes useless (before a init() for example)
	void						tempDelete();

	/// Unaccess mirror (use only after init() or tempMirrorize()) and store a temporary value
	void						tempRestore( const T& srcValue );

protected:

	/// Pointer to the value in shared memory
	T							*_Pt;
};


/**
 * CMirrorPropValueRO (read-only)
 * Read-only version of CMirrorPropValue.
 * Allows to use a const dataset argument.
 */
template <class T>
class CMirrorPropValueRO : public CMirrorPropValueBase<T>
{
public:

	/// Default constructor
	CMirrorPropValueRO() : CMirrorPropValueBase<T>() {}

	/// Constructor in mirror
	CMirrorPropValueRO( const CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) : CMirrorPropValueBase<T>( const_cast<CMirroredDataSet&>(dataSet), entityIndex, propIndex ) {}

	/// Slow constructor in mirror
	CMirrorPropValueRO( const CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName ) : CMirrorPropValueBase<T>( const_cast<CMirroredDataSet&>(dataSet), entityId, propName ) {}

	/// Init (use only after default construction)
	void						init( const CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) { CMirrorPropValueBase<T>::init( const_cast<CMirroredDataSet&>(dataSet), entityIndex, propIndex ); }

	/// Slow init (use only after default construction)
	void						init( const CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName ) { CMirrorPropValueBase<T>::init( const_cast<CMirroredDataSet&>(dataSet), entityId, propName ); }
};


/**
 * Location of a value of a property (which dataset, which entity, which property).
 * This class should not be instanciated on its own but used inside CMirrorPropValue.
 *
 * This class is a simpler version of CPropLocationPacked.
 * The memory usage is either 8 or 10 bytes.
 * All the method implementations are inline.
 */
class CPropLocationUnpacked
{
public:

	/// Constructor
	CPropLocationUnpacked( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex );

	/// Initialize
	void				init( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex );

	/// Go back to default state (deinit)
	void				reset();

	/// Get dataset
	CMirroredDataSet	*dataSet() const;

	/// Get entity index
	TDataSetRow			dataSetRow() const;

	/// Get property index
	TPropertyIndex		propIndex() const;

#ifdef NL_DEBUG
	/// Return true if the specified property must be read-only (debug feature)
	bool				isReadOnly() const;

	/// Set changed, test if row is valid, and do monitoring
	void				setChanged( const char *valueStr );
#else
	/// Set changed
	void				setChanged();
#endif

	/// Return the location as a string
	std::string			toString() const;

	/// Default constructor
	CPropLocationUnpacked()
	{
		reset();
	}

protected:

//#ifdef NL_DEBUG
	/// Return true if the row is valid for writing/reading
	bool				rowIsUsed() const;

	/// Display the new value when it has changed, if monitoring is enabled for the property
	void				monitorValue( const char *valueStr ) const;
//#endif

private:

	CMirroredDataSet	*_DataSet;
	TDataSetRow			_DataSetRow;
	TPropertyIndex		_PropIndex;
};


#ifdef CHECK_DATASETROW_VALIDITY // does not support packed prop locations because needs to store removal counter in datasetrow

template <int N>
class CPropLocationPacked : public CPropLocationUnpacked
{
public:
	/// Constructor
	CPropLocationPacked( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex ) : CPropLocationUnpacked( dataSet, propIndex, entityIndex ) {}

	CPropLocationPacked() : CPropLocationUnpacked() {}
};

#else

/**
 * Location of a value of a property (which dataset, which entity, which property).
 * This class should not be instanciated on its own but used inside CMirrorPropValue.
 *
 * The template parameter tells the number of bits to use to store the identifier of the dataset.
 * The size of the property index is then (16 - 4 - nbBitsForDataset) bits.
 * For example, if nbBitsForDataset is 3 (8 datasets at most), you can have up to 2^9 = 512 properties per dataset.
 * The datasetrow is stored on 20 bits (i.e. no more than 1,048,573 rows allowed).
 * The memory usage of this class is 4 bytes.
 *
 * All the method implementations are inline.
 *
 * \seeaslo CPropLocation1DS
 * \seealso CPropLocationUnpacked
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002, 2003
 */
template <int nbBitsForDataset>
class CPropLocationPacked
{
public:

	/// Constructor
	CPropLocationPacked( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex );

	/// Initialize
	void				init( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex );

	/// Get dataset
	CMirroredDataSet	*dataSet() const;

	/// Get entity index
	TDataSetRow			dataSetRow() const;

	/// Get property index
	TPropertyIndex		propIndex() const;

//#ifdef NL_DEBUG
	/// Return true if the specified property must be read-only (debug feature)
	bool				isReadOnly() const;

	/// Set changed, test if row is valid, and do monitoring
	void				setChanged( const char *valueStr );
/*#else
	/// Set changed
	void				setChanged();
#endif*/

	/// Default constructor
	CPropLocationPacked()
	{
		reset();
	}

protected:

//#ifdef NL_DEBUG
	/// Return true if the row is valid for writing/reading
	bool				rowIsUsed() const;

	/// Display the new value when it has changed, if monitoring is enabled for the property
	void				monitorValue( const char *valueStr ) const;
///#endif

private:

	TDataSetRow16		_DataSetRow16;
	uint16				_PropIndexAndDataSetAndDSR4;
};

#endif

#ifdef CHECK_DATASETROW_VALIDITY

#define CPropLocationPacked1DS CPropLocationUnpacked

#else

/**
 * Location of a value of a property (which dataset, which entity, which property).
 * This class should not be instanciated on its own but used inside CMirrorPropValue.
 *
 * This class is a speed optimization of CPropLocationPacked when there is only one
 * dataset. The memory usage is 4 bytes as well. The max numbers are the same.
 * All the method implementations are inline.
 *
 * \seealso CMirrorPropValue1DS
 * \seealso CMirrorPropValueAlice1DS
 * \seealso CMirrorPropValueCF1DS
 */
class CPropLocationPacked1DS
{
public:

	/// Constructor
	CPropLocationPacked1DS( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex );

	/// Initialize
	void				init( CMirroredDataSet& dataSet, TPropertyIndex propIndex, const TDataSetRow& entityIndex );

	/// Go back to default state (deinit)
	void				reset();

	/// Get dataset
	CMirroredDataSet	*dataSet() const;

	/// Get entity index
	TDataSetRow			dataSetRow() const;

	/// Get property index
	TPropertyIndex		propIndex() const;

#ifdef NL_DEBUG
	/// Return true if the specified property must be read-only (debug feature)
	bool				isReadOnly() const;

	/// Set changed, test if row is valid, and do monitoring
	void				setChanged( const char *valueStr );
#else
	/// Set changed
	void				setChanged();
#endif

	/// Default constructor
	CPropLocationPacked1DS()
	{
		reset();
	}

protected:

//#ifdef NL_DEBUG
	/// Return true if the row is valid for writing/reading
	bool				rowIsUsed() const;

	/// Display the new value when it has changed, if monitoring is enabled for the property
	void				monitorValue( const char *valueStr ) const;
//#endif

private:

	TDataSetRow16		_DataSetRow16;
	uint16				_PropIndexAndDSR4;
};

#endif


/**
 * Handler of a value of a property.
 *
 * This class allows to store a reference to a value in the mirror.
 * The initialization can be done in the constructor (with arguments)
 * or in init() after a default construction.
 *
 * Storing a value before the entity is added into the mirror:
 * In the latter case, the class can store a temporary
 * value, while the mirror is not ready. When the mirror is ready, you
 * can put the temporary value into the mirror with tempMirrorize().
 * (then this method replaces the call to init()). Don't destroy a
 * CMirrorPropValue object containing a temporary value without
 * calling tempDelete() before. To have a class which "knows" if
 * the value is local (in temp storage) or in mirror, see the class
 * CMirrorPropValueAlice.
 *
 * Atomicity:
 * If your template type is a class containing several members (e.g.
 * a struct), you shouldn't assign the members independantly. Instead,
 * set up a temporary object, assign its members, then copy it into
 * the CMirrorPropValue object. In other words, the atomicity of
 * assignment in a CMirrorPropValue is needed but not provided, you
 * have to handle it.
 *
 * Setting values:
 * You can activate or not the define MIRROR_ASSIGN_ONLY_IF_CHANGED
 * in mirror_prop_value_inline.h if you want the assignment (operator=)
 * to set a new value only if it is different from the previous one.
 * This will add a comparison, but will prevent useless transfers if
 * you often set a property to the same value twice or more.
 *
 * Memory overhead:
 * In addition to the pointer stored in the base class, this class
 * contains information on the location in the mirror. Its size
 * depend on the template class passed in CPropLocationClass.
 * The default packed class uses 4 bytes, therefore the total is 8
 * bytes on a 32-bit architecture.
 * See also CMirrorPropValue1DS (more efficient when you have loaded
 * only one dataset) and CMirrorPropValueUnpacked below.
 * If you need only a read-only handler (if your service will not
 * modify the value of a property), consider using the class
 * CMirrorPropValueBase instead of CMirrorPropValue.
 *
 * All the method implementations are inline.
 *
 * \seealso CMirrorPropValueBase
 * \seealso CMirrorPropValue1DS
 * \seealso CMirrorPropValueUnpacked
 * \seealso CMirrorPropValueAlice
 * \seealso CMirrorPropValueAlice1DS
 * \seealso CMirrorPropValueAliceUnpacked
 * \seealso CMirrorPropValueCF
 * \seealso CMirrorPropValueCF1DS
 * \seealso CMirrorPropValueCFUnpacked
 * \seealso CMirrorPropValueBaseCF
 * \seealso CMirrorPropValueList
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002, 2003
 */
template <class T, class CPropLocationClass = CPropLocationUnpacked >
class CMirrorPropValue : public CMirrorPropValueBase<T>
{
public:

	/// Default constructor
	CMirrorPropValue() : CMirrorPropValueBase<T>(), _PropLocation() {}

	/// Constructor in mirror
	CMirrorPropValue( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow constructor in mirror
	CMirrorPropValue( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	/// Init (after default construction). If you have stored a temp value, call tempDelete() before.
	void						init( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow init (after default construction). If you have stored a temp value, call tempDelete() before.
	void						init( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	/// Return true if the CMirrorPropValue<T> object is initialized or contains a temporary value (i.e. operator () can be called)
	bool						isReadable() const;

	/// Return true if the CMirrorPropValue<T> object is initialized in the mirror
	bool						isInitialized() const;

	/// Get the value
	const T&					operator() () const;

	/// Get the value (method provided for compatibility with previous mirror system)
								operator const T& () const { return (*this)(); };

	/// Get the value (method provided for compatibility with previous mirror system)
	const T&					getValue() const { return (*this)(); }

	/// Set the value. If T is a struct/union, you can set only a member with the macro SET_STRUCT_MEMBER
	CMirrorPropValue&			operator= ( const T& srcValue );

	/// Adapter for assignment
	CMirrorPropValue&			operator= ( const CMirrorPropValue& src ) { return operator=( src() ); }

	/// Adapter for assignment
	CMirrorPropValue&			operator= ( const CMirrorPropValueBase<T>& src ) { return operator=( src() ); }

	/// Return the timestamp of the property change
	NLMISC::TGameCycle			getTimestamp() const;

#ifdef STORE_CHANGE_SERVICEIDS
	/// Return the id of the latest service who changed the property value
	NLNET::TServiceId8					getWriterServiceId() const;
#endif

	/// Return the location
	const CPropLocationClass&	location() const;

	/// Serial from/to mirror
	void						serial( NLMISC::IStream& s );

	/** Special serial: if Reading, reassign in Temp storage (off-mirror), if Writing, get from Mirror or temp storage.
	 *  Precondition: if reading, tempStore() must have been called before and not tempMirrorize() yet.
	 *  Can be used to save a player state / revert several saves to choose which one to load.
	 */
	void						serialRTWM( NLMISC::IStream& s );

	/// Return an human-readable string (callable if provided by class T)
	std::string					toString() const;



	/** Allocate and store a temporary value, NOT in the mirror. The CMirrorPropValue<T>
	 *  object must be UNitialized (i.e. the default constructor has been called but
	 *  not any other constructor or init()).
	 */
	void						tempStore( const T& srcValue );

	/// Same as tempStore but no assignment is done
	void						tempAllocate();

	/// Assign a new value to the temporary value already allocated by tempStore() (you canNOT use operator = instead)
	void						tempReassign( const T& srcValue );

	/// Return the temporary value stored (you can use operator () instead)
	const T&					tempValue() const;

	/// Delete the temporary value stored if it becomes useless (before a init() for example)
	void						tempDelete();

	/// Unaccess mirror (use only after init() or tempMirrorize()) and store a temporary value
	void						tempRestore( const T& srcValue );

	/** Init the CMirrorPropValue<T> object
	 *  and move the temporary value stored to the new place in the mirror
	 *  and delete the temporary value (for a read-only property, call tempDelete() then init() instead)
	 */
	void						tempMirrorize( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/** Init the CMirrorPropValue<T> object (slow version)
	 *  and move the temporary value stored to the new place in the mirror
	 *  and delete the temporary value (for a read-only property, call tempDelete() then init() instead)
	 */
	void						tempMirrorize( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );


	// TODO: debug features

public: // but not recommended unless you know what you are doing

	/// Method provided for setting the members of a struct using the macro SET_STRUCT_MEMBER (setChanged() must be called after the method!)
	T&							directAccessForStructMembers();

	/// Method provided for setting the members of a struct using the macro SET_STRUCT_MEMBER
	void						setChanged();

protected:

	/// Information on the location of the value
	CPropLocationClass			_PropLocation;

	//friend T&	__dafs( CMirrorPropValue<T,CPropLocationClass>& mpv );
	//friend void __sc( CMirrorPropValue<T,CPropLocationClass>& mpv );
};


// Deprecated: Internal methods, use at your own risk
//template<class T, class CPropLocationClass> T&		__dafs( CMirrorPropValue<T,CPropLocationClass>& mpv );
//template<class T, class CPropLocationClass> void	__sc( CMirrorPropValue<T,CPropLocationClass>& mpv );


/** Macro to assign a member of a struct property value.
 * Example:
 *
 *   struct TMyStruct
 *   {
 *      uint32 Number;
 *   };
 *
 *   CMirrorPropValue<TMyStruct> myStruct;
 *
 *   (...)
 *
 *   // Set myStruct.Number to 4 into the mirror
 *   SET_STRUCT_MEMBER( myStruct, Number, 4 );
 */
#define SET_STRUCT_MEMBER( mirrorPropValue, structMember, srcValue ) \
	mirrorPropValue.directAccessForStructMembers().structMember = srcValue; \
	mirrorPropValue.setChanged();

/**
 * Same as SET_STRUCT_MEMBER but in the temp storage (before init)
 */
#define TEMP_REASSIGN_STRUCT_MEMBER( mirrorPropValue, structMember, srcValue ) \
	mirrorPropValue.directAccessForStructMembers().structMember = srcValue; \


/// Alias for CMirrorPropValue1DS (optimization for a single dataset loaded)
template <class T>
class CMirrorPropValue1DS : public CMirrorPropValue< T, CPropLocationPacked1DS >
{
public:
	CMirrorPropValue1DS() : CMirrorPropValue<T,CPropLocationPacked1DS>() {}
	CMirrorPropValue1DS( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) :
	CMirrorPropValue<T,CPropLocationPacked1DS>( dataSet, entityIndex, propIndex ) {}
	CMirrorPropValue1DS( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName ) :
	CMirrorPropValue<T,CPropLocationPacked1DS>( dataSet, entityId, propName ) {}
	CMirrorPropValue1DS&			operator= ( const T& srcValue ) { CMirrorPropValue<T,CPropLocationPacked1DS>::operator=(srcValue); return *this; }
	CMirrorPropValue1DS&			operator= ( const CMirrorPropValue1DS& srcValue ) { CMirrorPropValue<T,CPropLocationPacked1DS>::operator=(srcValue); return *this; }
	CMirrorPropValue1DS&			operator= ( const CMirrorPropValue< T, CPropLocationPacked1DS >& srcValue ) { CMirrorPropValue<T,CPropLocationPacked1DS>::operator=(srcValue); return *this; }
	CMirrorPropValue1DS&			operator= ( const CMirrorPropValueBase<T>& srcValue ) { CMirrorPropValue<T,CPropLocationPacked1DS>::operator=(srcValue); return *this; }
};

// Could do the same with CMirrorPropValueUnpacked if it was needed


/**
 * In CMirrorPropValue, the value can be accessed or written only when it is mirrorized,
 * or you must use explicitely the temp-storage functions. The class CMirrorPropValueAlice
 * allows to use the same common access/assignment methods whether the value is mirrorized
 * or not (in temp storage). "Alice" stands for the ability to pass through the mirror as
 * in Alice in Wonderland ;-).
 *
 * Memory overhead:
 * sizeof(CMirrorPropValue<T>) + sizeof(bool)
 *
 * TODO: now, CPropLocationPacked is not packed anymore. It means we could probably use the
 * dataset pointer to replace the bool _InMirror.
 *
 * \seealso CMirrorPropValueBase
 * \seealso CMirrorPropValue1DS
 * \seealso CMirrorPropValueUnpacked
 * \seealso CMirrorPropValueAlice
 * \seealso CMirrorPropValueAlice1DS
 * \seealso CMirrorPropValueAliceUnpacked
 * \seealso CMirrorPropValueCF
 * \seealso CMirrorPropValueCF1DS
 * \seealso CMirrorPropValueCFUnpacked
 * \seealso CMirrorPropValueBaseCF
 * \seealso CMirrorPropValueList
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002, 2003
 */
template <class T, class CPropLocationClass=CPropLocationPacked<2> >
class CMirrorPropValueAlice : protected CMirrorPropValue<T, CPropLocationClass>
{
public:

	/// Default constructor (value not mirrorized yet)
	CMirrorPropValueAlice() : CMirrorPropValue<T,CPropLocationClass>(), _InMirror(false) { tempStore(); /*nlinfo( "Alice %p constructed OFF mirror", this );*/ }

	/// Copy constructor: copy location (if in mirror) or value
	CMirrorPropValueAlice( const CMirrorPropValueAlice& src )
	{
		if ( src._InMirror )
		{
			CMirrorPropValueBase<T>::_Pt = src._Pt; // just clone this accessor by pointing to the same place in mirror
		}
		else
		{
			CMirrorPropValueBase<T>::_Pt = NULL;
			tempStore();
			*CMirrorPropValueBase<T>::_Pt = *src._Pt; // duplicate the value
		}
		_InMirror = src._InMirror;
		CMirrorPropValue<T,CPropLocationClass>::_PropLocation = src._PropLocation;
		/*nlinfo( "Copied Alice from %p to %p (%s)", &src, this, _InMirror?"IN":"OFF");*/
	}

	/// Destructor
	~CMirrorPropValueAlice();

	/// Constructor (value mirrorized)
	CMirrorPropValueAlice( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow constructor (value mirrorized)
	CMirrorPropValueAlice( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	/// Init in the mirror (no value assigned) (use only after default construction)
	void						init( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow init in the mirror (no value assigned) (use only after default construction)
	void						init( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	/// Change the location of a property *that is already in mirror*
	void						changeLocation( CMirroredDataSet& dataSet, const TDataSetRow& newEntityIndex, TPropertyIndex newPropIndex );

	/// Return always true
	bool						isReadable() const { return true; }

	/// Return true if the CMirrorPropValue<T> object is initialized in the mirror, false if local
	bool						isInitialized() const;

	/// Get the value
	const T&					operator() () const;

	/// Get the value (method provided for compatibility with previous mirror system)
								operator const T& () const { return (*this)(); };

	/// Get the value (method provided for compatibility with previous mirror system)
	const T&					getValue() const { return (*this)(); }

	/// Set the value. If T is a struct/union, you can set only a member with the macro SET_STRUCT_MEMBER
	CMirrorPropValueAlice&		operator= ( const T& srcValue );

	/// Adapter for assignment: copy the value, not the location
	CMirrorPropValueAlice&		operator= ( const CMirrorPropValueAlice& src ) { return operator=( src() ); }

	/// Adapter for assignment: copy the value, not the location
	CMirrorPropValueAlice&		operator= ( const CMirrorPropValue<T, CPropLocationClass>& src ) { return operator=( src() ); }

	/// Adapter for assignment: copy the value, not the location
	CMirrorPropValueAlice&		operator= ( const CMirrorPropValueBase<T>& src ) { return operator=( src() ); }

	/// Mirrorize (put the local value into the mirror)
	void						tempMirrorize( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Mirrorize (put the local value into the mirror) (slow version)
	void						tempMirrorize( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	/// Return the timestamp of the property change (if in mirror, otherwise return 0)
	NLMISC::TGameCycle			getTimestamp() const;

#ifdef STORE_CHANGE_SERVICEIDS
	/// Return the id of the latest service who changed the property value
	NLNET::TServiceId8					getWriterServiceId() const;
#endif

	/// Return the location
	const CPropLocationClass&	location() const { return CMirrorPropValue<T,CPropLocationClass>::location(); }

	/// Serial
	void						serial( NLMISC::IStream& s );

	/// SerialRTWM
	void						serialRTWM( NLMISC::IStream& s ) { CMirrorPropValue<T,CPropLocationClass>::serialRTWM(s); }

	/// Return an human-readable string (callable if provided by class T)
	std::string					toString() const { return CMirrorPropValue<T,CPropLocationClass>::toString(); }

protected:

	/// Allocate the local storage (no value assigned)
	void						tempStore();

public: // but not recommended unless you know what you are doing

	/// Method provided for setting the members of a struct using the macro SET_STRUCT_MEMBER (setChanged() must be called after the method!)
	T&							directAccessForStructMembers() { return CMirrorPropValue<T,CPropLocationClass>::directAccessForStructMembers(); }

	/// Method provided for setting the members of a struct using the macro SET_STRUCT_MEMBER
	void						setChanged() { CMirrorPropValue<T,CPropLocationClass>::setChanged(); }

	/// DEBUG !!!
	uint8						testFlagInMirror() { return *(uint8*)&_InMirror; }

private:

	/// True if the value is in mirror, false if local
	bool						_InMirror;

	//friend T&	__dafs( CMirrorPropValueAlice<T,CPropLocationClass>& mpv );
	//friend void __sc( CMirrorPropValueAlice<T,CPropLocationClass>& mpv );
};


// Deprecated: Internal methods, use at your own risk
//template<class T, class CPropLocationClass> T&		__dafs( CMirrorPropValueAlice<T,CPropLocationClass>& mpv );
//template<class T, class CPropLocationClass> void	__sc( CMirrorPropValueAlice<T,CPropLocationClass>& mpv );


/// Alias for CMirrorPropValueAlice1DS (optimization for a single dataset loaded)
template <class T>
class CMirrorPropValueAlice1DS : public CMirrorPropValueAlice< T, CPropLocationPacked1DS >
{
public:
	CMirrorPropValueAlice1DS() : CMirrorPropValueAlice<T,CPropLocationPacked1DS>() {}
	CMirrorPropValueAlice1DS( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) :
	  CMirrorPropValueAlice<T,CPropLocationPacked1DS>( dataSet, entityIndex, propIndex ) {}
	CMirrorPropValueAlice1DS( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName ) :
	  CMirrorPropValueAlice<T,CPropLocationPacked1DS>( dataSet, entityId, propName ) {}
	CMirrorPropValueAlice1DS&		operator= ( const T& srcValue ) { CMirrorPropValueAlice<T,CPropLocationPacked1DS>::operator=(srcValue); return *this; }
	CMirrorPropValueAlice1DS&		operator= ( const CMirrorPropValueAlice1DS& src ) { return operator=( src() ); }
	CMirrorPropValueAlice1DS&		operator= ( const CMirrorPropValueAlice< T, CPropLocationPacked1DS >& src ) { return operator=( src() ); }
	CMirrorPropValueAlice1DS&		operator= ( const CMirrorPropValue<T>& src ) { return operator=( src() ); }
	CMirrorPropValueAlice1DS&		operator= ( const CMirrorPropValueBase<T>& src ) { return operator=( src() ); }
};

// Could do the same with CMirrorPropValueAliceUnpacked if it was needed


/**
 * CMirrorPropValueCF is an alternative to using the property change notification methods
 * in CMirroredDataSet. It allows to test changes for the CMirrorPropValueCF objects stored.
 * It also keeps the latest previous value.
 *
 * You can test if the value of the property has changed, since the latest call to resetChangeFlag().
 * Each time and after you set yourself the value or acknowledge a change, call resetChangeFlag().
 * CF stands for "with Changed Flag". You can also get the previous value.
 *
 * If you don't plan to modify the values, consider using CMirrorPropValueBaseCF instead.
 *
 * Memory overhead:
 * sizeof(CMirrorPropValue<T>) + sizeof(T)
 * All the method implementations are inline.
 *
 * \seealso CMirrorPropValueBase
 * \seealso CMirrorPropValue1DS
 * \seealso CMirrorPropValueUnpacked
 * \seealso CMirrorPropValueAlice
 * \seealso CMirrorPropValueAlice1DS
 * \seealso CMirrorPropValueAliceUnpacked
 * \seealso CMirrorPropValueCF
 * \seealso CMirrorPropValueCF1DS
 * \seealso CMirrorPropValueCFUnpacked
 * \seealso CMirrorPropValueBaseCF
 * \seealso CMirrorPropValueList
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002, 2003
 */
template <class T, class CPropLocationClass=CPropLocationPacked<2> >
class CMirrorPropValueCF : public CMirrorPropValue<T, CPropLocationClass>
{
public:

	/// Default constructor
	CMirrorPropValueCF() : CMirrorPropValue<T>(), _PreviousValue() {}

	/// Constructor
	CMirrorPropValueCF( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow constructor
	CMirrorPropValueCF( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	/// Return true if the value has been set since the latest call to resetChangeFlag()
	bool						hasChanged() const;

	/// Reset the change flag(). Call it after setting or reading the value.
	void						resetChangeFlag();

	/// Return the previous value (call this method before resetChangeFlag())
	T							previousValue() const;

	CMirrorPropValueCF&			operator= ( const CMirrorPropValueCF& src ) { return operator=( src() ); }
	CMirrorPropValueCF&			operator= ( const CMirrorPropValue<T, CPropLocationClass>& src ) { return operator=( src() ); }
	CMirrorPropValueCF&			operator= ( const CMirrorPropValueBase<T>& src ) { return operator=( src() ); }

	// See also the methods in the base class CMirrorPropValue

private:

	/** The previous value known is stored. If we stored the timestamp instead, we could not see a change
	 * if it occured after but at the same cycle as resetChangeFlag().
	 */
	T							_PreviousValue;
};


/// Alias for CMirrorPropValueCF1DS (optimization for a single dataset loaded)
template <class T>
class CMirrorPropValueCF1DS : public CMirrorPropValueCF< T, CPropLocationPacked1DS >
{
public:
	CMirrorPropValueCF1DS() : CMirrorPropValueCF<T,CPropLocationPacked1DS>() {}
	CMirrorPropValueCF1DS( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex ) :
	  CMirrorPropValueCF<T,CPropLocationPacked1DS>( dataSet, entityIndex, propIndex ) {}
	CMirrorPropValueCF1DS( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName ) :
	  CMirrorPropValueCF<T,CPropLocationPacked1DS>( dataSet, entityId, propName ) {}
	CMirrorPropValueCF1DS&			operator= ( const T& srcValue ) { CMirrorPropValueCF<T,CPropLocationPacked1DS>::operator=(srcValue); return *this; }
	CMirrorPropValueCF1DS&			operator= ( const CMirrorPropValueCF1DS& src ) { return operator=( src() ); }
	CMirrorPropValueCF1DS&			operator= ( const CMirrorPropValueCF< T, CPropLocationPacked1DS >& src ) { return operator=( src() ); }
	CMirrorPropValueCF1DS&			operator= ( const CMirrorPropValue< T, CPropLocationPacked1DS >& src ) { return operator=( src() ); }
	CMirrorPropValueCF1DS&			operator= ( const CMirrorPropValueBase<T>& src ) { return operator=( src() ); }
};

// Could do the same with CMirrorPropValueCFUnpacked if it was needed


/**
 * Read-only variation of CMirrorPropValueCF (less memory overhead).
 * See documentation of CMirrorPropValueCF.
 *
 * Memory overhead:
 * sizeof(CMirrorPropValueBase<T>) + sizeof(T)
 *
 * \seealso CMirrorPropValueBase
 * \seealso CMirrorPropValueCF
 */
template <class T>
class CMirrorPropValueBaseCF : public CMirrorPropValueBase<T>
{
public:

	/// Default constructor
	CMirrorPropValueBaseCF() : CMirrorPropValueBase<T>(), _PreviousValue() {}

	/// Constructor
	CMirrorPropValueBaseCF( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow constructor
	CMirrorPropValueBaseCF( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	/// Return true if the value has been set since the latest call to resetChangeFlag()
	bool						hasChanged() const;

	/// Reset the change flag(). Call it after setting or reading the value.
	void						resetChangeFlag();

	/// Return the previous value (call this method before resetChangeFlag())
	T							previousValue() const;

	// See also the methods in the base class CMirrorPropValueBase

private:

	/** The previous value known is stored. If we stored the timestamp instead, we could not see a change
	 * if it occured after but at the same cycle as resetChangeFlag().
	 */
	T							_PreviousValue;
};


/// Alias for CMirrorPropValueROCF
#define CMirrorPropValueROCF CMirrorPropValueBaseCF


template <class T, class CPropLocationClass>
class CMirrorPropValueList;


/**
 * Handler of a value of a list.
 * Use the methods of CMirrorPropValueList<T> to obtain a CMirrorPropValueItem object.
 *
 * \seealso CMirrorPropValueList
 */
template <class T, class CPropLocationClass>
class CMirrorPropValueItem
{
public:

	/// Special constructor for evaluating values
	CMirrorPropValueItem( const T& value ) : _ParentList(NULL) { _Pt = const_cast<T*>(&value); }

	/// Constructor
	CMirrorPropValueItem( CMirrorPropValueList<T,CPropLocationClass> *parentlist, T *pt ) : _ParentList(parentlist), _Pt(pt) {}

	/// Get the value
	const T&					operator() () const;

	/// Set the value
	CMirrorPropValueItem&		operator= ( const T& srcValue );

	/// Equal operator
	bool						operator== ( const CMirrorPropValueItem<T,CPropLocationClass>& other ) const
	{
		return (*this)() == other();
	}

	/// Comparison operator
	bool						operator< ( const CMirrorPropValueItem<T,CPropLocationClass>& other ) const
	{
		return (*this)() < other();
	}

protected:

	CMirrorPropValueList<T,CPropLocationClass>		*_ParentList;
	T												*_Pt;
};

#if NL_ISO_SYNTAX
template <>
class CMirrorPropValueItem<NLMISC::CEntityId, CPropLocationUnpacked>
{
public:

  typedef CPropLocationUnpacked CPropLocationClass;

	/// Special constructor for evaluating values
	CMirrorPropValueItem( const uint64& value ) : _ParentList(NULL) { _Pt = const_cast<uint64*>(&value); }

	/// Constructor
	CMirrorPropValueItem( CMirrorPropValueList<NLMISC::CEntityId,CPropLocationClass> *parentlist, uint64 *pt ) : _ParentList(parentlist), _Pt(pt) {}

	/// Get the value
	const NLMISC::CEntityId&					operator() () const;

	/// Set the value
	CMirrorPropValueItem&		operator= ( const NLMISC::CEntityId& srcValue );

	/// Equal operator
	bool						operator== ( const CMirrorPropValueItem<NLMISC::CEntityId,CPropLocationClass>& other ) const
	{
		return (*this)() == other();
	}

	/// Comparison operator
	bool						operator< ( const CMirrorPropValueItem<NLMISC::CEntityId,CPropLocationClass>& other ) const
	{
		return (*this)() < other();
	}

protected:

	CMirrorPropValueList<NLMISC::CEntityId,CPropLocationClass>		*_ParentList;
	uint64												*_Pt;
};
#endif

/*
 * CMirrorPropValueList::iterator
 */
template <class T, class CPLC>
struct _CMirrorPropValueListIterator
{
	CMirrorPropValueItem<T,CPLC>	operator*() const		{ return CMirrorPropValueItem<T,CPLC>( _ParentList, &(_ParentList->_Container[_Index].Value) ); }
	_CMirrorPropValueListIterator&	operator++()			{ _Index = _ParentList->_Container[_Index].Next; return *this; }
	_CMirrorPropValueListIterator	operator++(int)			{ _CMirrorPropValueListIterator tmp = *this; _Index = _ParentList->_Container[_Index].Next; return tmp; }
	bool							operator==( const _CMirrorPropValueListIterator& other ) const	{ return (_Index == other._Index) && (_ParentList == other._ParentList); }
	bool							operator!=( const _CMirrorPropValueListIterator& other ) const	{ return (_Index != other._Index) || (_ParentList != other._ParentList); }
	_CMirrorPropValueListIterator&	operator=( const _CMirrorPropValueListIterator& other )			{ _ParentList = other._ParentList; _Index = other._Index; return *this; }

	_CMirrorPropValueListIterator()	: _ParentList(NULL), _Index(INVALID_SHAREDLIST_ROW) {}
	CMirrorPropValueList<T,CPLC>	*_ParentList;
	TSharedListRow					_Index;

	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	// typedef typename std::forward_iterator_tag iterator_category;
};


/*
 * CMirrorPropValueList::const_iterator
 */
template <class T, class CPLC>
struct _CCMirrorPropValueListIterator
{
	const CMirrorPropValueItem<T,CPLC>	operator*() const											{ return CMirrorPropValueItem<T,CPLC>( _ParentList, &(_ParentList->_Container[_Index].Value) ); }
	_CCMirrorPropValueListIterator&	operator++()													{ _Index = _ParentList->_Container[_Index].Next; return *this; }
	_CCMirrorPropValueListIterator	operator++(int)													{ _CCMirrorPropValueListIterator tmp = *this;	_Index = _ParentList->_Container[_Index].Next; return tmp; }
	bool							operator==( const _CCMirrorPropValueListIterator& other ) const	{ return (_Index == other._Index) && (_ParentList == other._ParentList); }
	bool							operator!=( const _CCMirrorPropValueListIterator& other ) const	{ return (_Index != other._Index) || (_ParentList != other._ParentList); }
	_CCMirrorPropValueListIterator&	operator=( const _CCMirrorPropValueListIterator& other )		{ _ParentList = other._ParentList; _Index = other._Index; return *this; }

	_CCMirrorPropValueListIterator() : _ParentList(NULL), _Index(INVALID_SHAREDLIST_ROW) {}
	CMirrorPropValueList<T,CPLC>	*_ParentList;
	TSharedListRow					_Index;

	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	// typedef typename std::forward_iterator_tag iterator_category;
};


/**
 * Handler of a single-linked list.
 * The interface is similar to the one of STLPort's class slist.
 *
 * Note: currently, the maximum number of items, shared by all the lists within one property, is 65535.
 *
 * Performance note: try to share such list properties only among the same physical machine, because it is designed
 * for shared memory; it will be very slow when travelling between two machines.
 *
 * IMPORTANT: With GCC 3.4 and higher, any non-pod type (e.g. a class) is prohibited as T.
 * It would lead to a warning "ignoring packed attribute on unpacked non-POD field".
 *
 * Warning: the following case may lead to orphan lists then the container may get full fast.
 * - Service A is in charge of spawning/despawning entities of type T.
 * - Service B writes some lists of property P.
 * - Service A does not declare property P (even in read mode).
 * In this case, service B *must* clear the lists after use, because the mirror of service A won't
 * clear the lists when reassigning a row because it is not aware of the list property.
 * If service B is closed before it has cleared all lists, some lists may remain as orphan lists.
 * There is a way to cope with this case:
 * At the next startup of service B, when B detects that the mirror of A is up (see CMirror::
 * setServiceMirrorUpCallback()), B can scan the rows of the ranges of B (see CMirroredDataSet::
 * getDeclaredEntityRanges()) and, on each non-empty list 'orphanList', call CMirroredDataSet::
 * reportOrphanSharedListCells(orphanList.size()) and then call orphanList.clear().
 * Sample code:
	void cleanOrphanTargetLists( uint16 serviceIdOfA )
	{
		const TDeclaredEntityRangeOfType& declERT = TheDataset.getDeclaredEntityRanges();

		uint nbCleanedTargetLists = 0, nbCleanedCells = 0;
		uint nbPreviousKnownCells = CMirroredDataSet::getNbKnownSharedListCells();
		const uint NbTypesOfEntitiesNotSpawnedByB = 2;
		RYZOMID::TTypeId typesOfEntitiesNotSpawnedByB [NbTypesOfEntitiesNotSpawnedByB] = { RYZOMID::npc, RYZOMID::creature };
		for ( uint i=0; i!=NbTypesOfEntitiesNotSpawnedByB; ++i )
		{
			// Get all ranges of the selected entity type
			pair<TDeclaredEntityRangeOfType::const_iterator,TDeclaredEntityRangeOfType::const_iterator>
				itPair = declERT.equal_range( typesOfEntitiesNotSpawnedByB[i] );
			for ( TDeclaredEntityRangeOfType::const_iterator it=itPair.first; it!=itPair.second; ++it )
			{
				// If the range belongs to the connecting service, scan for its orphan target lists and clear them
				const TDeclaredEntityRange& range = GET_ENTITY_TYPE_RANGE(it);
				if ( range.serviceId() != serviceIdOfA )
					continue;
				for ( TDataSetIndex entityIndex=range.baseIndex(); entityIndex<range.baseIndex()+range.size(); ++entityIndex )
				{
					CMirrorPropValueList<TDataSetRow> targetList( TheDataset, TheDataset.forceCurrentDataSetRow( entityIndex ), DSPropertyTARGET_LIST );
					if ( ! targetList.empty() )
					{
						uint nbCells = targetList.size();
						CMirroredDataSet::reportOrphanSharedListCells( (sint)nbCells );
						targetList.clear();
						++nbCleanedTargetLists;
						nbCleanedCells += nbCells;
					}
				}
			}
		}
		nlinfo( "%u target lists from previous session cleaned at connection of %s (%u cells)",
			nbCleanedTargetLists, CUnifiedNetwork::getInstance()->getServiceUnifiedName( serviceIdOfA ).c_str(),
			nbCleanedCells );
		nlassert( CMirroredDataSet::getNbKnownSharedListCells() == nbPreviousKnownCells );
	}
 *
 * \seealso CMirrorPropValueBase
 * \seealso CMirrorPropValue1DS
 * \seealso CMirrorPropValueUnpacked
 * \seealso CMirrorPropValueAlice
 * \seealso CMirrorPropValueAlice1DS
 * \seealso CMirrorPropValueAliceUnpacked
 * \seealso CMirrorPropValueCF
 * \seealso CMirrorPropValueCF1DS
 * \seealso CMirrorPropValueCFUnpacked
 * \seealso CMirrorPropValueBaseCF
 * \seealso CMirrorPropValueList
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
template <class T, class CPropLocationClass= CPropLocationUnpacked>
class CMirrorPropValueList
{
public:

	typedef uint32													size_type;
	typedef _CMirrorPropValueListIterator<T,CPropLocationClass>		iterator;
	typedef _CCMirrorPropValueListIterator<T,CPropLocationClass>	const_iterator;

#ifdef NL_OS_WINDOWS
	friend											iterator; // MSVC
	friend											const_iterator;
#else
	template <class U, class V> friend				class _CMirrorPropValueListIterator; // GCC3
	template <class U, class V> friend				class _CCMirrorPropValueListIterator;
#endif

	/// Constructor
	CMirrorPropValueList( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow constructor
	CMirrorPropValueList( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	bool											empty() const;
	void											clear( bool hidePropChange=false ); // hidePropChange will not notify the change to other services. Other methods such as push_front() always notify the change.
	CMirrorPropValueItem<T,CPropLocationClass>		front();
	const T&										front() const;
	void											push_front( const T& value );
	void											pop_front(); // assumes !empty()
	size_type										size() const; // linear complexity
	size_type										max_size() const;
	const_iterator									const_begin() const;
	const_iterator									const_end() const;
	iterator										begin();
	iterator										end();
	iterator										erase( iterator pos ); // linear complexity, prefer pop_front() and erase_after()
	iterator										erase_after( iterator pos );
	void											resize( size_type n, T t=T() ); // see doc in mirror_prop_value_inline.h

	void											testList( uint expectedSize ) const;

	/// Return the timestamp of the property change
	NLMISC::TGameCycle								getTimestamp() const;


#pragma pack(push, PACK_SHAREDLIST_CELL, 1)
	struct TSharedListCell
	{
		// If changing this struct, don't forget to change the places where it is initialized
		TSharedListRow				Next;
		T							Value;
#ifdef NL_OS_WINDOWS
	};
#else
	} __attribute__((packed));
#endif
#pragma pack(pop, PACK_SHAREDLIST_CELL)

	/// Return the location
	const CPropLocationClass&		location() const { return _PropLocation; }

	// TODO: debug features

protected:

	/// Default constructor
	CMirrorPropValueList() : _Container(NULL), _PtFront(NULL), _PropLocation() {}

	TSharedListRow		allocateNewCell();

	void				releaseCell( TSharedListRow row );

	/// Pointer to the list container in shared memory
	TSharedListCell		*_Container;

	/// Pointer to the 'first' index in shared memory
	TSharedListRow		*_PtFront;

	/// Information on the location of the value
	CPropLocationClass	_PropLocation;
};

// ------- Partial template specialization for CEntityId, because GCC 3.4 does not support packed structure with non-Pod member ------

#if NL_ISO_SYNTAX
template <>
class CMirrorPropValueList<NLMISC::CEntityId,CPropLocationUnpacked>
{
public:

	typedef CPropLocationUnpacked CPropLocationClass;

	typedef uint32													size_type;
	typedef _CMirrorPropValueListIterator<NLMISC::CEntityId,CPropLocationClass>		iterator;
	typedef _CCMirrorPropValueListIterator<NLMISC::CEntityId,CPropLocationClass>	const_iterator;

#ifdef NL_OS_WINDOWS
	friend											iterator; // MSVC
	friend											const_iterator;
#else
	template <class U, class V> friend				class _CMirrorPropValueListIterator; // GCC3
	template <class U, class V> friend				class _CCMirrorPropValueListIterator;
        template<class U, class V> friend                               class CMirrorPropValueItem;
#endif

	/// Constructor
	CMirrorPropValueList( CMirroredDataSet& dataSet, const TDataSetRow& entityIndex, TPropertyIndex propIndex );

	/// Slow constructor
	CMirrorPropValueList( CMirroredDataSet& dataSet, const NLMISC::CEntityId& entityId, const std::string& propName );

	bool											empty() const;
	void											clear( bool hidePropChange=false ); // hidePropChange will not notify the change to other services. Other methods such as push_front() always notify the change.
	CMirrorPropValueItem<NLMISC::CEntityId,CPropLocationClass>		front();
	NLMISC::CEntityId										front() const;
	void											push_front( const NLMISC::CEntityId& value );
	void											pop_front(); // assumes !empty()
	size_type										size() const; // linear complexity
	size_type										max_size() const;
	const_iterator									const_begin() const;
	const_iterator									const_end() const;
	iterator										begin();
	iterator										end();
	iterator										erase( iterator pos ); // linear complexity, prefer pop_front() and erase_after()
	iterator										erase_after( iterator pos );
	void											resize( size_type n, NLMISC::CEntityId t=NLMISC::CEntityId() ); // see doc in mirror_prop_value_inline.h

	void											testList( uint expectedSize ) const;

	/// Return the timestamp of the property change
	NLMISC::TGameCycle								getTimestamp() const;


#pragma pack(push, PACK_SHAREDLIST_CELL, 1)
	struct TSharedListCell
	{
		// If changing this struct, don't forget to change the places where it is initialized
		TSharedListRow				Next;
		uint64							Value;
#ifdef NL_OS_WINDOWS
	};
#else
	} __attribute__((packed));
#endif
#pragma pack(pop, PACK_SHAREDLIST_CELL)

	/// Return the location
	const CPropLocationClass&		location() const { return _PropLocation; }

	// TODO: debug features

protected:

	/// Default constructor
	CMirrorPropValueList() : _Container(NULL), _PtFront(NULL), _PropLocation() {}

	TSharedListRow		allocateNewCell();

	void				releaseCell( TSharedListRow row );

	/// Pointer to the list container in shared memory
	TSharedListCell		*_Container;

	/// Pointer to the 'first' index in shared memory
	TSharedListRow		*_PtFront;

	/// Information on the location of the value
	CPropLocationClass	_PropLocation;
};
#endif

// Inline definitions
#include "mirror_prop_value_inline.h"


#endif // NL_MIRROR_PROP_VALUE_H

/* End of mirror_prop_value.h */
