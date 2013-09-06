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

// CodeFast.cpp : Defines the entry point for the console application.
//

/*

  TODO
  - re-do 'case' parser to look for anything with a '::' sub-string		  
  - build handy command line behaviour
  - test ... test ... outfile timestamp on no change test...

*/
#include "nel/misc/types_nl.h"

#include <string>
#include <vector>
#include <assert.h>
#include <conio.h>
#include <io.h>
#include <stdio.h>

class COutputFile
{
public:
	enum TParamType { STRING, INT, FLOAT };

	// ctor
	COutputFile(const char *filename);

	// input string parsers
	void processString(const std::string &s);
	void processCase(const std::string &s);

	void addStruct(const std::string &name,const std::string &qualifiers);
	void addStructEntry(const std::string &paramName,const std::string &className,TParamType type,const std::string &defaultValue,const std::string &comment);

	void addEnum(const std::string &name);
	void addEnumEntry(const std::string &entry);

	void display() const;
	void generateOutput() const;
	void generateEnumOutput(std::string &outbuff) const;

private:
	std::string _FileName;

	struct CStructParam
	{
		std::string _Name;
		std::string _Class;
		TParamType _Type;
		std::string	_DefaultValue;
		std::string _Comment;
	};

	struct CStruct
	{
		std::string _Name;
		std::string _Qualifiers;
		std::vector<CStructParam> _Params;

		void display() const;
		void generateOutput(std::string &outbuff) const;
	};

	std::vector<CStruct> _Structures;
	std::vector<std::string> _Enums;
	std::vector<std::string> _EnumEntries;

	unsigned _OpenStruct;	// the index of the last open structure
};

inline  COutputFile::COutputFile(const char *filename)
{
	_OpenStruct=~0u;
	_FileName=filename;

	// display the string
	printf("OUTPUT FILE: %s\n",filename);
}

inline void COutputFile::processString(const std::string &s)
{
	/*
	special strings:
	'ENUM' <name>
	'STRUCT' <name>
	'-'('s'|'i'|'f') <class> <propertyName> ['='<default value>] ['//'<comment>]
	*/

	// see whether we have a struct parameter
	if (s.size()>5 && s[0]=='-')
	{
		// start by parsing the 'type' character
		if ( (s[1]!='s' && s[1]!='i' && s[1]!='f') || (s[2]!=' ' && s[2]!='\t') )
		{
			printf("Ignoring structure parameter due to bad type (should be 's','i' or 'f'): %s",s.c_str());
			return;
		}
		COutputFile::TParamType type= s[1]=='i'? COutputFile::INT: s[1]=='s'? COutputFile::STRING: FLOAT;

		// skip space after the 'type' character
		unsigned i=2;
		while ( i<s.size() && (s[i]==' '||s[i]=='\t') )
			++i;

		// extract the class name string
		std::string className;
		while ( i<s.size() && (s[i]!=' '&&s[i]!='\t') )
			className+=s[i++];

		// skip space before the property name
		while ( i<s.size() && (s[i]==' '||s[i]=='\t') )
			++i;

		// extract the property name string
		std::string propertyName;
		while ( i<s.size() && (s[i]!=' '&&s[i]!='\t'&&s[i]!='=') )
			propertyName+=s[i++];

		// skip space after the property name
		while ( i<s.size() && (s[i]==' '||s[i]=='\t') )
			++i;

		// if there is a default value string then extract it
		std::string defaultValue;
		if (i<s.size() && s[i]=='=')
		{
			// skip the '='
			++i;

			// skip space before the default value
			while ( i<s.size() && (s[i]==' '||s[i]=='\t') )
				++i;

			// extract the default value string
			while ( i<s.size() && (s[i]!=' '&&s[i]!='\t') )
				defaultValue+=s[i++];

			// skip space after the default value
			while ( i<s.size() && (s[i]==' '||s[i]=='\t') )
				++i;
		}

		// if there's a comment string then extract it
		std::string comment;
		if (i+1<s.size() && s[i]=='/' && s[i+1]=='/')
		{
			// copy the rest of the string into the comment
			comment=s.substr(i);
			i=s.size();
		}

		// make sure the whole thing's valid
		if (className.empty()||propertyName.empty()||i!=s.size())
		{
			printf("Ignoring structure parameter due to syntax error: %s",s.c_str());
			return;
		}

		// display the string
//		printf("FOUND struct param: %s\n",s.c_str());
		addStructEntry(propertyName,className,type,defaultValue,comment);
		return;
	}

	// see whether we have a 'struct'
	if (s.size()>7 && (s[6]==' ' || s[6]=='\t'))
	if ((s[0]|('a'^'A')=='s') && (s[1]|('a'^'A')=='t') && (s[2]|('a'^'A')=='r') && 
		(s[3]|('a'^'A')=='u') && (s[4]|('a'^'A')=='c') && (s[5]|('a'^'A')=='t'))
	{
		// we have a struct so skip the first 6 letters + opening spaces
		unsigned i=7;
		while (i<s.size() && (s[i]==' '||s[i]=='\t'))
			++i;

		// extract the struct name
		std::string name;
		unsigned j=i;
		// skip opening blanks
		while (j<s.size() && (s[j]==' '||s[j]=='\t'))
			++j;
		// copy out name
		while (j<s.size() && (s[j]!=' '&&s[j]!='\t' && s[j]!='/' && s[j]!=':'))
			name+=s[j++];
		// make sure the enum has a name
		if (name.empty())
		{
			printf("Struct refference skipped due to lack of name!\n");
			return;
		}

		// extract the struct qualifiers (if any)
		std::string qualifiers;
		// skip opening blanks
		while (j<s.size() && (s[j]==' '||s[j]=='\t'))
			++j;
		// copy out qualifiers
		while (j<s.size())
			qualifiers+=s[j++];

		// display the string
//		printf("FOUND STRUCT: name='%s' qualifiers='%s'\n",name.c_str(),qualifiers.c_str());
		addStruct(name,qualifiers);
		return;
	}

	// see whether we have an 'enum'
	if (s.size()>5 && (s[4]==' ' || s[4]=='\t'))
	if ((s[0]|('a'^'A')=='e') && (s[1]|('a'^'A')=='n') && (s[2]|('a'^'A')=='u') && (s[3]|('a'^'A')=='m'))
	{
		// we have an enum so skip the first 4 letters + opening spaces
		unsigned i=5;
		while (i<s.size() && (s[i]==' '||s[i]=='\t'))
			++i;

		// extract the enum name 
		std::string name;
		unsigned j=i;
		// skip opening blanks
		while (j<s.size() && (s[j]==' '||s[j]=='\t'))
			++j;
		// copy out name
		while (j<s.size() && (s[j]!=' '&&s[j]!='\t' && s[j]!='/'))
			name+=s[j++];
		// make sure the enum has a name
		if (name.empty())
		{
			printf("Enum refference skipped due to lack of name!\n");
			return;
		}

		// display the string
//		printf("FOUND ENUM: name='%s'\n",name.c_str());
		addEnum(name);
		return;
	}

	// display the string
	printf("FOUND UNKNOWN: %s\n",s.c_str());
}

inline void COutputFile::processCase(const std::string &s)
{
	// display the string
//	printf("FOUND 'case' string: %s\n",s.c_str());
	addEnumEntry(s);
}

inline void COutputFile::addStruct(const std::string &name,const std::string &qualifiers)
{
	unsigned i;
	for (i=0; i<_Structures.size();++i)
		if (name==_Structures[i]._Name)
		{
			printf("Warning: Double declaration of structure: %s\n",name.c_str());
			_OpenStruct=i;
			return;
		}
	_Structures.resize(i+1);
	_OpenStruct= i;
	_Structures[_OpenStruct]._Name= name;
	_Structures[_OpenStruct]._Qualifiers= qualifiers;
}

inline void COutputFile::addStructEntry(const std::string &paramName,const std::string &className,COutputFile::TParamType type,const std::string &defaultValue,const std::string &comment)
{
	if (_OpenStruct>=_Structures.size())
	{
		printf("Failed to add structure entry (%s) due to lack of open structure!\n",paramName.c_str());
		return;
	}

	unsigned paramCount=_Structures[_OpenStruct]._Params.size();
	_Structures[_OpenStruct]._Params.resize(paramCount+1);
	COutputFile::CStructParam &param= _Structures[_OpenStruct]._Params[paramCount];
	param._Class=className;
	param._Comment=comment;
	param._DefaultValue=defaultValue;
	param._Name=paramName;
	param._Type=type;
}

inline void COutputFile::addEnum(const std::string &name)
{
	unsigned i;
	for (i=0; i<_Enums.size();++i)
		if (name==_Enums[i])
		{
			printf("Warning: Double declaration of enum: %s\n",name.c_str());
			return;
		}
	_Enums.resize(i+1);
	_Enums[i]=name;
}

inline void COutputFile::addEnumEntry(const std::string &entry)
{
	// see whether this entry already exists and don't add a second time of this is the case
	for (unsigned i=_EnumEntries.size();i--;)
		if (_EnumEntries[i]==entry)
			return;

	// entry not found in existing values in vector so append it
	_EnumEntries.push_back(entry);
}

void COutputFile::display() const
{
	unsigned i;

	// display the filename
	printf("\nFILE: %s\n",_FileName.c_str());

	// display the set of structures
	for (i=0;i<_Structures.size();++i)
	{
		if (i==_OpenStruct)
			printf("\n* ");
		else
			printf("\n  ");
		_Structures[i].display();
	}

	// display the set of Enums
	for (i=0;i<_Enums.size();++i)
	{
		printf("\nENUM: %s\n",_Enums[i].c_str());

		unsigned j;
		for (j=0;j<_EnumEntries.size();++j)
		{
			if (_EnumEntries[j].substr(0,_Enums[i].size()+2)==_Enums[i]+"::")
				printf("=> %s\n",_EnumEntries[j].substr(_Enums[i].size()+2).c_str());
		}
	}
}

void COutputFile::generateEnumOutput(std::string &outbuff) const
{
	unsigned i;

	// output the set of Enums
	for (i=0;i<_Enums.size();++i)
	{
		// open the enum
		outbuff+="class ";
		outbuff+=_Enums[i]+"\n";
		outbuff+="{\n";
		outbuff+="public:\n";
		outbuff+="\tenum TValueType\n";
		outbuff+="\t{\n";

		// add values to the enum
		unsigned j;
		for (j=0;j<_EnumEntries.size();++j)
		{
			if (_EnumEntries[j].substr(0,_Enums[i].size()+2)==_Enums[i]+"::")
			{
				outbuff+="\t\t";
				outbuff+=_EnumEntries[j].substr(_Enums[i].size()+2);
				outbuff+=",\n";
			}
		}

		// close the enum
		outbuff+="\t\tNUM_VALUES,\n";
		outbuff+="\t\tBAD_VALUE= NUM_VALUES\n";
		outbuff+="\t};\n";
		outbuff+="\n";

		// IId: a template id interface class
		outbuff+="\tclass IId : public NLMISC::CRefCount\n";
		outbuff+="\t{\n";
		outbuff+="\tpublic:\n";
		outbuff+="\t\tIId() : ParsedOk(false) { }\n";
		outbuff+="\t\tTValueType id() const\t{ return _Id; }\n";
		outbuff+="\t\tvoid convertInput(std::vector<std::string> &args, const std::string &input)\n";
		outbuff+="\t\t{\n";
		outbuff+="\t\t\tunsigned i=0,j=0;\n";
		outbuff+="\t\t\twhile (i < input.size())\n";
		outbuff+="\t\t\t{\n";
		outbuff+="\t\t\t\tj=i;\n";
		outbuff+="\t\t\t\twhile( i<input.size() && input[i]!=':' ) ++i;\n";
		outbuff+="\t\t\t\targs.push_back(input.substr(j,i-j));\n";
		outbuff+="\t\t\t\t++i;\n";
		outbuff+="\t\t\t}\n";
		outbuff+="\t\t}\n\n";		
		outbuff+="\t\tbool ParsedOk;\n\n";
		outbuff+="\tprotected:\n";
		outbuff+="\t\tTValueType _Id;\n";
		outbuff+="\t};\n";
		outbuff+="\ttypedef NLMISC::CSmartPtr<IId> IIdPtr;";
		outbuff+="\n\n";

		// CId: a template specialisation of the id interface class 'IId'
		/*
		outbuff+="\ttemplate <TValueType ID> class CId: public IId\n";
		outbuff+="\t{\n";
		outbuff+="\tpublic:\n";
		outbuff+="\t\tCId()\t\t\t\t{ _Id=ID; }\n";
		outbuff+="\t};\n";
		outbuff+="\n";
		*/

		// open the '=' operator
		outbuff+="\tconst ";outbuff+=_Enums[i]+"& operator =(std::string copyOfStr)\n";
		outbuff+="\t{\n";
		outbuff+="\t\t// convert the string to lower case\n";
		outbuff+="\t\tfor (unsigned i=0;i<copyOfStr.size();++i)\n";
		outbuff+="\t\t\tif (copyOfStr[i]>='A' && copyOfStr[i]<='Z')\n";
		outbuff+="\t\t\t\tcopyOfStr[i]^=('A'^'a');\n";
		outbuff+="\n";

		// add values to the '=' operator
		for (j=0;j<_EnumEntries.size();++j)
		{
			if (_EnumEntries[j].substr(0,_Enums[i].size()+2)==_Enums[i]+"::")
			{
				// copy out original version of token string
				std::string s=_EnumEntries[j].substr(_Enums[i].size()+2);
				// generate lower case version of string
				std::string slower=s;
				for (unsigned k=s.size();k--;) 
					if (slower[k]>='A' && s[k]<='Z') 
						slower[k]^=('A'^'a');
				outbuff+="\t\tif (copyOfStr==\"";
				outbuff+=slower;
				outbuff+="\") {_Value=";
				outbuff+=s;
				outbuff+="; return *this;}\n";
			}
		}

		outbuff+="\n";
		outbuff+="\t\t_Value=BAD_VALUE;\n";
		outbuff+="\t\treturn *this;\n";
		outbuff+="\t}\n";
		outbuff+="\n";
		outbuff+="\t";outbuff+=_Enums[i]+"(const char* value)\n";
		outbuff+="\t{\n";
		outbuff+="\t\t*this=std::string(value);\n";
		outbuff+="\t}\n";
		outbuff+="\n";
		outbuff+="\t";outbuff+=_Enums[i]+"(const std::string &value)\n";
		outbuff+="\t{\n";
		outbuff+="\t\t*this=value;\n";
		outbuff+="\t}\n";
		outbuff+="\n";
		outbuff+="\tconst ";outbuff+=_Enums[i]+"& operator =(TValueType value)\n";
		outbuff+="\t{\n";
		outbuff+="\t\t_Value=value;\n";
		outbuff+="\t\treturn *this;\n";
		outbuff+="\t}\n";
		outbuff+="\n";
		outbuff+="\toperator TValueType() const\n";
		outbuff+="\t{\n";
		outbuff+="\t\treturn _Value;\n";
		outbuff+="\t}\n";
		outbuff+="\n";
		outbuff+="private:\n";
		outbuff+="\tTValueType _Value;\n";

		// close the enum
		outbuff+="};\n\n";
	}
}

void COutputFile::generateOutput() const
{
	// allocate a big output buffer (far bigger than the possible file size)
	std::string outbuff;

	// create the file header
	outbuff+="/*\n";
	outbuff+="\tFILE: ";
	outbuff+=_FileName+"\n\n";
	outbuff+="#ifndef RY_EGS_STATIC_BRICK_CPP_H\n";
	outbuff+="#define RY_EGS_STATIC_BRICK_CPP_H\n\n";
	outbuff+="\tWARNING: This file is autogenerated - any modifications will be lost at next regeneration\n\n";
	outbuff+="*/\n\n";

	outbuff+="#include \"nel/misc/smart_ptr.h\"\n\n";

	// generate the enum output
	generateEnumOutput(outbuff);

	// output the set of structures
	unsigned i;
	for (i=0;i<_Structures.size();++i)
	{
		_Structures[i].generateOutput(outbuff);
	}

	outbuff+="#endif\n\n";
	
	// read in the previous version of the output file
	char *inbuff=NULL;
	FILE *inf=fopen(_FileName.c_str(),"rb");
	if(inf!=NULL)
	{
		// allocate a RAM buffer for input file
		unsigned filesize=filelength(fileno(inf));
		inbuff=(char *)malloc(filesize+1);
		assert(inbuff!=NULL);

		// read the file into the RAM buffer and '0' terminate
		fread(inbuff,filesize,1,inf);
		inbuff[filesize]=0;

		// housekeeping
		fclose(inf);
	}

	// if the output has changed then write the new copy of the output file
	if (inbuff==NULL || outbuff!=inbuff)
	{
		// open the output file for writing
		FILE *outf=fopen(_FileName.c_str(),"wb");
		assert(outf!=NULL);

		// write the buffer and close the file
		fwrite(outbuff.c_str(),outbuff.size(),1,outf);
		fclose(outf);
	}

	// housekeeping
	if (inbuff!=NULL)
		free(inbuff);
}

void COutputFile::CStruct::display() const
{
	unsigned i;

	// display the filename
	printf("STRUCT: %s %s\n",_Name,_Qualifiers);

	// display the set of structures
	for (i=0;i<_Params.size();++i)
	{
		printf("=> type='%s' class='%s' name='%s' default='%s' comment='%s'\n",
			_Params[i]._Type==COutputFile::INT? "INT": _Params[i]._Type==COutputFile::STRING? "STR": "FLO",
			_Params[i]._Class.c_str(),
			_Params[i]._Name.c_str(),
			_Params[i]._DefaultValue.c_str(),
			_Params[i]._Comment.c_str());
	}
}

void COutputFile::CStruct::generateOutput(std::string &outbuff) const
{
	// open the structure
	outbuff+="struct ";
	outbuff+=_Name+" : public TBrickParam::IId\n";
	outbuff+="{\n";

	// declare structure data parameters
	unsigned i;
	for (i=0;i<_Params.size();++i)
	{
		outbuff+="\t";
		outbuff+=_Params[i]._Comment+"\n";
		outbuff+="\t";
		outbuff+=_Params[i]._Class+" "+_Params[i]._Name+";\n";
	}
	outbuff+="\n";

	// default constructor
	outbuff+="\t";
	outbuff+=_Name;
	if (_Params.size() > 0)
	{
		outbuff+="():\n";
		for (i=0;i<_Params.size();++i)
		{
			outbuff+="\t\t";
			outbuff+=_Params[i]._Name+"("+_Params[i]._DefaultValue;
			if (i != (_Params.size()-1))
				outbuff+="),\n";
			else
				outbuff+=")\n";
		}
	}
	else
	{
		outbuff+="()\n";
	}
	outbuff+="\t{\n";
	outbuff+="\t\t_Id = "+_Qualifiers+";\n";
	outbuff+="\t}\n";
	outbuff+="\n";

	// constructor from string
	outbuff+="\t";
	outbuff+=_Name;
	outbuff+="(const std::string&str)\n";
	outbuff+="\t{\n";
	outbuff+="\t\t*this=";
	outbuff+=_Name+"();\n";
	outbuff+="\t\t*this=str;\n";
	outbuff+="\t}\n";
	outbuff+="\n";

	// '=' operator	from string
	outbuff+="\tconst ";
	outbuff+=_Name+"& operator=(const std::string& input)\n";
	outbuff+="\t{\n";
	outbuff+="\t\tstd::vector<std::string> args;\n";
	outbuff+="\t\tconvertInput(args, input);\n";
	outbuff+="\n";
	outbuff+="\t\tif (args.size()!=";
	if (_Params.size()>100) outbuff+=('0'+((_Params.size()/100)%10));
	if (_Params.size()>10) outbuff+=('0'+((_Params.size()/10)%10));
	outbuff+=('0'+(_Params.size()%10));
	outbuff+=")\n";
	outbuff+="\t\t\treturn *this;\n";
	outbuff+="\n";
	outbuff+="\t\tParsedOk=true;\n";
	for (i=0;i<_Params.size();++i)
	{
		outbuff+="\t\t";
		outbuff+="NLMISC::fromString(args[";
		if (i>100) outbuff+=('0'+((i/100)%10));
		if (i>10) outbuff+=('0'+((i/10)%10));
		outbuff+=('0'+(i%10));
		outbuff+="], "+_Params[i]._Name+");\n";
	}
	outbuff+="\n";
	outbuff+="\t\treturn *this;\n";
	outbuff+="\t}\n";

	// close the struct
	outbuff+="};\n\n\n";
}


void processInputFile(const char *infile,COutputFile &outputFile)
{
	// open the input file
	FILE *inf=fopen(infile,"rb");
	assert(inf!=NULL);

	// allocate a RAM buffer for input file
	unsigned filesize=filelength(fileno(inf));
	char *inbuff=(char *)malloc(filesize+1);
	assert(inbuff!=NULL);

	// read the file into the RAM buffer and '0' terminate
	fread(inbuff,filesize,1,inf);
	inbuff[filesize]=0;

	// housekeeping
	fclose(inf);

	// scan the data read into the input buffer for special strings
	for (unsigned i=0;i<filesize;++i)
	{
		// look for our magic '$*' token
		if (inbuff[i]=='$' && inbuff[i+1]=='*')
		{
			// copy from the end of our token to end of line into a new string
			std::string s;
			for (unsigned j=i+2;j<filesize && inbuff[j]!=26 && inbuff[j]!='\n' && inbuff[j]!='\r';++j)
				s+=inbuff[j];

			// process the string
			outputFile.processString(s);
		}

		// checkout whether we have a 'case' word
		if (inbuff[i]=='c' && inbuff[i+1]=='a' && inbuff[i+2]=='s' && inbuff[i+3]=='e')
			// make sure we haven't found a 'case' in the middle of some other sub string
			if (i==0 || inbuff[i-1]==' ' || inbuff[i-1]=='\t' || inbuff[i-1]=='\n' || inbuff[i-1]=='\r' || inbuff[i-1]==':' || inbuff[i-1]==';')
			if (inbuff[i+4]==' ' || inbuff[i+4]=='\t' || inbuff[i+4]=='\n' || inbuff[i+4]=='\r')
		{
			// we've found a 'case' word followed by a blank of some sort so look for 
			// a single token followed by a lone ':'

			// start with our new index variable pointing at the second character after the word 'case'
			// note that by definition the first character after the word 'case' must have been a blank of some type
			unsigned j=i+5;

			// start by skipping blanks
			while (inbuff[j]==' '||inbuff[j]=='\t' ||inbuff[j]=='\n' ||inbuff[j]=='\r')
				++j;

			// copy out the next token into a new string
			std::string s;
			while (j<filesize-2 && (inbuff[j]!=':' || inbuff[j-1]==':' || inbuff[j+1]==':') && inbuff[j]!=' ' && inbuff[j]!='\t' && inbuff[j]!='\n' && inbuff[j]!='\r' && inbuff[j]!='/')
			{
				s+=inbuff[j++];
			}

			// skip trailing blanks
			while (inbuff[j]==' '||inbuff[j]=='\t' ||inbuff[j]=='\n' ||inbuff[j]=='\r')
				++j;

			// if we're pointing at a single ':' then we've found a plausible looking case token
			if (inbuff[j]==':' && inbuff[j+1]!=':')
				outputFile.processCase(s);
		}
	}

	// housekeeping
	free(inbuff);
}

int main(int argc, char* argv[])
{
	int i;
	for (i=1;i<argc;++i)
	{
		// sort out the pair of file names to use
		std::string srcFile=argv[i];
		std::string dstFile=srcFile+".h";

		// display a banner
		printf("\n");
		printf("===================================\n");
		printf("CRUNCHING: %s => %s\n",srcFile.c_str(),dstFile.c_str());
		printf("===================================\n\n");

		// instantiate the output buffer
		COutputFile outputFile(dstFile.c_str());

		// parse the source file
		processInputFile(srcFile.c_str(),outputFile);

		// display the list of information extracted from the source file
		outputFile.display();

		// generate the target file
		outputFile.generateOutput();
	}
	return 0;
}
