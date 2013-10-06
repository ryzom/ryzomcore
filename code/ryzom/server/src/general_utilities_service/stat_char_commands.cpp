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

#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/algo.h"
#include "game_share/persistent_data.h"

#include "stat_character.h"
#include "stat_job_manager.h"
#include "stat_char_info_extractor_factory.h"
#include "stat_char_scan_script.h"
#include "stat_globals.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const char* TmpScriptFileName= "__tmp_char_scan_script_file__";
const char* TmpOutputDirectoryName= "__tmp_char_scan_output_directory__";


//-----------------------------------------------------------------------------
// Commands - daily update commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(Stats,testDailyScript,"start the daily stats system","")
{
	if (args.size()!=0)
		return false;

	// create a new script object and add the tmp file
	CSmartPtr<CCharScanScriptFile> theCharScanScript= new CCharScanScriptFile;

	theCharScanScript->setOutputPath("../stats/");
	theCharScanScript->setOutputName("daily_test_");
	theCharScanScript->addInfoExtractor("Name");
	theCharScanScript->addInfoExtractor("Race");
	theCharScanScript->addInfoExtractor("Cult");
	theCharScanScript->addInfoExtractor("Position");
	theCharScanScript->addInfoExtractor("HighestSkills");
	theCharScanScript->addInfoExtractor("Money");
	theCharScanScript->addInfoExtractor("Sessions");
	theCharScanScript->addInfoExtractor("ActivityDuration");
	theCharScanScript->addInfoExtractor("PlayTime");
	theCharScanScript->addInfoExtractor("FirstActivityDate");
	theCharScanScript->addInfoExtractor("FirstActivityTime");
	theCharScanScript->addInfoExtractor("LastActivityDate");
	theCharScanScript->addInfoExtractor("LastActivityTime");
	theCharScanScript->addInfoExtractor("RecentPlayHistory");
	theCharScanScript->addFileListBuilder("AddFilesRecurse account_*_pdr.bin");
	theCharScanScript->addFileListBuilder("NewestFile");
	theCharScanScript->addFileListBuilder("Since 01/02/2007");
	theCharScanScript->addFileListBuilder("FileAge 14");

// active accounts
//	theCharScanScript->addFileListBuilder("AddFilesRecurse account_*_pdr.bin");
//	theCharScanScript->addFileListBuilder("FileAge 0 14");

// lost accounts
//	theCharScanScript->addFileListBuilder("AddFilesRecurse account_*_pdr.bin");
//	theCharScanScript->addFileListBuilder("NewestFile");
//	theCharScanScript->addFileListBuilder("Since 01/02/2007");
//	theCharScanScript->addFileListBuilder("FileAge 14");

	// create and launch the new job
	CSmartPtr<CCharacterScanJob> theJob= new CCharacterScanJob;
	bool noErrors= theCharScanScript->applyToJob(*theJob);
	if (noErrors)
		CJobManager::getInstance()->addJob(&*theJob);

	return true;
}


NLMISC_CATEGORISED_COMMAND(Stats,startDailyStats,"start the daily stats system","")
{
	if (args.size()!=0)
		return false;

//	todo();

	return true;
}


//-----------------------------------------------------------------------------
// Commands - misc character related
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(Stats,listCharNames,"display the names of the characters in the listed save files","[<input file specs>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1 && args.size()!=0)
		return false;

	std::string wildcard="account*_pdr.bin";
	if (args.size()==1)
		wildcard=args[0];

	std::vector<std::string> files;
	NLMISC::CPath::getPathContent(STAT_GLOBALS::getInputFilePath().c_str(),false,false,true,files);
	for (uint32 i=(uint32)files.size();i--;)
	{
		if (!NLMISC::testWildCard(NLMISC::CFile::getFilename(files[i]),wildcard))
		{
			files[i]=files.back();
			files.pop_back();
		}
	}
	std::sort(files.begin(),files.end());
	for (uint32 i=0;i<files.size();++i)
	{
		static CPersistentDataRecord	pdr;
		pdr.clear();
		pdr.readFromFile(files[i].c_str());

		CStatsScanCharacter c;
		c.apply(pdr);

		log.displayNL("%-40s Name: %s ",files[i].c_str(),c.EntityBase._Name.c_str());
	}

	return true;
}

//-----------------------------------------------------------------------------
// Commands - charScanner
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(Stats,listCharFilters,"display the list of filters that exist for characters","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CCharFilterFactory::getInstance()->displayFilterList(&log);

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,listCharInfoExtractors,"display the list of info extractors that exist for characters","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CCharInfoExtractorFactory::getInstance()->displayInfoExtractorList(&log);

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,listInputFileRules,"display the list of input file rules that exist for characters","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CFileListBuilderFactory::getInstance()->displayFileListBuilderList(&log);

	return true;
}

//-----------------------------------------------------------------------------
// Commands - jobsManager
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(Stats,jobsPause,"pause execution of jobs","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CJobManager::getInstance()->pause();

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,jobsResume,"resume execution of jobs","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CJobManager::getInstance()->resume();

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,jobsPromote,"pause execution of jobs","<jobId>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	uint32 idx;
	NLMISC::fromString(args[0], idx);
	if ( (idx==0 && args[0]!="0") )
	{
		nlwarning("Argument is not a valid job id - should be a number");
		return false;
	}

	CJobManager::getInstance()->promoteJob(idx);
	CJobManager::getInstance()->listJobs(&log);

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,JobUpdatesPerUpdate,"set or display the number of job updates per service update","[<count>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()>1)
		return false;

	if (args.size()==1)
	{
		uint32 count;
		NLMISC::fromString(args[0], count);
		if ( (count==0 && args[0]!="0") )
		{
			nlwarning("Argument is not a valid number");
			return false;
		}
		CJobManager::getInstance()->setJobUpdatesPerUpdate(count);
	}

	nlinfo("JobUpdatesPerUpdate %d",CJobManager::getInstance()->getJobUpdatesPerUpdate());

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,jobsStatus,"display the status of the job manager","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	log.displayNL("%s",CJobManager::getInstance()->getStatus().c_str());

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,jobsList,"display the list of unfinished jobs","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CJobManager::getInstance()->listJobs(&log);

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,jobsListAll,"display the list of all jobs (unfinished jobs are marked with a '*')","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	CJobManager::getInstance()->listJobHistory(&log);

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,jobsDisplayDetails,"display detailed info for the current job (or a given job)","[<job id>]")
{
	CNLSmartLogOverride logOverride(&log);

	switch (args.size())
	{
	case 0:
		CJobManager::getInstance()->displayCurrentJob(&log);
		break;

	case 1:
		{
			uint32 idx;
			NLMISC::fromString(args[0], idx);
			if ( (idx==0 && args[0]!="0") )
			{
				nlwarning("Argument is not a valid job id - should be a number");
				return false;
			}
			CJobManager::getInstance()->displayJob(idx,&log);
		}
		break;


	default:
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Global Variables for charScanScript Commands
//-----------------------------------------------------------------------------

static CSmartPtr<CCharScanScriptFile> TheCharScanScriptFile;


//-----------------------------------------------------------------------------
// Handy utility methods for charScanScript Commands
//-----------------------------------------------------------------------------

static std::string getActiveOutputPath()
{
	// extract the output directory name from the currently loaded script
	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no active script file right now from which to extract output directory");
		return "";
	}
	bool isOK=true;

	// write the current script file to a tmp file
	isOK=TheCharScanScriptFile->writeToFile(TmpScriptFileName);
	if (!isOK) return "";

	// create a new script object and assign the tmp file to it
	CCharScanScript script;
	script.addScriptFile(TmpScriptFileName);

	// create a char scan job for the script object
	CCharacterScanJob job;
	isOK=script.applyToJob(job);

	// extract the output directory from the scan job
	return isOK? job.getOutputPath(): std::string();
}

static std::string getOutputPath(const std::string& directoryName)
{
	// setup a temp script file with an output directory in order to manage the output root path...
	CCharScanScriptFile scriptFile;
	scriptFile.setOutputPath(directoryName);

	// build a file list for the script's output path and display it
	return scriptFile.getOutputPath();
}


//-----------------------------------------------------------------------------
// Commands - charScanScript
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptNew,"Create a nw scan script - wipe the previous scan script from memory","<description>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	TheCharScanScriptFile= new CCharScanScriptFile;

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptLoad,"Load a scan script from a disk file","<file_name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	TheCharScanScriptFile= new CCharScanScriptFile;
	TheCharScanScriptFile->parseFile(args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptSave,"Save the current scan script to a disk file","<file_name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded - nothing to save");
		return false;
	}

	TheCharScanScriptFile->writeToFile(args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptListFiles,"List the scan script files available","")
{
	CNLSmartLogOverride logOverride(&log);

	switch (args.size())
	{
	case 0:
		{
			CFileDescriptionContainer fdc;
			fdc.addFileSpec(STAT_GLOBALS::getScriptFilePath("*.css"));
			for (uint32 i=0;i<fdc.size();++i)
			{
				log.displayNL("%s",NLMISC::CFile::getFilenameWithoutExtension(fdc[i].FileName).c_str());
			}
		}
		return true;
	}

	return false;
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptListLines,"","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded - nothing to display");
		return false;
	}

	TheCharScanScriptFile->display();

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptDeleteLine,"","<line_number>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded");
		return false;
	}

	uint32 line;
	NLMISC::fromString(args[0], line);

	// line numbering starts at 1 so an invalid number will be inored anyway
	return TheCharScanScriptFile->deleteLine(line);
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptAddInfoExtractor,"","<infoExtractorName> [<args>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()<1)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded");
		return false;
	}

	CVectorSString argVector;
	argVector = reinterpret_cast<const vector<CSString> &>(args);
	CSString cmdLine;
	cmdLine.join(argVector,' ');

	return TheCharScanScriptFile->addInfoExtractor(cmdLine);
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptAddFilter,"","<filterName> [<args>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()<1)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded");
		return false;
	}

	CVectorSString argVector;
	argVector = reinterpret_cast<const vector<CSString> &>(args);
	CSString cmdLine;
	cmdLine.join(argVector,' ');

	return TheCharScanScriptFile->addFilter(cmdLine);
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptAddInputFileRule,"","<ruleName> [<args>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()<1)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded");
		return false;
	}

	CVectorSString argVector;
	argVector = reinterpret_cast<const vector<CSString> &>(args);
	CSString cmdLine;
	cmdLine.join(argVector,' ');

	return TheCharScanScriptFile->addFileListBuilder(cmdLine);
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptAddInclude,"","<include_file_name>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded");
		return false;
	}

	return TheCharScanScriptFile->addInclude(args[0]);
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptSetOutputDirectory,"","<directoryName>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded");
		return false;
	}

	return TheCharScanScriptFile->setOutputPath(args[0]);
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptRun,"","[<output_directory>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()>1)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded");
		return false;
	}

	// deal with output path overriding
	CSString oldOutputPath= TheCharScanScriptFile->getOutputPath();
	if (args.size()==1)
		TheCharScanScriptFile->setOutputPath(args[0]);

	// save the active file to a temp file
	TheCharScanScriptFile->writeToFile(TmpScriptFileName);

	// restore the output path to previous value
	TheCharScanScriptFile->setOutputPath(oldOutputPath);

	// create a new script object and add the tmp file
	CSmartPtr<CCharScanScript> theCharScanScript= new CCharScanScript;
	theCharScanScript->addScriptFile(TmpScriptFileName);

	// create and launch the new job
	CSmartPtr<CCharacterScanJob> theJob= new CCharacterScanJob;
	bool noErrors= theCharScanScript->applyToJob(*theJob);
	if (noErrors)
		CJobManager::getInstance()->addJob(&*theJob);

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,addJobCharScanScript,"","<file_name> [<output_directory>]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()>2)
		return false;

	CSString fileName;
	if (args.size()==1)
	{
		fileName= args[0];
	}
	else
	{
		// create a temp script to refference the script we want to run and override the output path
		CCharScanScriptFile scriptFile;
		scriptFile.setOutputPath(args[1]);
		scriptFile.addInclude(args[0]);
		fileName= TmpScriptFileName;
		scriptFile.writeToFile(fileName);
	}

	// create a new script object and add the file
	CSmartPtr<CCharScanScript> theCharScanScript= new CCharScanScript;
	theCharScanScript->addScriptFile(fileName);

	// create and launch the new job
	CSmartPtr<CCharacterScanJob> theJob= new CCharacterScanJob;
	bool noErrors= theCharScanScript->applyToJob(*theJob);
	if (noErrors)
		CJobManager::getInstance()->addJob(&*theJob);

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptListOutputFiles,"list the files in the output file directory generated by a charScan run","[<output_directory>]")
{
	CNLSmartLogOverride logOverride(&log);

	std::string outputPath;

	switch (args.size())
	{
	case 0:
		outputPath= getActiveOutputPath();
		break;

	case 1:
		outputPath= getOutputPath(args[0]);
		break;

	default:
		return false;
	}

	// build a file list for the job's output path and display it
	nlinfo("File list for directory: %s",outputPath.c_str());
	CFileDescriptionContainer fdc;
	fdc.addFileSpec(outputPath+"*");
	fdc.display(&log);

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptListOutputFileContents,"list the contents of a given output file","<file_name> [<output_directory>]")
{
	CNLSmartLogOverride logOverride(&log);

	std::string outputPath;

	switch (args.size())
	{
	case 1:
		outputPath= getActiveOutputPath();
		break;

	case 2:
		outputPath= getOutputPath(args[1]);
		break;

	default:
		return false;
	}

	// setup the full file name for the output file in question
	std::string fileName= outputPath+args[0];
	if (!NLMISC::CFile::fileExists(fileName))
	{
		nlwarning("File not found: %s",fileName.c_str());
		return false;
	}

	// load and display the file contents
	NLMISC::CSString s;
	s.readFromFile(fileName);
	NLMISC::CVectorSString lines;
	s.splitLines(lines);
	for (uint32 i=0;i<lines.size();++i)
		log.displayNL("%s",lines[i].c_str());

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptTestFileList,"list the set of files that will be treated in a charScan run (before filtering)","")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded");
		return false;
	}

	// save the active script file to a temp file
	TheCharScanScriptFile->writeToFile(TmpScriptFileName);

	// create a new script object and add the tmp file
	CSmartPtr<CCharScanScript> theCharScanScript= new CCharScanScript;
	theCharScanScript->addScriptFile(TmpScriptFileName);

	// create the new job
	CSmartPtr<CCharacterScanJob> theJob= new CCharacterScanJob;
	bool noErrors= theCharScanScript->applyToJob(*theJob);
	if (noErrors)
	{
		// have the job generate it's file list...
		nlinfo("File list for current script:");
		CFileDescriptionContainer fdc;
		theJob->getFileList(fdc);
		fdc.display(&log);
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptTestFilteredFileList,"list the set of files that will be treated in a charScan run (after application of filters)","generate|display")
{
	CNLSmartLogOverride logOverride(&log);

	static CRefPtr<CCharacterScanJob> jobInProgress;
	static CFileDescriptionContainer fdc;

	if (args.size()!=1)
		return false;

	// generating a new file list
	if (args[0]==CSString("generate").left((uint)args[0].size()))
	{
		if (TheCharScanScriptFile==NULL)
		{
			nlwarning("There is no scan script currently loaded");
			return false;
		}

		if (jobInProgress!=NULL)
		{
			nlwarning("A job is already running: %s",CJobManager::getInstance()->getStatus().c_str());
			return true;
		}

		// save the active script file to a temp file
		TheCharScanScriptFile->writeToFile(TmpScriptFileName);

		// create a new script object and add the tmp file
		CSmartPtr<CCharScanScript> theCharScanScript= new CCharScanScript;
		theCharScanScript->addScriptFile(TmpScriptFileName);

		// create and launch the new job
		CSmartPtr<CCharacterScanJob> theJob= new CCharacterScanJob;
		bool noErrors= theCharScanScript->applyToJob(*theJob);
		if (noErrors)
		{
			fdc.clear();
			jobInProgress= theJob;
			theJob->listFilesOnly(fdc);
			CJobManager::getInstance()->addJob(&*theJob);
		}
		return true;
	}

	// displaying the last generated file list
	if (args[0]==CSString("display").left((uint)args[0].size()))
	{
		nlinfo("Filtered file list for the current job");
		fdc.display(&log);
		if (jobInProgress!=NULL)
			nlinfo("... work still in progress ...");
		return true;
	}

	return false;
}

NLMISC_CATEGORISED_COMMAND(Stats,charScanScriptTestOutput,"run the current scan script for a single given input file and display the contents of the generated output files","<input_file_path>")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	if (TheCharScanScriptFile==NULL)
	{
		nlwarning("There is no scan script currently loaded");
		return false;
	}

	// deal with output path overriding
	CSString oldOutputPath= TheCharScanScriptFile->getOutputPath();
	TheCharScanScriptFile->setOutputPath(TmpOutputDirectoryName);

	// save the active file to a temp file
	TheCharScanScriptFile->writeToFile(TmpScriptFileName);

	// restore the output path to previous value
	TheCharScanScriptFile->setOutputPath(oldOutputPath);

	// create a new script object and add the tmp file
	CSmartPtr<CCharScanScript> theCharScanScript= new CCharScanScript;
	theCharScanScript->addScriptFile(TmpScriptFileName);

	// create the new job and execute for the file in question
	CSmartPtr<CCharacterScanJob> theJob= new CCharacterScanJob;
	bool noErrors= theCharScanScript->applyToJob(*theJob);
	if (noErrors)
	{
		theJob->deleteFilesInOutputDirectory();
		theJob->runForFile(args[0]);
	}

	// display the result
	std::string outputPath= getOutputPath(TmpOutputDirectoryName);
	CFileDescriptionContainer fdc;
	fdc.addFileSpec(outputPath+"*");
	for (uint32 f=0;f<fdc.size();++f)
	{
		nlinfo("");
		nlinfo("FILE: %s:",fdc[f].FileName.c_str());
		NLMISC::CSString s;
		s.readFromFile(fdc[f].FileName);
		NLMISC::CVectorSString lines;
		s.splitLines(lines);
		for (uint32 i=0;i<lines.size();++i)
			log.displayNL("%s",lines[i].c_str());
	}

	return true;
}


//-----------------------------------------------------------------------------
