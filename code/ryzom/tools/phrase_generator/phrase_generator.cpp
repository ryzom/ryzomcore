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

// phrase_generator.cpp : Defines the entry point for the console application.

//

//-b -p r:\code\ryzom\data_leveldesign -o r:/code/ryzom/data_leveldesign/leveldesign/game_element/sphrase/magic/ magic_bricks.csv -d -m
//   -p r:\code\ryzom\data_leveldesign -o r:/code/ryzom/data_leveldesign/leveldesign/game_element/sphrase/magic/ magic_bricks.csv -d -m

#include "stdafx.h"





// Misc
#include <nel/misc/types_nl.h>
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include <nel/misc/sstring.h>
#include <nel/misc/diff_tool.h>
#include "nel/misc/algo.h"
#include "nel/misc/words_dictionary.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_dfn.h"
#include "nel/georges/u_form_loader.h"
#include "nel/georges/u_type.h"
// Georges, bypassing interface
#include "georges/stdgeorges.h"
#include "georges/form.h"
// Game share
//#include "game_share/xml.h"
// Unicode language file
// C
#include <time.h>
#include <stdio.h>
#include <conio.h>
#include <fstream>
// stl
#include <map>

#include "skill_tree.h"

using namespace NLMISC;
using namespace NLGEORGES;
using namespace std;


typedef vector<string> vs;
typedef map< string, string > mss;
typedef map< string, vs > msvs;

bool				GenerateBrickProgression = false;
bool				ProduceDocFromExistingPhrases = false;
bool				MultipleDocFiles = false;
bool				Hypertext = true;
string				PhrasePath = "r:/code/ryzom/data_leveldesign/leveldesign/game_element/sphrase/";
uint				NbSheetsGenTries = 0;
uint				NbSheetsWritten = 0;
uint				NbSheetsRead = 0;
uint				NbSheetsRejected = 0;


bool				UseBricks;

const string brSheetType = "sbrick";
const string phSheetType = "sphrase";
const string PHRASE_MAGIC_PREFIX = "abm_"; // action bricks magic (bm for brick filter)

struct CBrickInfo
{
	CBrickInfo( const string& ls="", const string& t="",
				const vs& props=vs() ) :
	LearnSkills(ls), Text(t), Props(props) {}

	string	LearnSkills;
	string	Text; // UTF-8
	vs		Props;
};


vs					OptionBricks, CounterpartBricks, AllBricks, PhrasesWithInvalidCost, InvalidPhrases;
set<string>			UsedCounterpartBricks, UsedCounterpartBrickFamiliesInPhrase;
map<string, sint>	SabrinaCosts;
map<string, sint>	PhraseSabrinaCosts;
map<string, bool>	PhraseCastable;
map<string, CBrickInfo>	BrickInfo;
mss					TextToBrick;
multimap< uint, pair<string, string> >	Progression; // phrase code, min. skill
mss					PhraseNames, PhraseTitles;
vector<vs>			ValidPhrases;
set<string>			UsedBricks, GeneratedPhrases;
map<string, vs>		GrammarMandatParamBrickFamilies, GrammarOptionCreditBrickFamilies;
UFormLoader			*FormLoader;
vector<FILE*>		PhraseDocFiles( 26 );
string				DocFileName,DocFileNameRoot;
msvs				Phrases;
map<string,uint>	SkillNameToMaxSkill;
map<string,string>	PhraseCodeToLink;
CWordsDictionary	Dico;
CStaticSkillsTree	SkillsTree;


/*
 *
 */
string				inputSheetPath;
bool				inputSheetPathLoaded = false;
map<string, string>	inputSheetPathContent; // short filename without ext, full filename with path


//-----------------------------------------------
//	getBrickTypeLetterRPos
//
//-----------------------------------------------
uint getBrickTypeLetterRPos( string& brick )
{
	/*
	uint i =0;
	while( i<brick.size() && !isdigit(brick[i]) ) 
		i++;

	nlassert(i<brick.size());

	return (brick.size() - i + 2);
	*/

	uint i =brick.size()-1;
	while( i>=0 && (isdigit(brick[i]) || brick[i]=='_') ) 
		i--;

	return (brick.size() - i + 1);

} // getBrickTypeLetterRPos //




//-----------------------------------------------
//	loadSheetPath
//
// from georges2csv
//-----------------------------------------------
void	loadSheetPath()
{
	if (inputSheetPathLoaded)
		return;

	NLMISC::createDebug();
	NLMISC::WarningLog->addNegativeFilter( "CPath::insertFileInMap" );

	CPath::addSearchPath(inputSheetPath, true, false); // for Georges to work properly

	vector<string>	files;
	CPath::getPathContent (inputSheetPath, true, false, true, files);

	uint	i;
	for (i=0; i<files.size(); ++i)
	{
		string	filename = files[i];
		string	filebase = CFile::getFilenameWithoutExtension(filename);
		if( CFile::getExtension(filename)!="saibrick" && CFile::getExtension(filename)!="saiphrase" )
		{
			inputSheetPathContent[filebase] = filename;
		}
	}

	inputSheetPathLoaded = true;

} // loadSheetPath //


/*
 *
 */
void displayList( const string& title, const vector<string>& v, CLog *log=DebugLog )
{
	if ( ! title.empty() )
		log->displayRaw( "%s: ", title.c_str() );
	vector<string>::const_iterator ist;
	for ( ist=v.begin(); ist!=v.end(); ++ist )
		log->displayRaw( "%s ", (*ist).c_str() );
	log->displayRawNL( "" );
}


/*
 *
 */
class CStrIComparator : public binary_function<string, string, bool>
{
public:
	bool	operator() ( const string& s1, const string& s2 ) const
	{
		return (nlstricmp( s1, s2 ) == 0);
	}
};


/*
 *
 */
uint getIndexFromString( const string& s, const vector<string>& v, bool displayWarning=true )
{
	if ( v.empty() )
	{
		nlwarning( "Can't find '%s' in empty array", s.c_str() );
		return ~0;
	}
	else
	{
		vector<string>::const_iterator ist = find_if( v.begin(), v.end(), bind2nd(CStrIComparator(), s) );
		if ( ist == v.end() )
		{
			if ( displayWarning )
			{
				nlwarning( "Can't find '%s' in:", s.c_str() );
				displayList( "", v, WarningLog );
			}
			return ~0;
		}
		else
			return ist - v.begin();
	}
}


//-----------------------------------------------
// Erase every carriage returns of the string
//
//-----------------------------------------------
void eraseCarriageReturns( string& s )
{
	const char CR = '\n';
	string::size_type p = s.find( CR );
	while ( (p=s.find( CR )) != string::npos )
		s.erase( p, 1 );
} //



// First param: vector of indices of columns matching wantedColumnNames
// Second param: vector of fields matching wantedColumnNames
typedef void (*TDeliveryCallback) ( mss& );


//-----------------------------------------------
// loadCSVFile
//
//-----------------------------------------------
void	loadCSVFile( const char *filename, TDeliveryCallback deliveryCallback )
{
	char lineBuffer[2048];
	FILE *file;
	const char *SEPARATOR = ";";
	vector<string> args;
	vector<string>::iterator iarg;

	if ( (file = fopen( filename, "r" )) == NULL )
	{
		nlwarning( "Can't find file %s", filename );
	}
	else
	{
		// Read first line as header with column names
		lineBuffer[0] = '\0';
		fgets( lineBuffer, 2048, file );
		explode( lineBuffer, SEPARATOR, args );

		// Store column names (and get rid of carriage returns!)
		vector < string > columnNames;
		mss valuesByName;
		for ( iarg=args.begin(); iarg!=args.end(); ++iarg )
		{
			eraseCarriageReturns( *iarg );
			columnNames.push_back( *iarg );
			valuesByName.insert( make_pair( *iarg, string("") ) );
		}

		// for each line, deliver the value of the fields
		while ( ! feof(file) )
		{
			// Get from file
			lineBuffer[0] = '\0';
			fgets( lineBuffer, 2048, file );
			explode( lineBuffer, SEPARATOR, args );

			// Set values (and get rid of carriage returns!)
			for ( iarg=args.begin(); iarg!=args.end(); ++iarg )
			{
				eraseCarriageReturns( *iarg );
				valuesByName[columnNames[iarg-args.begin()]] = *iarg;
			}

			// Deliver the wanted fields
			deliveryCallback( valuesByName );
		}
	}

} // loadCSVFile //


//set<string>	Skills;


//-----------------------------------------------
//	brickDeliveryCallback
//
// Fetch brick code and sabrina cost
// - AllBricks
// - BrickInfo
// - OptionBricks
// - CounterpartBricks
// - SabrinaCosts
//-----------------------------------------------
void brickDeliveryCallback( mss& values )
{
	string s = values["Brick_id"];
	if ( s.empty() )
	{
		s = values["FILE"];
		if ( s.empty() )
			s = values["fileName"];
	}

	string brick = CFile::getFilenameWithoutExtension( s );
	strupr( brick );
	if ( brick.empty() )
	{
		nlwarning("<brickDeliveryCallback> can't get root filename of %s",s.c_str());
		return;
	}

	string sc = values["Basics.SabrinaCost"];
	string ls = values["Basics.LearnRequiresOneOfSkills"];
	string txt = values["name"]; // TODO: only for combat
	string fmn = values["familyName"];
	string propIdent = "Basics.Property";

	if ( UseBricks )
	{
		AllBricks.push_back( brick );
		string name = (txt.empty()) ? fmn : txt;
		vs props;

		// Find all Basics.Property N (assumes they are subsequent)
		mss::const_iterator imv = values.find( propIdent + " 0" );
		for ( ; imv!=values.end(); ++imv )
		{
			const string& colName = (*imv).first;
			const string& v = (*imv).second;
			if ( colName.find( propIdent ) != string::npos )
			{
				if ( v != "NULL" && !v.empty() )
					props.push_back( v );
			}
			else
				break;
		}
		BrickInfo.insert( make_pair( brick, CBrickInfo( ls, name, props ) ) );
	}

	// Store brick in right container
	string::size_type p = brick.size() - getBrickTypeLetterRPos(brick);
	if ( ((sint)p) >= 0 )
	{
		switch ( brick[p] )
		{
		case 'O': OptionBricks.push_back( brick );
			break;
		case 'C': CounterpartBricks.push_back( brick );
			break;
		}
	}
	else
	{
		nlwarning( "Invalid brick code: %s", brick.c_str() );
		return;
	}

	// Store cost
	sint sabrinaCost;
	if ( sc.empty() )
	{
		nldebug( "No sabrina cost for %s, assuming cost 0", brick.c_str() );
		sabrinaCost = 0;
	}
	else
	{
		sabrinaCost = atoi( sc.c_str() );
	}
	SabrinaCosts.insert( make_pair( brick, sabrinaCost ) );

	/* // Quick hack to generate skill codes
	string skill = brick.substr( 1, p-1 );
	if ( ! skill.empty() )
		Skills.insert( skill );*/

} // brickDeliveryCallback //



//-----------------------------------------------
//	loadBricks
//
//-----------------------------------------------
void	loadBricks( const char* filename )
{
	loadCSVFile( filename, brickDeliveryCallback );
	if ( ProduceDocFromExistingPhrases )
		nlinfo( "Loaded %u option bricks, %u counterpart bricks, %u sabrina costs", OptionBricks.size(), CounterpartBricks.size(), SabrinaCosts.size() );
	else if ( UseBricks )
		nlinfo( "Loaded %u bricks", AllBricks.size() );

	/*set<string>::const_iterator iss;
	for ( iss=Skills.begin(); iss!=Skills.end(); ++iss )
	{
		const string& skill = (*iss);
		InfoLog->displayRawNL( "  <DEFINITION Label=\"S%s\" Value=\"S%s\"/>", skill.c_str(), skill.c_str() );
	}*/

} // loadBricks //


/*
 *
 */
string	getRootBrickForOptionOrCredit( const string& ob )
{
	// Extract brick code radix
	string::size_type p = ob.size() - getBrickTypeLetterRPos(const_cast<string&>(ob));
	if ( (ob.size() <= getBrickTypeLetterRPos(const_cast<string&>(ob))) ||
		 ((ob[p] != 'O') && (ob[p] != 'C')) )
		nlerror( "%s is not an option or credit brick", ob.c_str() );
	string radix = ob.substr( 0, p );
	
	// Append root brick suffix
	return radix + "PA01";
}


/*
 *
 */
string	getBrickFamily( const string& b )
{
	if ( b.size() >= getBrickTypeLetterRPos(const_cast<string&>(b))+2 )
	{
		string::size_type p = b.size() - getBrickTypeLetterRPos(const_cast<string&>(b));
		return b.substr( 0, p+2 );
	}
	return string();
}


/*
 *
 */
bool	isFromBrickFamily( const string& brick, const string& fam )
{
	return nlstricmp( brick.substr( 0, fam.size() ), fam ) == 0;
}


/*
 *
 */
string	getFirstBrickOfFamily( const string& family )
{
	vs::const_iterator ib;
	for ( ib=AllBricks.begin(); ib!=AllBricks.end(); ++ib )
	{
		const string& brick = *ib;
		if ( isFromBrickFamily( brick, family ) )
			return brick;
	}
	return string();
}


/*
 *
 */
vs		getAllBricksOfFamily( const string& family )
{
	vs res;
	vs::const_iterator ib;
	for ( ib=AllBricks.begin(); ib!=AllBricks.end(); ++ib )
	{
		const string& brick = *ib;
		if ( isFromBrickFamily( brick, family ) )
			res.push_back( brick );
	}
	return res;
}


/*
 *
 */
uint getCompatibleCounterpartBrickForCost( uint phraseCost, vs& phrase )
{
	//nlinfo( "Searching credit for cost %u", phraseCost );

	// Get the lowest matching counterpart brick
	uint minHigherCounterpartValue = ~0, maxLowerCounterpartValue = 0, counterpartValue;
	vs::const_iterator icb, iPerfectMatch = CounterpartBricks.end(), iMinCb = CounterpartBricks.end(), iMaxCb = CounterpartBricks.end();
	for ( icb=CounterpartBricks.begin(); icb!=CounterpartBricks.end(); ++icb)
	{
		const string& cb = *icb;

		// Skip if family already used in current phrase
		if ( UsedCounterpartBrickFamiliesInPhrase.find( getBrickFamily( cb ) ) != UsedCounterpartBrickFamiliesInPhrase.end() )
			continue;

		counterpartValue = abs( SabrinaCosts[cb] );
		//nldebug( "Trying with credit %u", counterpartValue );
		if ( counterpartValue == phraseCost )
		{
			// Perfect match, check if not already taken
			if ( UsedCounterpartBricks.insert( cb ).second )
			{
				UsedCounterpartBrickFamiliesInPhrase.insert( getBrickFamily( cb ) );
				phrase.push_back( cb );
				return counterpartValue;
			}
			else
			{
				// If already taken, we will come back to it later
				iPerfectMatch = icb;
			}
		}
		else if ( counterpartValue > phraseCost )
		{
			// Higher => get the minimum
			if ( counterpartValue < minHigherCounterpartValue )
			{
				minHigherCounterpartValue = counterpartValue;
				iMinCb = icb;
			}
		}
		else // counterpartValue < phraseCost : store the max
		{
			if ( counterpartValue >= maxLowerCounterpartValue )
			{
				maxLowerCounterpartValue = counterpartValue;
				iMaxCb = icb;
			}
		}
	}
	if ( iPerfectMatch != CounterpartBricks.end() )
	{
		// We skipped a perfect match in order to try to get a new value. But none found. Now get back to the last value.
		phrase.push_back( *iPerfectMatch );
		UsedCounterpartBrickFamiliesInPhrase.insert( getBrickFamily( *iPerfectMatch ) );
		return abs( SabrinaCosts[*iPerfectMatch] );
	}
	else if ( iMinCb == CounterpartBricks.end() )
	{
		if ( iMaxCb == CounterpartBricks.end() )
		{
			nlerror( "No matching counterpart" );
			return ~0;
		}
		else
		{
			// No phrase possible with only one (more) counterpart, try with the max and more (recurse)
			UsedCounterpartBricks.insert( *iMaxCb );
			UsedCounterpartBrickFamiliesInPhrase.insert( getBrickFamily( *iMaxCb ) );
			phrase.push_back( *iMaxCb );
			return maxLowerCounterpartValue + getCompatibleCounterpartBrickForCost( phraseCost - maxLowerCounterpartValue, phrase );
		}
	}
	else
	{
		// Phrase possible with one (more) counterpart
		UsedCounterpartBricks.insert( *iMinCb );
		UsedCounterpartBrickFamiliesInPhrase.insert( getBrickFamily( *iMinCb ) );
		phrase.push_back( *iMinCb );
		return minHigherCounterpartValue;
	}
}


/*
 *
 */
void getCompatibleCounterpartBricks( vs& phrase )
{
	// Calculate the cost of the phrase
	sint phraseCost = 0;
	string phraseStr;
	vs::const_iterator ip;
	for ( ip=phrase.begin(); ip!=phrase.end(); ++ip )
	{
		const string& brick = *ip;
		sint sabrinaCost;
		map<string, sint>::const_iterator isc = SabrinaCosts.find( brick );
		if ( isc != SabrinaCosts.end() )
			sabrinaCost = (*isc).second;
		else
			sabrinaCost = 0;
		phraseCost += sabrinaCost;
		phraseStr += brick + " ";
	}
	
	// Find matching counterpart(s), only 1 per family
	UsedCounterpartBrickFamiliesInPhrase.clear();
	uint counterpartValue = getCompatibleCounterpartBrickForCost( phraseCost, phrase );

	displayList( toString( "+%3u -%3u", phraseCost, counterpartValue ), phrase );
}


/*
 *
 */
/*void getCompatiblePhraseByCounterpart( const string& counterpartBrick, vs& phrase )
{
	sint sabrinaCost = SabrinaCosts[counterpartBrick];

	// Assuming root brick cost is zero!
	vs::const_iterator iob;
	for ( iob=OptionBricks.begin(); iob!=OptionBricks.end(); ++iob )
	{
		// TODO: Find the highest cost that is lower or equal than the counterpart value
		const string& ob = *iob;
		if ( SabrinaCosts[ob] <= SabrinaCosts[counterpartBrick] )
			break; // currently, take the first found
	}
	if ( iob != OptionBricks.end() )
	{
		string rb = getRootBrickForOptionOrCredit( *iob );
		phrase.push_back( rb );
		phrase.push_back( *iob );
		phrase.push_back( counterpartBrick );
		nldebug( "%s %s %s: +%u -%u", rb.c_str(), (*iob).c_str(), counterpartBrick.c_str(),
				 SabrinaCosts[rb]+SabrinaCosts[*iob], SabrinaCosts[counterpartBrick] );
	}
	else
		nlwarning( "No matching phrase for counterpart %s", counterpartBrick.c_str() );
}*/


/*
 * Clear the form to reuse it (and all contents below node)
 */
void clearSheet( CForm *form, UFormElm* node )
{
	((CFormElm*)node)->clean();
	form->clean();
}



/*
 *
 */
inline void	explodeBrickAndParameters( const string& brickAndParams, vs& bricks )
{
	explode( brickAndParams, " ", bricks );
}


/*
 *
 */
string getBrickType( const string& brick )
{
	if ( brick.size() < 4 )
		return "INVALID TYPE in " + brick;
	else
	{
		switch ( brick[brick.size()-getBrickTypeLetterRPos(const_cast<string&>(brick))] )
		{
		case 'P': return "Root";
			break;
		case 'E': return "Effect";
			break;
		case 'O': return "Option";
			break;
		case 'M': return "Modifier";
			break;
		case 'C': return "Credit";
			break;
		default:
			return "INVALID TYPE in " + brick;
		}
	}
}


//-----------------------------------------------
//	printBrickInfo
//
//-----------------------------------------------
void	printBrickInfo( FILE *htmlfile, const string& brick, const string& grammarError, sint& sabrinaCost, uint& minSkillValue, string& minSkill )
{
	minSkill.clear();
	string b = brick;
	strupr( b );
	string brickType = getBrickType( b );
	sint sc = (brickType=="Credit") ? -abs( SabrinaCosts[b] ) : SabrinaCosts[b];
	CBrickInfo& bInfo = BrickInfo[b];
	fprintf( htmlfile, "<LI><B>%s %s</B> %s<BR>\n", brickType.c_str(), b.c_str(), bInfo.Text.c_str() );
	if ( ! grammarError.empty() )
	{
		fprintf( htmlfile, "<FONT COLOR=\"RED\">%s</FONT><BR>\n", grammarError.c_str() );
	}
	else	
	{
		fprintf( htmlfile, "Sabrina Cost: %d <BR>\n", sc );
		if( !bInfo.LearnSkills.empty() )
		{
			fprintf( htmlfile, "Skills required: %s<BR>\n", bInfo.LearnSkills.c_str() );
		}
		if( bInfo.Props.size() )
		{
			fprintf( htmlfile, "Properties:" );
			for ( vs::const_iterator ip = bInfo.Props.begin(); ip!=bInfo.Props.end(); ++ip )
			{
				fprintf( htmlfile, " %s", (*ip) );
			}
		}
	}
	fprintf( htmlfile, "</LI>\n" );

	// Calculate sabrina cost & skill value
	sabrinaCost = sc;
	if ( bInfo.LearnSkills.empty() )
		minSkillValue = 0;
	else
	{
		minSkillValue = ~0;
		vector<string> skillsAndValues;
		explode( bInfo.LearnSkills, ":", skillsAndValues, true );
		vector<uint> skillValues( skillsAndValues.size(), ~0 );
		vector<string>::iterator isv;
		for ( isv=skillsAndValues.begin(); isv!=skillsAndValues.end(); ++isv )
		{
			const string& sav = *isv;
			string::size_type p = sav.find( ' ' );
			if ( (p == string::npos) || (sav.size() == p+1) )
				nlwarning( "Invalid LearnRequiresOneOfSkills value '%s'", sav.c_str() );
			else
			{
				uint sv = atoi( sav.substr( p+1 ).c_str() );
				skillValues[isv-skillsAndValues.begin()] = sv;
				if ( sv < minSkillValue )
					minSkillValue = sv;
			}
		}

		for ( isv=skillsAndValues.begin(); isv!=skillsAndValues.end(); ++isv )
		{
			if ( skillValues[isv-skillsAndValues.begin()] == minSkillValue )
			{
				string& sav = *isv;
				if ( (! sav.empty()) && (sav[0] != 'S') )
					sav = 'S' + sav;
				if ( minSkill.find( sav ) == string::npos )
				{
					if ( ! minSkill.empty() )
						minSkill += ", ";
					minSkill += sav;
				}
			}
		}
	}

} // printBrickInfo //


//-----------------------------------------------
//	loadBrickGrammar
//
//-----------------------------------------------
void	loadBrickGrammar()
{
	uint nbRootBricks = 0;
	vs::const_iterator ib;
	for ( ib=AllBricks.begin(); ib!=AllBricks.end(); ++ib )
	{
		string brick = *ib;
		strupr( brick );
		if ( brick.size() >= 4 )
		{
			char brickType = brick[brick.size()-getBrickTypeLetterRPos(brick)];

			/*// As the root bricks may be absent from the table, deduce them (obsolete)
			if ( brickType == 'O' )
			{
				string rootBrick = getRootBrickForOptionOrCredit( brick );
				if ( GrammarOptionCreditBrickFamilies.find( rootBrick ) == GrammarOptionCreditBrickFamilies.end() )
				{
					brick = rootBrick;
					brickType = 'P';
				}
				else
				{
					continue;
				}
			}*/

			// If not skipped by previous 'continue'
			if ( (brickType == 'P') || (brickType == 'E') || (brickType == 'O' ) ) // root, effect, option
			{
				NLMISC::CSmartPtr<CForm> form = (CForm*)FormLoader->loadForm( (strlwr(static_cast<const string&>(brick))+"."+brSheetType).c_str() );
				if ( ! form )
				{
					nlwarning( "Can't load sheet %s", ((strlwr(static_cast<const string&>(brick)))+"."+phSheetType).c_str() );
					continue;
				}
				for ( uint i=0; i!=12; ++i )
				{
					string value;
					form->getRootNode().getValueByName( value, toString( "Mandatory.f%u", i ).c_str() );
					if ( (! value.empty()) && (value != "Unknown") )
					{
						GrammarMandatParamBrickFamilies[brick].push_back( value );
					}
				}
				if ( brickType == 'O' )
				{
					for ( uint i=0; i!=4; ++i )
					{
						string value;
						form->getRootNode().getValueByName( value, toString( "Parameter.f%u", i ).c_str() );
						if ( (! value.empty()) && (value != "Unknown") )
						{
							GrammarMandatParamBrickFamilies[brick].push_back( value );
						}
					}
				}
				if ( brickType == 'P' ) // root
				{
					++nbRootBricks;
					for ( uint i=0; i!=32; ++i )
					{
						string value;
						form->getRootNode().getValueByName( value, toString( "Optional.f%u", i ).c_str() );
						if ( (! value.empty()) && (value != "Unknown") )
						{
							GrammarOptionCreditBrickFamilies[brick].push_back( value );
						}
					}
					for ( uint i=0; i!=12; ++i )
					{
						string value;
						form->getRootNode().getValueByName( value, toString( "Credit.f%u", i ).c_str() );
						if ( (! value.empty()) && (value != "Unknown") )
						{
							GrammarOptionCreditBrickFamilies[brick].push_back( value );
						}
					}
				}
			}
		}
		else
		{
			nlwarning( "Invalid brick code %s", brick.c_str() );
		}
	}
	nlinfo( "%u bricks have mandatory/parameter grammar rules", GrammarMandatParamBrickFamilies.size() );
	nlinfo( "%u bricks have option/credit grammar rules", GrammarOptionCreditBrickFamilies.size() );
	nlinfo( "Found or deduced %u root bricks", nbRootBricks );

} // loadBrickGrammar //



//-----------------------------------------------
//	loadPhraseTitles
//
//-----------------------------------------------
void	loadPhraseTitles()
{
	STRING_MANAGER::TWorksheet worksheet;
	STRING_MANAGER::loadExcelSheet( "r:/code/ryzom/translation/translated/sphrase_words_en.txt", worksheet );
	uint cp, cn;
	if ( worksheet.findCol( ucstring("sphrase ID"), cp ) && worksheet.findCol( ucstring("name"), cn ) )
	{
		for ( std::vector<STRING_MANAGER::TWorksheet::TRow>::iterator ip = worksheet.begin(); ip!=worksheet.end(); ++ip )
		{
			if ( ip == worksheet.begin() ) // skip first row
				continue;
			STRING_MANAGER::TWorksheet::TRow& row = *ip;
			PhraseTitles.insert( make_pair( strlwr(row[cp].toString()), row[cn].toUtf8() ) );
		}
	}
	else
		nlwarning( "sphrase ID or name not found" );

	nlinfo( "Loaded %u phrase titles", PhraseTitles.size() );

} // loadPhraseTitles //


//-----------------------------------------------
//	loadBrickTitles
//
//-----------------------------------------------
void	loadBrickTitles()
{
	STRING_MANAGER::TWorksheet worksheet;
	STRING_MANAGER::loadExcelSheet( "r:/code/ryzom/translation/translated/sbrick_words_en.txt", worksheet );
	uint cp, cn, nbTitles = 0;
	if ( worksheet.findCol( ucstring("sbrick ID"), cp ) && worksheet.findCol( ucstring("name"), cn ) )
	{
		for ( std::vector<STRING_MANAGER::TWorksheet::TRow>::iterator ip = worksheet.begin(); ip!=worksheet.end(); ++ip )
		{
			if ( ip == worksheet.begin() ) // skip first row
				continue;
			STRING_MANAGER::TWorksheet::TRow& row = *ip;
			BrickInfo[strupr(row[cp].toString())].Text = row[cn].toUtf8();;
			++nbTitles;
		}
	}
	else
		nlwarning( "sbrick ID or name not found" );

	nlinfo( "Loaded %u brick titles", nbTitles );

} // loadBrickTitles //


/*
 *
 */
void	getChildrenBricks( const string& brick, vs& chFamilies )
{
	chFamilies = GrammarMandatParamBrickFamilies[brick];
}


/*
 *
 */
void	addError( string& errorStr, string& newError, uint& nbGrammarErrors )
{
	if ( ! errorStr.empty() )
		errorStr += "<BR>";
	errorStr += newError;
	++nbGrammarErrors;
}


/*
 *
 */
void	checkOptionOrCreditCompatibility( string& errorStr, const string& currentBrick, const string& rootBrick, uint& nbGrammarErrors )
{
	string brick = currentBrick;
	strupr( brick );
	if ( brick.size() >= 4 && brick[1]!='C' && brick[1]!='H') // C & H for craft and harvest
	{
		char brickType = brick[brick.size()-getBrickTypeLetterRPos(brick)];
		if ( (brickType == 'O') || (brickType == 'C') )
		{
			string rootBrick = getRootBrickForOptionOrCredit( brick );
			const vs& compatibleOptionOrCredits = GrammarOptionCreditBrickFamilies[rootBrick];
			vs::const_iterator ic;
			for ( ic=compatibleOptionOrCredits.begin(); ic!=compatibleOptionOrCredits.end(); ++ic )
			{
				if ( isFromBrickFamily( brick, (*ic) ) )
					break;
			}
			if ( ic == compatibleOptionOrCredits.end() )
			{
				addError( errorStr, toString( "This family is not compatible with options/credits of root %s", rootBrick.c_str() ), nbGrammarErrors );
			}
		}
	}
}


/*
 * Preconditions:
 * - grammarErrors.size() == phrase.size()
 * - r < phrase.size()
 *
 * Note: does not check that all bricks should have a different family
 */
void	checkGrammar( const vs& phrase, uint& r, vs& grammarErrors, uint& nbGrammarErrors, const string& rootBrick, bool readNext=true )
{
	uint origR = r;
	string grammarBrick = phrase[origR];
	strupr( grammarBrick );

	// Check option/credit
	checkOptionOrCreditCompatibility( grammarErrors[r], phrase[r], rootBrick, nbGrammarErrors );

	// Check mandatory/parameter
	vs chFamilies;
	getChildrenBricks( grammarBrick, chFamilies );
	++r;
	for ( vs::const_iterator icf=chFamilies.begin(); icf!=chFamilies.end(); ++icf )
	{
		// Detect incomplete phrase
		if ( r >= phrase.size() )
		{
			addError( grammarErrors[origR], "Missing mandatory/parameter " + (*icf) + " at the end", nbGrammarErrors );
			break;
		}

		// Detect wrong brick family
		if ( isFromBrickFamily( phrase[r], (*icf) ) )
		{
			// Check grammar using child as root
			checkGrammar( phrase, r, grammarErrors, nbGrammarErrors, phrase[r], false );
		}
		else
		{
			addError( grammarErrors[r], "Error: " + (*icf) + " expected (mandatory/parameter of " + grammarBrick + ")", nbGrammarErrors );
			++r;
		}
	}

	// Next
	if ( readNext && (r < phrase.size()) )
	{
		checkGrammar( phrase, r, grammarErrors, nbGrammarErrors, rootBrick );
	}
}


/*
 *
 */
char	getDocFileLetter( const string& sheetName )
{
	// skip abm_mt_, abm_ml_...
	char letter = 'a';
	uint nbUnderscoresToSkip = 2, nbUnderscoresFound = 0;
	for ( uint c=0; c!=sheetName.size(); ++c )
	{
		if ( nbUnderscoresFound == nbUnderscoresToSkip )
		{
			letter = sheetName[c];
			break;
		}
		if ( sheetName[c] == '_' )
			++nbUnderscoresFound;
	}
	return tolower( letter );
}


//-----------------------------------------------
//	testPhraseGrammarAndProduceDoc
//
//-----------------------------------------------
bool	testPhraseGrammarAndProduceDoc( const string& sheetName, const vs& phrase )
{
	string filename = strlwr( sheetName ) + "." + phSheetType;
	string phraseStatus;
	bool isPhraseCorrect = true;
	const char *rejectedstr = "(grammatically invalid)";
	
	// Check grammar for this phrase
	vs grammarErrors( phrase.size() );
	uint nbGrammarErrors = 0, r = 0;
	checkGrammar( phrase, r, grammarErrors, nbGrammarErrors, phrase[0] );
	if ( nbGrammarErrors != 0 )
	{
		InvalidPhrases.push_back( sheetName );
		isPhraseCorrect = false;
		phraseStatus = rejectedstr;
	}
	
	// Look-up phrase title
	string phraseTitle = PhraseTitles[sheetName];

	// Output phrase description
	char letter = 'a';
	if ( (! MultipleDocFiles) && (sheetName.size() > 3)  )
	{
		letter = tolower( sheetName[3] );
	}
	else
	{
		letter = getDocFileLetter( sheetName );
	}
	if ( letter < 'a' )
		letter = 'a';
	else if ( letter > 'z' )
		letter = 'z';
	FILE *htmlFile = PhraseDocFiles[letter - 'a'];
	sint sabrinaCost;
	fprintf( htmlFile, "<A NAME=\"%s\"></A><P><B>%s</B> %s %s<BR><UL>\n", sheetName.c_str(), filename.c_str(), phraseTitle.c_str(), phraseStatus.c_str() );
	vector<string> minBrickSkills( phrase.size() );
	vector<uint> minBrickSkillValues( phrase.size(), 0 );
	string brickMinSkill, maxSkill;
	sint posCost = 0, negCost = 0, totalCost;
	uint maxSkillValue = 0, brickMinSkillValue;
	for ( uint i=0; i!=phrase.size(); ++i )
	{
		printBrickInfo( htmlFile, phrase[i], grammarErrors[i], sabrinaCost, brickMinSkillValue, brickMinSkill );
		if ( sabrinaCost > 0 )
			posCost += sabrinaCost;
		else
			negCost += sabrinaCost;
		minBrickSkillValues[i] = brickMinSkillValue;
		minBrickSkills[i] = brickMinSkill;
		if ( brickMinSkillValue > maxSkillValue )
			maxSkillValue = brickMinSkillValue;
	}
	for ( uint i=0; i!=phrase.size(); ++i )
	{
		if ( minBrickSkillValues[i] == maxSkillValue )
		{
			if ( maxSkill.find( minBrickSkills[i] ) == string::npos )
			{
				if ( ! maxSkill.empty() )
					maxSkill += "; ";
				maxSkill += minBrickSkills[i];
			}
		}
	}
	if ( phrase.size() > 1 )
	{
		string effectOrOptionBrick = phrase[1];
		strupr( effectOrOptionBrick );
		if ( ! PhraseNames.insert( make_pair( sheetName, BrickInfo[effectOrOptionBrick].Text ) ).second )
			nlwarning( "Found duplicate phrase %s", sheetName.c_str() );
	}
	Progression.insert( make_pair( maxSkillValue, make_pair( sheetName, maxSkill ) ) );
	totalCost = posCost + negCost;
	PhraseSabrinaCosts.insert( make_pair(sheetName,totalCost) );
	char *redbegin = "", *redend = "";
	if ( totalCost > 0 )
	{
		map<string,bool>::const_iterator itCastable = PhraseCastable.find(sheetName);
		if( itCastable != PhraseCastable.end() )
		{
			if( (*itCastable).second )
			{
				redbegin = "<FONT COLOR=\"RED\">";
				redend = "</FONT>";
				PhrasesWithInvalidCost.push_back( sheetName );
				isPhraseCorrect = false;
			}
		}
	}
	fprintf( htmlFile, "<LI>%s<B>Total sabrina cost: </B>+%d %d = %d%s</LI>\n", redbegin, posCost, negCost, totalCost, redend );
	fprintf( htmlFile, "<LI><B>Minimum skill value required: %d</B></LI>\n", maxSkillValue );
	fprintf( htmlFile, "</UL></P>\n" );
	if ( ! isPhraseCorrect )
	{
		++NbSheetsRejected;
	}
	return isPhraseCorrect;

} // testPhraseGrammarAndProduceDoc //





/*
 *
 */
inline bool isSeparator( char c )
{
	return (c == ' ') || (c == '\t');
}



//-----------------------------------------------
//	produceDocFromExistingPhrases
//
// - Phrases
//-----------------------------------------------
void	produceDocFromExistingPhrases()
{
	vs files;
	CPath::getPathContent( PhrasePath, true, false, true, files );

	NbSheetsRead = 0;
	for ( vs::const_iterator ip=files.begin(); ip!=files.end(); ++ip )
	{
		if ( CFile::getExtension( *ip ) == phSheetType )
		{
			// Read george sheet
			NLMISC::CSmartPtr<UForm> form = (UForm*)FormLoader->loadForm( (*ip).c_str() );
			if ( ! form )
				nlerror( "Can't load sheet %s", (*ip).c_str() );

			// Get the bricks of the phrase
			vs phrase;
			for ( uint i=0; i!=100; ++i )
			{
				string value;
				form->getRootNode().getValueByName( value, toString( "brick %u", i ).c_str() );
				if ( !value.empty() )
				{	
					strupr( value );
					phrase.push_back( CFile::getFilenameWithoutExtension( value ) );
				}
			}

			Phrases.insert( make_pair(CFile::getFilenameWithoutExtension( *ip ), phrase) );

			// look if phrase is castable
			bool castable;
			form->getRootNode().getValueByName( castable, "castable");
			PhraseCastable.insert( make_pair(CFile::getFilenameWithoutExtension( *ip ), castable) );

			// Test grammar and produce doc
			testPhraseGrammarAndProduceDoc( CFile::getFilenameWithoutExtension( *ip ), phrase );

			++NbSheetsRead;
		}
	}
	nlinfo( "Total: %u phrases", NbSheetsRead );

} // produceDocFromExistingPhrases //


/*
 *
 */
string	getLink( const string& phrase )
{
	string res;
	if ( MultipleDocFiles && (! phrase.empty()) )
	{
		res += DocFileName + "_" + getDocFileLetter( phrase ) + ".html";
	}
	else
	{
		res += DocFileName + ".html";
	}
	res += "#" + phrase;
	//nlinfo( "%s", res.c_str() );
	return res;
}


/*
 *
 */
void	usage(char *argv0, FILE *out)
{
	fprintf(out, "\n");
	fprintf(out, "Syntax: %s [-p <sheet path>] <bricksFilename> [-o <phrasePath>] [-b] [-d] [-m] [-n]\n", argv0);
	fprintf(out, "-o: output phrase path (or input if -d is set)\n");
	fprintf(out, "-b: produce doc about brick learning infos\n");
	fprintf(out, "-d: browse existing phrases in <phrasePath> (and subdirs) and produce doc\n");
	fprintf(out, "-m: multiple doc html files, alphabetically (use with -g,-c,-d with numerous phrases)\n");
	fprintf(out, "-n: no hypertext (don't produce links phrases)\n");
	
	fprintf(out, "\n");
}




//-----------------------------------------------
//	makeIndexFile
//
//-----------------------------------------------
void makeIndexFile()
{
	FILE * indexFile = fopen( ("_" + DocFileNameRoot + "_INDEX.html").c_str(), "wt" );
	if( indexFile )
	{
		fprintf( indexFile, ("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n<title>Summary of " + DocFileNameRoot + "</title>\n</head><body>\n").c_str() );
	
		DocFileName = DocFileNameRoot + "_actions";
	
		if ( MultipleDocFiles )
		{
			// One HTML file per alphabet letter
			for ( uint l=0; l!=26; ++l )
			{
				string filename = toString( "%s_%c.html", DocFileName.c_str(), 'a'+l );
				PhraseDocFiles[l] = fopen( filename.c_str(), "wt" );
				fprintf( PhraseDocFiles[l], ("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n<title>" + DocFileName + toString( " - %c", (char)('A'+l) ) + "</title>\n</head><body>\n").c_str() );
				fprintf( indexFile, ("<A HREF=\"" + filename + "\">" + (char)('A'+l) + "</A> ").c_str() );
			}
		}
		else
		{
			// One single HTML file
			fprintf( indexFile, ("<A HREF=\"" + DocFileName + ".html\">Go to action details</A>").c_str() );
			PhraseDocFiles[0] = fopen( (DocFileName + ".html").c_str(), "wt" );
			fprintf( PhraseDocFiles[0], ("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n<title>" + DocFileName + "</title>\n</head><body>\n").c_str() );
			for ( uint l=1; l!=26; ++l )
			{
				PhraseDocFiles[l] = PhraseDocFiles[0];
			}
		}
		fprintf( indexFile, ("<BR><A HREF=\"" + DocFileName + "__by_skill_value.html" + "\">Go to action by skill value</A>\n").c_str() );
		fprintf( indexFile, ("<BR><A HREF=\"" + DocFileName + "__by_skill_value_detail.html" + "\">Go to action by skill value (detail)</A>\n").c_str() );
		fprintf( indexFile, ("<BR><A HREF=\"" + DocFileName + "__by_skill.html" + "\">Go to action by skill</A><BR>\n").c_str() );

		if( GenerateBrickProgression )
		{
			fprintf( indexFile, ("<BR><BR><A HREF=\"" + DocFileNameRoot + ".html" + "\">Go to brick list</A><BR>\n").c_str() );
		}
				
		produceDocFromExistingPhrases();

		for ( map< uint, pair<string, string> >::const_iterator ip=Progression.begin(); ip!=Progression.end(); ++ip )
		{
			const string& phraseCode = (*ip).second.first;
			string link = Hypertext ? toString( "<A HREF=\"%s\">%s</A>", getLink(phraseCode).c_str(), phraseCode.c_str() ) : "<B>" + phraseCode + "</B>";
			PhraseCodeToLink.insert( make_pair(phraseCode,link) );
		}

		// Summary (errors in phrases)
		fprintf( indexFile, "<BR><A NAME=\"summary\"></A>\n" );
		fprintf( indexFile, ("<FONT SIZE=\"20\">Summary of " + DocFileName + "</FONT><BR>\n").c_str() );
		if ( NbSheetsGenTries != 0 )
			fprintf( indexFile, "<P>%u valid sheets written on %u</P>\n", NbSheetsWritten, NbSheetsGenTries );
		if ( NbSheetsRead != 0 )
			fprintf( indexFile, "<P>%u sheets read</P>\n", NbSheetsRead );
		fprintf( indexFile, "<P>%u invalid sheets rejected", NbSheetsRejected );
		if ( ! PhrasesWithInvalidCost.empty() )
		{
			fprintf( indexFile, "<P><B>Phrases with invalid sabrina cost:</B><BR>\n" );
			for ( vs::const_iterator iip=PhrasesWithInvalidCost.begin(); iip!=PhrasesWithInvalidCost.end(); ++iip )
			{
				string link = Hypertext ? toString( "<A HREF=\"%s\">%s</A>", getLink(*iip).c_str(), (*iip).c_str() ) : "<B>" + (*iip) + "</B>";
				fprintf( indexFile, "%s<BR>\n", link.c_str() );
			}
			fprintf( indexFile, "</P>\n" );
		}
		else
		{
			fprintf( indexFile, "<P><B>All phrases have valid sabrina cost.</B></P>\n" );
		}
		if ( ! InvalidPhrases.empty() )
		{
			fprintf( indexFile, "<P><B>Grammatically invalid phrases:</B><BR>\n" );
			for ( vs::const_iterator iip=InvalidPhrases.begin(); iip!=InvalidPhrases.end(); ++iip )
			{
				string link = Hypertext ? toString( "<A HREF=\"%s\">%s</A>", getLink(*iip).c_str(), (*iip).c_str() ) : "<B>" + (*iip) + "</B>";
				fprintf( indexFile, "%s<BR>\n", link.c_str() );
			}
			fprintf( indexFile, "</P>\n" );
		}
		else
		{
			fprintf( indexFile, "<P><B>All phrases are grammatically valid.</B></P>\n" );
		}
		fprintf( indexFile, "</body></html>\n" );
		fclose( indexFile );
		
		if ( MultipleDocFiles )
		{
			for ( uint l=0; l!=26; ++l )
			{
				fprintf( PhraseDocFiles[l], "</body></html>\n" );
				fclose( PhraseDocFiles[l] );
			}
		}
		else
		{
			fprintf( PhraseDocFiles[0], "</body></html>\n" );
			fclose( PhraseDocFiles[0] );
		}
	}

} // makeIndexFile //


//-----------------------------------------------
//	makeActionsBySkillGroupFile
//
//-----------------------------------------------
void makeActionsBySkillGroupFile()
{
	// progression by skill
	FILE * actionsBySkillGroupFile = fopen( (DocFileName + "__by_skill.html").c_str(), "wt" );
	if( actionsBySkillGroupFile )
	{
		fprintf( actionsBySkillGroupFile, ("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n<title>Progression of " + DocFileName + "</title>\n</head><body>\n").c_str() );
		fprintf( actionsBySkillGroupFile, "<BR><A NAME=\"by_skill_group\"></A>\n" );
		fprintf( actionsBySkillGroupFile, "<P><B>ACTIONS BY SKILL GROUP:</B><BR>\n<table cellpadding=\"0\" cellspacing=\"1\" border=\"0\" style=\"text-align: left;\"><tbody>\n" );
		map<string, multimap<uint,string> > phrasesBySkill;
		for ( map< uint, pair<string, string> >::const_iterator ip=Progression.begin(); ip!=Progression.end(); ++ip )
		{
			const string& phraseCode = (*ip).second.first;
			string skillName = (*ip).second.second.substr(0,(*ip).second.second.find(" "));
			string skillValueStr = (*ip).second.second.substr((*ip).second.second.find(" ")+1,(*ip).second.second.size()-(*ip).second.second.find(" ")-1);
			uint skillValue = atoi(skillValueStr.c_str());
			
			map<string, multimap<uint,string> >::iterator it = phrasesBySkill.find(skillName);
			if( it != phrasesBySkill.end() )
			{
				(*it).second.insert(make_pair(skillValue,phraseCode));
			}
			else
			{
				multimap<uint,string> m;
				m.insert(make_pair(skillValue,phraseCode));
				phrasesBySkill.insert( make_pair(skillName,m) );
			}
		}

		map<string, multimap<uint,string> >::iterator itPhrasesBySkill;
		for( itPhrasesBySkill = phrasesBySkill.begin(); itPhrasesBySkill != phrasesBySkill.end(); ++itPhrasesBySkill )
		{
			CVectorSString dicoResult;
			Dico.lookup( (*itPhrasesBySkill).first, dicoResult, true );
			if( !dicoResult.empty() )
				fprintf( actionsBySkillGroupFile, "<tr><td><A HREF=\"#%s\">%s</A></td></tr>\n", (*itPhrasesBySkill).first.c_str(),dicoResult[0].c_str());
			else
				fprintf( actionsBySkillGroupFile, "<tr><td><A HREF=\"#%s\">%s</A></td></tr>\n", (*itPhrasesBySkill).first.c_str(),(*itPhrasesBySkill).first.c_str());
		}
		for( itPhrasesBySkill = phrasesBySkill.begin(); itPhrasesBySkill != phrasesBySkill.end(); ++itPhrasesBySkill )
		{
			CVectorSString dicoResult;
			Dico.lookup( (*itPhrasesBySkill).first, dicoResult, true );
			if( !dicoResult.empty() )
				fprintf( actionsBySkillGroupFile, "<tr><td><A NAME=\"%s\"><B>%s</B></A><BR></td></tr>\n", (*itPhrasesBySkill).first.c_str(), dicoResult[0].c_str() );
			else
				fprintf( actionsBySkillGroupFile, "<tr><td><A NAME=\"%s\"><B>%s</B></A><BR></td></tr>\n", (*itPhrasesBySkill).first.c_str(),(*itPhrasesBySkill).first.c_str() );
			
			multimap<uint,string>::iterator it;
			for( it = (*itPhrasesBySkill).second.begin(); it != (*itPhrasesBySkill).second.end(); ++it )
			{
				fprintf( actionsBySkillGroupFile, "<tr><td>%d</td><td>%s</td><td>%s<BR></td></tr>\n", (*it).first, PhraseCodeToLink[(*it).second].c_str(), PhraseTitles[(*it).second].c_str());
			}
		}
		fprintf( actionsBySkillGroupFile, "</tbody><table></P>\n" );
		fprintf( actionsBySkillGroupFile, "</body></html>\n" );
		fclose( actionsBySkillGroupFile );
	}

} // makeActionsBySkillGroupFile //



//-----------------------------------------------
//	makeActionsBySkillValueFile
//
//-----------------------------------------------
void makeActionsBySkillValueFile()
{
	FILE * actionsBySkillValueFile = fopen( (DocFileName + "__by_skill_value.html").c_str(), "wt" );
	if( actionsBySkillValueFile )
	{
		fprintf( actionsBySkillValueFile, ("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n<title>Progression of " + DocFileName + "</title>\n</head><body>\n").c_str() );

		// Progression (phrases sorted by skill value)
		fprintf( actionsBySkillValueFile, "<BR><A NAME=\"by_skill_value\"></A>\n" );
		fprintf( actionsBySkillValueFile, "<P><B>ACTIONS BY SKILL VALUE: <A HREF=\"%s\">[detail]</A></B>\n<table cellpadding=\"0\" cellspacing=\"1\" border=\"0\" style=\"text-align: left;\"><tbody>\n",(DocFileName + "__by_skill_value_detail.html").c_str() );
		fprintf( actionsBySkillValueFile, "<tr><td><B>File</B></td><td><B>Name</B></td><td><B>Skill needed</B><BR></td></tr>\n");
		map<string,string> phraseCodeToLink;
		for ( map< uint, pair<string, string> >::const_iterator ip=Progression.begin(); ip!=Progression.end(); ++ip )
		{
			const string& phraseCode = (*ip).second.first;
			fprintf( actionsBySkillValueFile, "<tr><td><font size=2>%s</font></td><td><font size=2>%s</font></td><td><B><font size=2>%s</font></B><BR></td></tr>\n", PhraseCodeToLink[phraseCode].c_str(), /*newbrickTitle.c_str(),*/ PhraseTitles[phraseCode].c_str(), (*ip).second.second.c_str() );
		}
		fprintf( actionsBySkillValueFile, "</tbody><table></P>\n" );
		fprintf( actionsBySkillValueFile, "</body></html>\n" );
		fclose( actionsBySkillValueFile );
	}

} // makeActionsBySkillValueFile //


//-----------------------------------------------
//	makeActionsBySkillValueDetailFile
//
//-----------------------------------------------
void makeActionsBySkillValueDetailFile()
{
	FILE * actionsBySkillValueDetailFile = fopen( (DocFileName + "__by_skill_value_detail.html").c_str(), "wt" );
	if( actionsBySkillValueDetailFile )
	{
		fprintf( actionsBySkillValueDetailFile, ("<html><head>\n<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n<title>Progression of " + DocFileName + "</title>\n</head><body>\n").c_str() );

		// Progression summary (phrases sorted by skill value)
		fprintf( actionsBySkillValueDetailFile, "<BR><A NAME=\"progression\"></A>\n" );
		fprintf( actionsBySkillValueDetailFile, "<P><B>ACTIONS BY SKILL VALUE:</B><BR>\n<table cellpadding=\"0\" cellspacing=\"1\" border=\"0\" style=\"text-align: left;\"><tbody>\n" );
		fprintf( actionsBySkillValueDetailFile, "<tr><td><B>File</B></td><td><B>Name</B></td><td><B>Skill needed</B><BR></td><td><B>Sabrina cost</B></td><td><B>Bricks ...</B></td></tr>\n");
		
		set<string> effects;
		map<string,set<string> > effectAndModifiers;
		for ( map< uint, pair<string, string> >::const_iterator ip=Progression.begin(); ip!=Progression.end(); ++ip )
		{
			const string& phraseCode = (*ip).second.first;
			fprintf( actionsBySkillValueDetailFile, "<tr><td><font size=2>%s</font></td><td><font size=2>%s</font></td><td><B><font size=2>%s</font></B></td><td><font size=2>%d</font></td>",PhraseCodeToLink[phraseCode].c_str(), PhraseTitles[phraseCode].c_str(), (*ip).second.second.c_str(),PhraseSabrinaCosts[phraseCode]);
						
			msvs::iterator itPhrases = Phrases.find( phraseCode );
			if( itPhrases != Phrases.end() )
			{
				string effect;
				uint modifierCount = 0;
				uint creditCount = 0;
				for( uint i = 0; i<(*itPhrases).second.size(); ++i )
				{
					string brick = (*itPhrases).second[i];
					string color;
					switch ( brick[brick.size()-getBrickTypeLetterRPos(brick)] )
					{
					case 'P': color = "Black";
						break;
					case 'E': 
						{
							color = "Brown"; 
							effects.insert(brick);
							if( effectAndModifiers.find(brick) == effectAndModifiers.end() )
							{
								set<string> s;
								effectAndModifiers.insert( make_pair(brick,s) );
							}
							effect = brick;
						}
						break;
					case 'O': color = "Green";
						break;
					case 'M': 
						{
							color = "Blue";
							effectAndModifiers[effect].insert(brick);
							modifierCount++;
						}
						break;
					case 'C': color = "Red"; creditCount++;
						break;
					default:
						color = "Black";
					}
					string text = BrickInfo[brick].Text;
					if( text.empty() )
					{
						text = strlwr(brick);
						nlwarning("%s not found in BrickInfo",brick.c_str());
					}
					else
					{
						if(text.find("$|sap")!=-1)
						{
							text = text.substr(0,text.size()-5);
							string str = brick.substr(brick.size()-5,5);
							text += toString(atoi(str.c_str()));
						}
					}
					fprintf( actionsBySkillValueDetailFile, "<td><FONT COLOR=\"%s\" SIZE=2>%s</FONT></td>",color.c_str(),text.c_str());
				}
			}
			else
			{
				nlerror("not found : %s",phraseCode.c_str());
			}
			fprintf( actionsBySkillValueDetailFile, "</tr>\n");
		}
		fprintf( actionsBySkillValueDetailFile, "</tbody><table></P>\n" );
		fprintf( actionsBySkillValueDetailFile, "</body></html>\n" );
		fclose( actionsBySkillValueDetailFile );
	}

} // makeActionsBySkillValueDetailFile //


//-----------------------------------------------
//		validateBrick
//
//-----------------------------------------------
bool validateBrick( const string& brk )
{
	if(brk[1]=='C') return true;
	if(brk[1]=='F') return true;
	if(brk[1]=='H') return true;
	if(brk[1]=='M') return true;
	if(brk[1]=='S') return true;
	return false;

} // validateBrick //



//-----------------------------------------------
//		makeSkillTreeFile
//
//-----------------------------------------------
void makeSkillTreeFile( char filter, string skillFamily, bool withTraduction )
{
	vector<map<string,uint16> > skillsArray;
	skillsArray.resize(6); // 6 tranches de skill
	uint i;
	for( i = 0; i<SkillsTree.SkillsTree.size(); ++i )
	{
		string skillCode = SkillsTree.SkillsTree[i].SkillCode;
		if( skillCode[1] == filter )
		{
			uint sIdx = skillCode.length()-2; // -1 for 'S', -1 for 0
			skillsArray[sIdx].insert( make_pair(skillCode,SkillsTree.SkillsTree[i].MaxSkillValue) );
		}
	}
	
	uint16 maxLine = 0;
	for( i=0; i<skillsArray.size(); ++i )
	{
		if( skillsArray[i].size() > maxLine )
		{
			maxLine = skillsArray[i].size();
		}
	}
	
	string filename = skillFamily + "_skill_tree.html";
	string filenameWithTraduction = skillFamily + "_skill_tree_detailed.html";
	FILE * skillTreeFile;
	if( withTraduction )
		skillTreeFile = fopen( filenameWithTraduction.c_str(), "wt" );
	else
		skillTreeFile = fopen( filename.c_str(), "wt" );
	fprintf( skillTreeFile,"<html><head>\n");
	fprintf( skillTreeFile,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
	fprintf( skillTreeFile,"<title>SKILL TREE ( %s )</title>\n",skillFamily.c_str());
	fprintf( skillTreeFile,"</head><body>\n");
	if( withTraduction )
		fprintf( skillTreeFile,"<b>SKILL TREE ( %s )</b>\n",skillFamily.c_str());
	else
		fprintf( skillTreeFile,"<b>SKILL TREE ( %s )</b>  [<A HREF=\"%s\">display traduction</A>]<BR>\n",skillFamily.c_str(),filenameWithTraduction.c_str());
	fprintf( skillTreeFile,"<table cellpadding=\"2\" cellspacing=\"2\" border=\"0\" style=\"text-align: left;\"><tbody>\n");
	fprintf( skillTreeFile,"<tr><td><b>0 to 20</b></td><td><b>20 to 50</b></td><td><b>50 to 100</b></td><td><b>100 to 150</b></td><td><b>150 to 200</b></td><td><b>200 to 250</b></td></tr>\n");
	
	uint j;
	// print line by line
	for( j=0; j<maxLine; ++j )
	{
		fprintf( skillTreeFile,"<tr>");
		// for each column
		for( i=0; i<skillsArray.size(); ++i )
		{
			uint p;
			map<string,uint16>::iterator itSkillcode;
			for( itSkillcode = skillsArray[i].begin(), p=0; itSkillcode != skillsArray[i].end() && p<j; ++itSkillcode,++p );
			if( itSkillcode != skillsArray[i].end() )
			{
				if( withTraduction )
				{
					CVectorSString dicoResult;
					Dico.lookup( (*itSkillcode).first, dicoResult, true );
					if(dicoResult.empty())
						fprintf( skillTreeFile,"<td>%s : ???</td>",(*itSkillcode).first.c_str());
					else
						fprintf( skillTreeFile,"<td>%s</td>",dicoResult[0].c_str());
				}
				else
					fprintf( skillTreeFile,"<td>%s</td>",(*itSkillcode).first.c_str());
			}
			else
				fprintf( skillTreeFile,"<td></td>");
		}
		fprintf( skillTreeFile,"</tr>\n");
	}
	
	fprintf( skillTreeFile, "</tbody><table></P>\n" );
	fprintf( skillTreeFile, "</body></html>\n" );
	fclose( skillTreeFile );

} // makeSkillTreeFile //





//-----------------------------------------------
//		MAIN
//
//-----------------------------------------------
int main(int argc, char* argv[])
{
	// parse command line
	const char *inputFilename = NULL;
	for ( uint i=1; (sint)i!=argc; i++ )
	{
		const char	*arg = argv[i];
		if ( arg[0] == '-' )
		{
			switch ( arg[1] )
			{
			case 'p':
				++i;
				if ( (sint)i == argc )
				{
					fprintf( stderr, "Missing <sheet path> after -p option\n" );
					usage( argv[0], stderr );
					exit( 0 );
				}
				inputSheetPath = argv[i];
				break;
			case 'o':
				++i;
				if ( (sint)i == argc )
				{
					fprintf( stderr, "Missing <phrasePath> after -o option\n" );
					usage( argv[0], stderr );
					exit( 0 );
				}
				PhrasePath = argv[i];
				if ( PhrasePath[PhrasePath.size()-1] != '/' )
					PhrasePath += '/';
				break;
			case 'b' :
				GenerateBrickProgression = true;
				break;
			case 'd':
				ProduceDocFromExistingPhrases = true;
				break;
			case 'm':
				MultipleDocFiles = true;
				break;
			case 'n':
				Hypertext = false;
				break;
			}
		}
		else
		{
			if ( CFile::getExtension(arg) == "csv" )
			{
				inputFilename = arg;
			}
			else
				nlerror( "Unrecognized extension in %s", arg );

		}
	}

	Dico.init();

	loadSheetPath();
	FormLoader = UFormLoader::createLoader();
	CSheetId::init();
	
	CSheetId skillTreeSheet("skills.skill_tree");
	CSmartPtr<UForm> skillTreeForm = FormLoader->loadForm( "skills.skill_tree" );
	SkillsTree.readGeorges( skillTreeForm, skillTreeSheet );


	makeSkillTreeFile('C',"craft", false);
	makeSkillTreeFile('F',"fight", false);
	makeSkillTreeFile('H',"forage", false);
	makeSkillTreeFile('M',"magic", false);
	
	makeSkillTreeFile('C',"craft", true);
	makeSkillTreeFile('F',"fight", true);
	makeSkillTreeFile('H',"forage", true);
	makeSkillTreeFile('M',"magic", true);
	

	// Load bricks from the csv
	UseBricks = ProduceDocFromExistingPhrases;
	if ( UseBricks )
	{
		if ( ! inputFilename )
		{
			usage( argv[0], stderr );
			exit( 0 );
		}
		loadBricks( inputFilename );
	}


	// Phrases
	if ( ProduceDocFromExistingPhrases )
	{
		loadBrickGrammar();
		loadBrickTitles();
		loadPhraseTitles();

		DocFileNameRoot = toString( "%s", CFile::getFilenameWithoutExtension( inputFilename ).c_str() );

		// index
		makeIndexFile();
				
		// progression by skill
		makeActionsBySkillGroupFile();
		
		// Progression (phrases sorted by skill value)
		makeActionsBySkillValueFile();
		
		// Progression (phrases sorted by skill value + detail)
		makeActionsBySkillValueDetailFile();
		
	}

	
	if( GenerateBrickProgression )
	{
		map<uint,map<string,set<string> > > levelToBrick;

		map<string,string> phraseToSkill;

		for ( map< uint, pair<string, string> >::const_iterator ip=Progression.begin(); ip!=Progression.end(); ++ip )
		{
			const string& phraseCode = (*ip).second.first;
					
			string skillTmp = (*ip).second.second.c_str();

			phraseToSkill.insert( make_pair(phraseCode,skillTmp) );

			if(skillTmp.empty()==false)
			{
				// get skill
				string skill = skillTmp.substr(0,skillTmp.find_first_of(" "));

				// get level
				string levelStr;
				if( skillTmp.find(";") != -1 )
				{
					sint idx = skillTmp.find_first_of(" ");
					levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
				}
				else
				{
					sint idx = skillTmp.find_first_of(" ");
					levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
				}
				if(levelStr.find(".")!=-1) levelStr = levelStr.substr(0,levelStr.size()-1);
				uint level = atoi(levelStr.c_str());


				map<uint,map<string,set<string> > >::iterator itLvl = levelToBrick.find(level);
				if( itLvl == levelToBrick.end() )
				{
					set<string> s;
					map<string,set<string> > mp;
					mp.insert(make_pair(skill,s));
					levelToBrick.insert(make_pair(level,mp));
				}
				else
				{
					if( (*itLvl).second.find(skill) == (*itLvl).second.end() )
					{
						set<string> s;
						(*itLvl).second.insert( make_pair(skill,s) );
					}
				}
				
				msvs::iterator itPhrases = Phrases.find( phraseCode );
				if( itPhrases != Phrases.end() )
				{
					string effect;
					for( uint i = 0; i<(*itPhrases).second.size(); ++i )
					{
						string brick = (*itPhrases).second[i];
												
						if( levelToBrick[level][skill].find(brick) == levelToBrick[level][skill].end() )
						{
							levelToBrick[level][skill].insert(brick);
						}
					}
				}
			}
		}




		// get family & color
		map<string,string> brickToColor;
		map<string,string> brickToFamily;
		map<string, CBrickInfo>::iterator itBInf;
		for( itBInf=BrickInfo.begin(); itBInf!=BrickInfo.end(); ++itBInf )
		{
			string brk = (*itBInf).first;

			if(!validateBrick(brk)) continue;
			
			string color;
			string family;
			if( brk[brk.size()-getBrickTypeLetterRPos(brk)] =='M' )
			{
				color = "Blue";
				family = "Modifier";
			}
			if( brk[brk.size()-getBrickTypeLetterRPos(brk)] =='C' )
			{
				color = "Red";
				family = "Credit";
			}
			if( brk[brk.size()-getBrickTypeLetterRPos(brk)] =='O' )
			{
				color = "Green";
				family = "Option";
			}
			if( brk[brk.size()-getBrickTypeLetterRPos(brk)] =='P' )
			{
				color = "Black";
				family = "Root";
			}
			if( brk[brk.size()-getBrickTypeLetterRPos(brk)] =='E' )
			{
				color = "Brown";
				family = "Effect";
			}
			
			brickToColor.insert(make_pair(brk,color));
			brickToFamily.insert(make_pair(brk,family));
		}

		// get phrases where the brick can be found
		map<string,map<string,string> > brickToPhrases;
		msvs::iterator itPhrases;
		for( itPhrases=Phrases.begin(); itPhrases!=Phrases.end(); ++itPhrases )
		{
			for( uint i = 0; i<(*itPhrases).second.size(); ++i )
			{
				string brick = (*itPhrases).second[i];
				if( brickToPhrases.find(brick)==brickToPhrases.end() )
				{
					map<string,string> m;
					
					m.insert(make_pair((*itPhrases).first,phraseToSkill[(*itPhrases).first]));
					brickToPhrases.insert(make_pair(brick,m));
				}
				else
				{
					brickToPhrases[brick].insert(make_pair((*itPhrases).first,phraseToSkill[(*itPhrases).first]));
				}
			}
		}

		// get skill when a brick is learnt
		map<string,string> brickToLearnSkill;
		map<string,map<string,string> >::iterator itLearn;
		for( itLearn=brickToPhrases.begin(); itLearn!=brickToPhrases.end(); ++itLearn )
		{
			string minSkill;
			uint minLevel = 250;

			mss::iterator itPh;
			for( itPh=(*itLearn).second.begin(); itPh!=(*itLearn).second.end(); ++itPh )
			{
				string skillTmp = (*itPh).second;
				string levelStr;
				if( skillTmp.find(";") != -1 )
				{
					sint idx = skillTmp.find_first_of(" ");
					levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
				}
				else
				{
					sint idx = skillTmp.find_first_of(" ");
					levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
				}
				uint level = atoi(levelStr.c_str());

				if( level<minLevel || minSkill.empty() )
				{
					minSkill = skillTmp;
					minLevel = level;
				}
			}

			brickToLearnSkill.insert(make_pair((*itLearn).first,minSkill));
		}

		// PHRASES

		// write header and title bar
		string filename;
		filename = DocFileNameRoot + "_m.html";
		FILE * brickPhraseDocFile_m = fopen( filename.c_str(), "wt" );
		fprintf( brickPhraseDocFile_m,"<html><head>\n");
		fprintf( brickPhraseDocFile_m,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		fprintf( brickPhraseDocFile_m,"<title>Brick phrases</title>\n");
		fprintf( brickPhraseDocFile_m,"</head><body>\n");
		fprintf( brickPhraseDocFile_m,"<table cellpadding=\"0\" cellspacing=\"1\" border=\"0\" style=\"text-align: left;\"><tbody>\n");

		filename = DocFileNameRoot + "_c.html";
		FILE * brickPhraseDocFile_c = fopen( filename.c_str(), "wt" );
		fprintf( brickPhraseDocFile_c,"<html><head>\n");
		fprintf( brickPhraseDocFile_c,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		fprintf( brickPhraseDocFile_c,"<title>Brick phrases</title>\n");
		fprintf( brickPhraseDocFile_c,"</head><body>\n");
		fprintf( brickPhraseDocFile_c,"<table cellpadding=\"0\" cellspacing=\"1\" border=\"0\" style=\"text-align: left;\"><tbody>\n");

		filename = DocFileNameRoot + "_o.html";
		FILE * brickPhraseDocFile_o = fopen( filename.c_str(), "wt" );
		fprintf( brickPhraseDocFile_o,"<html><head>\n");
		fprintf( brickPhraseDocFile_o,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		fprintf( brickPhraseDocFile_o,"<title>Brick phrases</title>\n");
		fprintf( brickPhraseDocFile_o,"</head><body>\n");
		fprintf( brickPhraseDocFile_o,"<table cellpadding=\"0\" cellspacing=\"1\" border=\"0\" style=\"text-align: left;\"><tbody>\n");

		filename = DocFileNameRoot + "_p.html";
		FILE * brickPhraseDocFile_p = fopen( filename.c_str(), "wt" );
		fprintf( brickPhraseDocFile_p,"<html><head>\n");
		fprintf( brickPhraseDocFile_p,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		fprintf( brickPhraseDocFile_p,"<title>Brick phrases</title>\n");
		fprintf( brickPhraseDocFile_p,"</head><body>\n");
		fprintf( brickPhraseDocFile_p,"<table cellpadding=\"0\" cellspacing=\"1\" border=\"0\" style=\"text-align: left;\"><tbody>\n");

		filename = DocFileNameRoot + "_e.html";
		FILE * brickPhraseDocFile_e = fopen( filename.c_str(), "wt" );
		fprintf( brickPhraseDocFile_e,"<html><head>\n");
		fprintf( brickPhraseDocFile_e,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		fprintf( brickPhraseDocFile_e,"<title>Brick phrases</title>\n");
		fprintf( brickPhraseDocFile_e,"</head><body>\n");
		fprintf( brickPhraseDocFile_e,"<table cellpadding=\"0\" cellspacing=\"1\" border=\"0\" style=\"text-align: left;\"><tbody>\n");

		for( itBInf=BrickInfo.begin(); itBInf!=BrickInfo.end(); ++itBInf )
		{
			string brk = (*itBInf).first;
			
			if(!validateBrick(brk)) continue;
			
			string code = strlwr(brk.c_str());

			if(brickToFamily[brk]=="Modifier")
				fprintf( brickPhraseDocFile_m,"<tr><td><A NAME=\"%s\"><FONT COLOR=\"Blue\">%s</FONT></A></td><td></td></tr>\n",brk.c_str(),code.c_str());
			if(brickToFamily[brk]=="Credit")
				fprintf( brickPhraseDocFile_c,"<tr><td><A NAME=\"%s\"><FONT COLOR=\"Red\">%s</FONT></A></td><td></td></tr>\n",brk.c_str(),code.c_str());
			if(brickToFamily[brk]=="Option")
				fprintf( brickPhraseDocFile_o,"<tr><td><A NAME=\"%s\"><FONT COLOR=\"Green\">%s</FONT></A></td><td></td></tr>\n",brk.c_str(),code.c_str());
			if(brickToFamily[brk]=="Effect")
				fprintf( brickPhraseDocFile_e,"<tr><td><A NAME=\"%s\"><FONT COLOR=\"Brown\">%s</FONT></A></td><td></td></tr>\n",brk.c_str(),code.c_str());
			if(brickToFamily[brk]=="Root")
				fprintf( brickPhraseDocFile_p,"<tr><td><A NAME=\"%s\"><FONT COLOR=\"Black\">%s</FONT></A></td><td></td></tr>\n",brk.c_str(),code.c_str());
			
			map<string,map<string,string> >::iterator itPhrases = brickToPhrases.find(brk);
			if( itPhrases != brickToPhrases.end() )
			{
				map<string,string>::iterator itPh;
				for( itPh=(*itPhrases).second.begin(); itPh!=(*itPhrases).second.end(); ++itPh )
				{
					if(brickToFamily[brk]=="Modifier")
						fprintf( brickPhraseDocFile_m,"<tr><td></td><td><A HREF=\"%s_%c.html#%s\">%s</A></td></tr>\n",DocFileName.c_str(),(*itPh).first[7],(*itPh).first.c_str(),(*itPh).first.c_str());
					if(brickToFamily[brk]=="Credit")
						fprintf( brickPhraseDocFile_c,"<tr><td></td><td><A HREF=\"%s_%c.html#%s\">%s</A></td></tr>\n",DocFileName.c_str(),(*itPh).first[7],(*itPh).first.c_str(),(*itPh).first.c_str());
					if(brickToFamily[brk]=="Option")
						fprintf( brickPhraseDocFile_o,"<tr><td></td><td><A HREF=\"%s_%c.html#%s\">%s</A></td></tr>\n",DocFileName.c_str(),(*itPh).first[7],(*itPh).first.c_str(),(*itPh).first.c_str());
					if(brickToFamily[brk]=="Effect")
						fprintf( brickPhraseDocFile_e,"<tr><td></td><td><A HREF=\"%s_%c.html#%s\">%s</A></td></tr>\n",DocFileName.c_str(),(*itPh).first[7],(*itPh).first.c_str(),(*itPh).first.c_str());
					if(brickToFamily[brk]=="Root")
						fprintf( brickPhraseDocFile_p,"<tr><td></td><td><A HREF=\"%s_%c.html#%s\">%s</A></td></tr>\n",DocFileName.c_str(),(*itPh).first[7],(*itPh).first.c_str(),(*itPh).first.c_str());
				}
			}
		}
		fprintf( brickPhraseDocFile_m, "</tbody><table></P>\n" );
		fprintf( brickPhraseDocFile_m, "</body></html>\n" );
		fclose( brickPhraseDocFile_m );

		fprintf( brickPhraseDocFile_c, "</tbody><table></P>\n" );
		fprintf( brickPhraseDocFile_c, "</body></html>\n" );
		fclose( brickPhraseDocFile_c );

		fprintf( brickPhraseDocFile_o, "</tbody><table></P>\n" );
		fprintf( brickPhraseDocFile_o, "</body></html>\n" );
		fclose( brickPhraseDocFile_o );

		fprintf( brickPhraseDocFile_e, "</tbody><table></P>\n" );
		fprintf( brickPhraseDocFile_e, "</body></html>\n" );
		fclose( brickPhraseDocFile_e );

		fprintf( brickPhraseDocFile_p, "</tbody><table></P>\n" );
		fprintf( brickPhraseDocFile_p, "</body></html>\n" );
		fclose( brickPhraseDocFile_p );


		// CODE

		// write header and title bar
		filename = DocFileNameRoot + ".html";
		FILE * brickDocFile = fopen( filename.c_str(), "wt" );
		fprintf( brickDocFile,"<html><head>\n");
		fprintf( brickDocFile,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		fprintf( brickDocFile,"<title>Bricks infos</title>\n");
		fprintf( brickDocFile,"</head><body>\n");
		fprintf( brickDocFile,"<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n");
		fprintf( brickDocFile,"<tr>\n");
		fprintf( brickDocFile,"<td><b>*Code*</b></td>\n");
		fprintf( brickDocFile,"<td><b><a href=\"%s_name.html\">Name</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickDocFile,"<td><b><a href=\"%s_family.html\">Family</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickDocFile,"<td><b>Required Skill Name</b></td>\n");
		fprintf( brickDocFile,"<td><b><a href=\"%s_required_skill_value.html\">Required Skill Value</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickDocFile,"<td><b>Learn Skill Name</b></td>\n");
		fprintf( brickDocFile,"<td><b><a href=\"%s_learn_skill_value.html\">Learn Skill Value</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickDocFile,"<td><b>Found In Phrases</b></td>\n");
		fprintf( brickDocFile,"</tr>\n");


		// write infos
		for( itBInf=BrickInfo.begin(); itBInf!=BrickInfo.end(); ++itBInf )
		{
			string brk = (*itBInf).first;

			if(!validateBrick(brk)) continue;
			
			string skillTmp = (*itBInf).second.LearnSkills;
			string skill = skillTmp.substr(0,skillTmp.find_first_of(" "));
			CVectorSString dicoResult;
			Dico.lookup( skill, dicoResult, true );
			if(dicoResult.empty()) continue;
			
			// color
			string color = brickToColor[brk];

			// code
			string code = strlwr(brk.c_str());
			fprintf( brickDocFile, "<tr><td><FONT COLOR=\"%s\">%s</FONT></td>\n",color.c_str(),code.c_str());
			
			// name
			string name = (*itBInf).second.Text;
			fprintf( brickDocFile, "<td>%s</td>\n",name.c_str());

			// family
			string family = brickToFamily[brk];
			fprintf( brickDocFile, "<td><FONT COLOR=\"%s\">%s</FONT></td>\n",color.c_str(),family.c_str());

			// required skill name
			fprintf( brickDocFile, "<td>%s</td>\n",dicoResult[0].c_str());
			
			// required skill value
			string levelStr;
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			fprintf( brickDocFile, "<td>%s</td>\n",levelStr.c_str());

			// learn skill name
			string learnSkillTmp = brickToLearnSkill[brk];
			skill = learnSkillTmp.substr(0,learnSkillTmp.find_first_of(" "));
			fprintf( brickDocFile, "<td>%s</td>\n",skill.c_str());

			// learn skill value
			if( learnSkillTmp.find(";") != -1 )
			{
				sint idx = learnSkillTmp.find_first_of(" ");
				levelStr = learnSkillTmp.substr(idx+1,learnSkillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = learnSkillTmp.find_first_of(" ");
				levelStr = learnSkillTmp.substr(idx+1,learnSkillTmp.size()-idx);
			}
			fprintf( brickDocFile, "<td>%s</td>\n",levelStr.c_str());


			// phrase list
			fprintf( brickDocFile, "<td>");
			map<string,map<string,string> >::iterator itPhrases = brickToPhrases.find(brk);
			if( itPhrases != brickToPhrases.end() )
			{
				map<string,string>::iterator itPh;
				uint i;
				for( itPh=(*itPhrases).second.begin(),i=0; itPh!=(*itPhrases).second.end() && i<2; ++itPh,++i )
				{
					if( MultipleDocFiles )
						fprintf( brickDocFile,"<A HREF=\"%s_%c.html#%s\">%s</A>,&nbsp&nbsp",DocFileName.c_str(),(*itPh).first[7],(*itPh).first.c_str(),(*itPh).first.c_str());
					else
						fprintf( brickDocFile,"<A HREF=\"%s.html#%s\">%s</A>,&nbsp&nbsp",DocFileName.c_str(),(*itPh).first.c_str(),(*itPh).first.c_str());
				}
				if( i==2 )
				{
					char type = family[0];
					fprintf( brickDocFile,"[<A HREF=\"%s_%c.html#%s\">...</A>]",DocFileNameRoot.c_str(),type,brk.c_str());
				}
			}

			fprintf( brickDocFile, "</td></tr>\n");
		}
		fprintf( brickDocFile, "</tbody><table></P>\n" );
		fprintf( brickDocFile, "</body></html>\n" );
		fclose( brickDocFile );



		// NAME

		// write header and title bar
		filename = DocFileNameRoot + "_name.html";
		FILE * brickNameDocFile = fopen( filename.c_str(), "wt" );
		fprintf( brickNameDocFile,"<html><head>\n");
		fprintf( brickNameDocFile,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		fprintf( brickNameDocFile,"<title>Bricks infos</title>\n");
		fprintf( brickNameDocFile,"</head><body>\n");
		fprintf( brickNameDocFile,"<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n");
		fprintf( brickNameDocFile,"<tr>\n");
		fprintf( brickNameDocFile,"<td><b><a href=\"%s.html\">Code</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickNameDocFile,"<td><b>*Name*</b></td>\n");
		fprintf( brickNameDocFile,"<td><b><a href=\"%s_family.html\">Family</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickNameDocFile,"<td><b>Required Skill Name</b></td>\n");
		fprintf( brickNameDocFile,"<td><b><a href=\"%s_required_skill_value.html\">Required Skill Value</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickNameDocFile,"<td><b>Learn Skill Name</b></td>\n");
		fprintf( brickNameDocFile,"<td><b><a href=\"%s_learn_skill_value.html\">Learn Skill Value</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickNameDocFile,"<td><b>Found In Phrases</b></td>\n");
		fprintf( brickNameDocFile,"</tr>\n");

		map<string,string> nameToCode;
		for( itBInf=BrickInfo.begin(); itBInf!=BrickInfo.end(); ++itBInf )
		{
			string brk = (*itBInf).first;
			
			if(!validateBrick(brk)) continue;
						
			// code
			string code = strlwr(brk.c_str());
				
			// name
			string name = (*itBInf).second.Text;
			if( !name.empty())
				nameToCode.insert( make_pair(name,brk) );
		}

		mss::iterator itNTC;
		for( itNTC=nameToCode.begin(); itNTC!=nameToCode.end(); ++itNTC )
		{
			itBInf=BrickInfo.find((*itNTC).second);

			string brk = (*itBInf).first;
			
			if(!validateBrick(brk)) continue;
			
			string skillTmp = (*itBInf).second.LearnSkills;
			string skill = skillTmp.substr(0,skillTmp.find_first_of(" "));
			CVectorSString dicoResult;
			Dico.lookup( skill, dicoResult, true );
			if(dicoResult.empty()) continue;
			
			// color
			string color = brickToColor[brk];
			
			// code
			string code = strlwr(brk.c_str());
			fprintf( brickNameDocFile, "<tr><td><FONT COLOR=\"%s\">%s</FONT></td>\n",color.c_str(),code.c_str());

			// name
			string name = (*itBInf).second.Text;
			fprintf( brickNameDocFile, "<td>%s</td>\n",name.c_str());
				
			// family
			string family = brickToFamily[brk];
			fprintf( brickNameDocFile, "<td><FONT COLOR=\"%s\">%s</FONT></td>\n",color.c_str(),family.c_str());
			
			// required skill name
			fprintf( brickNameDocFile, "<td>%s</td>\n",dicoResult[0].c_str());
				
			// required skill value
			string levelStr;
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			fprintf( brickNameDocFile, "<td>%s</td>\n",levelStr.c_str());
			
			// learn skill name
			skillTmp = brickToLearnSkill[brk];
			skill = skillTmp.substr(0,skillTmp.find_first_of(" "));
			fprintf( brickNameDocFile, "<td>%s</td>\n",skill.c_str());
			
			// learn skill value
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			fprintf( brickNameDocFile, "<td>%s</td>\n",levelStr.c_str());
			
			
			// phrase list
			fprintf( brickNameDocFile, "<td>");
			map<string,map<string,string> >::iterator itPhrases = brickToPhrases.find(brk);
			if( itPhrases != brickToPhrases.end() )
			{
				map<string,string>::iterator itPh;
				uint i;
				for( itPh=(*itPhrases).second.begin(),i=0; itPh!=(*itPhrases).second.end() && i<2; ++itPh,++i )
				{
					if( MultipleDocFiles )
						fprintf( brickNameDocFile,"<A HREF=\"%s_%c.html#%s\">%s</A>,&nbsp&nbsp",DocFileName.c_str(),(*itPh).first[7],(*itPh).first.c_str(),(*itPh).first.c_str());
					else
						fprintf( brickNameDocFile,"<A HREF=\"%s.html#%s\">%s</A>,&nbsp&nbsp",DocFileName.c_str(),(*itPh).first.c_str(),(*itPh).first.c_str());
				}
				if( i==2 )
				{
					char type = family[0];
					fprintf( brickNameDocFile,"[<A HREF=\"%s_%c.html#%s\">...</A>]",DocFileNameRoot.c_str(),type,brk.c_str());
				}
			}
			
			fprintf( brickNameDocFile, "</td></tr>\n");

		}

		fprintf( brickNameDocFile, "</tbody><table></P>\n" );
		fprintf( brickNameDocFile, "</body></html>\n" );
		fclose( brickNameDocFile );




		// FAMILY

		// write header and title bar
		filename = DocFileNameRoot + "_family.html";
		FILE * brickFamilyDocFile = fopen( filename.c_str(), "wt" );
		fprintf( brickFamilyDocFile,"<html><head>\n");
		fprintf( brickFamilyDocFile,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		fprintf( brickFamilyDocFile,"<title>Bricks infos</title>\n");
		fprintf( brickFamilyDocFile,"</head><body>\n");
		fprintf( brickFamilyDocFile,"<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n");
		fprintf( brickFamilyDocFile,"<tr>\n");
		fprintf( brickFamilyDocFile,"<td><b><a href=\"%s.html\">Code</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickFamilyDocFile,"<td><b><a href=\"%s_name.html\">Name</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickFamilyDocFile,"<td><b>*Family*</b></td>\n");
		fprintf( brickFamilyDocFile,"<td><b>Required Skill Name</b></td>\n");
		fprintf( brickFamilyDocFile,"<td><b><a href=\"%s_required_skill_value.html\">Required Skill Value</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickFamilyDocFile,"<td><b>Learn Skill Name</b></td>\n");
		fprintf( brickFamilyDocFile,"<td><b><a href=\"%s_learn_skill_value.html\">Learn Skill Value</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickFamilyDocFile,"<td><b>Found In Phrases</b></td>\n");
		fprintf( brickFamilyDocFile,"</tr>\n");


		// write infos
		multimap<string,string> familyToCode;
		for( itBInf=BrickInfo.begin(); itBInf!=BrickInfo.end(); ++itBInf )
		{
			string brk = (*itBInf).first;
			
			if(!validateBrick(brk)) continue;
						
			// family
			string family = brickToFamily[brk];
			
			familyToCode.insert( make_pair(family,brk) );
		}

		multimap<string,string>::iterator itFTC;
		for( itFTC=familyToCode.begin(); itFTC!=familyToCode.end(); ++itFTC )
		{
			itBInf=BrickInfo.find((*itFTC).second);
			
			string brk = (*itBInf).first;

			if(!validateBrick(brk)) continue;
			
			string skillTmp = (*itBInf).second.LearnSkills;
			string skill = skillTmp.substr(0,skillTmp.find_first_of(" "));
			CVectorSString dicoResult;
			Dico.lookup( skill, dicoResult, true );
			if(dicoResult.empty()) continue;

			// color
			string color = brickToColor[brk];
			
			// code
			string code = strlwr(brk.c_str());
			fprintf( brickFamilyDocFile, "<tr><td><FONT COLOR=\"%s\">%s</FONT></td>\n",color.c_str(),code.c_str());
			
			// name
			string name = (*itBInf).second.Text;
			fprintf( brickFamilyDocFile, "<td>%s</td>\n",name.c_str());

			// family
			string family = brickToFamily[brk];
			fprintf( brickFamilyDocFile, "<td><FONT COLOR=\"%s\">%s</FONT></td>\n",color.c_str(),family.c_str());

			// required skill name
			fprintf( brickFamilyDocFile, "<td>%s</td>\n",dicoResult[0].c_str());
			
			// required skill value
			string levelStr;
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			fprintf( brickFamilyDocFile, "<td>%s</td>\n",levelStr.c_str());

			// learn skill name
			skillTmp = brickToLearnSkill[brk];
			skill = skillTmp.substr(0,skillTmp.find_first_of(" "));
			fprintf( brickFamilyDocFile, "<td>%s</td>\n",skill.c_str());

			// learn skill value
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			fprintf( brickFamilyDocFile, "<td>%s</td>\n",levelStr.c_str());


			// phrase list
			fprintf( brickFamilyDocFile, "<td>");
			map<string,map<string,string> >::iterator itPhrases = brickToPhrases.find(brk);
			if( itPhrases != brickToPhrases.end() )
			{
				map<string,string>::iterator itPh;
				uint i;
				for( itPh=(*itPhrases).second.begin(),i=0; itPh!=(*itPhrases).second.end() && i<2; ++itPh,++i )
				{
					if( MultipleDocFiles )
						fprintf( brickFamilyDocFile,"<A HREF=\"%s_%c.html#%s\">%s</A>,&nbsp&nbsp",DocFileName.c_str(),(*itPh).first[7],(*itPh).first.c_str(),(*itPh).first.c_str());
					else
						fprintf( brickFamilyDocFile,"<A HREF=\"%s.html#%s\">%s</A>,&nbsp&nbsp",DocFileName.c_str(),(*itPh).first.c_str(),(*itPh).first.c_str());
				}
				if( i==2 )
				{
					char type = family[0];
					fprintf( brickFamilyDocFile,"[<A HREF=\"%s_%c.html#%s\">...</A>]",DocFileNameRoot.c_str(),type,brk.c_str());
				}
			}

			fprintf( brickFamilyDocFile, "</td></tr>\n");
		}
		fprintf( brickFamilyDocFile, "</tbody><table></P>\n" );
		fprintf( brickFamilyDocFile, "</body></html>\n" );
		fclose( brickFamilyDocFile );




		// REQUIRED SKILL VALUE

		// write header and title bar
		filename = DocFileNameRoot + "_required_skill_value.html";
		FILE * brickRequiredDocFile = fopen( filename.c_str(), "wt" );
		fprintf( brickRequiredDocFile,"<html><head>\n");
		fprintf( brickRequiredDocFile,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		fprintf( brickRequiredDocFile,"<title>Bricks infos</title>\n");
		fprintf( brickRequiredDocFile,"</head><body>\n");
		fprintf( brickRequiredDocFile,"<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n");
		fprintf( brickRequiredDocFile,"<tr>\n");
		fprintf( brickRequiredDocFile,"<td><b><a href=\"%s.html\">Code</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickRequiredDocFile,"<td><b><a href=\"%s_name.html\">Name</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickRequiredDocFile,"<td><b><a href=\"%s_family.html\">Family</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickRequiredDocFile,"<td><b>Required Skill Name</b></td>\n");
		fprintf( brickRequiredDocFile,"<td><b>*Required Skill Value*</b></td>\n");
		fprintf( brickRequiredDocFile,"<td><b>Learn Skill Name</b></td>\n");
		fprintf( brickRequiredDocFile,"<td><b><a href=\"%s_learn_skill_value.html\">Learn Skill Value</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickRequiredDocFile,"<td><b>Found In Phrases</b></td>\n");
		fprintf( brickRequiredDocFile,"</tr>\n");


		// write infos
		multimap<uint,string> requiredSkillValueToCode;
		for( itBInf=BrickInfo.begin(); itBInf!=BrickInfo.end(); ++itBInf )
		{
			string brk = (*itBInf).first;
			
			if(!validateBrick(brk)) continue;
						
			// required skill value
			string skillTmp = (*itBInf).second.LearnSkills;
			string levelStr;
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			uint level = atoi(levelStr.c_str());
				
			requiredSkillValueToCode.insert( make_pair(level,brk) );
		}

		multimap<uint,string>::iterator itRTC;
		for( itRTC=requiredSkillValueToCode.begin(); itRTC!=requiredSkillValueToCode.end(); ++itRTC )
		{
			itBInf=BrickInfo.find((*itRTC).second);

			string brk = (*itBInf).first;

			if(!validateBrick(brk)) continue;
			
			string skillTmp = (*itBInf).second.LearnSkills;
			string skill = skillTmp.substr(0,skillTmp.find_first_of(" "));
			CVectorSString dicoResult;
			Dico.lookup( skill, dicoResult, true );
			if(dicoResult.empty()) continue;
			
			// color
			string color = brickToColor[brk];

			// code
			string code = strlwr(brk.c_str());
			fprintf( brickRequiredDocFile, "<tr><td><FONT COLOR=\"%s\">%s</FONT></td>\n",color.c_str(),code.c_str());
			
			// name
			string name = (*itBInf).second.Text;
			fprintf( brickRequiredDocFile, "<td>%s</td>\n",name.c_str());

			// family
			string family = brickToFamily[brk];
			fprintf( brickRequiredDocFile, "<td><FONT COLOR=\"%s\">%s</FONT></td>\n",color.c_str(),family.c_str());

			// required skill name
			fprintf( brickRequiredDocFile, "<td>%s</td>\n",dicoResult[0].c_str());
			
			// required skill value
			string levelStr;
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			fprintf( brickRequiredDocFile, "<td>%s</td>\n",levelStr.c_str());

			// learn skill name
			skillTmp = brickToLearnSkill[brk];
			skill = skillTmp.substr(0,skillTmp.find_first_of(" "));
			fprintf( brickRequiredDocFile, "<td>%s</td>\n",skill.c_str());

			// learn skill value
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			fprintf( brickRequiredDocFile, "<td>%s</td>\n",levelStr.c_str());


			// phrase list
			fprintf( brickRequiredDocFile, "<td>");
			map<string,map<string,string> >::iterator itPhrases = brickToPhrases.find(brk);
			if( itPhrases != brickToPhrases.end() )
			{
				map<string,string>::iterator itPh;
				uint i;
				for( itPh=(*itPhrases).second.begin(),i=0; itPh!=(*itPhrases).second.end() && i<2; ++itPh,++i )
				{
					if( MultipleDocFiles )
						fprintf( brickRequiredDocFile,"<A HREF=\"%s_%c.html#%s\">%s</A>,&nbsp&nbsp",DocFileName.c_str(),(*itPh).first[7],(*itPh).first.c_str(),(*itPh).first.c_str());
					else
						fprintf( brickRequiredDocFile,"<A HREF=\"%s.html#%s\">%s</A>,&nbsp&nbsp",DocFileName.c_str(),(*itPh).first.c_str(),(*itPh).first.c_str());
				}
				if( i==2 )
				{
					char type = family[0];
					fprintf( brickRequiredDocFile,"[<A HREF=\"%s_%c.html#%s\">...</A>]",DocFileNameRoot.c_str(),type,brk.c_str());
				}
			}

			fprintf( brickRequiredDocFile, "</td></tr>\n");
		}
		fprintf( brickRequiredDocFile, "</tbody><table></P>\n" );
		fprintf( brickRequiredDocFile, "</body></html>\n" );
		fclose( brickRequiredDocFile );



		// LEARN SKILL VALUE

		// write header and title bar
		filename = DocFileNameRoot + "_learn_skill_value.html";
		FILE * brickLearnDocFile = fopen( filename.c_str(), "wt" );
		fprintf( brickLearnDocFile,"<html><head>\n");
		fprintf( brickLearnDocFile,"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n");
		fprintf( brickLearnDocFile,"<title>Bricks infos</title>\n");
		fprintf( brickLearnDocFile,"</head><body>\n");
		fprintf( brickLearnDocFile,"<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n");
		fprintf( brickLearnDocFile,"<tr>\n");
		fprintf( brickLearnDocFile,"<td><b><a href=\"%s.html\">Code</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickLearnDocFile,"<td><b><a href=\"%s_name.html\">Name</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickLearnDocFile,"<td><b><a href=\"%s_family.html\">Family</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickLearnDocFile,"<td><b>Required Skill Name</b></td>\n");
		fprintf( brickLearnDocFile,"<td><b><a href=\"%s_required_skill_value.html\">Required Skill Value</a></b></td>\n",DocFileNameRoot.c_str());
		fprintf( brickLearnDocFile,"<td><b>Learn Skill Name</b></td>\n");
		fprintf( brickLearnDocFile,"<td><b>*Learn Skill Value*</b></td>\n");
		fprintf( brickLearnDocFile,"<td><b>Found In Phrases</b></td>\n");
		fprintf( brickLearnDocFile,"</tr>\n");


		// write infos
		multimap<uint,string> learnSkillValueToCode;
		for( itBInf=BrickInfo.begin(); itBInf!=BrickInfo.end(); ++itBInf )
		{
			string brk = (*itBInf).first;
			
			if(!validateBrick(brk)) continue;
						
			// learn skill value
			string skillTmp = brickToLearnSkill[brk];
			string levelStr;
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			uint level = atoi(levelStr.c_str());
				
			learnSkillValueToCode.insert( make_pair(level,brk) );
		}

		multimap<uint,string>::iterator itLTC;
		for( itLTC=learnSkillValueToCode.begin(); itLTC!=learnSkillValueToCode.end(); ++itLTC )
		{
			itBInf=BrickInfo.find((*itLTC).second);

			string brk = (*itBInf).first;

			if(!validateBrick(brk)) continue;
			
			string skillTmp = (*itBInf).second.LearnSkills;
			string skill = skillTmp.substr(0,skillTmp.find_first_of(" "));
			CVectorSString dicoResult;
			Dico.lookup( skill, dicoResult, true );
			if(dicoResult.empty()) continue;
			
			// color
			string color = brickToColor[brk];

			// code
			string code = strlwr(brk.c_str());
			fprintf( brickLearnDocFile, "<tr><td><FONT COLOR=\"%s\">%s</FONT></td>\n",color.c_str(),code.c_str());
			
			// name
			string name = (*itBInf).second.Text;
			fprintf( brickLearnDocFile, "<td>%s</td>\n",name.c_str());

			// family
			string family = brickToFamily[brk];
			fprintf( brickLearnDocFile, "<td><FONT COLOR=\"%s\">%s</FONT></td>\n",color.c_str(),family.c_str());

			// required skill name
			fprintf( brickLearnDocFile, "<td>%s</td>\n",dicoResult[0].c_str());
			
			// required skill value
			string levelStr;
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			fprintf( brickLearnDocFile, "<td>%s</td>\n",levelStr.c_str());

			// learn skill name
			skillTmp = brickToLearnSkill[brk];
			skill = skillTmp.substr(0,skillTmp.find_first_of(" "));
			fprintf( brickLearnDocFile, "<td>%s</td>\n",skill.c_str());

			// learn skill value
			if( skillTmp.find(";") != -1 )
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.find_first_of(";")-idx-1);	
			}
			else
			{
				sint idx = skillTmp.find_first_of(" ");
				levelStr = skillTmp.substr(idx+1,skillTmp.size()-idx);
			}
			fprintf( brickLearnDocFile, "<td>%s</td>\n",levelStr.c_str());


			// phrase list
			fprintf( brickLearnDocFile, "<td>");
			map<string,map<string,string> >::iterator itPhrases = brickToPhrases.find(brk);
			if( itPhrases != brickToPhrases.end() )
			{
				map<string,string>::iterator itPh;
				uint i;
				for( itPh=(*itPhrases).second.begin(),i=0; itPh!=(*itPhrases).second.end() && i<2; ++itPh,++i )
				{
					if( MultipleDocFiles )
						fprintf( brickLearnDocFile,"<A HREF=\"%s_%c.html#%s\">%s</A>,&nbsp&nbsp",DocFileName.c_str(),(*itPh).first[7],(*itPh).first.c_str(),(*itPh).first.c_str());
					else
						fprintf( brickLearnDocFile,"<A HREF=\"%s.html#%s\">%s</A>,&nbsp&nbsp",DocFileName.c_str(),(*itPh).first.c_str(),(*itPh).first.c_str());
				}
				if( i==2 )
				{
					char type = family[0];
					fprintf( brickLearnDocFile,"[<A HREF=\"%s_%c.html#%s\">...</A>]",DocFileNameRoot.c_str(),type,brk.c_str());
				}
			}

			fprintf( brickLearnDocFile, "</td></tr>\n");
		}
		fprintf( brickLearnDocFile, "</tbody><table></P>\n" );
		fprintf( brickLearnDocFile, "</body></html>\n" );
		fclose( brickLearnDocFile );
		
	}
	
	
	return 0;

} // main //




