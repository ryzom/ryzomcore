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
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/variable.h"
#include "nel/misc/path.h"
#include "game_share/file_description_container.h"
#include "game_share/utils.h"
#include "stat_char_scan_script.h"
#include "stat_globals.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// methods CCharScanScript
//-------------------------------------------------------------------------------------------------

CCharScanScript::CCharScanScript()
{
}

bool CCharScanScript::addScriptFile(const std::string& fileName)
{
	std::string fullFileName=STAT_GLOBALS::getScriptFilePath(fileName);

	// make sure the file exists
	if (!NLMISC::CFile::fileExists(fullFileName))
	{
		nlwarning("script file not found: %s",fullFileName.c_str());
		return false;
	}

	// make sure the file hasn't already been included previously
	for (uint32 i=0;i<_ScriptFiles.size();++i)
	{
		if (_ScriptFiles[i].getFileName()==fullFileName)
		{
			nlwarning("attempt to include script file '%s' more than once",fullFileName.c_str());
			return true;
		}
	}

	// add & parse the new file
	return vectAppend(_ScriptFiles).parseFile(fullFileName);
}

bool CCharScanScript::applyToJob(CCharacterScanJob& job)
{
	bool isOK=true;

	// iterate backwards over the script files in order to apply the most important files last
	for (uint32 i=_ScriptFiles.size();i--;)
	{
		isOK &= _ScriptFiles[i].applyToJob(job);
	}

	return isOK;
}

//-------------------------------------------------------------------------------------------------
// methods CCharScanScriptFile
//-------------------------------------------------------------------------------------------------

bool CCharScanScriptFile::parseFile(const std::string& fileName, CCharScanScript* container)
{
	_FileName= fileName;

	// read the content of the input file
	bool result;
	NLMISC::CSString fileContent;
	result=fileContent.readFromFile(fileName);
	if (result==false)
	{
		nlwarning("Failed to read script file: %s",fileName.c_str());
		return false;
	}

	// split the file into lines and execute them one by one
	NLMISC::CVectorSString lines;
	fileContent.splitLines(lines);
	for (uint32 i=0;i<lines.size();++i)
	{
		// strip comments and leading and trailing blanks
		CSString theLine= lines[i].replace("//","\xff").splitTo('\xff').strip();
		if (theLine.empty())
			continue;

		CCharScanScriptCommandRegistry::getInstance()->execute(*this,theLine,container);
	}
	return true;
}

bool CCharScanScriptFile::applyToJob(CCharacterScanJob& job)
{
	bool result=true;

	// apply the file list builders
	for (uint32 i=0;i<_FileListBuilders.size();++i)
	{
		IFileListBuilder* builder= CFileListBuilderFactory::getInstance()->build(_FileListBuilders[i]);
		if (builder==NULL)
		{
			nlwarning("Failed to build filter description from line: %s",_FileListBuilders[i].c_str());
			result=false;
			continue;
		}
		job.addFileListBuilder(builder);
	}

	// apply the filters
	for (uint32 i=0;i<_Filters.size();++i)
	{
		ICharFilter* filter= CCharFilterFactory::getInstance()->build(_Filters[i]);
		if (filter==NULL)
		{
			nlwarning("Failed to build filter description from line: %s",_Filters[i].c_str());
			result=false;
			continue;
		}
		job.addFilter(filter);
	}

	// apply the info extractors
	for (uint32 i=0;i<_InfoExtractors.size();++i)
	{
		ICharInfoExtractor* infoExtractor= CCharInfoExtractorFactory::getInstance()->build(_InfoExtractors[i]);
		if (infoExtractor==NULL)
		{
			nlwarning("Failed to build filter description from line: %s",_InfoExtractors[i].c_str());
			result=false;
			continue;
		}
		job.addInfoExtractor(infoExtractor);
	}

	// apply the output path
	job.setOutputPath(getOutputPath());
	job.setOutputName(_OutputName);

	return result;
}

const std::string& CCharScanScriptFile::getFileName() const
{
	return _FileName;
}

std::string CCharScanScriptFile::getOutputPath() const
{
	// use the directory 'tmp' if no explicit directory name is given
	std::string path=_OutputPath.empty()? "tmp": _OutputPath;

	// pre-pend the output directory root (from the CVariable variable) to the path
	if (path[0]!='/')
	{
		path= STAT_GLOBALS::getOutputFilePath(path);
	}

	// standardise the path
	return NLMISC::CPath::standardizePath(path);
}

bool CCharScanScriptFile::writeToFile(const std::string& fileName)
{
	CSString scriptTxt;
	_FileName=fileName;
	if (!_OutputPath.empty())	scriptTxt+="outputPath "+_OutputPath+"\n";
	for (uint32 i=0;i<_Filters.size();++i)			scriptTxt+="filter "+_Filters[i]+"\n";
	for (uint32 i=0;i<_InfoExtractors.size();++i)	scriptTxt+="infoExtractor "+_InfoExtractors[i]+"\n";
	for (uint32 i=0;i<_FileListBuilders.size();++i)	scriptTxt+="inputFileRule "+_FileListBuilders[i]+"\n";
	for (uint32 i=0;i<_Includes.size();++i)			scriptTxt+="include "+_Includes[i]+"\n";
	return scriptTxt.writeToFile(_FileName);
}

void CCharScanScriptFile::display() const
{
	uint32 lineCount=0;
	if (!_OutputPath.empty())	nlinfo("%3i %s",++lineCount,("outputPath "+_OutputPath).c_str());
	for (uint32 i=0;i<_Filters.size();++i)			nlinfo("%3i %s",++lineCount,("filter "+_Filters[i]).c_str());
	for (uint32 i=0;i<_InfoExtractors.size();++i)	nlinfo("%3i %s",++lineCount,("infoExtractor "+_InfoExtractors[i]).c_str());
	for (uint32 i=0;i<_FileListBuilders.size();++i)	nlinfo("%3i %s",++lineCount,("inputFileRule "+_FileListBuilders[i]).c_str());
	for (uint32 i=0;i<_Includes.size();++i)			nlinfo("%3i %s",++lineCount,("include "+_Includes[i]).c_str());
}

bool CCharScanScriptFile::deleteLine(uint32 idx)
{
	uint32 lineCount=0;
	if (!_OutputPath.empty())	{ if (++lineCount==idx) { _OutputPath.clear(); return true; } }
	for (uint32 i=0;i<_Filters.size();++i)			{ if (++lineCount==idx) { _Filters.erase(_Filters.begin()+i); return true; } }
	for (uint32 i=0;i<_InfoExtractors.size();++i)	{ if (++lineCount==idx) { _InfoExtractors.erase(_InfoExtractors.begin()+i); return true; } }
	for (uint32 i=0;i<_FileListBuilders.size();++i)	{ if (++lineCount==idx) { _FileListBuilders.erase(_FileListBuilders.begin()+i); return true; } }
	for (uint32 i=0;i<_Includes.size();++i)			{ if (++lineCount==idx) { _Includes.erase(_Includes.begin()+i);  return true; } }

	nlwarning("Delete failed because there are only %d lines in the file and not %d",lineCount,idx);
	return false;
}

bool CCharScanScriptFile::setOutputPath(const std::string& path)
{
	_OutputPath= path;
	return true;
}

bool CCharScanScriptFile::setOutputName(const std::string& fileNameRoot)
{
	_OutputName= fileNameRoot;
	return true;
}

bool CCharScanScriptFile::addFilter(const std::string& rawArgs)
{
	_Filters.push_back(rawArgs);
	return true;
}

bool CCharScanScriptFile::addInfoExtractor(const std::string& rawArgs)
{
	_InfoExtractors.push_back(rawArgs);
	return true;
}

bool CCharScanScriptFile::addFileListBuilder(const std::string& rawArgs)
{
	_FileListBuilders.push_back(rawArgs);
	return true;
}

bool CCharScanScriptFile::addInclude(const std::string& include)
{
	// ensure the include doesn't already exist
	for (uint32 i=0;i<_Includes.size();++i)
	{
		if (_Includes[i]==include)
		{
			nlwarning("Attempt to include the same file more than once - ignored: %s",include.c_str());
			return true;
		}
	}

	_Includes.push_back(include);
	return true;
}


//-------------------------------------------------------------------------------------------------
// methods CCharScanScriptCommandRegistry
//-------------------------------------------------------------------------------------------------

CCharScanScriptCommandRegistry* CCharScanScriptCommandRegistry::getInstance()
{
	static CCharScanScriptCommandRegistry* ptr=NULL;
	if (ptr==NULL)
		ptr= new CCharScanScriptCommandRegistry;
	return ptr;
}

void CCharScanScriptCommandRegistry::registerScriptCommand(NLMISC::CSmartPtr<ICharScanScriptCommand> scriptCommand)
{
	// ensure that we don't have a name conflict with an existing script command
	for (uint32 i=0;i<_ScriptCommands.size();++i)
	{
		nlassert(scriptCommand->getName()!=_ScriptCommands[i]->getName());
	}

	// add the new script command
	_ScriptCommands.push_back(scriptCommand);
}

void CCharScanScriptCommandRegistry::displayScriptCommands(NLMISC::CLog* log)
{
	uint32 longestName=4;

	// iterate over the script commands to determine the length of the longest name
	for (uint32 i=0;i<_ScriptCommands.size();++i)
	{
		std::string s= _ScriptCommands[i]->getName();
		if (s.size()>longestName)
			longestName=(uint32)s.size();
	}

	// iterate over the script commands displaying names and description
	for (uint32 i=0;i<_ScriptCommands.size();++i)
	{
		log->displayNL("%-*s  %s",longestName,_ScriptCommands[i]->getName(),_ScriptCommands[i]->getDescription());
	}
}

bool CCharScanScriptCommandRegistry::execute(CCharScanScriptFile& scriptFile,const CSString& commandLine,CCharScanScript* container)
{
	// split the command line into its constituent parts
	CSString theCommand= commandLine.firstWordConst();
	CSString theRawArgs= commandLine.tailFromFirstWord().strip();
	CVectorSString theArgs;
	theRawArgs.splitByOneOfSeparators(" \t",theArgs,false,false,false,false,true);

	// try to locate and execute the given command
	for (uint32 i=0;i<_ScriptCommands.size();++i)
	{
		if (theCommand==_ScriptCommands[i]->getName())
		{
			return _ScriptCommands[i]->execute(scriptFile,theArgs,theRawArgs,commandLine,container);
		}
	}

	// we failed to find the command so bomb
	nlwarning("Unknown script command '%s' in line: %s",theCommand.c_str(),commandLine.c_str());
	return false;
}


//-----------------------------------------------------------------------------
// CHAR_SCAN_SCRIPT_COMMAND: instances
//-----------------------------------------------------------------------------

CHAR_SCAN_SCRIPT_COMMAND(include,"<include_file_name>","Include another script file")
{
	if (args.size()!=1)
		return false;

	if (scriptFile.addInclude(rawArgs)==false)
		return false;

	if (container==NULL)
		return true;

	return container->addScriptFile(args[0]);
}

CHAR_SCAN_SCRIPT_COMMAND(inputFileRule,"<name> [<args>]","Add a rule relative to the files to be parsed")
{
	if (rawArgs.strip().empty())
		return false;

	return scriptFile.addFileListBuilder(rawArgs);
}

CHAR_SCAN_SCRIPT_COMMAND(filter,"<name> [<args>]","Add a filter to limit the set criteria to determine which files' content to reflect in output")
{
	if (rawArgs.strip().empty())
		return false;

	return scriptFile.addFilter(rawArgs);
}

CHAR_SCAN_SCRIPT_COMMAND(infoExtractor,"<name> [<args>]","Add an info extractor")
{
	if (rawArgs.strip().empty())
		return false;

	return scriptFile.addInfoExtractor(rawArgs);
}

CHAR_SCAN_SCRIPT_COMMAND(outputPath,"<path>","Set the directory to which the output will be written")
{
	if (args.size()!=1)
		return false;

	return scriptFile.setOutputPath(args[0]);
}

CHAR_SCAN_SCRIPT_COMMAND(outputName,"<fileNameRoot>","Set the file name root (default to empty string)")
{
	if (args.size()!=1)
		return false;

	return scriptFile.setOutputName(args[0]);
}


//-----------------------------------------------------------------------------
