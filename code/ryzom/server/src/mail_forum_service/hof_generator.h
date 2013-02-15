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



#ifndef RY_HOF_GENERATOR_H
#define RY_HOF_GENERATOR_H

#include "shard_stat_db_manager.h"


/**
 * The Hall of Fame generator
 * It parses all the 'HoF Data Template' files (.hdt)
 * and generates data in PHP scripts for all shards
 *
 * \author Sebastien 'kxu' Guignot
 * \author Nevrax France
 * \date 2005 July
 */
class CHoFGenerator
{
public:
	/// get the singleton instance
	static CHoFGenerator * getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CHoFGenerator;
		return _Instance;
	}

	/// called at each service frame
	void serviceUpdate();

private:
	typedef CShardStatDBReader::TPlayerValues	TPlayerValues;
	typedef CShardStatDBReader::TGuildValues	TGuildValues;

	enum TTableField
	{
		FieldPlayerName,
		FieldPlayerValue,
		FieldGuildName,
		FieldGuildValue,

		FieldUnknown,
		NbFields = FieldUnknown
	};

	enum TWildcardOp
	{
		OpSum,
		OpMax,

		OpUnknown,
		NbOps = OpUnknown
	};

	struct CValueVar
	{
		std::string	VarName;
		std::string	Path;
	};

	struct CTableVar
	{
		std::string	VarName;
		std::string	Path;
		TTableField	Field;
		uint32		MaxRows;
	};

	struct CWildcardValueVar
	{
		std::string	VarName;
		std::string	PathPattern;
		TWildcardOp	Op;
	};

	struct CWildcardTableVar
	{
		std::string	VarName;
		std::string	PathPattern;
		TTableField	Field;
		uint32		MaxRows;
		TWildcardOp	Op;
	};

	struct CParsedData
	{
		void clear()
		{
			ValueVars.clear();
			TableVars.clear();
			WValueVars.clear();
			WTableVars.clear();
		}

		bool isEmpty() const
		{
			return (ValueVars.empty() && WValueVars.empty()	&& TableVars.empty() && WTableVars.empty());
		}

		std::vector<CValueVar>			ValueVars;
		std::vector<CTableVar>			TableVars;
		std::vector<CWildcardValueVar>	WValueVars;
		std::vector<CWildcardTableVar>	WTableVars;
	};

	struct CTableRow
	{
		bool operator<(const CTableRow & tableRow) const
		{
			return (Value > tableRow.Value);
		}

		std::string		Name;
		sint32			Value;
	};

	struct CTable
	{
		void sort();

		std::vector<CTableRow>	PlayerTable;
		std::vector<CTableRow>	GuildTable;
	};

private:
	/// private ctor
	CHoFGenerator();

	/// process a generation step
	void processGenerationStep();

	/// parse a HDT file
	/// \return false if parsing failed
	bool parseHDTFile(const std::string & fileName, CParsedData & parsedData);

	/// split parsed data to decompose the PHP generation in several steps
	void splitParsedData(const CParsedData & parsedData, std::vector<CParsedData> & parsedDataVec);

	/// generate PHP script from parsed data
	/// \param parsedData : parsed data to generate script for
	/// \param statDBReader : the shard SDB reader to use
	/// \param phpScript : append the generated script to this string
	/// \return false if generation failed
	bool generatePHPScript(const CParsedData & parsedData, const CShardStatDBReader & statDBReader, std::string & phpScript);

	/// generate a PHP array from a SDB table
	void generatePHPArray(const std::string & varName, const CTable & table, TTableField tableField, uint32 maxRows, std::string & phpArray);

	/// get sorted tables from a path pattern
	/// \param statDBReader : shard SDB reader
	/// \param pathPattern : the path pattern
	/// \param wildcardTables : return the wildcard tables (ie one table per wildcard operator)
	bool getSortedWildcardTables(const CShardStatDBReader & statDBReader, const std::string & pathPattern, std::vector<CTable> & wildcardTables) const;

	/// load a table from SDB
	void loadTable(const CShardStatDBReader & statDBReader, const TPlayerValues & playerValues, const TGuildValues & guildValues, CTable & table) const;

	/// return the result of 'leftVal op rightVal'
	sint32 applyWildcardOp(TWildcardOp op, sint32 leftVal, sint32 rightVal) const;

	TTableField toTableField(const std::string & tableField) const;
	TWildcardOp toWildcardOp(const std::string & op) const;

	/// add a variable name in the given vector or return false if the name already exists
	bool addVarName(std::vector<std::string> & varNames, const std::string & varName) const;

	const std::string & getCurrentHDTFile();
	const CShardStatDBReader & getCurrentSDBReader();
	const CParsedData & getCurrentStep();

	void setNextHDTFile();
	void setNextSDBReader();
	void setNextStep();

private:
	/// the singleton instance
	static CHoFGenerator * _Instance;

	NLMISC::TTime	_LastUpdateTime;
	NLMISC::TTime	_LastDirUpdateTime;

	/// HDT files to parse
	std::vector<std::string>		_HDTFiles;
	uint32							_CurrentHDTFileIndex;

	/// shard SDB readers
	std::vector<CShardStatDBReader>	_SDBReaders;
	uint32							_CurrentSDBReaderIndex;

	/// parsed data decomposed in several generation steps
	std::vector<CParsedData>		_Steps;
	uint32							_CurrentStepIndex;

	/// generated PHP script
	std::string		_GeneratedScript;
};


#endif // RY_HOF_GENERATOR_H
