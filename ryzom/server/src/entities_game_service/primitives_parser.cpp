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
#include "primitives_parser.h"
#include "nel/net/service.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/ligo/primitive_utils.h"
#include "server_share/primitive_cfg.h"
#include "server_share/used_continent.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace NLLIGO;

bool					CPrimitivesParser::_LigoInit = false;
NLLIGO::CLigoConfig		CPrimitivesParser::_LigoConfig;


const char *WORLD_EDITOR_CLASSES_FILE = "world_editor_classes.xml";


//-----------------------------------------------
// CPrimitivesParser init
//-----------------------------------------------
void CPrimitivesParser::init()
{
	// Init ligo
	if (!_LigoInit)
	{
	if (!_LigoConfig.readPrimitiveClass (WORLD_EDITOR_CLASSES_FILE, false))
	{
		// Should be in l:\leveldesign\world_editor_files
		nlerror ("Can't load ligo primitive config file world_editor_classes.xml");
		}
		_LigoInit = true;
	}

	// init the primitive context
	CPrimitiveContext::instance().CurrentLigoConfig = &_LigoConfig;


	// get the primitive files to load
	CPrimitiveCfg::readPrimitiveCfg();

	vector< string > mapConfigNames;

	// load only active primitive maps...
	{
		CConfigFile::CVar& usedPrimitives = IService::getInstance()->ConfigFile.getVar("UsedPrimitives");
		for ( uint i = 0; (sint)i<usedPrimitives.size(); ++i)
		{
			if (usedPrimitives.asString(i) != "")
				mapConfigNames.push_back(usedPrimitives.asString(i));
		}
	}

	set<string>	loadedPrimitives;	//	to remember the loadedprmitives

	// count number of primitives
	uint primCount = 0;
	for	(uint i=0; i<mapConfigNames.size(); ++i)
	{
		const vector<std::string> &prims = CPrimitiveCfg::getMap(mapConfigNames[i]);
		primCount += (uint)prims.size();
	}

	uint numLoadedPrim = 0;
	for	(uint i=0; i<mapConfigNames.size(); ++i)
	{
		const vector<std::string> &prims = CPrimitiveCfg::getMap(mapConfigNames[i]);

		nlinfo("CPrimitivesParser : loading %d primitives files for mapConfigName '%s' in continent '%s'", prims.size(), mapConfigNames[i].c_str(), CPrimitiveCfg::getContinentNameOf(mapConfigNames[i]).c_str());
		// resize the primitive vector
		// parse all the selected files
		vector<string>::const_iterator first(prims.begin()), last(prims.end());
		for	(;first != last; ++first, ++numLoadedPrim)
		{
			const	string fullName = CPath::lookup( *first );

			if	(loadedPrimitives.find(fullName)!=loadedPrimitives.end())	//	don't want to load a primitive twice!
				continue;
			loadedPrimitives.insert(fullName);

			_Primitives.push_back(TLoadedPrimitive());
			try
			{
				nldebug( "CPrimitivesParser : loading primitive file '%s' %u/%u", first->c_str(), numLoadedPrim+1, primCount);
				if	(	fullName.empty()
					||	!loadPrimitive( fullName.c_str(),_Primitives.back().Primitive ))
				{
					nlwarning("<CPrimitivesParser ctor> error in primitive file %s",fullName.c_str());
				}
				_Primitives.back().FileName = fullName;
//				_PrimitiveFiles.push_back(fullName);
			}
			catch(const Exception &e)
			{
				nlwarning("<CPrimitivesParser ctor> exception launched : %s",e.what() );
				_Primitives.pop_back();
			}

		}

	}
}// CPrimitivesParser ctor

//-----------------------------------------------
// CPrimitivesParser loadPrimitive
//-----------------------------------------------
bool CPrimitivesParser::loadPrimitive(const char* fileName,CPrimitives & primitives)
{
	bool	readXml = true;
	string binFileName = NLNET::IService::getInstance()->WriteFilesDirectory.toString() +"primitive_cache/"+CFile::getFilename(fileName)+".binprim";
	// Test if binary caching is wanted
	bool cachePrims = true;
	CConfigFile::CVar *cachePrimsVar = IService::getInstance()->ConfigFile.getVarPtr("CachePrims");
	if (cachePrimsVar)
	{
		cachePrims = cachePrimsVar->asInt() != 0;
	}
	bool cachePrimsLog = false;
	CConfigFile::CVar *cachePrimsLogVar = IService::getInstance()->ConfigFile.getVarPtr("CachePrimsLog");
	if (cachePrimsLogVar)
	{
		cachePrimsLog = cachePrimsLogVar->asInt() != 0;
	}
	if (cachePrims
		&& CFile::fileExists(binFileName)
		&& CFile::getFileModificationDate(binFileName) > CFile::getFileModificationDate(fileName))
	{
		// if primitive definition file is more recent, then must recompute all primitives
		std::string defFile = NLMISC::CPath::lookup(WORLD_EDITOR_CLASSES_FILE, false, false);
		if (!defFile.empty() &&
            CFile::getFileModificationDate(binFileName) > CFile::getFileModificationDate(defFile))
		{
			// ok, the cache is here and up to date !
			if (cachePrimsLog)
			{
				nlinfo("Loading '%s' from binary file '%s'",
					fileName,
					binFileName.c_str());
			}
			try
			{
				CIFile binFile(binFileName);
				CPrimitiveContext::instance().CurrentPrimitive = &primitives;
				primitives.serial(binFile);
				CPrimitiveContext::instance().CurrentPrimitive = NULL;
				// ok, all was fine, don't read in xml !
				readXml = false;
			}
			catch(...)
			{}
		}
	}
	//////////////////////////////////////////////
	if (readXml)
	{
		//open prim file
		CIFile file;
		if (!file.open (fileName))
		{
			nlwarning ("<CPrimitivesParser loadPrimitive> Failed to open file %s for reading.", fileName);
			return false;
		}
		// init XML stream
		CIXml xml;
		xml.init (file);

		// set the primitive context
		CPrimitiveContext::instance().CurrentPrimitive = &primitives;
		// read the stream
		if (!primitives.read (xml.getRootNode (), fileName, _LigoConfig))
		{
			nlwarning ("<CPrimitivesParser loadPrimitive> Error reading file %s", fileName);
			CPrimitiveContext::instance().CurrentPrimitive = NULL;
			return false;
		}
		CPrimitiveContext::instance().CurrentPrimitive = NULL;
		if (cachePrims)
		{
			CFile::createDirectory(IService::getInstance()->WriteFilesDirectory.toString()+"primitive_cache");
			COFile saveBin(binFileName);
			primitives.serial(saveBin);
		}
	}
	return true;

}// CPrimitivesParser loadPrimitive


bool CPrimitivesParser::getAlias(const NLLIGO::IPrimitive *prim, uint32 &alias)
{
	TPrimitiveClassPredicate pred("alias");
	IPrimitive *aliasNode = getPrimitiveChild(const_cast<NLLIGO::IPrimitive*>(prim), pred);
	if (aliasNode)
	{
		const CPrimAlias *pa = dynamic_cast<const CPrimAlias*>(aliasNode);
		alias = pa->getFullAlias();

		return true;
	}

	alias = 0;
	return false;
}

std::string CPrimitivesParser::aliasToString(uint32 alias)
{
	return _LigoConfig.aliasToString(alias);
}

uint32 CPrimitivesParser::aliasFromString(const std::string& alias)
{
	return _LigoConfig.aliasFromString(alias);
}

uint32 CPrimitivesParser::aliasGetStaticPart(uint32 alias)
{
	uint32 staticPart;
	uint32 dynPart;

	staticPart = (alias & _LigoConfig.getStaticAliasMask()) >> _LigoConfig.getDynamicAliasSize();
	dynPart = alias & _LigoConfig.getDynamicAliasMask();

	return staticPart;
}

uint32 CPrimitivesParser::aliasGetDynamicPart(uint32 alias)
{
	uint32 staticPart;
	uint32 dynPart;

	staticPart = (alias & _LigoConfig.getStaticAliasMask()) >> _LigoConfig.getDynamicAliasSize();
	dynPart = alias & _LigoConfig.getDynamicAliasMask();

	return dynPart;
}



