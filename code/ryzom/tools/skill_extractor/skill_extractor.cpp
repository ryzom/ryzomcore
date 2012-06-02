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




#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/config_file.h"

#include "string.h"

#include <vector>
#include <string>

using namespace std;
using namespace NLMISC;

// define macro for outputone code line
#define outLine( s )\
{\
	out = string(s);\
	fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );\
}\


// set of code
static set<string>	Codes;

struct CSkill
{
	string		SkillName;
	string		NormalizedSkillName;
	CSkill		*ParentSkillPtr;
	string		ParentSkill;
	uint16		MaxValue;
	string		Code;
	uint8		StageType;
	string		MainCategory;
	string		SecondaryCategory;
	vector<CSkill*> Children;

	CSkill() : ParentSkillPtr(NULL),MaxValue(0),StageType(0)
	{}

	CSkill(const CSkill &skill)
	{
		*this = skill;
	}

	CSkill &operator=(const CSkill &skill)
	{
		SkillName = skill.SkillName;
		NormalizedSkillName = skill.NormalizedSkillName;
		ParentSkillPtr = skill.ParentSkillPtr;
		ParentSkill = skill.ParentSkill;
		MaxValue = skill.MaxValue;
		Code = skill.Code;
		StageType = skill.StageType;
		MainCategory = skill.MainCategory;
		SecondaryCategory = skill.SecondaryCategory;
		Children = skill.Children;

		return *this;
	}

	void skillName(string name)
	{	
		SkillName = name;
/*		// fast correction to avoid strange cases when names end with a ' '
		sint i = SkillName.size();
		for ( ; i > 0; i-- )
		{
			if ( SkillName[i-1] != ' ' )
				break;
		}
		SkillName.resize( i );

		uint idxSpace;
		char c[2];
		c[0] = SkillName.substr( 0, 1).c_str()[0] - 32;
		c[1] = 0;
		NormalizedSkillName = string( c ) + SkillName.substr( 1 );
		while( ( idxSpace = NormalizedSkillName.find(" ") ) != string::npos )
		{
			string skillNameTmp = NormalizedSkillName.substr( 0, idxSpace );
			if( idxSpace < ( SkillName.size() - 1 ) )
			{
				c[0] = (*NormalizedSkillName.substr( idxSpace + 1, 1 ).c_str());
				if( c[0] >= 'a' ) c[0] -= 32;
				skillNameTmp = skillNameTmp + string( c ) + NormalizedSkillName.substr( idxSpace + 2 );
			}
			NormalizedSkillName = skillNameTmp;
		}
*/
		NormalizedSkillName = SkillName;
	}

	void buildCode()
	{
		if (ParentSkillPtr != NULL)
			Code = ParentSkillPtr->Code + Code;

		Codes.insert( Code );

		for (uint i = 0 ; i < Children.size() ; ++i)
			Children[i]->buildCode();
	}

	void writeInSheet(COFile &fo)
	{
		/*
	  <STRUCT Name="AccurateBleedingShot">
        <ATOM Name="Skill" Value="acurate bleeding shot "/>
        <ATOM Name="SkillCode" Value="none"/>
        <ATOM Name="MaxSkillValue" Value="50"/>
        <ARRAY Name="ChildSkills">
          <ATOM Name="AccurateBreathlessShot" Value="acurate breathless shot"/>
          <ATOM Name="AnimalSlideSlip" Value="animal slideslip"/>
	    </ARRAY>
	  </STRUCT>
	  */
		string out;
		out = string("        <STRUCT Name=\"")+ NormalizedSkillName + string("\">\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("          <ATOM Name=\"Skill\" Value=\"")+ SkillName + string("\"/>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("          <ATOM Name=\"SkillCode\" Value=\"")+ Code + string("\"/>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("          <ATOM Name=\"MaxSkillValue\" Value=\"")+ toString(MaxValue) + string("\"/>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("          <ATOM Name=\"Type of Stage\" Value=\"")+ toString(StageType) + string("\"/>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		
		if (ParentSkillPtr != NULL)
		{
			out = string("          <ATOM Name=\"ParentSkill\" Value=\"")+ ParentSkill + string("\"/>\n");			
		}
		else
		{
			out = string("          <ATOM Name=\"ParentSkill\" Value=\"\"/>\n");
		}
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );

		if( !Children.empty())
		{
			out = string("          <ARRAY Name=\"ChildSkills\">\n");
			fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
			for (uint i = 0 ; i < Children.size() ; ++i)
			{
				out = string("            <ATOM Name=\"") + Children[i]->NormalizedSkillName + string("\" Value=\"")+ Children[i]->SkillName + string("\"/>\n");
				fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
			}
			out = string("          </ARRAY>\n");
			fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		}
		out = string("        </STRUCT>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );

		for (uint i = 0 ; i < Children.size() ; ++i)
			Children[i]->writeInSheet(fo);
	}
};

struct CSkillTree
{
	void buildCode()
	{
		for (uint i = 0 ; i < RootSkills.size() ; ++i)
			RootSkills[i]->buildCode();
	}

	vector<CSkill*> RootSkills;
};

static CSkillTree SkillTree;
static map< string, CSkill> SkillNameToStruct;
char separators[]   = ";,";


//-----------------------------------------------
//	main
//
//-----------------------------------------------
sint main( sint argc, char ** argv )
{
	/////////////////////////////////////////////////////////////////////////////////////
	// Somes working variables
	char buffer[4096];
	
	// vector contained selector category
	vector< string > selector;

	/////////////////////////////////////////////////////////////////////////////////////
	// Check number of arguments
	if( argc < 4 )
	{
		printf("Create a file .typ for george contained a subset of skills\n\n");
		printf("SKILL_EXTRACTOR <output file> <input file skills> <[<selector> ...]\n");
		printf("	param 1 : create tree (yes) or no (no)\n");
		printf("    if output file have .typ extension, generate .typ george file format\n");
		printf("    if output file have .dfn extension, generate .dfn george file format\n");
		printf("    param 3 is sheet2 of SkillCategory.xls exported in csv format\n");
		printf("    Selector is category present in input file, selectors begins by '+' for or operation and by '.' for and operation\n");
		printf("    the first selector must be or operation (begin by '+' character)\n");
		return 1;
	}

	// parse the config file
	CConfigFile configFile;
	try
	{
		configFile.load( "skill_extractor.cfg" );
	}
	catch(const Exception &e ) 
	{
		nlwarning("<CShopTypeManager::initShopBase> skill_extractor.cfg %s",e.what());
		return 1;
	}

	//get the csv file
	CConfigFile::CVar * cfgVar = configFile.getVarPtr("CsvDir");
	if (!cfgVar)
	{
		printf("var 'CsvDir' not found in the skill_extractor.cfg");
		return 1;
	}
	const string& CSVDir = cfgVar->asString() + string("/");

	//get the path for the generated source files
	cfgVar = configFile.getVarPtr("SrcDir");
	if (!cfgVar)
	{
		printf("var 'SrcDir' not found in the skill_extractor.cfg");
		return 1;
	}
	const string& srcDir = cfgVar->asString() + string("/");

	//get the path for the generated source files
	cfgVar = configFile.getVarPtr("PdsDir");
	if (!cfgVar)
	{
		printf("var 'PdsDir' not found in the skill_extractor.cfg");
		return 1;
	}
	const string& pdsDir = cfgVar->asString() + string("/");

	//get the path for the generated dfn
	cfgVar = configFile.getVarPtr("DfnDir");
	if (!cfgVar)
	{
		printf("var 'DfnDir' not found in the skill_extractor.cfg");
		return 1;
	}
	const string& dfnDir = cfgVar->asString() + string("/");

	//get the path for the generated skill tree
	cfgVar = configFile.getVarPtr("SkillTreeDir");
	if (!cfgVar)
	{
		printf("var 'DfnDir' not found in the skill_extractor.cfg");
		return 1;
	}
	const string& treeDir = cfgVar->asString() + string("/");

	
	/////////////////////////////////////////////////////////////////////////////////////
	// Export .typ and .dfn file
	// open skill file
	CIFile f;
	if( ! f.open( CSVDir + string( argv[3] ) ) )
	{
		nlwarning( "File %s open failed", argv[3] );
		return 1;
	}
	
	// read all input file
	uint col;
	string skillName;
	char * ptr;
	map< string, CSkill>::const_iterator itSkillStruct;

	while( ! f.eof() )
	{
		f.getline( buffer, 4096 );
		col = 0;
		ptr = strtok( buffer, separators );
		CSkill skill;
		while( ptr && string( ptr ) != string(" ") )
		{			
			switch(col)
			{
			case 0: // skill name
				{
					skillName = strupr( string( ptr ) );					
					vector< string > emptyVectorOfString;
					skill.skillName(skillName);
				}
				break;
			case 1: // code
				skill.Code = toUpper(string( ptr ));
				break;		
			case 2: // parent skill
				skill.ParentSkill = toUpper(string( ptr ));
				break;
			case 3: // max skill value
				NLMISC::fromString(std::string(ptr), skill.MaxValue);
				break;
			case 4: // stage type
				NLMISC::fromString(std::string(ptr), skill.StageType);
				break;
			case 5: // main category
				skill.MainCategory = string( ptr );
				break;
			case 6: // secondary category
				skill.SecondaryCategory = string( ptr );
				break;
			default: // error ?
				break;
			};

			++col;
			ptr = strtok( 0, separators );			
		}

		if ( !skill.SkillName.empty())
		{
			// insert skill in the Map
			//pair< map< string, CSkill>::const_iterator, bool> skillInsert = SkillNameToStruct.insert( make_pair(skill.SkillName, skill) );
			//nlinfo("Insert skill %s, parent %s", skill.SkillName.c_str(), skill.ParentSkill.c_str() );
			SkillNameToStruct.insert( make_pair(skill.SkillName, skill) );
		}
	}
	f.close();

	// create the tree
	map< string, CSkill>::iterator itSkill;
	map< string, CSkill>::iterator itSkillEnd = SkillNameToStruct.end();
	uint count = 0;
	for ( itSkill = SkillNameToStruct.begin() ; itSkill != itSkillEnd ; ++itSkill )
	{
		++count;
		if ( (*itSkill).second.ParentSkill == string("NONE") || (*itSkill).second.ParentSkill.empty() )
		{
			SkillTree.RootSkills.push_back(&((*itSkill).second));
		}
		else
		{
			map< string, CSkill>::iterator its = SkillNameToStruct.find((*itSkill).second.ParentSkill);
			if (its == itSkillEnd)
			{
				nlwarning("ERROR : cannot find the parent skill %s for skill %s (skill %u)", (*itSkill).second.ParentSkill.c_str(), (*itSkill).second.SkillName.c_str(), count );
				nlstop;
			}			
			(*itSkill).second.ParentSkillPtr = &((*its).second);
			(*its).second.Children.push_back(&((*itSkill).second));
		}
	}

	SkillTree.buildCode();

	COFile fo;
	// create the skill tree if first param == yes
	if ( string( argv[1] ) == string("yes") )
	{
		if( ! fo.open( treeDir + string("skills.skill_tree"), false ) )
		{
			nlwarning(" Can't open file skills.skill_tree for writing");
			return 1;
		}
		
		string out("<?xml version=\"1.0\"?>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("<FORM Version=\"0.2\" State=\"modified\">\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("	<STRUCT>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("		<ARRAY Name=\"SkillData\">\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		
		for ( vector<CSkill*>::const_iterator itTree = SkillTree.RootSkills.begin() ; itTree != SkillTree.RootSkills.end() ; ++itTree)
		{
			(*itTree)->writeInSheet(fo);
		}

		out = string("		</ARRAY>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("	</STRUCT>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("</FORM>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		fo.close();

		// create the code .typ
		if( ! fo.open( string("_skillsCode.typ"), false ) )
		{
			nlwarning(" Can't open file _skillsCode.typ for writing");
			return 1;
		}
		
		out = string("<TYPE Type=\"String\" UI=\"NonEditableCombo\" Default=\"None\" Version=\"0.1\" State=\"modified\">\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("  <DEFINITION Label=\"unknown\" Value=\"unknown\"/>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		
		set<string>::const_iterator itCode;
		for ( itCode = Codes.begin() ; itCode != Codes.end() ; ++itCode )
		{
			out = string("  <DEFINITION Label=\"") +  (*itCode) + string("\" Value=\"") + (*itCode) + string("\"/>\n");
			fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		}

		out = string("</TYPE>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		fo.close();
	}

		
	// read category in command line
	for( sint i = 4; i < argc; ++i )
	{
		selector.push_back( string( argv[ i ] ) );
	}

	// generate a file containing skills and associated Code
	if( ! fo.open( string( "skill_codes.txt" ) ) )
	{
		nlwarning(" Can't open file %s for writing", "skill_codes.txt" );
		return 1;
	}
	for ( itSkill = SkillNameToStruct.begin() ; itSkill != itSkillEnd ; ++itSkill)
	{
		string out;
		string space;
		if ( (*itSkill).second.NormalizedSkillName.size() < 50)
			space.resize( 50 - (*itSkill).second.NormalizedSkillName.size(), ' ' );
		outLine((*itSkill).second.NormalizedSkillName + space + string("\t") + (*itSkill).second.Code + string("\n") );
	}


	// generate .typ or .dfn file	
	if( ! fo.open( dfnDir + string( argv[2] ), false ) )
	{
		nlwarning(" Can't open file %s for writing", argv[2] );
		return 1;
	}

	// output header of .typ or .dfn file
	string out("<?xml version=\"1.0\"?>\n");
	fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
	if( string( argv[2] ).find(".typ") != string::npos )
	{
		out = string("<TYPE Type=\"String\" UI=\"NonEditableCombo\" Default=\"unknown\" Version=\"0.1\" State=\"modified\">\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		out = string("  <DEFINITION Label=\"unknown\" Value=\"unknown\"/>\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
	}
	else
	{
		out = string("<DFN Version=\"0.0\" State=\"modified\">\n");
		fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
	}

	// parse all skills to export selected ones
	for ( itSkillStruct = SkillNameToStruct.begin() ; itSkillStruct != SkillNameToStruct.end() ; ++itSkillStruct )
	{
		bool selected = false;
		bool found = false;

		const CSkill &skill = (*itSkillStruct).second;

		for( vector< string >::iterator its = selector.begin(); its != selector.end(); ++its )
		{			
			found = false;

			if ( skill.MainCategory == (*its).substr(1) )
			{
				found = true;
			}
			else if ( skill.SecondaryCategory == (*its).substr(1) )
			{
				found = true;
			}
		
			if( found )
			{
				if( (*its).substr( 0, 1) == string("+") ) // or operation
				{
					selected = true;
				}
				else if( (*its).substr( 0, 1) == string(".") ) // and operation
				{
					selected &= true;
				}
			}
			else if( (*its).substr( 0, 1) == string(".") )
			{
				selected = false;
			}
		}

		if( selected )
		{
			if( string( argv[2] ).find(".typ") != string::npos )
			{
				out = string("  <DEFINITION Label=\"") + skill.SkillName + string("\" Value=\"") + skill.NormalizedSkillName + string("\"/>\n");
			}
			else
			{
				out = string("  <ELEMENT Name=\"") + skill.NormalizedSkillName + string("\" Type=\"Type\" Filename=\"creature_stat.typ\"/>\n");
			}
			fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
		}
	}

	if( string( argv[2] ).find(".typ") != string::npos )
	{
		out = string("</TYPE>\n");
	}
	else
	{
		out = string("</DFN>\n");
	}
	fo.serialBuffer( (uint8 *) const_cast< char * >(out.c_str()), (uint)out.size() );
	fo.close();

	/////////////////////////////////////////////////////////////////////////////////////
	// Generate skill.cpp and skill.h code file
	

	// output begin skill.h file
	//if( ! fo.open( string( "r:/code/ryzom/src_v2/game_share/skills.h" ) ) )
	if( ! fo.open( srcDir + string( "skills.h" ) ) )
	{
		nlwarning(" Can't open file %s for writing", "skills.h" );
		return 1;
	}

	// write header of header file
	outLine("/** \\file skills.h\n");
	outLine(" * skills enumeration: generated by skill extractor program\n");
	outLine(" *\n");
	outLine(" */\n");
	outLine("\n");
	outLine("#ifndef RY_SKILLS_H\n");
	outLine("#define RY_SKILLS_H\n");
	outLine("\n");
	outLine("#include \"nel/misc/types_nl.h\"\n");
	outLine("\n");
	outLine("#include <string.h>\n");
	outLine("\n");
	outLine( string("// NbSkills in enum : ") + toString( SkillNameToStruct.size() ) + string(" Report this in database.xml \n\n") );
	outLine("namespace SKILLS\n");
	outLine("{\n");
	outLine("	enum ESkills\n");
	outLine("	{\n");
	
	itSkill = SkillNameToStruct.begin();
	if (itSkill != itSkillEnd)
	{
		outLine(string("		") + (*itSkill).second.NormalizedSkillName + string(" = 0,\n") );
		for ( ++itSkill; itSkill != itSkillEnd ; ++itSkill)
		{
			outLine(string("		") + (*itSkill).second.NormalizedSkillName + string(",\n") );
		}
	}
	// output end skill enum and skill type enum and skill api
	outLine("\n");
	outLine("		NUM_SKILLS,\n");
	outLine("		unknown,\n");
	outLine("	};\n");
	outLine("\n");

	// output all skills
/*	for( it = skillsAndSelector.begin(); it != skillsAndSelector.end(); ++it )
	{
		uint idxSpace;
		out = (*it).first;
		while( ( idxSpace = out.find(" ") ) != string::npos )
		{
			string tmp = out.substr( 0, idxSpace );
			if( idxSpace < ( out.size() - 1 ) )
			{
				tmp = tmp + string("_") + out.substr( idxSpace + 1 );
			}
			out = tmp;
		}
		outLine( string("		") + out + string(",\n") );
	}

	// output end skill enum and skill type enum and skill api
	outLine("\n");
	outLine("		NUM_SKILLS,\n");
	outLine("		unknown\n");
	outLine("	};\n");
	outLine("\n");

	outLine("	enum ESkillType\n");
	outLine("	{\n");
	outLine("		skill,\n");
	outLine("		specialized_skill,\n");
	outLine("		training_characteristic,\n");
	outLine("		training_resist,\n");
	outLine("		training_score,\n");
	outLine("\n");
	outLine("		unknown_skill_type\n");
	outLine("	};\n");
	outLine("\n");
*/
	outLine("	/**\n");
	outLine("	 * get the right skill enum from the input string\n");
	outLine("	 * \\param str the input string\n");
	outLine("	 * \\return the ESkills associated to this string (Unknown if the string cannot be interpreted)\n");
	outLine("	 */\n");
	outLine("	ESkills	toSkill ( const std::string &str );\n");
	outLine("\n");
	outLine("	/**\n");
	outLine("	 * get the right skill string from the gived enum\n");
	outLine("	 * \\param skill the skill to convert\n");
	outLine("	 * \\return the string associated to this enum number (Unknown if the enum number not exist)\n");
	outLine("	 */\n");
	outLine("	const std::string& toString( uint16 skill );\n");
	outLine("\n");
	outLine("	/**\n");
	outLine("	 * get the skill category name\n");
	outLine("	 * \\param s is the enum number\n");
	outLine("	 * \\return the string name of skill type (Unknown if the enum number not exist)\n");
	outLine("	 */\n");
	outLine("	const std::string& getSkillCategoryName( uint16 s );\n");
	outLine("\n");
	outLine("}; // SKILLS\n");
	outLine("\n");
	outLine("#endif // RY_SKILLS_H\n");
	outLine("/* End of skills.h */\n");

	/////////////////////////////////////////////////////////////////////////////////////
	// begin output skill.cpp file
//	if( ! fo.open( string( "r:/code/ryzom/src_v2/game_share/skills.cpp" ) ) )
	if( ! fo.open( srcDir + string( "skills.cpp" ) ) )
	{
		nlwarning(" Can't open file skills.cpp for writing");
		return 1;
	}

	outLine("/** \\file skills.cpp\n");
	outLine(" * \n");
	outLine(" */\n\n");

	outLine("#include \"stdpch.h\"\n");
	outLine("\n");
	outLine("#include \"nel/misc/debug.h\"\n");
	outLine("#include \"skills.h\"\n");
	outLine("#include \"nel/misc/string_conversion.h\"\n");
	outLine("\n");
	outLine("using namespace std;\n");
	outLine("using namespace NLMISC;\n");
	outLine("\n");
	outLine("namespace SKILLS\n");
	outLine("{\n");
	outLine("\n");
	outLine("static string UnknownString(\"Unknown\");\n");
	outLine("\n");
	outLine("\tNL_BEGIN_STRING_CONVERSION_TABLE (ESkills)\n");

	// parser all skills and init the conversion map
	for ( itSkill = SkillNameToStruct.begin() ; itSkill != itSkillEnd ; ++itSkill)
	{
		outLine (string ("\t  NL_STRING_CONVERSION_TABLE_ENTRY(") + (*itSkill).second.NormalizedSkillName + string(")\n") );
	}
	//outLine (string ("	{ \"unknown\", unknown },\n" ) );
	outLine (string("\t  NL_STRING_CONVERSION_TABLE_ENTRY(unknown)\n") );
	
	outLine("\tNL_END_STRING_CONVERSION_TABLE(ESkills, SkillsConversion, unknown)\n");
	outLine("\n");
	outLine("\n");
	outLine("\tESkills toSkill( const std::string &str )\n");
	outLine("\t{\n");
	outLine("\t	return SkillsConversion.fromString(str);\n");
	outLine("\t}\n");
	outLine("\n");
	outLine("\tconst std::string& toString( uint16 skill )\n");
	outLine("\t{\n");
	outLine("\t	return SkillsConversion.toString((ESkills)skill);\n");
	outLine("\t}\n");
	outLine("\n");
	outLine("\n");
	outLine("\tconst std::string& getSkillCategoryName( uint16 s )\n");
	outLine("\t{\n");
	/*outLine("	if( s < sizeof(SkillCategoryStrings)/sizeof(SkillCategoryStrings[0]) )\n");
	outLine("	{\n");
	outLine("		return SkillCategoryStrings[ s ];\n");
	outLine("	}\n");
	outLine("	else return UnknownString;\n");
	*/
	outLine("\t	return UnknownString;\n");
	outLine("\t}\n");
	outLine("\n");
	outLine("}; // SKILLS\n");
	fo.close();

	/////////////////////////////////////////////////////////////////////////////////////
	// Generate skills.pds script file
	
	if( ! fo.open( pdsDir + string( "skills.pds" ) ) )
	{
		nlwarning(" Can't open file %s for writing", "skills.pds" );
		return 1;
	}
	
	outLine( string("// NbSkills in enum : ") + toString( SkillNameToStruct.size() ) + string(" Report this in database.xml \n\n") );
	outLine("file \"skills.h\"\n");
	outLine("{\n");
	outLine("\tenum TSkill\n");
	outLine("\t{\n");
	outLine("\t\tBeginSkill\n");
	outLine("\t\t{\n");

	itSkill = SkillNameToStruct.begin();
	if (itSkill != itSkillEnd)
	{
		outLine(string("\t\t\t") + (*itSkill).second.NormalizedSkillName);
		for ( ++itSkill; itSkill != itSkillEnd ; ++itSkill)
		{
			outLine(",\n");
			outLine(string("\t\t\t") + (*itSkill).second.NormalizedSkillName);
		}
	}
	outLine("\n\t\t}\n");
	outLine("\t} EndSkill\n");
	outLine("}\n");
	fo.close();

	nlinfo("job finish");
	return EXIT_SUCCESS;
}
