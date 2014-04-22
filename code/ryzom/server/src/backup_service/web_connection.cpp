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

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/variable.h>
#include <nel/misc/command.h>
#include <nel/misc/algo.h>
#include <nel/misc/path.h>

#include <nel/net/service.h>
//#include <nel/net/buf_server.h>

using namespace std;
using namespace NLMISC;
using namespace NLNET;


/// Callback type for web connection
typedef void	(*WebCallback)(CMemStream &msgin, TSockId host);

CBufServer*		WebServer = NULL;



void	initWebConnection()
{
	nlassert(WebServer == NULL);

	uint16 port = (uint16) IService::getInstance ()->ConfigFile.getVar ("WebPort").asInt();

	// if the WebPort is set to 0 then we don't initialise the web server
	if (port==0)
		return;

	nlinfo("Initialise Web socket on port %d", port);

	WebServer = new CBufServer ();
	nlassert(WebServer != NULL);
	WebServer->init (port);
}


void	cbExecute(CMemStream &msgin, TSockId host)
{
	std::string	str;
	msgin.serial(str);
	ICommand::execute(str, *NLMISC::InfoLog);
}



void	cbGetSaveList(CMemStream &msgin, TSockId host);
void	cbRestoreSave(CMemStream &msgin, TSockId host);
void	cbCopyOverSave(CMemStream &msgin, TSockId host);








WebCallback WebCallbackArray[] =
{
	cbExecute,
	cbGetSaveList,
	cbRestoreSave,
	cbCopyOverSave,
};


void	updateWebConnection()
{

	nlassert(WebServer != NULL);

	try
	{
		WebServer->update ();

		while (WebServer->dataAvailable ())
		{
			// create a string mem stream to easily communicate with web server
			NLMISC::CMemStream msgin (true);
			TSockId		host;
			bool		success = false;
			std::string	reason;

			try
			{
				WebServer->receive (msgin, &host);

				uint32	fake = 0;
				msgin.serial(fake);

				uint8	messageType = 0xff;
				msgin.serial (messageType);

				if (messageType < sizeof(WebCallbackArray)/sizeof(WebCallbackArray[0]))
				{
					WebCallbackArray[messageType](msgin, host);
					success = true;
				}
			}
			catch (const Exception &e)
			{
				nlwarning ("Error during receiving: '%s'", e.what ());
				reason = e.what();
			}

			if(!success)
			{
				nlwarning ("Failed to decode Web command");

				CMemStream	msgout;
				uint32		fake = 0;
				msgout.serial(fake);

				std::string	result = "0:Failed to decode command";
				if (!reason.empty())
					result += " ("+reason+")";
				msgout.serial (result);
				WebServer->send (msgout, host);
			}
		}
	}
	catch (const Exception &e)
	{
		nlwarning ("Error during update: '%s'", e.what ());
	}
}

void	releaseWebConnection()
{
	nlassert(WebServer != NULL);

	delete WebServer;
	WebServer = 0;
}





/*
 *
 * Callbacks
 *
 */

// Automatic SaveShardRoot path standardization
void cbOnSaveShardRootModified( NLMISC::IVariable& var )
{
	var.fromString( CPath::standardizePath( var.toString() ) );
}

CVariable<string>	IncrementalBackupDirectory("backup", "IncrementalBackupDirectory", "Directory to find incremental backuped archives", "", 0, true);
// (SaveShardRoot from game_share/backup_service_interface.cpp is not instanciated because the nothing is used from that file)
extern CVariable<string>	SaveShardRoot;
CVariable<string>	SaveTemplatePath("backup", "SaveTemplatePath", "Directory to find saves (with shard and account replacement strings)", "$shard/characters/account_$userid_$charid$ext", 0, true);
CVariable<string>	SaveExtList("backup", "SaveExtList", "List of possible extensions for save files (space separated)", "_pdr.bin _pdr.xml .bin", 0, true);


string	checkFile(const string& templatePath, const string& shard, const string& userid, const string& charid, const vector<string>& extensions)
{
	string	result;

	uint	i;
	for (i=0; i<extensions.size(); ++i)
	{
		string	file = templatePath;

		strFindReplace(file, string("$shard"), shard);
		strFindReplace(file, string("$userid"), userid);
		strFindReplace(file, string("$charid"), charid);
		strFindReplace(file, string("$ext"), extensions[i]);

		if (CFile::fileExists(file))
		{
			result +=	file + ":" +
						toString(CFile::getFileModificationDate(file)) + ":" +
						toString(CFile::getFileSize(file)) + "\n";
		}
	}

	return result;
}

void	cbGetSaveList(CMemStream &msgin, TSockId host)
{
	string	str;
	msgin.serial(str);

	vector<string>	params;

	explode(str, string("%%"), params, true);

	string	incrementalDir = IncrementalBackupDirectory;
	string	saveShardRoot = SaveShardRoot;
	string	templatePath = SaveTemplatePath;
	string	extList = SaveExtList;

	string	shard;
	string	userid;
	string	charid;

	uint	i;
	for (i=0; i<params.size(); ++i)
	{
		vector<string>	param;
		explode(params[i], string("="), param, false);

		if (param.empty())
			continue;

		string	var = param[0];
		string	val;
		if (param.size() > 1)
			val = param[1];

		if (var == "incrementalDir")
			incrementalDir = val;
		if (var == "saveShardRoot")
			saveShardRoot = val;
		if (var == "templatePath")
			templatePath = val;
		if (var == "extList")
			extList = val;
		if (var == "shard")
			shard = val;
		if (var == "userid")
			userid = val;
		if (var == "charid")
			charid = val;
	}

	incrementalDir = CPath::standardizePath(incrementalDir);
	saveShardRoot = CPath::standardizePath(saveShardRoot);

	vector<string>	extensions;
	explode(extList, string(" "), extensions, false);

	string	result = checkFile(saveShardRoot+templatePath, shard, userid, charid, extensions);

	vector<string>	incrementalDirectories;
	CPath::getPathContent(incrementalDir, false, true, false, incrementalDirectories);

	if (!incrementalDirectories.empty())
	{
		std::sort(incrementalDirectories.begin(), incrementalDirectories.end());

		for (i=(uint)incrementalDirectories.size()-1; (sint)i>=0; --i)
		{
			string	p = CPath::standardizePath(incrementalDirectories[i], true);
			// avoid double / inside path
			p += (templatePath[0] == '/' ? templatePath.substr(1) : templatePath);

			result += checkFile(p, shard, userid, charid, extensions);
		}
	}

	CMemStream	msgout;
	uint32		fake	= 0;
	msgout.serial(fake);

	result = "1:"+result;

	msgout.serial (result);
	WebServer->send (msgout, host);
}


void	cbRestoreSave(CMemStream &msgin, TSockId host)
{
	string	str;
	msgin.serial(str);

	vector<string>	params;

	explode(str, string("%%"), params, true);

	string	saveShardRoot = SaveShardRoot;
	string	templatePath = SaveTemplatePath;

	string	shard;
	string	userid;
	string	charid;
	string	file;

	uint	i;
	for (i=0; i<params.size(); ++i)
	{
		vector<string>	param;
		explode(params[i], string("="), param, false);

		if (param.empty())
			continue;

		string	var = param[0];
		string	val;
		if (param.size() > 1)
			val = param[1];

		if (var == "saveShardRoot")
			saveShardRoot = val;
		if (var == "templatePath")
			templatePath = val;
		if (var == "shard")
			shard = val;
		if (var == "userid")
			userid = val;
		if (var == "charid")
			charid = val;
		if (var == "file")
			file = val;
	}

	saveShardRoot = CPath::standardizePath(saveShardRoot);

	string	outputfile = CPath::standardizePath(CFile::getPath(saveShardRoot+templatePath))+CFile::getFilename(file);

	strFindReplace(outputfile, string("$shard"), shard);
	strFindReplace(outputfile, string("$userid"), userid);	// just in case...
	strFindReplace(outputfile, string("$charid"), charid);	//

	bool	success;

	success = CFile::copyFile(outputfile, file, false);

	CMemStream	msgout;
	uint32		fake	= 0;
	msgout.serial(fake);

	string	result;

	if (success)
	{
		result = "1:Restore "+outputfile+" succeded.";
	}
	else
	{
		result = "0:Failed to restore "+outputfile+", copy failed.";
	}
	msgout.serial (result);
	WebServer->send (msgout, host);
}

void	cbCopyOverSave(CMemStream &msgin, TSockId host)
{
	string	str;
	msgin.serial(str);

	vector<string>	params;

	explode(str, string("%%"), params, true);

	string	saveShardRoot = SaveShardRoot;
	string	templatePath = SaveTemplatePath;
	string	extList = SaveExtList;

	string	shard;
	string	userid;
	string	charid;
	string	file;

	uint	i;
	for (i=0; i<params.size(); ++i)
	{
		vector<string>	param;
		explode(params[i], string("="), param, false);

		if (param.empty())
			continue;

		string	var = param[0];
		string	val;
		if (param.size() > 1)
			val = param[1];

		if (var == "saveShardRoot")
			saveShardRoot = val;
		if (var == "templatePath")
			templatePath = val;
		if (var == "extList")
			extList = val;
		if (var == "shard")
			shard = val;
		if (var == "userid")
			userid = val;
		if (var == "charid")
			charid = val;
		if (var == "file")
			file = val;
	}

	saveShardRoot = CPath::standardizePath(saveShardRoot);

	vector<string>	extensions;
	explode(extList, string(" "), extensions, false);

	for (i=0; i<extensions.size(); ++i)
	{
		if (file.size() >= extensions[i].size() && file.substr(file.size()-extensions[i].size()) == extensions[i])
		{
			break;
		}
	}

	bool	success = false;
	string	result;

	if (i < extensions.size())
	{
		string	outputfile = CPath::standardizePath(saveShardRoot)+templatePath;

		strFindReplace(outputfile, string("$shard"), shard);
		strFindReplace(outputfile, string("$userid"), userid);
		strFindReplace(outputfile, string("$charid"), charid);
		strFindReplace(outputfile, string("$ext"), extensions[i]);

		success = CFile::copyFile(outputfile, file, false);

		if (!success)
			result = "Failed to copy "+file+" over "+outputfile+".";
	}
	else
	{
		result = "failed to match valid extension";
	}

	result = string(success ? "1:" : "0:")+result;

	CMemStream	msgout;
	uint32		fake	= 0;
	msgout.serial(fake);

	msgout.serial (result);
	WebServer->send (msgout, host);
}
