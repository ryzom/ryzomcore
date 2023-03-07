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

#include "nel/misc/debug.h"
#include "nel/misc/path.h"

//#include <io.h>
#include <stdio.h>

#include "xml.h"


using namespace NLMISC;

static inline bool isBlank(char c)
{
	static bool lookup[]=
	{// 0 1 2 3  4 5 6 7  8 9 A B  C D E F  0 1 2 3  4 5 6 7  8 9 A B  C D E F								  
		0,0,0,0, 0,0,0,0, 0,1,1,0, 0,1,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, //   0  
		1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, //  32  
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, //  64  
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, //  96  
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 128  
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 160  
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 192  
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 224  
	};
	return lookup[uint8(c)];
}

static inline bool isTxt(char c)
{
	static bool lookup[]=
	{// 0 1 2 3  4 5 6 7  8 9 A B  C D E F  0 1 2 3  4 5 6 7  8 9 A B  C D E F								  
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, //   0 
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0, //  32
		0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,1, //  64
		0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0, //  96
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 128
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 160
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 192
		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, // 224
	};
	return lookup[uint8(c)];
}

static inline bool isArgTxt(char c)
{
	static bool lookup[]=
	{// 0 1 2 3  4 5 6 7  8 9 A B  C D E F  0 1 2 3  4 5 6 7  8 9 A B  C D E F								  
		0,0,0,0, 0,0,0,0, 0,1,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, //   0 
		1,1,0,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 0,1,0,1, //  32
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, //  64
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, //  96
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, // 128
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, // 160
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, // 192
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, // 224
	};
	return lookup[uint8(c)];
}

static inline bool isFreeTxt(char c)
{
	static bool lookup[]=
	{// 0 1 2 3  4 5 6 7  8 9 A B  C D E F  0 1 2 3  4 5 6 7  8 9 A B  C D E F								  
		0,0,0,0, 0,0,0,0, 0,1,1,0, 0,1,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, //   0 
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 0,1,0,1, //  32
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, //  64
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, //  96
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, // 128
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, // 160
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, // 192
		1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1, // 224
	};
	return lookup[uint8(c)];
}

static bool xmlTxtToString(char *txt,uint length,std::string &destination)
{
	bool result=true;

	destination.erase();
	destination.reserve(length);

	for (char *ptr=txt;ptr<txt+length;ptr++)
		if (ptr[0]=='&' && ptr[1]=='a' && ptr[2]=='m' && ptr[3]=='p' && ptr[4]==';')
		{
			destination+='&';
			ptr+=4;
		}
		else if (ptr[0]=='&' && ptr[1]=='l' && ptr[2]=='t' && ptr[3]==';')
		{
			destination+='<';
			ptr+=3;
		}
		else if (ptr[0]=='&' && ptr[1]=='g' && ptr[2]=='t' && ptr[3]==';')
		{
			destination+='>';
			ptr+=3;
		}
		else if (ptr[0]=='&' && ptr[1]=='q' && ptr[2]=='u' && ptr[3]=='o' && ptr[4]=='t' && ptr[5]==';' )
		{
			destination+='\"';
			ptr+=5;
		}
		else
		{
			result = result && (*ptr!='&');
			destination+=*ptr;
		}

	return result;
}

bool CxmlNode::read(const std::string &filename)
{
	// opening the file
	FILE *inf=fopen(filename.c_str(),"rt");
	if (inf==0)
	{
		nlwarning("Failed to open file for reading: %s",filename.c_str());
		return false;
	}
	nlinfo("READING: %s",filename.c_str());

	// allocate memmory	for the read buffer
	//uint bufsize=filelength(fileno(inf))+1;
	uint32 bufsize = CFile::getFileSize(filename);
	char *buf=new char[bufsize+1];
	if (buf==0)
	{
		nlwarning("Failed to allocate memmory for input file: %s",filename.c_str());
		fclose(inf);
		return false;
	}

	// read the data into the buffer and nul terminate it
	uint count;
	count=fread(buf,bufsize,1,inf);
	nlassert(count<bufsize);
	buf[count]=0;

	// close the file
	fclose(inf);

	// skip to the body of the file - if this is a true xml file there is only one body
	char *ptr;
	for (ptr=buf;*ptr!=0;ptr++)
	{
		if (ptr[0]=='<' && ptr[1]!='?')
			break;
	}
	if (*ptr==0)
	{
		nlwarning("Failed to find XML file body for file: %s",filename.c_str());
		delete [] (buf);
		return false;
	}

	// pass parsing control over to the main xml file clause
	bool retval= parseInput(ptr);

	// make sure there's nothing left in the input data stream after parseInput() finishes
	while (retval && isBlank(*ptr)) ptr++;
	if (*ptr!=0)
		retval=false;
		
	// housekeeping and exit
	delete [] (buf);
	return retval;
}

bool CxmlNode::parseInput(char * &ptr)
{
	// make sure this clause hasn't already been used
	nlassert(_name.empty());	
	nlassert(_txt.empty());

	char *marker0;

	// if there's no start of clause marker consider we have free text
	if (*ptr!='<')
	{
		for (;isFreeTxt(*ptr);ptr++) _txt+=*ptr;
		// make sure the free text is followed by a clause marker of some sort
		return (*ptr=='<');
	}

	// skip the opening '<'	of the start of clause marker 
	*ptr++;

	// if we have '<!' parse as comment to closing '>'
	if (*ptr=='!')
	{
		while (isFreeTxt(*++ptr)) _txt+=*ptr;
		// skip the end of comment marker
		if (*ptr!='>')
			return false;
		ptr++;
		return true;
	}

	// this should be a normal named clause so identify the clause name
	while (isBlank(*ptr)) ptr++;
	for (marker0=ptr; isTxt(*ptr); ptr++);
	xmlTxtToString(marker0,ptr-marker0,_name);

	// extract arguments
	while (isBlank(*ptr)) ptr++;
	while (isTxt(*ptr))
	{
		// extract the argument name for the next argument
		for (marker0=ptr; isTxt(*ptr); ptr++);
		std::string argName;
		xmlTxtToString(marker0,ptr-marker0,argName);

		// skip the '=' sign and following '"' sign
		while (isBlank(*ptr)) ptr++;
		if(*ptr!='=')
			return false;
		do { ptr++; }while (isBlank(*ptr));
		if(*ptr!='\"')
			return false;
		ptr++;

		// find the string delimited by the quotes
		for (marker0=ptr; isArgTxt(*ptr); ptr++);
		if(*ptr!='\"')
			return false;
		xmlTxtToString(marker0,ptr-marker0,_args[argName]);

		// skip blanks in preparation for next arg (or end of args)
		while (isBlank(*ptr)) ptr++;
	}

	// if this is a self-contained clause (ie '<'...'/>') then we've finished
	if (*ptr=='/' && ptr[1]=='>')
	{
		ptr+=2;
		return true;
	}

	// skip the end of clause header marker 
	if (*ptr!='>')
		return false;
	ptr++;

	// read sub-clauses
	while (*ptr && !(ptr[0]=='<'&&ptr[1]=='/'))
	{
		CxmlNode *childClause =new CxmlNode;
		_children.push_back(childClause);
		childClause->parseInput(ptr);
	}

	// skip the opening '</' of the clause end marker
	if (!(ptr[0]=='<'&&ptr[1]=='/'))
		return false;
	ptr+=2;

	// read the clause name and make sure it matches the original version
	while (isBlank(*ptr)) ptr++;
	for (marker0=ptr; isTxt(*ptr); ptr++);
	std::string name;
	xmlTxtToString(marker0,ptr-marker0,name);
	if (name!=_name)
		return false;

	// skip the closing '>' of the clause end marker
	while (isBlank(*ptr)) ptr++;
	if (*ptr!='>')
		return false;
	ptr+=2;

	return true;
}


