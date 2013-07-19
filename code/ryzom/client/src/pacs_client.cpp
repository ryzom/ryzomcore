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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Client.
#include "pacs_client.h"
#include "user_entity.h"
#include "ig_callback.h"
#include "ig_client.h"

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NLPACS;


////////////
// GLOBAL //
////////////
UMoveContainer		*PACS = 0;
UGlobalRetriever	*GR = 0;
URetrieverBank		*RB = 0;
const float			LRRefeshRadius = 400.f;
CIGCallback			*IGCallbacks = 0;
// World Images
const uint8 staticWI  = 0;	//  Static World Image
const uint8 dynamicWI = 1;	// Dynamic World Image
// Collision Masks.
const UMovePrimitive::TCollisionMask MaskColNone	= 0x00000000;
const UMovePrimitive::TCollisionMask MaskColPlayer	= 0x00000001;
const UMovePrimitive::TCollisionMask MaskColNpc		= 0x00000002;
const UMovePrimitive::TCollisionMask MaskColDoor	= 0x00000004;
const UMovePrimitive::TCollisionMask MaskColAll		= 0xFFFFFFFF;
TPacsPrimMap	PacsPrims;

const uint16 UserDataTree	= 0;
const uint16 UserDataLift	= 1;
const uint16 UserDataDoor	= 2;
const uint16 UserDataEntity	= 3;


///////////////
// FUNCTIONS //
///////////////
//-----------------------------------------------
// initPACS :
// Initialize PACS.
//-----------------------------------------------
void initPACS(const char* rbank, const char* gr, NLMISC::IProgressCallback &/* progress */)
{
	// Check old PACS is well released.
	nlassertex(RB==0,   ("RB should be Null before the init."));
	nlassertex(GR==0,   ("GR should be Null before the init."));
	nlassertex(PACS==0, ("PACS should be Null before the init."));

	if(rbank != 0 && gr != 0)
	{
		RB = NLPACS::URetrieverBank::createRetrieverBank(rbank, false);
		GR = NLPACS::UGlobalRetriever::createGlobalRetriever(gr, RB);
		if (GR)
		{
			CAABBox		cbox = GR->getBBox();

			uint gw = (uint)(cbox.getHalfSize().x*2.0 / RYZOM_ENTITY_SIZE_MAX) + 1;
			uint gh = (uint)(cbox.getHalfSize().y*2.0 / RYZOM_ENTITY_SIZE_MAX) + 1;


			PACS = UMoveContainer::createMoveContainer(GR, gw, gh, RYZOM_ENTITY_SIZE_MAX, 2);
		}
		else
			nlwarning("Could not create global retriever for %s, %s", rbank, gr);
	}

	// Try to create a PACS with another method.
	if(PACS == 0)
		PACS = UMoveContainer::createMoveContainer(15000.0, -25000.0, 20000.0, -20000.0, 16, 16, RYZOM_ENTITY_SIZE_MAX, 2);

	// Set the static world image.
	if(PACS)
		PACS->setAsStatic(staticWI);
	else
		nlwarning("initPACS: cannot create PACS at all.");
}// initPACS //

//-----------------------------------------------
// releasePACS :
// Initialize PACS.
//-----------------------------------------------
void releasePACS ()
{
	// Move container presents ?
	if (PACS)
	{
		UMoveContainer::deleteMoveContainer (PACS);
		PACS = NULL;
	}
	// Global retriever presents ?
	if (GR)
	{
		UGlobalRetriever::deleteGlobalRetriever (GR);
		GR = NULL;
	}
	// Retriever bank loader ?
	if (RB)
	{
		URetrieverBank::deleteRetrieverBank (RB);
		RB = NULL;
	}
}// initPACS //

//-----------------------------------------------
// getCluster :
// Get the cluster from a global position.
//-----------------------------------------------
UInstanceGroup *getCluster(const UGlobalPosition &gp)
{
	// Cannot find the cluster if GR is Null.
	if(GR==0)
		return 0;

	const string &strPos = GR->getIdentifier(gp);
	if(strPos.empty())
		return 0;
	// try to find the ig in the loaded ig map
	std::map<std::string, UInstanceGroup *>::const_iterator igIt = IGLoaded.find(strlwr(strPos));
	if (igIt != IGLoaded.end())
	{
		return igIt->second;
	}

	// searh in the fyros city igs
	if(strPos == "col_appart")
		return IGCity["apart.ig"];
	else if(strPos == "col_forge")
		return IGCity["forge.ig"];
	else if(strPos == "col_mairie")
		return IGCity["mairie.ig"];
	else if(strPos == "col_taverne")
		return IGCity["taverne.ig"];
	else if(strPos == "col_warschool")
		return IGCity["warschool.ig"];
	else if("col_street_1" || "col_street_2")
		return IGCity["street.ig"];
	else
	{
		nlwarning("getCluster : %s : unknown Identifier.", strPos.c_str());
		return 0;
	}
}// getCluster //

//-----------------------------------------------
void initLandscapeIGCallbacks()
{
	releaseLandscapeIGCallbacks();
	nlassert(IGCallbacks == NULL); // should be initilized only once!
	IGCallbacks = new CIGCallback();
}


//-----------------------------------------------
void releaseLandscapeIGCallbacks()
{
	delete IGCallbacks;
	IGCallbacks = NULL;
}

///===================================================================================

void addPacsPrim(const std::string &fileName)
{
	std::string ppName = NLMISC::strlwr(NLMISC::CFile::getFilenameWithoutExtension(fileName));
	if (PacsPrims.find(ppName) != PacsPrims.end())
	{
		nlwarning(("Pacs primitive " + ppName + " already has been inserted").c_str());
		return;
	}
	std::auto_ptr<NLPACS::UPrimitiveBlock> pb(NLPACS::UPrimitiveBlock::createPrimitiveBlockFromFile(fileName));
	PacsPrims[ppName] = pb.release();
}

///===================================================================================

void deletePrimitiveBlocks()
{
	for(TPacsPrimMap::iterator it = PacsPrims.begin(); it != PacsPrims.end(); ++it)
	{
		delete it->second;
	}
	PacsPrims.clear();
}

///===================================================================================

void initPrimitiveBlocks()
{
	//-----------------------------
	// load pacs landscape collisions
	//-----------------------------

	static const char primitiveListFile[] = "landscape_col_prim_pacs_list.txt";
	std::string lookupPathPrimList = CPath::lookup(primitiveListFile, false, true);
	if (lookupPathPrimList.empty())
	{
		nlwarning("Unable to find the file containing pacs primitives list : %s", primitiveListFile);
		return;
	}
	//
	char tmpBuff[300];
	NLMISC::CIFile inputFile;
	if(!inputFile.open(lookupPathPrimList, false))
	{
		nlwarning("Couldn't open %s", primitiveListFile);
		return;
	}
	//
	while(!inputFile.eof())
	{
		inputFile.getline(tmpBuff, 300);
		std::string primFile = CPath::lookup(tmpBuff, false, true);
		if (!primFile.empty())
		{
			try
			{
				addPacsPrim(primFile);
			}
			catch (const NLMISC::Exception &)
			{
				nlwarning("Error while loading %s", primFile.c_str());
			}
		}
		else
		{
			nlwarning("Couldn't find %s", tmpBuff);
		}
	}
	//
	inputFile.close();
}
