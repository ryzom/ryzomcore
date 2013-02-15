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




#include "nel/misc/config_file.h"
#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/path.h"
#include "nel/misc/algo.h"
#include <string>

using namespace NLMISC;


/** This app generates sheets of all spell in ryzom
  * Each spell is composed of a projectile and an impact part
  * ...
  */


// write a sheet atom
static writeAtom(IStream &f, const std::string &name, const std::string &value)
{
	f.xmlPushBegin("ATOM");				
		f.xmlSetAttrib("Name");				
		std::string nameNotConst = name;
		f.serial(nameNotConst);
		f.xmlSetAttrib("Value");
		std::string valueNotConst = value;
		f.serial(valueNotConst);		
	f.xmlPushEnd();
	f.xmlPop();
}

//****************************************************************************************************************************
/**
  * A set of user params
  */
class CUserParams
{
public:
	enum { NumUserParams = 4 };
	std::string UserParam[NumUserParams];	
	bool empty() const
	{
		for(uint k = 0; k < NumUserParams; ++k)
		{
			if (!UserParam[k].empty()) return false;
		}
		return true;
	}
public:	
	void serial(IStream &f) throw(EStream)
	{
		for(uint k = 0; k < NumUserParams; ++k)
		{		
			if (!UserParam[k].empty()) writeAtom(f, toString("UserParam%d", (int) k), UserParam[k]);
		}
	}
};

//****************************************************************************************************************************
/** a single fx and its parameters
  */
class CFX
{
public:
	std::string	  PSName;
	CUserParams   UserParams;
public:
	bool empty() const
	{
		return PSName.empty() && UserParams.empty();
	}
	void serial(IStream &f) throw(EStream)
	{
		if (!PSName.empty()) writeAtom(f, "PSName", PSName);
		UserParams.serial(f);
	}
};

//****************************************************************************************************************************
/** A set of fx
  */
class CFXSet
{
public:
	enum { NumFX = 4 };
	CFX FX[NumFX];
public:
	bool empty() const
	{
		for(uint k = 0; k < NumFX; ++k)
		{
			if (!FX[k].empty()) return false;
		}
		return true;
	}
	void serial(IStream &f) throw(EStream)
	{
		for(uint k = 0; k < NumFX; ++k)
		{
			if (!FX[k].empty())
			{		
				f.xmlPushBegin("STRUCT");										
					f.xmlSetAttrib("Name");
					std::string name = toString("FX%d", (int) k).c_str();
					f.serial(name);					
				f.xmlPushEnd();			
					FX[k].serial(f);
				f.xmlPop();
			}
		}
	}
	void setUserParams(const CUserParams &params)
	{
		for(uint k = 0; k < NumFX; ++k)
		{
			FX[k].UserParams = params;
		}
	}	
};

//****************************************************************************************************************************
/** A spell sheet, with some args defined
  */
class CSpellSheet
{
public:
	std::vector<std::string> Parents;
	CFXSet Projectile;
	CFXSet Impact;
public:	
	// write that spell as a sheet
	void writeSheet(const std::string &sheetName)
	{
		COFile f;
		if (!f.open(sheetName, false, true))
		{
			nlwarning("Can't write %s", sheetName.c_str());
			return;
		}
		try
		{	
			COXml xmlStreamOut;
			xmlStreamOut.init(&f);
			xmlStreamOut.xmlPushBegin("FORM");

			IStream &xmlStream = xmlStreamOut;

				/*
				std::string revision = "$revision$";
				xmlStream.xmlSerial(revision, "Revision");
				std::string state = "modified";
				xmlStream.xmlSerial(state, "State");
				*/				
				xmlStream.xmlSetAttrib("Revision");
				std::string revision = "$revision$";
				xmlStream.serial(revision);
				xmlStream.xmlSetAttrib("State");
				std::string state = "modified";
				xmlStream.serial(state);								
			xmlStream.xmlPushEnd();
			// write parent list
			for(uint k = 0; k < Parents.size(); ++k)
			{
				xmlStream.xmlPushBegin("PARENT");					
					xmlStream.xmlSetAttrib("Filename");
					xmlStream.serial(Parents[k]);									
				xmlStream.xmlPushEnd();
				xmlStream.xmlPop();
			}
			if (!Projectile.empty() || !Impact.empty())
			{			
				xmlStream.xmlPush("STRUCT");
					if (!Projectile.empty())
					{
						xmlStream.xmlPushBegin("STRUCT");
							xmlStream.xmlSetAttrib("Name");
							std::string name = "Projectile";
							xmlStream.serial(name);					
						xmlStream.xmlPushEnd();			
							Projectile.serial(xmlStream);
						xmlStream.xmlPop();
					}
					if (!Impact.empty())
					{
						xmlStream.xmlPushBegin("STRUCT");										
							xmlStream.xmlSetAttrib("Name");
							std::string name = "Impact";
							xmlStream.serial(name);					
						xmlStream.xmlPushEnd();
							Impact.serial(xmlStream);
						xmlStream.xmlPop();
					}
				xmlStream.xmlPop();
			}
			else
			{
				xmlStream.xmlPush("STRUCT");
				xmlStream.xmlPop();
			}
			xmlStream.xmlPop();
		}
		catch(const EStream &)
		{
			nlwarning("Cant write %s", sheetName.c_str());
		}
	}
};
//****************************************************************************************************************************
/** Generate list of spell
  */
static void generateSpellList(CConfigFile &cf, const std::string &sheetName)
{
	CConfigFile::CVar *spellList = cf.getVarPtr("spell_list");
	if (!spellList)
	{
		nlwarning("Can't read spell list");
		return;
	}	
	COFile f;
	if (!f.open(sheetName, false, true))
	{
		nlwarning("Can't write %s", sheetName.c_str());
		return;
	}
	try
	{	
		COXml xmlStreamOut;
		xmlStreamOut.init(&f);
		xmlStreamOut.xmlPush("FORM");
		IStream &xmlStream = xmlStreamOut;
		xmlStream.xmlPush("STRUCT");
		xmlStream.xmlPushBegin("ARRAY");
			xmlStream.xmlSetAttrib("Name");
			std::string name = "List";
			xmlStream.serial(name);					
		xmlStream.xmlPushEnd();				
		for(uint k = 0; k < (uint) spellList->size(); ++k)
		{
			std::vector<std::string> result;
			NLMISC::splitString(spellList->asString(k), "|", result);
			if (result.size()  < 2)
			{
				nlwarning("Should provide at list spell name and id");
			}
			xmlStream.xmlPush("STRUCT");				
				writeAtom(xmlStream, "ID", result[1]);
				writeAtom(xmlStream, "SheetBaseName", result[0]);
			xmlStream.xmlPop();
		}
		xmlStream.xmlPop(); // STRUCT
		xmlStream.xmlPop(); // FORM
	}
	catch(const EStream &)
	{
		nlwarning("Cant write %s", sheetName.c_str());
	}
}

//****************************************************************************************************************************
int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		nlwarning("Usage : %s config_file_name.cfg", argv[0]);
		return -1;
	}
	CConfigFile cf;
	try
	{	
		cf.load(argv[1]);
	}
	catch(const NLMISC::EConfigFile &)
	{
		nlwarning("Error in config file %s", argv[1]);
		return -1;
	}	
	catch(...)
	{
		nlwarning("Can't read config file %s", argv[1]);
		return -1;
	}
	// output for sheets
	std::string outputPath;
	CConfigFile::CVar *outputPathVar = cf.getVarPtr("output_path");
	if (outputPathVar)
	{
		outputPath = outputPathVar->asString() + "/";
	}
	// output for 'levels' parents
	std::string levelParentsPath;
	CConfigFile::CVar *levelParentsPathVar = cf.getVarPtr("level_parents");
	if (levelParentsPathVar)
	{
		levelParentsPath = levelParentsPathVar->asString() + "/";
	}
	// output for projectile parents
	std::string projectileParentsPath;
	CConfigFile::CVar *projectileParentsPathVar = cf.getVarPtr("projectile_base");
	if (projectileParentsPathVar)
	{
		projectileParentsPath= projectileParentsPathVar->asString() + "/";
	}	
	// output for 'projectile by level and mode' parents
	std::string projectileByLevelAndModeParentsPath;
	CConfigFile::CVar *projectileByLevelAndModeParentsPathVar = cf.getVarPtr("projectile_by_level_and_mode_parents");
	if (projectileByLevelAndModeParentsPathVar)
	{
		projectileByLevelAndModeParentsPath = projectileByLevelAndModeParentsPathVar->asString() + "/";
	}
	// output for 'base spells' parents
	std::string baseSpellPath;
	CConfigFile::CVar *baseSpellPathVar = cf.getVarPtr("spell_base");
	if (baseSpellPathVar)
	{
		baseSpellPath = baseSpellPathVar->asString() + "/";
	}
	// output for 'spell by levels' parents
	std::string spellByLevelParentsPath;
	CConfigFile::CVar *spellByLevelParentsPathVar = cf.getVarPtr("spell_by_level_parents");
	if (spellByLevelParentsPathVar)
	{
		spellByLevelParentsPath = spellByLevelParentsPathVar->asString() + "/";
	}
	// output for 'final spell'
	std::string finalSpellPath;
	CConfigFile::CVar *finalSpellPathVar = cf.getVarPtr("final_spells");
	if (finalSpellPathVar)
	{
		finalSpellPath = finalSpellPathVar->asString() + "/";
	}
	


	// read number of levels
	CConfigFile::CVar *numLevelVar = cf.getVarPtr("num_levels");
	if (!numLevelVar)
	{
		nlwarning("Can't read number of spell levels");
		return -1;
	}
	uint numSpellLevels = numLevelVar->asInt();

	std::vector<CUserParams> userParams(numSpellLevels);

	// read user params set for each level
	for(uint level = 0; level < numSpellLevels; ++level)
	{
		std::string varName = toString("user_params_level%d", (int) (level + 1));
		CConfigFile::CVar *up = cf.getVarPtr(varName);
		if (!up)
		{
			nlwarning("Can't read var %s", varName.c_str());
		}
		else
		{
			for(uint k = 0; k < CUserParams::NumUserParams; ++k)
			{
				userParams[level].UserParam[k] = up->asString(k);
			}
		}		
	}

	// read types of spells (offensif, curatif ...)
	CConfigFile::CVar *spellTypesList = cf.getVarPtr("spell_types"); 
	if (!spellTypesList)
	{
		nlwarning("Can't read types of spells");
		return -1;
	}
	// read modes of spells
	CConfigFile::CVar *spellModesList = cf.getVarPtr("spell_modes");

	// read name of ps for projectiles
	std::vector<std::string> projPSNames;
	projPSNames.resize(spellTypesList->size() * spellModesList->size());
	CConfigFile::CVar *projectileNames = cf.getVarPtr("projectile_fx");
	if (projectileNames)
	{
		for(uint k = 0; k < (uint) projectileNames->size(); ++k)
		{
			// entry are expected to have the following form : 
			// "type|mode|fx_name.ps"
			std::vector<std::string> params;
			NLMISC::splitString(projectileNames->asString(k), "|", params);
			if (params.size() != 3)
			{
				nlwarning("Bad param for projectile ps name : %s", projectileNames->asString(k).c_str());
			}
			else
			{
				bool found = false;
				// find the mode				
				for (uint mode = 0; mode < (uint) spellModesList->size(); ++mode)
				{
					if (spellModesList->asString(mode) == params[1])
					{						
						for (uint type = 0; type < (uint) spellTypesList->size(); ++type)
						{
							if (spellTypesList->asString(type) == params[0])
							{
								projPSNames[type + mode * spellTypesList->size()] = params[2];
								//nlwarning("%s : found", projectileNames->asString(k).c_str());
								found = true;
								break;
							}
						}
						if (found) break;
					}
				}
				//if (!found) nlwarning("%s : not found", projectileNames->asString(k).c_str());
			}			
		}
	}

	nlinfo("Generate projectiles parent sheets...");
	// gen projectiles base sheet
	CSpellSheet baseProjectileSheet;
	baseProjectileSheet.writeSheet(outputPath + projectileParentsPath + "_projectile_base.spell");	
	// gen projectiles parent sheets
	for(uint type = 0; type < (uint) spellTypesList->size(); ++type)
	{
		for(uint mode = 0; mode < (uint) spellModesList->size(); ++mode)
		{
			std::string sheetName = "_projectile_" + spellTypesList->asString(type) + "_" + spellModesList->asString(mode) + ".spell";
			CSpellSheet ss;
			ss.Parents.push_back("_projectile_base.spell");
			// affect ps name if known
			ss.Projectile.FX[0].PSName = projPSNames[type + mode * spellTypesList->size()];
			ss.writeSheet(outputPath + projectileParentsPath + sheetName);			
		}
	}
	
	nlinfo("Generate sheets by level...");
	// generate sheets by level
	for(uint level = 0; level < numSpellLevels; ++level)
	{		
		// gen projectiles by level sheets (parent sheets)
		std::string sheetName = toString("_projectile_lvl%d.spell", (int) (level + 1));
		CSpellSheet projectileSheet;
		projectileSheet.Projectile.setUserParams(userParams[level]);
		projectileSheet.writeSheet(outputPath + levelParentsPath + sheetName);
		// gen impact level sheets
		sheetName = toString("_impact_lvl%d.spell", (int) (level + 1));
		CSpellSheet impactSheet;
		impactSheet.Impact.setUserParams(userParams[level]);
		impactSheet.writeSheet(outputPath + levelParentsPath + sheetName);
		
	}

	nlinfo("Generate projectile list (by mode, type and levels)...");
	// generate projectile list (by mode, type and levels)
	for(uint type = 0; type < (uint) spellTypesList->size(); ++type)
	{
		for(uint mode = 0; mode < (uint) spellModesList->size(); ++mode)
		{
			for(uint level = 0; level < (uint) numSpellLevels; ++level)
			{			
				CSpellSheet ss;
				ss.Parents.resize(2);
				ss.Parents[0] = toString("_projectile_lvl%d.spell", (int) (level + 1)); // inherit level
				ss.Parents[1] = "_projectile_" + spellTypesList->asString(type) + "_" + spellModesList->asString(mode) + ".spell"; // inherit mode and type
				std::string sheetName = "_projectile_" + spellTypesList->asString(type) + "_" + spellModesList->asString(mode) + toString("_lvl%d.spell", (int) (level + 1));
				ss.writeSheet(outputPath + projectileByLevelAndModeParentsPath + sheetName);			
			}
		}
	}
	//
	nlinfo("Generate spell list...");
	// read list of spells
	// the string format for spells is : "sheet_name|ps_name"
	// the name of the particle system is optionnal
	CConfigFile::CVar *spellList = cf.getVarPtr("spell_list");
	if (!spellList)
	{
		nlwarning("Can't read spell list");
		return -1;
	}
	for(uint k = 0; k < (uint) spellList->size(); ++k)
	{
		std::string spellName = spellList->asString(k);
		std::vector<std::string> result;
		NLMISC::splitString(spellName, "|", result);
		if (result.size()  < 3)
		{
			nlwarning("Should provide at least spell name, id and mode");
		}
		else
		{		
			// generate parent sheet
			CSpellSheet baseSpellSheet;
			if (result.size() > 3)
			{			
				baseSpellSheet.Impact.FX[0].PSName = result[3];
			}
			baseSpellSheet.writeSheet(outputPath + baseSpellPath + "_" + result[0] + ".spell");
			// generate child sheet
			// - by spell level
			// - by spell type (chain, bomb, spray ...)

			// save spells by level
			for(uint level = 0; level < numSpellLevels; ++level)
			{
				CSpellSheet leveledSpell;
				leveledSpell.Parents.resize(2);
				leveledSpell.Parents[0] = "_" + result[0] + ".spell";
				leveledSpell.Parents[1] = toString("_impact_lvl%d.spell", (int) (level + 1));
				leveledSpell.writeSheet(outputPath + spellByLevelParentsPath + "_" + result[0] + toString("_lvl%d.spell", (int) (level + 1)));
			}

			// save spell with good projectile and level
			for(uint level = 0; level < numSpellLevels; ++level)
			{
				for(uint mode = 0; mode < (uint) spellModesList->size(); ++mode)
				{
					CSpellSheet finalSheet;
					finalSheet.Parents.resize(2);
					finalSheet.Parents[0] = "_" + result[0] + toString("_lvl%d.spell", (int) (level + 1));
					finalSheet.Parents[1] = "_projectile_" + result[2] + "_" + spellModesList->asString(mode) + toString("_lvl%d.spell", (int) (level + 1));
					//finalSheet.writeSheet(outputPath + finalSpellPath + result[0] + toString("_lvl%d_", (int) (level + 1)) + result[2] + "_" + spellModesList->asString(mode) + ".spell");
					finalSheet.writeSheet(outputPath + finalSpellPath + result[0] + "_" + spellModesList->asString(mode) + toString("_lvl%d", (int) (level + 1)) + ".spell");
				}
			}
		}
	}
	// generate spell list with their ids
	CConfigFile::CVar *spellListFile = cf.getVarPtr("spell_list_file");
	if (spellListFile)
	{	
		generateSpellList(cf, outputPath + spellListFile->asString());
	}
	nlinfo("Done");
	return 0;
}

