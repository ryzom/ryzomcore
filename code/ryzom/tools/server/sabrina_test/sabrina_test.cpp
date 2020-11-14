/** \file sabrina_test.cpp
 *
 * $Id: sabrina_test.cpp,v 1.2 2004/03/01 19:22:19 lecroart Exp $
 */



#include "nel/misc/command.h"
#include "nel/misc/path.h"

#include "game_share/ryzom_entity_id.h"
#include "game_share/tick_event_handler.h"

#include "sabrina_test.h"
#include "sabrina/sabrina_pointers.h"
#include "sabrina/sabrina_phrase_description.h"
#include "sabrina/sabrina_phrase_manager.h"


using namespace NLMISC;
using namespace NLNET;
using namespace std;

//---------------------------------------------------
//	CallbackArray
//---------------------------------------------------
TUnifiedCallbackItem CallbackArray[] = 
{
	{	"AZERTY",	NULL	},
};

//---------------------------------------------------
//	Globals
//---------------------------------------------------
namespace SABTEST
{
	CServiceTest* TheService;
	uint32 SimTickCount;
}


//---------------------------------------------------
// Service Init :
// 
//---------------------------------------------------
void CServiceTest::init ()
{
	SABTEST::TheService=this;
	setUpdateTimeout(100);

	CTickEventHandler::init(SABTEST::TheService->tickUpdate);

	CSabrinaStaticPhraseDescriptionManager::init();
	CSabrinaPhraseManager::init();
}

//---------------------------------------------------
// Service update :
// 
//---------------------------------------------------
bool CServiceTest::update ()
{
	if (SABTEST::SimTickCount)
	{
		CTickEventHandler::simulateTick();
		--SABTEST::SimTickCount;
	}
	return true;
}

//---------------------------------------------------
// Tick update :
// 
//---------------------------------------------------
void CServiceTest::tickUpdate ()
{
	CSabrinaPhraseManager::update();
}

//---------------------------------------------------
// Service release :
// 
//---------------------------------------------------
void CServiceTest::release ()	
{
	CSabrinaStaticPhraseDescriptionManager::release();
	CSabrinaPhraseManager::release();
}

/****************************************************************\
						Service register
\****************************************************************/
NLNET_SERVICE_MAIN (CServiceTest, "SabrinaTest", "sabrina_test", 0, CallbackArray, "", "")


/****************************************************************\
						Command section
\****************************************************************/

ISabrinaActor* getActorByName(const CSString& name)
{
	CEntityBase* result=NULL;

	result= SABTEST::plrMgr()->getPlayer(name);
	if (result!=NULL)
		return result->getSabrinaActor();

	result= SABTEST::botMgr()->getBot(name);
	if (result!=NULL)
		return result->getSabrinaActor();

	return NULL;
}

NLMISC_COMMAND(newPlayer,"createPlayer","<name>")
{
	if(args.size() != 1) return false;
	if (getActorByName(args[0])!=NULL)
	{
		nlinfo("Actor already exists: %s",args[0].c_str());
		return true;
	}
	SABTEST::plrMgr()->addPlayer(args[0]);
	return true;
}

NLMISC_COMMAND(newNPC,"createNPC","<name>")
{
	if(args.size() != 1) return false;
	if (getActorByName(args[0])!=NULL)
	{
		nlinfo("Actor already exists: %s",args[0].c_str());
		return true;
	}
	SABTEST::botMgr()->addNPC(args[0]);
	return true;
}

NLMISC_COMMAND(newCreature,"createCreature","<name> <sheet>")
{
	if(args.size() != 2) return false;
	if (getActorByName(args[0])!=NULL)
	{
		nlinfo("Actor already exists: %s",args[0].c_str());
		return true;
	}
	SABTEST::botMgr()->addCreature(args[0],NLMISC::CSheetId(args[1]));
	return true;
}


NLMISC_COMMAND(plrTarget,"plrTarget","<name> <tgt>")
{
	if(args.size() != 2) return false;

	ISabrinaActor* actor= getActorByName(args[0]);
	ISabrinaActor* target= getActorByName(args[1]);

	if (actor==NULL)
	{
		nlinfo("Failed to identify actor: %s",args[0].c_str());
		return true;
	}
	if (target==NULL)
	{
		nlinfo("Failed to identify target: %s",args[1].c_str());
		return true;
	}

	actor->getEntity()->setTarget(target->getEntity());

	return true;
}


NLMISC_COMMAND(plrMemSPhrase,"plrMemSPhrase","<player> <memory bank name> <slot in memory bank> <sphrase file name>")
{
	if(args.size() != 4) return false;

	// get a pointer to the player.
	CSabrinaActorPlayer* actor= const_cast<CSabrinaActorPlayer*>(SABTEST::plrMgr()->getPlayer(args[0])->getSabAct());
	if (actor==NULL)
	{
		nlinfo("Failed to identify player: %s",args[0].c_str());
		return true;
	}

	// convert 'slot' param to int
	uint32 slot;
	if ( CSString(args[2]).atoi(slot)==false )
	{
		nlwarning("Invalid slot...");
		return false;
	}

	// lookup the phrase in the static phrase table
	ISabrinaPhraseDescriptionPtr phrase= CSabrinaStaticPhraseDescriptionManager::getPhrase(NLMISC::CSheetId(args[3]));
	if (phrase==NULL)
	{
		nlwarning("Phrase file not found...");
		return false;
	}

	// do the memorising
	actor->memorize(args[1],slot,phrase);
	return true;
}

NLMISC_COMMAND(plrMemUserPhrase,"plrMemSPhrase","<player> <memory bank name> <slot in memory bank> <phrase name> <brick>[<brick>[...]]")
{
	if(args.size() < 5) return false;

	// get a pointer to the player.
	CSabrinaActorPlayer* actor= const_cast<CSabrinaActorPlayer*>(SABTEST::plrMgr()->getPlayer(args[0])->getSabAct());
	if (actor==NULL)
	{
		nlinfo("Failed to identify player: %s",args[0].c_str());
		return true;
	}

	// convert 'slot' params to int
	uint32 slot;
	if ( CSString(args[2]).atoi(slot)==false )
		return false;

	// setup a vector of bricks
	std::vector<NLMISC::CSheetId> bricks;
	for (uint32 i=4;i<args.size();++i)
		bricks.push_back(NLMISC::CSheetId(args[i]));

	// do the memorising
	ISabrinaPhraseDescriptionPtr newPhrase= (ISabrinaPhraseDescription*)new CSabrinaPhraseDescriptionUser(args[3],bricks);
	actor->memorize(args[1],slot,newPhrase);
	return true;
}

NLMISC_COMMAND(plrUsePhrase, "plrUsePhrase ", "<player> <memory bank name> <slot in memory bank>")
{
	if(args.size() != 3) return false;

	// get a pointer to the player.
	CSabrinaActorPlayer* actor= const_cast<CSabrinaActorPlayer*>(SABTEST::plrMgr()->getPlayer(args[0])->getSabAct());
	if (actor==NULL)
	{
		nlinfo("Failed to identify player: %s",args[0].c_str());
		return true;
	}

	// convert 'slot' params to string
	uint32 slot;
	if ( CSString(args[2]).atoi(slot)==false )
		return false;

	// do the memorising
	actor->setActiveMemoryBank(args[1]);
	actor->executeAction(slot);
	return true;
}


NLMISC_COMMAND(listActors,"listEntities","")
{
	if(args.size() != 0) return false;
	SABTEST::botMgr()->listCreatures();
	SABTEST::botMgr()->listNPCs();
	SABTEST::plrMgr()->listPlayers();
	return true;
}

NLMISC_COMMAND(simTick,"simulate 1 or more tick","[<tick count>]")
{
	uint32 count=1;
	if (args.size()>0)
		count=atoi(args[0].c_str());
	if (count==0)
		return false;

	SABTEST::SimTickCount=count;
	return true;
}

NLMISC_COMMAND(makeSheetId,"make sheet id file from scratch","")
{
	if(args.size() != 0) return false;

	std::string fileName="src_v2/sabrina_test/sheet_id.bin.test";
	nlinfo("Creating file: %s",fileName.c_str());

	std::map<NLMISC::CSheetId,std::string> allFiles;

	std::vector<std::string> itemFiles;
	std::vector<std::string> brickFiles;
	std::vector<std::string> phraseFiles;

	NLMISC::CPath::getFileList("sitem",itemFiles);
	NLMISC::CPath::getFileList("sbrick",brickFiles);
	NLMISC::CPath::getFileList("sphrase",phraseFiles);

	nlinfo("adding %d sitem files",itemFiles.size());
	nlinfo("adding %d sbrick files",brickFiles.size());
	nlinfo("adding %d sphrase files",phraseFiles.size());

	uint32 i;
	for (i=itemFiles.size();i--;)	allFiles[NLMISC::CSheetId().build(i+1,1)]= itemFiles[i];
	for (i=brickFiles.size();i--;)	allFiles[NLMISC::CSheetId().build(i+1,2)]= brickFiles[i];
	for (i=phraseFiles.size();i--;)	allFiles[NLMISC::CSheetId().build(i+1,3)]= phraseFiles[i];

	NLMISC::COFile file(fileName);
	file.serialCont(allFiles);
	file.close();

	return true;
}
