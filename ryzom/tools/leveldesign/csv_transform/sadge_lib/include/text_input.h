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

//-------------------------------------------------------------------------------------------------
// project: sadge lib
// file:	text_input.h
// version:	$Id: text_input.h,v 1.2 2004/06/15 17:33:37 boucher Exp $	
//
//-------------------------------------------------------------------------------------------------

#ifndef SADGE_TEXT_INPUT_H
#define SADGE_TEXT_INPUT_H

//-------------------------------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/path.h"

#include "sadge_lib/include/text_output.h"

#include <stdio.h>

//----------------------------------------------------------------------
// CTextInput - an object that encapsulates output
//----------------------------------------------------------------------

class CTextInput
{
public:
	CTextInput(CTextOutput *warnings=NULL)
	{
		// initialise the pointer properties
		_Warnings=warnings;
		_SubFile=NULL;
		_Buffer=NULL;

		// call the 'reset' method to initalise other properties
		clear();
	}

	CTextInput(const char *fileName,CTextOutput *warnings=NULL)
	{
		// use default ctor to perform common initialisation code
		*this=CTextInput();

		// initialise the warning channel pointer (if supplied)
		_Warnings=warnings;

		// read the input file
		readFile(fileName);
	}

	~CTextInput()
	{
		clear();
	}

	void clear()
	{
		// free the input buffer
		if (_Buffer!=NULL)
			free(_Buffer);
		_Buffer=NULL;

		// clear any child input manager objects
		if (_SubFile!=NULL)
			delete _SubFile;
		_SubFile=NULL;

		// clear the 'used file names' list
		_FileNames.clear();

		// reset 
		_FileLength=0;
		_CurPos=0;
		_LineNumber=1;
	}

	void readFile(const char *fileName)
	{
		// make sure we've never read this file before
		unsigned i;
		for (i=0;i<_FileNames.size();++i)
			if (fileName==_FileNames[i])
			{
				*_Warnings<<"Ignoring 'read' of file: "<<fileName<<" because file already read previously\n";
				return;
			}

		// if we already have a buffer allocated then we must open a sub-file context
		if (_Buffer!=NULL)
		{
			// if we have an open sub file then recurse into it else setup new sub file here
			if (_SubFile!=NULL)
				_SubFile->readFile(fileName);
			else
			{
				_SubFile=new CTextInput(fileName);
				_FileNames.push_back(fileName);
			}
			return;
		}

		// make a note of the file anme for later
		_FileNames.push_back(fileName);

		// open the file
		FILE* inf=fopen(fileName,"rb");
		if 	(inf==NULL)
		{
			printf("File not found: %s\n",fileName);
			goto ERROR_LABEL;
		}

		// allocate a big buffer for input
		_FileLength= (inf!=NULL)? NLMISC::CFile::getFileSize(inf): 0;
		_Buffer=(char *)malloc(_FileLength+1);
		if (_Buffer==NULL)
		{
			printf("Failed to allocate %d bytes of RAM required to read file: %s\n",_FileLength+1,fileName);
			goto ERROR_LABEL;
		}

		// read the input
		if (fread(_Buffer,1,_FileLength,inf)!=_FileLength)
		{
			printf("Failed to allocate %d bytes of RAM required to read file: %s\n",_FileLength+1,fileName);
			goto ERROR_LABEL;
		}

		// housekeeping
		_Buffer[_FileLength]=0;
		fclose(inf);
		return;

	ERROR_LABEL:
		// in case of error we have a few things to clean up properly...
		clear();
		if (inf!=NULL)
			fclose(inf);
	}

	bool eof()
	{
		return _CurPos>=_FileLength;
	}

	NLMISC::CSString getLine()
	{
		// if there's an open sub file then recurse into it
		if (_SubFile!=NULL)
		{
			// check that we're not at the end of the sub file
			if (!_SubFile->eof())
				return _SubFile->getLine();

			// we're at the end of the sub file so close it down and return to business as usual
			delete _SubFile;
			_FileNames.pop_back();
			_SubFile=NULL;
		}

		NLMISC::CSString result;

		// skip leading empty lines
		while (_Buffer[_CurPos]=='\n' || _Buffer[_CurPos]=='\r')
		{
			if (_Buffer[_CurPos]=='\n')
				++_LineNumber;
			++_CurPos;
		}

		// copy out the text to the next '\n' or '\r'
		while (!eof() && _Buffer[_CurPos]!='\n' && _Buffer[_CurPos]!='\r')
		{
			result+= _Buffer[_CurPos];
			++_CurPos;
		}

		return result;
	}

	NLMISC::CSString getLineName()
	{
		NLMISC::CSString result;
		if (_SubFile!=NULL)
			result=_SubFile->getLineName()+'<';
		result+=NLMISC::toString("%s:%d:",_FileNames.back().c_str(),_LineNumber);
		return result;
	}

private:
	unsigned _FileLength;
	unsigned _CurPos;
	unsigned _LineNumber;
	char *_Buffer;

	std::vector<NLMISC::CSString> _FileNames;
	CTextInput *_SubFile;

	CTextOutput *_Warnings;
};

#endif
