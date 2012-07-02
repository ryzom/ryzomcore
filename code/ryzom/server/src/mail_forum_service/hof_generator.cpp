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

#include "stdpch.h"
#include "hof_generator.h"

#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/config_file.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

extern CVariable<std::string>	WebRootDirectory;

CVariable<bool>			HoFEnableGenerator("hof", "HoFEnableGenerator", "Set if HoF generator is enabled", true, 0, true);
CVariable<bool>			HoFVerbose("hof", "HoFVerbose", "Set if HoF generator is verbose", false, 0, true);
CVariable<std::string>	HoFHDTDirectory("hof", "HoFHDTDirectory", "Directory where HDT files are stored", "/local/www/hof", 0, true);
CVariable<uint32>		HoFGeneratorUpdatePeriod("hof", "HoFGeneratorUpdatePeriod", "HoF generator update period in milliseconds", 1000, 0, true);
CVariable<uint32>		HoFGeneratorDirUpdatePeriod("hof", "HoFGeneratorDirUpdatePeriod", "HoF generator directory update period in seconds", 60, 0, true);


CHoFGenerator * CHoFGenerator::_Instance = NULL;

// ****************************************************************************
// Helpers
// ****************************************************************************

// ****************************************************************************
static string createSingleQuotedPHPString(const string & s)
{
	string res;
	res += "'";
	for (uint i = 0; i < s.size(); i++)
	{
		if (s[i] == '\'')
		{
			res += "\\";
		}
		res += s[i];
	}
	res += "'";

	return res;
}

// ****************************************************************************
// CHoFGenerator
// ****************************************************************************

// ****************************************************************************
CHoFGenerator::CHoFGenerator()
{
	_LastUpdateTime			= 0;
	_LastDirUpdateTime		= 0;
	_CurrentHDTFileIndex	= 0;
	_CurrentSDBReaderIndex	= 0;
	_CurrentStepIndex		= 0;
}

// ****************************************************************************
void CHoFGenerator::serviceUpdate()
{
	if (!HoFEnableGenerator.get())
		return;

	TTime now = CTime::getLocalTime();

	if (now - _LastDirUpdateTime >= HoFGeneratorDirUpdatePeriod.get() * 1000)
	{
		// update the directory content
		// ie rebuild the list of HDT files

		_LastDirUpdateTime = now;
		_HDTFiles.clear();

		string dir = CPath::standardizePath(HoFHDTDirectory.get());
		if (CFile::isExists(dir))
		{
			std::vector<std::string> files;
			CPath::getPathContent(dir, true, false, true, files);
			for (uint i = 0; i < files.size(); i++)
			{
				const string & fileName = files[i];

				if (toLower(CFile::getExtension(fileName)) == "hdt")
				{
					_HDTFiles.push_back(fileName);
				}
			}
		}

		if (_CurrentHDTFileIndex >= _HDTFiles.size())
		{
			_CurrentHDTFileIndex	= 0;
			_CurrentSDBReaderIndex	= 0;
			_CurrentStepIndex		= 0;
			_GeneratedScript.clear();
		}
	}

	if (now - _LastUpdateTime >= HoFGeneratorUpdatePeriod.get())
	{
		_LastUpdateTime = now;
		processGenerationStep();
	}
}

// ****************************************************************************
const std::string & CHoFGenerator::getCurrentHDTFile()
{
	nlassert(_CurrentHDTFileIndex < _HDTFiles.size());
	return _HDTFiles[_CurrentHDTFileIndex];
}

// ****************************************************************************
const CShardStatDBReader & CHoFGenerator::getCurrentSDBReader()
{
	nlassert(_CurrentSDBReaderIndex < _SDBReaders.size());
	return _SDBReaders[_CurrentSDBReaderIndex];
}

// ****************************************************************************
const CHoFGenerator::CParsedData & CHoFGenerator::getCurrentStep()
{
	nlassert(_CurrentStepIndex < _Steps.size());
	return _Steps[_CurrentStepIndex];
}

// ****************************************************************************
void CHoFGenerator::setNextHDTFile()
{
	// clear the generated script
	_GeneratedScript.clear();

	// reset step index
	_CurrentStepIndex = 0;

	// reset the SDB reader index
	_CurrentSDBReaderIndex = 0;

	_CurrentHDTFileIndex++;
	if (_CurrentHDTFileIndex >= _HDTFiles.size())
		_CurrentHDTFileIndex = 0;
}

// ****************************************************************************
void CHoFGenerator::setNextSDBReader()
{
	// clear the generated script
	_GeneratedScript.clear();

	// reset step index
	_CurrentStepIndex = 0;

	_CurrentSDBReaderIndex++;
	if (_CurrentSDBReaderIndex >= _SDBReaders.size())
	{
		// set the next HDT file
		setNextHDTFile();
	}
}

// ****************************************************************************
void CHoFGenerator::setNextStep()
{
	_CurrentStepIndex++;
	if (_CurrentStepIndex >= _Steps.size())
	{
		H_AUTO(CHoFGenerator_setNextStep_writePHPFile);

		// build the PHP file name
		string filePath = WebRootDirectory.get() + toString("/%u/", getCurrentSDBReader().getShardId());
		string baseName = CFile::getFilenameWithoutExtension(getCurrentHDTFile());
		string phpFileName = filePath + baseName + ".php";

		// create the directory if it does not exist
		if (!CFile::isExists(filePath))
			CFile::createDirectoryTree(filePath);

		// write the PHP file
		COFile ofile;
		if (ofile.open(phpFileName))
		{
			static const string phpHeader = "<?php\n";
			static const string phpFooter = "?>\n";
			
			ofile.serialBuffer((uint8*)(&phpHeader[0]), (uint)phpHeader.size());
			ofile.serialBuffer((uint8*)(&_GeneratedScript[0]), (uint)_GeneratedScript.size());
			ofile.serialBuffer((uint8*)(&phpFooter[0]), (uint)phpFooter.size());

			if (HoFVerbose.get())
			{
				nlinfo("script file '%s' has been successfully generated from '%s' for shard %u",
					phpFileName.c_str(),
					getCurrentHDTFile().c_str(),
					getCurrentSDBReader().getShardId()
					);
			}
		}
		else
		{
			nlwarning("cannot open file '%s' for writing", phpFileName.c_str());
		}

		// set the next SDB reader
		setNextSDBReader();
	}
}

// ****************************************************************************
void CHoFGenerator::processGenerationStep()
{
	H_AUTO(CHoFGenerator_processGenerationStep);

	if (_HDTFiles.empty())
		return;

	// get shard SDB readers
	CShardStatDBManager::getInstance()->getShardStatDBReaders(_SDBReaders);
	if (_SDBReaders.empty())
		return;

	// if we begin a new generation loop (a HDT file will generate PHP scripts for all shards)
	if (_CurrentSDBReaderIndex == 0 && _CurrentStepIndex == 0)
	{
		// parse the HDT file
		CParsedData parsedData;
		if (!parseHDTFile(getCurrentHDTFile(), parsedData))
		{
			nlwarning("parsing the file '%s' failed (see error above)", getCurrentHDTFile().c_str());

			// skip invalid HDT file
			setNextHDTFile();
			return;
		}

		// split parsed data in generation steps
		splitParsedData(parsedData, _Steps);
		if (_Steps.empty())
		{
			if (HoFVerbose.get())
				nlinfo("'%s' is empty, no script will be generated from it", getCurrentHDTFile().c_str());

			// skip empty HDT file
			setNextHDTFile();
			return;
		}
	}

	// generate a PHP script part
	if (!generatePHPScript(getCurrentStep(), getCurrentSDBReader(), _GeneratedScript))
	{
		if (HoFVerbose.get())
			nlwarning("script generation from '%s' failed for shard %u", getCurrentHDTFile().c_str(), getCurrentSDBReader().getShardId());

		// skip the SDB reader if an error occured
		setNextSDBReader();
		return;
	}

	// set the next step
	setNextStep();
}

// ****************************************************************************
bool CHoFGenerator::parseHDTFile(const std::string & fileName, CParsedData & parsedData)
{
	H_AUTO(CHoFGenerator_parseHDTFile);

	parsedData.clear();

	CConfigFile cfgFile;
	try
	{
		H_AUTO(CHoFGenerator_parseHDTFile_1);
		cfgFile.load(fileName);
	}
	catch (const Exception & e)
	{
		nlwarning("cannot load file '%s' : %s", fileName.c_str(), e.what());
		return false;
	}

	CConfigFile::CVar *	cfgVar;
	vector<string>		varNames;

	H_BEFORE(CHoFGenerator_parseHDTFile_2);

	// parse Values
	cfgVar = cfgFile.getVarPtr("Values");
	if (cfgVar != NULL)
	{
		const sint nbParams = 2;
		if (cfgVar->size() % nbParams != 0)
		{
			nlwarning("invalid number of entries in 'Values'");
			return false;
		}

		uint i = 0;
		while (i+nbParams <= cfgVar->size())
		{
			CValueVar valueVar;
			valueVar.VarName = cfgVar->asString(i);
			if (!addVarName(varNames, valueVar.VarName))
			{
				nlwarning("variable '%s' already exists", valueVar.VarName.c_str());
				return false;
			}

			valueVar.Path = cfgVar->asString(i+1);

			parsedData.ValueVars.push_back(valueVar);
			i += nbParams;
		}
	}

	// parse WildcardValues
	cfgVar = cfgFile.getVarPtr("WildcardValues");
	if (cfgVar != NULL)
	{
		const sint nbParams = 3;
		if (cfgVar->size() % nbParams != 0)
		{
			nlwarning("invalid number of entries in 'WildcardValues'");
			return false;
		}

		uint i = 0;
		while (i+nbParams <= cfgVar->size())
		{
			CWildcardValueVar wValueVar;
			wValueVar.VarName = cfgVar->asString(i);
			if (!addVarName(varNames, wValueVar.VarName))
			{
				nlwarning("variable '%s' already exists", wValueVar.VarName.c_str());
				return false;
			}

			wValueVar.PathPattern = cfgVar->asString(i+1);

			string sOp = cfgVar->asString(i+2);
			wValueVar.Op = toWildcardOp(sOp);
			if (wValueVar.Op == OpUnknown)
			{
				nlwarning("op '%s' is unknown", sOp.c_str());
				return false;
			}

			parsedData.WValueVars.push_back(wValueVar);
			i += nbParams;
		}
	}

	// parse Tables
	cfgVar = cfgFile.getVarPtr("Tables");
	if (cfgVar != NULL)
	{
		const sint nbParams = 4;
		if (cfgVar->size() % nbParams != 0)
		{
			nlwarning("invalid number of entries in 'Tables'");
			return false;
		}

		uint i = 0;
		while (i+nbParams <= cfgVar->size())
		{
			CTableVar tableVar;
			tableVar.VarName = cfgVar->asString(i);
			if (!addVarName(varNames, tableVar.VarName))
			{
				nlwarning("variable '%s' already exists", tableVar.VarName.c_str());
				return false;
			}

			tableVar.Path = cfgVar->asString(i+1);

			string sField = cfgVar->asString(i+2);
			tableVar.Field = toTableField(sField);

			if (tableVar.Field == FieldUnknown)
			{
				nlwarning("table field '%s' is unknown", sField.c_str());
				return false;
			}

			string sMaxRows = cfgVar->asString(i+3);
			if (sMaxRows == "*")
				tableVar.MaxRows = 0xffffFFFF;
			else
				NLMISC::fromString(sMaxRows, tableVar.MaxRows);

			parsedData.TableVars.push_back(tableVar);
			i += nbParams;
		}
	}

	// parse WildcardTables
	cfgVar = cfgFile.getVarPtr("WildcardTables");
	if (cfgVar != NULL)
	{
		const sint nbParams = 5;
		if (cfgVar->size() % nbParams != 0)
		{
			nlwarning("invalid number of entries in 'WildcardTables'");
			return false;
		}

		uint i = 0;
		while (i+nbParams <= cfgVar->size())
		{
			CWildcardTableVar wTableVar;
			wTableVar.VarName = cfgVar->asString(i);
			if (!addVarName(varNames, wTableVar.VarName))
			{
				nlwarning("variable '%s' already exists", wTableVar.VarName.c_str());
				return false;
			}

			wTableVar.PathPattern = cfgVar->asString(i+1);

			string sField = cfgVar->asString(i+2);
			wTableVar.Field = toTableField(sField);

			if (wTableVar.Field == FieldUnknown)
			{
				nlwarning("table field '%s' is unknown", sField.c_str());
				return false;
			}

			string sMaxRows = cfgVar->asString(i+3);
			if (sMaxRows == "*")
				wTableVar.MaxRows = 0xffffFFFF;
			else
				NLMISC::fromString(sMaxRows, wTableVar.MaxRows);

			string sOp = cfgVar->asString(i+4);
			wTableVar.Op = toWildcardOp(sOp);
			if (wTableVar.Op == OpUnknown)
			{
				nlwarning("op '%s' is unknown", sOp.c_str());
				return false;
			}

			parsedData.WTableVars.push_back(wTableVar);
			i += nbParams;
		}
	}

	if (parsedData.isEmpty())
	{
		nlwarning("HDT file '%s' is empty", fileName.c_str());
		return false;
	}

	H_AFTER(CHoFGenerator_parseHDTFile_2);

	return true;
}

// ****************************************************************************
void CHoFGenerator::splitParsedData(const CParsedData & parsedData, std::vector<CParsedData> & parsedDataVec)
{
	// clear the vector
	parsedDataVec.clear();

	// 1 step for all value vars
	{
		CParsedData parsedValues;
		parsedValues.ValueVars = parsedData.ValueVars;
		if (!parsedValues.isEmpty())
			parsedDataVec.push_back(parsedValues);
	}

	// 1 step for all wildcard value vars
	{
		CParsedData parsedWValues;
		parsedWValues.WValueVars = parsedData.WValueVars;
		if (!parsedWValues.isEmpty())
			parsedDataVec.push_back(parsedWValues);
	}

	// 1 step per table (table vars with the same path)
	{
		typedef map<std::string, std::vector<CTableVar> > TTableVarsByPath;
		TTableVarsByPath tableVarsByPath;
		for (uint i = 0; i < parsedData.TableVars.size(); i++)
		{
			const CTableVar & tableVar = parsedData.TableVars[i];
			tableVarsByPath[tableVar.Path].push_back(tableVar);
		}

		for (TTableVarsByPath::const_iterator it = tableVarsByPath.begin(); it != tableVarsByPath.end(); ++it)
		{
			CParsedData parsedTablesWithSamePath;
			parsedTablesWithSamePath.TableVars = (*it).second;
			parsedDataVec.push_back(parsedTablesWithSamePath);
		}
	}

	// 1 step per wildcard table (wildcard table vars with the same path pattern)
	{
		typedef map<std::string, std::vector<CWildcardTableVar> > TWTableVarsByPathPattern;
		TWTableVarsByPathPattern wTableVarsByPathPattern;
		for (uint i = 0; i < parsedData.WTableVars.size(); i++)
		{
			const CWildcardTableVar & wTableVar = parsedData.WTableVars[i];
			wTableVarsByPathPattern[wTableVar.PathPattern].push_back(wTableVar);
		}

		for (TWTableVarsByPathPattern::const_iterator it = wTableVarsByPathPattern.begin(); it != wTableVarsByPathPattern.end(); ++it)
		{
			CParsedData parsedWTablesWithSamePathPattern;
			parsedWTablesWithSamePathPattern.WTableVars = (*it).second;
			parsedDataVec.push_back(parsedWTablesWithSamePathPattern);
		}
	}
}

// ****************************************************************************
bool CHoFGenerator::generatePHPScript(const CParsedData & parsedData, const CShardStatDBReader & statDBReader, std::string & phpScript)
{
	H_AUTO(CHoFGenerator_generatePHPScript);

	if (parsedData.isEmpty())
		return true;

	map<std::string, CTable> sortedTablePerPath;
	map<std::string, vector<CTable> > sortedWildcardTablesPerPathPattern;

	// get sorted tables
	for (uint i = 0; i < parsedData.TableVars.size(); i++)
	{
		H_AUTO(CHoFGenerator_generatePHPScript_1);

		const string & path = parsedData.TableVars[i].Path;
		if (sortedTablePerPath.find(path) != sortedTablePerPath.end())
			continue;

		CTable & table = sortedTablePerPath[path];

		const TPlayerValues * playerValues;
		const TGuildValues * guildValues;
		if (!statDBReader.getTable(path, playerValues, guildValues))
		{
			if (HoFVerbose.get())
				nlwarning("cannot get table at '%s' for shard %u", path.c_str(), statDBReader.getShardId());
			return false;
		}
		loadTable(statDBReader, *playerValues, *guildValues, table);
		table.sort();
	}

	// get sorted wildcard tables
	for (uint i = 0; i < parsedData.WTableVars.size(); i++)
	{
		H_AUTO(CHoFGenerator_generatePHPScript_2);

		const string & pathPattern = parsedData.WTableVars[i].PathPattern;
		if (sortedWildcardTablesPerPathPattern.find(pathPattern) != sortedWildcardTablesPerPathPattern.end())
			continue;

		vector<CTable> & wildcardTables = sortedWildcardTablesPerPathPattern[pathPattern];

		if (!getSortedWildcardTables(statDBReader, pathPattern, wildcardTables))
		{
			if (HoFVerbose.get())
				nlwarning("cannot get tables at '%s' for shard %u", pathPattern.c_str(), statDBReader.getShardId());
			return false;
		}
		nlassert(wildcardTables.size() == NbOps);
	}

	H_BEFORE(CHoFGenerator_generatePHPScript_3);

	// generate PHP script

	if (!parsedData.ValueVars.empty())
	{
		for (uint i = 0; i < parsedData.ValueVars.size(); i++)
		{
			const CValueVar & valueVar = parsedData.ValueVars[i];

			sint32 val;
			if (!statDBReader.getValue(valueVar.Path, val))
			{
				if (HoFVerbose.get())
					nlwarning("cannot get value at '%s' for shard %u", valueVar.Path.c_str(), statDBReader.getShardId());
				return false;
			}

			phpScript += toString("$%s = %d;\n", valueVar.VarName.c_str(), val);
		}
	}

	if (!parsedData.WValueVars.empty())
	{
		for (uint i = 0; i < parsedData.WValueVars.size(); i++)
		{
			const CWildcardValueVar & wValueVar = parsedData.WValueVars[i];

			vector<sint32> wildcardValues;
			if (!statDBReader.getValues(wValueVar.PathPattern, wildcardValues))
			{
				if (HoFVerbose.get())
					nlwarning("cannot get values at '%s' for shard %u", wValueVar.PathPattern.c_str(), statDBReader.getShardId());
				return false;
			}

			nlassert(!wildcardValues.empty());
			sint32 val = wildcardValues[0];
			for (uint k = 1; k < wildcardValues.size(); k++)
			{
				val = applyWildcardOp(wValueVar.Op, val, wildcardValues[k]);
			}

			phpScript += toString("$%s = %d;\n", wValueVar.VarName.c_str(), val);
		}
	}

	if (!parsedData.TableVars.empty())
	{
		for (uint i = 0; i < parsedData.TableVars.size(); i++)
		{
			const CTableVar & tableVar = parsedData.TableVars[i];

			const CTable & table = sortedTablePerPath[tableVar.Path];
			
			string phpArray;
			generatePHPArray(tableVar.VarName, table, tableVar.Field, tableVar.MaxRows, phpArray);
			phpScript += phpArray + "\n";
		}
	}

	if (!parsedData.WTableVars.empty())
	{
		for (uint i = 0; i < parsedData.WTableVars.size(); i++)
		{
			const CWildcardTableVar & wTableVar = parsedData.WTableVars[i];

			nlassert(wTableVar.Op >= 0 && wTableVar.Op < NbOps);
			const CTable & table = sortedWildcardTablesPerPathPattern[wTableVar.PathPattern][wTableVar.Op];

			string phpArray;
			generatePHPArray(wTableVar.VarName, table, wTableVar.Field, wTableVar.MaxRows, phpArray);
			phpScript += phpArray + "\n";
		}
	}

	H_AFTER(CHoFGenerator_generatePHPScript_3);

	return true;
}

// ****************************************************************************
void CHoFGenerator::generatePHPArray(const std::string & varName, const CTable & table, TTableField tableField, uint32 maxRows, std::string & phpArray)
{
	H_AUTO(CHoFGenerator_generatePHPArray);

	const vector<CTableRow> * tableRows = NULL;
	bool nameField;

	if (tableField == FieldPlayerName)
	{
		tableRows = &table.PlayerTable;
		nameField = true;
	}
	else if (tableField == FieldPlayerValue)
	{
		tableRows = &table.PlayerTable;
		nameField = false;
	}
	else if (tableField == FieldGuildName)
	{
		tableRows = &table.GuildTable;
		nameField = true;
	}
	else if (tableField == FieldGuildValue)
	{
		tableRows = &table.GuildTable;
		nameField = false;
	}
	else
	{
		nlstop;
		return;
	}

	phpArray.clear();
	phpArray += toString("$%s = array(", varName.c_str());

	uint32 nbRows = (uint32)tableRows->size();
	if (maxRows < nbRows)
		nbRows = maxRows;

	for (uint i = 0; i < nbRows; i++)
	{
		if (nameField)
		{
			phpArray += createSingleQuotedPHPString((*tableRows)[i].Name);
		}
		else
		{
			phpArray += toString((*tableRows)[i].Value);
		}

		if (i < nbRows-1)
		{
			phpArray += ",";
		}
	}

	phpArray += ");";
}

// ****************************************************************************
bool CHoFGenerator::getSortedWildcardTables(const CShardStatDBReader & statDBReader, const std::string & pathPattern, std::vector<CTable> & wildcardTables) const
{
	H_AUTO(CHoFGenerator_getSortedWildcardTables);

	TPlayerValues wildcardPlayerValues[NbOps];
	TGuildValues wildcardGuildValues[NbOps];

	vector<const TPlayerValues *> playerValuesVec;
	vector<const TGuildValues *> guildValuesVec;
	if (!statDBReader.getTables(pathPattern, playerValuesVec, guildValuesVec))
		return false;

	for (uint i = 0; i < playerValuesVec.size(); i++)
	{
		const TPlayerValues * playerValues = playerValuesVec[i];
		for (TPlayerValues::const_iterator it = playerValues->begin(); it != playerValues->end(); ++it)
		{
			for (sint op = 0; op < NbOps; op++)
			{
				TPlayerValues::iterator itW = wildcardPlayerValues[op].find((*it).first);
				if (itW == wildcardPlayerValues[op].end())
				{
					wildcardPlayerValues[op][(*it).first] = (*it).second;
				}
				else
				{
					(*itW).second = applyWildcardOp(TWildcardOp(op), (*itW).second, (*it).second);
				}
			}
		}
	}

	for (uint i = 0; i < guildValuesVec.size(); i++)
	{
		const TGuildValues * guildValues = guildValuesVec[i];
		for (TGuildValues::const_iterator it = guildValues->begin(); it != guildValues->end(); ++it)
		{
			for (sint op = 0; op < NbOps; op++)
			{
				TGuildValues::iterator itW = wildcardGuildValues[op].find((*it).first);
				if (itW == wildcardGuildValues[op].end())
				{
					wildcardGuildValues[op][(*it).first] = (*it).second;
				}
				else
				{
					(*itW).second = applyWildcardOp(TWildcardOp(op), (*itW).second, (*it).second);
				}
			}
		}
	}

	wildcardTables.clear();
	wildcardTables.resize(NbOps);
	for (sint op = 0; op < NbOps; op++)
	{
		loadTable(statDBReader, wildcardPlayerValues[op], wildcardGuildValues[op], wildcardTables[op]);
		wildcardTables[op].sort();
	}

	return true;
}

// ****************************************************************************
void CHoFGenerator::loadTable(const CShardStatDBReader & statDBReader, const TPlayerValues & playerValues, const TGuildValues & guildValues, CTable & table) const
{
	H_AUTO(CHoFGenerator_loadTable);

	table.PlayerTable.clear();
	table.GuildTable.clear();

	// get player values
	for (TPlayerValues::const_iterator it = playerValues.begin(); it != playerValues.end(); ++it)
	{
		const CEntityId & playerId = (*it).first;
		string playerName;
		if (!statDBReader.getPlayerName(playerId, playerName))
		{
			playerName = "Not found: " + playerId.toString();
		}

		CTableRow tableRow;
		tableRow.Name	= playerName;
		tableRow.Value	= (*it).second;

		table.PlayerTable.push_back(tableRow);
	}

	// get guild values
	for (TGuildValues::const_iterator it = guildValues.begin(); it != guildValues.end(); ++it)
	{
		const EGSPD::TGuildId & guildId = (*it).first;
		string guildName;
		if (!statDBReader.getGuildName(guildId, guildName))
		{
			guildName = toString("Not found: %u", guildId);
		}

		CTableRow tableRow;
		tableRow.Name	= guildName;
		tableRow.Value	= (*it).second;

		table.GuildTable.push_back(tableRow);
	}
}

// ****************************************************************************
sint32 CHoFGenerator::applyWildcardOp(TWildcardOp op, sint32 leftVal, sint32 rightVal) const
{
	sint32 res;
	if (op == OpMax)
	{
		if (leftVal > rightVal)
			res = leftVal;
		else
			res = rightVal;
	}
	else if (op == OpSum)
	{
		res = leftVal + rightVal;
	}
	else
	{
		res = 0;
		nlstop;
	}

	return res;
}

// ****************************************************************************
CHoFGenerator::TTableField CHoFGenerator::toTableField(const std::string & tableField) const
{
	TTableField res;

	if (tableField == "PlayerName")
		res = FieldPlayerName;
	else if (tableField == "PlayerValue")
		res = FieldPlayerValue;
	else if (tableField == "GuildName")
		res = FieldGuildName;
	else if (tableField == "GuildValue")
		res = FieldGuildValue;
	else
		res = FieldUnknown;

	return res;
}

// ****************************************************************************
CHoFGenerator::TWildcardOp CHoFGenerator::toWildcardOp(const std::string & op) const
{
	TWildcardOp res;

	if (op == "Sum")
		res = OpSum;
	else if (op == "Max")
		res = OpMax;
	else
		res = OpUnknown;

	return res;
}

// ****************************************************************************
bool CHoFGenerator::addVarName(std::vector<std::string> & varNames, const std::string & varName) const
{
	for (uint i = 0; i < varNames.size(); i++)
	{
		if (varNames[i] == varName)
			return false;
	}

	varNames.push_back(varName);
	return true;
}

// ****************************************************************************
// CHoFGenerator::CTable
// ****************************************************************************

// ****************************************************************************
void CHoFGenerator::CTable::sort()
{
	H_AUTO(CHoFGenerator_CTable_sort);

	std::sort(PlayerTable.begin(), PlayerTable.end());
	std::sort(GuildTable.begin(), GuildTable.end());
}

