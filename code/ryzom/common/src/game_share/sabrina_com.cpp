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



#include "stdpch.h"
#include "sabrina_com.h"

using namespace NLMISC;
using namespace std;

// ***************************************************************************
CSabrinaCom::CSabrinaCom(IBrickContainer *bc)
{
	nlassert(bc);
	_BC= bc;
}

// ***************************************************************************
CSabrinaCom::~CSabrinaCom()
{
	_BC= NULL;
}

// ***************************************************************************
void		CSabrinaCom::getPhraseCost(const std::vector<NLMISC::CSheetId> &bricks, uint32 &pos, uint32 &neg) const
{
	pos= 0;
	neg= 0;
	float rpos= 1.0f;
	float rneg= 1.0f;
	// Yoyo: for now easy, sum pos or neg SabrinaCost
	for(uint i=0;i<bricks.size();i++)
	{
		sint32	cost= _BC->getSabrinaCost(bricks[i]);
		if(cost>0)
			pos+= cost;
		else
			neg+= (-cost);

		float rcost = _BC->getSabrinaRelativeCost(bricks[i]);
		if( rcost>0.f)
			rpos+= rcost;
		else
			rneg-= rcost;
	}

	pos = (uint32)(pos * rpos);
	neg = (uint32)(neg * rneg);
}

// ***************************************************************************
uint32		CSabrinaCom::getPhraseMaxBrickCost(const std::vector<NLMISC::CSheetId> &bricks) const
{
	uint32 costMax= 0;
	for(uint i=0;i<bricks.size();i++)
	{
		sint32	cost= _BC->getSabrinaCost(bricks[i]);
		if(cost>0)
			costMax= max(costMax, (uint32)cost);
	}

	return costMax;
}

// ***************************************************************************
sint32		CSabrinaCom::getPhraseBrickAndParamCost(const std::vector<NLMISC::CSheetId> &bricks, uint brickIndex) const
{
	if(brickIndex>=bricks.size())
		return 0;

	sint32	res= 0;

	// Yoyo: for now easy, sum the brick and all params cost.
	uint		numParams= 0;

	// Add the brick and its params cost.
	for(uint i=brickIndex;i<min((uint)bricks.size(), brickIndex+numParams+1);i++)
	{
		res+= _BC->getSabrinaCost(bricks[i]);
		// If this brick adds a parameter (main brick or base parameter)
		numParams+= _BC->getNumParameters(bricks[i]);
	}

	return res;
}


// ***************************************************************************
float		CSabrinaCom::getPhraseBrickAndParamRelativeCost(const std::vector<NLMISC::CSheetId> &bricks, uint brickIndex) const
{
	if(brickIndex>=bricks.size())
		return 0.f;

	float	res= 0.f;

	// Yoyo: for now easy, sum the brick and all params relative cost.
	uint		numParams= 0;

	// Add the brick and its params cost.
	for(uint i=brickIndex;i<min((uint)bricks.size(), brickIndex+numParams+1);i++)
	{
		res+= _BC->getSabrinaRelativeCost(bricks[i]);
		// If this brick adds a parameter (main brick or base parameter)
		numParams+= _BC->getNumParameters(bricks[i]);
	}

	return res;
}


// ***************************************************************************
void		CSabrinaCom::filterMandatoryComposition(const std::vector<NLMISC::CSheetId> &/* phraseBricks */, std::vector<NLMISC::CSheetId> &/* mandatoryBricks */) const
{
	// Yoyo: for now, no simple rule known => never exclude mandatories.
}

// ***************************************************************************
TOOL_TYPE::TCraftingToolType		CSabrinaCom::getPhraseFaberPlanToolType(const std::vector<NLMISC::CSheetId> &phraseBricks) const
{
	// Valid only if not empty, and a faber one,
	if( phraseBricks.size()>=1 && _BC->getBrickType(phraseBricks[0])== BRICK_TYPE::FABER )
	{
		return _BC->getFaberPlanToolType(phraseBricks[0]);
	}

	return TOOL_TYPE::Unknown;
}

// ***************************************************************************
NLMISC::CSheetId	CSabrinaCom::getPhraseBestDisplayBrick(const std::vector<NLMISC::CSheetId> &phraseBricks) const
{
	if(phraseBricks.empty())
		return CSheetId();

	BRICK_TYPE::EBrickType	bType= _BC->getBrickType(phraseBricks[0]);

	// Magic? take the coster mandatory brick.
	// Combat? take the coster optional brick
	if( bType== BRICK_TYPE::MAGIC ||
		bType== BRICK_TYPE::COMBAT )
	{
		// default still to root (error case)
		uint	best= 0;
		uint	bestCost= 0;

		// The cost of the effect is its + all its params.
		for(uint i=1;i<phraseBricks.size();)
		{
			uint	effectCost= 0;
			uint	effectId= i;

			// If the brick is a MagicEffect, or a CombatOptional
			uint indexInFamily;
			CSheetId	brickId= phraseBricks[i];
			BRICK_FAMILIES::TBrickFamily	bf= _BC->getBrickFamily(brickId, indexInFamily);
			if( (BRICK_FAMILIES::isMandatoryFamily(bf) && bType==BRICK_TYPE::MAGIC) ||
				(BRICK_FAMILIES::isOptionFamily(bf) && bType==BRICK_TYPE::COMBAT) )
			{
				effectCost= _BC->getSabrinaCost(brickId);

				// next brick
				i++;

				// add its params cost
				uint	numParams= _BC->getNumParameters(brickId);
				for(;numParams>0 && i<phraseBricks.size();i++,numParams--)
				{
					CSheetId	brickId= phraseBricks[i];
					effectCost+= _BC->getSabrinaCost(brickId);
				}
			}
			else
				// next brick
				i++;

			// coster?
			if(effectCost>bestCost)
			{
				bestCost= effectCost;
				best= effectId;
			}
		}

		return phraseBricks[best];
	}
	// SpecialPower? take the first mandatory brick.
	else if(bType== BRICK_TYPE::SPECIAL_POWER)
	{
		if( phraseBricks.size()>=3 )
			return phraseBricks[2];
		if( phraseBricks.size()>=2 )
			return phraseBricks[1];
	}
	// Prospection & extraction?
	// Take the last most expensive brick (except the root and ecotype specializing)
	else if ( (bType == BRICK_TYPE::FORAGE_PROSPECTION) || (bType == BRICK_TYPE::FORAGE_EXTRACTION) )
	{
		uint bestIndex = 0;
		uint bestCost = 0;
		uint maxIndexInExtractionFamily = 0;
		for ( uint i=1; i<phraseBricks.size(); ++i ) // skip the root brick (index 0)
		{
			const CSheetId& brickId = phraseBricks[i];

			// Skip counterpart (because their value is positive), and skip any ecotype specializing brick
			uint indexInFamily;
			BRICK_FAMILIES::TBrickFamily brickFamily = _BC->getBrickFamily( brickId, indexInFamily );
			if ( brickFamily == BRICK_FAMILIES::Unknown )
				continue;
			if ( BRICK_FAMILIES::isCreditFamily( brickFamily ) )
				continue;

			// Do not print ecosystem specialization if not alone in the phrase
			if ( (brickFamily == BRICK_FAMILIES::BHFPMA) && (phraseBricks.size() > 4) )
				continue;
			if ( (brickFamily == BRICK_FAMILIES::BHFEMA) && ((maxIndexInExtractionFamily != 1) || (phraseBricks.size() != 7)) ) // works only for default phrase where eco. spec. is at the end
				continue;

			// Calc the last highest cost
			uint brickCost = _BC->getSabrinaCost( brickId);
			if ( brickCost >= bestCost )
			{
				bestCost = brickCost;
				bestIndex = i;
			}
			if ( (brickFamily >= BRICK_FAMILIES::BHFEEA) && (brickFamily <= BRICK_FAMILIES::BHFEEC) && (indexInFamily > maxIndexInExtractionFamily) )
				maxIndexInExtractionFamily = indexInFamily;
		}
		if ( (bType == BRICK_TYPE::FORAGE_EXTRACTION) && (maxIndexInExtractionFamily==1) && (phraseBricks.size()==5) )
			bestIndex = 0; // "Basic Extraction": index of the root brick
		return phraseBricks[bestIndex];
	}

	// Other cases: return the rootBrick.
	return phraseBricks[0];
}


// ***************************************************************************
bool	CSabrinaCom::isMainDisplayIconInOverSlot(const NLMISC::CSheetId &brickId, bool& iconOver2NotSuitableForActionDisplay) const
{
	uint indexInFamily;
	BRICK_FAMILIES::TBrickFamily brickFamily = _BC->getBrickFamily( brickId, indexInFamily );

	iconOver2NotSuitableForActionDisplay = ((brickFamily >= BRICK_FAMILIES::BeginForageProspection) && (brickFamily <= BRICK_FAMILIES::EndForageExtraction));

	// True only for the 3 extraction bricks
	return ((brickFamily >= BRICK_FAMILIES::BHFEEA) && (brickFamily <= BRICK_FAMILIES::BHFEEC));
}
