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
#include "nel/misc/config_file.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/load_form.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive.h"
#include "nel/ligo/primitive_utils.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace STRING_MANAGER;

vector<string>	Filters;

static CLigoConfig		LigoConfig;
static bool	RemoveOlds = false;

struct TCreatureInfo
{
	CSheetId	SheetId;
	bool		ForceSheetName;
	bool		DisplayName;


	void	readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
	{
		const NLGEORGES::UFormElm &item=form->getRootNode();

		SheetId=sheetId;
		item.getValueByName(ForceSheetName, "3d data.ForceDisplayCreatureName");
		item.getValueByName(DisplayName, "3d data.DisplayName");
	}

	void serial(NLMISC::IStream &f)
	{
		f.serial(SheetId);
		f.serial(ForceSheetName);
		f.serial(DisplayName);
	}


	static uint getVersion () 
	{ 
		return 1;
	}

	void removed()
	{
	}
	
};

std::map<CSheetId, TCreatureInfo>	Creatures;

TCreatureInfo *getCreature(const std::string &sheetName)
{
	CSheetId id(sheetName+".creature");

	if (Creatures.find(id) != Creatures.end())
		return &(Creatures.find(id)->second);
	else
		return NULL;
}

string cleanupName(const std::string &name)
{
	string ret;

	for (uint i=0; i<name.size(); ++i)
	{
		if (name[i] != ' ')
			ret += name[i];
		else
			ret += '_';
	}

	return ret;
}

ucstring cleanupUcName(const ucstring &name)
{
	ucstring ret;

	for (uint i=0; i<name.size(); ++i)
	{
		if (name[i] != ' ')
			ret += name[i];
		else
			ret += '_';
	}

	return ret;
}


/*
	Removes first and last '$'
*/
ucstring makeGroupName(const ucstring & translationName)
{
	ucstring ret = translationName;
	if (ret.size() >= 2)
	{
		if ( *ret.begin() == ucchar('$'))
		{
			ret=ret.substr(1);
		}
		if ( *ret.rbegin() == ucchar('$'))
		{
			ret = ret.substr(0, ret.size()-1);
		}
	}
	ret = cleanupUcName(ret);
	return ret;	
}

struct TEntryInfo
{
	string	SheetName;
};

set<string>					GenericNames;
map<string, TEntryInfo>		SimpleNames;
set<string>					Functions;


string	removeAndStoreFunction(const std::string &fullName)
{
	string::size_type pos = fullName.find("$");
	if (pos == string::npos)
		return fullName;
	else
	{
		// extract and store the function name
		string ret;

		ret = fullName.substr(0, pos);
		string::size_type pos2 = fullName.find("$", pos+1);

		string fct = fullName.substr(pos+1, pos2-(pos+1));

		ret += fullName.substr(pos2+1);

		if (Functions.find(fct) == Functions.end())
		{
			nldebug("Adding function '%s'", fct.c_str());
			Functions.insert(fct);
		}

		return ret;
	}
}


void addGenericName(const std::string &name, const std::string &sheetName)
{
	TCreatureInfo *c = getCreature(sheetName);
	if (!c || c->ForceSheetName || !c->DisplayName)
		return;
	
	if (SimpleNames.find(name) != SimpleNames.end())
	{
		nldebug("Name '%s' is now a generic name", name.c_str());
		GenericNames.insert(name);
		SimpleNames.erase(name);

	}
	else if (GenericNames.find(name) == GenericNames.end())
	{
		nldebug("Adding generic name '%s'", name.c_str());
		GenericNames.insert(name);
	}
}

void addSimpleName(const std::string &name, const std::string &sheetName)
{
	TCreatureInfo *c = getCreature(sheetName);
	if (!c || c->ForceSheetName || !c->DisplayName)
		return;

	if (SimpleNames.find(name) != SimpleNames.end())
	{
		addGenericName(name, sheetName);
	}
	else if (GenericNames.find(name) != GenericNames.end())
	{
		return;
	}
	else
	{
		nldebug("Adding simple name '%s'", name.c_str());
		
		TEntryInfo ei;
		ei.SheetName = sheetName;

		SimpleNames.insert(make_pair(name, ei));
	}
}

int extractBotNames(int argc, char *argv[])
{
	//-------------------------------------------------------------------
	// read the parameters
	for (int i=2; i<argc; ++i)
	{
		string s = argv[i];
		if (s == "-r")
		{
			// active remove mode
			RemoveOlds = true;
		}
		else
		{
			nlwarning("Unknow option '%s'", argv[i]);
			return -1;
		}
	}

	//-------------------------------------------------------------------
	// read the configuration file
	CConfigFile	cf;

	cf.load("bin/translation_tools.cfg");

	//-------------------------------------------------------------------
	// read the vars
	CConfigFile::CVar &paths = cf.getVar("Paths");
	CConfigFile::CVar &filtersVar = cf.getVar("Filters");
	CConfigFile::CVar &ligoClassFile= cf.getVar("LigoClassFile");
	CConfigFile::CVar &georgesPaths= cf.getVar("GeorgesPaths");
	CConfigFile::CVar &pathNoRecurse= cf.getVar("PathsNoRecurse");
	CConfigFile::CVar &workBotNamesFile= cf.getVar("WorkBotNamesFile");
	CConfigFile::CVar &transBotNamesFile= cf.getVar("TransBotNamesFile");
	CConfigFile::CVar &workTitleFile= cf.getVar("WorkTitleFile");

	for (uint i=0; i<paths.size(); ++i)
	{
		CPath::addSearchPath(paths.asString(i), true, false);
	}
	for (uint i=0; i<pathNoRecurse.size(); ++i)
	{
		CPath::addSearchPath(pathNoRecurse.asString(i), false, false);
	}

	for (uint i=0; i<filtersVar.size(); ++i)
	{
		Filters.push_back(filtersVar.asString(i));
	}


	//-------------------------------------------------------------------
	// init the sheets
	CSheetId::init(false);
	const string PACKED_SHEETS_NAME = "bin/translation_tools_creature.packed_sheets";
	loadForm("creature", PACKED_SHEETS_NAME, Creatures, false, false);

	if (Creatures.empty())
	{
		for (uint i=0;i<georgesPaths.size();++i)
			CPath::addSearchPath(georgesPaths.asString(i).c_str(), true, false);

		loadForm("creature", PACKED_SHEETS_NAME, Creatures, true);
	}


	//-------------------------------------------------------------------
	// init ligo config
	string ligoPath = CPath::lookup(ligoClassFile.asString(), true, true);
	LigoConfig.readPrimitiveClass(ligoPath.c_str(), false);
	NLLIGO::Register();

	CPrimitiveContext::instance().CurrentLigoConfig = &LigoConfig;

	//-------------------------------------------------------------------
	// ok, ready for the real work,
	// first, read the primitives files and parse the primitives
	vector<string>	files;
	CPath::getFileList("primitive", files);

	for (uint i=0; i<files.size(); ++i)
	{
		string pathName = files[i];
		pathName = CPath::lookup(pathName);

		// check filters
		uint j=0;
		for (j=0; j<Filters.size(); ++j)
		{
			if (pathName.find(Filters[j]) != string::npos)
				break;
		}
		if (j != Filters.size())
			// skip this file
			continue;

		nlinfo("Loading file '%s'...", CFile::getFilename(pathName).c_str());
		
		CPrimitives primDoc;
		CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
		loadXmlPrimitiveFile(primDoc, pathName, LigoConfig);

		// now parse the file

		// look for group template
		{
			TPrimitiveClassPredicate pred("group_template_npc");
			TPrimitiveSet result;

			CPrimitiveSet<TPrimitiveClassPredicate> ps;
			ps.buildSet(primDoc.RootNode, pred, result);

			for (uint i=0; i<result.size(); ++i)
			{
				string name;
				string countStr;
				string sheetStr;
				result[i]->getPropertyByName("name", name);
				result[i]->getPropertyByName("count", countStr);
				result[i]->getPropertyByName("bot_sheet_look", sheetStr);

				uint32 count;
				NLMISC::fromString(countStr, count);

				if (count != 0)
				{
					if (sheetStr.empty())
					{
						nlwarning("In '%s', empty sheet !", buildPrimPath(result[i]).c_str());
					}
					else
					{
						addGenericName(removeAndStoreFunction(name), sheetStr);
					}
				}
			}
		}
		// look for bot template
		{
			TPrimitiveClassPredicate pred("bot_template_npc");
			TPrimitiveSet result;

			CPrimitiveSet<TPrimitiveClassPredicate> ps;
			ps.buildSet(primDoc.RootNode, pred, result);

			for (uint i=0; i<result.size(); ++i)
			{
				string name;
				string sheetStr;
				result[i]->getPropertyByName("name", name);
				result[i]->getPropertyByName("sheet_look", sheetStr);

				if (sheetStr.empty())
				{
					// take the sheet in the parent
					result[i]->getParent()->getPropertyByName("bot_sheet_look", sheetStr);
				}

				if (sheetStr.empty())
				{
					nlwarning("In '%s', empty sheet !", buildPrimPath(result[i]).c_str());
				}
				else
				{
					addGenericName(removeAndStoreFunction(name), sheetStr);
				}
			}
		}
		// look for npc_group 
		{
			TPrimitiveClassPredicate pred("npc_group");
			TPrimitiveSet result;

			CPrimitiveSet<TPrimitiveClassPredicate> ps;
			ps.buildSet(primDoc.RootNode, pred, result);

			for (uint i=0; i<result.size(); ++i)
			{
				string name;
				string countStr;
				string sheetStr;
				result[i]->getPropertyByName("name", name);
				result[i]->getPropertyByName("count", countStr);
				result[i]->getPropertyByName("bot_sheet_client", sheetStr);

				uint32 count;
				NLMISC::fromString(countStr, count);

				if (count > 0 && sheetStr.empty())
				{
					nlwarning("In '%s', empty sheet !", buildPrimPath(result[i]).c_str());
				}
				else
				{
					if (count == 1)
					{
						addSimpleName(removeAndStoreFunction(name), sheetStr);
					}
					else if (count > 1)
					{
						addGenericName(removeAndStoreFunction(name), sheetStr);
					}
				}
			}
		}
		// look for bot 
		{
			TPrimitiveClassPredicate pred("npc_bot");
			TPrimitiveSet result;

			CPrimitiveSet<TPrimitiveClassPredicate> ps;
			ps.buildSet(primDoc.RootNode, pred, result);

			for (uint i=0; i<result.size(); ++i)
			{
				string name;
				string sheetStr;
				result[i]->getPropertyByName("name", name);
				result[i]->getPropertyByName("sheet_client", sheetStr);

				if (sheetStr.empty())
				{
					// take the sheet in the parent
					result[i]->getParent()->getPropertyByName("bot_sheet_client", sheetStr);
				}

				if (sheetStr.empty())
				{
					nlwarning("In '%s', empty sheet !", buildPrimPath(result[i]).c_str());
				}
				else
				{
					TEntryInfo ei;
					addSimpleName(removeAndStoreFunction(name), sheetStr);
				}
			}
		}
	}

	//-------------------------------------------------------------------
	// step 2 : load the reference file

	nlinfo("Looking for missing translation:");

	TWorksheet			botNames;
	loadExcelSheet(workBotNamesFile.asString(), botNames, true);
	TWorksheet			transBotNames;
	loadExcelSheet(transBotNamesFile.asString(), transBotNames, true);

	TWorksheet			fcts;
	loadExcelSheet(workTitleFile.asString(), fcts, true);


	// add missing element

	uint	nbAddSimpleName = 0;
	uint	nbAddFunction = 0;
	uint	nbAddGenericName = 0;

	uint botIdCol;
	nlverify(botNames.findId(botIdCol));
	uint transIdCol;
	nlverify(transBotNames.findId(transIdCol));
	uint	fctsIdCol;
	nlverify(fcts.findId(fctsIdCol));

	// special treatment to add the sheet_name col
	{
		uint sheetCol;
		if (!botNames.findCol(ucstring("sheet_name"), sheetCol))
		{
			botNames.insertColumn(botNames.ColCount);
			botNames.setData(0, botNames.ColCount-1, ucstring("sheet_name"));
		}
		
		if (!transBotNames.findCol(ucstring("sheet_name"), sheetCol))
		{
			transBotNames.insertColumn(transBotNames.ColCount);
			transBotNames.setData(0, transBotNames.ColCount-1, ucstring("sheet_name"));
		}
	}
	// 1 - simple names
	{
		nlinfo("  Simple names...");


		map<string, TEntryInfo>::iterator first(SimpleNames.begin()), last(SimpleNames.end());
		for (; first != last; ++first)
		{
			uint rowIdx = 0;
			if (!botNames.findRow(botIdCol, first->first, rowIdx))
			{
				// we need to add the entry
				rowIdx = botNames.size();
				botNames.resize(botNames.size()+1);

				botNames.setData(rowIdx, ucstring("bot name"), first->first);
				botNames.setData(rowIdx, ucstring("translated name"), first->first);
				botNames.setData(rowIdx, ucstring("sheet_name"), first->second.SheetName);

				nbAddSimpleName++;
			}
			else
			{
				// set/update the sheet name info
				// try to restore the existing translation
				uint transRowIdx = 0;
				if (transBotNames.findRow(transIdCol, first->first, transRowIdx))
				{
					ucstring wkBotName = botNames.getData(rowIdx, ucstring("bot name"));
					ucstring wkSheetName = botNames.getData(rowIdx, ucstring("sheet_name"));								
					ucstring wkTranslationName = botNames.getData(rowIdx, ucstring("translated name"));
					ucstring ucWkHash;
					uint64 hash = CI18N::makeHash(wkBotName + wkTranslationName +wkSheetName);						
					CI18N::hashToUCString(hash, ucWkHash);
					ucstring trUcHash = transBotNames[transRowIdx][0];
					bool isWkTranslationNameAGroupName = wkTranslationName.find(ucstring("$")) != ucstring::npos;
					bool hashIsValide = std::equal(ucWkHash.begin(), ucWkHash.end(), trUcHash.begin()+1);
					// Hash is equal get the translation
					if (hashIsValide && !isWkTranslationNameAGroupName)
					{
						wkTranslationName = transBotNames.getData(transRowIdx, ucstring("translated name"));
						wkSheetName = transBotNames.getData(transRowIdx, ucstring("sheet_name"));
						botNames.setData(rowIdx, ucstring("translated name"), wkTranslationName);
						botNames.setData(rowIdx, ucstring("sheet_name"), wkSheetName);
						hash = CI18N::makeHash(wkBotName + wkTranslationName + wkSheetName);						
  						// update the hash code
						CI18N::hashToUCString(hash, transBotNames[transRowIdx][0]);							
					}
					// bots_name.txt has been manually changed. We trust what the Level Designer has done. We don't destroy is work.
					// or it is a simple 
					else
					{
						//use the "translated name" of the manually changed  work/bot_name.txt
						botNames.setData(rowIdx, ucstring("translated name"), wkTranslationName);
						botNames.setData(rowIdx, ucstring("sheet_name"), wkSheetName);	
					}					
				}
			}
		}
	}

	// 2 - generic names
	
	{
		nlinfo("  Generic names...");

		set<string>::iterator first(GenericNames.begin()), last(GenericNames.end());
		for (; first != last; ++first)
		{
			string gnName = "gn_" + cleanupName(*first);

			ucstring fctsTitleId;
			ucstring fctsName;
			// add or modify the bot names
			uint rowIdx;
			if (!botNames.findRow(botIdCol, *first, rowIdx)) 
			{
				// we need to add the entry
				rowIdx = botNames.size();
				botNames.resize(botNames.size()+1);

				botNames.setData(rowIdx, ucstring("bot name"), *first);
				botNames.setData(rowIdx, ucstring("translated name"), ucstring("$") + gnName + "$");
				botNames.setData(rowIdx, ucstring("sheet_name"), ucstring());
				fctsTitleId = gnName;
				fctsName = *first;

				nbAddSimpleName++;
			}
			else
			{
				// look in the translated table to remember the translated name to write it in the string file
				ucstring wkBotName = botNames.getData(rowIdx, ucstring("bot name"));				
				ucstring wkTranslationName = botNames.getData(rowIdx, ucstring("translated name"));
				ucstring wkSheetName = botNames.getData(rowIdx, ucstring("sheet_name"));

				
				nlinfo("Bot name:%s\n",wkBotName.toString().c_str());
				bool isWkTranslationNameAGroupName = wkTranslationName.find(ucstring("$")) != ucstring::npos;
				
				if ( isWkTranslationNameAGroupName ) //work name looks like "$gn_***$: do not modify
				{

					//Do not change work/bot_name.txt
					// update work/world_title.txt

					ucstring transName;
					fctsTitleId = makeGroupName(wkTranslationName);
					uint transRowIdx;
					if (transBotNames.findRow(transIdCol, *first, transRowIdx))
					{
						transName = transBotNames.getData(transRowIdx, ucstring("translated name"));

						if (transName.find(ucstring("$")) != ucstring::npos)
						{
							transName = fctsTitleId;
						}
					}
					else
					{
						transName = fctsTitleId;
					}
					//Do not touch anything
					botNames.setData(rowIdx, ucstring("translated name"), wkTranslationName);
					botNames.setData(rowIdx, ucstring("sheet_name"), wkSheetName); 
					// fctsTitleId = makeGroupName(wkTranslationName);
					fctsName = transName;

				}
				else // WkTranslationName != "$gn*$"
				{
						uint transRowIdx;
						ucstring transName;
						ucstring wkSheetName;
						// Get the translation as a simple name.
						if (transBotNames.findRow(transIdCol, *first, transRowIdx))
						{
		
							transName = transBotNames.getData(transRowIdx, ucstring("translated name"));
							ucstring trSheetName = transBotNames.getData(transRowIdx, ucstring("sheet_name"));

							//tr."translation name" is 
							if (transName.find(ucstring("$")) != ucstring::npos)
							{
								//get Translation, update hash
								botNames[rowIdx][1] = transName;
								botNames[rowIdx][2] = trSheetName;		
								fctsTitleId = makeGroupName(transName);
								fctsName = makeGroupName(transName);
								ucstring trNewUcHash;
								uint64 hash = CI18N::makeHash(wkBotName + transName +trSheetName);						
								CI18N::hashToUCString(hash, trNewUcHash);
								transBotNames[transRowIdx][0] = ucstring("_") + trNewUcHash;
							}
							else //botNames."translated name" != $gn_$ && tansName."translated name" != $gn_$
							{

								// get the translation back
								//update work/bot_name.txt
								wkTranslationName = ucstring("$")+gnName+"$";
								botNames[rowIdx][0] = wkBotName;
								botNames[rowIdx][1] = wkTranslationName;
								botNames[rowIdx][2] = wkSheetName;		
									
									//update translated/bot_name.txt

								fctsName = transName;	//transName	
								fctsTitleId = gnName;
								ucstring trNewUcHash;
								uint64 hash = CI18N::makeHash(botNames[rowIdx][0] + botNames[rowIdx][1] +botNames[rowIdx][2]);						
								CI18N::hashToUCString(hash, trNewUcHash);
								transBotNames[transRowIdx][0] = ucstring("_") + trNewUcHash;
							}

						}
						else //There is no translation yet
						{
								fctsName = wkTranslationName;
								wkTranslationName = ucstring("$")+gnName+"$";
								botNames[rowIdx][0] = wkBotName;
								botNames[rowIdx][1] = wkTranslationName;
								botNames[rowIdx][2] = wkSheetName;		
								fctsTitleId = gnName;

				
						}				
				}

			}


			// look for a corresponding entry
			uint gnNameRow;


			if (!fcts.findRow(fctsIdCol, fctsTitleId, gnNameRow))
			{
				
				// not found, add it
				gnNameRow = fcts.size();
				fcts.resize(fcts.size()+1);
				fcts.setData(gnNameRow, ucstring("title_id"), fctsTitleId);
				fcts.setData(gnNameRow, ucstring("name"), fctsName);										
				nbAddGenericName++;
				
			}
			else //Update 
			{
			
			}
		}
	}


	// 3 - functions
	{
		nlinfo("  Functions...");

		set<string>::iterator first(Functions.begin()), last(Functions.end());
		for (; first != last; ++first)
		{
			string fctName = *first;
			// look for a corresponding entry
			uint functionRow;
			if (!fcts.findRow(fctsIdCol, fctName, functionRow))
			{
				// not found, add it
				functionRow = fcts.size();
				fcts.resize(fcts.size()+1);

				fcts.setData(functionRow, ucstring("title_id"), fctName);
				fcts.setData(functionRow, ucstring("name"), *first);

				nbAddFunction++;
			}
		}
	}

	// display summary
	nlinfo("Adding %u new simple name", nbAddSimpleName);
	nlinfo("Adding %u new generic name", nbAddGenericName);
	nlinfo("Adding %u new function name", nbAddFunction);

	// saving the modified files

	ucstring s = prepareExcelSheet(botNames);
	CI18N::writeTextFile(workBotNamesFile.asString(), s, false);
	s = prepareExcelSheet(transBotNames);
	CI18N::writeTextFile(transBotNamesFile.asString(), s, false);
	s = prepareExcelSheet(fcts);
	CI18N::writeTextFile(workTitleFile.asString(), s, false);

	return 0;
}
