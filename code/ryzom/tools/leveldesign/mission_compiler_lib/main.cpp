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

#include "nel/misc/i18n.h"
#include "mission_compiler.h"
#include "nel/misc/config_file.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

class CMissionData;


class IStep;

int main(int argc, char *argv[])
{
	new NLMISC::CApplicationContext;
	CPath::addSearchPath("L:\\primitives\\", true, false);

	bool test = false;
	if (argc == 4 && string(argv[3]) == "-test")
	{
		test = true;
	}
	else if ( argc != 3)
	{
		printf("%s <world_edit_class> <primitive_file> [-test]", argv[0]);
		return -1;
	}
	
	string sourceDocName;
	if (!test)
		sourceDocName = argv[2];
	else
		sourceDocName = "test_compilateur.primitive";
	
	// remove the path
	sourceDocName = CFile::getFilename(sourceDocName);
	// init ligo
	NLLIGO::CLigoConfig LigoConfig;

	CPrimitiveContext::instance().CurrentLigoConfig = &LigoConfig;
	
	nlinfo("Reading ligo configuration file...");
	if (!LigoConfig.readPrimitiveClass (argv[1], false))
	{
		nlwarning("Can't read '%s' !", argv[1]);
		return -1;
	}
	
	NLLIGO::Register();

	nlinfo("Reading primitive file...");
	
	CPrimitives		primDoc;
	CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
	loadXmlPrimitiveFile(primDoc, sourceDocName, LigoConfig);
	
	CMissionCompiler	mc;

	if (test)
	{
		nlinfo("Compiling test mission");

		try
		{
			mc.compileMissions(primDoc.RootNode, sourceDocName);
			TMissionDataPtr testMission = mc.getMission(0);

			CSString script = testMission->generateMissionScript(sourceDocName);
			script += "======================================================"+NL;
			script += testMission->generatePhraseFile();
			script += "======================================================"+NL;
			script += testMission->generateDotScript();
			script = script.replace(NL.c_str(), "\n");

			char *tmp = ::getenv("TEMP");
		
			FILE *fp = ::fopen((string(tmp)+"/compiled_mission.script").c_str(), "w");
			::fwrite(script.data(), script.size(), 1, fp);
			::fclose(fp);

			system((string("\"C:\\Program Files\\Beyond Compare 2\\bc2.exe\" ")+string(tmp)+"/compiled_mission.script test_compilateur.script").c_str());
		}
		catch(const EParseException &e)
		{
			nlwarning(e.Why.c_str());
			return -1;
		}
		return 0;
	}
	
	nlinfo("Compiling missions...");
	try
	{
		mc.compileMissions(primDoc.RootNode, sourceDocName);

		mc.installCompiledMission(LigoConfig, sourceDocName);
/*		std::vector <TMissionDataPtr> &missions = mc.getMissions();
		// generate the mission script into the npcs...
		{
			map<string, TLoadedPrimitive >	loadedPrimitives;

			// First loop to remove any mission that belong to the compiled primitive file
			for (uint i=0; i<missions.size(); ++i)
			{
				CMissionData &mission = *(missions[i]);
				// first, look for the primitive file to load
				string fileName = mission.getGiverPrimitive();
				if (fileName.empty())
				{
					// use mission primitive instead
					fileName = sourceDocName;
				}
				if (loadedPrimitives.find(fileName) == loadedPrimitives.end())
				{
					string fullFileName = CPath::lookup(fileName);
					if (fullFileName.empty())
					{
						nlwarning("Can't find primitive file '%s' in path", fileName.c_str());
						throw EParseException(NULL, "Destination primitive file not found");
					}
					// we need to load this primitive file.
					CPrimitives *primDoc = new CPrimitives;
					if (loadXmlPrimitiveFile(*primDoc, fullFileName, LigoConfig))
					{
						// the primitive file is loaded correctly
						loadedPrimitives.insert(make_pair(fileName, TLoadedPrimitive(primDoc, fullFileName)));
					}
					else
						throw EParseException(NULL, "Can't read primitive file");
				}
				TLoadedPrimitive &loadedPrim = loadedPrimitives[fileName];
				CPrimitives *primDoc = loadedPrim.PrimDoc;

				TPrimitiveSet scripts;
				CPrimitiveSet<TPrimitiveClassPredicate> filter;
				filter.buildSet(primDoc->RootNode, TPrimitiveClassPredicate("mission"), scripts);
				
				// for each script, check if it was generated, and if so, check the name
				// of the source primitive file.
				for (uint i=0; i<scripts.size(); ++i)
				{
					vector<string> *script;
					if (scripts[i]->getPropertyByName("script", script) && !script->empty())
					{						
						// Format should be : #compiled from <source_primitive_name>
						if (script->front().find("compiled from"))
						{
							// we have a compiled mission
							if (script->front().find(sourceDocName))
							{
								// ok, this mission is compiled from the same primitive, remove it
								scripts[i]->getParent()->removeChild(scripts[i]);
							}
						}
					}
				}
			}

			// second loop to assign compiled mission to giver npc
			for (uint i=0; i<missions.size(); ++i)
			{
				CMissionData &mission = *(missions[i]);
				string fileName = mission.getGiverPrimitive();
				if (fileName.empty())
				{
					// no giver primitive file specified in the mission, use the mission primitive instead
					fileName = sourceDocName;
				}

				TLoadedPrimitive &loadedPrim = loadedPrimitives[fileName];
				CPrimitives *primDoc = loadedPrim.PrimDoc;

				TPrimitiveSet bots;
				CPrimitiveSet<TPrimitiveClassAndNamePredicate> filter;
				filter.buildSet(primDoc->RootNode, TPrimitiveClassAndNamePredicate("npc_bot", mission.getGiverName()), bots);

				if (bots.empty())
				{
					nlwarning("Can't find bot '%s' in primitive '%s' !", 
						mission.getGiverName().c_str(),
						fileName.c_str());
					throw EParseException(NULL, "Can't find giver in primitive");
				}
				else if (bots.size() > 1)
				{
					nlwarning("Found more than one bot named '%s' in primitive '%s' !", 
						mission.getGiverName().c_str(),
						fileName.c_str());
					throw EParseException(NULL, "More than one bot with giver name in primitive");
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
				script->addPropertyByName("alias", new CPropertyString(mission.getAlias()));
				string scriptLines = mission.generateMissionScript();
				vector<string> lines;
				explode(scriptLines, NL, lines, false);

				script->addPropertyByName("script", new CPropertyStringArray(lines));

				// insert the script into the giver
				giver->insertChild(script);
			}

			// Save the modified primitive files
			while (!loadedPrimitives.empty())
			{
				TLoadedPrimitive &loadedPrim = loadedPrimitives.begin()->second;
				saveXmlPrimitiveFile(*(loadedPrim.PrimDoc), loadedPrim.FullFileName);

				// Free the memory
				delete loadedPrim.PrimDoc;

				loadedPrimitives.erase(loadedPrimitives.begin());
			}
		}

		// generate the phrase file (in any)
		{
			string phraseFileName = CFile::getFilenameWithoutExtension(sourceDocName) + "_en.txt";

			CSString content;

			for (uint i=0; i<missions.size(); ++i)
			{
				content += missions[i]->generatePhraseFile();
			}
			// transform NL (\n\r) into single \n
			content = content.replace(NL.c_str(), "\n");
			ucstring ucs;
			ucs.fromUtf8(content);

			CI18N::writeTextFile(phraseFileName, ucs, true);
		}
*/
	}
	catch (const EParseException &e)
	{
		CPrimitiveContext::instance().CurrentLigoConfig = NULL;
		nlerror("Compilation error : '%s'", e.Why.c_str());
	}

	CPrimitiveContext::instance().CurrentLigoConfig = NULL;
}


