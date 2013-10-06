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

#include "mission_compiler.h"
#include "step.h"
#include "nel/misc/i18n.h"
#include "nel/ligo/primitive_utils.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;


// hack to get access to string manager item enumeration to string without including
// almost all of the Ryzom server side project
namespace STRING_MANAGER
{
	NL_BEGIN_STRING_CONVERSION_TABLE (TParamType)	
		NL_STRING_CONVERSION_TABLE_ENTRY( item )
		NL_STRING_CONVERSION_TABLE_ENTRY( place )
		NL_STRING_CONVERSION_TABLE_ENTRY( creature )
		NL_STRING_CONVERSION_TABLE_ENTRY( skill )
		NL_STRING_CONVERSION_TABLE_ENTRY( role )
		NL_STRING_CONVERSION_TABLE_ENTRY( ecosystem )
		NL_STRING_CONVERSION_TABLE_ENTRY( race )
		NL_STRING_CONVERSION_TABLE_ENTRY( sbrick )
		NL_STRING_CONVERSION_TABLE_ENTRY( faction )
		NL_STRING_CONVERSION_TABLE_ENTRY( guild )
		NL_STRING_CONVERSION_TABLE_ENTRY( player )
		NL_STRING_CONVERSION_TABLE_ENTRY( bot )
		{ "int", integer},
//		NL_STRING_CONVERSION_TABLE_ENTRY( integer )
		NL_STRING_CONVERSION_TABLE_ENTRY( time )
		NL_STRING_CONVERSION_TABLE_ENTRY( money )
		NL_STRING_CONVERSION_TABLE_ENTRY( compass )
		NL_STRING_CONVERSION_TABLE_ENTRY( string_id )
		NL_STRING_CONVERSION_TABLE_ENTRY( dyn_string_id )
		NL_STRING_CONVERSION_TABLE_ENTRY( self )
		NL_STRING_CONVERSION_TABLE_ENTRY( creature_model )
		NL_STRING_CONVERSION_TABLE_ENTRY( entity )
		NL_STRING_CONVERSION_TABLE_ENTRY( body_part )
		NL_STRING_CONVERSION_TABLE_ENTRY( score )
		NL_STRING_CONVERSION_TABLE_ENTRY( sphrase )
		NL_STRING_CONVERSION_TABLE_ENTRY( characteristic )
		NL_STRING_CONVERSION_TABLE_ENTRY( damage_type )
		NL_STRING_CONVERSION_TABLE_ENTRY( bot_name)
		NL_STRING_CONVERSION_TABLE_ENTRY( power_type )		
		NL_STRING_CONVERSION_TABLE_ENTRY( literal )		
	NL_END_STRING_CONVERSION_TABLE(TParamType, ParamTypeConversion, NB_PARAM_TYPES)

	//-----------------------------------------------
	// stringToParamType
	//-----------------------------------------------
	TParamType stringToParamType( const std::string & str )
	{
		return ParamTypeConversion.fromString( str );
	}

	//-----------------------------------------------
	// stringToParamType
	//-----------------------------------------------
	const std::string & paramTypeToString( TParamType type )
	{
		return ParamTypeConversion.toString( type );
	}
}

// utility to 'tabulate' the lines in a string
void tabulateLine(std::string &text, uint nbTabs)
{
	if (text.empty())
		return;
	string::size_type pos = 0;
	string	tabs;
	
	for (uint i=0; i<nbTabs; ++i)
		tabs += "\t";

	// add a tab at start
	text = tabs + text;

	// add a tab at each new line
	while ((pos = text.find('\n', pos)) != string::npos)
	{
		if (pos < text.size()-1 && text[pos+1] == '\r')
		{
			// add after the '\r' char
			++pos;
		}
		if (pos < text.size()-1)
			text = text.substr(0, pos+1) + tabs + text.substr(pos+1);
	}
}



class GenderExtractor
{
public:
	

	GenderExtractor(const std::string & literal, const std::string& identifier, unsigned int level=0);

	std::string operator()(unsigned int i) const;

	unsigned int size() const;

	
	~GenderExtractor();
private:	
	bool extractMarkup(const std::string& literal, const std::string & markup,  std::string &before,  std::string &inside,  std::string & after);

	bool parseMarkup(const std::string& literal, const std::string & markup, std::string& newPhrase, bool include = false );

	std::string getPhrase(unsigned int i) const;

	std::string getExtension(unsigned int i) const;
	
	std::string getCondition(unsigned int i) const;

	std::string getEntity(unsigned int i) const;

	std::string getIdentifier(unsigned int i) const;


private:
	
	
	bool _Entity;
	std::string _EntityName;

	std::string _Text;
	std::string _Identifier;
	
	GenderExtractor* _Female;
	GenderExtractor* _Male;



};
GenderExtractor::~GenderExtractor()
{
	delete _Female;
	delete _Male;
}
std::string GenderExtractor::getIdentifier(unsigned int i) const
{
	return _Identifier + getExtension(i);
}

std::string GenderExtractor::operator()(unsigned int i) const
{
	std::string ret("\t");
	std::string condition = getCondition(i);

	ret += condition.empty() ? "":std::string("( ") + condition + " )" + NL + "\t\t";
	ret +=  getIdentifier(i) + "\t[" + getPhrase(i) + "]" + NL;
	return ret;
}


GenderExtractor::GenderExtractor(const std::string & literal, const std::string& identifier, unsigned int level)
{

	
	static const char * es[] ={"e", "e1", "e2", "e3"};
	static const char * fs[] ={"f", "f1", "f2", "f3"};
	static const char * hs[] ={"h", "h1", "h2", "h3"};

	const char * e = es[level];
	const char * f = fs[level];
	const char * h = hs[level];

	_Identifier = toLower(identifier);

	std::string newPhrase;

	std::string before;
	std::string after;
	std::string femaleText;
	std::string maleText;

	_Entity =  extractMarkup(literal, e, before, _EntityName, after);
	if (_EntityName.size() > 2)
	{
		if (_EntityName[0] == '$'  && _EntityName[_EntityName.size() - 1] == '$')
		{
			_EntityName = _EntityName.substr(1, _EntityName.size() - 2);
		}
	}

	std::string newLiteral = before + after;

	bool isFemale = parseMarkup(newLiteral,f,newPhrase, true);		
	if ( isFemale) 
	{
		parseMarkup(newPhrase,h,newPhrase, false);
		femaleText = newPhrase;
	}

	bool isMale = parseMarkup(newLiteral, h, newPhrase, true);
	if (isMale)
	{

		parseMarkup(newPhrase, f, newPhrase, false);
		maleText = newPhrase;
	}

	if (isMale != isFemale)
	{
		std::string goodMarkup = isMale ? std::string("") +"<" + h + "></" + h + ">" : std::string("")+"<"+f+"></"+f+">";
		std::string badMarkup  = isFemale ? std::string("") +"<" + h + "></" + h + ">" : std::string("")+"<"+f+"></"+f+">";
		std::string exceptionText = std::string("Expression ") + identifier + " that contains a tag " + goodMarkup + " needs also tags " + badMarkup + " even empty.";		
		throw EParseException(0, exceptionText.c_str());
	}

	if (!isMale && !isFemale)
	{
		_Text = literal;
		_Female = 0;
		_Male = 0;

	}
	else
	{ 
		if (!_Entity) {	_EntityName = "self"; } 
		_Female = new GenderExtractor(femaleText, identifier, level+1);
		_Male =  new GenderExtractor(maleText, identifier, level+1);
	}



}

bool GenderExtractor::extractMarkup(const std::string& literal, const std::string & markup,  std::string &before,  std::string &inside,  std::string & after)
{
	std::string::size_type posBegin;
	std::string::size_type posEnd;
	std::string::size_type posInside;
	
	std::string beginMarkup = std::string("<") + markup + std::string(">");
	std::string endMarkup = std::string("</") + markup + std::string(">");
	posBegin = literal.find(beginMarkup);
	if ( posBegin != std::string::npos )
	{
		posEnd = literal.find(endMarkup, posBegin + beginMarkup.size());
		if (posEnd != std::string::npos)
		{			
			before = literal.substr(0, posBegin);
			posInside = posBegin + beginMarkup.size();
			inside = literal.substr(posInside, posEnd - posInside);
			after = literal.substr(posEnd+endMarkup.size());
			return true;
		}	
	}
	after = literal;
	return false;
}

bool GenderExtractor::parseMarkup(const std::string& literal, const std::string & markup, std::string& newPhrase, bool include )
{
	
	bool markupExist;
	bool changed = false;
	std::string oldPhrase = literal;
	
	newPhrase = "";
	do 
	{
		std::string before;
		std::string inside;	
		std::string after;
		markupExist = extractMarkup(oldPhrase, markup, before, inside, after);
		newPhrase += before;
		if (include){ newPhrase += inside; }
		oldPhrase = after;
		if (markupExist){ changed = true; }

	} while(markupExist);
	
	newPhrase += oldPhrase;
	return changed;	
}

std::string GenderExtractor::getPhrase(unsigned int i) const
{ 
	if ( i%2 == 0) { return _Male ?  _Male->getPhrase(i/2) :  _Text; } 
	if ( i%2 == 1) { nlassert(_Female); return  _Female->getPhrase(i/2);}
	nlassert(0);
	return "";
}

std::string GenderExtractor::getExtension(unsigned int i) const
{ 
	if ( i%2 == 0) { return _Male ?  std::string("_m") + _Male->getExtension(i/2) :  ""; } 
	if ( i%2 == 1) { nlassert(_Female); return  std::string("_f") + _Female->getExtension(i/2);}
	nlassert(0);
	return "";
}

std::string GenderExtractor::getCondition(unsigned int i) const
{ 

	//if ( i%2 == 0) { return _Male ?  std::string("\t(") + _Male->getExtension(i/2) :  "\t"; } 
	//if ( i%2 == 1) { nlassert(_Female); return  std::string("_f") + _Female->getExtension(i/2);}

	if ( i%2 == 0) 
	{ 
		if (_Male)
		{
			std::string next = _Male->getCondition(i/2);
			std::string current = _EntityName + ".gender = male";
			return next.size() ? current + " & " + next : current;
			
		}
		else
		{
			return "";
		}		
	}
	
	if ( i%2 == 1) 
	{	
		std::string next = _Female->getCondition(i/2);
		std::string current = _EntityName + ".gender = female";
		return next.size() ? current + " & " + next : current;		
	} 

	nlassert(0);

	return "";
}

unsigned int GenderExtractor::size() const
{
	return _Male ? _Male->size() +  _Female->size(): 1;
}


string CPhrase::genPhrase()
{
	string ret;
	if (!_PhraseLiterals.empty())
	{
		for (uint p=0; p<_PhraseLiterals.size(); ++p)
		{
			string identifier = _PhraseId;
			if (_NumEntry != 0)
				identifier += toString("_%u", p+1);

			GenderExtractor gender(_PhraseLiterals[p], identifier, 0);

			ret += identifier + " (";
			// generate default param list
			if (_DefaultParams.size() > p)
			{
				for (uint i=0; i<_DefaultParams[p].size(); ++i)
				{
					ret += STRING_MANAGER::paramTypeToString(_DefaultParams[p][i].ParamType) + " "+_DefaultParams[p][i].ParamName;
					if (i != _DefaultParams[p].size()-1 || !_AdditionalParams.empty())
						ret += ", ";
				}
			}
			// generate additional param list
			for (uint i=0; i<_AdditionalParams.size(); ++i)
			{
				ret += STRING_MANAGER::paramTypeToString(_AdditionalParams[i].ParamType) + " "+_AdditionalParams[i].ParamName;
				if (i != _AdditionalParams.size()-1)
					ret += ", ";
			}
			ret += ")" + NL;
			ret += "{" + NL;

			for (unsigned int i = 0; i < gender.size(); ++i)							
			{				
				ret += gender(i);
			}

			ret += "}" + NL + NL;
		}
	}
nlinfo("genphrase: %s", ret.c_str());
	return ret;
}




bool CMissionCompiler::generateDotScript(NLLIGO::IPrimitive *missionPrim, std::string &dotScript, std::string &log)
{
	//assume that the mission is compiled in the last compiled mission slot
	try
	{
		if (compileMission(missionPrim, string()))
		{
			dotScript = _CompiledMission.back()->generateDotScript();
			return true;
		}
		else
		{
			return false;
		}
	}
	catch(const EParseException & e)
	{
		log = e.Why;
		return false;
	}
}

/*
bool CMissionCompiler::parseGlobalMissionData(IPrimitive *mission, CMissionData &md)
{
	// Mission name
	string *s;
	if (!mission->getPropertyByName("name", s) || s->empty())
		throw EParseException(mission, "missing mission name !");
	md.setMissionName(*s);

	// giver primitive file
	if (!mission->getPropertyByName("giver_primitive", s) || s->empty())
		throw EParseException(mission, "missing giver primitive !");
	md.setGiverPrimitive(*s);

	// giver name
	if (!mission->getPropertyByName("giver_primitive", s) || s->empty())
		throw EParseException(mission, "missing giver primitive !");
	md.setGiverName(*s);
	// If the mission is under a npc_bot node, then the giver is directly taken
	// from the npc name
	if (mission->getParent())
	{
		if (mission->getParent()->getPropertyByName("class", s) && *s == "npc_bot")
		{
			if (mission->getParent()->getPropertyByName("name", s))
				md.setGiverName(*s);
		}
	}

	// TODO : read all other params... 

	return true;
}
*/

void CMissionData::initHeaderPhrase(IPrimitive *prim)
{
	CPhrase::TPredefParams params;
	params.resize(1);
	params[0].push_back(CPhrase::TParamInfo("giver", STRING_MANAGER::bot));
	_MissionTitle.initPhrase(*this, prim, _MissionTitleRaw, 0, params); 
	_MissionDescription.initPhrase(*this, prim, _MissionDescriptionRaw, 0, params);
	_MissionAutoMenu.initPhrase(*this, prim, _MissionAutoMenuRaw);
}

bool CMissionCompiler::compileMission(NLLIGO::IPrimitive *rootPrim, const std::string &primFileName)
{
	TPrimitiveClassPredicate pred("mission_tree");
	if (!pred(rootPrim))
		return false;

	IPrimitive	*mission = rootPrim;
	CMissionData	*pmd = new CMissionData;
	CMissionData	&md = *pmd;

	// Read the mission name
	string missionName = md.getProperty(mission, "name", false, false);
	if( missionName.find(' ') != string::npos)
	{
		throw EParseException(mission, toString("Mission name '%s' must not contains space", missionName.c_str()).c_str());
	}
	md.setMissionName(missionName);
	// Create a temporary primitive node to create default variable
	{	
		// giver default var
		IPrimitive *temp = new CPrimNode();
		temp->addPropertyByName("class", new CPropertyString("var_npc"));
		temp->addPropertyByName("name", new CPropertyString("giver = giver"));
		temp->addPropertyByName("npc_name", new CPropertyString("giver"));
		temp->addPropertyByName("var_name", new CPropertyString("giver"));

		IVar *var = IVar::createVar(md, temp);
		md.addVariable(NULL, var);

		delete temp;
	}

	{
		// player default var
		IPrimitive *temp = new CPrimNode();
		temp->addPropertyByName("class", new CPropertyString("var_npc"));
		temp->addPropertyByName("name", new CPropertyString("player = player"));
		temp->addPropertyByName("npc_name", new CPropertyString("player"));
		temp->addPropertyByName("var_name", new CPropertyString("player"));

		IVar *var = IVar::createVar(md, temp);
		md.addVariable(NULL, var);

		delete temp;
	}

	{	
		// guild_name default var
		IPrimitive *temp = new CPrimNode();
		temp->addPropertyByName("class", new CPropertyString("var_text"));
		temp->addPropertyByName("name", new CPropertyString("guild_name = guild_name"));
		temp->addPropertyByName("npc_name", new CPropertyString("guild_name"));
		temp->addPropertyByName("var_name", new CPropertyString("guild_name"));

		IVar *var = IVar::createVar(md, temp);
		md.addVariable(NULL, var);

		delete temp;
	}

	// first, start by reading mission variables
	IPrimitive	*variables;
	{
		TPrimitiveClassPredicate predTmp("variables");
		variables= NLLIGO::getPrimitiveChild(mission, predTmp);
	}

	if (!variables)
	{
		nlwarning("Can't find variables !");
		return false;
	}
	parseVariables(md, variables);

	// read global mission data
	md.parseMissionHeader(rootPrim);

	// now, we can init the mission header phrase (they need variable knwoled)
	md.initHeaderPhrase(rootPrim);
	
	IPrimitive	*preReq;
	{
		TPrimitiveClassPredicate predTmp("pre_requisite");
		preReq = getPrimitiveChild(mission, predTmp);
	}

	if (!preReq)
	{
		nlwarning("Can't find pre requisite !");
		return false;
	}
	parsePreRequisite(md, preReq);
	
/*	IPrimitive	*steps = getPrimitiveChild(mission, TPrimitivePropertyPredicate("step_tag", "true"));
	if (!steps)
	{
		nlwarning("Can't find steps !");
		return false;
	}
*/	parseSteps(md, mission);

	// Store the compiled mission
	_CompiledMission.push_back(pmd);
	
	string script = md.generateMissionScript(primFileName);

	nlinfo("The script :");
	nlinfo("%s", script.c_str());

	string phrases = md.generatePhraseFile();
	nlinfo("The phrase file is :");
	{
		vector<string> lines;
		explode(phrases, string("\n"), lines, false);
		for (uint i=0; i<lines.size(); ++i)
		{
			if(lines[i][0] == '\r') lines[i] = lines[i].substr(1);
			nlinfo("%s", lines[i].c_str());
		}
	}

	string dot = md.generateDotScript();
	nlinfo("The dot script is :");
	{
		vector<string> lines;
		explode(dot, string("\n"), lines, false);
		for (uint i=0; i<lines.size(); ++i)
		{
			if(lines[i][0] == '\r') lines[i] = lines[i].substr(1);
			nlinfo("%s", lines[i].c_str());
		}
	}
	return true;
}

bool	CMissionCompiler::compileMissions(IPrimitive *rootPrim, const std::string &primFileName)
{
	bool ret = true;
	// 1st, build a set of mission_scrip nodes
	NLLIGO::TPrimitiveSet	missionTrees;
	
	CPrimitiveSet<TPrimitiveClassPredicate>	scriptsSet;
	
	TPrimitiveClassPredicate pred("mission_tree");
	scriptsSet.buildSet(rootPrim, pred, missionTrees);
	
	nlinfo("Found %u mission tree in the primitive file", missionTrees.size());
	
	for (uint i=0; i<missionTrees.size(); ++i)
	{
//		try
//		{
			compileMission(missionTrees[i], primFileName);
//		}
//		catch (const EParseException &e)
//		{
//			nlwarning("Error while parsing a mission: '%s'", e.Why.c_str());
//			ret = false;
//		}

	}
	
	return ret;
	
}

bool CMissionCompiler::installCompiledMission(NLLIGO::CLigoConfig &ligoConfig, const std::string &primFileName)
{
	// generate the mission script into the npcs...
	{
		map<string, TLoadedPrimitive >	loadedPrimitives;

		// store the previous alias value
		map<string, uint32>			missionAlias;

		// First loop to remove any mission that belong to the compiled primitive file
		for (uint i=0; i<_CompiledMission.size(); ++i)
		{
			CMissionData &mission = *(_CompiledMission[i]);
			// first, look for the primitive file to load
			string fileName = mission.getGiverPrimitive();
			if (fileName.empty())
			{
				// use mission primitive instead
				fileName = primFileName;
			}
			if (loadedPrimitives.find(toLower(fileName)) == loadedPrimitives.end())
			{
				string fullFileName = CPath::lookup(fileName, false);
				if (fullFileName.empty())
				{
					throw EParseException(NULL, toString("Can't find primitive file '%s' in path", fileName.c_str()).c_str());
				}
				// we need to load this primitive file.
				CPrimitives *primDoc = new CPrimitives;
				CPrimitiveContext::instance().CurrentPrimitive = primDoc;
				if (loadXmlPrimitiveFile(*primDoc, fullFileName, ligoConfig))
				{
					// the primitive file is loaded correctly
					loadedPrimitives.insert(make_pair(toLower(fileName), TLoadedPrimitive(primDoc, fullFileName)));
					CPrimitiveContext::instance().CurrentPrimitive = NULL;
				}
				else
				{
					CPrimitiveContext::instance().CurrentPrimitive = NULL;
					throw EParseException(NULL, toString("Can't read primitive file '%s'", fullFileName.c_str()).c_str());
				}
			}
			TLoadedPrimitive &loadedPrim = loadedPrimitives[toLower(fileName)];
			CPrimitives *primDoc = loadedPrim.PrimDoc;

			TPrimitiveSet scripts;
			CPrimitiveSet<TPrimitiveClassPredicate> filter;
			TPrimitiveClassPredicate pred("mission");
			filter.buildSet(primDoc->RootNode, pred, scripts);
			
			// for each script, check if it was generated, and if so, check the name
			// of the source primitive file.
			for (uint i=0; i<scripts.size(); ++i)
			{
				vector<string> *script;
				if (scripts[i]->getPropertyByName("script", script) && !script->empty())
				{						
					string missionName;

					scripts[i]->getPropertyByName("name", missionName);

					// Format should be : #compiled from <source_primitive_name>
					if (script->front().find("generated from") != string::npos)
					{
						// we have a compiled mission
						if (script->front().find(CFile::getFilename(primFileName)) != string::npos)
						{
							// ok, this mission is compiled from the same primitive
							
							// store it's alias
							TPrimitiveClassPredicate pred("alias");

							IPrimitive *p = getPrimitiveChild(scripts[i], pred);

							if (p)
							{
								CPrimAlias *pa = dynamic_cast<CPrimAlias*>(p);
								if (pa)
								{
									uint32 alias = pa->getAlias();
									missionAlias.insert(make_pair(missionName, alias));
								}
							}
							else
							{
								nlwarning("Can't find alias prim in primitive '%s'", buildPrimPath(scripts[i]).c_str());
							}
						
							// and remove it
							scripts[i]->getParent()->removeChild(scripts[i]);
						}
					}
				}
			}
		}

		// second loop to assign compiled mission to giver npc
		for (uint i=0; i<_CompiledMission.size(); ++i)
		{
			CMissionData &mission = *(_CompiledMission[i]);
			string fileName = mission.getGiverPrimitive();
			if (fileName.empty())
			{
				// no giver primitive file specified in the mission, use the mission primitive instead
				fileName = primFileName;
			}

			TLoadedPrimitive &loadedPrim = loadedPrimitives[toLower(fileName)];
			CPrimitives *primDoc = loadedPrim.PrimDoc;
			CPrimitiveContext::instance().CurrentPrimitive = primDoc;

			TPrimitiveSet bots;
			CPrimitiveSet<TPrimitiveClassAndNamePredicate> filter;
			TPrimitiveClassAndNamePredicate pred("npc_bot", mission.getGiverName());
			filter.buildSet(primDoc->RootNode, pred, bots);

			if (bots.empty())
			{
				string err = toString("Can't find bot '%s' in primitive '%s' !", 
					mission.getGiverName().c_str(),
					fileName.c_str());
				throw EParseException(NULL, err.c_str());
			}
			else if (bots.size() > 1)
			{
				string err = toString("Found more than one bot named '%s' in primitive '%s' !", 
					mission.getGiverName().c_str(),
					fileName.c_str());
				throw EParseException(NULL, err.c_str());
			}

			// ok, all is good, we can add the mission node to the giver
			IPrimitive *giver = bots.front();
			// create a new node for the mission
			IPrimitive *script = new CPrimNode;
			// set the class
			script->addPropertyByName("class", new CPropertyString("mission"));
			// set the name
			script->addPropertyByName("name", new CPropertyString(mission.getMissionName()));
//				string alias(toString("%u", makeHash32(mission.getMissionName())));
//			script->addPropertyByName("alias", new CPropertyString(mission.getAlias()));
			string scriptLines = mission.generateMissionScript(primFileName);
			vector<string> lines;
			explode(scriptLines, NL, lines, false);

			script->addPropertyByName("script", new CPropertyStringArray(lines));

			// insert the script into the giver
			giver->insertChild(script);

			// add the alias
			{
				CPrimAlias *pa = new CPrimAlias;
				pa->addPropertyByName("class", new CPropertyString ("alias"));
				pa->addPropertyByName("name", new CPropertyString ("alias"));

				if (missionAlias.find(mission.getMissionName()) != missionAlias.end())
				{
					// restore the previous alias
					primDoc->forceAlias(pa, missionAlias.find(mission.getMissionName())->second);
				}

				// insert in first place
				script->insertChild(pa, 0);
			}

			CPrimitiveContext::instance().CurrentPrimitive = NULL;
		}

		// Save the modified primitive files
		while (!loadedPrimitives.empty())
		{
			TLoadedPrimitive &loadedPrim = loadedPrimitives.begin()->second;
			if (!saveXmlPrimitiveFile(*(loadedPrim.PrimDoc), loadedPrim.FullFileName))
				return false;

			_FilesToPublish.push_back(loadedPrim.FullFileName);

			// Free the memory
			delete loadedPrim.PrimDoc;

			loadedPrimitives.erase(loadedPrimitives.begin());
		}
	}

	// generate the phrase file (if any)
	{
		string phraseFileName = CFile::getFilenameWithoutExtension(primFileName) + "_wk.txt";

		CSString content;

		for (uint i=0; i<_CompiledMission.size(); ++i)
		{
			content += _CompiledMission[i]->generatePhraseFile();
		}
		// transform NL (\n\r) into single \n
		content = content.replace(NL.c_str(), "\n");
		ucstring ucs;
		ucs.fromUtf8(content);

		CI18N::writeTextFile(phraseFileName, ucs, true);

		_FilesToPublish.push_back(phraseFileName);
	}

	return true;
}


bool CMissionCompiler::publishFiles(const std::string &serverPathPrim, const std::string &serverPathText, const std::string &localPathText)
{
	for (uint i=0 ; i<_FilesToPublish.size() ; i++)
	{
		string dst, src = _FilesToPublish[i];

		string::size_type n = src.find("primitives");
		if (n == string::npos)
		{
			// text files : copy it and check include in phrase_rites_wk.txt

			// server
			string textFile = CPath::standardizePath(serverPathText) + "phrase_rites_wk.txt";
			includeText(textFile, string("#include \"") + src + string("\"\n"));
			dst = CPath::standardizePath(serverPathText) + src;
			NLMISC::CFile::copyFile(dst, src);

			// local
			textFile = CPath::standardizePath(localPathText) + "phrase_rites_wk.txt";
			includeText(textFile, string("#include \"") + src + string("\"\n"));
			dst = CPath::standardizePath(localPathText) + src;
			NLMISC::CFile::copyFile(dst, src);
		}
		else
		{
			// primitive file : copy to server
			dst = CPath::standardizePath(serverPathPrim) + string(src, n, src.size());
			NLMISC::CFile::copyFile(dst, src);
		}
	}
	return true;
}

bool CMissionCompiler::includeText(const std::string filename, const std::string text)
{
	FILE *f = fopen(filename.c_str(), "r+");
	if (f == NULL)
		return false;

	bool isIn = false;
	char buffer[1024];

	// Check for UTF8 format
	fread(buffer, 1, 3, f);
	if (buffer[0] != -17 || buffer[1] != -69 || buffer[2] != -65)
		fseek(f, 0, SEEK_SET);

	// Compare each line
	while(fgets(buffer, 1024, f))
	{
		if (!strcmp(text.c_str(), buffer))
		{
			isIn = true;
			break;
		}
	}

	if (!isIn)
		fputs(text.c_str(), f);

	fclose(f);
	return true;
}

bool CMissionCompiler::parsePreRequisite(CMissionData &md, IPrimitive *preReq)
{
	md.parsePrerequisites(preReq);
	return true;
}

bool CMissionCompiler::parseOneStep(CMissionData &md, IPrimitive *stepToParse, IStep *parent, bool bEndOfBranch)
{
	IStep *step = IStep::createStep(md, stepToParse);
	if (step != NULL)
	{

		if (!step->isAJump() && !step->getStepName().empty())
		{
			if (md.getStepByName(step->getStepName()) != NULL)
			{
				string err = toString("Step '%s' already defined !", step->getStepName().c_str());
				throw EParseException(step->getPrimitive(), err.c_str());
			}

			if (step->getStepName().find(' ') != string::npos)
			{
				throw EParseException(step->getPrimitive(), toString("Step name '%s' must not contains space", step->getStepName().c_str()).c_str());
			}
			md.addStepName(step->getStepName(), step);
		}

		TPrimitiveSet subBranchs = step->getSubBranchs();
		
		// Add the step (if no parent add to the mission data)
		if (parent == NULL)
		{
			if (!md.addStep(step))
			{
				throw EParseException(stepToParse, "Error parsing mission step");
			}
		}
		else
		{
			parent->addSubStep(step);
		}

		CStepIf *pSI = dynamic_cast<CStepIf *>(step);
		// If this is a IF step : parse with 'step' as a parent 

		IStep *pParentStep = NULL;
		
		if ((dynamic_cast<CStepIf*>(step) != NULL) ||
			(dynamic_cast<CStepPlayerReconnect*>(step) != NULL))
			pParentStep = step;

		if (!subBranchs.empty())
		{
			// need to parse subbranch before continuing
			for (uint i=0; i<subBranchs.size(); ++i)
			{					
				if (!parseOneStep(md, subBranchs[i], pParentStep, i==(subBranchs.size()-1)))
					return false;
			}
		}

		// if this is the last step, flag it as such
		step->EndOfBranch = bEndOfBranch;
	}
	return true;
}

bool CMissionCompiler::parseSteps(CMissionData &md, IPrimitive *steps, IStep *parent)
{
	TPrimitiveSet childs;
	TPrimitivePropertyPredicate pred("step_tag", "true");
	filterPrimitiveChilds(steps, pred, childs);

	if (childs.empty())
	{
		CPrimNode node;
		node.addPropertyByName("class", new CPropertyString("end"));
		node.addPropertyByName("name", new CPropertyString(""));
		IStep *step = IStep::createStep(md, &node);
		delete step;
//		md.addStep(step);
	}
	if (!childs.empty())
	{
		for (uint i=0; i<childs.size(); ++i)
		{
			IPrimitive *child = childs[i];

			parseOneStep(md, childs[i], NULL, i == (childs.size()-1));

		}
	}
	return true;
}

string CMissionCompiler::getProp(IPrimitive *prim, const string &propName)
{
	string s;
	bool ret = prim->getPropertyByName(propName.c_str(), s);
	if (!ret)
		throw EParseException(prim, toString("Property %s does't exist", propName.c_str()).c_str());
	
	return s;
}

string CMissionCompiler::getClass(IPrimitive *prim)
{
	string className;
	bool ret = prim->getPropertyByName("class", className);
	nlassert(ret);
	return className;
}

bool CMissionCompiler::parseVariables(CMissionData &md, IPrimitive *variables)
{
	for (uint i=0; i<variables->getNumChildren(); ++i)
	{
		IPrimitive *child;
		if (variables->getChild(child, i))
		{
			IVar *var = IVar::createVar(md, child);
			if (var)
			{
				nldebug("Adding variable '%s' as type %u", var->getVarName().c_str(), var->getVarType());
				md.addVariable(child, var);
			}
		}
	}
	return true;
}

template <class VectorType>
bool strtokquote(const string &src, VectorType &tokens)
{
	enum TMode
	{
		read_blank,
		read_token,
		read_quoted
	};

	string	temp;
	TMode	mode = read_blank;
	
	for (uint i=0; i<src.size(); ++i)
	{
		switch (mode)
		{
		case read_blank:
			if (src[i] != ' ' && src[i] != '\t' && src[i] != '\n' && src[i] != '\r')
			{
				// end of blank !
				if (src[i] == '\"')
				{
					// begin of a quoted string
					temp = "\"";
					mode = read_quoted;
				}
				else
				{
					// begin of a token
					temp.clear();
					temp += src[i];
					mode = read_token;
				}
			}
			break;
		case read_token:
			if (src[i] == ' ' || src[i] == '\t' || src[i] == '\n' || src[i] == '\r' || src[i] == '\"')
			{
				// end of token
				tokens.push_back(temp);
				temp.clear();
				--i;
				mode = read_blank;
			}
			else
			{
				temp += src[i];
			}
			break;
		case read_quoted:
			if (src[i] == '\\')
			{
				// special treatment for escape command
				if (i < src.size()-1)
				{
					temp += src[i];
					temp += src[i+1];
					// skip escaped char
					i++;
				}
				else
				{
					nlwarning("Error parsing escape char in quoted string");
					return false;
				}
			}
			else if (src[i] != '\"')
			{
				// just add this char
				temp += src[i];
			}
			else
			{
				// end of quoted string
				temp += src[i];
				tokens.push_back(temp);
				temp.clear();
				mode = read_blank;
			}
			break;
		}
	}
	if (!temp.empty())
	{
		if (mode == read_quoted)
		{
			nlwarning("Missing closing quote at end of string while reading text in '%s'", src.c_str());
			return false;
		}
		tokens.push_back(temp);
	}
	return true;
}

template <class VectorType>
bool strtokquote(const vector<string> &src, VectorType &tokens)
{
	for (uint i=0; i<src.size(); ++i)
	{
		if (!strtokquote(src[i], tokens))
			return false;
	}
	return true;
}

struct TFindParamPred : std::unary_function<CPhrase::TParamInfo, bool>
{
	string	Name;
	TFindParamPred(const std::string &name)
		: Name (name)
	{}

	bool operator() (const CPhrase::TParamInfo &paramInfo) const
	{
		return paramInfo.ParamName == Name;
	}
};


bool CPhrase::isEmpty()
{
	return _PhraseId.empty();
}

bool CPhrase::asAdditionnalParams()
{
	return !_AdditionalParams.empty();
}


void CPhrase::initPhrase (CMissionData &md, 
					IPrimitive *prim, 
					const vector<string> &texts, 
					uint32 numEntry, 
					const TPredefParams &predefParams )
{
//	nlassert(numEntry == predefParams.size());

	// store the predefined/default parameters
	_DefaultParams = predefParams;
	// store the number of entry to generate (for literal with variant)
	_NumEntry = numEntry;

	numEntry = max(uint32(1), numEntry);

	_PhraseLiterals.clear();
//	_PhraseLiterals.resize(numEntry);

	// first, concatenate the text vector
	string text;
	for (uint i=0; i<texts.size(); ++i)
	{
		text = text + texts[i];
		if (i != texts.size() -1)
			text += "\n";
	}

	nldebug("phrase text: %s", text.c_str());

	CVectorSString	tokens;

	if (!strtokquote(text, tokens))
		throw EParseException(prim, toString("failed to tokenize the string '%s'", text.c_str()).c_str());

	if (tokens.empty())
		// nothing to parse
		return;

	// storage for additional parameters
	vector<string>	params;

retry:
	// ok, the string is parsed, now we can analyze it
	// look at the first letter of the first token to determine the type of data we have
	if (tokens[0][0] == '\"')
	{
		// we have a literal, so we must found numEntry literal, then a suffix tag for the phrase name
		if (tokens.size() != numEntry +1)
			throw EParseException(prim, toString("bad number of tokens in phrase : need %u (%u entries + 1 suffix), found %u\n(in : '%s')", 
									numEntry+1, 
									numEntry, 
									tokens.size(),
									text.c_str()
									).c_str());

		_PhraseLiterals.resize(numEntry);
		for (uint i=0; i<numEntry; ++i)
		{
			CSString text = tokens[i];
			// remove quotation marks
			text = text.leftCrop(1);
			text = text.rightCrop(1);

			// store the literal phrase value
			_PhraseLiterals[i] = text;
			// escape any ']' in the string
			_PhraseLiterals[i] = CSString(_PhraseLiterals[i]).replace("]", "\\]");

			// now, we can analyse the string content, looking for parameters replacement
			while (text.contains('$'))
			{
				// 'advance' to replacement point
				text = text.splitFrom('$');
				if (!text.empty())
				{
					if (text[0] != '$')
					{
						if (!text.contains('$'))
							throw EParseException(prim, "missing parameter closing tag '$'");

						string::size_type paramStart = _PhraseLiterals[i].size() - text.size();
						// ok, we found a parameter
						CSString p = text.splitTo('$', true);
						// remove any subpart access
						p = p.splitTo('.');
						if (i >= predefParams.size() || find_if(predefParams[i].begin(), predefParams[i].end(), TFindParamPred(static_cast<string&>(p))) == predefParams[i].end())
						{
							// this param is not in the predefined params list, add it to the optional params
							params.push_back(p);
						}	

						// remove any compiler param from the phrase literal
						if (p.find("@") != string::npos)
						{
							string::size_type pos = _PhraseLiterals[i].find(p, paramStart);
							if (pos != string::npos)
							{
								string::size_type pos2 = _PhraseLiterals[i].find("@", pos);
								if (pos2 != string::npos)
								{
									while (pos2 < _PhraseLiterals[i].size() 
										&& _PhraseLiterals[i][pos2] != '.' 
										&& _PhraseLiterals[i][pos2] != '$')
									{
										_PhraseLiterals[i].erase(pos2, 1);
									}
								}

							}
						}
					}
					else
					{
						// this is an escaped $, skip it
						text.leftCrop(1);
					}
				}
			}
		}
		// last, read the suffix
		_Suffixe = tokens.back();

		// generate identifier
		_PhraseId = toUpper(md.getMissionName()+"_"+_Suffixe);

		set<string>	ps;
		// select only unique params
		ps.insert(params.begin(), params.end());

		vector<string> temp(ps.begin(), ps.end());
		params.swap(temp);

	}
	else if (tokens[0][0] == '$')
	{
		// we have a variable substitution. Retrieve the var and recall init

		// do the var replacement
		CVectorSString	tokens2;

		tokens[0] = md.replaceVar(prim, tokens[0]);

		if (!strtokquote(tokens[0], tokens2))
			throw EParseException(prim, toString("failed to tokenize the string ('%s')", tokens[0].c_str()).c_str());

		tokens2.insert(tokens2.end(), tokens.begin()+1, tokens.end());
		tokens.swap(tokens2);
		
		// and retry the decoding
		goto retry;
	}
	else
	{
		// this should be a simple identifier, followed by any number of additional parameters

		// do the var replacement
//		tokens = md.replaceVar(prim, tokens);
//		untagVar(tokens[0]);
	
		// ok, now extract the phrase label and the additional parameters
		_PhraseId = tokens[0];
		for (uint i=1; i<tokens.size(); ++i)
		{
			untagVar(tokens[i]);
			if (predefParams.empty() || find_if(predefParams[0].begin(), predefParams[0].end(), TFindParamPred(static_cast<string&>(tokens[i]))) == predefParams[0].end())
			{
				// this param is not in the predefined params list, add it to the optional params
				params.push_back(tokens[i]);
			}
		}
	}

	// now, build the parameter list

	vector<string>::iterator first(params.begin()), last(params.end());
	for (; first != last; ++first)
	{
		string name, param;
		vector<string> parts;
		NLMISC::explode(*first, string("@"), parts, false);
		
		if (parts.size() > 0)
			name = parts[0];
		if (parts.size() > 1)
			param = parts[1];

		const string &varName = name;

		if (varName != "self")
		{
			IVar *var = md.getVariable(varName);
			if (var == NULL)
			{
				string err = toString("Can't find variable '%s' referenced from a phrase", 
					name.c_str());
				throw EParseException(prim, err.c_str());
			}

			TParamInfo pi;
			pi.ParamName = name;
			pi.CompilerParam = param;
			pi.ParamType = var->getStringManagerType();
			_AdditionalParams.push_back(pi);
		}
	}
}

std::string CPhrase::genScript(CMissionData &md)
{
	std::string ret;

	ret = _PhraseId;
	for (uint i=0; i<_AdditionalParams.size(); ++i)
	{
		IVar *var = md.getVariable(_AdditionalParams[i].ParamName);
		if (var == NULL)
		{
			string err = toString("Can't find variable named '%s' to generate phrase param", _AdditionalParams[i].ParamName.c_str());
			throw EParseException(NULL, err.c_str());
		}
		ret += "; " + var->evalVar(_AdditionalParams[i].CompilerParam);
	}

	return ret;
}

CMissionData::CMissionData()
{
	// init all datas
	_MonoInstance = false;
	_MissionAuto = false;
	_RunOnce = false;
	_Replayable = false;
	_Solo = false;
	_Guild = false;
	_NotInJournal = false;
	_AutoRemoveFromJournal = false;
	_PlayerReplayTimer = 0;
	_GlobalReplayTimer = 0;
	_NotProposed = false;
	_NonAbandonnable = false;
	_NeedValidation = false;
	_FailIfInventoryIsFull = false;
}

CMissionData::~CMissionData()
{
	while (!_Variables.empty())
	{
		delete _Variables.begin()->second;
		_Variables.erase(_Variables.begin());
	}

	while (!_Steps.empty())
	{
		delete _Steps.back();
		_Steps.pop_back();
	}
}

void CMissionData::setMissionName(const string &missionName)
{
	_MissionName = missionName;
}

const string &CMissionData::getMissionName()	{ return _MissionName;}

bool CMissionData::addVariable(NLLIGO::IPrimitive *prim, IVar *var)
{
	if (_Variables.find(var->getVarName()) != _Variables.end())
		throw EParseException(prim, toString("Variable '%s' already defined !", var->getVarName().c_str()).c_str());
	
	_Variables.insert(make_pair(var->getVarName(), var));
	_VariablesOrder.push_back(var);
	return true;
}

IVar *CMissionData::getVariable(const string &varName)
{
	map<string, IVar*>::iterator it(_Variables.find(varName));
	if (it != _Variables.end())
		return it->second;
	return NULL;
}

IStep *CMissionData::getNextStep(IStep *current)
{
	for (uint i=0; i<_Steps.size(); ++i)
	{
		if (_Steps[i] == current && i < _Steps.size()-1)
			return _Steps[i+1];
	}
	return NULL;
}

IStep *CMissionData::getStepByName(const std::string &stepName)
{
	if (_StepsByNames.find(stepName) != _StepsByNames.end())
	{
		return _StepsByNames[stepName];
	}

	return NULL;
}


bool CMissionData::addStep(IStep *step)
{
	_Steps.push_back(step);
	return true;
}

string CMissionData::genPreRequisites()
{
	string ret;
	if (!_ReqSkills.empty())
	{
		ret += "req_skill : ";
		for (uint i=0; i<_ReqSkills.size(); ++i)
		{
			ret += _ReqSkills[i].Skill+" "+_ReqSkills[i].MinLevel+" "+_ReqSkills[i].MaxLevel;
			if (i < _ReqSkills.size()-1)
				ret +="; ";
			else
				ret += NL;
		}
	}
	if (!_ReqMissionDone.empty())
	{
		for (uint i=0; i<_ReqMissionDone.size(); ++i)
		{
			ret += "req_mission : "+ _ReqMissionDone[i]+NL;
		}
	}
	if (!_ReqMissionNotDone.empty())
	{
		for (uint i=0; i<_ReqMissionNotDone.size(); ++i)
		{
			ret += "req_mission_neg : "+_ReqMissionNotDone[i]+NL;
		}
	}
	if (!_ReqMissionRunning.empty())
	{
		for (uint i=0; i<_ReqMissionRunning.size(); ++i)
		{
			ret += "req_mission_running : "+_ReqMissionRunning[i]+NL;
		}
	}
	if (!_ReqMissionNotRunning.empty())
	{
		
		for (uint i=0; i<_ReqMissionNotRunning.size(); ++i)
		{
			ret += "req_mission_running_neg : "+_ReqMissionNotRunning[i]+NL;
		}
	}
	if (!_ReqWearItem.empty())
	{
		ret += "req_wear : ";
		for (uint i=0; i<_ReqWearItem.size(); ++i)
		{
			ret += _ReqWearItem[i];
			if(i < _ReqWearItem.size()-1)
				ret +="; ";
			ret += NL;
		}
	}
	if (!_ReqOwnItem.empty())
	{
		ret += "req_item : ";
		for (uint i=0; i<_ReqOwnItem.size(); ++i)
		{
			ret += _ReqOwnItem[i];
			if(i < _ReqOwnItem.size()-1)
				ret +="; ";
			ret += NL;
		}
	}
	if (!_ReqTitle.empty())
	{
		ret += "req_title : "+_ReqTitle+NL;
	}
	if (!_ReqFames.empty())
	{
		for (uint i=0; i<_ReqFames.size(); ++i)
		{
			ret += "req_fame : "+_ReqFames[i].Faction+" "+_ReqFames[i].Fame;
			ret += NL;
		}
	}
	if(_ReqGuild)
	{
		ret += "req_guild"+NL;
	}
	if (!_ReqGrade.empty())
	{
		ret += "req_grade : "+_ReqGrade+NL;
	}
	if (!_ReqTeamSize.empty())
	{
		ret += "req_team_size : "+_ReqTeamSize+NL;
	}
	if (!_ReqBrick.empty())
	{
		ret += "req_brick : ";
		for (uint i=0; i<_ReqBrick.size(); ++i)
		{
			ret += _ReqBrick[i];
			if(i < _ReqBrick.size()-1)
				ret +="; ";
			ret += NL;
		}
	}
	if (!_ReqSeason.empty())
	{
		ret += "req_season : "+_ReqSeason+NL;
	}
	if (!_ReqEncyclo.empty())
	{
		ret += "req_encyclo_thema : " + _ReqEncyclo + NL;
	}
	if (!_ReqEncycloNeg.empty())
	{
		ret += "req_encyclo_thema_neg : " + _ReqEncycloNeg + NL;
	}
	if (!_ReqEventFaction.empty())
	{
		ret += "req_event_faction : " + _ReqEventFaction + NL;
	}
	
	return ret;
}


string CMissionData::generateMissionScript(const std::string &primFileName)
{
	_JumpPoints.clear();
	// first, gather jump point list
	for (uint i=0; i<_Steps.size(); ++i)
	{
		set<TJumpInfo>	temp;
		_Steps[i]->fillStepJump(*this, temp);

		// remove any jump to the next step (normal flow)
		if (i < _Steps.size()-1)
		{
			set<TJumpInfo>::iterator first(temp.begin()), last(temp.end());
			for (; first != last; )
			{
				const TJumpInfo &ji = *first;

				if (ji.StepName == _Steps[i+1]->getStepName() && ji.Discardable)
				{
					temp.erase(first);
					first = temp.begin();
				}
				else
					++first;
			}
		}

		_JumpPoints.insert(temp.begin(), temp.end());
	}
	// generate the script
	string script;
	// generate mission header
	script += "# script generated from '"+CFile::getFilename(primFileName)+"'"+NL+NL;
	script += "#mission tags and pre-requisites"+NL;
	if (_MonoInstance)
		script += "mono"+NL;
	if (_RunOnce)
		script += "once"+NL;
	if (_Replayable)
		script += "replayable"+NL;
	if (_Solo)
		script += "solo"+NL;
	if (_Guild)
		script += "guild"+NL;
	if (_NotInJournal)
		script += "no_list"+NL;
	if (_AutoRemoveFromJournal)
		script += "auto_remove"+NL;
	if (!_MissionCategory.empty())
		script += "mission_category : "+_MissionCategory+NL;
	if (_PlayerReplayTimer != 0)
		script += "player_replay_timer : "+toString("%u", _PlayerReplayTimer)+NL;
	if (_GlobalReplayTimer != 0)
		script += "global_replay_timer : "+toString("%u", _GlobalReplayTimer)+NL;
	if (_NotProposed)
		script += "not_proposed"+NL;
	if (_MissionAuto)
		script += string("auto : ")+_MissionAutoMenu.genScript(*this)+NL;
	if (_NonAbandonnable)
		script += "non_abandonnable"+NL;
	if (!_MissionIcon.empty())
		script += "mission_icon : "+_MissionIcon+NL;
	if (_NeedValidation)
		script += "need_validation"+NL;
	if (_FailIfInventoryIsFull)
		script += "fail_if_inventory_is_full"+NL;
	
	if (!_ParentMissions.empty())
	{
		set<string>::iterator first(_ParentMissions.begin()), last(_ParentMissions.end());
		for (; first != last; ++first)
		{
			script += "parent : "+ *first+NL;
		}
	}

	script += NL+"#Variables declaration"+NL;

	// declare all the variables
	{
		std::vector<IVar*>::iterator first(_VariablesOrder.begin()), last(_VariablesOrder.end());
		for (; first != last; ++first)
		{
			script += (*first)->genDecl(*this);
		}
	}

	script += NL+"#pre-requisites"+NL;
	script += genPreRequisites();

	script += NL+"#script"+NL;
	// generate mission title and desc
	script += "mission_title : "+_MissionTitle.genScript(*this)+NL;
	script += "mission_desc : "+_MissionDescription.genScript(*this)+NL;

	// generate steps scripts
	for (uint i=0; i<_Steps.size(); ++i)
	{
		script += "# "+_Steps[i]->getStepName()+NL;
		if (_JumpPoints.find(_Steps[i]->getStepName()) != _JumpPoints.end()
			&& !_Steps[i]->isAJump())
		{
			// insert a jump point
			script += "jump_point : " + _Steps[i]->getStepName() + NL;
		}

		script += _Steps[i]->genCode(*this);
		//if (_Steps[i]->EndOfBranch && !_Steps[i]->isAJump())
		//	script += "end"+NL;
	}

	return script;
}

string CMissionData::generatePhraseFile()
{
	string ret;
	// generate header phrase
	ret = _MissionTitle.genPhrase();
	ret += _MissionDescription.genPhrase();
	ret += _MissionAutoMenu.genPhrase();

	// generate var phrase
	for (uint i=0; i<_VariablesOrder.size(); ++i)
	{
		ret += _VariablesOrder[i]->genPhrase();
	}

	// generate step phrase
	for (uint i=0; i<_Steps.size(); ++i)
	{
		ret += _Steps[i]->genPhrase();
	}
	return ret;
}

string CMissionData::generateDotScript()
{
	string ret = "digraph " + _MissionName + NL;
	ret += "{" + NL;

	// set default shape to 'record'
	ret += "node [shape=record]"+NL;

	ret += "\t__start__ [shape=\"point\", peripheries=2, label=\"\"]"+NL;

	// 1st pass, generate node for each step
	for (uint i=0; i<_Steps.size(); ++i)
	{
		if (!_Steps[i]->isEnd() && !_Steps[i]->isAJump())
		{
			ret += "\t"+_Steps[i]->getStepName();
			ret += " [URL=\""+buildPrimPath(_Steps[i]->getPrimitive())+"\"]"+NL;
		}
	}

	ret += "\t__end__ [shape=\"point\"]"+NL;

	// activate red color for shapes that are created after this points
	ret += "node [color=red]"+NL;

	// 2nd pass, generate link between steps
	for (uint i=0; i<_Steps.size(); ++i)
	{
		if (_Steps[i]->isAJump())
			continue;

		if (i == 0)
		{
			ret += "\t__start__ -> " + _Steps[i]->getStepName() + NL;
		}
		set<TJumpInfo> jumps;
		_Steps[i]->fillStepJump(*this, jumps);
		// there is a link there
		while (!jumps.empty())
		{
			const TJumpInfo &ji = *(jumps.begin());
			if (_StepsByNames.find(ji.StepName) != _StepsByNames.end() 
				&& _StepsByNames[ji.StepName]->isAJump())
			{
				// this step is a jump, skip to link to the jump destination
				IStep *jumpStep = _StepsByNames[ji.StepName];
				set<TJumpInfo> jumpJump;
				jumpStep->fillStepJump(*this, jumpJump);
				if (jumpJump.size() != 1)
				{
					string str = toString("Step jump contains %u jumps destination instead of 1", jumpJump.size());
					throw EParseException(jumpStep->getPrimitive(), str.c_str());
				}

				ret += "\t"+_Steps[i]->getStepName() + " -> " + jumpJump.begin()->StepName+" [label=\""+ji.JumpName+"\"]" + NL;
			}
			else
			{
				ret += "\t"+_Steps[i]->getStepName() + " -> " + ji.StepName+" [label=\""+jumps.begin()->JumpName+"\"]" + NL;
			}
			jumps.erase(jumps.begin());
		}

	}

	ret += "}" + NL;

	return ret;
}


void CMissionData::parseMissionHeader(NLLIGO::IPrimitive *prim)
{
//	_MissionName = getProperty(prim, "name", false, false);
//	if( _MissionName.find(' ') != string::npos)
//	{
//		throw EParseException(prim, toString("Mission name '%s' must not contains space", _MissionName.c_str()).c_str());
//	}
	_GiverPrimitive = getProperty(prim,"giver_primitive", true, false);
	_MissionGiver = getProperty(prim, "mission_giver", true, false);

//	_Alias = getProperty(prim, "alias", false, false);

	// If the mission is under a npc_bot node, then the giver is directly taken
	// from the npc name
	if (prim->getParent())
	{
		if (getProperty(prim->getParent(), "class", false, false) == "npc_bot")
		{
			_MissionGiver = getProperty(prim->getParent(), "name", false, false);
		}
	}

	vector<string> vs;
	_MissionTitleRaw = getPropertyArray(prim, "mission_title", false, false);
//	_MissionTitle.init(*this, prim, vs); 
	_MissionDescriptionRaw = getPropertyArray(prim, "mission_description", false, false);
//	_MissionDescription.init(*this, prim, vs);
	_MonoInstance = strlwr(getProperty(prim, "mono_instance", true, false)) == "true";
	_RunOnce = strlwr(getProperty(prim, "run_only_once", true, false)) == "true";
	_Replayable = strlwr(getProperty(prim, "replayable", true, false)) == "true";
	
	_NeedValidation = strlwr(getProperty(prim, "need_validation", true, false)) == "true";
	
	_MissionAutoMenuRaw = getPropertyArray(prim, "phrase_auto_menu", false, false);

	// audience setting 
	string s = getProperty(prim, "audience", false, false);
	if (s == "solo")
		_Solo = true;
	else if (s == "guild")
		_Guild = true;
	
	_NotInJournal = NLMISC::toLower(getProperty(prim, "not_in_journal", false, false)) == "true";
	_AutoRemoveFromJournal = NLMISC::toLower(getProperty(prim, "auto_remove_from_journal", false, false)) == "true";
	_MissionCategory = getProperty(prim, "mission_category", false, false);
	NLMISC::fromString(getProperty(prim, "player_replay_timer", true, false), _PlayerReplayTimer);
	NLMISC::fromString(getProperty(prim, "global_replay_timer", true, false), _GlobalReplayTimer);
	_NotProposed = NLMISC::toLower(getProperty(prim, "not_proposed", false, false)) == "true";
	_MissionAuto = NLMISC::toLower(getProperty(prim, "automatic", false, false)) == "true";
	_NonAbandonnable = NLMISC::toLower(getProperty(prim, "non_abandonnable", false, false)) == "true";
	_FailIfInventoryIsFull = NLMISC::toLower(getProperty(prim, "fail_if_inventory_is_full", false, false)) == "true";
	_MissionIcon = getProperty(prim, "mission_icon", false, false);

	if (_MissionAuto)
	{
		if (_MissionAutoMenuRaw.empty())
		{
			string error = toString("Mission is flagged automatic, but no phrase_auto_menu defined !");
			throw EParseException(prim, error.c_str());
		}
	}

	vs = getPropertyArray(prim, "parent_missions", true, false);
	_ParentMissions.insert(vs.begin(), vs.end());

}

void CMissionData::parsePrerequisites(NLLIGO::IPrimitive *prim)
{
	// skills
	vector<string>	vs;
	vs = getPropertyArray(prim, "require_skill/min_level/max_level", true, false);
	for (uint i=0; i<vs.size(); ++i)
	{
		if (!vs[i].empty())
		{
			vector<string> parts;
			strtokquote(vs[i], parts);
			if (parts.size() != 3)
			{
				throw EParseException(prim, toString("Invalide argument count in line %u of require_skill array. Need 3, found %u", i, parts.size()).c_str());
			}
			TReqSkill rs;
			rs.Skill = parts[0];
			rs.MinLevel = parts[1];
			rs.MaxLevel = parts[2];

			_ReqSkills.push_back(rs);

		}
	}
	// Mission done
	vs = getPropertyArray(prim, "require_mission_done", true, false);
	for (uint i=0; i<vs.size(); ++i)
	{
		if (!vs[i].empty())
		{
			vector<string> parts;
			strtokquote(vs[i], parts);
			if (parts.size() != 1)
			{
				throw EParseException(prim, toString("Invalide argument count in line %u of require_mission_done array. Need 1, found %u", i, parts.size()).c_str());
			}
			_ReqMissionDone.push_back(parts[0]);
		}
	}
	// Mission not done
	vs = getPropertyArray(prim, "require_mission_not_done", true, false);
	for (uint i=0; i<vs.size(); ++i)
	{
		if (!vs[i].empty())
		{
			vector<string> parts;
			strtokquote(vs[i], parts);
			if (parts.size() != 1)
			{
				throw EParseException(prim, toString("Invalide argument count in line %u of require_mission_not_done array. Need 1, found %u", i, parts.size()).c_str());
			}
			_ReqMissionNotDone.push_back(parts[0]);
		}
	}
	// Mission running
	vs = getPropertyArray(prim, "require_mission_running", true, false);
	for (uint i=0; i<vs.size(); ++i)
	{
		if (!vs[i].empty())
		{
			vector<string> parts;
			strtokquote(vs[i], parts);
			if (parts.size() != 1)
			{
				throw EParseException(prim, toString("Invalide argument count in line %u of require_mission_running array. Need 1, found %u", i, parts.size()).c_str());
			}
			_ReqMissionRunning.push_back(parts[0]);
		}
	}
	// Mission not running
	vs = getPropertyArray(prim, "require_mission_not_running", true, false);
	for (uint i=0; i<vs.size(); ++i)
	{
		if (!vs[i].empty())
		{
			vector<string> parts;
			strtokquote(vs[i], parts);
			if (parts.size() != 1)
			{
				throw EParseException(prim, toString("Invalide argument count in line %u of require_mission_not_running array. Need 1, found %u", i, parts.size()).c_str());
			}
			_ReqMissionNotRunning.push_back(parts[0]);
		}
	}
	// wearing item
	vs = getPropertyArray(prim, "require_wearing_item", true, false);
	for (uint i=0; i<vs.size(); ++i)
	{
		if (!vs[i].empty())
		{
			vector<string> parts;
			strtokquote(vs[i], parts);
			if (parts.size() != 1)
			{
				throw EParseException(prim, toString("Invalide argument count in line %u of require_wearing_item array. Need 1, found %u", i, parts.size()).c_str());
			}
			_ReqWearItem.push_back(parts[0]);
		}
	}
	// own item
	vs = getPropertyArray(prim, "require_own_item", true, false);
	for (uint i=0; i<vs.size(); ++i)
	{
		if (!vs[i].empty())
		{
			vector<string> parts;
			strtokquote(vs[i], parts);
			if (parts.size() != 1)
			{
				throw EParseException(prim, toString("Invalide argument count in line %u of require_own_item array. Need 1, found %u", i, parts.size()).c_str());
			}
			_ReqOwnItem.push_back(parts[0]);
		}
	}
	// title
	_ReqTitle = getProperty(prim, "require_title", true, false);
	// fame
	vs = getPropertyArray(prim, "require_faction/fame", true, false);
	for (uint i=0; i<vs.size(); ++i)
	{
		if (!vs[i].empty())
		{
			vector<string> parts;
			strtokquote(vs[i], parts);
			if (parts.size() != 2)
			{
				throw EParseException(prim, toString("Invalide argument count in line %u of require_faction/fame array. Need 2, found %u", i, parts.size()).c_str());
			}
			TReqFame rf;
			rf.Faction = parts[0];
			rf.Fame = parts[1];

			_ReqFames.push_back(rf);
		}
	}
	// guild 
	if (getProperty(prim, "require_guild_membership", true, false) == "true")
		_ReqGuild = true;
	else
		_ReqGuild = false;
	// grade
	_ReqGrade = getProperty(prim, "require_guild_grade", true, false);
	// team size
	_ReqTeamSize = getProperty(prim, "require_team_size", true, false);
	// brick
	vs = getPropertyArray(prim, "require_brick_knowledge", true, false);
	for (uint i=0; i<vs.size(); ++i)
	{
		if (!vs[i].empty())
		{
			vector<string> parts;
			strtokquote(vs[i], parts);
			if (parts.size() != 1)
			{
				throw EParseException(prim, toString("Invalide argument count in line %u of require_brick_knowledge array. Need 1, found %u", i, parts.size()).c_str());
			}
			_ReqBrick.push_back(parts[0]);
		}
	}
	// season
	_ReqSeason = getProperty(prim, "require_season", true, false);
	// encyclopedia
	_ReqEncyclo = getProperty(prim, "require_encyclo_thema", true, false);
	_ReqEncycloNeg = getProperty(prim, "require_encyclo_thema_neg", true, false);

	if ((!_ReqEncyclo.empty() && !_ReqEncycloNeg.empty())
		|| (!_ReqEncycloNeg.empty() && !_ReqEncyclo.empty()))
	{
		string err = toString("You can't mix positive and negative encyclopedy requirement");
		throw EParseException(prim, err.c_str());
	}
	// event faction
	_ReqEventFaction = getProperty(prim, "require_event_faction", true, false);
}

std::string CMissionData::replaceVar(NLLIGO::IPrimitive *prim, const std::string &str)
{
	string::size_type pos = 0;
	string::size_type pos2 = 0;
	string ret;

	while (pos < str.size())
	{
		if (str[pos] != '$')
		{
			ret += str[pos++];
		}
		else if (pos+1 < str.size() && str[pos+1] == '$')
		{
			// check that this $ is not escaped
			ret += '$';
			pos+=2;
		}
		else
		{
			// ok, this is not an escaped $
			CSString varName;
			// skip the initial '$'
			pos++;
//			while (str[pos] != ' ' && str[pos] != '\t' && str[pos] != '\n' && str[pos] != '\r')
			while (pos < str.size() && str[pos] != '$')
				varName += str[pos++];

			if (pos >= str.size())
			{
				string err = toString("Error while parsing variable in '%s', missing closing '$'", str.c_str());
				throw EParseException (NULL, err.c_str());
			}

			// skip the final '$'
			pos++;

			// split the var name and subpart
			vector<string> varParts;
			explode(string(varName), string("@"), varParts, true);

			if (varParts.empty() || varParts.size() > 2)
			{
				throw EParseException(prim, toString("Error parsing varName '%s' in string '%s'", varName.c_str(), str.c_str()).c_str());
			}

			if (_Variables.find(varParts.front()) == _Variables.end())
			{
				string err = toString("Unknown variable '%s' in string '%s'", varParts.front().c_str(), str.c_str());
				throw EParseException (prim, err.c_str());
			}

			IVar *var = _Variables[varParts[0]];

			if (varParts.size() == 1)
				ret += var->evalVar("");
			else
				ret += var->evalVar(varParts[1]);

		}
	}

	return ret;
}

std::vector<std::string> CMissionData::replaceVar(NLLIGO::IPrimitive *prim, const std::vector<std::string> &strs)
{
	vector<string> ret;

	for (uint i=0; i<strs.size(); ++i)
	{
		ret.push_back(replaceVar(prim, strs[i]));
	}

	return ret;
}

std::string CMissionData::getProperty(NLLIGO::IPrimitive *prim, const std::string &propertyName, bool replaceVar, bool canFail)
{
	string ret;
	string *s;
	if (!prim->getPropertyByName(propertyName.c_str(), s))
	{
		if (!canFail)
		{
			string err = toString("Can't find property '%s'", propertyName.c_str());
			throw EParseException (prim, err.c_str());
		}
	}
	else
	{
		ret = *s;
	}

	if (replaceVar)
	{
		ret = this->replaceVar(prim, ret);
	}

	return ret;
}

std::vector<std::string> CMissionData::getPropertyArray(NLLIGO::IPrimitive *prim, const std::string &propertyName, bool replaceVar, bool canFail)
{
	vector<string> ret;
	vector<string> *vs;
	if (!prim->getPropertyByName(propertyName.c_str(), vs))
	{
		if (!canFail)
		{
			string err = toString("Can't find property '%s'", propertyName.c_str());
			throw EParseException (prim, err.c_str());
		}
	}
	else
	{
		ret = *vs;
	}

	if (replaceVar)
	{
		ret = this->replaceVar(prim, ret);
	}
	return ret;
}

bool CMissionData::isThereAJumpTo(const std::string &stepName)
{
	if (_JumpPoints.find(stepName) != _JumpPoints.end())
		return true;
	else
		return false;
}

void TCompilerVarName::init(const std::string &defaultName, STRING_MANAGER::TParamType type, CMissionData &md, NLLIGO::IPrimitive *prim, const std::string propName)
{
	_DefaultName = defaultName;
	_ParamType = type;

	_VarName = md.getProperty(prim, propName, false, false);
	// remove the variable tag if any
	untagVar(_VarName);

	_VarValue = md.getProperty(prim, propName, true, false);
}

void TCompilerVarName::initWithText(const std::string &defaultName, STRING_MANAGER::TParamType type, CMissionData &md, NLLIGO::IPrimitive *prim, const std::string &text)
{
	_DefaultName = defaultName;
	_ParamType = type;

	_VarName = text;
	// remove the variable tag if any
	untagVar(_VarName);

	_VarValue = md.replaceVar(prim, text);
}


CPhrase::TParamInfo TCompilerVarName::getParamInfo() const
{
	if (_VarName.empty())
		return CPhrase::TParamInfo(_DefaultName, _ParamType);
	else
		return CPhrase::TParamInfo(_VarName, _ParamType);
}


bool TCompilerVarName::empty() const
{
	return _VarValue.empty();		
}

TCompilerVarName::operator const std::string  () const
{
	return _VarValue;
}

TCompilerVarName::operator CPhrase::TParamInfo() const
{
	return getParamInfo();
}


std::string operator+(const TCompilerVarName& left, const std::string & right) { return left._VarValue + right;}

std::string operator+(const std::string & left, const TCompilerVarName& right) { return left + right._VarValue;}

std::vector<TCompilerVarName> TCompilerVarName::getPropertyArrayWithText(const std::string &defaultName, STRING_MANAGER::TParamType type, CMissionData &md, NLLIGO::IPrimitive *prim, const std::string & arrayProperyName)
{
	std::vector<TCompilerVarName> compilerParams;

	std::vector<std::string> values = md.getPropertyArray(prim, arrayProperyName,false, false);
	uint first = 0;
	uint last = (uint)values.size();
	compilerParams.resize(last);
	for ( ; first != last; ++first) 
	{		
		compilerParams[first].initWithText( toString("%s%d", defaultName.c_str(), first+1) , type, md, prim,  values[first]);			
	}

	return compilerParams;
}

std::vector<TCompilerVarName> TCompilerVarName::getPropertyArrayWithTextStaticDefaultName(const std::string &defaultName, STRING_MANAGER::TParamType type, CMissionData &md, NLLIGO::IPrimitive *prim, const std::string & arrayProperyName)
{
	std::vector<TCompilerVarName> compilerParams;
	std::vector<std::string> values = md.getPropertyArray(prim, arrayProperyName,false, false);
	uint first = 0;
	uint last = (uint)values.size();
	compilerParams.resize(last);
	for ( ; first != last; ++first) 
	{		
		compilerParams[first].initWithText( defaultName, type, md, prim,  values[first]);			
	}
	return compilerParams;
}

