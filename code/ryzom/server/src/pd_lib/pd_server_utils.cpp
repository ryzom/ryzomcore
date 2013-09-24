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

#include "pd_server_utils.h"
#include "pd_lib.h"

#include <nel/misc/path.h>
#include <nel/misc/debug.h>
#include <nel/net/service.h>

#include <nel/misc/i_xml.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/file.h>

#include <time.h>

#include "db_reference_file.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


/*
 * Load a Reference index file
 */
bool	CRefIndex::load(const string& filename)
{
	CIFile		reffile;
	CIXml		ixml;

	if (!reffile.open(filename) || !ixml.init(reffile))
		return false;

	try
	{
		serial(ixml);
	}
	catch (const Exception&)
	{
		return false;
	}

	return true;
}

/*
 * Load a Reference index file
 */
bool	CRefIndex::load()
{
	string	rootpath = getRootPath();
	if (rootpath.empty())
		return false;

	return load(rootpath+"ref");
}

/*
 * Save a Reference index file
 */
bool	CRefIndex::save(const string& filename)
{
	COFile		reffile;
	COXml		oxml;

	if (!reffile.open(filename) || !oxml.init(&reffile))
		return false;

	try
	{
		serial(oxml);
	}
	catch (const Exception&)
	{
		return false;
	}

	return true;
}

/*
 * Save a Reference index file
 */
bool	CRefIndex::save()
{
	string	path = getPath();
	if (path.empty())
		return false;

	return checkDirectory(path) && save(path+"ref");
}

/*
 * Set As Valid Reference
 */
bool	CRefIndex::setAsValidRef()
{
	string	rootpath = getRootPath();
	if (rootpath.empty())
		return false;

	return save(rootpath+"ref");
}


/*
 * Build next Reference index file
 */
bool	CRefIndex::buildNext()
{
	if (!load())
		Index = 0;
	else
		++Index;

	setup();

	if (!setupDirectory())
		return false;

	if (!save())
		return false;

	return true;
}

/*
 * Get next Reference index file
 */
void	CRefIndex::getNext()
{
	++Index;

	setup();
}



/*
 * Get (and setup if needed) database root path
 */
std::string	CRefIndex::getRootPath()
{
	string	path = getNominalRootPath();

	if (!CFile::isExists(path) || !CFile::isDirectory(path))
	{
		if (!CFile::createDirectoryTree(path))
		{
			nlwarning("getRootPath(): unable to create save path '%s'", path.c_str());
			return string("");
		}

		if (!CFile::setRWAccess(path))
		{
			nlwarning("getRootPath(): failure, can't set RW access to path '%s', can't start.", path.c_str());
			return string("");
		}
	}

	return path;
}


/*
 * Get Nominal Root Path
 */
std::string	CRefIndex::getNominalRootPath()
{
	return RY_PDS::CPDSLib::getRootDirectory(DatabaseId);
}


/*
 * Get reference path
 */
std::string	CRefIndex::getPath()
{
	return NLMISC::CPath::standardizePath(getRootPath() + Path);
}




/*
 * Setup reference directory
 */
bool	CRefIndex::setupDirectory()
{
	string	rootpath = getRootPath();
	if (rootpath.empty())
		return false;

	string	path = NLMISC::CPath::standardizePath(rootpath + Path);

	if (!checkDirectory(path))
	{
		nlwarning("CRefIndex::setupDirectory(): failed, can't check directory '%s'", path.c_str());
		return false;
	}

	if (!checkDirectory(getHoursUpdatePath()))
	{
		nlwarning("CRefIndex::setupDirectory(): failed, can't check subdirectory '%s'", getHoursUpdatePath().c_str());
		return false;
	}

	if (!checkDirectory(getMinutesUpdatePath()))
	{
		nlwarning("CRefIndex::setupDirectory(): failed, can't check subdirectory '%s'", getMinutesUpdatePath().c_str());
		return false;
	}

	if (!checkDirectory(getSecondsUpdatePath()))
	{
		nlwarning("CRefIndex::setupDirectory(): failed, can't check subdirectory '%s'", getSecondsUpdatePath().c_str());
		return false;
	}

	if (!checkDirectory(getLogPath()))
	{
		nlwarning("CRefIndex::setupDirectory(): failed, can't check subdirectory '%s'", getLogPath().c_str());
		return false;
	}

	return true;
}

/*
 * Check directory
 */
bool	CRefIndex::checkDirectory(const std::string& path)
{
	if (!CFile::isExists(path))
	{
		if (!CFile::createDirectoryTree(path))
		{
			nlwarning("CRefIndex::checkDirectory(): failed, can't create directory '%s'", path.c_str());
			return false;
		}

		if (!CFile::setRWAccess(path))
		{
			nlwarning("CRefIndex::checkDirectory(): failed, can't set RW access to directory '%s'", path.c_str());
			return false;
		}
	}
	else if (!CFile::isDirectory(path))
	{
		nlwarning("CRefIndex::checkDirectory(): failed, directory '%s' is already a file", path.c_str());
		return false;
	}

	return true;
}


/*
 * Set Time stamp
 */
void	CRefIndex::setTimestamp()
{
	Timestamp.setToCurrent();
}



/*
 * Get Seconds update path
 */
std::string	CRefIndex::getSecondsUpdatePath()
{
	return getRootPath() + "seconds/";
}

/*
 * Get Minutes update path
 */
std::string	CRefIndex::getMinutesUpdatePath()
{
	return getRootPath() + "minutes/";
}

/*
 * Get Hours update path
 */
std::string	CRefIndex::getHoursUpdatePath()
{
	return getRootPath() + "hours/";
}

/*
 * Get Log path
 */
std::string	CRefIndex::getLogPath()
{
	return getRootPath() + "logs/";
}




/*
 * Constructor
 */
CDatabaseState::CDatabaseState()
{
	Id = 0xffffffff;
	LastUpdateId = (0-1);
	CurrentIndex = 0;
}

/*
 * Serial method
 */
void	CDatabaseState::serial(NLMISC::IStream& s)
{
	s.xmlPush("database_state");

	s.serialCheck(NELID("DBST"));
	uint	version = s.serialVersion(0);

	s.xmlPush("name");
	s.serial(Name);
	s.xmlPop();

	s.xmlPush("id");
	s.serial(Id);
	s.xmlPop();

	s.xmlPush("lastupdateid");
	s.serial(LastUpdateId);
	s.xmlPop();

	s.xmlPush("currentindex");
	s.serial(CurrentIndex);
	s.xmlPop();

	s.xmlPush("endtimestamp");
	if (s.isReading())
	{
		std::string	ts;
		s.serial(ts);
		EndTimestamp.fromString(ts.c_str());
	}
	else
	{
		std::string	ts = EndTimestamp.toString();
		s.serial(ts);
	}
	s.xmlPop();

	s.xmlPop();
}

/*
 * Save State
 */
bool	CDatabaseState::save(CRefIndex& ref)
{
	COFile		f;
	COXml		oxml;

	string		filename = fileName(ref);

	if (!f.open(filename) || !oxml.init(&f))
		return false;

	try
	{
		serial(oxml);
	}
	catch (const Exception&)
	{
		return false;
	}

	return true;
}

/*
 * Load State
 */
bool	CDatabaseState::load(CRefIndex& ref, bool usePrevious)
{
	return load(ref.getRootPath(), usePrevious);
}


/*
 * Load State
 */
bool	CDatabaseState::load(const std::string& rootpath, bool usePrevious)
{
	CIFile		f;
	CIXml		ixml;

	string		filename = CPath::standardizePath(rootpath);
	if (usePrevious)
		filename += "previous_state";
	else
		filename += "state";

	if (!f.open(filename) || !ixml.init(f))
		return false;

	try
	{
		serial(ixml);
	}
	catch (const Exception&)
	{
		return false;
	}

	return true;
}

/*
 * State exists in path
 */
bool	CDatabaseState::exists(const std::string& rootpath)
{
	string		filename = CPath::standardizePath(rootpath) + "state";
	return CFile::fileExists(filename);
}


uint64	CMixedStreamFile::_ReadBytes = 0;
uint64	CMixedStreamFile::_WrittenBytes = 0;
