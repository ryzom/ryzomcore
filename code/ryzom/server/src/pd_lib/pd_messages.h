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

#ifndef RY_PD_MESSAGES_H
#define RY_PD_MESSAGES_H

/*
 * NeL Includes
 */
#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/time_nl.h>
#include <nel/misc/stream.h>
#include <nel/misc/entity_id.h>
#include <nel/misc/sheet_id.h>
#include <nel/misc/variable.h>
#include <nel/misc/hierarchical_timer.h>

/*
 * PD Lib Includes
 */
#include "pd_utils.h"
#include "db_description_parser.h"
#include "timestamp.h"

namespace RY_PDS
{

#define	MAX_MESSAGE_REMAP					32767
#define MESSAGE_REMAP_MASK					0x7fff
#define MESSAGE_REMAP_ENTITYID_PRESENT		0x8000
#define MESSAGE_SETPARENT_ENTITYID_PRESENT	0x8000


/**
 * Object Circular Mapper
 * Allows to map an object value through little sized uint (uint8 or uint16), allowing to reallocate
 * mapping when all values are used.
 * This is used to map long values (CEntityIds for instance) that are expected to appear frequently
 * in a stream with short keys, without making sure all values fit in mapping table.
 * For instance, if using uint8 as mapping key, when all 256 values are used, previously used key
 * 0 is remapped to the new object value that appears in stream and so on (reallocations are done
 * circularly through key values). Mask value is for test purposes only, not to be changed!
 */
template<typename Key, typename Object, int Mask = 0xffffffff, typename TBackMap = std::map<Object, Key> >
class CObjCircMapper
{
public:

	CObjCircMapper()
	{
		_Next = 0;
		_Max = 0;
		// prepare mapping table
		_FrontMap.resize(getMappingSize()+1);
	}

	/**
	 * Serialise object from stream
	 */
	void	serial(NLMISC::IStream& s, Object& o)
	{
		Key	fakeFlags = 0;
		serial(s, o, fakeFlags);
	}

	/**
	 * Serialise object from stream, with flags added to msbits
	 * WARNING: flags should NEVER interfere with Mask!
	 * WARNING: when stream is reading, lower bits of flags are unspecified!
	 */
	void	serial(NLMISC::IStream& s, Object& o, Key& flags)
	{
		if (s.isReading())
		{
			Key	k;
			s.serial(flags);

			// remove flags from read key
			k = (flags & Mask);

			// if key is next value to be mapped
			if (k == _Next)
			{
				// serial in object value to map
				_Next = ((_Next+1)&Mask);
				s.serial(o);
				// and map it to key
				_FrontMap[k] = o;
			}
			else
			{
				// already seen key? just copy object value
				o = _FrontMap[k];
			}
		}
		else
		{
			// search for object value in map
			typename TBackMap::iterator	it = _BackMap.find(o);
			// not yet found or mapping key is just next key to alloc
			if (it == _BackMap.end() || (*it).second == _Next)
			{
				// if mapping key is next, we have to force reallocation
				// as serial in code can't know if value is new or not...
				Key	k = _Next;
				_Next = ((_Next+1)&Mask);
				// if new key as already circle'd down, unmap previous association
				if (k < _Max)
				{
					#ifdef NL_DEBUG
					typename TBackMap::iterator	it = _BackMap.find(_FrontMap[k]);
					nlassert(it != _BackMap.end() && (*it).second == k);
					_BackMap.erase(it);
					#else
					_BackMap.erase(_FrontMap[k]);
					#endif
				}
				else
				{
					// else just increase max seen key...
					_Max = ((uint)k)+1;
				}
				// do mapping
				_BackMap[o] = k;
				_FrontMap[k] = o;
				// serial mapping
				k |= (flags & (~Mask));
				s.serial(k);
				s.serial(o);
			}
			else
			{
				// mapping found and correct, only serial key out
				Key	k = ((*it).second | (flags & (~Mask)));
				s.serial(k);
			}
		}
	}

private:

	/// Back Mapping, from object values to keys
	TBackMap	_BackMap;

	/// Front Mapping, from keys to object values
	typedef typename std::vector<Object>		TFrontMap;
	TFrontMap	_FrontMap;

	/// Next Key to map
	Key			_Next;
	/// Max mapped Key
	uint		_Max;

	uint	getMappingSize() const
	{
		return (((uint)1)<<(sizeof(Key)*8))-1;
	}
};

typedef CObjCircMapper<uint8, NLMISC::CEntityId>	TEntityIdCircMapper;


class CMsgObjectIndex
{
public:

	CMsgObjectIndex() : Raw(0)	{}
	CMsgObjectIndex(uint8 table, uint32 row)
	{
		Raw = 0;
		set(table, row);
	}

	void	set(uint8 table, uint32 row)
	{
		Table = table;
		Row = row;
	}

	union
	{
		uint64			Raw;

		struct
		{
			uint32		Row;
			uint8		Table;
		};
	};

	void	serial(NLMISC::IStream& f)	{ f.serial(Table, Row); }

	bool	operator < (const CMsgObjectIndex& a) const
	{
		return Raw < a.Raw;
	}
};

typedef CObjCircMapper<uint8, CMsgObjectIndex, 0x7f>	TObjectIndexCircMapper;

/**
 * Database update message
 */
class CDbMessage
{
public:

	CDbMessage() : Selected(false), ContextDepth(0), _ObjectIdPresent(false)	{ }


	/// Type of message, 4bits -> 16 message types available
	enum THeaderType
	{
		UpdateValue = 0,
		SetParent = 1,
		AllocRow = 2,
		DeallocRow = 3,
		ReleaseRow = 4,

		EndRemapMessages = 4,

		LoadRow = 5,
		AddString = 6,
		UnmapString = 7,

		Log = 8,
		PushContext = 9,
		PopContext = 10,

		LogChat = 11,

		End
	};


	/// \name setup message methods
	// @{

	/// update value
	template<typename T>
	void					updateValue(TColumnIndex column, const T& value)
	{
		setHeader(UpdateValue);

		uint	sz = 0;
				
		// update 20101119 by packpro
		if (sizeof(value) == 1)
		{ 
			sz = 0; 
			memcpy(&(_Value0[0]), &value, sizeof(value));
		}
		else if (sizeof(value) == 2)
		{ 
			sz = 1; 
			memcpy(&(_Value1[0]), &value, sizeof(value));
		}
		else if (sizeof(value) == 4)
		{ 
			sz = 2;
			memcpy(&(_Value2[0]), &value, sizeof(value));
		}
		else if (sizeof(value) == 8)
		{ 
			sz = 3; 
			memcpy(&(_Value3[0]), &value, sizeof(value));
		}

		//if (sizeof(value) == 1)			{ sz = 0; _Value0[0] = *(uint8*)(&value); }
		//else if (sizeof(value) == 2)	{ sz = 1; _Value1[0] = *(uint16*)(&value); }
		//else if (sizeof(value) == 4)	{ sz = 2; _Value2[0] = *(uint32*)(&value); }
		//else if (sizeof(value) == 8)	{ sz = 3; _Value3[0] = *(uint64*)(&value); }
		_ColumnAndSize = (uint16)(column | (sz << 14));
	}

	/// update value
	template<typename T>
	void					updateValue(TColumnIndex column, const T& value, const NLMISC::CEntityId& objectId)
	{
		setHeader(UpdateValue);

		uint	sz;

		// update 20101119 by packpro
		if (sizeof(value) == 1)
		{ 
			sz = 0; 
			memcpy(&(_Value0[0]), &value, sizeof(value));
		}
		else if (sizeof(value) == 2)
		{ 
			sz = 1; 
			memcpy(&(_Value1[0]), &value, sizeof(value));
		}
		else if (sizeof(value) == 4)
		{ 
			sz = 2;
			memcpy(&(_Value2[0]), &value, sizeof(value));
		}
		else if (sizeof(value) == 8)
		{ 
			sz = 3; 
			memcpy(&(_Value3[0]), &value, sizeof(value));
		}
		//if (sizeof(value) == 1)			{ sz = 0; _Value0[0] = *(uint8*)(&value); }
		//else if (sizeof(value) == 2)	{ sz = 1; _Value1[0] = *(uint16*)(&value); }
		//else if (sizeof(value) == 4)	{ sz = 2; _Value2[0] = *(uint32*)(&value); }
		//else if (sizeof(value) == 8)	{ sz = 3; _Value3[0] = *(uint64*)(&value); }

		_ColumnAndSize = (uint16)(column | (sz << 14));

		//_MapTableRow |= MESSAGE_REMAP_ENTITYID_PRESENT;
		_ObjectIdPresent = true;
		_ObjectId = objectId;
	}

	/// set parent
	void					setParent(TColumnIndex column, const CObjectIndex& parent)
	{
		setHeader(SetParent);

		_ColumnAndSize = (uint16)column;
		_Value3[0] = *(uint64*)(&parent);
	}

	/// set parent, only child object has an entityId as key
	void					setParent(TColumnIndex column, const CObjectIndex& parent, const NLMISC::CEntityId& objectId)
	{
		setHeader(SetParent);

		_ColumnAndSize = (uint16)column;
		_Value3[0] = *(uint64*)(&parent);

		//_MapTableRow |= MESSAGE_REMAP_ENTITYID_PRESENT;
		_ObjectIdPresent = true;
		_ObjectId = objectId;
	}

	/// set parent, only parent object has an entityId as key
	void					setParent(TColumnIndex column, const CObjectIndex& parent, const NLMISC::CEntityId& newParentId, const NLMISC::CEntityId& previousParentId)
	{
		setHeader(SetParent);

		_ColumnAndSize = (uint16)column;
		_Value3[0] = *(uint64*)(&parent);

		_ColumnAndSize |= MESSAGE_SETPARENT_ENTITYID_PRESENT;
		_NewParentId = newParentId;
		_PreviousParentId = previousParentId;
	}

	/// set parent, both child and parent objects have an entityId as key
	void					setParent(TColumnIndex column, const CObjectIndex& parent, const NLMISC::CEntityId& objectId, const NLMISC::CEntityId& newParentId, const NLMISC::CEntityId& previousParentId)
	{
		setHeader(SetParent);

		_ColumnAndSize = (uint16)column;
		_Value3[0] = *(uint64*)(&parent);

		//_MapTableRow |= MESSAGE_REMAP_ENTITYID_PRESENT;
		_ObjectIdPresent = true;
		_ObjectId = objectId;

		_ColumnAndSize |= MESSAGE_SETPARENT_ENTITYID_PRESENT;
		_NewParentId = newParentId;
		_PreviousParentId = previousParentId;
	}

	/// Is Object EntityId present
	bool					objectEntityIdPresent() const
	{
		//return (_MapTableRow & MESSAGE_REMAP_ENTITYID_PRESENT) != 0;
		return _ObjectIdPresent;
	}

	/// Are Parents EntityId present
	bool					parentsEntityIdPresent() const
	{
		return (_ColumnAndSize & MESSAGE_SETPARENT_ENTITYID_PRESENT) != 0;
	}

	/// allocate row
	void					allocRow(uint64 key)
	{
		setHeader(AllocRow);

		_Value3[0] = key;
	}

	/// deallocate row
	void					deallocRow()
	{
		setHeader(DeallocRow);
	}

	/// allocate row
	void					allocRow(uint64 key, const NLMISC::CEntityId& objectId)
	{
		setHeader(AllocRow);

		//_MapTableRow |= MESSAGE_REMAP_ENTITYID_PRESENT;
		_ObjectIdPresent = true;
		_ObjectId = objectId;

		_Value3[0] = key;
	}

	/// deallocate row
	void					deallocRow(const NLMISC::CEntityId& objectId)
	{
		setHeader(DeallocRow);

		//_MapTableRow |= MESSAGE_REMAP_ENTITYID_PRESENT;
		_ObjectIdPresent = true;
		_ObjectId = objectId;
	}

	/// load row
	void					loadRow(TTableIndex table, uint64 key)
	{
		setHeader(LoadRow);

		_ObjectIndex.set((uint8)table, 0);
		//_Table = (uint8)table;
		_Value3[0] = key;
	}



	/// Add string
	void					addString(uint64 skey, const ucstring& str)
	{
		setHeader(AddString);

		_Value3[0] = skey;
		_String = str;
	}

	/// Add string
	void					unmapString(uint64 skey)
	{
		setHeader(UnmapString);

		_Value3[0] = skey;
	}

	/// Release a row in memory
	void					releaseRow()
	{
		setHeader(ReleaseRow);
	}





	/// Log message
	void					log(uint logId, uint bufferByteSize)
	{
		setHeader(Log);
		_LogId = logId;
		_LogBuffer.resize(bufferByteSize);
	}

	/// Push Log Parameter
	template<typename T>
	void					pushParameter(uint byteOffset, const T& parameter)
	{
		nlassertex(byteOffset+sizeof(T) <= _LogBuffer.size(), ("Internal error! failed to push parameter at %d (size=%d), beyond buffer limit (%d)", byteOffset, sizeof(T), _LogBuffer.size()));
		memcpy(&(_LogBuffer[byteOffset]), &parameter, sizeof(parameter));
	}

	/// Push Log Parameter (string)
	void					pushParameter(uint byteOffset, const std::string& parameter)
	{
		nlassertex(byteOffset+sizeof(uint16) <= _LogBuffer.size(), ("Internal error! failed to push parameter at %d (size=%d), beyond buffer limit (%d)", byteOffset, sizeof(uint16), _LogBuffer.size()));
		// get current string index
		uint16	bo = (uint16)_ExtLogBuffer.size();
		_ExtLogBuffer.resize(bo+parameter.size()+1);
		memcpy(&(_ExtLogBuffer[bo]), parameter.c_str(), parameter.size()+1);
		memcpy(&(_LogBuffer[byteOffset]), &bo, sizeof(uint16));
	}

	/// Push Log Context
	void					pushContext()
	{
		setHeader(PushContext);
	}

	/// Pop Log Context
	void					popContext()
	{
		setHeader(PopContext);
	}




	/// Log Chat sentence
	void					logChat(const ucstring& sentence, const NLMISC::CEntityId& sender, const std::vector<NLMISC::CEntityId>& receivers)
	{
		setHeader(LogChat);
		_String = sentence;
		*(NLMISC::CEntityId*)(&(_Value3[0])) = sender;

		uint	bufferSize = (uint)receivers.size()*sizeof(NLMISC::CEntityId);
		if (bufferSize > 0)
		{
			_LogBuffer.resize(bufferSize);
			NLMISC::CEntityId*	srcBuffer = (NLMISC::CEntityId*)(&(receivers[0]));
			NLMISC::CEntityId*	dstBuffer = (NLMISC::CEntityId*)(&(_LogBuffer[0]));
			memcpy(dstBuffer, srcBuffer, bufferSize);
		}
		else
		{
			_LogBuffer.clear();
		}
	}


	// @}


	/// Get message type
	THeaderType					getType() const					{ return _Type; }

	/// Set Type of message
	void						setType(THeaderType type)		{ _Type = type; }



	/// \name common part methods
	// @{

	TTableIndex					getTable() const				{ return (TTableIndex)_ObjectIndex.Table; }
	TRowIndex					getRow() const					{ return (TRowIndex)_ObjectIndex.Row; }
	uint32						getStringId() const				{ return _StringId; }

	// @}



	/// \name Update database value specific methods
	// @{

	TColumnIndex				getColumn() const			{ return (TColumnIndex)(_ColumnAndSize&0x3fff); }
	const void*					getData() const				{ return &_Value0[0]; }
	uint						getDatasize() const			{ return 1 << (_ColumnAndSize>>14); }
	uint8						getValue8bits() const		{ return _Value0[0]; }
	uint16						getValue16bits() const		{ return _Value1[0]; }
	uint32						getValue32bits() const		{ return _Value2[0]; }
	uint64						getValue64bits() const		{ return _Value3[0]; }
	CObjectIndex				getObjectIndex() const		{ return *(CObjectIndex*)(&(_Value3[0])); }
	const ucstring&				getString() const			{ return _String; }

	bool						asBool() const				{ return _Value0[0] != 0; }
	char						asChar() const				{ return (char)_Value0[0]; }
	ucchar						asUCChar() const			{ return (ucchar)_Value1[0]; }
	uint8						asUint8() const				{ return (uint8)_Value0[0]; }
	uint16						asUint16() const			{ return (uint16)_Value1[0]; }
	uint32						asUint32() const			{ return (uint32)_Value2[0]; }
	uint64						asUint64() const			{ return (uint64)_Value3[0]; }
	sint8						asSint8() const				{ return (sint8)_Value0[0]; }
	sint16						asSint16() const			{ return (sint16)_Value1[0]; }
	sint32						asSint32() const			{ return (sint32)_Value2[0]; }
	sint64						asSint64() const			{ return (sint64)_Value3[0]; }
	float						asFloat() const				{ return *(float*)(&_Value2[0]); }
	double						asDouble() const			{ return *(double*)(&_Value3[0]); }
	const NLMISC::CSheetId&		asSheetId() const			{ return *(NLMISC::CSheetId*)(&_Value2[0]); }
	const NLMISC::CEntityId&	asEntityId() const			{ return *(NLMISC::CEntityId*)(&_Value3[0]); }

	const NLMISC::CEntityId&	getObjectId() const			{ return _ObjectId; }
	const NLMISC::CEntityId&	getNewParentId() const		{ return _NewParentId; }
	const NLMISC::CEntityId&	getPreviousParentId() const	{ return _PreviousParentId; }

	uint16						getLogId() const			{ return _LogId; }
	const std::vector<uint8>&	getLogBuffer() const		{ return _LogBuffer; }

	void	setupTableAndRow(TTableIndex table, TRowIndex row)
	{
		_ObjectIndex.set((uint8)table, (uint32)row);
	}

	// @}

	/// \name Log analysis/display
	// @{

	/// Dump Message content to string as a human readable message
	void						getHRContent(const CDBDescriptionParser& description, std::string& result) const;

	/// Does message contains CEntityId?
	bool						contains(const CDBDescriptionParser& description, const NLMISC::CEntityId& id);

	/// Does message contains string?
	bool						contains(const CDBDescriptionParser& description, const std::string& str);

	/// Build Log string
	std::string					buildLogString(const CDBDescriptionParser& description) const;

	/// Is Value modified
	bool						valueModified(uint table, uint column)
	{
		return ((getType() == UpdateValue || getType() == SetParent) && getTable() == table && getColumn() == column);
	}

	/// Is message selected
	bool						Selected;
	/// Message context depth
	uint16						ContextDepth;

	// @}

	/// \name Serializing
	// @{

	/// Serial message
	void					serial(NLMISC::IStream &f, TObjectIndexCircMapper& indexMapper, TEntityIdCircMapper& eidMapper);

	/// Get Message Header Size
	uint32					getMessageHeaderSize();

	// @}

private:

	/**
	 * Type of message
	 * Type is not serialised directly in message, but in containing folder
	 */
	THeaderType				_Type;


	/**
	 * Message Id
	 * Refers to the 'entity' used/updated by the message
	 */
	union					// 32 bits
	{
		uint32				_StringId;
		uint16				_LogId;
	};

	/// \name Extra info
	// @{

	uint16					_ColumnAndSize;

	CMsgObjectIndex			_ObjectIndex;

	union					// 64 bits
	{
		uint8				_Value0[8];
		uint16				_Value1[4];
		uint32				_Value2[2];
		uint64				_Value3[1];
	};

	bool					_ObjectIdPresent;
	NLMISC::CEntityId		_ObjectId;
	NLMISC::CEntityId		_NewParentId;
	NLMISC::CEntityId		_PreviousParentId;

	ucstring				_String;
	std::vector<uint8>		_LogBuffer;
	std::vector<uint8>		_ExtLogBuffer;

	// @}

	void					setHeader(THeaderType type)	{ _Type = type; }
};






/**
 * A Folder a Db Messages, all of the same kind.
 * Based on the assumption that update value messages are the main kind of messages
 * and that the follow in series...
 * Thus, it should save one byte per message...
 */
class CDbMessageFolder
{
public:

	CDbMessageFolder()
	{
		_Type = 0xff;
		_NumMessages = 0;
	}

	/**
	 * Constructor
	 */
	CDbMessageFolder(uint8 type)
	{
		_Type = type;
		_NumMessages = 0;
	}


	/**
	 * Get Folder Type
	 */
	uint8	getType() const			{ return _Type; }

	/**
	 * Get Number of messages in folder
	 */
	uint32	getNumMessages() const	{ return _NumMessages; }

	/**
	 * Folder is full
	 */
	bool	full() const			{ return _NumMessages == MAX_MESSAGE_REMAP; }

	/**
	 * Add a message to folder
	 */
	void	addMessage(const CDbMessage& msg)
	{
		nlassert(_NumMessages < MAX_MESSAGE_REMAP);
		nlassert(msg.getType() == _Type);
		++_NumMessages;
	}

	/**
	 * Serialise folder
	 */
	void	serial(NLMISC::IStream& f)
	{
		f.serial(_Type, _NumMessages);
		nlassert(_Type < CDbMessage::End);
	}

private:

	/// Type of messages in folder
	uint8	_Type;

	/// Number of messages in folder
	uint16	_NumMessages;
};








/**
 * A Queue of messages
 */
class CDbMessageQueue
{
public:

	/**
	 * Constructor
	 */
	CDbMessageQueue()
	{
		clear();
	}

	/**
	 * Clear
	 */
	void			clear()
	{
		_Messages.clear();
		_Folders.clear();
	}

	/**
	 * Get Next Message to be written
	 */
	CDbMessage&		nextMessage()
	{
		_Messages.resize(_Messages.size()+1);
		return _Messages.back();
	}

	/**
	 * Get Current Message to be written
	 */
	CDbMessage&		currentMessage()
	{
		nlassert(!_Messages.empty());
		return _Messages.back();
	}


	/**
	 * Get Number of Messages in queue
	 */
	uint32			getNumMessages() const
	{
		return (uint32)_Messages.size();
	}

	/**
	 * Get Message
	 */
	CDbMessage&		getMessage(uint32 message)
	{
		nlassert(message < _Messages.size());
		return _Messages[message];
	}

	/**
	 * Serialise message queue
	 */
	void			serial(NLMISC::IStream& f)
	{
		H_AUTO(PDLIB_MsgQueue_serial);

		// build folders first if writing to stream
		if (!f.isReading())
		{
			buildFolders();
		}

		uint32	numFolders = (uint32)_Folders.size();
		uint32	numMessages = (uint32)_Messages.size();

		f.serial(numFolders);
		f.serial(numMessages);

		if (f.isReading())
		{
			_Folders.resize(numFolders);
			_Messages.resize(numMessages);
		}

		//f.serialCont(_BackRemap);

		TEntityIdCircMapper		EIdMapper;
		TObjectIndexCircMapper	IndexMapper;

		// for each folder, write message stored in it
		uint	i, message = 0;
		for (i=0; i<_Folders.size(); ++i)
		{
			CDbMessageFolder&	folder = _Folders[i];
			f.serial(folder);

			uint	j;
			for (j=0; j<folder.getNumMessages(); ++j)
			{
				nlassert(message < numMessages);

				CDbMessage&		msg = _Messages[message++];
				msg.setType((CDbMessage::THeaderType)folder.getType());

				msg.serial(f, IndexMapper, EIdMapper);
			}
		}

		// remap messages
		if (f.isReading())
		{
			uint	currentDepth = 0;
			for (i=0; i<_Messages.size(); ++i)
			{
				CDbMessage&		msg = _Messages[i];

				if (msg.getType() == CDbMessage::PopContext)
					--currentDepth;
				msg.ContextDepth = currentDepth;
				if (msg.getType() == CDbMessage::PushContext)
					++currentDepth;
			}
		}
	}


private:

	/// List of messages
	std::vector<CDbMessage>			_Messages;

	/// List of folders
	std::vector<CDbMessageFolder>	_Folders;


	/**
	 * Build message folders
	 */
	void			buildFolders()
	{
		_Folders.clear();

		uint	i;
		for (i=0; i<_Messages.size(); ++i)
		{
			if (_Folders.empty() || _Folders.back().full() || _Messages[i].getType() != _Folders.back().getType())
				_Folders.push_back(CDbMessageFolder(_Messages[i].getType()));

			_Folders.back().addMessage(_Messages[i]);
		}
	}

};





/**
 * A Split Queue
 * Handle multiple queues, so one update may be splitted into multiple messages
 */
class CDbMessageSplitQueue
{
public:

	/**
	 * Constructor
	 */
	CDbMessageSplitQueue()
	{
	}

	/**
	 * Clearup
	 */
	void	clear()
	{
		_Queues.clear();
	}

	/**
	 * Get Next Message to be written, no mapping to be done
	 */
	CDbMessage&	nextMessage()
	{
		if (empty())
			forceNextQueue();

		return _Queues.back().nextMessage();
	}

	/**
	 * Get Next Remappable Message to be written
	 */
	CDbMessage&	nextMessage(uint8 table, uint32 row)
	{
		if (empty())
			forceNextQueue();

		// here, queue allows to map message
		CDbMessage&	msg = _Queues.back().nextMessage();

		msg.setupTableAndRow(table, row);

		// and return it
		return msg;
	}

	/**
	 * Get Current Message
	 */
	CDbMessage&	currentMessage()
	{
		return _Queues.back().currentMessage();
	}

	/**
	 * Force MsgQueue to fill next queue
	 */
	void		forceNextQueue()
	{
		if (_Queues.empty() || _Queues.back().getNumMessages() > 0)
		{
			_Queues.push_back(CDbMessageQueue());
		}
	}

	/**
	 * Is Queue Empty?
	 */
	bool		empty() const
	{
		return _Queues.empty();
	}

	/**
	 * Number of message in queue
	 */
	uint32		getNumMessagesEnqueued() const
	{
		std::list<CDbMessageQueue>::const_iterator	it;
		uint32	totalMessages = 0;
		for (it=_Queues.begin(); it!=_Queues.end(); ++it)
			totalMessages += (*it).getNumMessages();

		return totalMessages;
	}


	/**
	 * begin()
	 */
	std::list<CDbMessageQueue>::iterator	begin()		{ return _Queues.begin(); }

	/**
	 * end()
	 */
	std::list<CDbMessageQueue>::iterator	end()		{ return _Queues.end(); }

	/**
	 * size()
	 */
	uint									size() const	{ return (uint)_Queues.size(); }

	/**
	 * get()
	 */
	CDbMessageQueue&						get(uint i)
	{
		std::list<CDbMessageQueue>::iterator	it = _Queues.begin();
		while (i-- > 0)
			++it;
		return (*it);
	}


private:

	/// Used Queues
	std::list<CDbMessageQueue>		_Queues;

};





class CUpdateLog
{
public:

	CUpdateLog() : UpdateId(0xffffffff), _OwnUpdates(false), _Updates(NULL)		{ }

	~CUpdateLog();

	/// UpdateId sent by client for this update
	uint32						UpdateId;

	/// Start date for this update
	CTimestamp					StartStamp;

	/// Start date for this update
	CTimestamp					EndStamp;

	/// Serial log
	void						serial(NLMISC::IStream& f);

	/// Display UpdateLog content (using a database description)
	void						display(const CDBDescriptionParser& description, NLMISC::CLog& log, bool onlySelected = false);




	/**
	 * Check log timestamp boundaries
	 */
	bool						checkTimestampBoundaries(const CTimestamp& begin, const CTimestamp& end);

	/**
	 * Is Empty
	 */
	bool						isEmpty();

	/**
	 * Select contexts and messages containing a given entityId
	 * return true if there were at least one message selected
	 */
	bool						selectMessages(const CDBDescriptionParser& description, const NLMISC::CEntityId& id);

	/**
	 * Select contexts and messages containing a given string
	 * return true if there were at least one message selected
	 */
	bool						selectMessages(const CDBDescriptionParser& description, const std::string& str);

	/**
	 * Select contexts and messages containing modification of a value for a given entityId
	 * return true if there were at least one message selected
	 */
	bool						selectMessages(const CDBDescriptionParser& description, const NLMISC::CEntityId& id, const std::string& valuePath);

	/**
	 * Select contexts and messages containing a list of entityIds (limited at most to 32 entityIds)
	 * return true if there were at least one message selected
	 */
	bool						selectMessages(const CDBDescriptionParser& description, const std::vector<NLMISC::CEntityId>& ids);

	class CLogProcessor
	{
	public:
		/// process log, return true if some messages were selected
		virtual bool	processLog(CUpdateLog& log, const CDBDescriptionParser& description) = 0;
	};

	/**
	 * Apply process on log files
	 */
	static void					processLogs(const std::string& path,
											const CTimestamp& begin,
											const CTimestamp& end,
											NLMISC::CLog& log,
											CLogProcessor* processor,
											float* progress = NULL);

	/**
	 * Display log for a given entity id, between 2 dates
	 */
	static void					displayLogs(const CDBDescriptionParser& description,
											const NLMISC::CEntityId& id,
											const CTimestamp& begin,
											const CTimestamp& end,
											const std::string& path,
											NLMISC::CLog& log,
											float* progress = NULL);

	/**
	 * Display log between 2 dates
	 */
	static void					displayLogs(const std::string& path,
											const CTimestamp& begin,
											const CTimestamp& end,
											NLMISC::CLog& log,
											float* progress = NULL);

	/**
	 * Display log for a given entity id, between 2 dates
	 */
	static void					displayLogs(const std::string& path,
											const NLMISC::CEntityId& id,
											const CTimestamp& begin,
											const CTimestamp& end,
											NLMISC::CLog& log,
											float* progress = NULL);

	/**
	 * Display log for a given entity id and a specified value to be modified, between 2 dates
	 */
	static void					displayLogs(const std::string& path,
											const NLMISC::CEntityId& id,
											const std::string& valuePath,
											const CTimestamp& begin,
											const CTimestamp& end,
											NLMISC::CLog& log,
											float* progress = NULL);

	/**
	 * Display log for a list of given entity id, between 2 dates
	 */
	static void					displayLogs(const std::string& path,
											const std::vector<NLMISC::CEntityId>& ids,
											const CTimestamp& begin,
											const CTimestamp& end,
											NLMISC::CLog& log,
											float* progress = NULL);

	/**
	 * Display log for a list of given entity id, between 2 dates
	 */
	static void					displayLogs(const std::string& path,
											const std::string& str,
											const CTimestamp& begin,
											const CTimestamp& end,
											NLMISC::CLog& log,
											float* progress = NULL);

	/**
	 * Elect matching description
	 */
	static std::string			electDescription(const std::string& logFile);




	/**
	 * Set updates
	 */
	void						setUpdates(CDbMessageQueue* updates);

	/**
	 * Create updates
	 */
	void						createUpdates();

	/**
	 * Get Updates
	 */
	CDbMessageQueue*			getUpdates()	{ return _Updates; }

private:

	bool						_OwnUpdates;

	/// Updates contained in message
	CDbMessageQueue*			_Updates;

	/// Release Updates (and delete if owned)
	void						releaseUpdates()
	{
		if (_OwnUpdates && _Updates != NULL)
			delete _Updates;

		_Updates = NULL;
		_OwnUpdates = false;
	}
};





//
// CDbMessage inline methods
//

inline void	CDbMessage::serial(NLMISC::IStream &f, TObjectIndexCircMapper& indexMapper, TEntityIdCircMapper& eidMapper)
{
	switch (_Type)
	{
	case UpdateValue:
		{
			uint8	flags = (objectEntityIdPresent() ? 0x80 : 0);
			indexMapper.serial(f, _ObjectIndex, flags);
			_ObjectIdPresent = ((flags & 0x80) != 0);

			f.serial(_ColumnAndSize);

			switch (_ColumnAndSize & 0xc000)
			{
			case 0x0000:	f.serial(_Value0[0]); break;
			case 0x4000:	f.serial(_Value1[0]); break;
			case 0x8000:	f.serial(_Value2[0]); break;
			case 0xc000:	f.serial(_Value3[0]); break;
			}

			// serial owner CEntityId if present
			if (objectEntityIdPresent())
				eidMapper.serial(f, _ObjectId);
		}
		break;

	case SetParent:
		{
			uint8	flags = (objectEntityIdPresent() ? 0x80 : 0);
			indexMapper.serial(f, _ObjectIndex, flags);
			_ObjectIdPresent = ((flags & 0x80) != 0);
			f.serial(_ColumnAndSize);
			f.serial(_Value3[0]);

			// serial object CEntityId if present
			if (objectEntityIdPresent())
				eidMapper.serial(f, _ObjectId);

			// serial parents CEntityId if present
			if ((_ColumnAndSize & MESSAGE_SETPARENT_ENTITYID_PRESENT) != 0)
			{
				eidMapper.serial(f, _NewParentId);
				eidMapper.serial(f, _PreviousParentId);
			}
		}
		break;

	case AllocRow:
		{
			uint8	flags = (objectEntityIdPresent() ? 0x80 : 0);
			indexMapper.serial(f, _ObjectIndex, flags);
			_ObjectIdPresent = ((flags & 0x80) != 0);
			f.serial(_Value3[0]);

			// serial owner CEntityId if present
			if (objectEntityIdPresent())
				eidMapper.serial(f, _ObjectId);
		}
		break;

	case DeallocRow:
		{
			uint8	flags = (objectEntityIdPresent() ? 0x80 : 0);
			indexMapper.serial(f, _ObjectIndex, flags);
			_ObjectIdPresent = ((flags & 0x80) != 0);
			// serial owner CEntityId if present
			if (objectEntityIdPresent())
				eidMapper.serial(f, _ObjectId);
		}
		break;

	case LoadRow:
		f.serial(_ObjectIndex.Table);
		f.serial(_Value3[0]);
		break;

	case AddString:
		f.serial(_Value3[0]);
		f.serial(_String);
		break;

	case UnmapString:
		f.serial(_Value3[0]);
		break;

	case ReleaseRow:
		indexMapper.serial(f, _ObjectIndex);
		break;

	case Log:
		{
			f.serial(_LogId);

			if (f.isReading())
			{
				uint8	sz;

				f.serial(sz);
				_LogBuffer.resize(sz);
				if (sz > 0)
					f.serialBuffer(&(_LogBuffer[0]), sz);

				f.serial(sz);
				_ExtLogBuffer.resize(sz);
				if (sz > 0)
					f.serialBuffer(&(_ExtLogBuffer[0]), sz);
			}
			else
			{
				uint8	sz;
				nlassert(_LogBuffer.size() <= 255);
				sz = (uint8)_LogBuffer.size();
				f.serial(sz);
				if (sz > 0)
					f.serialBuffer(&(_LogBuffer[0]), sz);

				nlassert(_ExtLogBuffer.size() <= 255);
				sz = (uint8)_ExtLogBuffer.size();
				f.serial(sz);
				if (sz > 0)
					f.serialBuffer(&(_ExtLogBuffer[0]), sz);
			}
		}
		break;

	case PushContext:
		break;

	case PopContext:
		break;

	case LogChat:
		// serial chat sentence
		f.serial(_String);
		// serial sender
		f.serial(_Value3[0]);
		// serial receivers list (whole buffer as uint8*)
		f.serialCont(_LogBuffer);
		break;

	default:
		nlerror("CDbMessage::serial(): unable to serial message type '%d'", _Type);
		break;
	}
}

/*
 * Get Message Header Size
 */
inline uint32	CDbMessage::getMessageHeaderSize()
{
	uint	size = 0;

	switch (_Type)
	{
	case UpdateValue:
	case SetParent:
		size += sizeof(_ObjectIndex.Table)+sizeof(_ObjectIndex.Row);
		size += sizeof(_ColumnAndSize);
		break;

	case AllocRow:
		size += sizeof(_ObjectIndex.Table)+sizeof(_ObjectIndex.Row);
		break;

	case DeallocRow:
		size += sizeof(_ObjectIndex.Table)+sizeof(_ObjectIndex.Row);
		break;

	case LoadRow:
		size += sizeof(_ObjectIndex.Table);
		size += sizeof(_Value3[0]);
		break;

	case ReleaseRow:
		size += sizeof(_ObjectIndex.Table)+sizeof(_ObjectIndex.Row);
		break;

	case Log:
		size += sizeof(_LogId);
		size += 2;
		break;

	default:
		break;
	}

	return size;
}

/*
 * Serial log
 */
inline void	CUpdateLog::serial(NLMISC::IStream& f)
{
	f.serialCheck(NELID("ULOG"));

	uint	version = f.serialVersion(1);

	f.serial(UpdateId);

	if (version >= 1)
	{
		f.serial(StartStamp);
		f.serial(EndStamp);
	}

	if (f.isReading())
	{
		releaseUpdates();
		_Updates = new RY_PDS::CDbMessageQueue();
		_OwnUpdates = true;
	}

	f.serial(*_Updates);
}


}; // RY_PDS

#endif //RY_PD_MESSAGES_H

