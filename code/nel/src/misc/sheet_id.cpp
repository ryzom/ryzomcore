// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

/* This class is case unsensitive. It means that you can call build() and
 * buildIdVector() with string with anycase, it'll work.
 */

#include "stdmisc.h"

#include "nel/misc/file.h"
#include "nel/misc/path.h"

#include "nel/misc/sheet_id.h"
#include "nel/misc/common.h"
#include "nel/misc/hierarchical_timer.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

CSheetId::CChar CSheetId::_AllStrings;
CStaticMap<uint32,CSheetId::CChar> CSheetId::_SheetIdToName;
CStaticMap<CSheetId::CChar,uint32, CSheetId::CCharComp> CSheetId::_SheetNameToId;
//map<uint32,std::string> CSheetId::_SheetIdToName;
//map<std::string,uint32> CSheetId::_SheetNameToId;
vector<std::string> CSheetId::_FileExtensions;
bool CSheetId::_Initialised=false;
bool CSheetId::_RemoveUnknownSheet=true;
bool CSheetId::_DontHaveSheetKnowledge = false;
std::map<std::string, uint32> CSheetId::_DevTypeNameToId;
std::vector<std::vector<std::string> > CSheetId::_DevSheetIdToName;
std::map<std::string, uint32> CSheetId::_DevSheetNameToId;

#define NL_TEMP_YUBO_NO_SOUND_SHEET_ID

#ifdef NL_TEMP_YUBO_NO_SOUND_SHEET_ID
namespace { bool a_NoSoundSheetId = false; const uint32 a_NoSoundSheetType = 80; }
#endif

const CSheetId CSheetId::Unknown(0);

void CSheetId::cbFileChange (const std::string &filename)
{
	nlinfo ("SHEETID: %s changed, reload it", filename.c_str());

	loadSheetId();
}

//-----------------------------------------------
//	CSheetId
//
//-----------------------------------------------
CSheetId::CSheetId( uint32 sheetRef)
{
	_Id.Id = sheetRef;

#ifdef NL_DEBUG_SHEET_ID
	// Yoyo: don't access the static map, because of order of static ctor call.
	// For now, all static CSheetId are 0 (eg: CSheetId::Unknown)
	if(sheetRef)
	{
		CStaticMap<uint32, CChar>::iterator it(_SheetIdToName.find(sheetRef));
		if (it != _SheetIdToName.end())
		{
			_DebugSheetName = it->second.Ptr;
		}
		else
			_DebugSheetName = NULL;
	}
	else
	{
		_DebugSheetName = NULL;
	}
#endif
}


//-----------------------------------------------
//	CSheetId
//
//-----------------------------------------------
CSheetId::CSheetId( const string& sheetName )
{
	if (!buildSheetId(sheetName))
	{
		if(sheetName.empty())
			nlwarning("SHEETID: Try to create an CSheetId with empty name. TODO: check why.");
		else
			nlwarning("SHEETID: The sheet '%s' is not in sheet_id.bin, setting it to Unknown",sheetName.c_str());
		//std::string stack;
		//NLMISC::getCallStack(stack);
		//std::vector<std::string> contexts;
		//NLMISC::explode(stack, string("\n"), contexts);
		//nldebug("Dumping callstack :");
		//for (uint i=0; i<contexts.size(); ++i)
		//	nldebug("  %3u : %s", i, contexts[i].c_str());
		*this = Unknown;
	}

	// nldebug("LIST_SHEET_ID: %s (%s)", toString().c_str(), sheetName.c_str());

} // CSheetId //

CSheetId::CSheetId( const std::string& sheetName, const std::string &defaultType )
{
	// Don't use this function without defaultType, use the one above.
	nlassert(defaultType.size() != 0);
	
	if (sheetName.rfind('.') == std::string::npos)
	{
		std::string withType = sheetName + "." + defaultType;
		*this = CSheetId(withType);
		// nldebug("SHEETID: Constructing CSheetId from name '%s' without explicit type, defaulting as '%s' to '%s'", sheetName.c_str(), defaultType.c_str(), withType.c_str());
	}
	else
	{
		*this = CSheetId(sheetName);
	}
}


//-----------------------------------------------
//	Build
//
//-----------------------------------------------
bool CSheetId::buildSheetId(const std::string& sheetName)
{
	nlassert(_Initialised);
	
	// When no sheet_id.bin is loaded, use dynamically assigned IDs.
	if (_DontHaveSheetKnowledge)
	{
		std::string sheetNameLc = toLower(sheetName);
		std::map<std::string, uint32>::iterator it = _DevSheetNameToId.find(sheetNameLc);
		if (it == _DevSheetNameToId.end())
		{
			// Create a new dynamic sheet ID.
			// nldebug("SHEETID: Creating a dynamic sheet id for '%s'", sheetName.c_str());
			std::string sheetType = CFile::getExtension(sheetNameLc);
			std::string sheetName = CFile::getFilenameWithoutExtension(sheetNameLc);
			std::map<std::string, uint32>::iterator tit = _DevTypeNameToId.find(sheetType);
			uint32 typeId;
			if (tit == _DevTypeNameToId.end())
			{
				_FileExtensions.push_back(sheetType);
				_DevSheetIdToName.push_back(std::vector<std::string>());
				typeId = (uint32)_FileExtensions.size() - 1;
				_DevTypeNameToId[sheetType] = typeId;
				std::string unknownNewType = std::string("unknown." + sheetType);
				_DevSheetIdToName[typeId].push_back(unknownNewType);
				_Id.IdInfos.Type = typeId;
				_Id.IdInfos.Id = _DevSheetIdToName[typeId].size() - 1;
				_DevSheetNameToId[unknownNewType] = _Id.Id;
				if (sheetName == "unknown")
					return true; // Return with the unknown sheet id of this type
			}
			else
			{
				typeId = tit->second;
				_Id.IdInfos.Type = typeId;
			}
			// Add a new sheet name to the type
			_DevSheetIdToName[typeId].push_back(sheetNameLc);
			_Id.IdInfos.Id = _DevSheetIdToName[typeId].size() - 1;
			// nldebug("SHEETID: Type %i, id %i, sheetid %i", _Id.IdInfos.Type, _Id.IdInfos.Id, _Id.Id);
			_DevSheetNameToId[sheetNameLc] = _Id.Id;
			return true;
		}
		_Id.Id = it->second;
		return true;
	}

	// try looking up the sheet name in _SheetNameToId
	CStaticMap<CChar,uint32,CCharComp>::const_iterator itId;
	CChar c;
	c.Ptr = new char [sheetName.size()+1];
	strcpy(c.Ptr, sheetName.c_str());
	toLower(c.Ptr);

	itId = _SheetNameToId.find (c);
	delete [] c.Ptr;
	if( itId != _SheetNameToId.end() )
	{
		_Id.Id = itId->second;
#ifdef NL_DEBUG_SHEET_ID
		// store debug info
		_DebugSheetName = itId->first.Ptr;
#endif
		return true;
	}

	// we failed to find the sheet name in the sheetname map so see if the string is numeric
	if (sheetName.size()>1 && sheetName[0]=='#')
	{
		uint32 numericId;
		NLMISC::fromString((const char*)(sheetName.c_str()+1), numericId);
		if (NLMISC::toString("#%u",numericId)==sheetName)
		{
			_Id.Id= numericId;
			return true;
		}
	}
	
#ifdef NL_TEMP_YUBO_NO_SOUND_SHEET_ID
	if (a_NoSoundSheetId && sheetName.find(".sound") != std::string::npos)
	{
		std::string sheetNameLc = toLower(sheetName);
		std::map<std::string, uint32>::iterator it = _DevSheetNameToId.find(sheetNameLc);
		if (it == _DevSheetNameToId.end())
		{
			// nldebug("SHEETID: Creating a temporary sheet id for '%s'", sheetName.c_str());
			_DevSheetIdToName[0].push_back(sheetName);
			_Id.IdInfos.Type = a_NoSoundSheetType;
			_Id.IdInfos.Id = _DevSheetIdToName[0].size() - 1;
			_DevSheetNameToId[sheetNameLc] = _Id.Id;
			return true;
		}
		_Id.Id = it->second;
		return true;
	}
#endif
	
	return false;
}

void CSheetId::loadSheetId ()
{
	H_AUTO(CSheetIdInit);
	//nldebug("Loading sheet_id.bin");

	// Open the sheet id to sheet file name association
	CIFile file;
	std::string path = CPath::lookup("sheet_id.bin", false, false);
	if(!path.empty() && file.open(path))
	{
		// clear entries
		_FileExtensions.clear ();
		_SheetIdToName.clear ();
		_SheetNameToId.clear ();

		// reserve space for the vector of file extensions
		_FileExtensions.resize(1 << (NL_SHEET_ID_TYPE_BITS));

		// Get the map from the file
		map<uint32,string> tempMap;
		contReset(tempMap);
		file.serialCont(tempMap);
		file.close();

		if (_RemoveUnknownSheet)
		{
			uint32 removednbfiles = 0;
			uint32 nbfiles = (uint32)tempMap.size();

			// now we remove all files that not available
			map<uint32,string>::iterator itStr2;
			for( itStr2 = tempMap.begin(); itStr2 != tempMap.end(); )
			{
				if (CPath::exists ((*itStr2).second))
				{
					++itStr2;
				}
				else
				{
					map<uint32,string>::iterator olditStr = itStr2;
					//nldebug ("Removing file '%s' from CSheetId because the file not exists", (*olditStr).second.c_str ());
					itStr2++;
					tempMap.erase (olditStr);
					removednbfiles++;
				}
			}
			nlinfo ("SHEETID: Removed %d files on %d from CSheetId because these files don't exist", removednbfiles, nbfiles);
		}

		// Convert the map to one big string and 1 static map (id to name)
		{
			// Get the number and size of all strings
			vector<CChar> tempVec; // Used to initialise the first map
			uint32 nNb = 0;
			uint32 nSize = 0;
			map<uint32,string>::const_iterator it = tempMap.begin();
			while (it != tempMap.end())
			{
				nSize += (uint32)it->second.size()+1;
				nNb++;
				it++;
			}

			// Make the big string (composed of all strings) and a vector referencing each string
			tempVec.resize(nNb);
			_AllStrings.Ptr = new char[nSize];
			it = tempMap.begin();
			nSize = 0;
			nNb = 0;
			while (it != tempMap.end())
			{
				tempVec[nNb].Ptr = _AllStrings.Ptr+nSize;
				strcpy(_AllStrings.Ptr+nSize, it->second.c_str());
				toLower(_AllStrings.Ptr+nSize);
				nSize += (uint32)it->second.size()+1;
				nNb++;
				it++;
			}

			// Finally build the static map (id to name)
			_SheetIdToName.reserve(tempVec.size());
			it = tempMap.begin();
			nNb = 0;
			while (it != tempMap.end())
			{
				_SheetIdToName.add(pair<uint32, CChar>(it->first, CChar(tempVec[nNb])));

				nNb++;
				it++;
			}

			// The vector of all small string is not needed anymore we have all the info in
			// the static map and with the pointer AllStrings referencing the beginning.
		}

		// Build the invert map (Name to Id) & file extension vector
		{
			uint32 nSize = (uint32)_SheetIdToName.size();
			_SheetNameToId.reserve(nSize);
			CStaticMap<uint32,CChar>::iterator itStr;
			for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
			{
				// add entry to the inverse map
				_SheetNameToId.add( make_pair((*itStr).second, (*itStr).first) );

				// work out the type value for this entry in the map
				TSheetId sheetId;
				sheetId.Id=(*itStr).first;
				uint32 type = sheetId.IdInfos.Type;

				// check whether we need to add an entry to the file extensions vector
				if (_FileExtensions[type].empty())
				{
					// find the file extension part of the given file name
					_FileExtensions[type] = toLower(CFile::getExtension((*itStr).second.Ptr));
				}
				nSize--;
			}
			_SheetNameToId.endAdd();
		}
	}
	else
	{
		nlerror("<CSheetId::init> Can't open the file sheet_id.bin");
	}
	nldebug("Finished loading sheet_id.bin: %u entries read",_SheetIdToName.size());
}


//-----------------------------------------------
//	init
//
//-----------------------------------------------
void CSheetId::init(bool removeUnknownSheet)
{
	// allow multiple calls to init in case libraries depending on sheetid call this init from their own
	if (_Initialised)
	{
		if (_DontHaveSheetKnowledge)
			nlinfo("SHEETID: CSheetId is already initialized without sheet_id.bin");
		return;
	}

//	CFile::addFileChangeCallback ("sheet_id.bin", cbFileChange);

	_RemoveUnknownSheet = removeUnknownSheet;

	loadSheetId ();
	_Initialised=true;

#ifdef NL_TEMP_YUBO_NO_SOUND_SHEET_ID
	if (typeFromFileExtension("sound") == std::numeric_limits<uint32>::max())
	{
		nlwarning("SHEETID: Loading without known sound sheet id, please update sheet_id.bin with .sound sheets");
		nlassert(_FileExtensions.size() == 1 << (NL_SHEET_ID_TYPE_BITS));
		nlassert(_FileExtensions[a_NoSoundSheetType].empty());
		_FileExtensions[a_NoSoundSheetType] = "sound";
		_DevSheetIdToName.push_back(std::vector<std::string>());
		_DevSheetIdToName[0].push_back("unknown.sound");
		TSheetId id;
		id.IdInfos.Type = a_NoSoundSheetType;
		id.IdInfos.Id = _DevSheetIdToName[0].size() - 1;
		nlassert(id.IdInfos.Id == 0);
		_DevSheetNameToId["unknown.sound"] = id.Id;
		a_NoSoundSheetId = true;
	}
#endif

} // init //

void CSheetId::initWithoutSheet()
{
	if (_Initialised)
	{
		nlassert(_DontHaveSheetKnowledge);
		return;
	}
	
	_Initialised = true;
	_DontHaveSheetKnowledge = true;
	
	// Initialize id 0,0 as unknown.unknown
	CSheetId unknownunknown = CSheetId("unknown.unknown");
	nlassert(unknownunknown == CSheetId::Unknown);
}



//-----------------------------------------------
//	uninit
//
//-----------------------------------------------
void CSheetId::uninit()
{
	delete [] _AllStrings.Ptr;
	_FileExtensions.clear();
	_DevTypeNameToId.clear();
	_DevSheetIdToName.clear();
	_DevSheetNameToId.clear();
} // uninit //

//-----------------------------------------------
//	operator=
//
//-----------------------------------------------
CSheetId& CSheetId::operator=( const CSheetId& sheetId )
{
	if (!_Initialised) init(false);

	if(this == &sheetId)
	{
		return *this;
	}

	_Id.Id = sheetId.asInt();

#ifdef NL_DEBUG_SHEET_ID
	_DebugSheetName = sheetId._DebugSheetName;
#endif

    return *this;


} // operator= //


//-----------------------------------------------
//	operator=
//
//-----------------------------------------------
CSheetId& CSheetId::operator=( const string& sheetName )
{

	if (!buildSheetId(sheetName))
		*this = Unknown;

	// nldebug("LIST_SHEET_ID: %s (%s)", toString().c_str(), sheetName.c_str());

	return *this;

} // operator= //


//-----------------------------------------------
//	operator=
//
//-----------------------------------------------
CSheetId& CSheetId::operator=( uint32 sheetRef )
{
	if (!_Initialised) init(false);

	_Id.Id = sheetRef;

	return *this;

} // operator= //



//-----------------------------------------------
//	operator<
//
//-----------------------------------------------
bool CSheetId::operator < (const CSheetId& sheetRef ) const
{
	if (!_Initialised) init(false);

	if (_Id.Id < sheetRef.asInt())
	{
		return true;
	}

	return false;

} // operator< //



//-----------------------------------------------
//	toString
//
//-----------------------------------------------
string CSheetId::toString(bool ifNotFoundUseNumericId) const
{
	if (!_Initialised) init(false);
	
	if (_DontHaveSheetKnowledge)
	{
		// FIXME: When someone punches in a fake sheet id this will 
		// fail.
		return _DevSheetIdToName[_Id.IdInfos.Type][_Id.IdInfos.Id];
	}

	CStaticMap<uint32,CChar>::const_iterator itStr = _SheetIdToName.find (_Id.Id);
	if( itStr != _SheetIdToName.end() )
	{
		return string((*itStr).second.Ptr);
	}
	else
	{
#ifdef NL_TEMP_YUBO_NO_SOUND_SHEET_ID
		if (a_NoSoundSheetId && _Id.IdInfos.Type == a_NoSoundSheetType)
		{
			return _DevSheetIdToName[0][_Id.IdInfos.Id];
		}
#endif
		// This nlwarning is commented out because the loggers are mutexed, therefore
		// you couldn't use toString() within a nlwarning().
		//nlwarning("<CSheetId::toString> The sheet %08x is not in sheet_id.bin",_Id.Id);
		if (ifNotFoundUseNumericId)
		{
			return NLMISC::toString( "#%u", _Id.Id );
		}
		else
		{
			return NLMISC::toString( "<Sheet %d not found in sheet_id.bin>", _Id.Id );
		}
	}

} // toString //

void CSheetId::serial(NLMISC::IStream	&f) throw(NLMISC::EStream)
{
	nlassert(!_DontHaveSheetKnowledge);
	
	f.serial( _Id.Id );

#ifdef NL_DEBUG_SHEET_ID
	CStaticMap<uint32, CChar>::iterator it(_SheetIdToName.find(_Id.Id));
	if (it != _SheetIdToName.end())
		_DebugSheetName = it->second.Ptr;
	else
		_DebugSheetName = NULL;
#endif
}

void CSheetId::serialString(NLMISC::IStream &f, const std::string &defaultType) throw(NLMISC::EStream)
{
	nlassert(_Initialised);
	
	if (f.isReading())
	{
		std::string sheetName;
		f.serial(sheetName);
		*this = CSheetId(sheetName, defaultType);
	}
	else
	{
		// if this assert fails, you may be using an outdated id bin
		nlassert(*this != CSheetId::Unknown);
		std::string sheetName = toString();
		f.serial(sheetName);
	}
}


//-----------------------------------------------
//	display
//
//-----------------------------------------------
void CSheetId::display()
{
	if (!_Initialised) init(false);

	CStaticMap<uint32,CChar>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		//nlinfo("%d %s",(*itStr).first,(*itStr).second.c_str());
		nlinfo("SHEETID: (%08x %d) %s",(*itStr).first,(*itStr).first,(*itStr).second.Ptr);
	}

} // display //



//-----------------------------------------------
//	display
//
//-----------------------------------------------
void CSheetId::display(uint32 type)
{
	if (!_Initialised) init(false);

	CStaticMap<uint32,CChar>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		// work out the type value for this entry in the map
		TSheetId sheetId;
		sheetId.Id=(*itStr).first;

		// decide whether or not to display the entry
		if (type==sheetId.IdInfos.Type)
		{
			//nlinfo("%d %s",(*itStr).first,(*itStr).second.c_str());
			nlinfo("SHEETID: (%08x %d) %s",(*itStr).first,(*itStr).first,(*itStr).second.Ptr);
		}
	}

} // display //



//-----------------------------------------------
//	buildIdVector
//
//-----------------------------------------------
void CSheetId::buildIdVector(std::vector <CSheetId> &result)
{
	if (!_Initialised) init(false);

	CStaticMap<uint32,CChar>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		result.push_back( (CSheetId)(*itStr).first );
	}

} // buildIdVector //


//-----------------------------------------------
//	buildIdVector
//
//-----------------------------------------------
void CSheetId::buildIdVector(std::vector <CSheetId> &result, uint32 type)
{
	if (!_Initialised) init(false);
	nlassert(type < (1 << (NL_SHEET_ID_TYPE_BITS)));

	CStaticMap<uint32,CChar>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		// work out the type value for this entry in the map
		TSheetId sheetId;
		sheetId.Id=(*itStr).first;

		// decide whether or not to use the entry
		if (type==sheetId.IdInfos.Type)
		{
			result.push_back( (CSheetId)sheetId.Id );
		}
	}

} // buildIdVector //

//-----------------------------------------------
//	buildIdVector
//
//-----------------------------------------------
void CSheetId::buildIdVector(std::vector <CSheetId> &result, std::vector <std::string> &resultFilenames,uint32 type)
{
	if (!_Initialised) init(false);
	nlassert(type < (1 << (NL_SHEET_ID_TYPE_BITS)));

	CStaticMap<uint32,CChar>::const_iterator itStr;
	for( itStr = _SheetIdToName.begin(); itStr != _SheetIdToName.end(); ++itStr )
	{
		// work out the type value for this entry in the map
		TSheetId sheetId;
		sheetId.Id=(*itStr).first;

		// decide whether or not to use the entry
		if (type==sheetId.IdInfos.Type)
		{
			result.push_back( (CSheetId)sheetId.Id );
			resultFilenames.push_back( (*itStr).second.Ptr );
		}
	}

} // buildIdVector //

//-----------------------------------------------
//	buildIdVector
//
//-----------------------------------------------
void CSheetId::buildIdVector(std::vector <CSheetId> &result,const std::string &fileExtension)
{
	uint32 type=typeFromFileExtension(fileExtension);
	if (type != std::numeric_limits<uint32>::max())
		buildIdVector(result, type);

} // buildIdVector //

//-----------------------------------------------
//	buildIdVector
//
//-----------------------------------------------
void CSheetId::buildIdVector(std::vector <CSheetId> &result, std::vector <std::string> &resultFilenames,const std::string &fileExtension)
{
	uint32 type=typeFromFileExtension(fileExtension);
	if (type != std::numeric_limits<uint32>::max())
		buildIdVector(result,resultFilenames, type);

} // buildIdVector //


//-----------------------------------------------
//	typeFromFileExtension
//
//-----------------------------------------------
uint32 CSheetId::typeFromFileExtension(const std::string &fileExtension)
{
	if (!_Initialised) init(false);

	uint i;
	for (i=0;i<_FileExtensions.size();i++)
		if (toLower(fileExtension)==_FileExtensions[i])
			return i;

	return std::numeric_limits<uint32>::max();
} // typeFromFileExtension //


//-----------------------------------------------
//	fileExtensionFromType
//
//-----------------------------------------------
const std::string &CSheetId::fileExtensionFromType(uint32 type)
{
	if (!_Initialised) init(false);
	nlassert(type < (1<<(NL_SHEET_ID_TYPE_BITS)));

	return _FileExtensions[type];

} // fileExtensionFromType //

//-----------------------------------------------
//	build
//
//-----------------------------------------------
void	CSheetId::buildSheetId(uint32 shortId, uint32 type)
{
	nlassert(shortId < (1<<NL_SHEET_ID_ID_BITS));
	nlassert(type < (1<<(NL_SHEET_ID_TYPE_BITS)));

	_Id.IdInfos.Id= shortId;
	_Id.IdInfos.Type= type;

#ifdef NL_DEBUG_SHEET_ID
	CStaticMap<uint32, CChar>::iterator it(_SheetIdToName.find(_Id.Id));
	if (it != _SheetIdToName.end())
	{
		_DebugSheetName = it->second.Ptr;
	}
	else
		_DebugSheetName = NULL;
#endif

}

} // NLMISC
