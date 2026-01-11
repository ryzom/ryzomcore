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



#ifndef NL_ACTION_SINT64_H
#define NL_ACTION_SINT64_H

#include "nel/misc/types_nl.h"
#include "action.h"


namespace CLFECOMMON {

/**
 * Action containing a SInt64 value.
 * To create such an object, call CActionFactory::create( TProperty, TPropIndex ).
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CActionSint64 : public CAction
{
public:

	static CAction *create () { return new CActionSint64; }

	/** Register a property to set the number of bits
	 * that must be transmitted.
	 */
	static void registerNumericProperty( TPropIndex propIndex, uint nbbits );

	/// TEMP
	static void registerNumericPropertiesRyzom();

	/** This function creates initializes its fields using the buffer.
	 * \param buffer pointer to the buffer where the data are
	 * \size size of the buffer
	 */
	virtual void	unpack (NLMISC::CBitMemStream &message);

	/// This functions is used when you want to transform an action into an IStream.
	virtual void	serial (NLMISC::IStream &f);

	/** Returns the size of this action when it will be send to the UDP connection:
	 * the size is IN BITS, not in bytes (the actual size is this one plus the header size)
	 */
	virtual uint32	size () { return _NbBits; }

	/// Returns the maximum size of this action (INCLUDING the header size handled by CActionFactory!)
	static uint32	getMaxSizeInBit() { return 64; }

	/// Sets the value of the action
	virtual void	setValue(const TValue &value ) { _Value = value; }

	/// Same, but avoids virtual
	void			setValue64(const TValue &value ) { _Value = value; }

	/// Sets the number of bits to transmit
	void			setNbBits( TPropIndex propIndex ) { _NbBits = _PropertyToNbBit[propIndex];
#ifdef NL_DEBUG
	if ( _NbBits == 0 )
		nlwarning( "PropertyToNbBits[%hu] is null, check CNetworkConnection::init()", propIndex );
#endif
	}

	/// Sets the value of the action
	virtual TValue	getValue() const { return _Value; }

	/** Returns false because the action is not a "continuous action".
	 * BUT the property may be continuous, without using the benefits of CContinuousAction (deltas).
	 */
	virtual bool	isContinuous() const { return false; }


	void			setAndPackValue( const TValue& value, NLMISC::CBitMemStream& outMsg )
	{
		_Value = value;
		outMsg.serialAndLog2( _Value, _NbBits );
	}

	void			setAndPackValueNBits( const TValue& value, NLMISC::CBitMemStream& outMsg, uint nbits )
	{
		_Value = value;
		outMsg.serialAndLog2( _Value, nbits );
	}

	void			packFast( NLMISC::CBitMemStream& outMsg )
	{
		outMsg.serialAndLog2( _Value, _NbBits );
	}

private:

	uint64			_Value;
	uint			_NbBits;

	/// Constructor
	CActionSint64() : _Value( (sint64)0 ), _NbBits( 0 ) {}

	/** This function transform the internal field and transform them into a buffer for the UDP connection.
	 * \param buffer pointer to the buffer where the data will be written
	 * \size size of the buffer
	 */
	virtual void pack (NLMISC::CBitMemStream &message);

	/** This method intialises the action with a default state */
	virtual void reset()			{ _Value = 0; _NbBits = 0; };

	friend class CActionFactory;

	/// Init
	static void init();

	static uint		_PropertyToNbBit [MAX_PROPERTIES_PER_ENTITY];

};

}

#endif // NL_ACTION_SINT64_H

/* End of action_sint64.h */
