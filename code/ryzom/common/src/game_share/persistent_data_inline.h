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

//-----------------------------------------------------------------------------
// inlines CPersistentDataRecord
//-----------------------------------------------------------------------------

inline void CPersistentDataRecord::addString(const std::string& name,uint16 &result)
{
	// check whether the value of 'result' is already correct
	if (result<_StringTable.size())
		if (_StringTable[result]==name)
			return;

	// no luck so do the full work...
	result= addString(name);
}

inline void CPersistentDataRecord::addString(const char* name,uint16 &result)
{
	// check whether the value of 'result' is already correct
	if (result<_StringTable.size())
		if (strcmp(_StringTable[result].c_str(),name)==0)
			return;

	// no luck so do the full work...
	result= addString(name);
}

inline void CPersistentDataRecord::push(TToken token,const CArg& arg)
{
	arg.push(token,_TokenTable,_ArgTable);
}

inline void CPersistentDataRecord::push(TToken token,bool val)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::SINT_TOKEN);
	_ArgTable.push_back((sint32)val);
}

inline void CPersistentDataRecord::push(TToken token,sint8 val)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::SINT_TOKEN);
	_ArgTable.push_back((sint32)val);
}

inline void CPersistentDataRecord::push(TToken token,sint16 val)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::SINT_TOKEN);
	_ArgTable.push_back((sint32)val);
}

inline void CPersistentDataRecord::push(TToken token,sint32 val)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::SINT_TOKEN);
	_ArgTable.push_back(val);
}

inline void CPersistentDataRecord::push(TToken token,sint64 val)
{
	// create a union for splitting the i64 value into 2 32bit parts and map the union onto the input value
	struct C64BitParts
	{
		uint32	i32_1;
		uint32	i32_2;
	} &valueInBits= *(C64BitParts*)&val;

	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::EXTEND_TOKEN);
	_ArgTable.push_back(valueInBits.i32_1);
	_TokenTable.push_back((token<<3)+CArg::SINT_TOKEN);
	_ArgTable.push_back(valueInBits.i32_2);
}

inline void CPersistentDataRecord::push(TToken token,uint8 val)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::UINT_TOKEN);
	_ArgTable.push_back(val);
}

inline void CPersistentDataRecord::push(TToken token,uint16 val)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::UINT_TOKEN);
	_ArgTable.push_back(val);
}

inline void CPersistentDataRecord::push(TToken token,uint32 val)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::UINT_TOKEN);
	_ArgTable.push_back(val);
}

inline void CPersistentDataRecord::push(TToken token,uint64 val)
{
	// create a union for splitting the i64 value into 2 32bit parts and map the union onto the input value
	struct C64BitParts
	{
		uint32	i32_1;
		uint32	i32_2;
	} &valueInBits= *(C64BitParts*)&val;

	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::EXTEND_TOKEN);
	_ArgTable.push_back(valueInBits.i32_1);
	_TokenTable.push_back((token<<3)+CArg::UINT_TOKEN);
	_ArgTable.push_back(valueInBits.i32_2);
}

inline void CPersistentDataRecord::push(TToken token,float val)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::FLOAT_TOKEN);
	_ArgTable.push_back(*(sint32*)&val);
}

inline void CPersistentDataRecord::push(TToken token,double val)
{
	// create a union for splitting the i64 value into 2 32bit parts and map the union onto the input value
	struct C64BitParts
	{
		uint32	i32_1;
		uint32	i32_2;
	} &valueInBits= *(C64BitParts*)&val;

	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::EXTEND_TOKEN);
	_ArgTable.push_back(valueInBits.i32_1);
	_TokenTable.push_back((token<<3)+CArg::FLOAT_TOKEN);
	_ArgTable.push_back(valueInBits.i32_2);
}

inline void CPersistentDataRecord::push(TToken token,const std::string& val)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::STRING_TOKEN);
	_ArgTable.push_back(addString(val));
}

inline void CPersistentDataRecord::push(TToken token,const ucstring& val)
{
	// treat ucstrings as strings
	push(token,val.toUtf8());
}

inline void CPersistentDataRecord::push(TToken token,NLMISC::CSheetId val)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token and value to the relavent data buffers
	_TokenTable.push_back((token<<3)+CArg::EXTEND_TOKEN);
	_ArgTable.push_back(CArg::ET_SHEET_ID);
	_TokenTable.push_back((token<<3)+CArg::STRING_TOKEN);
	_ArgTable.push_back(val.asInt());
}

inline void CPersistentDataRecord::push(TToken token,const NLMISC::CEntityId& val)
{
	// this one is a bit complictaed to unrole by hand - better leave it to standard CArg::push()
	CArg::EntityId(val).push(token,_TokenTable,_ArgTable);
}

inline void CPersistentDataRecord::push(TToken token)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	// store the token to the relavent data buffer
	_TokenTable.push_back((token<<3)+CArg::FLAG_TOKEN);
}

inline void CPersistentDataRecord::pushStructBegin(TToken token)
{
	// make sure the token is valid
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	#endif

	_TokenTable.push_back(token<<3|CArg::STRUCT_BEGIN);
	_WritingStructStack.push_back(token);
}

inline void CPersistentDataRecord::pushStructEnd(TToken token)
{
	#ifdef NL_DEBUG
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	BOMB_IF(_WritingStructStack.empty(),"Trying to pop past end of stack",return);
	BOMB_IF(_WritingStructStack.back()!=token,"Attempting to end a structure with the wrong delimiting token",return);
	#endif

	_WritingStructStack.pop_back();
	_TokenTable.push_back(token<<3|CArg::STRUCT_END);
}

//-------------------------------------------------------------------------
// set of accessors for retrieving data from a CPersistentDataRecord
//-------------------------------------------------------------------------

inline bool CPersistentDataRecord::isEndOfData() const
{
	DROP_IF( (_TokenOffset==_TokenTable.size()) && !(_ArgOffset==_ArgTable.size()),"Argument table and token table sizes don't match", return true);
	DROP_IF( _TokenOffset>_TokenTable.size(),"Attempt to access beyond end of data...", return true);
	return _TokenOffset==_TokenTable.size();
}

inline bool CPersistentDataRecord::isEndOfStruct() const
{
	if (isEndOfData())
		return true;
	if (_ReadingStructStack.empty())
		return false;
	if (peekNextTokenType()!=CArg::STRUCT_END)
		return false;
	DROP_IF(_ReadingStructStack.back()!=peekNextToken(),"Opening and closing structure tokens don't match",return false)
	return true;
}

inline bool CPersistentDataRecord::isStartOfStruct() const
{
//	DROP_IF(isEndOfData(),"Attempt to read past end of input data",return 0);
	return (peekNextTokenType()==CArg::STRUCT_BEGIN);
}

inline bool CPersistentDataRecord::isTokenWithNoData() const
{
//	DROP_IF(isEndOfData(),"Attempt to read past end of input data",return 0);
	return (peekNextTokenType()==CArg::FLAG);
}

inline CPersistentDataRecord::TToken CPersistentDataRecord::peekNextToken() const
{
	DROP_IF(isEndOfData(),"Attempt to read past end of input data",return 0);
	// the 3 low bits contain arg type information - uninteresting here
	return _TokenTable[_TokenOffset]>>3;
}

inline const NLMISC::CSString& CPersistentDataRecord::peekNextTokenName() const
{
	TToken t= peekNextToken();
	return _StringTable[t];
}

inline CPersistentDataRecord::CArg::TType CPersistentDataRecord::peekNextTokenType() const
{
	DROP_IF(isEndOfData(),"Attempt to read past end of input data",return CArg::TType(0));
	uint32 tokenType= _TokenTable[_TokenOffset]&7;
	if (tokenType==CArg::EXTEND_TOKEN)
	{
		DROP_IF(_TokenOffset+1>=_TokenTable.size(),"Attempt to read past end of input data",return CArg::TType(0));
		return CArg::token2Type(_TokenTable[_TokenOffset+1]&7,true);
	}
	return CArg::token2Type(_TokenTable[_TokenOffset]&7,false);
}

inline const CPersistentDataRecord::CArg& CPersistentDataRecord::peekNextArg() const
{
	CPersistentDataRecord::peekNextArg(TempArg);
	return TempArg;
}

inline void CPersistentDataRecord::peekNextArg(CPersistentDataRecord::CArg& result) const
{
	result.setType(peekNextTokenType());
	if (result.isExtended())
	{
		BOMB_IF(_ArgOffset+1>=_ArgTable.size(),"Attempt to overrun end of input data",return);
		DROP_IF((_TokenTable[_TokenOffset+0]&~7)!=(_TokenTable[_TokenOffset+1]&~7),"2 dwords of 64 bit have non-matching identifiers",return);
		result._Value.i32_1 = _ArgTable[_ArgOffset+0];
		result._Value.i32_2 = _ArgTable[_ArgOffset+1];
		nlassert(result._Value.ExType == result._Value.i32_1);
		nlassert(result._Value.i32_2 == result._Value.ex32_1);
		nlassert(result._Value.ExData32 == result._Value.ex32_1);

		if (result._Type == CArg::EXTEND_TYPE && result._Value.ExType >= CArg::ET_64_BIT_EXTENDED_TYPES)
		{
			// this is a 96 bit extended type, read one more value
			BOMB_IF(_ArgOffset+2>=_ArgTable.size(),"Attempt to overrun end of input data",return);
			BOMB_IF((_TokenTable[_TokenOffset+0]&~7)!=(_TokenTable[_TokenOffset+2]&~7),"3 dwords of 96 bit have non-matching identifiers",return);
			result._Value.ex32_2 = _ArgTable[_ArgOffset+2];

			nlassert((uint64(result._Value.ex32_2)<<32|result._Value.ex32_1) == result._Value.ExData64);
		}
	}
	else if (!result.isFlag())
	{
		result._Value.i32_1 = _ArgTable[_ArgOffset];
	}
	if (result._Type==CArg::STRING)
	{
		result._String=lookupString(result._Value.i32_1);
	}
	return;
}

//inline CPersistentDataRecord::CArg CPersistentDataRecord::peekNextArg() const
//{
//	CArg arg;
//	arg.setType(peekNextTokenType());
//	if (arg.isExtended())
//	{
//		BOMB_IF(_ArgOffset+1>=_ArgTable.size(),"Attempt to overrun end of input data",return arg);
//		DROP_IF((_TokenTable[_TokenOffset+0]&~7)!=(_TokenTable[_TokenOffset+1]&~7),"2 dwords of 64 bit have non-matching identifiers",return arg);
//		arg._Value.i32_1 = _ArgTable[_ArgOffset+0];
//		arg._Value.i32_2 = _ArgTable[_ArgOffset+1];
//		nlassert(arg._Value.ExType == arg._Value.i32_1);
//		nlassert(arg._Value.i32_2 == arg._Value.ex32_1);
//		nlassert(arg._Value.ExData32 == arg._Value.ex32_1);
//
//		if (arg._Type == CArg::EXTEND_TYPE && arg._Value.ExType >= CArg::ET_64_BIT_EXTENDED_TYPES)
//		{
//			// this is a 96 bit extended type, read one more value
//			BOMB_IF(_ArgOffset+2>=_ArgTable.size(),"Attempt to overrun end of input data",return arg);
//			BOMB_IF((_TokenTable[_TokenOffset+0]&~7)!=(_TokenTable[_TokenOffset+2]&~7),"3 dwords of 96 bit have non-matching identifiers",return arg);
//			arg._Value.ex32_2 = _ArgTable[_ArgOffset+2];
//
//			nlassert((uint64(arg._Value.ex32_2)<<32|arg._Value.ex32_1) == arg._Value.ExData64);
//		}
//	}
//	else if (!arg.isFlag())
//	{
//		arg._Value.i32_1 = _ArgTable[_ArgOffset];
//	}
//	if (arg._Type==CArg::STRING)
//	{
//		arg._String=lookupString(arg._Value.i32_1);
//	}
//	return arg;
//}

inline const CPersistentDataRecord::CArg& CPersistentDataRecord::popNextArg(TToken token)
{
	CPersistentDataRecord::popNextArg(token,TempArg);
	return TempArg;
}

inline void CPersistentDataRecord::popNextArg(TToken token,CPersistentDataRecord::CArg& result)
{
	#ifdef NL_DEBUG
		BOMB_IF(peekNextToken()!=token,"Error on read code - token requested doesn't match token found",return);
	#else
		nlunreferenced(token);
	#endif

	peekNextArg(result);
	if (result.isFlag())
	{
		++_TokenOffset;
	}
	else if (result.isExtended())
	{
		_ArgOffset+=2;
		_TokenOffset+=2;

		if (result._Type == CArg::EXTEND_TYPE && result._Value.ExType >= CArg::ET_64_BIT_EXTENDED_TYPES)
		{
			// this is a 96 bit extended type, skip another pair
			_ArgOffset++;
			_TokenOffset++;
		}
	}
	else
	{
		++_ArgOffset;
		++_TokenOffset;
	}
}

//inline CPersistentDataRecord::CArg CPersistentDataRecord::popNextArg(TToken token)
//{
//	CArg arg;
//	DROP_IF(peekNextToken()!=token,"Error on read code - token requested doesn't match token found",return arg);
//	arg= peekNextArg();
//	if (arg.isFlag())
//	{
//		++_TokenOffset;
//	}
//	else if (arg.isExtended())
//	{
//		_ArgOffset+=2;
//		_TokenOffset+=2;
//
//		if (arg._Type == CArg::EXTEND_TYPE && arg._Value.ExType >= CArg::ET_64_BIT_EXTENDED_TYPES)
//		{
//			// this is a 96 bit extended type, skip another pair
//			_ArgOffset++;
//			_TokenOffset++;
//		}
//	}
//	else
//	{
//		++_ArgOffset;
//		++_TokenOffset;
//	}
//	return arg;
//}

inline void CPersistentDataRecord::pop(TToken token,bool& result)
{
	popNextArg(token,TempArg);
	result= (TempArg.asSint()!=0);
}

inline void CPersistentDataRecord::pop(TToken token,sint8& result)
{
	popNextArg(token,TempArg);
	result= (sint8)TempArg.asSint();
}
inline void CPersistentDataRecord::pop(TToken token,sint16& result)
{
	popNextArg(token,TempArg);
	result= (sint16)TempArg.asSint();
}
inline void CPersistentDataRecord::pop(TToken token,sint32& result)
{
	popNextArg(token,TempArg);
	result= (sint32)TempArg.asSint();
}
inline void CPersistentDataRecord::pop(TToken token,sint64& result)
{
	popNextArg(token,TempArg);
	result= (sint64)TempArg.asSint();
}

inline void CPersistentDataRecord::pop(TToken token,uint8& result)
{
	popNextArg(token,TempArg);
	result= (uint8)TempArg.asUint();
}

inline void CPersistentDataRecord::pop(TToken token,uint16& result)
{
	popNextArg(token,TempArg);
	result= (uint16)TempArg.asUint();
}
inline void CPersistentDataRecord::pop(TToken token,uint32& result)
{
	popNextArg(token,TempArg);
	result= (uint32)TempArg.asUint();
}
inline void CPersistentDataRecord::pop(TToken token,uint64& result)
{
	popNextArg(token,TempArg);
	result= (uint64)TempArg.asUint();
}

inline void CPersistentDataRecord::pop(TToken token,float& result)
{
	popNextArg(token,TempArg);
	result= TempArg.asFloat();
}
inline void CPersistentDataRecord::pop(TToken token,double& result)
{
	popNextArg(token,TempArg);
	result= TempArg.asDouble();
}
inline void CPersistentDataRecord::pop(TToken token,std::string& result)
{
	popNextArg(token,TempArg);
	result= TempArg.asString();
}
inline void CPersistentDataRecord::pop(TToken token,ucstring& result)
{
	popNextArg(token,TempArg);
	result= TempArg.asUCString();
}

inline void CPersistentDataRecord::pop(TToken token,NLMISC::CSheetId& result)
{
	popNextArg(token,TempArg);
	result= TempArg.asSheetId();
}
inline void CPersistentDataRecord::pop(TToken token,NLMISC::CEntityId& result)
{
	popNextArg(token,TempArg);
	result= TempArg.asEntityId();
}

inline void CPersistentDataRecord::pop(TToken /* token */)
{
	++_TokenOffset;
}

inline void CPersistentDataRecord::popStructBegin(TToken token)
{
	DROP_IF(peekNextToken()!=token,"Attempting to enter a structure with the wrong delimiting token",return);
	DROP_IF(peekNextTokenType()!=CArg::STRUCT_BEGIN,"Attempting to enter a structure with the wrong delimiting token type",return);
	_ReadingStructStack.push_back(token);
	++_TokenOffset;
}

inline void CPersistentDataRecord::popStructEnd(TToken token)
{
	DROP_IF(_ReadingStructStack.empty(),"Attempting to pop end of a structure with nothing left in the open structure stack",return);
	TToken nextToken=peekNextToken();
	TToken topToken=_ReadingStructStack.back();
	DROP_IF(topToken!=token,"Attempting to pop end of a structure with the wrong delimiting token",return);
	DROP_IF(nextToken!=token,"Attempting to pop end of a structure with the wrong delimiting token",return);
	DROP_IF(peekNextTokenType()!=CArg::STRUCT_END,"Attempting to leave a structure with the wrong delimiting token type",return);
	_ReadingStructStack.pop_back();
	++_TokenOffset;
}


//-------------------------------------------------------------------------
// methods CPersistentDataRecord::CArg
//-------------------------------------------------------------------------

inline CPersistentDataRecord::CArg::CArg()
{
	_Type= STRING;
	_Value.i64= 0;
}

inline CPersistentDataRecord::CArg::CArg(const std::string& type,const std::string& value,CPersistentDataRecord& pdr)
{
	if (setType(type)==false)
		return;
	switch (_Type)
	{
	case SINT32:		_Value.i32=NLMISC::CSString(value).atoi();		break;
	case UINT32:		_Value.i32=NLMISC::CSString(value).atoi();		break;
	case SINT64:		_Value.i64=NLMISC::atoiInt64(value.c_str());	break;
	case UINT64:		_Value.i64=NLMISC::atoiInt64(value.c_str());	break;
	case FLOAT32:		NLMISC::fromString(value, _Value.f32);			break;
	case FLOAT64:		NLMISC::fromString(value, _Value.f64);			break;
	case STRING:		_Value.i32=pdr.addString(value); _String=value;	break;
	case EXTEND_TYPE:
		switch(_Value.ExType)
		{
		case ET_SHEET_ID:
			{
				// Cf. CArg::asString()
				if (value.size() != 0 && value[0] == '#')
				{
					_Value.ex32_1 = NLMISC::CSString(value.c_str()+1).atoi();
				}
				else
				{
					NLMISC::CSheetId sheetId(value);
					STOP_IF( (sheetId == NLMISC::CSheetId::Unknown && value != "unknown.unknown"), "sheet_id.bin version does not match save game version" );
					_Value.ExData32 = sheetId.asInt();
				}
			}
			break;
		case ET_ENTITY_ID:
			{
				NLMISC::CEntityId entityId(value);
				_Value.ExData64 = entityId.getRawId();
			}
			break;
		default:			STOP("This should never happen!");				break;
		}
		break;
	case FLAG:															break;
	default:			STOP("This should never happen!");				break;
	}
}

inline uint64 CPersistentDataRecord::CArg::asUint() const
{
	switch (_Type)
	{
	case STRUCT_BEGIN:
	case STRUCT_END:	BOMB("Can't extract a value from a structure delimiter", return 0);
	case SINT32:		return (uint64)(sint32)_Value.i32;
	case UINT32:		return (uint64)(uint32)_Value.i32;	// 2 casts to ensure no sign extend
	case SINT64:		return (uint64)_Value.i64;
	case UINT64:		return (uint64)_Value.i64;
	case FLOAT32:		return (uint64)_Value.f32;
	case FLOAT64:		return (uint64)_Value.f64;
	case STRING:		return (uint64)NLMISC::atoiInt64(_String.c_str());
	case FLAG:			return 1;
	case EXTEND_TYPE:
		switch(_Value.ExType)
		{
		case ET_SHEET_ID:	return _Value.ExData32;
		case ET_ENTITY_ID:	return _Value.ExData64;
		default:			break;
		}
	default:			break;
	}
	STOP("This should never happen!");
	return 0;
}

inline sint64 CPersistentDataRecord::CArg::asSint() const
{
	switch (_Type)
	{
	case STRUCT_BEGIN:
	case STRUCT_END:	BOMB("Can't extract a value from a structure delimiter", return 0);
	case SINT32:		return (sint64)(sint32)_Value.i32;
	case UINT32:		return (sint64)(uint32)_Value.i32;
	case SINT64:		return (sint64)_Value.i64;
	case UINT64:		return (sint64)_Value.i64;
	case FLOAT32:		return (sint64)_Value.f32;
	case FLOAT64:		return (sint64)_Value.f64;
	case STRING:		return (sint64)NLMISC::atoiInt64(_String.c_str());
	case FLAG:			return 1;
	case EXTEND_TYPE:
		switch(_Value.ExType)
		{
		case ET_SHEET_ID:	return _Value.ExData32;
		case ET_ENTITY_ID:	return _Value.ExData64;
		default:			break;
		}
	default:			break;
	}
	STOP("This should never happen!");
	return 0;
}

inline float CPersistentDataRecord::CArg::asFloat() const
{
	switch (_Type)
	{
	case STRUCT_BEGIN:
	case STRUCT_END:	BOMB("Can't extract a value from a structure delimiter", return 0);
	case SINT32:		return (float)(sint32)_Value.i32;
	case UINT32:		return (float)(uint32)_Value.i32;
	case SINT64:		return (float)(sint64)_Value.i64;
	case UINT64:		return (float)(uint64)_Value.i64;
	case FLOAT32:		return (float)_Value.f32;
	case FLOAT64:		return (float)_Value.f64;
	case STRING:		return (float)atof(_String.c_str());
	case FLAG:			return 1.0f;
	case EXTEND_TYPE:
		switch(_Value.ExType)
		{
		case ET_SHEET_ID:	return float(_Value.ExData32);
		case ET_ENTITY_ID:	return float(_Value.ExData64);
		default:			break;
		}
	default:			break;
	}
	STOP("This should never happen!");
	return 0.0f;
}

inline double CPersistentDataRecord::CArg::asDouble() const
{
	switch (_Type)
	{
	case STRUCT_BEGIN:
	case STRUCT_END:	BOMB("Can't extract a value from a structure delimiter", return 0);
	case SINT32:		return (double)(sint32)_Value.i32;
	case UINT32:		return (double)(uint32)_Value.i32;
	case SINT64:		return (double)(sint64)_Value.i64;
	case UINT64:		return (double)(uint64)_Value.i64;
	case FLOAT32:		return (double)_Value.f32;
	case FLOAT64:		return (double)_Value.f64;
	case STRING:		return (double)atof(_String.c_str());
	case FLAG:			return 1.0;
	case EXTEND_TYPE:
		switch(_Value.ExType)
		{
		case ET_SHEET_ID:	return double(_Value.ExData32);
		case ET_ENTITY_ID:	return double(_Value.ExData64);
		default:			break;
		}
	default:			break;
	}
	STOP("This should never happen!");
	return 0.0;
}

inline NLMISC::CSString CPersistentDataRecord::CArg::asString() const
{
	switch (_Type)
	{
	case STRUCT_BEGIN:
	case STRUCT_END:	BOMB("Can't extract a value from a structure delimiter", return 0);
	case SINT32:		return NLMISC::toString((sint32)_Value.i32);
	case UINT32:		return NLMISC::toString((uint32)_Value.i32);
	case SINT64:		return NLMISC::toString((sint64)_Value.i64);
	case UINT64:		return NLMISC::toString((uint64)_Value.i64);
	case FLOAT32:		return NLMISC::toString(_Value.f32);
	case FLOAT64:		return NLMISC::toString(_Value.f64);
	case STRING:		return _String;
	case FLAG:			return "1";
	case EXTEND_TYPE:
		switch(_Value.ExType)
		{
		case ET_SHEET_ID:
			{
				NLMISC::CSheetId sheetId(_Value.ExData32);
				return sheetId.toString(true);
			}
		case ET_ENTITY_ID:
			{
				NLMISC::CEntityId entityId(_Value.ExData64);
				return entityId.toString();
			}
		default:
			break;
		}
	default:			break;
	}
	STOP("This should never happen!");
	return "";
}

inline ucstring CPersistentDataRecord::CArg::asUCString() const
{
	switch (_Type)
	{
	case STRUCT_BEGIN:
	case STRUCT_END:	BOMB("Can't extract a value from a structure delimiter", return ucstring());
	case SINT32:		return (ucstring)NLMISC::toString((sint32)_Value.i32);
	case UINT32:		return (ucstring)NLMISC::toString((uint32)_Value.i32);
	case SINT64:		return (ucstring)NLMISC::toString((sint64)_Value.i64);
	case UINT64:		return (ucstring)NLMISC::toString((uint64)_Value.i64);
	case FLOAT32:		return (ucstring)NLMISC::toString(_Value.f32);
	case FLOAT64:		return (ucstring)NLMISC::toString(_Value.f64);
	case STRING:		{ ucstring s; s.fromUtf8(_String); return s; }
	case FLAG:			return (ucstring)"1";
	case EXTEND_TYPE:
		switch(_Value.ExType)
		{
		case ET_SHEET_ID:
			{
				NLMISC::CSheetId sheetId(_Value.ExData32);
				return sheetId.toString(true);
			}
		case ET_ENTITY_ID:
			{
				NLMISC::CEntityId entityId(_Value.ExData64);
				return entityId.toString();
			}
		default:
			break;
		}
	default:			break;
	}
	STOP("This should never happen!");
	return ucstring("");
}

inline NLMISC::CEntityId CPersistentDataRecord::CArg::asEntityId() const
{
	switch (_Type)
	{
	case STRUCT_BEGIN:
	case STRUCT_END:	BOMB("Can't extract a value from a structure delimiter", return NLMISC::CEntityId());
	case SINT32:		return NLMISC::CEntityId((uint64)_Value.i32);
	case UINT32:		return NLMISC::CEntityId((uint64)_Value.i32);
	case SINT64:		return NLMISC::CEntityId((uint64)_Value.i64);
	case UINT64:		return NLMISC::CEntityId((uint64)_Value.i64);
	case FLOAT32:		return NLMISC::CEntityId((uint64)_Value.f32);
	case FLOAT64:		return NLMISC::CEntityId((uint64)_Value.f64);
	case STRING:
		{
			// try a convertion from '(0x000000000000000:00:00:00)' format
			NLMISC::CEntityId result(_String);

			// if the convertion returned '0' then check for decimal format
			if (result.asUint64()==0)
			{
				result= asUint();
			}

			// return whatever we found
			return result;
		}
	case FLAG:			return NLMISC::CEntityId();
	case EXTEND_TYPE:
		switch(_Value.ExType)
		{
		case ET_SHEET_ID:
			{
				return NLMISC::CEntityId();
			}
		case ET_ENTITY_ID:
			return NLMISC::CEntityId(_Value.ExData64);
		default:
			break;
		}
	default:
		break;
	}
	STOP("This should never happen!");
	return NLMISC::CEntityId();
}



inline NLMISC::CSheetId CPersistentDataRecord::CArg::asSheetId() const
{
	switch (_Type)
	{
	case STRUCT_BEGIN:
	case STRUCT_END:	BOMB("Can't extract a value from a structure delimiter", return NLMISC::CSheetId(0u));
	case SINT32:		return NLMISC::CSheetId((uint32)_Value.i32);
	case UINT32:		return NLMISC::CSheetId((uint32)_Value.i32);
	case SINT64:		return NLMISC::CSheetId((uint32)_Value.i64);
	case UINT64:		return NLMISC::CSheetId((uint32)_Value.i64);
	case FLOAT32:		return NLMISC::CSheetId((uint32)_Value.f32);
	case FLOAT64:		return NLMISC::CSheetId((uint32)_Value.f64);
	case STRING:		return NLMISC::CSheetId(_String);
	case FLAG:			return NLMISC::CSheetId();
	case EXTEND_TYPE:
		switch(_Value.ExType)
		{
		case ET_SHEET_ID:
			{
				return NLMISC::CSheetId (_Value.ExData32);
			}
		case ET_ENTITY_ID:
			return NLMISC::CSheetId();
		default:
			break;
		}
	default:			break;
	}
	STOP("This should never happen!");
	return NLMISC::CSheetId();
}

inline NLMISC::CSString CPersistentDataRecord::CArg::typeName() const
{
	switch (_Type)
	{
	case STRUCT_BEGIN:
	case STRUCT_END:	BOMB("Can't extract a value from a structure delimiter", return 0);
	case SINT32:		return "SINT32";
	case UINT32:		return "UINT32";
	case SINT64:		return "SINT64";
	case UINT64:		return "UINT64";
	case FLOAT32:		return "FLOAT";
	case FLOAT64:		return "DOUBLE";
	case STRING:		return "STRING";;
	case FLAG:			return "FLAG";
	case EXTEND_TYPE:
		switch(_Value.ExType)
		{
		case ET_SHEET_ID:	return "SHEET_ID";
		case ET_ENTITY_ID:	return "ENTITY_ID";
		default:			break;
		}
	default:			break;
	}
	STOP("This should never happen!");
	return "";
}

inline bool CPersistentDataRecord::CArg::setType(const std::string &name)
{
	_Type=SINT32;	if (typeName()==name) return true;
	_Type=UINT32;	if (typeName()==name) return true;
	_Type=SINT64;	if (typeName()==name) return true;
	_Type=UINT64;	if (typeName()==name) return true;
	_Type=FLOAT32;	if (typeName()==name) return true;
	_Type=FLOAT64;	if (typeName()==name) return true;
	_Type=FLAG;		if (typeName()==name) return true;
	_Type=STRING;	if (typeName()==name) return true;
	// special case for extended types
	_Type=EXTEND_TYPE;
	{
		_Value.ExType = ET_SHEET_ID;	if (typeName()==name) return true;
		_Value.ExType = ET_ENTITY_ID;	if (typeName()==name) return true;
	}
	DROP(("Failed to find match for type name '"+name+"' => defaulting to string").c_str(),return false);
}

inline void CPersistentDataRecord::CArg::setType(CPersistentDataRecord::CArg::TType value)
{
	BOMB_IF(value<0 || value>=CArg::NB_TYPE,"Invalid argument type",return);
	_Type=value;
}

inline bool CPersistentDataRecord::CArg::isFlag() const
{
	return _Type==FLAG;
}

inline bool CPersistentDataRecord::CArg::isExtended() const
{
	return isTypeExtended(_Type);
}

inline void CPersistentDataRecord::CArg::push(TToken token, std::vector<TToken>& tokenTable, std::vector<uint32>& argTable) const
{
	BOMB_IF( ((token<<3)>>3)!= token, "Invalid token - Insufficient numeric precision", return);
	switch (_Type)
	{
	case FLAG:
		tokenTable.push_back((token<<3)+CArg::type2Token(_Type));
		break;

	case STRING:	// drop though to INT32
	case FLOAT32:	// drop though to INT32
	case SINT32:
	case UINT32:
		tokenTable.push_back((token<<3)+CArg::type2Token(_Type));
		argTable.push_back(_Value.i32);
		break;

	case FLOAT64:	// drop though to INT64
	case SINT64:
	case UINT64:
	case EXTEND_TYPE:
		tokenTable.push_back((token<<3)+CArg::EXTEND_TOKEN);
		argTable.push_back(_Value.i32_1);
		tokenTable.push_back((token<<3)+CArg::type2Token(_Type));
		argTable.push_back(_Value.i32_2);
		if (_Type == EXTEND_TYPE && _Value.ExType >= ET_64_BIT_EXTENDED_TYPES)
		{
			// 96 bit extended type, add another 32 bit value
			tokenTable.push_back((token<<3)+CArg::type2Token(_Type));
			argTable.push_back(_Value.ex32_2);
		}
		break;

	default:
		BOMB("This should never happen!!!",return);
	}
}

inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::EntityId(NLMISC::CEntityId val)
{
	CArg arg;
	arg._Type= EXTEND_TYPE;
	arg._Value.ExType = ET_ENTITY_ID;
	arg._Value.ExData64 = val.getRawId();
	return arg;
}


inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::SheetId(NLMISC::CSheetId val)
{
	CArg arg;
	arg._Type= EXTEND_TYPE;
	arg._Value.ExType = ET_SHEET_ID;
	arg._Value.ExData32 = val.asInt();
	return arg;
}


inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::Int32(sint32 val)
{
	CArg arg;
	arg._Type= SINT32;
	arg._Value.i32= val;
	return arg;
}

inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::Int32(uint32 val)
{
	CArg arg;
	arg._Type= UINT32;
	arg._Value.i32= val;
	return arg;
}

inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::Int64(sint64 val)
{
	CArg arg;
	arg._Type= SINT64;
	arg._Value.i64= val;
	return arg;
}

inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::Int64(uint64 val)
{
	CArg arg;
	arg._Type= UINT64;
	arg._Value.i64= val;
	return arg;
}

inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::Float32(float val)
{
	CArg arg;
	arg._Type= FLOAT32;
	arg._Value.f32= val;
	return arg;
}

inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::Float64(double val)
{
	CArg arg;
	arg._Type= FLOAT64;
	arg._Value.f64= val;
	return arg;
}

inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::String(const std::string& value,CPersistentDataRecord& pdr)
{
	CArg arg;
	arg._Type= STRING;
	arg._Value.i32= pdr.addString(value);
	arg._String= value;
	return arg;
}

inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::UCString(const ucstring& value,CPersistentDataRecord& pdr)
{
	NLMISC::CSString s = value.toUtf8();
	CArg arg;
	arg._Type= STRING;
	arg._Value.i32= pdr.addString(s);
	arg._String= s;
	return arg;
}

inline CPersistentDataRecord::CArg CPersistentDataRecord::CArg::Flag()
{
	CArg arg;
	arg._Type= FLAG;
	arg._Value.i32= 1;
	return arg;
}

inline CPersistentDataRecord::CArg::TType CPersistentDataRecord::CArg::token2Type(uint32 token,bool extend)
{
	switch (token)
	{
	case BEGIN_TOKEN:		return STRUCT_BEGIN;
	case END_TOKEN:			return STRUCT_END;
	case FLAG_TOKEN:		return FLAG;
	case SINT_TOKEN:		return extend? SINT64: SINT32;
	case UINT_TOKEN:		return extend? UINT64: UINT32;
	case FLOAT_TOKEN:		return extend? FLOAT64: FLOAT32;
	case STRING_TOKEN:		return extend? EXTEND_TYPE: STRING;
	}
	STOP("This should never happen!");
	return CArg::TType(0);
}

inline CPersistentDataRecord::TToken CPersistentDataRecord::CArg::type2Token(uint32 type)
{
	switch (type)
	{
	case STRUCT_BEGIN:	return BEGIN_TOKEN;
	case STRUCT_END:	return END_TOKEN;
	case FLAG:			return FLAG_TOKEN;
	case SINT32:		return SINT_TOKEN;
	case UINT32:		return UINT_TOKEN;
	case FLOAT32:		return FLOAT_TOKEN;
	case STRING:		return STRING_TOKEN;
	case SINT64:		return SINT_TOKEN;
	case UINT64:		return UINT_TOKEN;
	case FLOAT64:		return FLOAT_TOKEN;
	case EXTEND_TYPE:	return STRING_TOKEN;
	}
	STOP("This should never happen!");
	return 0;
}

inline bool CPersistentDataRecord::CArg::isTypeExtended(uint32 type)
{
	switch (type)
	{
	case STRUCT_BEGIN:
	case STRUCT_END:	BOMB("Can't extract a value from a structure delimiter", return 0);
	case SINT32:
	case UINT32:
	case FLOAT32:
	case STRING:
	case FLAG:			return false;
	case SINT64:
	case UINT64:
	case FLOAT64:
	case EXTEND_TYPE:	return true;
	}
	STOP("This should never happen!");
	return false;
}

