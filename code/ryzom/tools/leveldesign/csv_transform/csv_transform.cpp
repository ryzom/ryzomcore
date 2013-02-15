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

// forBoris.cpp : Defines the entry point for the console application.
//

#include <nel/misc/types_nl.h>

#include "nel/misc/sstring.h"
#include "nel/misc/file.h"
#include "sadge_lib/include/text_input.h"
#include "sadge_lib/include/text_output.h"

using namespace NLMISC;
using namespace std;

CTextOutput info(CTextOutput::ECHO_STDOUT);
CTextOutput warn(CTextOutput::ECHO_STDERR);
CTextOutput out(CTextOutput::RECORD+CTextOutput::ECHO_STDOUT);


void splitStringCSV(const CSString& in,const CSString& separator,vector<CSString>& result)
{
	bool	lastEmpty = false;
	CSString s=in.strip();
	if (s.right((uint)separator.size()) == separator)
	{
		// there is en empty column at end of line, remember this
		lastEmpty = true;
	}

	while (!s.empty())
	{
		// deal with empty columns first
		while (s.left((uint)separator.size())==separator)
		{
			s=s.leftCrop((uint)separator.size());
			result.push_back("");
		}
		if (!s.empty())
		{
			string::size_type pos = s.find(separator.c_str(), 0);
			CSString col = s.left((uint)pos);
			s=s.leftCrop((uint)pos);
			if (!col.empty() && col[0] == '\"')
			{
				result.push_back(s.firstWordOrWords(true));
			}
			else
				result.push_back(col);
			// we have a non-empty column...
/*			result.push_back(s.firstWordOrWords(true));
			while (s.left(separator.size())!=separator && !s.empty())
				result.back()+=s.firstWordOrWords(true);
*/
			s=s.leftCrop((uint)separator.size());
		}
	}
	if (lastEmpty)
		result.push_back("");
}


class CCruncher
{
private:
	map<CSString,CSString> paras;		// the map of paragraph names to paragraph bodies
	map<CSString,CSString> headers;		// the map of column names to paragraph header bodies
	map<CSString,CSString> footers;		// the map of column names to paragraph footer bodies
	map<CSString,bool> headerIsOpen;	// map of open headers...
	vector<CSString> tblColumnNames;	// the names of the table columns in a table
	vector<CSString> lastTblCols;		// the last set of table columns
	CSString tblSep;					// the table column separator
	CSString outputName;				// the name of the output file that we're building
	CSString paraName;					// the name of the para, header or footer that we're building
	CSString fileHeader;				// text to insert at start of output file
	CSString fileFooter;				// text to insert at end of output file
	CSString fileBody;					// the body text of the file
	bool firstTblLine;					// boolean used to identify the first line of a table...
	bool readColumnName;				// boolean used to identify need to read column name in first table line
	CSString tabTablePara;				// name of the paragraph for tabbed table import
	enum { DEFAULT, PARA, HEADER, FOOTER, FILE_HEADER, FILE_FOOTER, TBL, TABTBL, TEXT } mode;

public:
	CCruncher(): firstTblLine(true), readColumnName(false), mode(DEFAULT)
	{
	}

	void crunch(CTextInput& infile)
	{
		//---------------------------------------------------------------------------------------------
		// process input file contents

		while (!infile.eof())
		{
			CSString line=infile.getLine(); 

			if (line.strip().left(1)=="#")
			{
				//-------------------------------------------------------------------------------------
				// deal with keywords

				dealWithKeyword(line,infile);
			}
			else
			{
				//-------------------------------------------------------------------------------------
				// deal with non-keyword lines

				switch (mode)
				{
				case TBL:
				case TABTBL:
					addTableLine(line);
					break;

				case PARA:
					// append text to the current para
					paras[paraName]+=line+"\n";
					break;

				case HEADER:
					// append text to the current para
					headers[paraName]+=line+"\n";
					headerIsOpen[paraName]=false;
					break;

				case FILE_HEADER:
					// append text to the current para
					fileHeader+=line+"\n";
					break;

				case FILE_FOOTER:
					// append text to the current para
					fileFooter+=line+"\n";
					break;

				case FOOTER:
					// append text to the current para
					footers[paraName]+=line+"\n";
					break;
				case TEXT:
					// append the text directly to the output
					fileBody += line+"\n";
					break;
				default:
					if (!line.strip().empty())
						warn<<"ignoring line: "<<line<<"\n";
					break;
				}
			}
		}

		//---------------------------------------------------------------------------------------------
		// housekeeping

		if (outputName!="")
			flush(outputName);
	}

private:
	void closeFooters()
	{
		// deal with footer paragraphs
		for (uint32 i=(uint32)tblColumnNames.size();i--;)
		{
			if (headerIsOpen[tblColumnNames[i]])
			{
				CSString s=footers[tblColumnNames[i]];
				for (uint32 j=1;i<lastTblCols.size()&&j<tblColumnNames.size();++j)
					s=s.replace(("$"+tblColumnNames[j]+"$").c_str(),lastTblCols[j].c_str());
				fileBody+=s;
				headerIsOpen[tblColumnNames[i]]=false;
			}
		}
	}

	void flush(const CSString &filename)
	{
		closeFooters();
		CSString str = fileHeader + fileBody + fileFooter;

		NLMISC::COFile of(filename, false, true);
		of.serialBuffer((uint8*)str.data(), (uint)str.size());
/*		out<<fileHeader;
		out<<fileBody;
		out<<fileFooter;
		out.write(filename,true);
*/		fileBody.clear();
	}

	void addTableLine(CSString line)
	{
		// split the line into CSV columns
		vector<CSString> cols;
		splitStringCSV(line,tblSep,cols);

		if (!tabTablePara.empty())
		{
			// prepend the paragraph name to the columns names
			cols.insert(cols.begin(), tabTablePara);
		}

		// if not an empty line...
		if (!cols.empty())
		{
			if (!firstTblLine && !readColumnName)
			{
				// deal with footer paragraphs
				uint32 firstChange;
				for (firstChange=0;firstChange<tblColumnNames.size();++firstChange)
				{
					if (headerIsOpen.find(tblColumnNames[firstChange]) != headerIsOpen.end())
					{
						if (headerIsOpen[tblColumnNames[firstChange]])
						{
							if (firstChange>=cols.size())
								break;
							if (cols[firstChange]!=lastTblCols[firstChange])
								break;
						}
					}
				}

				for (uint32 i=(uint32)tblColumnNames.size();(i--)>firstChange;)
				{
					CSString s=footers[tblColumnNames[i]];
					for (uint32 j=1;j<lastTblCols.size()&&j<tblColumnNames.size();++j)
						s=s.replace(("$"+tblColumnNames[j]+"$").c_str(),lastTblCols[j].c_str());
					fileBody+=s;
					headerIsOpen[tblColumnNames[i]]=false;
				}

			}
			else if (!readColumnName)
			{
				firstTblLine=false;
			}

			if (!readColumnName)
			{
				// deal with header paragraphs
				for (uint32 i=0;i<tblColumnNames.size();++i)
				{
					if (i>=cols.size() 
						|| i>=lastTblCols.size() 
						|| cols[i]!=lastTblCols[i] 
						|| (cols[i]!="" && !headerIsOpen[tblColumnNames[i]]))
					{
						CSString s=headers[tblColumnNames[i]];
						for (uint32 j=1;j<cols.size()&&j<tblColumnNames.size();++j)
							s=s.replace(("$"+tblColumnNames[j]+"$").c_str(),cols[j].c_str());
						fileBody+=s;
						if (!s.empty())
							headerIsOpen[tblColumnNames[i]]=true;
					}
				}
				// lookup the para text
				CSString s=paras[cols[0]];
				// perform column substitutions
				for (uint32 i=1;i<cols.size()&&i<tblColumnNames.size();++i)
					s=s.replace(("$"+tblColumnNames[i]+"$").c_str(),cols[i].c_str());
				// output the dooberry
				fileBody+=s;

				// store away the column contents for comparisson on next iteration
				lastTblCols=cols;
			}
			else
			{
				tblColumnNames = cols;
				readColumnName=false;
			}
		}
	}

	void dealWithKeyword(CSString line,CTextInput& infile)
	{
		CSString rest=line.strip().leftCrop(1);
		CSString keyword= rest.firstWordOrWords(true).strip();
		rest= rest.strip();

		if (keyword=="include")
		{
			if (!rest.empty())
				infile.readFile(rest.c_str());
			else
				warn<<"#include requires a file name\n";
		}
		else if (keyword=="output")
		{
			if (outputName!="")
			{
				flush(outputName);
			}
			firstTblLine=true;
			outputName=rest;
		}
		else if (keyword=="para")
		{
			mode=PARA;
			closeFooters();
			paraName=rest;
			paras[paraName] = "";
		}
		else if (keyword=="header")
		{
			mode=HEADER;
			closeFooters();
			paraName=rest;
			headers[paraName] = "";
		}
		else if (keyword=="footer")
		{
			mode=FOOTER;
			closeFooters();
			paraName=rest;
			footers[paraName] = "";
		}
		else if (keyword=="file_header")
		{
			mode=FILE_HEADER;
			closeFooters();
			paraName=rest;
		}
		else if (keyword=="file_footer")
		{
			mode=FILE_FOOTER;
			closeFooters();
			paraName=rest;
		}
		else if (keyword=="tbl")
		{
			mode=TBL;
			closeFooters();
			firstTblLine=true;
			tabTablePara.clear();
			if (rest.empty())
			{
			}
			else if (rest.left(1)==",")
			{
				tblSep=',';
			}
			else if (rest.left(1)==";")
			{
				tblSep=';';
			}
			else
			{
				warn<<"failed to interpret line: "<<line<<"\n";
				mode=DEFAULT;
				closeFooters();
				return;
			}
			tblColumnNames.clear();
			splitStringCSV(line,tblSep,tblColumnNames);
		}
		else if (keyword=="tabtbl")
		{
			mode=TABTBL;
			closeFooters();
			readColumnName=true;
			firstTblLine=true;
			if (!rest.empty())
			{
				tabTablePara = rest;
			}
			else
			{
				warn<<"failed to interpret line: "<<line<<"\n";
				mode=DEFAULT;
				closeFooters();
				return;
			}
			tblSep = "\t";
			tblColumnNames.clear();
//			splitStringCSV(line,tblSep,tblColumnNames);
		}
		else if (keyword=="text")
		{
			mode=TEXT;
			closeFooters();
		}
		else if (keyword=="comment")
		{
			// do nothing
		}
		else if (keyword=="")
		{
			mode=DEFAULT;
			closeFooters();
		}
		else
		{
			warn<<"failed to interpret line: "<<line<<"\n";
			mode=DEFAULT;
			closeFooters();
		}
	}
};

int main(int argc, char* argv[])
{
	new NLMISC::CApplicationContext;

	//---------------------------------------------------------------------------------------------
	// process input args & open the input file

	if (argc!=2)
	{
		warn<<"syntax: "<<argv[0]<<" <file_name>\n";
		return 1;
	}

	CTextInput infile(argv[1]);

	if (infile.eof())
	{
		warn<<"File not found or empty: "<<argv[1]<<"\n";
		return 1;
	}

	info<<"Processing input file: "<<argv[1]<<"\n";

	CCruncher cruncher;
	cruncher.crunch(infile);

//	getch();
	return 0;
}

