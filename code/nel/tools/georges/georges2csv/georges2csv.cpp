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

/*

	Script commands:
		OUTPUT <output file name>
		FIELD <field name>
		SOURCE <field name>
		SCANFILES <extension>


*/


// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/command.h"
#include "nel/misc/common.h"
#include "nel/misc/path.h"
//#include "nel/memory/memory_manager.h"
#include "nel/misc/i18n.h"
#include "nel/misc/sstring.h"
#include "nel/misc/algo.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_dfn.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/load_form.h"

// Georges, bypassing interface
#include "nel/georges/form.h"

// Basic C++
#include <iostream>
//#include <conio.h>
#include <stdio.h>
#include <limits>
//#include <io.h>

// stl
#include <map>

using namespace NLMISC;
using namespace std;
using namespace NLGEORGES;


/*
	some handy prototypes
*/
void setOutputFile(char *);
void addField(char *);
void addSource(char *);
void scanFiles(std::string extension);
void executeScriptFile(const string &);

/*
	Some globals
*/
FILE *Outf = NULL;

class CField
{
public:
	std::string _name;
//	bool _evaluated;
	UFormElm::TEval	_evaluated;
	CField(const std::string &name, UFormElm::TEval eval) 
		: _name(name), _evaluated(eval)
	{ }
};
std::vector<CField> fields;
std::vector<std::string> files;

vector<string>		inputScriptFiles;
vector<string>		inputCsvFiles;
vector<string>		inputSheetPaths;
bool				inputSheetPathLoaded = false;
map<string, string>	inputSheetPathContent;

const char	*SEPARATOR = ";";
const char	*ARRAY_SEPARATOR = "|";


class	CDfnField
{
public:

	explicit	CDfnField	(const std::string	&name)	:	_isAnArray(false), _name(name)
	{}

	CDfnField	(const std::string	&name, const bool &isAnArray)	:	_isAnArray(isAnArray), _name(name)
	{}

	virtual	~CDfnField	()
	{}

	bool	operator	<(const CDfnField &other)	const
	{
		return	_name<other._name;
	}

	bool	operator	==(const CDfnField &other)	const
	{
		return	_name==other._name;
	}

	const	std::string	&getName		()	const
	{
		return	_name;
	}

	const	bool	&isAnArray	()	const
	{
		return	_isAnArray;
	}

private:
	bool		_isAnArray;
	std::string	_name;
};


/** Replace _false_ and _true_ with true and false
 *	this is used because excell force true and false in
 *	uppercase when is save the file in cvs mode.
 */
void replaceTrueAndFalseTagFromCsv(vector<string> &args)
{
	for (uint i=0; i<args.size(); ++i)
	{
		CSString str = args[i];

		str = str.replace("_false_", "false");
		str = str.replace("_true_", "true");

		args[i] = str;
	}
}

/** Replace false and true with _false_ and _true_
 *	this is used because excell force true and false in
 *	uppercase when is save the file in cvs mode.
 *	NB : this do the opposite jobs of the previous function
 */
void replaceTrueAndFalseTagToCsv(string &arg)
{
	CSString str = arg;

	str.replace("false", "_false_");
	str.replace("true", "_true_");

	arg = str;
}


/*
	Some routines for dealing with script input
*/
void setOutputFile(const CSString &filename)
{
	if (Outf!=NULL)
		fclose(Outf);
	Outf = nlfopen(filename.c_str(), "wt");
	if (Outf == NULL)
	{
		fprintf(stderr, "Can't open output file '%s' ! aborting.", filename.c_str());
		getchar();
		exit(1);
	}
	fields.clear();
}

void addField(const CSString &name)
{
	fields.push_back(CField(name, UFormElm::Eval));
}

void addSource(const CSString &name)
{
	fields.push_back(CField(name, UFormElm::NoEval));
}

void buildFileVector(std::vector<std::string> &filenames, const std::string &filespec)
{
	uint i,j;
	// split up the filespec into chains
	CSString filters = filespec;
	filters.strip();
	std::vector<std::string> in, out;

	while (!filters.empty())
	{
		CSString filter = filters.strtok(" \t");
		if (filter.empty())
			continue;

		switch (filter[0])
		{
		case '+':
			in.push_back(filter.leftCrop(1)); break;
			break;
		case '-':
			out.push_back(filter.leftCrop(1)); break;
			break;
		default:
			fprintf(stderr,"Error in '%s' : filter must start with '+' or '-'\n",
				filter.c_str());
			getchar(); 
			exit(1);
		}
	}

/*	for (i=0;i<filespec.size();)
	{
		for (j=i;j<filespec.size() && filespec[j]!=' ' && filespec[j]!='\t';j++) {}
		switch(filespec[i])
		{
		case '+': 
			in.push_back(filespec.substr(i+1,j-i-1)); break;
		case '-': 
			out.push_back(filespec.substr(i+1,j-i-1)); break;
		default: 
			fprintf(stderr,"Filter must start with '+' or '-'\n",&(filespec[i])); getchar(); exit(1);
		}
		i=j;
		while (i<filespec.size() && (filespec[i]==' ' || filespec[i]=='\t')) i++; // skip white space
	}
*/
	// use the filespec as a filter while we build the sheet file vector
	for (i=0;i<files.size();i++)
	{
		bool ok=true;

		// make sure the filename includes all of the include strings
		for (j=0;j<in.size() && ok;j++)
		{
			if (!testWildCard(CFile::getFilename(files[i]), in[j]))
			{
				ok=false;
			}
		}

		// make sure the filename includes none of the exclude strings
		for (j=0;j<out.size() && ok;j++)
		{
			if (testWildCard(CFile::getFilename(files[i]), out[j]))
			{
				ok=false;
			}
		}

		// if the filename matched all of the above criteria then add it to the list
		if (ok)
		{
			printf("Added: %s\n",CFile::getFilename(files[i]).c_str());
			filenames.push_back(files[i]);
		}
	}
	printf("Found: %u matching files (from %u)\n",(uint)filenames.size(),(uint)files.size());

}


void	addQuotesRoundString	(std::string	&valueString)
{
	// add quotes round strings
	std::string hold=valueString;
	valueString.erase();
	valueString='\"';
	for (uint i=0;i<hold.size();i++)
	{
		if (hold[i]=='\"')
			valueString+="\"\"";
		else
			valueString+=hold[i];
	}
	valueString+='\"';	
}

void	setErrorString	(std::string	&valueString, const UFormElm::TEval &evaluated, const UFormElm::TWhereIsValue	&where)
{
	if	(evaluated==UFormElm::NoEval)
	{
		switch(where)
		{
		case UFormElm::ValueForm:			valueString="ValueForm"; break;
		case UFormElm::ValueParentForm:		valueString="ValueParentForm"; break;
		case UFormElm::ValueDefaultDfn:		valueString="ValueDefaultDfn"; break;
		case UFormElm::ValueDefaultType:	valueString="ValueDefaultType"; break;
		default: valueString="ERR";
		}
		
	}
	else
	{
		valueString="ERR";
	}
	
}



/*
	Scanning the files ... this is the business!!
*/
void scanFiles(const CSString &filespec)
{
	std::vector<std::string> filenames;

	buildFileVector(filenames, filespec);

	// if there's no file, nothing to do
	if (filenames.empty())
		return;

	// display the table header line
	fprintf(Outf,"FILE");
	for (uint i=0;i<fields.size();i++)
		fprintf(Outf,"%s%s",SEPARATOR, fields[i]._name.c_str());
	fprintf(Outf,"\n");

	UFormLoader *formLoader = NULL;
	NLMISC::TTime last = NLMISC::CTime::getLocalTime ();
	NLMISC::TTime start = NLMISC::CTime::getLocalTime ();

	NLMISC::CSmartPtr<UForm> form;


	for (uint j = 0; j < filenames.size(); j++)
	{
		if	(NLMISC::CTime::getLocalTime () > last + 5000)
		{
			last = NLMISC::CTime::getLocalTime ();
			if	(j>0)
			{
				nlinfo ("%.0f%% completed (%d/%d), %d seconds remaining", (float)j*100.0/filenames.size(),j,filenames.size(), (filenames.size()-j)*(last-start)/j/1000);
			}

		}

		//std::string p = NLMISC::CPath::lookup (filenames[j], false, false);
		std::string p = filenames[j];
		if (p.empty()) continue;

		// create the georges loader if necessary
		if (formLoader == NULL)
		{
			WarningLog->addNegativeFilter("CFormLoader: Can't open the form file");
			formLoader = UFormLoader::createLoader ();
		}

		// Load the form with given sheet id
//		form = formLoader->loadForm (sheetIds[j].toString().c_str ());
		form = formLoader->loadForm (filenames[j].c_str ());
		if	(form)
		{
			// the form was found so read the true values from George
//			std::string s;
			fprintf(Outf,"%s",CFile::getFilenameWithoutExtension(filenames[j]).c_str());
			for	(uint i=0;i<fields.size();i++)
			{
				UFormElm::TWhereIsValue where;
				UFormElm	*fieldForm=NULL;
				std::string	valueString;
				
				form->getRootNode ().getNodeByName(&fieldForm, fields[i]._name);

				if	(fieldForm)
				{
					if	(fieldForm->isArray())	//	if its an array
					{
						uint	arraySize=0,arrayIndex=0;
						fieldForm->getArraySize(arraySize);
						while (arrayIndex<arraySize)
						{
							if	(fieldForm->getArrayValue(valueString,arrayIndex,fields[i]._evaluated, &where))
								;//addQuotesRoundString	(valueString);
							else
								setErrorString	(valueString, fields[i]._evaluated, where);

							arrayIndex++;
							if (arrayIndex<arraySize)	//	another value in the array..
								valueString+=ARRAY_SEPARATOR;
						}

					}
					else
					{
						if	(form->getRootNode ().getValueByName(valueString,fields[i]._name, fields[i]._evaluated, &where))	//fieldForm->getValue(valueString,fields[i]._evaluated))
							;//addQuotesRoundString	(valueString);
						else
							setErrorString	(valueString, fields[i]._evaluated, where);
					}
				}
//				else	//	node not found.
//				{
//					setErrorString	(valueString, fields[i]._evaluated, where);
//				}

				replaceTrueAndFalseTagToCsv(valueString);

				fprintf(Outf,"%s%s", SEPARATOR, valueString.c_str());
				
//				UFormElm::TWhereIsValue where;
//
//				bool result=form->getRootNode ().getValueByName(s,fields[i]._name, fields[i]._evaluated,&where);
//				if (!result)
//				{
//					if (fields[i]._evaluated)
//					{
//						s="ERR";
//					}
//					else
//					{
//						switch(where)
//						{
//						case UFormElm::ValueForm: s="ValueForm"; break;
//						case UFormElm::ValueParentForm: s="ValueParentForm"; break;
//						case UFormElm::ValueDefaultDfn: s="ValueDefaultDfn"; break;
//						case UFormElm::ValueDefaultType: s="ValueDefaultType"; break;
//						default: s="ERR";
//						}
//						
//					}
//					
//				}
//				else
//				{
//					// add quotes round strings
//					std::string hold=s;
//					s.erase();
//					s='\"';
//					for (uint i=0;i<hold.size();i++)
//					{
//						if (hold[i]=='\"')
//							s+="\"\"";
//						else
//							s+=hold[i];
//					}
//					s+='\"';
//				}
//				fprintf(Outf,"%s%s", SEPARATOR, s);
			}
			fprintf(Outf,"\n");
		}

	}

	// free the georges loader if necessary
	if (formLoader != NULL)
	{
		UFormLoader::releaseLoader (formLoader);
		WarningLog->removeFilter ("CFormLoader: Can't open the form file");
	}

	// housekeeping
//	sheetIds.clear ();
	filenames.clear ();

	fields.clear();
}


//void executeScriptBuf(char *txt)
void executeScriptBuf(const string &text)
{
	CSString buf = text;
	CVectorSString	lines;

	vector<string>	tmpLines;
	NLMISC::explode(std::string(buf.c_str()), std::string("\n"), tmpLines, true);
	lines.resize(tmpLines.size());
	for (uint i=0; i<tmpLines.size();i++)
	{
		lines[i]= tmpLines[i];
	}

	for (uint i=0; i<lines.size(); ++i)
	{
		CSString line = lines[i];
		line = line.strip();
		if (line.empty() || line.find("//") == 0)
		{
			// comment or empty line, skip
			continue;
		}
		CSString command = line.strtok(" \t");
		line = line.strip();

		
		if (command == "DFNPATH")
		{
			//CPath::getPathContent(args,true,false,true,files);
			CPath::addSearchPath(line, true, false); // for the dfn files
		}
		else if (command == "PATH")
		{
			files.clear();
			CPath::getPathContent(line, true,false,true,files);
			CPath::addSearchPath(line, true, false); // for the dfn files
		}
		else if (command == "OUTPUT")
		{
			setOutputFile(line);
		}
		else if (command == "FIELD")
		{
			addField(line);
		}
		else if (command == "SOURCE")
		{
			addSource(line);
		}
		else if (command == "SCANFILES")
		{
			scanFiles(line);
		}
		else if (command == "SCRIPT")
		{
			executeScriptFile(line);
		}
		else
		{
			fprintf(stderr,"Unknown command: '%s' '%s'\n", command.c_str(), line.c_str());
		}
	}

	
}

void executeScriptFile(const string &filename)
{
	ucstring	temp;
	CI18N::readTextFile(filename, temp, false, false);

	if (temp.empty())
	{
		fprintf(stderr, "the field '%s' is empty.\n", filename.c_str());
		return;
	}
	string buf = temp.toString();

	executeScriptBuf(buf);
}

void	loadSheetPath()
{
	if (inputSheetPathLoaded)
		return;

	NLMISC::createDebug();
	NLMISC::WarningLog->addNegativeFilter( "CPath::insertFileInMap" );

	vector<string> files;
	vector<string> pathsToAdd;
	for (uint i=0; i<inputSheetPaths.size(); ++i)
	{
		explode( inputSheetPaths[i], std::string("*"), pathsToAdd );
		for ( vector<string>::const_iterator ip=pathsToAdd.begin(); ip!=pathsToAdd.end(); ++ip )
		{
			CPath::addSearchPath( *ip, true, false );
			CPath::getPathContent( *ip, true, false, true, files );
		}
	}
	
	uint	i;
	for (i=0; i<files.size(); ++i)
	{
		string& filename = files[i];
//		string& filebase = CFile::getFilenameWithoutExtension(filename);
		const string& filebase = CFile::getFilename(filename);
		inputSheetPathContent[filebase] = filename;
	}

	inputSheetPathLoaded = true;
}


/*
 *
 */
void fillFromDFN( UFormLoader *formLoader, set<CDfnField>& dfnFields, UFormDfn *formDfn, const string& rootName, const string& dfnFilename )
{
	uint i;
	for ( i=0; i!=formDfn->getNumEntry(); ++i )
	{
		string entryName, rootBase;
		formDfn->getEntryName( i, entryName );
		rootBase = rootName.empty() ? "" : (rootName+".");

		UFormDfn::TEntryType entryType;
		bool array;
		formDfn->getEntryType( i, entryType, array );
		switch ( entryType )
		{
			case UFormDfn::EntryVirtualDfn:
			{
				CSmartPtr<UFormDfn> subFormDfn = formLoader->loadFormDfn( (entryName + ".dfn").c_str() );
				if ( ! subFormDfn )
					nlwarning( "Can't load virtual DFN %s", entryName.c_str() );
				else
					fillFromDFN( formLoader, dfnFields, subFormDfn, rootBase + entryName, entryName + ".dfn" );
				break;
			}
			case UFormDfn::EntryDfn:
			{
				UFormDfn *subFormDfn;
				if ( formDfn->getEntryDfn( i, &subFormDfn) )
				{
					string filename;
					formDfn->getEntryFilename( i, filename );
					fillFromDFN( formLoader, dfnFields, subFormDfn, rootBase + entryName, filename ); // recurse
				}
				break;
			}
			case UFormDfn::EntryType:
			{
				const	std::string	finalName(rootBase+entryName);
				dfnFields.insert( CDfnField(finalName, array) );
				//nlinfo( "DFN entry: %s (in %s)", (rootBase + entryName).c_str(), dfnFilename.c_str() );
				break;
			}
		}
	}
}


/*
 * Clear the form to reuse it (and all contents below node)
 */
void clearSheet( CForm *form, UFormElm* node )
{
	((CFormElm*)node)->clean();
	form->clean();
}


/*
 * - Remove CSV carriage returns.
 * - Ensure there is no non-ascii char (such as Excel's special blank crap), set them to ' '.
 */
void eraseCarriageReturnsAndMakeBlankNonAsciiChars( string& s )
{
	const char CR = '\n';
	string::size_type p = s.find( CR );
	while ( (p=s.find( CR )) != string::npos )
		s.erase( p, 1 );
	for ( p=0; p!=s.size(); ++p )
	{
		uint8& c = (uint8&)s[p]; // ensure the test is unsigned
		if ( c > 127 )
		{
			//nldebug( "Blanking bad char %u in '%s'", c, s.c_str() );
			s[p] = ' ';
		}
	}
}


string OutputPath;


/*
 * CSV -> Georges
 */
void	convertCsvFile( const string &file, bool generate, const string& sheetType )
{
	const uint			BUFFER_SIZE = 16*1024;
	char			lineBuffer[BUFFER_SIZE];

	vector<string>	fields;
	vector<string>	args;

	FILE *s = nlfopen(file, "r");
	
	if (s == NULL)
	{
		fprintf(stderr, "Can't find file %s to convert\n", file.c_str());
		return;
	}

	if (!fgets(lineBuffer, BUFFER_SIZE, s))
	{
		nlwarning("fgets() failed");
		return;
	}

	loadSheetPath();

	UFormLoader *formLoader = UFormLoader::createLoader ();
	NLMISC::CSmartPtr<CForm> form;
	NLMISC::CSmartPtr<UFormDfn> formDfn;

	explode(std::string(lineBuffer), std::string(SEPARATOR), fields);

	vector<bool> activeFields( fields.size(), true );

	// Load DFN (generation only)
	set<CDfnField>	dfnFields;
	if ( generate )
	{
		formDfn = formLoader->loadFormDfn( (sheetType + ".dfn").c_str() );
		if ( ! formDfn )
			nlerror( "Can't find DFN for %s", sheetType.c_str() );
		fillFromDFN( formLoader, dfnFields, formDfn, "", sheetType );

		// Display missing fields and check fields against DFN
		uint i;
		for ( i=1; i!=fields.size(); ++i )
		{
			eraseCarriageReturnsAndMakeBlankNonAsciiChars( fields[i] );
			if ( fields[i].empty() )
			{
				nlinfo( "Skipping field #%u (empty)", i );
				activeFields[i] = false;
			}
			else if ( nlstricmp( fields[i], "parent" ) == 0 )
			{
				fields[i] = toLower( fields[i] );
			}
			else
			{
				set<CDfnField>::iterator ist = dfnFields.find( CDfnField(fields[i]) );
				if ( ist == dfnFields.end() )
				{
					nlinfo( "Skipping field #%u (%s, not found in %s DFN)", i, fields[i].c_str(), sheetType.c_str() );
					activeFields[i] = false;
				}
			}
		}
		for ( i=1; i!=fields.size(); ++i )
		{
			if ( activeFields[i] )
				nlinfo( "Selected field: %s", fields[i].c_str() );
		}
	}

	string addExtension = "." + sheetType;
	uint dirmapLetterIndex = std::numeric_limits<uint>::max();
	bool dirmapLetterBackward = false;
	vector<string> dirmapDirs;
	string dirmapSheetCode;
	bool WriteEmptyProperties = false, WriteSheetsToDisk = true;
	bool ForceInsertParents = false;

	if ( generate )
	{
		// Get the directory mapping
		try
		{
			CConfigFile dirmapcfg;
			dirmapcfg.load( sheetType + "_dirmap.cfg" );

			if ( OutputPath.empty() )
			{
				CConfigFile::CVar *path = dirmapcfg.getVarPtr( "OutputPath" );
				if ( path )
					OutputPath = path->asString();
				if ( ! OutputPath.empty() )
				{
					if ( OutputPath[OutputPath.size()-1] != '/' )
						OutputPath += '/';
					else if ( ! CFile::isDirectory( OutputPath ) )
						nlwarning( "Output path does not exist" );
				}
			}

			CConfigFile::CVar *letterIndex1 = dirmapcfg.getVarPtr( "LetterIndex" );
			if ( letterIndex1 && letterIndex1->asInt() > 0 )
			{
				dirmapLetterIndex = letterIndex1->asInt() - 1;

				CConfigFile::CVar *letterWay = dirmapcfg.getVarPtr( "LetterWay" );
				dirmapLetterBackward = (letterWay && (letterWay->asInt() == 1));
				
				CConfigFile::CVar dirs = dirmapcfg.getVar( "Directories" );
				for ( uint idm=0; idm!=dirs.size(); ++idm )
				{
					dirmapDirs.push_back( dirs.asString( idm ) );
					nlinfo( "Directory: %s", dirmapDirs.back().c_str() );
					if ( ! CFile::isExists( OutputPath + dirmapDirs.back() ) )
					{
						CFile::createDirectory( OutputPath + dirmapDirs.back() );
					}
					else
					{
						if ( ! CFile::isDirectory( OutputPath + dirmapDirs.back() ) )
						{
							nlwarning( "Already existing but not a directory!" );
						}
					}
				}

				nlinfo( "Mapping letter #%u (%s) of sheet name to directory", dirmapLetterIndex + 1, dirmapLetterBackward?"backward":"forward" );
			}
			
			CConfigFile::CVar *sheetCode = dirmapcfg.getVarPtr( "AddSheetCode" );
			if ( sheetCode )
				dirmapSheetCode = sheetCode->asString();
			nlinfo( "Sheet code: %s", dirmapSheetCode.c_str() );

			if ( ! dirmapLetterBackward )
				dirmapLetterIndex += (uint)dirmapSheetCode.size();

			CConfigFile::CVar *wep = dirmapcfg.getVarPtr( "WriteEmptyProperties" );
			if ( wep )
				WriteEmptyProperties = (wep->asInt() == 1);
			nlinfo( "Write empty properties mode: %s", WriteEmptyProperties ? "ON" : "OFF" );

			CConfigFile::CVar *wstd = dirmapcfg.getVarPtr( "WriteSheetsToDisk" );
			if ( wstd )
				WriteSheetsToDisk = (wstd->asInt() == 1);
			nlinfo( "Write sheets to disk mode: %s", WriteSheetsToDisk ? "ON" : "OFF" );

			CConfigFile::CVar *fiparents = dirmapcfg.getVarPtr( "ForceInsertParents" );
			if ( fiparents )
				ForceInsertParents = (fiparents->asInt() == 1);
			nlinfo( "Force insert parents mode: %s", ForceInsertParents ? "ON" : "OFF" );
		}
		catch (const EConfigFile &e)
		{
			nlwarning( "Problem in directory mapping: %s", e.what() );
		}
		

		nlinfo( "Using output path: %s", OutputPath.c_str() );
		nlinfo( "Press a key to generate *.%s", sheetType.c_str() );
		getchar();
		nlinfo( "Generating...." );

	}
	else
		nlinfo("Updating modifications (only modified fields are updated)");

	set<string> newSheets;
	uint nbNewSheets = 0, nbModifiedSheets = 0, nbUnchangedSheets = 0, nbWritten = 0;
	while (!feof(s))
	{
		lineBuffer[0] = '\0';
		if (!fgets(lineBuffer, BUFFER_SIZE, s))
		{
			nlwarning("fgets() failed");
			break;
		}

		explode(std::string(lineBuffer), std::string(SEPARATOR), args);

		if (args.size() < 1)
			continue;

		eraseCarriageReturnsAndMakeBlankNonAsciiChars( args[0] );
		replaceTrueAndFalseTagFromCsv(args);

		// Skip empty lines
		if ( args[0].empty() || (args[0] == string(".")+sheetType) )
			continue;

		//nldebug( "%s: %u", args[0].c_str(), args.size() );
		string	filebase = dirmapSheetCode+args[0]; /*+"."+sheetType;*/
		if (filebase.find("."+sheetType) == string::npos)
		{
			filebase += "." + sheetType;
		}
		filebase = toLower(filebase);
		string	filename, dirbase;
		bool	isNewSheet=true;

		// Locate existing sheet
//		map<string, string>::iterator it = inputSheetPathContent.find( CFile::getFilenameWithoutExtension( filebase ) );
		map<string, string>::iterator it = inputSheetPathContent.find( CFile::getFilename( filebase ) );
		
		if	(it == inputSheetPathContent.end())
		{
			// Not found
			if ( ! generate )
			{
				if ( ! filebase.empty() )
				{
					nlwarning( "Sheet %s not found", filebase.c_str( )); 
					continue;
				}
			}
			else
			{
				// Load template sheet
				filename = toLower(filebase);
				form = (CForm*)formLoader->loadForm( (string("_empty.") + sheetType).c_str() );
				if (form == NULL)
				{
					nlerror( "Can't load sheet _empty.%s", sheetType.c_str() );
				}

				// Deduce directory from sheet name
				if ( dirmapLetterIndex != std::numeric_limits<uint>::max() )
				{
					if ( dirmapLetterIndex < filebase.size() )
					{
						uint letterIndex;
						char c;
						if ( dirmapLetterBackward )
							letterIndex = (uint)(filebase.size() - 1 - (CFile::getExtension( filebase ).size()+1)) - dirmapLetterIndex;
						else
							letterIndex = dirmapLetterIndex;
						c = tolower( filebase[letterIndex] );
						vector<string>::const_iterator idm;
						for ( idm=dirmapDirs.begin(); idm!=dirmapDirs.end(); ++idm )
						{
							if ( (! (*idm).empty()) && (tolower((*idm)[0]) == c) )
							{
								dirbase = (*idm) + "/";
								break;
							}
						}
						if ( idm==dirmapDirs.end() )
						{
							nlinfo( "Directory mapping not found for %s (index %u)", filebase.c_str(), letterIndex );
							dirbase.clear(); // put into root
						}
					}
					else
					{
						nlerror( "Can't map directory with letter #%u, greater than size of %s + code", dirmapLetterIndex, filebase.c_str() );
					}
				}

				nlinfo( "New sheet: %s", filebase.c_str() );
				++nbNewSheets;
				if ( ! newSheets.insert( filebase ).second )
					nlwarning( "Found duplicate sheet: %s", filebase.c_str() );
				isNewSheet = true;
			}
		}
		else // an existing sheet was found
		{

			// Load sheet (skip if failed)
			dirbase.clear();
			filename = (*it).second; // whole path
			form = (CForm*)formLoader->loadForm( filename.c_str() );
			if (form == NULL)
			{
				nlwarning( "Can't load sheet %s", filename.c_str() );
				continue;
			}

			isNewSheet = false;
		}

		const	UFormElm	&rootForm=form->getRootNode();
		bool displayed = false;
		bool isModified = false;
		uint i;
		for (	i=1;	i<args.size		()
					&&	i<fields.size	();	++i )
		{
			const string	&var = fields[i];
			string			&val = args[i];

			eraseCarriageReturnsAndMakeBlankNonAsciiChars( val );

			// Skip column with inactive field (empty or not in DFN)
			if ( (! activeFields[i]) )
				continue;

			// Skip setting of empty cell except if required
			if ( (! WriteEmptyProperties) && val.empty() )
				continue;

			// Special case for parent sheet
			if	(var == "parent") // already case-lowered
			{
				vector<string> parentVals;
				explode( val, std::string(ARRAY_SEPARATOR), parentVals );
				if ( (parentVals.size() == 1) && (parentVals[0].empty()) )
					parentVals.clear();

				if ( (isNewSheet || ForceInsertParents) && (! parentVals.empty()) )
				{
					// This is slow. Opti: insertParent() should have an option to do it without loading the form
					//	parent have same type that this object (postulat).
					uint	nbinsertedparents=0;

					for ( uint p=0; p!=parentVals.size(); ++p )
					{						
						string	localExtension=(parentVals[p].find(addExtension)==string::npos)?addExtension:"";
						string	parentName=parentVals[p]+localExtension;

						CSmartPtr<CForm> parentForm = (CForm*)formLoader->loadForm(CFile::getFilename(parentName.c_str()).c_str());
						if ( ! parentForm )
						{
							nlwarning( "Can't load parent form %s", parentName.c_str() );
						}
						else
						{
							form->insertParent( p, parentName.c_str(), parentForm );
							isModified=true;
							displayed = true;
							nbinsertedparents++;
						}

					}
					nlinfo( "Inserted %u parent(s)", nbinsertedparents );
				}
				// NOTE: Changing the parent is not currently implemented!
				continue;
			}

			const	UFormElm	*fieldForm=NULL;
						
			if	(rootForm.getNodeByName(&fieldForm, var))
			{
				UFormDfn	*dfnForm=const_cast<UFormElm&>(rootForm).getStructDfn();
				nlassert(dfnForm);
								
				vector<string> memberVals;
				explode( val, std::string(ARRAY_SEPARATOR), memberVals );
				uint32	memberIndex=0;
				
				while (memberIndex<memberVals.size())
				{
					const	uint	currentMemberIndex=memberIndex;
					std::string		memberVal=memberVals[memberIndex];
					memberIndex++;
					
					if	(!memberVal.empty())
					{
						if (memberVal[0] == '"')
							memberVal.erase(0, 1);
						if (memberVal.size()>0 && memberVal[memberVal.size()-1] == '"')
							memberVal.resize(memberVal.size()-1);
						
						if (memberVal == "ValueForm" ||
							memberVal == "ValueParentForm" ||
							memberVal == "ValueDefaultDfn" ||
							memberVal == "ValueDefaultType" ||
							memberVal == "ERR")
							continue;
					}


					//				nlassert(fieldDfn);
					//				virtual bool getEntryFilenameExt (uint entry, std::string &name) const = 0;
					//				virtual bool getEntryFilename (uint entry, std::string &name) const = 0;
					if (dfnForm)
					{
						string	fileName;
						string	fileNameExt;
						bool	toto=false;
						static	string	filenameTyp("filename.typ");
						string	extension;
			
						uint	fieldIndex;
						if (dfnForm->getEntryIndexByName (fieldIndex, var))	//	field exists.
						{
							dfnForm->getEntryFilename(fieldIndex,fileName);
							if (fileName==filenameTyp)
							{
								dfnForm->getEntryFilenameExt(fieldIndex,fileNameExt);
								if (	!fileNameExt.empty()
									&&	fileNameExt!="*.*")
								{
									string::size_type	index=fileNameExt.find(".");
									if (index==string::npos)	//	not found.
									{
										extension=fileNameExt;
									}
									else
									{
										extension=fileNameExt.substr(index+1);
									}
									
									if	(memberVal.find(extension)==string::npos)	// extension not found.
									{
										memberVal=NLMISC::toString("%s.%s",memberVal.c_str(),extension.c_str());
									}
									
								}
								
							}
							
						}
						
					}
					

					if	(dfnForm->isAnArrayEntryByName(var))
					{
						if	(	!isNewSheet
							&&	fieldForm!=NULL)
						{
							uint arraySize;
							const UFormElm *arrayNode = NULL;
							if (fieldForm->isArray() 
								&& fieldForm->getArraySize(arraySize) && arraySize == memberVals.size())
							{
								string	test;
								if	(	fieldForm->getArrayValue(test, currentMemberIndex)
									&&	test==memberVal	)
								{
									continue;
								}
							}
						}
						//nldebug( "%s: %s '%s'", args[0].c_str(), var.c_str(), memberVal.c_str() );
						// need to put the value at the correct index.
						const	std::string	fieldName=NLMISC::toString("%s[%u]", var.c_str(), currentMemberIndex).c_str();
						const_cast<UFormElm&>(rootForm).setValueByName(memberVal, fieldName);
						isModified=true;
						displayed = true;
					}
					else
					{
						if	(!isNewSheet)
						{
							string	test;
							if	(	rootForm.getValueByName(test, var)
								&&	test==memberVal	)
							{
								continue;
							}
							
						}					
						//nldebug( "%s: %s '%s'", args[0].c_str(), var.c_str(), memberVal.c_str() );
						const_cast<UFormElm&>(rootForm).setValueByName(memberVal, var);
						isModified=true;
						displayed = true;
					}
					
					if	(!isNewSheet)
					{
						isModified = true;
						if (!displayed)
							nlinfo("in %s:", filename.c_str());
						displayed = true;
						nlinfo("%s = %s", var.c_str(), memberVal.c_str());
					}

				}

			}
			else	//	field Node not found :\ (bad)
			{
			}
			
		}

		if ( ! isNewSheet )
		{
			if ( isModified )
				++nbModifiedSheets;
			else
				++nbUnchangedSheets;
		}

		// Write sheet
		if ( isNewSheet || displayed )
		{
			if ( WriteSheetsToDisk )
			{
				++nbWritten;
				string	path = isNewSheet ? OutputPath : "";
				string	ext = (filename.find( addExtension ) == string::npos) ? addExtension : "";
				string	absoluteFileName=path + dirbase + filename + ext;
				
//				nlinfo("opening: %s",  absoluteFileName.c_str() );
				COFile	output(absoluteFileName);
				if	(!output.isOpen())
				{
					nlinfo("creating path: %s",  (path + dirbase).c_str() );
					NLMISC::CFile::createDirectory(path + dirbase);
				}

//				nlinfo("opening2: %s",  absoluteFileName.c_str() );
				output.open	(absoluteFileName);
				
				if (!output.isOpen())
				{					
					nlinfo("ERROR! cannot create file path: %s",  absoluteFileName.c_str() );
				}
				else
				{
					form->write(output);
					output.close();
					
					if	(!CPath::exists(filename + ext))
						CPath::addSearchFile(absoluteFileName);					
				}					

			}
			clearSheet( form, &form->getRootNode() );
		}

	}
	nlinfo( "%u sheets processed (%u new, %u modified, %u unchanged - %u written)", nbNewSheets+nbModifiedSheets+nbUnchangedSheets, nbNewSheets, nbModifiedSheets, nbUnchangedSheets, nbWritten );
	UFormLoader::releaseLoader (formLoader);
}

//
void	usage(char *argv0, FILE *out)
{
	fprintf(out, "\n");
	fprintf(out, "Syntax: %s [-p <sheet path>] [-s <field_separator>] [-g <sheet type>] [-o <output path>] [<script file name> | <csv file name>]", argv0);
	fprintf(out, "(-g = generate sheet files, needs template sheet _empty.<sheet type> and <sheet type>_dirmap.cfg in the current folder");
	fprintf(out, "\n");
	fprintf(out, "Script commands:\n");
	fprintf(out, "\tDFNPATH\t\t<search path for george dfn files>\n");
	fprintf(out, "\tPATH\t\t<search path for files to scan>\n");
	fprintf(out, "\tOUTPUT\t\t<output file>\n");
	fprintf(out, "\tFIELD\t\t<field in george file>\n");
	fprintf(out, "\tSOURCE\t\t<field in george file>\n");
	fprintf(out, "\tSCANFILES\t[+<text>|-<text>[...]]\n");
	fprintf(out, "\tSCRIPT\t\t<script file to execute>\n");
	fprintf(out, "\n");
}

int main(int argc, char* argv[])
{
	bool generate = false;
	string sheetType;

	// parse command line
	uint	i;
	for (i=1; (sint)i<argc; i++)
	{
		const char	*arg = argv[i];
		if (arg[0] == '-')
		{
			switch (arg[1])
			{
			case 'p':
				++i;
				if ((sint)i == argc)
				{
					fprintf(stderr, "Missing <sheet path> after -p option\n");
					usage(argv[0], stderr);
					exit(0);
				}
				inputSheetPaths.push_back(argv[i]);
				break;
			case 's':
				++i;
				if ((sint)i == argc)
				{
					fprintf(stderr, "Missing <field_separator> after -s option\n");
					usage(argv[0], stderr);
					exit(0);
				}
				SEPARATOR = argv[i];
				break;
			case 'g':
				++i;
				if ((sint)i == argc)
				{
					fprintf(stderr, "Missing <sheetType> after -g option\n");
					usage(argv[0], stderr);
					exit(0);
				}
				generate = true;
				sheetType = string(argv[i]);
				break;
			case 'o':
				++i;
				if ((sint)i == argc)
				{
					fprintf(stderr, "Missing <output path> after -o option\n");
					usage(argv[0], stderr);
					exit(0);
				}
				OutputPath = string(argv[i]);
				break;
			default:
				fprintf(stderr, "Unrecognized option '%c'\n", arg[1]);
				usage(argv[0], stderr);
				exit(0);
				break;
			}
		}
		else
		{
			if (CFile::getExtension(arg) == "csv")
			{
				inputCsvFiles.push_back(arg);
			}
			else
			{
				inputScriptFiles.push_back(arg);
			}
		}
	}

	if (inputScriptFiles.empty() && inputCsvFiles.empty())
	{
		fprintf(stderr, "Missing input script file or csv file\n");
		usage(argv[0], stderr);
		exit(0);
	}



	for (i=0; i<inputScriptFiles.size(); ++i)
		executeScriptFile(inputScriptFiles[i]);

	for (i=0; i<inputCsvFiles.size(); ++i)
		convertCsvFile(inputCsvFiles[i], generate, sheetType);

	fprintf(stderr,"\nDone.\n");
	getchar();
	return 0;
}

