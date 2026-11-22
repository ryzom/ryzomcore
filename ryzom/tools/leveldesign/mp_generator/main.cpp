// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "moulinette.h"
#include "utils.h"
#include "nel/misc/algo.h"
#include "nel/misc/common.h"

#include <algorithm>

TRMItem currentDocItem;
CRMData SortableData;


// Preloaded Files
CSString	FamilyTypContent;
CSString	GroupTypContent;
CSString	WKContent;


// Stop the run: past this point the generated data would be wrong
static void fail( const std::string &message )
{
	nlwarning( "%s", message.c_str() );
	fprintf( stderr, "ERROR: %s\n", message.c_str() );
	exit( 1 );
}


// A file's path as pendingFiles keeps it
static string writtenKey( const std::string &fileName )
{
	return toLowerAscii( CPath::standardizePath( CFile::getPath( fileName ) ) + CFile::getFilename( fileName ) );
}


// A file this run makes. Nothing is written while the run computes and checks: a stop leaves the leveldesign as it
// was, and WritePendingFiles writes everything at the end. A file made twice must come out the same (a creature
// material is made once per creature that gives it); different contents mean two table lines make one sheet.
static void writeFile( const CSString &content, const std::string &fileName )
{
	const string key = writtenKey( fileName );
	map<string, CPendingFile>::iterator it = pendingFiles.find( key );
	if ( it != pendingFiles.end() )
	{
		if ( it->second.Content != content )
			fail( toString( "%s would be made twice with different contents: two lines of the tables make the same file", fileName.c_str() ) );
		return;
	}
	CPendingFile &file = pendingFiles[key];
	file.Path = fileName;
	file.Content = content;
}


// The content this run will write to a file, or NULL
static CSString *pendingContent( const std::string &fileName )
{
	map<string, CPendingFile>::iterator it = pendingFiles.find( writtenKey( fileName ) );
	return it == pendingFiles.end() ? NULL : &it->second.Content;
}


// The cells of a line of a table, empty ones included
static void splitCells( const std::string &line, vector<CSString> &cells )
{
	cells.clear();
	string::size_type pos = 0;
	for ( ;; )
	{
		const string::size_type next = line.find( ';', pos );
		cells.push_back( CSString( line.substr( pos, next == string::npos ? string::npos : next - pos ) ).strip() );
		if ( next == string::npos )
			break;
		pos = next + 1;
	}
}


// The stats of a raw material in an item part, by the names of _item_faber_mp_parameters.dfn; the tables name them so
static const char *s_StatNames[] = {
	"Durability", "Weight", "SapLoad", "DMG", "Speed", "Range", "DodgeModifier", "ParryModifier",
	"AdversaryDodgeModifier", "AdversaryParryModifier", "ProtectionFactor", "MaxSlashingProtection",
	"MaxBluntProtection", "MaxPiercingProtection", "ElementalCastingTimeFactor", "ElementalPowerFactor",
	"OffensiveAfflictionCastingTimeFactor", "OffensiveAfflictionPowerFactor", "HealCastingTimeFactor",
	"HealPowerFactor", "DefensiveAfflictionCastingTimeFactor", "DefensiveAfflictionPowerFactor", "AcidProtection",
	"ColdProtection", "FireProtection", "RotProtection", "ShockWaveProtection", "PoisonProtection",
	"ElectricityProtection", "DesertResistance", "ForestResistance", "LacustreResistance", "JungleResistance",
	"PrimaryRootResistance",
};


// The index of a stat by its name, -1 for an empty cell; a name no raw material has stops the run
static int statIndex( const CSString &cell, const char *table, const CSString &row )
{
	nlctassert( ( sizeof( s_StatNames ) / sizeof( s_StatNames[0] ) ) == NumMPStats );
	if ( cell.empty() )
		return -1;
	for ( int i = 0; i < NumMPStats; ++i )
	{
		if ( cell == s_StatNames[i] )
			return i;
	}
	fail( toString( "%s, %s: %s is no stat of a raw material (_item_faber_mp_parameters.dfn)", table, row.c_str(), cell.c_str() ) );
	return -1;
}


// Assign a new raw mat to a creature
void AssignerMP( const CSString& creatureName, const CSString& materialName )
{
	// we check if a creature is degenerated or not
	if ( ( creatureName.c_str()[3] != 'c' ) && ( creatureName.c_str()[3] != 'd' ) 
		&& ( creatureName.c_str()[3] != 'f' ) && ( creatureName.c_str()[3] != 'j' )
		&& ( creatureName.c_str()[3] != 'l' ) && ( creatureName.c_str()[3] != 'p' ) )
	{
	}
	else
	{
		// kept until all families are done, the files are written once by SaveAssignments()
		vector<CSString> &materials = creatureAssignments[creatureName];
		if ( std::find( materials.begin(), materials.end(), materialName ) == materials.end() )
			materials.push_back( materialName );
	}
}


// Find the raw material assignment files that exist, wherever they are below RAW_MATERIAL_ASSIGN
void LoadAssignmentFiles()
{
	printf( "-- REGISTERING RAW MATERIAL ASSIGNMENT FILES --\n" );
	if ( !CFile::isDirectory( RAW_MATERIAL_ASSIGN ) )
		return;

	vector<string> files;
	CPath::getPathContent( RAW_MATERIAL_ASSIGN, true, false, true, files );
	for ( uint i=0; i<files.size(); ++i )
	{
		if ( CFile::getExtension( files[i] ) == "creature" )
			assignmentFiles[ CFile::getFilenameWithoutExtension( files[i] ) ] = files[i];
	}
}


// A placeholder raw material: the system materials the creature sheet generator gives a creature until its own are assigned
static bool isPlaceholderMaterial( const CSString &materialEntry )
{
	return materialEntry.contains( "Value=\"system_mp" );
}


// A raw material sheet as this tool names them: m, the family number, the creature or source code, the ecosystem, the
// class letter and the variant (m0692chdca01.sitem)
static bool isRawMaterialSheetName( const string &name )
{
	if ( name.size() != 18 || name[0] != 'm' || name.substr( 12 ) != ".sitem" )
		return false;
	for ( uint i = 1; i < 12; ++i )
	{
		const bool digit = ( i <= 4 ) || ( i >= 10 );
		if ( digit ? ( name[i] < '0' || name[i] > '9' ) : ( name[i] < 'a' || name[i] > 'z' ) )
			return false;
	}
	return true;
}


// An entry of a quartering table that this tool owns: a material of one of the creature models of the tables
// (m<family>c<model code>...), which the tool assigns; deposit, invasion and goo items (dxa, ixx, cxx) and anything
// else in the table were put there by hand and stay
static bool isOwnedMaterial( const string &item )
{
	string name = toLowerAscii( item );
	if ( CFile::getExtension( name ).empty() )
		name += ".sitem";
	return isRawMaterialSheetName( name ) && name[5] == 'c' && creatureModelNames.count( CSString( name.substr( 6, 2 ) ) ) != 0;
}


// The quality of a creature's raw materials: its level (Basics.XPLevel of its sheet), which quartering gives the
// materials (CCharacter::harvest takes the MaxQuality of the creature's entry), up to the 250 of a material sheet. The
// sheet must name its assignment file as its parent, else what the tool writes there would do nothing.
static uint creatureQuality( const string &creatureName )
{
	map<string, string>::const_iterator it = creatureFiles.find( creatureName );
	if ( it == creatureFiles.end() )
		fail( toString( "The creature sheet %s.creature is not below %s", creatureName.c_str(), CREATURE_PATH.c_str() ) );
	CSString sheet;
	if ( !sheet.readFromFile( it->second ) )
		fail( toString( "Cannot read %s", it->second.c_str() ) );
	if ( !sheet.contains( ( "<PARENT Filename=\"_" + creatureName + "_mp.creature\"/>" ).c_str() ) )
		fail( toString( "%s does not name _%s_mp.creature as its parent: the raw materials written there would not be its own; generate_creature.py gives each creature sheet that parent",
			it->second.c_str(), creatureName.c_str() ) );
	const CSString level = sheet.splitFrom( "<ATOM Name=\"XPLevel\" Value=\"" ).splitTo( "\"" );
	if ( level.empty() || level.atoi() < 1 )
		fail( toString( "%s gives no Basics.XPLevel: its raw materials take that level as their quality", it->second.c_str() ) );
	return (uint)std::min( level.atoi(), 250 );
}


// Write the raw materials of this run into the assignment files of the creatures it processed. The raw material
// entries are this tool's: those of an earlier run give way to this run's, the placeholders too once the creature has
// materials; any other entry stays, in front. Each material gets the creature's level as its quality.
void SaveAssignments()
{
	printf( "-- SAVING RAW MATERIAL ASSIGNMENTS --\n" );
	uint nbChanged = 0, nbPlaceholders = 0, nbWithMaterials = 0;
	// the files are written once all of them have been read and checked
	vector<pair<string, CSString> > toWrite;

	for ( set<string>::const_iterator itCreature = processedCreatures.begin(); itCreature != processedCreatures.end(); ++itCreature )
	{
		const string &creatureName = *itCreature;
		map<string, vector<CSString> >::const_iterator itAssigned = creatureAssignments.find( creatureName );
		const vector<CSString> noMaterials;
		const vector<CSString> &materials = ( itAssigned != creatureAssignments.end() ) ? itAssigned->second : noMaterials;
		if ( !materials.empty() )
			++nbWithMaterials;

		// the creature sheet names its assignment file as its parent: a file the sheet does not name would do nothing
		const string baseName = "_" + creatureName + "_mp";
		map<string, string>::const_iterator itFile = assignmentFiles.find( baseName );
		if ( itFile == assignmentFiles.end() )
		{
			if ( materials.empty() )
				continue;
			fail( toString( "The creature %s has no raw material assignment file %s.creature below %s (RawMaterialAssignmentPath of raw_material_generation.cfg): generate_creature.py writes one for each creature, and the creature sheet names it as its parent",
				creatureName.c_str(), baseName.c_str(), RAW_MATERIAL_ASSIGN.c_str() ) );
		}
		const string fileName = itFile->second;
		CSString data;
		if ( !data.readFromFile( fileName ) )
			fail( toString( "Cannot read the raw material assignment file %s", fileName.c_str() ) );

		// the Harvest struct, from its line to the line that closes it (both at the indentation the tools write)
		const string text = data;
		const string eol = data.contains( "\r\n" ) ? "\r\n" : "\n";
		const string harvestOpen = "\n    <STRUCT Name=\"Harvest\">" + eol;
		const string harvestClose = "\n    </STRUCT>" + eol;
		const string::size_type begin = text.find( harvestOpen );
		const string::size_type end = begin == string::npos ? string::npos : text.find( harvestClose, begin + harvestOpen.size() - 1 );
		if ( end == string::npos )
			fail( toString( "The raw material assignment file %s has no Harvest struct in the form this tool writes", fileName.c_str() ) );

		// the entries there, each from its "<STRUCT Name="MPn">" line to its "</STRUCT>" line
		const string entryOpen = "      <STRUCT Name=\"MP";
		const string entryClose = "\n      </STRUCT>" + eol;
		vector<CSString> entries; // each entry from the quote after its number
		bool placeholderDropped = false;
		string::size_type pos = begin + harvestOpen.size();
		while ( pos < end + 1 )
		{
			const string::size_type entryEnd = text.find( entryClose, pos );
			if ( text.compare( pos, entryOpen.size(), entryOpen ) != 0 || entryEnd == string::npos || entryEnd > end )
				fail( toString( "The raw material assignment file %s has a Harvest struct this tool does not read", fileName.c_str() ) );
			// an empty entry (<STRUCT Name="MPn"/>) on its own line: it would swallow the next one
			const string::size_type lineEnd = text.find( eol, pos );
			if ( lineEnd != string::npos && lineEnd < entryEnd && text.compare( lineEnd - 2, 2, "/>" ) == 0 )
				fail( toString( "The raw material assignment file %s has an empty entry (%s), which this tool does not read: remove it", fileName.c_str(), text.substr( pos, lineEnd - pos ).c_str() ) );
			const string entry = text.substr( pos, entryEnd + entryClose.size() - pos );
			const CSString rest = entry.substr( entry.find( '"', entryOpen.size() ) );
			const string item = rest.splitFrom( "Name=\"AssociatedItem\" Value=\"" ).splitTo( "\"" );
			if ( isPlaceholderMaterial( rest ) )
			{
				// until the creature has materials of its own
				if ( materials.empty() )
					entries.push_back( rest );
				else
					placeholderDropped = true;
			}
			else if ( !isOwnedMaterial( item ) )
			{
				entries.push_back( rest );
			}
			pos = entryEnd + entryClose.size();
		}

		if ( !materials.empty() )
		{
			const uint quality = creatureQuality( creatureName );
			for ( uint i = 0; i < materials.size(); ++i )
			{
				entries.push_back( "\">" + eol
					+ toString( "        <ATOM Name=\"MinQuality\" Value=\"%u\"/>", quality ) + eol
					+ toString( "        <ATOM Name=\"MaxQuality\" Value=\"%u\"/>", quality ) + eol
					+ "        <ATOM Name=\"AssociatedItem\" Value=\"" + materials[i] + "\"/>" + eol
					+ "      </STRUCT>" + eol );
			}
		}

		// a creature that had materials and gets none would give nothing when quartered
		if ( entries.empty() )
			fail( toString( "The creature %s gets no raw material from the tables, and its quartering table %s would be left empty (the materials of an earlier run are this run's to replace): give its model a family for its level code, or put a placeholder (system_mp_basic.sitem, as generate_creature.py writes) back in that file",
				creatureName.c_str(), fileName.c_str() ) );

		// the creature sheets have room for ten (_creature_harvest.dfn, MP1 to MP10)
		if ( entries.size() > 10 )
			fail( toString( "%s would get %u raw materials, a creature has room for 10", fileName.c_str(), (uint)entries.size() ) );

		string harvest;
		for ( uint i = 0; i < entries.size(); ++i )
			harvest += entryOpen + toString( "%u", i + 1 ) + entries[i];
		const string result = text.substr( 0, begin + harvestOpen.size() ) + harvest + text.substr( end + 1 );
		if ( result == text )
			continue;

		toWrite.push_back( make_pair( fileName, CSString( result ) ) );
		++nbChanged;
		if ( placeholderDropped )
			++nbPlaceholders;
	}

	for ( uint i = 0; i < toWrite.size(); ++i )
		writeFile( toWrite[i].second, toWrite[i].first );

	printf( "    %u creatures, %u with raw materials, %u assignment files to change, %u placeholders to replace\n",
		(uint)processedCreatures.size(), nbWithMaterials, nbChanged, nbPlaceholders );
}


// save stats for each craft part
void LoadCraftParts()
{
	CSString data, ligne;
	int index;

	printf( "-- LOADING CRAFT PARTS --\n" );
	if ( !data.readFromFile( ITEM_MP_PARAM_DFN ) )
		fail( toString( "Cannot read %s", ITEM_MP_PARAM_DFN.c_str() ) );
	// the item parts of the DFN, one per letter A to Z ("A MpL (Blade)")
	bool named[NumMPCraftParts] = { false };
	for ( CSString rest = data.splitFrom( "Name=\"" ); !rest.empty(); rest = rest.splitFrom( "Name=\"" ) )
	{
		index = rest[0] - 'A';
		if ( index < 0 || index >= NumMPCraftParts || named[index] )
			fail( toString( "%s: the element %s is no item part, or one twice (a letter A to Z, then the part's name)", ITEM_MP_PARAM_DFN.c_str(), rest.splitTo( "\"" ).c_str() ) );
		craftParts[index].Desc = rest.splitTo( "\"" );
		named[index] = true;
	}
	for ( int i = 0; i < NumMPCraftParts; ++i )
	{
		if ( !named[i] )
			fail( toString( "%s has no item part %c", ITEM_MP_PARAM_DFN.c_str(), 'A' + i ) );
	}

	if ( !data.readFromFile( ITEM_PARTS_CSV ) )
		fail( toString( "Cannot read %s", ITEM_PARTS_CSV.c_str() ) );
	data = data.replace( "\r", "" );

	// the first line names the columns: the part letter, its name, then the stats by their names; a part carries the
	// stats whose cell is not empty
	vector<CSString> cells;
	do
		ligne = data.splitTo( "\n", true );
	while ( ligne.strip().empty() && !data.empty() );
	splitCells( ligne, cells );
	if ( cells.size() < 3 || cells[0] != "Part" || cells[1] != "Name" )
		fail( toString( "%s must start with a line Part;Name;<stat>;...", ITEM_PARTS_CSV.c_str() ) );
	vector<int> columns( cells.size(), -1 );
	set<int> namedStats;
	for ( uint c = 2; c < cells.size(); ++c )
	{
		if ( cells[c].empty() )
			fail( toString( "%s: column %u of the first line is empty: every column after Part and Name names the stat it marks", ITEM_PARTS_CSV.c_str(), c + 1 ) );
		columns[c] = statIndex( cells[c], "rm_item_parts.csv", "header" );
		if ( !namedStats.insert( columns[c] ).second )
			fail( toString( "%s: the first line names %s twice", ITEM_PARTS_CSV.c_str(), cells[c].c_str() ) );
	}

	bool found[NumMPCraftParts] = { false };
	while ( !data.empty() )
	{
		ligne = data.splitTo( "\n", true );
		if ( ligne.strip().empty() )
			continue;
		splitCells( ligne, cells );
		if ( cells[0].size() != 1 || cells[0][0] < 'A' || cells[0][0] >= 'A' + NumMPCraftParts || cells.size() > columns.size() )
			fail( toString( "%s: the line %s is no item part (a letter A to Z, then a cell per column)", ITEM_PARTS_CSV.c_str(), ligne.c_str() ) );
		index = cells[0][0] - 'A';
		if ( found[index] )
			fail( toString( "%s has two lines for the item part %c", ITEM_PARTS_CSV.c_str(), cells[0][0] ) );
		found[index] = true;
		for ( int i = 0; i < NumMPStats; ++i )
			craftParts[index].Carac[i] = false;
		for ( uint c = 2; c < cells.size(); ++c )
			craftParts[index].Carac[columns[c]] = !cells[c].empty();
	}
	for ( int i = 0; i < NumMPCraftParts; ++i )
	{
		if ( !found[i] )
			fail( toString( "%s has no line for the item part %c", ITEM_PARTS_CSV.c_str(), 'A' + i ) );
	}
}


// Save creatures filenames in bestiary
void LoadCreatureFiles()
{
	printf( "-- REGISTERING CREATURE FILES --\n" );
	vector<string> files;
	// on parcours les fichiers dans le repertoire
	CPath::getPathContent ( CREATURE_PATH, true, false, true, files );

	uint nbCreatures = 0;
	for (uint32 i=0; i<files.size(); ++i)
	{
		string filename = files[i];
		if ( CFile::getExtension( filename ) != "creature" )
			continue;
		string filebase = CFile::getFilenameWithoutExtension( filename );
		if ( creatureFiles.find( filebase ) != creatureFiles.end() )
			fail( toString( "Two creature sheets have the name %s.creature: %s and %s; a sheet name is its id, so one of them must go",
				filebase.c_str(), creatureFiles[filebase].c_str(), filename.c_str() ) );
		creatureFiles[ filebase ] = filename;
		if ( filebase.size() == 6 && filebase[0] == 'c' )
			++nbCreatures;
	}
	if ( nbCreatures == 0 )
		fail( toString( "No creature sheet (c<code><ecosystem><zone><digit>.creature) below %s: CreaturePath of raw_material_generation.cfg names the folder of the creature sheets, game_elem/creature in the leveldesign",
			CREATURE_PATH.c_str() ) );
}


// Save items levels list to generate for each creature 
void InitCreatureMP()
{
	CSString data, ligne;
	vector<CSString> cells;

	printf( "-- LOADING CREATURE MODELS --\n" );
	if ( !data.readFromFile( CREATURE_MODELS_CSV ) )
		fail( toString( "Cannot read %s", CREATURE_MODELS_CSV.c_str() ) );
	data = data.replace( "\r", "" );

	// one line per creature model: its code, its name (the first word of the names of its families), its category;
	// a line with an empty first cell is a comment
	while ( !data.empty() )
	{
		ligne = data.splitTo( "\n", true );
		splitCells( ligne, cells );
		if ( cells[0].empty() )
			continue;
		if ( cells.size() < 2 || cells[0].size() != 2 || cells[1].empty() )
			fail( toString( "%s: the line %s is no creature model (code;name;category)", CREATURE_MODELS_CSV.c_str(), ligne.c_str() ) );
		const CSString code = cells[0].toLower();
		const CSString &nom = cells[1];
		if ( creatureModelNames.find( code ) != creatureModelNames.end() )
			fail( toString( "%s has the creature code %s on two lines", CREATURE_MODELS_CSV.c_str(), code.c_str() ) );
		creatureModelNames[code] = nom;

		// the creatures of the model (c<code><ecosystem><zone><digit>.creature) in the ecosystems that get raw
		// materials: the common one and the five regions; the goo and invasion creatures keep what they have
		ListeCreatureMP listeCreatureMP;
		static const char s_Ecosystems[] = "cdfjlp";
		for ( const char *eco = s_Ecosystems; *eco; ++eco )
		{
			for ( int level = 0; level < 10; level++ )
			{
				for ( int creatureLevel = 1; creatureLevel <= 8; creatureLevel++ )
				{
					const string fileName = toString( "c%s%c%c%d", code.c_str(), *eco, 'a' + level, creatureLevel );
					if ( creatureFiles.find( fileName ) == creatureFiles.end() )
						continue;
					if ( level > 5 )
						fail( toString( "The creature sheet %s is of zone %c: the classes of raw materials (%s) and of mission materials (Plain in zone a to Magnificient in f) cover the zones a to f; give it a zone of those or extend both",
							creatureFiles[fileName].c_str(), 'a' + level, CREATURE_CLASSES_CSV.c_str() ) );
					CreatureMPItem *creatureMP = new CreatureMPItem;
					creatureMP->creatureLevel = creatureLevel;
					creatureMP->eco = *eco;
					creatureMP->itemLevel = level;
					creatureMP->codeCreature = code;
					creatureMP->creatureFileName = fileName;
					listeCreatureMP.push_back( creatureMP );
					processedCreatures.insert( fileName );
				}
			}
		}
		if ( itemsAGenerer.find( nom ) != itemsAGenerer.end() )
			fail( toString( "%s names the creature model %s twice", CREATURE_MODELS_CSV.c_str(), nom.c_str() ) );
		// a model's families give the materials of its creatures: with none, they would give nothing
		if ( listeCreatureMP.empty() )
			fail( toString( "%s: the creature model %s (%s) has no creature sheet c%s<ecosystem><zone><digit>.creature of the common ecosystem or the five regions below %s (CreaturePath of raw_material_generation.cfg): take the model out of the tables (make_tables.py) or check CreaturePath",
				CREATURE_MODELS_CSV.c_str(), nom.c_str(), code.c_str(), code.c_str(), CREATURE_PATH.c_str() ) );
		itemsAGenerer[nom] = listeCreatureMP;
	}
	printf( "    %u creature models, %u creatures\n", (uint)itemsAGenerer.size(), (uint)processedCreatures.size() );
}


// The class of the craft materials of a creature by its level code, from rm_creature_classes.csv
void LoadCreatureClasses()
{
	static const char *s_ClassNames[] = { "Basic", "Fine", "Choice", "Excellent", "Supreme" };
	CSString data, ligne;
	vector<CSString> cells;
	if ( !data.readFromFile( CREATURE_CLASSES_CSV ) )
		fail( toString( "Cannot read %s", CREATURE_CLASSES_CSV.c_str() ) );
	data = data.replace( "\r", "" );

	// a header line, then one line per level code: the zone letter and the digit of the creature sheet names, and the
	// class of the creature's craft materials, or - for none
	do
		ligne = data.splitTo( "\n", true );
	while ( ligne.strip().empty() && !data.empty() );
	while ( !data.empty() )
	{
		ligne = data.splitTo( "\n", true );
		if ( ligne.strip().empty() )
			continue;
		splitCells( ligne, cells );
		int classIndex = -2;
		if ( cells.size() >= 2 && cells[1] == "-" )
			classIndex = -1;
		for ( int i = 0; cells.size() >= 2 && i < 5; ++i )
		{
			if ( cells[1] == s_ClassNames[i] )
				classIndex = i;
		}
		if ( cells[0].size() != 2 || cells[0][0] < 'a' || cells[0][0] > 'f' || cells[0][1] < '1' || cells[0][1] > '8' || classIndex == -2 )
			fail( toString( "%s: the line %s is no level code (zone a to f, digit 1 to 8) and class (b3;Basic, Fine, Choice, Excellent, Supreme or -)", CREATURE_CLASSES_CSV.c_str(), ligne.c_str() ) );
		if ( creatureClasses.find( cells[0] ) != creatureClasses.end() )
			fail( toString( "%s has the level code %s on two lines", CREATURE_CLASSES_CSV.c_str(), cells[0].c_str() ) );
		creatureClasses[cells[0]] = classIndex;
	}
}


// Return quality maximum foran item level
int GetMaxQuality( int level )
{
	if ( level == 0 )
		return 50;

	return ( level * 50 );
}


// Return energy maximum for an item level
int GetStatEnergy( int level )
{
	if ( level == 0 )
		return 20;
	
	return ( 20 + ( level-1 ) * 15 );
}


// Return item class name depending on its characteristics
void GetItemClass( int level, bool mission, bool creature, CSString& outClassName )
{
	static CSString missionClasses[] = { "Plain", "Average", "Prime", "Select", 
		                                 "Superb", "Magnificient" };

	static CSString nonMissionClasses[] = { "", "Basic", "Fine", "Choice", 
		                                    "Excellent", "Supreme" };

	const int index = mission ? level : ( creature ? level + 1 : level );
	// Plain to Magnificient are 0 to 5, Basic to Supreme 1 to 5
	if ( index < ( mission ? 0 : 1 ) || index > 5 )
		fail( toString( "No %s class of raw materials for the level %d: the classes go from %s to %s", mission ? "mission" : "craft", level,
			mission ? "Plain (zone a)" : "Basic", mission ? "Magnificient (zone f)" : "Supreme" ) );
	if ( mission )
		outClassName = missionClasses[ index ];
	else
		outClassName = nonMissionClasses[ index ];
}


// Retourne la couleur de l'item
void GetItemColor( int color, char eco, int level, CSString& outStr )
{
	CSString commonColors [4] = { "Beige", "Green", "Turquoise", "Violet" };
	CSString rareColors [2] = { "Red", "Blue" };
	CSString primeRootColors [2] = { "White", "Black" };
	
	if ( eco == 'p' )
		outStr = primeRootColors[ color % 2 ];
	else if ( ( eco != 'c' ) && ( level >= 2 ) )
		outStr = rareColors[ color % 2 ];
	else if ( ( eco == 'c' ) && ( level >= 3 ) )
		outStr = rareColors[ color % 2 ];
	else
		outStr = commonColors[ color % 4 ];
} 


bool endsWith( const CSString& s, const CSString& substring )
{
	return (s.right( (uint)substring.size() ) == substring);
}

bool endsWith( const CSString& s, const char *substring )
{
	return endsWith(s, CSString(substring));
}


// Generate item names
void GenerateItemNames( const CSString& nomMP, char eco, int level, bool mission, bool creature, CSString& outStr )
{
	CSString itemClass, prefix, singularWithNoPrefix;
	CSString ia = "a";

	GetItemClass( level, mission, creature, itemClass );
	currentDocItem.push( DtStatQuality, itemClass );

	if ( !mission )
	{
		if ( ( creature && ( level == 3 ) ) || ( !creature && ( level == 4 ) ) )
			ia = "an";
	}

	singularWithNoPrefix = itemClass + " ";

	// selection de l'eco-systeme
	switch ( eco )
	{
	case 'c' :
		break;

	case 'd' :
		singularWithNoPrefix += "Desert ";
		break;

	case 'f' :
		singularWithNoPrefix += "Forest ";
		break;

	case 'j' :
		singularWithNoPrefix += "Jungle ";
		break;

	case 'l' :
		singularWithNoPrefix += "Lakes ";
		break;

	case 'p' :
		singularWithNoPrefix += "Prime Root ";
		break;
	}

	singularWithNoPrefix += nomMP;

	// Contenant
	if ( endsWith( nomMP, "Wood" ) )
		prefix = "Bundle";
	else if ( endsWith( nomMP, "Bark" ) || endsWith( nomMP, "Moss" ) || endsWith( nomMP, "Sawdust" ) ||
			  endsWith( nomMP, "Straw" ) || endsWith( nomMP, "Dust" ) || endsWith( nomMP, "Soil" ) ||
			  endsWith( nomMP, "Cereal" ) )
		prefix = "Handful";
	else if ( endsWith( nomMP, "Resin" ) || endsWith( nomMP, "Wax" ) )
		prefix = "Portion";
	else if ( endsWith( nomMP, "Whiskers" ) || endsWith( nomMP, "Hairs" ) )
		prefix = "Tuft";
	else if ( endsWith( nomMP, "Silk" ) )
		prefix = "Ball";
	else if ( endsWith( nomMP, "Sap" ) || endsWith( nomMP, "Residue" ) || endsWith( nomMP, "Honey" ) ||
			  endsWith( nomMP, "Blood" ) )
		prefix = "Phial";
	else if ( endsWith( nomMP, "Fruit" ) )
		prefix = "Piece";
	else if ( endsWith( nomMP, "Flesh" ) )
		prefix = "Morsel";
	else if ( endsWith( nomMP, "Saliva" ) )
		prefix = "Sample";
	else if ( endsWith( nomMP, "Pollen" ) || endsWith( nomMP, "Fiber" ) || endsWith( nomMP, "Amber" ) ||
			  endsWith( nomMP, "Leather" ) || endsWith( nomMP, "Oil" ) )
		ia = "some";
	else if ( endsWith( nomMP, "Pelvis" ) || endsWith( nomMP, "Eye" ) || endsWith( nomMP, "Spine" ) ||
			  endsWith( nomMP, "Hoof" ) || endsWith( nomMP, "Mandible" ) || endsWith( nomMP, "Claw") ||
			  endsWith( nomMP, "Tail" ) || endsWith( nomMP, "Trunk" ) || endsWith( nomMP, "Shell" ) ||
			  endsWith( nomMP, "Sting" ) || endsWith( nomMP, "Skin" ) || endsWith( nomMP, "Beak" ) ||
			  endsWith( nomMP, "Wing" ) || endsWith( nomMP, "Horn" ) || endsWith( nomMP, "Rostrum" ) ||
			  endsWith( nomMP, "Skull" ) || endsWith( nomMP, "Pistil" ) )
		prefix = "Fragment"; // number-limited creature objects

	if ( ! prefix.empty() )
	{
		outStr = prefix + " of " + singularWithNoPrefix;
		ia = "a";
	}
	else
		outStr = singularWithNoPrefix;

	CSString singular = outStr;
	currentDocItem.push( DtTitle, outStr );

	// A, The
	outStr += "\t" + ia + "\tthe\t";

	// Plural
	if ( prefix.empty() )
		outStr += singular + "s";
	else
		outStr += prefix + "s" + " of " + singularWithNoPrefix;

	outStr += "\t\tthe";
}


// The number of a family in item_mp_family.typ (its whole label): the tables make no new family, and each family has its
// text in wk.uxt (mpfam<number>), which the client shows
int GetNumeroMP( const CSString& nomMP, uint lineNumber )
{
	CSString result = FamilyTypContent.splitFrom( toString( "Label=\"%s\" Value=\"", nomMP.c_str() ).c_str() );
	if ( result.empty() )
		fail( toString( "%s, line %u: the family %s is not in %s: the tables make no new family (make_tables.py takes the families from the translation)",
			FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), ITEM_MP_FAMILY_TYP.c_str() ) );
	const int res = result.splitTo( "\"" ).atoi();
	if ( !WKContent.contains( toString( "\nmpfam%d\t", res ).c_str() ) && !WKContent.contains( toString( "\nmpfam%d ", res ).c_str() ) )
		fail( toString( "%s, line %u: the family %s (%d) has no text mpfam%d in %s", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), res, res, WK_UXT.c_str() ) );
	return res;
}


// The number of a group in item_mp_group.typ, likewise
int GetNumeroGroupe( const CSString& groupe, uint lineNumber )
{
	CSString result = GroupTypContent.splitFrom( toString( "Label=\"%s\" Value=\"", groupe.c_str() ).c_str() );
	if ( result.empty() )
		fail( toString( "%s, line %u: the group %s is not in %s: the tables make no new group", FAM_PROP_CSV.c_str(), lineNumber, groupe.c_str(), ITEM_MP_GROUPE_TYP.c_str() ) );
	const int res = result.splitTo( "\"" ).atoi();
	if ( !WKContent.contains( toString( "\nmpgroup%d\t", res ).c_str() ) && !WKContent.contains( toString( "\nmpgroup%d ", res ).c_str() ) )
		fail( toString( "%s, line %u: the group %s (%d) has no text mpgroup%d in %s", FAM_PROP_CSV.c_str(), lineNumber, groupe.c_str(), res, res, WK_UXT.c_str() ) );
	return res;
}


// Generate parent item for a raw mat
void CreateParentSItem( int numMP, 
					    const CSString& nomMP,
						const CSString& groupe,
						bool dropOrSell,
						const CSString& icon,
						const CSString& overlay )
{
	CSString output;
	CSString outputFileName;

	// output filename
	outputFileName = toString( "%s_parent/_m%04d.sitem", MP_DIRECTORY.c_str(), numMP );

	// xml header
	output = "<?xml version=\"1.0\"?>\n<FORM Version=\"0.0\" State=\"modified\">\n";
	
	// basics
	output += "  <STRUCT>\n    <STRUCT Name=\"basics\">\n";
	output += "      <ATOM Name=\"Drop or Sell\" Value=\"";

	if ( !dropOrSell )
		// mission items can't be sold
		output += "false\"/>\n";
	else
		// the others, yes
		output += "true\"/>\n";

	output += "      <ATOM Name=\"Bulk\" Value=\"0.5\"/>\n    </STRUCT>\n";

	// raw mat
	output += "    <STRUCT Name=\"mp\">\n";
	output += "      <ATOM Name=\"Family\" Value=\"";
	output += nomMP;
	output += "\"/>\n      <ATOM Name=\"Group\" Value=\"";
	output += groupe;
	output += "\"/>\n    </STRUCT>\n";

	// 3d
	output += "    <STRUCT Name=\"3d\">\n";
	
	if ( !icon.empty() )
	{
		output += "      <ATOM Name=\"icon\" Value=\"";
		output += icon;
		output += "\"/>\n";
	}

	if ( !overlay.empty() )
	{
		output += "      <ATOM Name=\"text overlay\" Value=\"";
		output += overlay;
		output += "\"/>\n";
	}

	output += "    </STRUCT>\n  </STRUCT>\n";

	// end of file
	output += "  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n</FORM>\n";

	// final write
	writeFile( output, outputFileName );

}


// Fill information related to craft with an item
void FillCraftData( const CSString &nomMP, char craft, int eco, int level, bool creature, int bestStat,  
				    int worstStat1, int worstStat2, CSString& outStr )
{
	CSString data;
	char buf[10];
	int index;
	static int mediumStatsByStatQuality[] = { 20, 35, 50, 65, 80 };
	int stat, remaining;
	int stats[NumMPStats];

	index = craft - 'A';

	outStr = "        <STRUCT Name=\"";
	outStr += craftParts[index].Desc;
	outStr += "\">\n";

	remaining = 0;

	currentDocItem.push( DtCraftSlotName, craftParts[index].Desc.splitFrom( "(" ).splitTo( ")" ) );

	// save stats for each characteristic
	for ( int i=0; i<NumMPStats; i++ )
	{
		if ( craftParts[index].Carac[i]  )
		{
			// Basic to Supreme: the level of a deposit or special item counts from 1, a creature's from 0
			const int classIndex = creature ? level : level - 1;
			if ( classIndex < 0 || classIndex > 4 )
				fail( toString( "%s, %s: no craft class for the level %d of a %s raw material in the item part %s: the classes go from Basic to Supreme",
					FAM_PROP_CSV.c_str(), nomMP.c_str(), level, creature ? "creature" : "deposit or special", craftParts[index].Desc.c_str() ) );
			stat = mediumStatsByStatQuality[ classIndex ];
			
			// manage weak/strong points of a raw mat
			if ( i == bestStat )
			{
				if ( worstStat2 == -1 )
					stat += 20;
				else
					stat += 40;

				if ( stat > 100 )
				{
					// if a stat exceeds 100, other are increased
					remaining = stat - 100;
					stat = 100;
				}
			}
			else if ( ( i == worstStat1 ) || ( i == worstStat2 ) )
			{
				stat -= 20;

				// minimum durability
				if ( ( i == 0 ) && ( stat < 1 ) )
					stat = 1;
			}

			stats[i] = stat;
		}
		else 
			stats[i] = -1;
	}

	// what the specialty has above 100 goes to the other stats, a point each in turn, so that the stats of the part
	// add up to the class's total
	while ( remaining > 0 )
	{
		bool given = false;
		for ( int i = 0; i < NumMPStats && remaining > 0; ++i )
		{
			if ( stats[i] != -1 && i != bestStat && i != worstStat1 && i != worstStat2 && stats[i] < 100 )
			{
				++stats[i];
				--remaining;
				given = true;
			}
		}
		if ( !given )
			fail( toString( "%s, %s: its specialty in the item part %s goes above 100 and the part has no other stat to take the rest",
				FAM_PROP_CSV.c_str(), nomMP.c_str(), craftParts[index].Desc.c_str() ) );
	}

	// add information for each characteristic
	for ( int i=0; i<NumMPStats; i++ )
	{
		if ( stats[i] != -1 )
		{
			outStr += "          <ATOM Name=\"";
			outStr += s_StatNames[i];
			outStr += "\" Value=\"";
			
			sprintf( buf, "%d", stats[i] );

			outStr += buf;
			outStr += "\"/>\n";
		}
	}

	// CraftCivSpec depending on ecosystem
	outStr += "          <ATOM Name=\"CraftCivSpec\" Value=\"";
	CSString craftCiv;

	switch ( eco )
	{
	case 'c' :
		outStr += "common";
		craftCiv = "All";
		break;

	case 'd' :
		outStr += "fyros";
		craftCiv = "Fyros";
		break;

	case 'f' :
		outStr += "matis";
		craftCiv = "Matis";
		break;

	case 'j' :
		outStr += "zorai";
		craftCiv = "Zorai";
		break;

	case 'l' :
		outStr += "tryker";
		craftCiv = "Tryker";
		break;

	case 'p' :
		outStr += "common";
		craftCiv = "All";
		break;
	}

	currentDocItem.push( DtCraftCivSpec, craftCiv );

	outStr += "\"/>\n        </STRUCT>\n";
}


// Create item sheets
// Returns false where the tool makes no sheet: a mission material of a creature of the common ecosystem
bool CreateSheet( int numMP, const CSString& nomMP,
				  const CSString& code, char eco,
				  int level, const MPCraftStats& craftStats,
				  bool specialItem = false, int variation = 1 )
{
	CSString output, directory, itemName, craftInfo, color;
	CSString outputFileName, ecoStr;
	char chaineNum[5];
	bool creature = ( code != "dxa" ) && ( code != "cxx" ) && ( code != "ixx" );

	// Creation du nom de fichier
	sprintf( chaineNum, "%04d", numMP );

	switch ( eco )
	{
	case 'd' :
		directory = "desert";
		ecoStr = "Desert";
		break;

	case 'f' :
		directory = "forest";
		ecoStr = "Forest";
		break;

	case 'j' :
		directory = "jungle";
		ecoStr = "Jungle";
		break;

	case 'l' :
		directory = "lacustre";
		ecoStr = "Lacustre";
		break;

	case 'p' :
		directory = "prime_roots";
		ecoStr = "PrimeRoots";
		break;

	default :
		directory = "common";
		ecoStr = "Common";
		eco = 'c';
		break;
	}

	if ( ( eco == 'c' ) && creature && ( craftStats.Craft.empty() ) )
		return false;

	outputFileName = toString( "m%04d%s%c%c%02d.sitem", numMP, code.c_str(), eco, 
		                     'a' + level, variation );

	if ( craftStats.Craft.empty() )
	{
		CSString levelZone = toString( "%c", 'a' + level );
		currentDocItem.push( DtLevelZone, levelZone.toUpper() );
	}
	else
		currentDocItem.push( DtLevelZone, "-" );
	
	// fill sheets information
	output = "<?xml version=\"1.0\"?>\n<FORM Version=\"0.0\" State=\"modified\">\n";
	output += "  <PARENT Filename=\"_m";
	output += eco;
	output += ".sitem\"/>\n  <PARENT Filename=\"_m";
	output += chaineNum;
	output += ".sitem\"/>\n";

	// if a creature code, add it its parent
	if ( creature )
	{
		output += "  <PARENT Filename=\"_m";
		output += code;
		output += ".sitem\"/>\n";
		creature = true;
		generatedCreatureCodes.insert( code );
	}

	output += "  <STRUCT>\n";


    output += "    <STRUCT Name=\"mp\">\n";
	output += "      <ATOM Name=\"MpColor\" Value=\"";
	
	// mission materials always Beige
	if ( craftStats.Craft.empty() )
	{
		output += "Beige\"/>\n";
		if(craftStats.UsedAsCraftRequirement)
			output += "      <ATOM Name=\"UsedAsCraftRequirement\" Value=\"true\"/>\n";
	}
	else
	{
		// get the color
		GetItemColor( craftStats.color, eco, level, color );
		output += color;
		output += "\"/>\n";

		currentDocItem.push( DtColor, color );

		// add craft data
		output += "      <STRUCT Name=\"MpParam\">\n";
		for ( uint i=0; i<craftStats.Craft.size(); i++ )
		{
			int bestStat, worstStat1, worstStat2;

			if ( i == 0 )
			{
				bestStat = craftStats.bestStatA;
				worstStat1 = craftStats.worstStatA1;
				worstStat2 = craftStats.worstStatA2;
			}
			else if ( i == 1 )
			{
				bestStat = craftStats.bestStatB;
				worstStat1 = craftStats.worstStatB1;
				worstStat2 = craftStats.worstStatB2;
			}
			else
			{
				bestStat = -1;
				worstStat1 = -1;
				worstStat2 = -1;
			}

			currentDocItem.push( DtProp, toString( "%c", craftStats.Craft.c_str()[i] ).c_str() );
			
			FillCraftData( nomMP, craftStats.Craft.c_str()[i], eco, level, creature, bestStat,  
					   worstStat1, worstStat2, craftInfo );
			output += craftInfo;
		}

		output += "      </STRUCT>\n";
	}

	output += "      <ATOM Name=\"MaxQuality\" Value=\"";
	if ( creature || specialItem || ( code == "cxx" ) )
	{
		CSString maxQuality = toString( "%d", 250 ); 
		output += maxQuality;
		currentDocItem.push( DtMaxLevel, maxQuality );
	}
	else
	{
		CSString maxQuality = toString( "%d", GetMaxQuality( level ) );
		output += maxQuality;
		currentDocItem.push( DtMaxLevel, maxQuality );
	}

	output += "\"/>\n      <ATOM Name=\"StatEnergy\" Value=\"";

	CSString statEnergy;
	if ( ( variation == 2 ) && ( numMP == 695 ) ) // cas particulier pour le kitin trophy (beurk)
		statEnergy = "0";
	else if ( !creature || ( craftStats.Craft.empty() ) )
		statEnergy = toString( "%d", GetStatEnergy( level ) );
	else if ( variation < 2 )
		statEnergy = toString( "%d", GetStatEnergy( level + 1 ) );
		
	output += statEnergy;
	currentDocItem.push( DtAverageEnergy, statEnergy );

	output += "\"/>\n    </STRUCT>\n  </STRUCT>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n</FORM>\n";

	writeFile( output, toString( "%s%s/%s", MP_DIRECTORY.c_str(), directory.c_str(), outputFileName.c_str() ) );

	// Generate names
	if ( !specialItem )
	{
		outputFileName = toString( "m%04d%s%c%c%02d", numMP, code.c_str(), eco, 'a' + level, variation );
		output = outputFileName;

		GenerateItemNames( nomMP, eco, level, ( craftStats.Craft.empty() ), creature, itemName );
		output += "\t" + itemName;
		itemNames.insert( output );
	}

	currentDocItem.push( DtEcosystem, ecoStr );
	currentDocItem.push( DtName, outputFileName );

	SortableData.updateItemAppend( currentDocItem, DtName );
	currentDocItem.reset( DtName );
	currentDocItem.reset( DtEcosystem );
	currentDocItem.reset( DtMaxLevel );
	currentDocItem.reset( DtAverageEnergy );
	currentDocItem.reset( DtTitle );
	currentDocItem.reset( DtLevelZone );
	currentDocItem.reset( DtStatQuality );
	currentDocItem.reset( DtCraftSlotName );
	currentDocItem.reset( DtCraftCivSpec );
	currentDocItem.reset( DtColor );
	return true;
}


// Generate deposits items for harvested raw mats
void GenerateDepositItems( int numMP, const CSString& nomMP, const MPCraftStats& craftStats, const CSString& loc )
{
	CSString code;

	if ( loc.left(1) == "I" )
		code = "ixx";
	else if ( loc.left(1) == "D" )
		code = "dxa";
	else
		code = "cxx";

	// pas de craft = items de mission
	if ( craftStats.Craft.empty() )
	{
		if ( loc != "G" )
			CreateSheet( numMP, nomMP, code, 'c', 0, craftStats );
		
		for ( int i=1; i<6; i++ )
		{
			CreateSheet( numMP, nomMP, code, 'c', i, craftStats );
		}
	}
	else
	{
		// 2 items in common
		CreateSheet( numMP, nomMP, code, 'c', 1, craftStats );
		CreateSheet( numMP, nomMP, code, 'c', 2, craftStats );

		// 3 items per zone
		for ( int i=0; i<3; i++ )
		{
			CreateSheet( numMP, nomMP, code, 'd', 3+i, craftStats );
			CreateSheet( numMP, nomMP, code, 'f', 3+i, craftStats );
			CreateSheet( numMP, nomMP, code, 'j', 3+i, craftStats );
			CreateSheet( numMP, nomMP, code, 'l', 3+i, craftStats );
			CreateSheet( numMP, nomMP, code, 'p', 3+i, craftStats );
		}
	}
}


// Generate creatures items for a looted raw mat
void GenerateCreatureItems( int numMP, CSString& nomMP, const MPCraftStats& craftStats )
{
	map<CSString, ListeCreatureMP>::const_iterator itLCMP;
	int quality;

	// Get items levels to generate for the creature
	itLCMP = itemsAGenerer.find( nomMP.firstWord() );

	if ( itLCMP != itemsAGenerer.end() )
	{
		ListeCreatureMP::const_iterator itMP = (*itLCMP).second.begin();

		// for each level of an item to generate
		while ( itMP != (*itLCMP).second.end() )
		{
			// save its stats
			char eco = (*itMP)->eco;
			int creatureLevel = (*itMP)->creatureLevel;
			int itemLevel = (*itMP)->itemLevel;
			CSString creatureFileName = "c";
			creatureFileName += (*itMP)->codeCreature.toLower();

			if ( !craftStats.Craft.empty() )
			{
				// the class of the craft materials by the creature's level code (rm_creature_classes.csv)
				const string levelCode = toString( "%c%d", 'a' + itemLevel, creatureLevel );
				map<string, int>::const_iterator itClass = creatureClasses.find( levelCode );
				if ( itClass == creatureClasses.end() )
					fail( toString( "%s gives no class for the level code %s of the creature %s", CREATURE_CLASSES_CSV.c_str(), levelCode.c_str(), (*itMP)->creatureFileName.c_str() ) );
				quality = itClass->second;
				if ( quality >= 0 )
				{
					if ( quality < 2 )
					{
						CreateSheet( numMP, nomMP, creatureFileName, 'c', quality, craftStats );

						AssignerMP( (*itMP)->creatureFileName, 
							toString( "m%04d%s%c%c01.sitem", numMP, creatureFileName.c_str(), 'c', 'a' + quality ) );
					}
					else
					{
						CreateSheet( numMP, nomMP, creatureFileName, eco, quality, craftStats );

						AssignerMP( (*itMP)->creatureFileName, 
							toString( "m%04d%s%c%c01.sitem", numMP, creatureFileName.c_str(), eco, 'a' + quality ) );
					}

					currentDocItem.push( DtCreature, (*itMP)->creatureFileName );
				}
			}
			else
			{
				// pas de MP de mission pour les boss
				if ( creatureLevel < 5 && CreateSheet( numMP, nomMP, creatureFileName, eco, itemLevel, craftStats ) )
				{
					AssignerMP( (*itMP)->creatureFileName,
							toString( "m%04d%s%c%c01.sitem", numMP, creatureFileName.c_str(), eco, 'a' + itemLevel ) );

					currentDocItem.push( DtCreature, (*itMP)->creatureFileName );
				}
			}

			itMP++;
		}
	}
}


// Generate special item
void GenerateSpecialItem( int numMP, const CSString& nomMP, const MPCraftStats& craftStats, 
						  const CSString& loc,CSString& itemData, int variation )
{
	CSString info, code, name;
	info = itemData.splitTo( "/", true ).toLower();

	if ( loc.left(1) == "I" )
		code = "ixx";
	else if ( loc.left(1) == "D" )
		code = "dxa";
	else
		code = "cxx";

	// its ecosystem letter and class letter (cf: common, Supreme); a craft material's classes go from b, Basic
	const char lowestClass = craftStats.Craft.empty() ? 'a' : 'b';
	if ( info.size() != 2 || string( "cdfjlp" ).find( info[0] ) == string::npos || info[1] < lowestClass || info[1] > 'f' )
		fail( toString( "%s, %s: the special item %s must start with an ecosystem letter (c, d, f, j, l or p) and a class letter (%c to f)",
			FAM_PROP_CSV.c_str(), nomMP.c_str(), info.c_str(), lowestClass ) );
	const string directories = "cdfjlp";
	static const char *s_Directories[] = { "common", "desert", "forest", "jungle", "lacustre", "prime_roots" };
	const string sheet = toString( "%s%s/m%04d%s%s%02d.sitem", MP_DIRECTORY.c_str(), s_Directories[directories.find( info[0] )], numMP, code.c_str(), info.c_str(), variation );
	if ( pendingContent( sheet ) )
		fail( toString( "%s, %s: the special item %s makes %s, which the family already makes", FAM_PROP_CSV.c_str(), nomMP.c_str(), info.c_str(), CFile::getFilename( sheet ).c_str() ) );
	CreateSheet( numMP, nomMP, code, info.c_str()[0], info.c_str()[1]-'a', craftStats, true, variation );

	name = toString( "m%04d%s%s%02d\t", numMP, code.c_str(), info.c_str(), variation ); 
	name += itemData.splitTo( "/", true ); // singular
	name += "\t";
	name += itemData.splitTo( "/", true ); // undefined article
	name += "\t";
	name += itemData.splitTo( "/", true ); // defined article
	name += "\t";
	name += itemData.splitTo( "/", true ); // plural
	name += "\t\t";
	name += itemData.splitTo( "/", true ); // plural article

	itemNames.insert( name );
}


// The attributes of a family (R used as a craft requirement, D0 and D1 cannot or can be dropped and sold), separated by
// "-"; craftStats must be good for extraInfo init
void	parseSpecialAttributes(const CSString &specialAttributes, MPCraftStats &craftStats, CExtraInfo &extraInfo, const CSString &nomMP, uint lineNumber)
{
	// evaluate DropOrSell according to CraftStats: can DropOrSell if it is a MP Craft
	extraInfo.DropOrSell= !craftStats.Craft.empty();

	// parse attributes
	vector<string> strArray;
	splitString(specialAttributes, "-", strArray);
	for(uint i=0;i<strArray.size();i++)
	{
		if(nlstricmp(strArray[i], "R")==0)
			craftStats.UsedAsCraftRequirement= true;
		else if(nlstricmp(strArray[i], "D0")==0)
			extraInfo.DropOrSell= false;
		else if(nlstricmp(strArray[i], "D1")==0)
			extraInfo.DropOrSell= true;
		else
			fail( toString( "%s, line %u: %s has the attribute %s; the attributes are R, D0 and D1", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), strArray[i].c_str() ) );
	}
}


// The families of the tables so far, by name in lower case, and the line of each
static map<string, uint> s_FamilyLines;


// New raw mat to process
void NewMP( const CSString& ligne, uint lineNumber )
{
	CSString overlay;
	MPCraftStats craftStats;
	CExtraInfo	extraInfo;
	int numMP;
	CSortedStringSet specialNames;

	// the cells: family, group, craft parts, attributes, -, location, icon, -, -, -, the specialty and weaknesses of the
	// first part, those of the second, colour, -, special only, then the special items; a line with an empty first
	// cell is a header or a comment
	vector<CSString> cells;
	splitCells( ligne, cells );
	CSString nomMP = cells[0];
	if ( nomMP.empty() )
		return;
	if ( cells.size() < 19 )
		fail( toString( "%s, line %u: %s has %u cells, a family line has 19 and its special items after them", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), (uint)cells.size() ) );

	// the name makes file names (the deposit family list of the World Editor) and translation keys
	for ( uint i = 0; i < nomMP.size(); ++i )
	{
		const char c = nomMP[i];
		if ( !( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == ' ' || c == '\'' || c == '-' ) )
			fail( toString( "%s, line %u: the family %s has a character other than letters, digits, space, ' and -", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str() ) );
	}
	// one line per family, whatever the case of its letters
	const string familyKey = toLowerAscii( nomMP );
	if ( s_FamilyLines.find( familyKey ) != s_FamilyLines.end() )
		fail( toString( "%s, line %u: the family %s has a line already, line %u", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), s_FamilyLines[familyKey] ) );
	s_FamilyLines[familyKey] = lineNumber;

	const CSString &groupe = cells[1];
	craftStats.Craft = cells[2];
	parseSpecialAttributes( cells[3], craftStats, extraInfo, nomMP, lineNumber );
	const CSString &loc = cells[5];
	const CSString &icon = cells[6];

	// the location: deposits, creatures, goo creatures, or invasion and goo rewards (I and the kinds of creatures that
	// give them), which are special items only
	const string special = toLowerAscii( cells[18] );
	if ( !special.empty() && special != "x" )
		fail( toString( "%s, line %u: %s has %s for special only, which is x or empty", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), cells[18].c_str() ) );
	const bool specialOnly = special == "x";
	bool invasion = loc.size() > 1 && loc[0] == 'I';
	for ( uint i = 1; invasion && i < loc.size(); ++i )
		invasion = loc[i] >= 'a' && loc[i] <= 'z';
	if ( loc != "D" && loc != "C" && loc != "G" && !invasion )
		fail( toString( "%s, line %u: %s has the location %s; the locations are D (deposits), C (creatures), G (goo creatures) and I followed by the creature kinds (invasion and goo rewards)",
			FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), loc.c_str() ) );
	if ( invasion && !specialOnly )
		fail( toString( "%s, line %u: %s is an invasion or goo reward (location %s), which has special items only", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), loc.c_str() ) );

	// the craft parts, each once
	for ( uint i = 0; i < craftStats.Craft.size(); ++i )
	{
		const int part = craftStats.Craft[i] - 'A';
		if ( part < 0 || part >= NumMPCraftParts )
			fail( toString( "%s, line %u: %s serves %c, which is no item part (A to Z)", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), craftStats.Craft[i] ) );
		if ( craftStats.Craft.find_first_of( craftStats.Craft[i] ) != i )
			fail( toString( "%s, line %u: %s serves the item part %c twice", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), craftStats.Craft[i] ) );
	}

	// the specialty and the weaknesses of the family in its first two item parts, by stat name
	craftStats.bestStatA = statIndex( cells[10], "rm_fam_prop.csv", nomMP );
	craftStats.worstStatA1 = statIndex( cells[11], "rm_fam_prop.csv", nomMP );
	craftStats.worstStatA2 = statIndex( cells[12], "rm_fam_prop.csv", nomMP );
	craftStats.bestStatB = statIndex( cells[13], "rm_fam_prop.csv", nomMP );
	craftStats.worstStatB1 = statIndex( cells[14], "rm_fam_prop.csv", nomMP );
	craftStats.worstStatB2 = statIndex( cells[15], "rm_fam_prop.csv", nomMP );
	// a specialty comes with one or two weaknesses, the second after the first, three different stats, of the part they
	// are given for, and none for a part the family does not serve: else the stats of the part would not add up to the
	// class, or a cell would be ignored
	const int profiles[2][3] = { { craftStats.bestStatA, craftStats.worstStatA1, craftStats.worstStatA2 },
		{ craftStats.bestStatB, craftStats.worstStatB1, craftStats.worstStatB2 } };
	static const char *s_Groups[] = { "first part (BestA, WorstA1, WorstA2)", "second part (BestB, WorstB1, WorstB2)" };
	for ( uint i = 0; i < 2; ++i )
	{
		const int *p = profiles[i];
		const bool given = p[0] >= 0 || p[1] >= 0 || p[2] >= 0;
		if ( given && i >= craftStats.Craft.size() )
			fail( toString( "%s, line %u: %s serves %u item parts and gives stats for its %s", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), (uint)craftStats.Craft.size(), s_Groups[i] ) );
		if ( ( p[1] >= 0 && p[0] < 0 ) || ( p[2] >= 0 && p[1] < 0 ) )
			fail( toString( "%s, line %u: %s gives a weakness in its %s without the specialty or the first weakness before it", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), s_Groups[i] ) );
		if ( p[0] >= 0 && p[1] < 0 )
			fail( toString( "%s, line %u: %s gives a specialty in its %s and no weakness to pay for it", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), s_Groups[i] ) );
		if ( ( p[0] >= 0 && ( p[0] == p[1] || p[0] == p[2] ) ) || ( p[1] >= 0 && p[1] == p[2] ) )
			fail( toString( "%s, line %u: %s gives one stat twice in the specialty and weaknesses of its %s", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), s_Groups[i] ) );
		for ( uint k = 0; i < craftStats.Craft.size() && k < 3; ++k )
		{
			// a stat the part does not carry would be dropped, and the stats of the family would not add up to its class
			if ( p[k] >= 0 && !craftParts[craftStats.Craft[i] - 'A'].Carac[p[k]] )
				fail( toString( "%s, line %u: %s gives %s in its %s, which the item part %c does not carry", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), s_StatNames[p[k]], s_Groups[i], craftStats.Craft[i] ) );
		}
	}

	// the colour: an index into the colours of a class and ecosystem, a number
	const CSString &color = cells[16];
	for ( uint i = 0; i < color.size(); ++i )
	{
		if ( color[i] < '0' || color[i] > '9' || color.size() > 3 )
			fail( toString( "%s, line %u: %s has the colour %s, which is a number up to 999", FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), color.c_str() ) );
	}
	craftStats.color = color.atoi();

	// the special items
	for ( uint i = 19; i < cells.size(); ++i )
	{
		if ( !cells[i].empty() )
			specialNames.insert( cells[i] );
	}

	// a creature family gives the materials of the model it is named after
	if ( loc == "C" && itemsAGenerer.find( nomMP.firstWord() ) == itemsAGenerer.end() )
		fail( toString( "%s, line %u: the creature family %s (location C) is named after no creature model of %s: its first word must be a model's name; add the model or give the family another location",
			FAM_PROP_CSV.c_str(), lineNumber, nomMP.c_str(), CREATURE_MODELS_CSV.c_str() ) );

	currentDocItem.push( DtRMFamily, nomMP );
	currentDocItem.push( DtGroup, groupe );

	// get raw mat number
	numMP = GetNumeroMP( nomMP, lineNumber );
	printf( "    Processing Family %d : %s\n", numMP, nomMP.c_str() );

	GetNumeroGroupe( groupe, lineNumber );


	// Add the MPFamily into the list
	if(numMP>=(sint)MPFamilies.size())
		MPFamilies.resize(numMP+1);
	MPFamilies[numMP].Name= nomMP;
	MPFamilies[numMP].Icon= icon;


	// raw mats found in deposits or goo
	if ( loc != "C" )
	{
		if ( !specialOnly )
		{
			// Generate items
			GenerateDepositItems( numMP, nomMP, craftStats, loc );
		}

		// on enregistre les items se trouvant dans les deposits
		if ( loc == "D" )
		{
			// an empty file, its name is what the World Editor lists for the "mps" of a deposit
			writeFile( CSString(), toString( "%s%s_%d.mp", DEPOSIT_MPS.c_str(), nomMP.toLower().replace( " ", "_" ).c_str(), numMP ) );
		}

		overlay = nomMP.firstWord().toUpper().left(6);
	}
	// looted raw mats
	else
	{
		GenerateCreatureItems( numMP, nomMP, craftStats );
	}


	// special items
	CSString codeSpecial, nouveauCode;
	int variation = 1;
	CSortedStringSet::const_iterator it = specialNames.begin();

	while ( it != specialNames.end() )
	{
		CSString name = (*it);

		nouveauCode = name.left(2).toLower();

		if ( nouveauCode == codeSpecial )
			variation++;
		else
			variation = 1;

		GenerateSpecialItem( numMP, nomMP, craftStats, loc, name, variation );
		codeSpecial = nouveauCode;
		it++;
	}


	// Create parent sheet for raw mat
	CreateParentSItem( numMP, nomMP, groupe, extraInfo.DropOrSell, icon, overlay );

	currentDocItem.reset( DtRMFamily );
	currentDocItem.reset( DtGroup );
	currentDocItem.reset( DtProp );
	currentDocItem.reset( DtCreature );
}


// Generate Primitive Necklace (special object)
void CreatePrimitiveNecklace()
{
	CSString output;
	
	output = "<?xml version=\"1.0\"?>\n";
	output += "<FORM Version=\"0.0\" State=\"modified\">\n";
	output += "  <PARENT Filename=\"_mc.sitem\"/>\n";
	output += "  <PARENT Filename=\"_m0696.sitem\"/>\n";
	output += "  <STRUCT>\n    <STRUCT Name=\"mp\">\n";
	output += "      <ATOM Name=\"MpColor\" Value=\"Beige\"/>\n";
	output += "      <ATOM Name=\"MaxQuality\" Value=\"250\"/>\n";
	output += "      <ATOM Name=\"StatEnergy\" Value=\"0\"/>\n";
	output += "    </STRUCT>\n  </STRUCT>\n  <STRUCT/>\n  <STRUCT/>\n";
	output += "  <STRUCT/>\n  <STRUCT/>\n</FORM>\n";
	writeFile( output, toString( "%scommon/m0696ixxcc01.sitem", MP_DIRECTORY.c_str() ) );

	// and its family, which rm_fam_prop.csv does not have
	CreateParentSItem( 696, "Invasion Reward", "Undefined", false, "PA_pendant.tga", "" );
	
	itemNames.insert( "m0696ixxcc01	Primitive Necklace	a	the	Primitive Necklaces		the" );

}


// Save names
void ItemNamesSave()
{
	printf( "-- SAVING ITEM NAMES --\n");
	CSString data;
	if ( !data.readFromFile( ITEM_WORDS_WK ) )
		fail( toString( "Cannot read %s", ITEM_WORDS_WK.c_str() ) );
	const string eol = data.contains( "\r\n" ) ? "\r\n" : "\n";

	// the names of this run that the file does not have yet, by item id; one name per id
	string newNames;
	uint nbNew = 0;
	set<string> ids;
	for ( CSortedStringSet::const_iterator it = itemNames.begin(); it != itemNames.end(); ++it )
	{
		const CSString id = (*it).splitTo( '\t' );
		if ( !ids.insert( toLowerAscii( id ) ).second )
			fail( toString( "Two names for the raw material %s: two lines of %s make the same sheet", id.c_str(), FAM_PROP_CSV.c_str() ) );
		if ( !data.contains( ( "\n" + id + "\t" ).c_str() ) )
		{
			newNames += (*it) + eol;
			++nbNew;
		}
	}
	if ( nbNew == 0 )
	{
		printf( "    %u names, all already in %s\n", (uint)itemNames.size(), ITEM_WORDS_WK.c_str() );
		return;
	}

	// in front of the prospector items, which follow the raw materials in the file
	const string anchor = eol + "prospector";
	const string text = data;
	const string::size_type pos = text.find( anchor );
	if ( pos == string::npos )
		fail( toString( "%s has no prospector line to add the raw material names before", ITEM_WORDS_WK.c_str() ) );
	writeFile( CSString( text.substr( 0, pos + eol.size() ) + newNames + text.substr( pos + eol.size() ) ), ITEM_WORDS_WK );
	printf( "    %u new names for %s\n", nbNew, ITEM_WORDS_WK.c_str() );
}


// Load Customized Properties defined in raw_material_generation.cfg
void LoadCustomizedProperties()
{
	CSString data, name, prop, val;

	printf( "-- REGISTERING CUSTOMIZED PROPERTIES --\n" );

	if ( !data.readFromFile( "raw_material_generation.cfg" ) )
		fail( "raw_material_generation.cfg not found in the working folder" );

	// the files this run makes, by file name in lower case: the properties go on them
	map<string, string> byName;
	for ( map<string, CPendingFile>::const_iterator it = pendingFiles.begin(); it != pendingFiles.end(); ++it )
		byName[toLowerAscii( CFile::getFilename( it->second.Path ) )] = it->first;

	// the quoted strings between "CustomizedProperties = {" and "};", three per property: sheet, property, value
	const string list = data.splitFrom( "CustomizedProperties" ).splitFrom( "{" ).splitTo( "};" );
	vector<CSString> values;
	for ( string::size_type pos = list.find( '"' ); pos != string::npos; pos = list.find( '"', pos ) )
	{
		const string::size_type close = list.find( '"', pos + 1 );
		if ( close == string::npos )
			fail( "raw_material_generation.cfg has a string with no closing quote in CustomizedProperties" );
		values.push_back( list.substr( pos + 1, close - pos - 1 ) );
		pos = close + 1;
	}
	if ( values.size() % 3 != 0 )
		fail( "raw_material_generation.cfg must give three strings per entry of CustomizedProperties: sheet, property, value" );

	for ( uint i=0; i<values.size(); i+=3 )
	{
		name = values[i];
		prop = values[i+1].replace( " ", "" );
		val = values[i+2].replace( " ", "" );

		TRMItem item;
		item.push( DtName, name );
		item.push( DtCustomizedProperties, prop );
		SortableData.updateItemAppend( item, DtName );

		CSString str, output;
		map<string, string>::const_iterator itName = byName.find( toLowerAscii( name ) );

		// a sheet this run makes
		if ( itName != byName.end() )
		{
			CSString zone = prop.splitTo( ".", true );
			CSString &content = pendingFiles[itName->second].Content;
			str = content;

			if ( !str.contains( zone.c_str() ) )
			{
				output = "<STRUCT>\n    <STRUCT Name=\"";
				output += toString( "%s\">\n      <ATOM Name=\"", zone.c_str() );
				output += toString( "%s\" Value=\"%s\"/>\n    </STRUCT>", prop.c_str(), val.c_str() );

				// check if property is not already inserted
				if ( !str.contains( output.c_str() ) )
				{
					str = str.replace( "<STRUCT>", output.c_str() );
					content = str;
				}
			}
			else
			{
				output = toString( "    <STRUCT Name=\"%s\">\n", zone.c_str() );
				output += toString( "      <ATOM Name=\"" );
				output += toString( "%s\" Value=\"%s\"/>\n", prop.c_str(), val.c_str() );

				// check if property is not already inserted
				if ( !str.contains( toString( "%s\" Value=\"%s\"/>\n", prop.c_str(), val.c_str() ).c_str() ) )
				{
					str = str.replace( toString( "    <STRUCT Name=\"%s\">\n", zone.c_str() ).c_str(), output.c_str() );
					content = str;
				}
			}
		}
		else
		{
			fail( toString( "raw_material_generation.cfg (CustomizedProperties) sets %s on %s, which this run does not make: correct the sheet name, or give the tables the material back",
				prop.c_str(), name.c_str() ) );
		}
	}
}


// Write the parent sheets the generated raw materials name: one per ecosystem (_mc.sitem to _mp.sitem) and one per
// creature model (_mchd.sitem, its icon overlay the model name of creature_models.csv as the deposit families have it)
void SaveParentSItems()
{
	printf( "-- SAVING ECOSYSTEM AND CREATURE MODEL PARENTS --\n" );

	struct CEcosystemParent { char Eco; const char *Ecosystem; const char *Background; };
	static const CEcosystemParent ecosystemParents[] = {
		{ 'c', "Common", "BK_generic.tga" },
		{ 'd', "Desert", "BK_fyros.tga" },
		{ 'f', "Forest", "BK_matis.tga" },
		{ 'j', "Jungle", "BK_zorai.tga" },
		{ 'l', "Lacustre", "BK_tryker.tga" },
		{ 'p', "PrimeRoots", "BK_primes.tga" },
	};
	for ( uint i=0; i<sizeof(ecosystemParents)/sizeof(ecosystemParents[0]); ++i )
	{
		CSString output = "<?xml version=\"1.0\"?>\n<FORM Version=\"0.0\" State=\"modified\">\n";
		output += "  <STRUCT>\n";
		output += "    <STRUCT Name=\"basics\">\n";
		output += "      <ATOM Name=\"family\" Value=\"raw material (mp)\"/>\n";
		output += "      <ATOM Name=\"stackable\" Value=\"99\"/>\n";
		output += "      <ATOM Name=\"Saleable\" Value=\"false\"/>\n";
		output += "    </STRUCT>\n";
		output += "    <STRUCT Name=\"mp\">\n";
		output += toString( "      <ATOM Name=\"Ecosystem\" Value=\"%s\"/>\n", ecosystemParents[i].Ecosystem );
		output += "      <ATOM Name=\"Category\" Value=\"Faber\"/>\n";
		output += "    </STRUCT>\n";
		output += "    <STRUCT Name=\"3d\">\n";
		output += toString( "      <ATOM Name=\"icon background\" Value=\"%s\"/>\n", ecosystemParents[i].Background );
		output += "    </STRUCT>\n";
		output += "  </STRUCT>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n</FORM>\n";
		writeFile( output, toString( "%s_parent/_m%c.sitem", MP_DIRECTORY.c_str(), ecosystemParents[i].Eco ) );
	}

	for ( set<CSString>::const_iterator it = generatedCreatureCodes.begin(); it != generatedCreatureCodes.end(); ++it )
	{
		// "chd": c, then the code of creature_models.csv
		map<CSString, CSString>::const_iterator itName = creatureModelNames.find( (*it).substr( 1 ) );
		if ( itName == creatureModelNames.end() || itName->second.empty() )
			fail( toString( "creature_models.csv has no name for the creature code of %s", (*it).c_str() ) );
		CSString output = "<?xml version=\"1.0\"?>\n<FORM Version=\"0.0\" State=\"modified\">\n";
		output += "  <STRUCT>\n";
		output += "    <STRUCT Name=\"3d\">\n";
		output += toString( "      <ATOM Name=\"text overlay\" Value=\"%s\"/>\n", itName->second.toUpper().left( 6 ).c_str() );
		output += "    </STRUCT>\n";
		output += "  </STRUCT>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n  <STRUCT/>\n</FORM>\n";
		writeFile( output, toString( "%s_parent/_m%s.sitem", MP_DIRECTORY.c_str(), (*it).c_str() ) );
	}
	printf( "    6 ecosystems, %u creature models\n", (uint)generatedCreatureCodes.size() );
}

// Stop if any file of the leveldesign names one of the files about to be removed (by its name without extension, as a
// whole word: primitives write sheet names without .sitem). Not read: this tool's own outputs (the raw material
// folders, the deposit family list, the family icons), its tables, the translation, the lists of all sheets the build
// makes (sheets.txt, sheet_id.bin), and git's and Python's folders; but raw_material_generation.cfg is. A file the run
// will write is read as it will be written, everything else as it is.
static void CheckNoReferences( const set<string> &names )
{
	// names of letters, digits and underscores are found as words; the few with ' or - (deposit family files) as text
	// between non-word characters
	vector<string> otherNames;
	for ( set<string>::const_iterator it = names.begin(); it != names.end(); ++it )
	{
		if ( it->find_first_of( "'-" ) != string::npos )
			otherNames.push_back( *it );
	}
	printf( "    looking for references to the %u files to remove\n", (uint)names.size() );
	const CSString skippedFolders[] = { MP_DIRECTORY + "_parent/", MP_DIRECTORY + "common/", MP_DIRECTORY + "desert/",
		MP_DIRECTORY + "forest/", MP_DIRECTORY + "jungle/", MP_DIRECTORY + "lacustre/", MP_DIRECTORY + "prime_roots/",
		DEPOSIT_MPS, TRANSLATION_PATH, TABLE_PATH, LEVEL_DESIGN_PATH + ".git/" };
	const CSString skippedFiles[] = { IC_FAMILIES_FORAGE_SOURCE, LEVEL_DESIGN_PATH + "game_elem/sheets.txt",
		LEVEL_DESIGN_PATH + "game_elem/sheet_id.bin" };
	vector<string> files;
	CPath::getPathContent( LEVEL_DESIGN_PATH, true, false, true, files );
	// and the cfg, whose CustomizedProperties name sheets
	files.push_back( CPath::getFullPath( "raw_material_generation.cfg", false ) );
	uint nbRead = 0;
	for ( uint i = 0; i < files.size(); ++i )
	{
		const string key = writtenKey( files[i] );
		bool skip = key.find( "/__pycache__/" ) != string::npos;
		for ( uint j = 0; !skip && j < sizeof( skippedFolders ) / sizeof( skippedFolders[0] ); ++j )
			skip = key.compare( 0, skippedFolders[j].size(), toLowerAscii( skippedFolders[j] ) ) == 0;
		for ( uint j = 0; !skip && j < sizeof( skippedFiles ) / sizeof( skippedFiles[0] ); ++j )
			skip = key == writtenKey( skippedFiles[j] );
		if ( skip )
			continue;
		// what the run will write there, or what is there now
		CSString text;
		const CSString *pending = pendingContent( files[i] );
		if ( pending )
			text = *pending;
		else if ( !text.readFromFile( files[i] ) )
			fail( toString( "Cannot read %s", files[i].c_str() ) );
		++nbRead;
		const string content = text;
		if ( !otherNames.empty() )
		{
			const string lower = toLowerAscii( content );
			for ( uint n = 0; n < otherNames.size(); ++n )
			{
				for ( string::size_type at = lower.find( otherNames[n] ); at != string::npos; at = lower.find( otherNames[n], at + 1 ) )
				{
					const string::size_type after = at + otherNames[n].size();
					const bool wordBefore = at > 0 && ( isalnum( (unsigned char)lower[at - 1] ) || lower[at - 1] == '_' );
					const bool wordAfter = after < lower.size() && ( isalnum( (unsigned char)lower[after] ) || lower[after] == '_' );
					if ( !wordBefore && !wordAfter )
						fail( toString( "%s names %s, which the tables no longer make and this run would remove: take that reference out, or give the tables the material back, before running again",
							files[i].c_str(), otherNames[n].c_str() ) );
				}
			}
		}
		// every word of letters, digits and underscores, in lower case
		string::size_type pos = 0;
		while ( pos < content.size() )
		{
			string::size_type end = pos;
			while ( end < content.size() && ( isalnum( (unsigned char)content[end] ) || content[end] == '_' ) )
				++end;
			if ( end > pos )
			{
				const string word = toLowerAscii( content.substr( pos, end - pos ) );
				if ( names.count( word ) )
					fail( toString( "%s names %s, which the tables no longer make and this run would remove: take that reference out, or give the tables the material back, before running again",
						files[i].c_str(), word.c_str() ) );
				pos = end;
			}
			else
				++pos;
		}
	}
	printf( "    %u files read, none names them\n", nbRead );
}


// The raw material sheets, their parents and the deposit family files that an earlier run wrote and this one does not:
// these folders hold what this tool makes from the tables, so a material the tables no longer have goes once all is
// written (RemoveStaleOutputs). Nothing may name them.
vector<string> staleFiles;
void FindStaleOutputs()
{
	printf( "-- RAW MATERIALS THE TABLES NO LONGER HAVE --\n" );
	struct COutputFolder { CSString Folder; const char *Extension; };
	const COutputFolder folders[] = { { MP_DIRECTORY + "_parent/", "sitem" }, { MP_DIRECTORY + "common/", "sitem" },
		{ MP_DIRECTORY + "desert/", "sitem" }, { MP_DIRECTORY + "forest/", "sitem" }, { MP_DIRECTORY + "jungle/", "sitem" },
		{ MP_DIRECTORY + "lacustre/", "sitem" }, { MP_DIRECTORY + "prime_roots/", "sitem" }, { DEPOSIT_MPS, "mp" } };
	vector<string> stale;
	set<string> staleSheets; // the names of the stale files without extension, in lower case
	for ( uint i = 0; i < sizeof( folders ) / sizeof( folders[0] ); ++i )
	{
		vector<string> files;
		CPath::getPathContent( folders[i].Folder, false, false, true, files );
		for ( uint j = 0; j < files.size(); ++j )
		{
			if ( CFile::getExtension( files[j] ) != folders[i].Extension || pendingFiles.count( writtenKey( files[j] ) ) )
				continue;
			stale.push_back( files[j] );
			staleSheets.insert( toLowerAscii( CFile::getFilenameWithoutExtension( files[j] ) ) );
		}
	}

	if ( !stale.empty() )
		CheckNoReferences( staleSheets );
	staleFiles = stale;
	printf( "    %u files to remove\n", (uint)stale.size() );
}


void RemoveStaleOutputs()
{
	printf( "-- REMOVING RAW MATERIALS THE TABLES NO LONGER HAVE --\n" );
	for ( uint i = 0; i < staleFiles.size(); ++i )
	{
		if ( !CFile::deleteFile( staleFiles[i] ) )
			fail( toString( "Cannot remove %s", staleFiles[i].c_str() ) );
	}
	printf( "    %u files removed\n", (uint)staleFiles.size() );
}


// Write what the run made, once every check has passed: the files whose content changes, in folders made as needed
void WritePendingFiles()
{
	printf( "-- WRITING --\n" );
	uint nbWritten = 0;
	for ( map<string, CPendingFile>::const_iterator it = pendingFiles.begin(); it != pendingFiles.end(); ++it )
	{
		const CPendingFile &file = it->second;
		CSString current;
		if ( CFile::fileExists( file.Path ) && ( ( file.Content.empty() && CFile::getFileSize( file.Path ) == 0 ) || ( current.readFromFile( file.Path ) && current == file.Content ) ) )
			continue;
		const string folder = CFile::getPath( file.Path );
		if ( !CFile::isDirectory( folder ) && !CFile::createDirectoryTree( folder ) )
			fail( toString( "Cannot create the folder %s", folder.c_str() ) );
		if ( file.Content.empty() )
		{
			// an empty file (the deposit family list), which CSString::writeToFile does not write
			FILE *f = nlfopen( file.Path, "wb" );
			if ( f == NULL )
				fail( toString( "Cannot write %s", file.Path.c_str() ) );
			fclose( f );
		}
		else if ( !file.Content.writeToFile( file.Path ) )
		{
			fail( toString( "Cannot write %s", file.Path.c_str() ) );
		}
		++nbWritten;
	}
	printf( "    %u files made, %u written (the others are as they were)\n", (uint)pendingFiles.size(), nbWritten );
}


// Generate _ic_families.forage_source()
void	SaveFamiliesForageSource()
{
	CSString output;

	printf( "-- GROUP ICONS FOR _ic_groups.forage_source --\n");
	
	output = "<?xml version=\"1.0\"?>\n";
	output+= "<FORM Revision=\"$Revision: 1.9 $\" State=\"modified\">\n";
	output+= "    <STRUCT>\n";
	output+= "    <ARRAY Name=\"Icons\">\n";

	// the icons in the file now, for the families that this run does not generate (they are in item_mp_family.typ
	// but not in rm_fam_prop.csv): the array is indexed by family number, so those keep their place and their icon
	vector<string> previousIcons;
	{
		CSString previous;
		previous.readFromFile( IC_FAMILIES_FORAGE_SOURCE );
		const string text = previous.splitFrom( "<ARRAY Name=\"Icons\">" ).splitTo( "</ARRAY>" );
		// each atom of the array is an icon, "<ATOM Value="..."/>", or "<ATOM/>" when it is empty
		const string atom = "<ATOM";
		const string value = "Value=\"";
		for ( string::size_type pos = text.find( atom ); pos != string::npos; pos = text.find( atom, pos ) )
		{
			const string::size_type end = text.find( '>', pos );
			if ( end == string::npos )
				fail( toString( "%s has an icon atom with no end", IC_FAMILIES_FORAGE_SOURCE.c_str() ) );
			const string element = text.substr( pos, end - pos );
			const string::size_type v = element.find( value );
			if ( v == string::npos )
			{
				previousIcons.push_back( string() );
			}
			else
			{
				const string::size_type close = element.find( '"', v + value.size() );
				if ( close == string::npos )
					fail( toString( "%s has an icon with no closing quote", IC_FAMILIES_FORAGE_SOURCE.c_str() ) );
				previousIcons.push_back( element.substr( v + value.size(), close - v - value.size() ) );
			}
			pos = end + 1;
		}
	}

	const uint nbIcons = (uint)std::max( MPFamilies.size(), previousIcons.size() );
	uint nbKept = 0;
	for ( uint i=0; i!=nbIcons; ++i )
	{
		const bool generated = i < MPFamilies.size() && !MPFamilies[i].Name.empty();
		if ( !generated && i < previousIcons.size() && !previousIcons[i].empty() )
			++nbKept;
		const string icon = generated ? string( MPFamilies[i].Icon ) : ( i < previousIcons.size() ? previousIcons[i] : string() );
		output+= toString("      <ATOM Value=\"%s\"/>\n", icon.c_str());
	}
	printf( "    %u icons, %u of families this run does not generate kept\n", nbIcons, nbKept );

    output+= "    </ARRAY>\n";
	output+= "  </STRUCT>\n";
	output+= "  <STRUCT/>\n";
	output+= "  <STRUCT/>\n";
	output+= "  <STRUCT/>\n";
	output+= "  <STRUCT/>\n";
	output+= "  <LOG></LOG>\n";
	output+= "</FORM>\n";

	writeFile( output, IC_FAMILIES_FORAGE_SOURCE );
}


// Generate documentation
void GenerateDoc()
{
	CProducedDocHtml			MainDoc;
	CFile::createDirectory("doc");
	MainDoc.open( "doc/rm.html", "Raw materials by generation order", true );
	MainDoc.write( "<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n" );
	MainDoc.write( "<tr>" );
	for ( uint32 c=0; c!=DtNbCols; ++c )
	{
		MainDoc.write( "<td><b><a href=\"rm_" + string(DataColStr[c]) + ".html\">" + string(DataColStr[c]) + "</a></b></td>" );
	}
	MainDoc.write( "</tr>" );
	for ( CRMData::CItems::const_iterator isd=SortableData.items().begin(); isd!=SortableData.items().end(); ++isd )
	{
		MainDoc.write( (*isd).toHTMLRow() );
	}
	MainDoc.write( "</tbody><table>\n" );
	
	// Produce alt docs
	CProducedDocHtml			AltDocs[DtNbCols];
	for ( uint32 c=0; c!=DtNbCols; ++c )
	{
		AltDocs[c].open( "doc/rm_" + string(DataColStr[c]) + ".html", "Raw materials by " + string(DataColStr[c]), true );
		AltDocs[c].write( "<table cellpadding=\"1\" cellspacing=\"1\" border=\"0\"><tbody>\n" );
		AltDocs[c].write( "<tr>" );
		for ( uint32 cc=0; cc!=DtNbCols; ++cc )
			if ( cc == c )
				AltDocs[c].write( "<td><b>" + string(DataColStr[cc]) + "</b></td>" );
			else
				AltDocs[c].write( "<td><b><a href=\"rm_" + string(DataColStr[cc]) + ".html\">" + string(DataColStr[cc]) + "</a></b></td>" );
		AltDocs[c].write( "</tr>" );
		string previousKey = "[NO PREVIOUS]"; // not a blank string, because it may be a valid value
		string previousName;
		for ( CRMData::CLookup::const_iterator isd=SortableData.lookup( c ).begin(); isd!=SortableData.lookup( c ).end(); ++isd )
		{
			const TRMItem& item = SortableData.getRow( (*isd).second );
			AltDocs[c].write( item.toHTMLRow( c, (*isd).first, previousKey, DtName, previousName ) );

			previousKey = (*isd).first;
			previousName = item.Fields[DtName][0];
		}
		AltDocs[c].write( "</tbody><table>\n" );
		AltDocs[c].save();
	}
}


// A folder of raw_material_generation.cfg (Key = "path"), with forward slashes and a final one
static CSString cfgPath( const CSString &cfg, const char *key )
{
	const CSString value = cfg.splitFrom( toString( "%s = \"", key ).c_str() ).splitTo( "\"" );
	if ( value.empty() )
		fail( toString( "raw_material_generation.cfg gives no %s", key ) );
	return CPath::standardizePath( value, true );
}


// Initialize directories from raw_material_generation.cfg
void SetupDirectories()
{
	CSString data;
	
	if ( !data.readFromFile( "raw_material_generation.cfg" ) )
		fail( "raw_material_generation.cfg not found in the working folder" );

	// beurk :s Use CConfigFile instead
	LEVEL_DESIGN_PATH = cfgPath( data, "LevelDesignPath" );
	TRANSLATION_PATH = cfgPath( data, "TranslationPath" );
	CREATURE_PATH = cfgPath( data, "CreaturePath" );
	RAW_MATERIAL_ASSIGN = cfgPath( data, "RawMaterialAssignmentPath" );
	TABLE_PATH = cfgPath( data, "TablePath" );

	printf( "Level Design Path : %s\nTranslation Path : %s\nCreature Path : %s\nRaw Material Assignment Path : %s\nTable Path : %s\n\n",
		LEVEL_DESIGN_PATH.c_str(), TRANSLATION_PATH.c_str(), CREATURE_PATH.c_str(), RAW_MATERIAL_ASSIGN.c_str(), TABLE_PATH.c_str() );

	// Ryzom Core's raw material tables
	FAM_PROP_CSV = TABLE_PATH + "rm_fam_prop.csv";
	ITEM_PARTS_CSV = TABLE_PATH + "rm_item_parts.csv";
	CREATURE_MODELS_CSV = TABLE_PATH + "creature_models.csv";
	CREATURE_CLASSES_CSV = TABLE_PATH + "rm_creature_classes.csv";
	const CSString tables[] = { FAM_PROP_CSV, ITEM_PARTS_CSV, CREATURE_MODELS_CSV, CREATURE_CLASSES_CSV };
	for ( uint i = 0; i < sizeof( tables ) / sizeof( tables[0] ); ++i )
	{
		if ( !CFile::fileExists( tables[i] ) )
			fail( toString( "%s not found: TablePath of raw_material_generation.cfg names the folder of Ryzom Core's raw material tables, scripts/raw_material_tables in the leveldesign, where make_tables.py writes them",
				tables[i].c_str() ) );
	}

	ITEM_MP_FAMILY_TYP = LEVEL_DESIGN_PATH + "DFN/game_elem/_item/item_mp_family.typ";
	ITEM_MP_GROUPE_TYP = LEVEL_DESIGN_PATH + "DFN/game_elem/_item/item_mp_group.typ";
	ITEM_MP_PARAM_DFN = LEVEL_DESIGN_PATH + "DFN/game_elem/_item/_item_mp_param.dfn";
	MP_DIRECTORY = LEVEL_DESIGN_PATH + "game_element/sitem/raw_material/";
	DEPOSIT_MPS = LEVEL_DESIGN_PATH + "game_element/deposit_system/mps/";

	IC_FAMILIES_FORAGE_SOURCE = LEVEL_DESIGN_PATH + "game_element/forage_source/_ic_families.forage_source";

	WK_UXT = TRANSLATION_PATH + "work/wk.uxt";
	ITEM_WORDS_WK = TRANSLATION_PATH + "work/item_words_wk.txt";

	// what is read must be there
	const CSString inputs[] = { ITEM_MP_FAMILY_TYP, ITEM_MP_GROUPE_TYP, ITEM_MP_PARAM_DFN, WK_UXT, ITEM_WORDS_WK };
	for ( uint i=0; i<sizeof(inputs)/sizeof(inputs[0]); ++i )
	{
		if ( !CFile::fileExists( inputs[i] ) )
			fail( toString( "%s not found, check the paths of raw_material_generation.cfg", inputs[i].c_str() ) );
	}
	if ( !CFile::isDirectory( CREATURE_PATH ) )
		fail( toString( "The creature folder %s (CreaturePath of raw_material_generation.cfg) not found", CREATURE_PATH.c_str() ) );
	if ( !CFile::isDirectory( RAW_MATERIAL_ASSIGN ) )
		fail( toString( "The folder %s (RawMaterialAssignmentPath of raw_material_generation.cfg) not found: it holds the creatures' quartering tables, game_elem/creature in the leveldesign",
			RAW_MATERIAL_ASSIGN.c_str() ) );
}


// Browse all raw mats families to do all related processes
void LoadFamillesMP()
{
	printf( "-- LOADING RAW MATERIAL FAMILIES --\n" );

	// Preload wk.uxt, item_mp_family.typ, and item_mp_group.typ (avoid to reload them each time)
	FamilyTypContent.readFromFile( ITEM_MP_FAMILY_TYP );
	GroupTypContent.readFromFile( ITEM_MP_GROUPE_TYP );
	WKContent.readFromFile( WK_UXT );
	
	// avoid huge resize of vector<string>
	MPFamilies.reserve(1000);

	CSString fileData, ligne;

	if ( !fileData.readFromFile( FAM_PROP_CSV ) )
		fail( toString( "Cannot read %s", FAM_PROP_CSV.c_str() ) );

	// the lines without their carriage returns: one left at the end would be read as a special item of the family
	fileData = fileData.replace( "\r", "" );

	// every line, a blank one included: stopping at it would leave the families after it out, and their sheets removed
	uint lineNumber = 0;
	while ( !fileData.empty() )
	{
		ligne = fileData.splitTo( "\n", true );
		++lineNumber;
		if ( ligne.strip().empty() )
			continue;
		NewMP( ligne, lineNumber );
	}

	// we manually add Primitive Necklace because it's not specified in raw mats list
	CreatePrimitiveNecklace();
}


// Programme principal
int main( int argc, char* argv[] )
{
	new CApplicationContext;
	SortableData.init( true );
	
	SetupDirectories();

	LoadCraftParts();

	LoadCreatureFiles();

	LoadAssignmentFiles();

	InitCreatureMP();

	LoadCreatureClasses();

	// everything is made in memory and checked first: a stop leaves the leveldesign as it was
	LoadFamillesMP();

	SaveAssignments();

	SaveParentSItems();

	ItemNamesSave();

	LoadCustomizedProperties();

	SaveFamiliesForageSource();

	FindStaleOutputs();

	// then written
	WritePendingFiles();

	RemoveStaleOutputs();
		
	printf( "-- GENERATING DOCUMENTATION --\n" );
	try
	{
		GenerateDoc();
	}
	catch(const Exception &e)
	{
		nlwarning("%s", e.what());
		nlwarning("HTML Doc generation failed\n");
	}

	printf( "-- DONE --\n" );
	
	return 0;
}
