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

#ifndef SHEETBUILDER_H
#define SHEETBUILDER_H

// misc
#include <nel/misc/types_nl.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/config_file.h>

// std
#include <string>
#include <stdlib.h>
#include <stdio.h>

using namespace NLMISC;
using namespace std;

#ifndef NL_MK_SH_ID_CFG
#define NL_MK_SH_ID_CFG "."
#endif // NL_MK_SH_ID_CFG

/**
 *	TFormId
 */
union TFormId
{
	uint32		Id;

	struct
	{
		uint32	Type	: 8;
		uint32	Id		: 24;
	} FormIDInfos;

	void serial(NLMISC::IStream &f)
	{
		f.serial(Id);
	}
};

bool operator<(const TFormId &fid1, const TFormId &fid2)
{
	return fid1.Id<fid2.Id;
}


map<string,TFormId> FormToId;
map<TFormId,string> IdToForm;
map<string,uint8> FileTypeToId;
map<uint8,string> IdToFileType;
map<uint8,uint32> TypeToLastId;
set<string> ExtensionsAllowed;

// stat

uint32 NbTypesAdded = 0;
uint32 NbFilesAdded = 0;
uint32 NbFilesAlreadyAdded = 0;
uint32 NbFilesUnknownType = 0;
uint32 NbFilesDiscarded = 0;

//manageFile
//void manageFile( WIN32_FIND_DATA& findFileData, list<string>& dirs, string& currentDir );

// addId
void addId( string fileName );

// getFileType
bool getFileType( string &fileName, string &fileType );

//-----------------------------------------------
//	getFirstFreeFileTypeId
//
//-----------------------------------------------
sint16 getFirstFreeFileTypeId()
{
	for( sint16 id=0; id<256; ++id )
	{
		if( IdToFileType.find((uint8)id) == IdToFileType.end() )
		{
			return id;
		}
	}

	return -1;

} // getFirstFreeFileTypeId //


//-----------------------------------------------
//	readFormId
//
//-----------------------------------------------
void readFormId( string &outputFileName )
{
	CIFile f;
	if( f.open( outputFileName ) )
	{
		f.serialCont( IdToForm );
	}

	// insert an unknown entry
	TFormId formId;
	formId.Id = 0;
	IdToForm.insert( make_pair( formId, string("unknown.unknown") ) );

	// remove integer file extensions (created by CVS) and init FileTypeToId (associates the form type to the form type id)
	map<TFormId,string>::iterator itIF;
	for( itIF = IdToForm.begin(); itIF != IdToForm.end();  )
	{
		// get the file type from form name
		TFormId fid = (*itIF).first;
		string fileType;

		if((*itIF).second.empty() || (*itIF).second=="." || (*itIF).second==".." || (*itIF).second[0]=='_' || (*itIF).second.find(".#")==0)
		{
			map<TFormId,string>::iterator itErase = itIF;
			++itIF;
			IdToForm.erase(itErase);
		}
		else
		{
			if( getFileType( (*itIF).second, fileType ) )
			{
				// insert the association (file type/file type id)
				map<string,uint8>::iterator itFT = FileTypeToId.find(fileType);
				if( itFT == FileTypeToId.end() )
				{
					FileTypeToId.insert( make_pair(fileType,fid.FormIDInfos.Type) );
				}
			}
			else
			{
				nlwarning("Unknown file type for the file : %s",(*itIF).second.c_str());
			}
			++itIF;
		}
	}


	// init FormToId (associates the form name to its id )
	for( itIF = IdToForm.begin(); itIF != IdToForm.end(); ++itIF )
	{
		FormToId.insert( make_pair((*itIF).second,(*itIF).first) );
	}

	// init IdToFileType (associates the form type id to the form type name)
	map<string,uint8>::iterator itIFT;
	for( itIFT = FileTypeToId.begin(); itIFT != FileTypeToId.end(); ++itIFT )
	{
		IdToFileType.insert( make_pair((*itIFT).second,(*itIFT).first) );
	}

	// init TypeToLastId (associates the type id to the last index used for this type)
	for( itIF = IdToForm.begin(); itIF != IdToForm.end(); ++itIF )
	{
		uint8 type = (*itIF).first.FormIDInfos.Type;
		uint32 id = (*itIF).first.FormIDInfos.Id;
		map<uint8,uint32>::iterator itTLI = TypeToLastId.find( type );
		if( itTLI != TypeToLastId.end() )
		{
			if( (*itTLI).second < id )
			{
				(*itTLI).second = id;
			}
		}
		else
		{
			TypeToLastId.insert( make_pair(type,id) );
		}
	}

} // readFormId //


//-----------------------------------------------
//	makeId
//
//-----------------------------------------------
void makeId( list<string>& dirs )
{
	list<string>::const_iterator itDir;
	for( itDir = dirs.begin(); itDir != dirs.end(); ++itDir )
	{
		nlinfo ("Searching files in directory '%s'...", (*itDir).c_str());
		vector<string> files;
		CPath::getPathContent(*itDir,true,false,true,files);

		nlinfo ("Found %d files in directory '%s'", files.size(), (*itDir).c_str());
		for(uint i = 0; i < files.size(); i++)
		{
			addId(CFile::getFilename(files[i]));
		}
	}

} // makeId //


//-----------------------------------------------
//	addId
//
//-----------------------------------------------
void addId( string fileName )
{
	if(fileName.empty() || fileName=="." || fileName==".." || fileName[0]=='_' || fileName.find(".#")==0)
	{
		//nlinfo("Discarding file '%s'", fileName.c_str());
		NbFilesDiscarded++;
		return;
	}
	else
	{
		if( !ExtensionsAllowed.empty() )
		{
			string extStr = CFile::getExtension( fileName );
			if( ExtensionsAllowed.find(extStr) == ExtensionsAllowed.end() )
			{
				NbFilesDiscarded++;
				return;
			}
		}
	}

	// if the file is new
	map<string,TFormId>::iterator itFI = FormToId.find( fileName );
	if( itFI == FormToId.end() )
	{
		// double check : if file not found we check with lower case version of filename
		map<string,TFormId>::iterator itFILwr = FormToId.find( toLower(fileName) );
		if( itFILwr != FormToId.end() )
		{
			nlwarning("Trying to add %s but the file %s is already known ! becareful with lower case and upper case.", fileName.c_str(), toLower(fileName).c_str());
			NbFilesDiscarded++;
			return;
		}

		string fileType;
		if( getFileType( fileName, fileType ) )
		{
			map<string,uint8>::iterator itFTI = FileTypeToId.find( fileType );
			TFormId fid;

			// if the type of this file is a new type
			if( itFTI == FileTypeToId.end() )
			{
				sint16 firstFreeFileTypeId = getFirstFreeFileTypeId();
				if( firstFreeFileTypeId == -1 )
				{
					nlwarning("MORE THAN 256 FILE TYPES!!!!");
				}
				else
				{
					FileTypeToId.insert( make_pair(fileType,(uint8)firstFreeFileTypeId) );
					IdToFileType.insert( make_pair((uint8)firstFreeFileTypeId,fileType) );
					TypeToLastId.insert( make_pair((uint8)firstFreeFileTypeId,0) );

					fid.FormIDInfos.Type = (uint8)firstFreeFileTypeId;
					fid.FormIDInfos.Id = 0;

					nlinfo("Adding file type '%s' with id %d", fileType.c_str(), firstFreeFileTypeId);
					NbTypesAdded++;
				}
			}
			// else the file type already exist
			else
			{
				// id of the file type
				uint8 fileTypeId = (*itFTI).second;

				// last id used for this file type
				map<uint8,uint32>::iterator itTLI = TypeToLastId.find(fileTypeId);
				nlassert(itTLI != TypeToLastId.end());
				(*itTLI).second++;

				// add the new association
				fid.FormIDInfos.Type = fileTypeId;
				fid.FormIDInfos.Id = (*itTLI).second;
			}
			FormToId.insert( make_pair(fileName,fid) );
			IdToForm.insert( make_pair(fid,fileName) );
			nlinfo("Adding file '%s' id %d with type '%s' id %d", fileName.c_str(), fid.FormIDInfos.Id, fileType.c_str(), fid.FormIDInfos.Type);
			NbFilesAdded++;
		}
		else
		{
			//nlinfo("Unknown file type for the file : '%s' --> not added",fileName.c_str());
			NbFilesUnknownType++;
		}
	}
	else
	{
		//nlinfo("Skipping file '%s', already in the file", fileName.c_str());
		NbFilesAlreadyAdded++;
	}
} // addId //


//-----------------------------------------------
//	getFileType
//
//-----------------------------------------------
bool getFileType( string &fileName, string &fileType )
{
	fileType = CFile::getExtension(CFile::getFilename(fileName));
	return !fileType.empty();

} // getFileType //


//-----------------------------------------------
//	display
//
//-----------------------------------------------
void display()
{
	nldebug ("Output :");
	map<TFormId,string>::iterator it1;
	for( it1 = IdToForm.begin(); it1 != IdToForm.end(); ++it1 )
	{
		nldebug("type: %d id: %d file: %s", (*it1).first.FormIDInfos.Type, (*it1).first.FormIDInfos.Id, (*it1).second.c_str());
	}

} // display //

#endif // SHEETBUILDER_H
