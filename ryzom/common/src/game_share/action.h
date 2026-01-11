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

#ifndef NL_ACTION_H
#define NL_ACTION_H

//
// Includes
//

#include <nel/misc/types_nl.h>
#include <nel/misc/bit_mem_stream.h>
#include <nel/misc/vector.h>

#include "entity_types.h"
#include "ryzom_entity_id.h"


namespace CLFECOMMON {

//
// Action code type
//

typedef	uint8	TActionCode;

//
// Action codes
//

// 0->3 are special action coded in few bits

const TActionCode	ACTION_POSITION_CODE = 0;
const TActionCode	ACTION_GENERIC_CODE = 1;
const TActionCode	ACTION_GENERIC_MULTI_PART_CODE = 2;
const TActionCode	ACTION_SINT64 = 3;

const TActionCode	ACTION_SYNC_CODE = 10;
const TActionCode	ACTION_DISCONNECTION_CODE = 11;
const TActionCode	ACTION_ASSOCIATION_CODE = 12;
const TActionCode	ACTION_LOGIN_CODE = 13;

const TActionCode	ACTION_TARGET_SLOT_CODE = 40;

const TActionCode	ACTION_DUMMY_CODE = 99;


#undef TEST_POSITION_CORRECTNESS


//
// Classes
//

class CAction
{
public:
/*
	union TValue
	{
	public:
		TValue() {}
//		TValue(double d) : Double(d) {}
//		TValue(float f) : Float(f) {}
		TValue(sint32 i) : Int(i) {}
		TValue(uint64 u) : UInt64(u) {}
		TValue(sint64 s) : SInt64(s) {}
//		TValue(float x, float y, float z) { Vector.x = x; Vector.y = y; Vector.z = z; }
//		TValue(const NLMISC::CVector& v ) { Vector.x = v.x; Vector.y = v.y; Vector.z = v.z; }
	public:
//		NLMISC::CVector				getVector() const { return NLMISC::CVector(Vector.x, Vector.y, Vector.z); }
//		float						getFloat() const { return Float; }
//		double						getDouble() const { return Double; }
		sint32						getInt() const { return Int; }
		uint64						getUInt64() const { return UInt64; }
		sint64						getInt64() const  { return SInt64; }
	public:
//		struct { float x, y, z; }	Vector;
//		float						Float;
//		double						Double;
		sint32						Int;
		uint64						UInt64;
		sint64						SInt64;
	};
*/

	typedef sint64	TValue;

public:

	virtual ~CAction() {}

	/** This function creates initializes its fields using the buffer.
	 * \param buffer pointer to the buffer where the data are
	 * \size size of the buffer
	 */
	virtual void	unpack (NLMISC::CBitMemStream &/* message */) { }

	/// This functions is used when you want to transform an action into an IStream.
	virtual void	serial (NLMISC::IStream &/* f */) { }

	/** Returns the size of this action when it will be send to the UDP connection:
	 * the size is *IN BITS*, not in bytes (the actual size is this one plus the header size)
	 */
	virtual uint32	size () { return 0; }

	/// Returns the priority of this action, it can changed dynamically if you want (1 is very urgent)
	uint32			priority () { return _Priority; }

	/// Sets the priority
	void			setPriority( uint32 prio ) { _Priority = prio; }

	/// Sets the value of the action
	virtual void	setValue(const TValue &/* value */) { }

	/// Sets the value of the action
	virtual TValue	getValue() const { return (CAction::TValue)0; }

	/// Returns true if the property is continuous
	virtual bool	isContinuous() const { return false; }

	/// This value must be set (in ms) in the ctor of your action to know when this action must be resent to the other side.
	uint32				Timeout; // TODO: remove?
	/// Gamecycle to measure time between sending and ack (impulsions only)
	NLMISC::TGameCycle	GameCycle; // TEMP?

	TActionCode			Code;
	TPropIndex			PropertyCode;
	TCLEntityId			Slot;

protected:

	/// Default ctor that initialize Timeout value
	CAction ();

	/** This function transform the internal field and transform them into a buffer for the UDP connection.
	 * \param buffer pointer to the buffer where the data will be written
	 * \size size of the buffer
	 */
	virtual void pack (NLMISC::CBitMemStream &/* message */) { }

	/** This method intialises the action with a default state */
	virtual void reset() { }

	friend class CActionFactory;

private:

	uint32	_Priority;
};


class CActionImpulsion : public CAction
{
public:
	virtual ~CActionImpulsion()	{}
	bool	AllowExceedingMaxSize;
};


extern const CAction::TValue	NullValue;

}

#endif // NL_ACTION_H

/* End of action.h */
