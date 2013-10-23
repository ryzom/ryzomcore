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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef LOGGER_SERVICE_ITF
#define LOGGER_SERVICE_ITF
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "nel/misc/entity_id.h"
	
#include "nel/misc/sheet_id.h"
	
#include "game_share/inventories.h"
	
#include <vector>
	
namespace LGS
{
	
	class TParamDesc;

	class TListParamValues;

	class TLogDefinition;

	class TLogInfo;



	struct TSupportedParamType
	{
		enum TValues
		{
			spt_uint32,
			spt_uint64,
			spt_sint32,
			spt_float,
			spt_string,
			spt_entityId,
			spt_sheetId,
			spt_itemId,
			/// the highest valid value in the enum
			last_enum_item = spt_itemId,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 8
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(spt_uint32, 0));
				indexTable.insert(std::make_pair(spt_uint64, 1));
				indexTable.insert(std::make_pair(spt_sint32, 2));
				indexTable.insert(std::make_pair(spt_float, 3));
				indexTable.insert(std::make_pair(spt_string, 4));
				indexTable.insert(std::make_pair(spt_entityId, 5));
				indexTable.insert(std::make_pair(spt_sheetId, 6));
				indexTable.insert(std::make_pair(spt_itemId, 7));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(spt_uint32)
				NL_STRING_CONVERSION_TABLE_ENTRY(spt_uint64)
				NL_STRING_CONVERSION_TABLE_ENTRY(spt_sint32)
				NL_STRING_CONVERSION_TABLE_ENTRY(spt_float)
				NL_STRING_CONVERSION_TABLE_ENTRY(spt_string)
				NL_STRING_CONVERSION_TABLE_ENTRY(spt_entityId)
				NL_STRING_CONVERSION_TABLE_ENTRY(spt_sheetId)
				NL_STRING_CONVERSION_TABLE_ENTRY(spt_itemId)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TSupportedParamType()
			: _Value(invalid_val)
		{
		}
		TSupportedParamType(TValues value)
			: _Value(value)
		{
		}

		TSupportedParamType(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TSupportedParamType &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TSupportedParamType &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TSupportedParamType &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TSupportedParamType &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TSupportedParamType &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TSupportedParamType &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
	


//	template <class T>	LGS::TSupportedParamType getParamType();
//	template <class T>	LGS::TSupportedParamType getParamType<uint32>() { return LGS::TSupportedParamType::spt_uint32;}
//	template <class T>	LGS::TSupportedParamType getParamType<uint64>() { return LGS::TSupportedParamType::spt_uint64;}
//	template <class T>	LGS::TSupportedParamType getParamType<sint32>() { return LGS::TSupportedParamType::spt_sint32;}
//	template <class T>	LGS::TSupportedParamType getParamType<float>() { return LGS::TSupportedParamType::spt_float;}
//	template <class T>	LGS::TSupportedParamType getParamType<std::string>() { return LGS::TSupportedParamType::spt_string;}
//	template <class T>	LGS::TSupportedParamType getParamType<NLMISC::CEntityId>() { return LGS::TSupportedParamType::spt_entityId;}
//	template <class T>	LGS::TSupportedParamType getParamType<NLMISC::CSheetId>() { return LGS::TSupportedParamType::spt_sheetId;}
//	template <class T>	LGS::TSupportedParamType getParamType<INVENTORIES::TItemId>() { return LGS::TSupportedParamType::spt_itemId;}

	struct TParamValue
	{
		TParamValue()
			:	_Type(TSupportedParamType::invalid_val)
		{}

		TParamValue(uint32 value)
		{
			_Type = TSupportedParamType::spt_uint32;
			_UInt32Val = value;
		}

		TParamValue(uint64 value)
		{
			_Type = TSupportedParamType::spt_uint64;
			_UInt64Val = value;
		}

		TParamValue(bool value)
		{
			_Type = TSupportedParamType::spt_uint32;
			_UInt32Val = value ? 1 : 0;
		}

		TParamValue(sint32 value)
		{
			_Type = TSupportedParamType::spt_sint32;
			_SInt32Val = value;
		}

		TParamValue(float value)
		{
			_Type = TSupportedParamType::spt_float;
			_FloatVal = value;
		}

		TParamValue(const std::string &value)
		{
			_Type = TSupportedParamType::spt_string;
			_StringVal = value;
		}

		TParamValue(const NLMISC::CEntityId &value)
		{
			_Type = TSupportedParamType::spt_entityId;
			_EntityId = value;
		}

		TParamValue(const NLMISC::CSheetId &value)
		{
			_Type = TSupportedParamType::spt_sheetId;
			_SheetId = value;
		}

		TParamValue(INVENTORIES::TItemId itemId)
		{
			_Type = TSupportedParamType::spt_itemId;
			_ItemId = itemId;
		}

		TParamValue &operator = (const TParamValue &other)
		{
			_Type = other._Type;

			switch (_Type.getValue())
			{
			case TSupportedParamType::spt_uint32:
				_UInt32Val = other._UInt32Val;
				break;
			case TSupportedParamType::spt_uint64:
				_UInt64Val = other._UInt64Val;
				break;
			case TSupportedParamType::spt_sint32:
				_SInt32Val = other._SInt32Val;
				break;
			case TSupportedParamType::spt_float:
				_FloatVal = other._FloatVal;
				break;
			case TSupportedParamType::spt_string:
				_StringVal = other._StringVal;
				break;
			case TSupportedParamType::spt_entityId:
				_EntityId = other._EntityId;
				break;
			case TSupportedParamType::spt_sheetId:
				_SheetId = other._SheetId;
				break;
			case TSupportedParamType::spt_itemId:
				_ItemId = other._ItemId;
				break;
			default:
				nlstop;
			};

			return *this;
		}

		void serial(NLMISC::IStream &s)
		{
			// read the type 
			s.serial(_Type);

			// serial the value
			switch (_Type.getValue())
			{
			case TSupportedParamType::spt_uint32:
				s.serial(_UInt32Val);
				break;
			case TSupportedParamType::spt_uint64:
				s.serial(_UInt64Val);
				break;
			case TSupportedParamType::spt_sint32:
				s.serial(_SInt32Val);
				break;
			case TSupportedParamType::spt_float:
				s.serial(_FloatVal);
				break;
			case TSupportedParamType::spt_string:
				s.serial(_StringVal);
				break;
			case TSupportedParamType::spt_entityId:
				s.serial(_EntityId);
				break;
			case TSupportedParamType::spt_sheetId:
				s.serial(_SheetId);
				break;
			case TSupportedParamType::spt_itemId:
				s.serial(_ItemId);
				break;
			default:
				nlstop;
			};
		}

		std::string toString() const
		{
			switch (_Type.getValue())
			{
			case TSupportedParamType::spt_uint32:
				return NLMISC::toString(_UInt32Val);
				break;
			case TSupportedParamType::spt_uint64:
				return NLMISC::toString(_UInt64Val);
				break;
			case TSupportedParamType::spt_sint32:
				return NLMISC::toString(_SInt32Val);
				break;
			case TSupportedParamType::spt_float:
				return NLMISC::toString(_FloatVal);
				break;
			case TSupportedParamType::spt_string:
				return _StringVal;
				break;
			case TSupportedParamType::spt_entityId:
				return _EntityId.toString();
				break;
			case TSupportedParamType::spt_sheetId:
				return _SheetId.toString();
				break;
			case TSupportedParamType::spt_itemId:
				return NLMISC::toString(_ItemId);
				break;
			default:
				nlstop;

				return "";
			};
		}

		TSupportedParamType getType() const
		{
			return _Type;
		}

		uint32 get_uint32() const
		{
			nlassert(_Type == TSupportedParamType::spt_uint32);
			return _UInt32Val;
		}
		uint64 get_uint64() const
		{
			nlassert(_Type == TSupportedParamType::spt_uint64);
			return _UInt64Val;
		}
		sint32 get_sint32() const
		{
			nlassert(_Type == TSupportedParamType::spt_sint32);
			return _SInt32Val;
		}
		float get_float() const
		{
			nlassert(_Type == TSupportedParamType::spt_float);
			return _FloatVal;
		}
		const std::string &get_string() const
		{
			nlassert(_Type == TSupportedParamType::spt_string);
			return _StringVal;
		}
		const NLMISC::CEntityId &get_entityId() const
		{
			nlassert(_Type == TSupportedParamType::spt_entityId);
			return _EntityId;
		}
		const NLMISC::CSheetId &get_sheetId() const
		{
			nlassert(_Type == TSupportedParamType::spt_sheetId);
			return _SheetId;
		}
		const INVENTORIES::TItemId &get_itemId() const
		{
			nlassert(_Type == TSupportedParamType::spt_itemId);
			return _ItemId;
		}


		const uint32 &get(const uint32 *typeTag) const
		{
			nlassert(_Type == TSupportedParamType::spt_uint32);
			return _UInt32Val;
		}

		const uint64 &get(const uint64 *typeTag) const
		{
			nlassert(_Type == TSupportedParamType::spt_uint64);
			return _UInt64Val;
		}

		const sint32 &get(const sint32 *typeTag) const
		{
			nlassert(_Type == TSupportedParamType::spt_sint32);
			return _SInt32Val;
		}

		const float &get(const float *typeTag) const
		{
			nlassert(_Type == TSupportedParamType::spt_float);
			return _FloatVal;
		}

		const std::string &get(const std::string *typeTag) const
		{
			nlassert(_Type == TSupportedParamType::spt_string);
			return _StringVal;
		}

		const NLMISC::CEntityId &get(const NLMISC::CEntityId *typeTag) const
		{
			nlassert(_Type == TSupportedParamType::spt_entityId);
			return _EntityId;
		}

		const NLMISC::CSheetId &get(const NLMISC::CSheetId *typeTag) const
		{
			nlassert(_Type == TSupportedParamType::spt_sheetId);
			return _SheetId;
		}

		const INVENTORIES::TItemId &get(const INVENTORIES::TItemId *typeTag) const
		{
			nlassert(_Type == TSupportedParamType::spt_itemId);
			return _ItemId;
		}

		bool operator == (const TParamValue &other) const
		{
			if (_Type != other._Type)
				return false;

			switch (_Type.getValue())
			{
			case TSupportedParamType::spt_uint32:
				return _UInt32Val == other._UInt32Val;
				break;
			case TSupportedParamType::spt_uint64:
				return _UInt64Val == other._UInt64Val;
				break;
			case TSupportedParamType::spt_sint32:
				return _SInt32Val == other._SInt32Val;
				break;
			case TSupportedParamType::spt_float:
				return _FloatVal == other._FloatVal;
				break;
			case TSupportedParamType::spt_string:
				return _StringVal == other._StringVal;
				break;
			case TSupportedParamType::spt_entityId:
				return _EntityId == other._EntityId;
				break;
			case TSupportedParamType::spt_sheetId:
				return _SheetId == other._SheetId;
				break;
			case TSupportedParamType::spt_itemId:
				return _ItemId == other._ItemId;
				break;
			default:
				return false;;
			};
		}

		bool operator < (const TParamValue &other) const
		{
			if (_Type != other._Type)
				return false;

			switch (_Type.getValue())
			{
			case TSupportedParamType::spt_uint32:
				return _UInt32Val < other._UInt32Val;
				break;
			case TSupportedParamType::spt_uint64:
				return _UInt64Val < other._UInt64Val;
				break;
			case TSupportedParamType::spt_sint32:
				return _SInt32Val < other._SInt32Val;
				break;
			case TSupportedParamType::spt_float:
				return _FloatVal < other._FloatVal;
				break;
			case TSupportedParamType::spt_string:
				return _StringVal < other._StringVal;
				break;
			case TSupportedParamType::spt_entityId:
				return _EntityId < other._EntityId;
				break;
			case TSupportedParamType::spt_sheetId:
				return _SheetId < other._SheetId;
				break;
			case TSupportedParamType::spt_itemId:
				return _ItemId < other._ItemId;
				break;
			default:
				return false;;
			};
		}

	private:
		TSupportedParamType	_Type;

		union
		{
			uint32		_UInt32Val;
			uint64		_UInt64Val;
			sint32		_SInt32Val;
			float		_FloatVal;
		};
		std::string				_StringVal;
		NLMISC::CEntityId		_EntityId;
		NLMISC::CSheetId		_SheetId;
		INVENTORIES::TItemId	_ItemId;
	};


			// Describe a param for a log entry
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TParamDesc
	{
	protected:
		// Name of the parameter
		std::string	_Name;
		// Type of the parameter
		TSupportedParamType	_Type;
		// Flag indicating that this parameter is a list
		bool	_List;
	public:
		// Name of the parameter
		const std::string &getName() const
		{
			return _Name;
		}

		std::string &getName()
		{
			return _Name;
		}


		void setName(const std::string &value)
		{


				_Name = value;

				
		}
			// Type of the parameter
		TSupportedParamType getType() const
		{
			return _Type;
		}

		void setType(TSupportedParamType value)
		{

				_Type = value;

		}
			// Flag indicating that this parameter is a list
		bool getList() const
		{
			return _List;
		}

		void setList(bool value)
		{

				_List = value;

		}
	
		bool operator == (const TParamDesc &other) const
		{
			return _Name == other._Name
				&& _Type == other._Type
				&& _List == other._List;
		}


		// constructor
		TParamDesc()
		{
			// Default initialisation
			_List = false;

		}
		
		void serial(NLMISC::IStream &s)
		{
			s.serial(_Name);
			s.serial(_Type);
			s.serial(_List);

		}
		

	private:
	

	};


		// A vector of parameter value
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TListParamValues
	{
	protected:
		// 
		std::list < TParamValue >	_Params;
	public:
		// 
		const std::list < TParamValue > &getParams() const
		{
			return _Params;
		}

		std::list < TParamValue > &getParams()
		{
			return _Params;
		}


		void setParams(const std::list < TParamValue > &value)
		{


				_Params = value;

				
		}
	
		bool operator == (const TListParamValues &other) const
		{
			return _Params == other._Params;
		}


		// constructor
		TListParamValues()
		{

		}
		
		void serial(NLMISC::IStream &s)
		{
			s.serialCont(_Params);

		}
		

	private:
	

	};


		// Definition of a log entry
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TLogDefinition
	{
	protected:
		// The name of the log, used to identify the log
		std::string	_LogName;
		// This log is a context log
		bool	_Context;
		// The textual content of the log, contains '__CLOSE_CONTEXT__' to close a log context
		std::string	_LogText;
		// 
		std::vector < TParamDesc >	_Params;
		// 
		std::vector < TParamDesc >	_ListParams;
	public:
		// The name of the log, used to identify the log
		std::string getLogName() const
		{
			return _LogName;
		}

		void setLogName(std::string value)
		{

				_LogName = value;

		}
			// This log is a context log
		bool getContext() const
		{
			return _Context;
		}

		void setContext(bool value)
		{

				_Context = value;

		}
			// The textual content of the log, contains '__CLOSE_CONTEXT__' to close a log context
		std::string getLogText() const
		{
			return _LogText;
		}

		void setLogText(std::string value)
		{

				_LogText = value;

		}
			// 
		const std::vector < TParamDesc > &getParams() const
		{
			return _Params;
		}

		std::vector < TParamDesc > &getParams()
		{
			return _Params;
		}


		void setParams(const std::vector < TParamDesc > &value)
		{


				_Params = value;

				
		}
			// 
		const std::vector < TParamDesc > &getListParams() const
		{
			return _ListParams;
		}

		std::vector < TParamDesc > &getListParams()
		{
			return _ListParams;
		}


		void setListParams(const std::vector < TParamDesc > &value)
		{


				_ListParams = value;

				
		}
	
		bool operator == (const TLogDefinition &other) const
		{
			return _LogName == other._LogName
				&& _Context == other._Context
				&& _LogText == other._LogText
				&& _Params == other._Params
				&& _ListParams == other._ListParams;
		}


		// constructor
		TLogDefinition()
		{
			// Default initialisation
			_Context = false;

		}
		
		void serial(NLMISC::IStream &s)
		{
			s.serial(_LogName);
			s.serial(_Context);
			s.serial(_LogText);
			s.serialCont(_Params);
			s.serialCont(_ListParams);

		}
		

	private:
	

	};


		// A log entry data. This contains the parameter of a log 
	// entry to be stored in the log archive
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TLogInfo
	{
	protected:
		// 
		std::string	_LogName;
		// 
		uint32	_TimeStamp;
		// 
		std::vector < TParamValue >	_Params;
		// 
		std::vector < TListParamValues >	_ListParams;
	public:
		// 
		std::string getLogName() const
		{
			return _LogName;
		}

		void setLogName(std::string value)
		{

				_LogName = value;

		}
			// 
		uint32 getTimeStamp() const
		{
			return _TimeStamp;
		}

		void setTimeStamp(uint32 value)
		{

				_TimeStamp = value;

		}
			// 
		const std::vector < TParamValue > &getParams() const
		{
			return _Params;
		}

		std::vector < TParamValue > &getParams()
		{
			return _Params;
		}


		void setParams(const std::vector < TParamValue > &value)
		{


				_Params = value;

				
		}
			// 
		const std::vector < TListParamValues > &getListParams() const
		{
			return _ListParams;
		}

		std::vector < TListParamValues > &getListParams()
		{
			return _ListParams;
		}


		void setListParams(const std::vector < TListParamValues > &value)
		{


				_ListParams = value;

				
		}
	
		bool operator == (const TLogInfo &other) const
		{
			return _LogName == other._LogName
				&& _TimeStamp == other._TimeStamp
				&& _Params == other._Params
				&& _ListParams == other._ListParams;
		}


		// constructor
		TLogInfo()
		{

		}
		
		void serial(NLMISC::IStream &s)
		{
			s.serial(_LogName);
			s.serial(_TimeStamp);
			s.serialCont(_Params);
			s.serialCont(_ListParams);

		}
		

	private:
	

	};


	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CLoggerServiceSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CLoggerServiceSkel>	TInterceptor;
	protected:
		CLoggerServiceSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CLoggerServiceSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors 
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {}
	
		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CLoggerServiceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void registerClient_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reportLog_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CLoggerServiceSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// A logger client register itself wy providing it's definition of 
		// the log content. It is mandatory that ALL client share
		// Exactly the same definition of log.
		virtual void registerClient(NLNET::IModuleProxy *sender, uint32 shardId, const std::vector < TLogDefinition > &logDef) =0;
		// A client send a log
		virtual void reportLog(NLNET::IModuleProxy *sender, const std::vector < TLogInfo > &logInfos) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CLoggerServiceProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CLoggerServiceSkel	*_LocalModuleSkel;


	public:
		CLoggerServiceProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "LoggerService");
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CLoggerServiceSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CLoggerServiceProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// A logger client register itself wy providing it's definition of 
		// the log content. It is mandatory that ALL client share
		// Exactly the same definition of log.
		void registerClient(NLNET::IModule *sender, uint32 shardId, const std::vector < TLogDefinition > &logDef);
		// A client send a log
		void reportLog(NLNET::IModule *sender, const std::vector < TLogInfo > &logInfos);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_registerClient(NLNET::CMessage &__message, uint32 shardId, const std::vector < TLogDefinition > &logDef);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportLog(NLNET::CMessage &__message, const std::vector < TLogInfo > &logInfos);
	



	};

}
	
#endif
