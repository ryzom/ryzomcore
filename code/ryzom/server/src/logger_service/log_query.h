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


#ifndef LOG_QUERY_H
#define LOG_QUERY_H

#include "nel/misc/types_nl.h"

#include <memory>
#include <iterator>

#include "nel/misc/common.h"

#include "server_share/logger_service_itf.h"
#include "game_share/shard_names.h"

#include "logger_service.h"
#include "log_storage.h"

struct EIncompatibleType : public NLMISC::Exception
{
	LGS::TSupportedParamType	ValueType;
	LGS::TSupportedParamType	RequiredType;

	EIncompatibleType(LGS::TSupportedParamType valueType, LGS::TSupportedParamType requiredType)
		:	ValueType(valueType),
			RequiredType(requiredType)
	{
	}

	virtual const char	*what() const throw()
	{
		return "Incompatible types";
	}

};

/// A set of log entry index, used as 'result set' for a predicate node
typedef std::set<uint32>	TLogEntries;

/// Define a time slice for opening log file
struct TTimeSlice
{
	/// the inclusive start date
	uint32		StartDate;
	/// The excluded end date
	uint32		EndDate;	
};

const TTimeSlice FullTimeSlice = {0, ~0};

/// Defile the complete selected time line
typedef std::vector<TTimeSlice>	TTimeLine;


/// The token returned by the lexer function (or manually set by parser)
enum TTokenType
{
	tt_ID,
	tt_TYPE,
	tt_STRING,
	tt_INT,
	tt_NAKED_HEXA,
	tt_LONG,
	tt_FLOAT,
	tt_DATE,
	tt_ENTITY_ID,
	tt_ITEM_ID,
	tt_OPEN_PAR,
	tt_CLOSE_PAR,
	tt_OPEN_BRACKET,
	tt_CLOSE_BRACKET,
	tt_OPEN_BRACE,
	tt_CLOSE_BRACE,
	tt_COLON,
	tt_DASH,
	tt_DOT,
	tt_LESS,
	tt_LESS_EQUAL,
	tt_GREATER,
	tt_GREATER_EQUAL,
	tt_EQUAL,
	tt_like,
	tt_full_context,
	tt_output_prefix,
	tt_NOT_EQUAL,
	tt_or,
	tt_and,
	tt_yesterday,
	tt_secs,
	tt_mins,
	tt_hours,
	tt_days,
	tt_weeks,
	tt_months,
	tt_years,

	tt_EOF
};



/// Base class for the parsed request tree
struct TQueryNode
{
	TQueryNode	*LeftNode;
	TQueryNode	*RightNode;
//	TQueryNode	*ParentNode;

	TQueryNode()
		:	LeftNode(NULL),
			RightNode(NULL)
//			ParentNode(NULL)
	{}

	virtual ~TQueryNode()
	{
		if (LeftNode)
			delete LeftNode;
		if (RightNode)
			delete RightNode;
	}

	// init the node
	virtual bool init() =0;


	/// Evaluate the node result
	virtual TLogEntries evalNode(const CLogStorage &logs) =0;

	/// Evaluate a date against the predicate (only test the date predicate)
	virtual TTimeLine evalDate() =0;

	/// dump the node
	void dump(NLMISC::CLog &log, const std::string &tab) const
	{
		dumpNode(log, tab);

		if (LeftNode)
		{
			log.displayNL("%s + => ", tab.c_str());
			LeftNode->dump(log, tab+"\t");
		}
		if (RightNode)
		{
			log.displayNL("%s + => ", tab.c_str());
			RightNode->dump(log, tab+"\t");
		}
	}

	/// Display node content
	virtual void dumpNode(NLMISC::CLog &log, const std::string &tab) const =0;

};

//extern LGS::TSupportedParamType ConversionTable[LGS::TSupportedParamType::nb_enum_items][LGS::TSupportedParamType::nb_enum_items];
extern bool ConversionTable[LGS::TSupportedParamType::nb_enum_items][LGS::TSupportedParamType::nb_enum_items];


/// A type to store the list of selected parameter and log types
struct TSelectedParam
{
	uint32	LogDefIndex;
	bool	ListParam;
	uint32	ParamIndex;
};

/// For each param id, associate a vector of log def index
typedef std::map<CLogStorage::TLogParamId, std::vector<TSelectedParam> > TSelectedParams;

/// A big function to do all allowed type conversion
inline LGS::TParamValue convertParam(const LGS::TParamValue &value, LGS::TSupportedParamType type)
{
	switch (value.getType().getValue())
	{
	case LGS::TSupportedParamType::spt_uint32:
		switch (type.getValue())
		{
		case LGS::TSupportedParamType::spt_uint32:
			return value;
		case LGS::TSupportedParamType::spt_uint64:
			return LGS::TParamValue((uint64)value.get_uint32());
		case LGS::TSupportedParamType::spt_sint32:
			return LGS::TParamValue((sint32)value.get_uint32());
		case LGS::TSupportedParamType::spt_float:
			return LGS::TParamValue((float)value.get_uint32());
		case LGS::TSupportedParamType::spt_string:
			return LGS::TParamValue(NLMISC::toString(value.get_uint32()));
		case LGS::TSupportedParamType::spt_entityId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_sheetId:
			return LGS::TParamValue(NLMISC::CSheetId(value.get_uint32()));
		case LGS::TSupportedParamType::spt_itemId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		}
		break;
	case LGS::TSupportedParamType::spt_uint64:
		switch (type.getValue())
		{
		case LGS::TSupportedParamType::spt_uint32:
			return LGS::TParamValue((uint32)value.get_uint64());
		case LGS::TSupportedParamType::spt_uint64:
			return value;
		case LGS::TSupportedParamType::spt_sint32:
			return LGS::TParamValue((sint32)value.get_uint64());
		case LGS::TSupportedParamType::spt_float:
			return LGS::TParamValue((float)value.get_uint64());
		case LGS::TSupportedParamType::spt_string:
			return LGS::TParamValue(NLMISC::toString(value.get_uint64()));
		case LGS::TSupportedParamType::spt_entityId:
			return LGS::TParamValue(NLMISC::CEntityId(value.get_uint64()));
		case LGS::TSupportedParamType::spt_sheetId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_itemId:
			return LGS::TParamValue(INVENTORIES::TItemId(value.get_uint64()));
		}
		break;
	case LGS::TSupportedParamType::spt_sint32:
		switch (type.getValue())
		{
		case LGS::TSupportedParamType::spt_uint32:
			return LGS::TParamValue((uint32)value.get_sint32());
		case LGS::TSupportedParamType::spt_uint64:
			return LGS::TParamValue((uint64)value.get_sint32());
		case LGS::TSupportedParamType::spt_sint32:
			return value;
		case LGS::TSupportedParamType::spt_float:
			return LGS::TParamValue((float)value.get_sint32());
		case LGS::TSupportedParamType::spt_string:
			return LGS::TParamValue(NLMISC::toString(value.get_sint32()));
		case LGS::TSupportedParamType::spt_entityId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_sheetId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_itemId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		}
		break;
	case LGS::TSupportedParamType::spt_float:
		switch (type.getValue())
		{
		case LGS::TSupportedParamType::spt_uint32:
			return LGS::TParamValue((uint64)value.get_float());
		case LGS::TSupportedParamType::spt_uint64:
			return LGS::TParamValue((uint64)value.get_float());
		case LGS::TSupportedParamType::spt_sint32:
			return LGS::TParamValue((sint32)value.get_float());
		case LGS::TSupportedParamType::spt_float:
			return value;
		case LGS::TSupportedParamType::spt_string:
			return LGS::TParamValue(NLMISC::toString(value.get_float()));
		case LGS::TSupportedParamType::spt_entityId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_sheetId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_itemId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		}
		break;
	case LGS::TSupportedParamType::spt_string:
		switch (type.getValue())
		{
		case LGS::TSupportedParamType::spt_uint32:
		{
			uint32 tmp;
			NLMISC::fromString(value.get_string(), tmp);
			return LGS::TParamValue(tmp);
		}
		case LGS::TSupportedParamType::spt_uint64:
			return LGS::TParamValue((uint64)atol(value.get_string().c_str()));
		case LGS::TSupportedParamType::spt_sint32:
		{
			sint32 tmp;
			NLMISC::fromString(value.get_string(), tmp);
			return LGS::TParamValue(tmp);
		}
		case LGS::TSupportedParamType::spt_float:
			return LGS::TParamValue((float)atof(value.get_string().c_str()));
		case LGS::TSupportedParamType::spt_string:
			return value;
		case LGS::TSupportedParamType::spt_entityId:
			return LGS::TParamValue(NLMISC::CEntityId(value.get_string()));
		case LGS::TSupportedParamType::spt_sheetId:
			return LGS::TParamValue(NLMISC::CSheetId(value.get_string()));
		case LGS::TSupportedParamType::spt_itemId:
			return LGS::TParamValue(INVENTORIES::TItemId(value.get_string()));
		}
		break;
	case LGS::TSupportedParamType::spt_entityId:
		switch (type.getValue())
		{
		case LGS::TSupportedParamType::spt_uint32:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_uint64:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_sint32:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_float:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_string:
			return value.get_entityId().toString();
		case LGS::TSupportedParamType::spt_entityId:
			return value;
		case LGS::TSupportedParamType::spt_sheetId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_itemId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		}
		break;
	case LGS::TSupportedParamType::spt_sheetId:
		switch (type.getValue())
		{
		case LGS::TSupportedParamType::spt_uint32:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_uint64:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_sint32:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_float:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_string:
			return value.get_sheetId().toString(true);
		case LGS::TSupportedParamType::spt_entityId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_sheetId:
			return value;
		case LGS::TSupportedParamType::spt_itemId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		}
		break;
	case LGS::TSupportedParamType::spt_itemId:
		switch (type.getValue())
		{
		case LGS::TSupportedParamType::spt_uint32:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_uint64:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_sint32:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_float:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_string:
			return value.get_itemId().toString();
		case LGS::TSupportedParamType::spt_entityId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_sheetId:
				throw EIncompatibleType(value.getType().getValue(), type.getValue());
		case LGS::TSupportedParamType::spt_itemId:
			return value;
		}
		break;
	}

	nlstop;
	return LGS::TParamValue();
}

/// The combiner node, may be specialized with 'or' or 'and' combiner.
template <class Combiner>
struct TCombineNode : public TQueryNode
{
	virtual bool init()
	{
		return true;
	}

	/// Evaluate the node result
	virtual TLogEntries evalNode(const CLogStorage &logs)
	{
		TLogEntries leftEntries, rightEntries;

		nlassert(LeftNode && RightNode);

		leftEntries = LeftNode->evalNode(logs);
		rightEntries = RightNode->evalNode(logs);

		// combine the two entries
		Combiner comb;

		return comb(leftEntries, rightEntries);
	}

	/// Evaluate a date against the predicate (only test the date predicate)
	virtual TTimeLine evalDate()
	{
		nlassert(LeftNode && RightNode);

		TTimeLine left = LeftNode->evalDate();
		TTimeLine right = RightNode->evalDate();

		Combiner comb;

		return comb(left, right);
	}


	virtual void dumpNode(NLMISC::CLog &log, const std::string &tab) const
	{
		log.displayNL("%s%s", tab.c_str(), typeid(this).name());
	}

};


/// Given the parameter index selected, build the list of logs that use the selected param
inline void buildSelectedLogList(const std::vector<uint32> &selectedIndex, const CLogStorage &logs, const std::vector<TSelectedParam> &selectedParams, TLogEntries &result)
{
	// for each log type to test
	for (uint j=0; j<selectedParams.size(); ++j)
	{
		uint entryIndex = 0;
		// for each selected parameter index
		for (uint k=0; k<selectedIndex.size(); ++k)
		{
			uint32 currentParamIndex = selectedIndex[k];
			
			while (entryIndex < logs._DiskLogEntries.size())
			{
				const CLogStorage::TDiskLogEntry &dle = logs._DiskLogEntries[entryIndex];
				
				if (dle.LogType == selectedParams[j].LogDefIndex)
				{
					if (selectedParams[j].ListParam)
					{
						// read the variable parameter
						const CLogStorage::TParamIndex &indexes = dle.ListParamIndex[selectedParams[j].ParamIndex];
						if (indexes.empty() 
							|| indexes.back() < currentParamIndex)
						{
							// this log is empty or too old, advance to next one
							++entryIndex;
							continue;
						}
						else if (indexes.front() <=  currentParamIndex)
						{
							// we found the list that contains the selected index
							result.insert(entryIndex);
							++entryIndex;
							
							// advance to next parameter
							break;
						}
						else
						{
							// we have passed over the logs, no matching found!
							break;
						}
					}
					else
					{
						// the log type match
						if (dle.ParamIndex[selectedParams[j].ParamIndex] < currentParamIndex)
						{
							// this log is too old, advance to next one
							++entryIndex;
							continue;
						}
						else if (dle.ParamIndex[selectedParams[j].ParamIndex] == currentParamIndex)
						{
							// this is the log we are looking for
							result.insert(entryIndex);
							++entryIndex;
							
							// advance to next parameter
							break;
						}
						else
						{
							// we have passed over the logs, no matching found!
							break;
						}
					}
				}
				else
					++entryIndex;
			}
		}
	}
}


/// The predicate node. This node apply the operator with the reference value and the matching log paramters type
template <class Operator>
struct TTypePredicateNode : public TQueryNode
{
	/// The reference value used to compute the predicate
	LGS::TParamValue			_RefValue;
	/// The type of parameter that we check
	LGS::TSupportedParamType	_ParamType;
	
	/// The log info vector
	const TLogDefinitions		&_LogDefs;


	/// The list of parameter that match the name/type
	TSelectedParams				_SelectedParams;


	TTypePredicateNode(const LGS::TParamValue &refValue, const LGS::TSupportedParamType &paramType, const TLogDefinitions &logDefs )
		:	_RefValue(refValue),
			_ParamType(paramType),
			_LogDefs(logDefs)
	{
	}

	bool init()
	{
		// Build a list of compatible parameters
		for (uint i=0; i<_LogDefs.size(); ++i)
		{
			const LGS::TLogDefinition &logDef = _LogDefs[i];

			// read each param
			for (uint j=0; j<logDef.getParams().size(); ++j)
			{
				const LGS::TParamDesc &paramDesc = logDef.getParams()[j];

				// check the parameter type and conversion validity
				if (paramDesc.getType() == _ParamType)
				{
					CLogStorage::TLogParamId lpi;
					lpi.ParamName = paramDesc.getName();
					lpi.ParamType = paramDesc.getType();
					TSelectedParam	sp;
					sp.LogDefIndex = i;
					sp.ListParam = false;
					sp.ParamIndex = j;
					// select this one
//					_SelectedParams.insert(std::make_pair(logDef.getLogName(), j));
					_SelectedParams[lpi].push_back(sp);
				}
			}
			// read each list param
			for (uint j=0; j<logDef.getListParams().size(); ++j)
			{
				const LGS::TParamDesc &paramDesc = logDef.getListParams()[j];

				// check the parameter type and conversion validity
				if (paramDesc.getType() == _ParamType)
				{
					CLogStorage::TLogParamId lpi;
					lpi.ParamName = paramDesc.getName();
					lpi.ParamType = paramDesc.getType();
					TSelectedParam	sp;
					sp.LogDefIndex = i;
					sp.ListParam = true;
					sp.ParamIndex = j;
					// select this one
//					_SelectedParams.insert(std::make_pair(logDef.getLogName(), j));
					_SelectedParams[lpi].push_back(sp);
				}
			}
		}
		return !_SelectedParams.empty();
	}

	TLogEntries evalNode(const CLogStorage &logs)
	{
		Operator op;
		TLogEntries ret;

		// parse each param table, then look back in the logs for the selected entry
		TSelectedParams::iterator first(_SelectedParams.begin()), last(_SelectedParams.end());
		for (;first != last; ++first)
		{
			std::vector<uint32>	selectedIndex;

			const CLogStorage::TLogParamId &lpi = first->first;
			const std::vector<TSelectedParam> &sps = first->second;

			// retrieve the table for this param
			CLogStorage::TParamsTables::const_iterator it(logs._ParamTables.find(lpi));
			if (it != logs._ParamTables.end())
			{
				const CLogStorage::TParamsTable &pt = it->second;
				
				// parse each entry of the param table
				for (uint j=0; j<pt.size(); ++j)
				{
					// apply the operator
					
					if (op(pt[j], _RefValue))
					{
						// the operator returned true, add this log to the set of
						// matching logs
						selectedIndex.push_back(j);
					}
				}
				
				// now, look back for the logs that use the selected parameter
				buildSelectedLogList(selectedIndex, logs, sps, ret);
			}
		}

		return ret;
	}

	/// Evaluate a date against the predicate (only test the date predicate)
	virtual TTimeLine evalDate()
	{
		// type predicate cannot work on date, consider returning a complete timeline!
		TTimeLine tl;
		tl.push_back(FullTimeSlice);
		return tl;
	}


	virtual void dumpNode(NLMISC::CLog &log, const std::string &tab) const
	{
		log.displayNL("%s%s", tab.c_str(), typeid(this).name());
		log.displayNL("%s  ParamType : %s, refValue : %s", tab.c_str(), _ParamType.toString().c_str(), NLMISC::toString(_RefValue).c_str());

		TSelectedParams::const_iterator first(_SelectedParams.begin()), last(_SelectedParams.end());
		for (; first != last; ++first)
		{
			for (uint i=0; i<first->second.size(); ++i)
			{
				const LGS::TLogDefinition &logDef = _LogDefs[first->second[i].LogDefIndex];
				log.displayNL("%s  Matching in log '%s', parameter %u", tab.c_str(), logDef.getLogName().c_str(), first->second[i].ParamIndex);
			}

		}
	}
};

/// The predicate node. This node apply the operator with the reference value and the matching log paramters name
template <class Operator>
struct TPredicateNode : public TQueryNode
{
	/// The reference value used to compute the predicate
	LGS::TParamValue	_RefValue;
	/// Name of the	parameter
	std::string			_ParameterName;
	
	/// The log info vector
	const TLogDefinitions &_LogDefs;


	/// The list of parameter that match the name/type
	TSelectedParams		_SelectedParams;


	TPredicateNode(const LGS::TParamValue &refValue, const std::string &paramName, const TLogDefinitions &logDefs )
		:	_RefValue(refValue),
			_ParameterName(paramName),
			_LogDefs(logDefs)
	{
	}

	bool init()
	{
		if (_ParameterName == "LogDate")
		{
			if (!ConversionTable[_RefValue.getType().getValue()][LGS::TSupportedParamType::spt_uint32])
				throw EIncompatibleType(_RefValue.getType().getValue(), LGS::TSupportedParamType::spt_uint32);
			// any log is applicable
			CLogStorage::TLogParamId lpi;
			lpi.ParamName = "LogDate";
			lpi.ParamType = LGS::TSupportedParamType::invalid_val;
//				_SelectedParams.insert(std::make_pair(logDef.getLogName(), logDef.getParams().size()));
			// Create a fake entry in the selected param container
			_SelectedParams[lpi];
		}
		else if (_ParameterName == "LogName")
		{
			if (!ConversionTable[_RefValue.getType().getValue()][LGS::TSupportedParamType::spt_string])
				throw EIncompatibleType(_RefValue.getType().getValue(), LGS::TSupportedParamType::spt_string);
			// any log is applicable
			CLogStorage::TLogParamId lpi;
			lpi.ParamName = "LogName";
			lpi.ParamType = LGS::TSupportedParamType::invalid_val;
			// Create a fake entry in the selected param container
			_SelectedParams[lpi];
		}
		else if (_ParameterName == "ShardId")
		{
			if (!ConversionTable[_RefValue.getType().getValue()][LGS::TSupportedParamType::spt_uint32]
				&&!ConversionTable[_RefValue.getType().getValue()][LGS::TSupportedParamType::spt_string])
				throw EIncompatibleType(_RefValue.getType().getValue(), LGS::TSupportedParamType::spt_uint32);
			// any log is applicable
			CLogStorage::TLogParamId lpi;
			lpi.ParamName = "ShardId";
			lpi.ParamType = LGS::TSupportedParamType::invalid_val;
			// Create a fake entry in the selected param container
			_SelectedParams[lpi];
		}
		else
		{
			// Build a list of compatible parameters
			for (uint i=0; i<_LogDefs.size(); ++i)
			{
				const LGS::TLogDefinition &logDef = _LogDefs[i];
				
				{
					// read each param
					for (uint j=0; j<logDef.getParams().size(); ++j)
					{
						const LGS::TParamDesc &paramDesc = logDef.getParams()[j];
						
						// check the parameter type and conversion validity
						if (paramDesc.getName() == _ParameterName && ConversionTable[_RefValue.getType().asIndex()][paramDesc.getType().asIndex()])
						{
							CLogStorage::TLogParamId lpi;
							lpi.ParamName = paramDesc.getName();
							lpi.ParamType = paramDesc.getType();
							TSelectedParam sp;
							sp.LogDefIndex = i;
							sp.ListParam = false;
							sp.ParamIndex = j;
							
							// select this one
							_SelectedParams[lpi].push_back(sp);
						}
					}
					// read each list param
					for (uint j=0; j<logDef.getListParams().size(); ++j)
					{
						const LGS::TParamDesc &paramDesc = logDef.getListParams()[j];

						// check the parameter type and conversion validity
						if (paramDesc.getName() == _ParameterName && ConversionTable[_RefValue.getType().asIndex()][paramDesc.getType().asIndex()])
						{
							CLogStorage::TLogParamId lpi;
							lpi.ParamName = paramDesc.getName();
							lpi.ParamType = paramDesc.getType();
							TSelectedParam	sp;
							sp.LogDefIndex = i;
							sp.ListParam = true;
							sp.ParamIndex = j;

							// select this one
							_SelectedParams[lpi].push_back(sp);
						}
					}
				}
			}
		}
		return !_SelectedParams.empty();
	}

	/// Evaluate a date against the predicate (only test the date predicate)
	virtual TTimeLine evalDate()
	{
		if (_SelectedParams.size() == 1 && _SelectedParams.begin()->first.ParamType == LGS::TSupportedParamType::invalid_val)
		{
			if(_ParameterName == "LogDate")
			{
				TTimeSlice ts;
				TTimeLine tl;
				switch (Operator::getOperatorType())
				{
				case tt_LESS:
					ts.StartDate = 0;
					ts.EndDate = _RefValue.get_uint32();
					tl.push_back(ts);
					break;
				case tt_LESS_EQUAL:
					ts.StartDate = 0;
					ts.EndDate = _RefValue.get_uint32()+1;
					tl.push_back(ts);
					break;
				case tt_GREATER:
					ts.StartDate = _RefValue.get_uint32()+1;
					ts.EndDate = ~0;
					tl.push_back(ts);
					break;
				case tt_GREATER_EQUAL:
					ts.StartDate = _RefValue.get_uint32();
					ts.EndDate = ~0;
					tl.push_back(ts);
					break;
				case tt_EQUAL:
					ts.StartDate = _RefValue.get_uint32();
					ts.EndDate = _RefValue.get_uint32()+1;
					tl.push_back(ts);
					break;
				case tt_NOT_EQUAL:
					// need to push 2 slice
					ts.StartDate = 0;
					ts.EndDate = _RefValue.get_uint32();
					tl.push_back(ts);
					ts.StartDate = _RefValue.get_uint32()+1;
					ts.EndDate = ~0;
					tl.push_back(ts);
					break;
				default:
					break;
				}

				return tl;
			}
		}
		
		// otherwise, return full time range
		return TTimeLine(1, FullTimeSlice);
	}


	TLogEntries evalNode(const CLogStorage &logs)
	{
		// apply the operator on all the log entry

		Operator op;
		TLogEntries ret;

		if (_SelectedParams.size() == 1 && _SelectedParams.begin()->first.ParamType == LGS::TSupportedParamType::invalid_val)
		{
			if(_ParameterName == "LogDate")
			{
				LGS::TParamValue ref = convertParam(_RefValue, LGS::TSupportedParamType::spt_uint32);

				// this is a date comparison
				for (uint i=0; i<logs._DiskLogEntries.size(); ++i)
				{
					const CLogStorage::TDiskLogEntry &dle = logs._DiskLogEntries[i];
					if (op(dle.LogDate, _RefValue.get_uint32()))
						ret.insert(i);
				}
			}
			else if (_ParameterName == "LogName")
			{
				std::vector<uint32>	matchingLogs;
				LGS::TParamValue ref = convertParam(_RefValue, LGS::TSupportedParamType::spt_string);
				// select only log of a given log name (aka type)
				
				for (uint32 defIndex=0; defIndex<_LogDefs.size(); ++defIndex)
				{
					if (op(LGS::TParamValue(_LogDefs[defIndex].getLogName()), ref))
						matchingLogs.push_back(defIndex);
				}

				for (uint32 j=0; j<matchingLogs.size(); ++j)
				{
					uint32 logType = matchingLogs[j];
					for (uint i=0; i<logs._DiskLogEntries.size(); ++i)
					{
						if (logs._DiskLogEntries[i].LogType == logType)
							ret.insert(i);
					}
				}
			}
			else if (_ParameterName == "ShardId")
			{
				std::vector<uint32>	matchingLogs;
				LGS::TParamValue ref;

				// if we have a string, we interpret it as a shard name
				if (_RefValue.getType() == LGS::TSupportedParamType::spt_string)
					ref = LGS::TParamValue(CShardNames::getInstance().getShardId(_RefValue.get_string()));
				else
					ref = convertParam(_RefValue, LGS::TSupportedParamType::spt_uint32);

				// select only log of the correct shard id
				for (uint i=0; i<logs._DiskLogEntries.size(); ++i)
				{
					if (op(logs._DiskLogEntries[i].ShardId, ref.get_uint32()))
						ret.insert(i);
				}
			}
			else
				nlstop;
		}
		else
		{
			// parse each param table, then look back in the logs for the selected entry
			TSelectedParams::iterator first(_SelectedParams.begin()), last(_SelectedParams.end());
			for (;first != last; ++first)
			{
				std::vector<uint32>	selectedIndex;
				
				const CLogStorage::TLogParamId &lpi = first->first;
				const std::vector<TSelectedParam> &sps = first->second;
				
				// retrieve the table for this param
				CLogStorage::TParamsTables::const_iterator it(logs._ParamTables.find(lpi));
				if (it != logs._ParamTables.end())
				{
					const CLogStorage::TParamsTable &pt = it->second;
					
					LGS::TParamValue ref;
					if (Operator::getOperatorType() == tt_like)
					{
						// special case, keep the reference type
						ref = _RefValue;
					}
					else
					{
						ref = convertParam(_RefValue, lpi.ParamType);
					}
					
					// parse each entry of the param table
					for (uint j=0; j<pt.size(); ++j)
					{
						// apply the operator
						if (op(pt[j], ref))
						{
							// the operator returned true, add this log to the set of
							// matching logs
							selectedIndex.push_back(j);
						}
					}
					
					// now, look back for the logs that use the selected parameter
					buildSelectedLogList(selectedIndex, logs, sps, ret);
				}
			}
		}
		return ret;
	}

	virtual void dumpNode(NLMISC::CLog &log, const std::string &tab) const
	{
		log.displayNL("%s%s", tab.c_str(), typeid(this).name());
		log.displayNL("%s  ParamName : %s, refValue : %s", tab.c_str(), _ParameterName.c_str(), NLMISC::toString(_RefValue).c_str());

		TSelectedParams::const_iterator first(_SelectedParams.begin()), last(_SelectedParams.end());
		for (; first != last; ++first)
		{
			for (uint i=0; i<first->second.size(); ++i)
			{
				const LGS::TLogDefinition &logDef = _LogDefs[first->second[i].LogDefIndex];
				log.displayNL("%s  Matching in log '%s', parameter %u", tab.c_str(), logDef.getLogName().c_str(), first->second[i].ParamIndex);
			}
		}
	}
};

/// The operators classes
/// NB : all operator are implemented by using the '==' and '<' operator
struct TEqualOp
{
	template <class T>
	bool operator () (const T &value, const T &ref) const
	{
		return value == ref;
	}

	static TTokenType getOperatorType()	{ return tt_EQUAL;};
};

struct TLessOp
{
	template <class T>
	bool operator () (const T &value, const T &ref) const
	{
		return value < ref;
	}
	static TTokenType getOperatorType()	{ return tt_LESS;};
};

struct TLessEqualOp
{
	template <class T>
	bool operator () (const T &value, const T &ref) const
	{
		return value == ref || value < ref;
	}
	static TTokenType getOperatorType()	{ return tt_LESS_EQUAL;};
};

struct TGreaterOp
{
	template <class T>
	bool operator () (const T &value, const T &ref) const
	{
		return !(value < ref || value == ref);
	}
	static TTokenType getOperatorType()	{ return tt_GREATER;};
};

struct TGreaterEqualOp
{
	template <class T>
	bool operator () (const T &value, const T &ref) const
	{
		return !(value < ref) ;
	}
	static TTokenType getOperatorType()	{ return tt_GREATER_EQUAL;};
};

struct TNotEqualOp
{
	template <class T>
	bool operator () (const T &value, const T &ref) const
	{
		return !(value == ref);
	}
	static TTokenType getOperatorType()	{ return tt_NOT_EQUAL;};
};

struct TLikeOp
{
	template <class T>
	bool operator () (const T &value, const T &ref) const
	{
		std::string refStr = NLMISC::toString(ref);
		std::string valueStr = NLMISC::toString(value);

		return valueStr.find(refStr) != std::string::npos;
	}
	static TTokenType getOperatorType()	{ return tt_like;};
};

struct TOrCombiner
{
	TLogEntries operator () (const TLogEntries &leftEntry, const TLogEntries &rightEntry) const
	{
		TLogEntries ret;
		std::set_union(leftEntry.begin(), leftEntry.end(), 
			rightEntry.begin(), rightEntry.end(), 
			std::inserter(ret, ret.begin()));

		return ret;
	}

	TTimeLine operator ()(const TTimeLine &left, const TTimeLine &right) const
	{
		TTimeLine ret;

		std::map<uint32, sint32> timeTags;

		// first loop to init all entry needed to zero
		for (uint i=0; i<left.size(); ++i)
		{
			timeTags[left[i].StartDate] =0;
			timeTags[left[i].EndDate] =0;
		}
		for (uint i=0; i<right.size(); ++i)
		{
			timeTags[right[i].StartDate] =0;
			timeTags[right[i].EndDate] =0;
		}

		// second loop to inc/dec the start and end date
		for (uint i=0; i<left.size(); ++i)
		{
			timeTags[left[i].StartDate] +=1;
			timeTags[left[i].EndDate] -=1;
		}
		for (uint i=0; i<right.size(); ++i)
		{
			timeTags[right[i].StartDate] +=1;
			timeTags[right[i].EndDate] -=1;
		}


		sint32 count = 0;
		TTimeSlice ts;
		std::map<uint32, sint32>::iterator first(timeTags.begin()), last(timeTags.end());
		for (; first != last; ++first)
		{
			if (count == 0 && first->second > 0)
				ts.StartDate = first->first;
			else if (count > 0 && count+first->second == 0)
			{
				ts.EndDate = first->first;
				ret.push_back(ts);
			}

			count += first->second;
		}
		return ret;
	}
};

struct TAndCombiner
{
	TLogEntries operator () (const TLogEntries &leftEntry, const TLogEntries &rightEntry) const
	{
		TLogEntries ret;
		std::set_intersection(leftEntry.begin(), leftEntry.end(), 
			rightEntry.begin(), rightEntry.end(), 
			std::inserter(ret, ret.begin()));
		return ret;
	}

	TTimeLine operator ()(const TTimeLine &left, const TTimeLine &right) const
	{
		TTimeLine ret;

		std::map<uint32, sint32> timeTags;

		// first loop to init all entry needed to zero
		for (uint i=0; i<left.size(); ++i)
		{
			timeTags[left[i].StartDate] =0;
			timeTags[left[i].EndDate] =0;
		}
		for (uint i=0; i<right.size(); ++i)
		{
			timeTags[right[i].StartDate] =0;
			timeTags[right[i].EndDate] =0;
		}

		// second loop to inc/dec the start and end date
		for (uint i=0; i<left.size(); ++i)
		{
			timeTags[left[i].StartDate] +=1;
			timeTags[left[i].EndDate] -=1;
		}
		for (uint i=0; i<right.size(); ++i)
		{
			timeTags[right[i].StartDate] +=1;
			timeTags[right[i].EndDate] -=1;
		}

		sint32 count = 0;
		TTimeSlice ts;
		std::map<uint32, sint32>::iterator first(timeTags.begin()), last(timeTags.end());
		for (; first != last; ++first)
		{
			if (count < 2 && count+first->second == 2)
				ts.StartDate = first->first;
			else if (count == 2 && first->second < 0)
			{
				ts.EndDate = first->first;
				ret.push_back(ts);
			}

			count += first->second;
		}
		return ret;
	}
};

/** This class implement the query parser.
 *	It take a query string in input and return the root of a tree that implement
 *	the request.
 */
class CQueryParser
{
	/// Iterator in the parser are string iterator
	typedef std::string::const_iterator iterator;

	/// Struct to store a token returned by the lexer
	struct TToken
	{
		/// The position that generated this token
		iterator	It;
		/// The token type
		TTokenType	TokenType;
		/// The text value of the token
		std::string Text;
	};

	typedef	std::map<std::string, TTokenType> TKeywords;
	/// A set a predefined keyword. If a token found by the parser is equal to one 
	/// of the keyword, then the lexer substitute the token type with the
	/// one associated with the keyword
	TKeywords	_Keywords;

	/// The log definition used to build the query tree (needed to build the list
	/// of matching log types in the predicate nodes)
	const TLogDefinitions &_LogDefs;

public:
	/// Exception class thrown by the lexer/parser in case or error
	struct EInvalidQuery : public NLMISC::Exception 
	{
		/// The iterator in the string where the error is detected
		iterator	It;
		/// The error string
		const char		*ErrorStr;
		EInvalidQuery(iterator it, const char *erroStr)
			:	It(it),
				ErrorStr(erroStr)
		{}

		virtual const char* what() const throw ()
		{
			return ErrorStr;
		}

	};

	struct TParserResult
	{
		/// The query tree
		mutable std::auto_ptr<TQueryNode> QueryTree;

		/// Option to extract full context with selected logs
		bool		FullContext;

		/// Option to add a prefix to the output file
		std::string	OutputPrefix;

		TParserResult()
			:	FullContext(false)
		{
		}

		TParserResult(const TParserResult &other)
			:	QueryTree(other.QueryTree),
				FullContext(other.FullContext),
				OutputPrefix(other.OutputPrefix)

		{
		}
	};


	/// Constructor
	CQueryParser(const TLogDefinitions &logDefs);
	
	/// Parse the queryStr and return the root of the query tree
	TParserResult parseQuery(const std::string &queryStr, bool parseOnlyOption);
	
private:
	/// Convert a string into a supported param type (if possible)
	LGS::TSupportedParamType parseParamType(const std::string &typeName);

	/// Parse the options at the start of the query
	void parseOptions(CQueryParser::TParserResult &parserResult, CQueryParser::iterator &it, CQueryParser::iterator end);
	/// Parse one option
	bool parseOption(CQueryParser::TParserResult &parserResult, CQueryParser::iterator &it, CQueryParser::iterator end);


	/// Build a predicate node according to the specified operator, reference value and left value type.
	TQueryNode *buildPredicate(const LGS::TParamValue &refVal, const TToken &operatorType, const TToken &leftValue, iterator &it, iterator end);
	/// Parse a singed integer
	sint32 parseSint(iterator &it, iterator end);
	/// Parse an item id
	std::string parseItemId(iterator &it, iterator end);
	/// Parse an entity id
	std::string parseEntityId(iterator &it, iterator end);
	/// Parse a constant value
	LGS::TParamValue parseConstant(iterator &it, iterator end);
	/// Parse a predicate
	TQueryNode *parsePredicate(iterator &it, iterator end);
	/// Parse an atom (either a parenthesis grouping or an atom)
	TQueryNode *parseAtom(iterator &it, iterator end);
	/// Parse an 'and' expression
	TQueryNode *parseAndExpr(iterator &it, iterator end);
	/// Parse an expression
	TQueryNode *parseExpr(iterator &it, iterator end);
	/// Parse a date
	bool parseDATE(iterator &it, iterator end);
	/// parse an ID
	bool parseID(iterator &it, iterator end);
	/// parse an STRING
	bool parseSTRING(iterator &it, iterator end);
	/// parse an FLOAT
	bool parseFLOAT(iterator &it, iterator end);
	/// Parse a long int (64 bits)
	bool parseLONG_INT(iterator &it, iterator end);
	/// Parse a naked hexa (ie. without the '0x' prefix)
	bool parseNAKED_HEXA(iterator &it, iterator end);
	/// parse an INT
	bool parseINT(iterator &it, iterator end);
	/// The lexer
	TToken getNextToken(iterator &it, iterator end);
	/// Skip the WhiteSpace 
	iterator skipWS(iterator it, iterator end);

	/// Check is a string contains a naked hexadecimal value
	bool isNakedHexa(const std::string &text);

};


#endif //LOG_QUERY_H
