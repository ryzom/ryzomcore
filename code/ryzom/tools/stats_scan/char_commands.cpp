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

/**
 * todo:
 *	- charScanScript commands
 *		* path for script files in cfg
 *		- charScanScriptNew <description>
 *		- charScanScriptLoad <file_name>
 *		- charScanScriptSave <file_name>
 *		- charScanScriptListFiles
 *		- charScanScriptListLines
 *		- charScanScriptDeleteLine <line_number>
 *		- charScanScriptAddInputFiles <directory> <wildcards>
 *		- charScanScriptAddInfoExtractor <infoExtractorName> [<args>]
 *		- charScanScriptAddFilter <filterName> [<args>]
 *		- charScanScriptAddPreInclude <include_file_name>
 *		- charScanScriptAddPostInclude <include_file_name>
 *		- charScanScriptSetOutputDirectory <directoryName>
 *		- charScanScriptSetDescription <description>
 *		- charScanScriptRun [-s <file_name>] [-o <output_directory>]
 *		- addJobCharScanScript [-s <file_name>] [-o <output_directory>]
 *		- charScanScriptListOutputFiles [<output_directory>]
 *		- charScanScriptListOutputFileContents <file_name> [<output_directory>]
 *		- charScanScriptTestFileList
 *		- charScanScriptTestOutput <test_input_file_path>
 *		- charScanScriptHelp
 * - path for generated files (from cfg + override) => managed incrementally with directory numbering
 * - add incremental update systems
 *		- add tables with a column per day ()
 *		- add tables with a column per week (limited number of weeks)
 *		- add tables with a column per month (limited number of months)
 */

/*
bool CCharFilterFactory::beginScanJob(const std::vector<std::string>& files, const std::vector<std::string>& filters)
{
	bool noErrors= true;

	// create a new job
	NLMISC::CSmartPtr<CCharacterScanJob> job=new CCharacterScanJob;

	// look after the filters
	for (uint32 i=0;i<filters.size();++i)
	{
		// locate the info extractor corresponding to the given name
		uint32 j=0;
		for (j=0;j<_Filters.size();++j)
			if (filters[i]==_Filters[j]->getName())
				break;

		// if none found then warn and skip passed
		if(j>=_Filters.size())
		{
			nlwarning("Unknown info extractor: %s",filters[i].c_str());
			noErrors=false;
			continue;
		}

		// add the info extractor to the new job
		job->addFilter(_Filters[j]);
	}

	// look after the files
	job->addFiles(files);

	// add the job to the job manager
	if (noErrors==true)
		CJobManager::getInstance()->addJob(&*job);

	return noErrors;
}

*/
//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/algo.h"
#include "game_share/persistent_data.h"

#include "character.h"
#include "job_manager.h"
#include "char_info_extractor_factory.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

CVariable<string>	SourceDirectory("variables", "SourceDirectory", "Directory we scan for files", string("."), 0, true);


//-----------------------------------------------------------------------------
// Commands - misc character related
//-----------------------------------------------------------------------------

NLMISC_COMMAND(listCharNames,"display the names of the characters int he listed save files","<input file name>")
{
	if (args.size()!=1 && args.size()!=0)
		return false;

	std::string wildcard="pdr_account*";
	if (args.size()==1)
		wildcard=args[0];

	std::vector<std::string> files;
	NLMISC::CPath::getPathContent(SourceDirectory.get().c_str(),false,false,true,files);
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

NLMISC_COMMAND(listCharInfoExtractors,"display the list of info extractors that exist for characters","")
{
	if (args.size()!=0)
		return false;

	CCharInfoExtractorFactory::getInstance()->displayInfoExtractorList(&log);

	return true;
}

//-----------------------------------------------------------------------------
// Commands - jobsManager
//-----------------------------------------------------------------------------

NLMISC_COMMAND(jobsPause,"pause execution of jobs","")
{
	if (args.size()!=0)
		return false;

	CJobManager::getInstance()->pause();

	return true;
}

NLMISC_COMMAND(jobsResume,"resume execution of jobs","")
{
	if (args.size()!=0)
		return false;

	CJobManager::getInstance()->resume();

	return true;
}

NLMISC_COMMAND(jobsPromote,"pause execution of jobs","<jobId>")
{
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

NLMISC_COMMAND(JobUpdatesPerUpdate,"set or display the number of job updates per service update","[<count>]")
{
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

NLMISC_COMMAND(jobsStatus,"display the status of the job manager","")
{
	if (args.size()!=0)
		return false;

	log.displayNL("%s",CJobManager::getInstance()->getStatus().c_str());

	return true;
}

NLMISC_COMMAND(jobsList,"display the list of unfinished jobs","")
{
	if (args.size()!=0)
		return false;

	CJobManager::getInstance()->listJobs(&log);

	return true;
}

NLMISC_COMMAND(jobsListAll,"display the list of all jobs (unfinished jobs are marked with a '*')","")
{
	if (args.size()!=0)
		return false;

	CJobManager::getInstance()->listJobHistory(&log);

	return true;
}

NLMISC_COMMAND(jobsDisplayDetails,"display detailed info for the current job (or a given job)","[<job id>]")
{
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
// Commands - charScanScript
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
