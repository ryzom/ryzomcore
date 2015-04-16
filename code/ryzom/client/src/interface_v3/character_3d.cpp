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



// ------------------------------------------------------------------------------------------------
#include "stdpch.h"
#include "character_3d.h"
#include "interface_manager.h"

#include "../color_slot_manager.h"
#include "../sheet_manager.h"
#include "../gabarit.h"
#include "../misc.h"
#include "../time_client.h"
#include "../player_cl.h"
#include "../player_r2_cl.h"
#include "../entities.h"

#include "../r2/editor.h"
#include "../client_cfg.h"

// ------------------------------------------------------------------------------------------------
using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace R2;

extern CEntityAnimationManager	*EAM;


// ------------------------------------------------------------------------------------------------
// SCharacter3DSetup
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
SCharacter3DSetup::SCharacter3DSetup ()
{
	// Setup a naked male fyros
	LeftHandItemIsShield = false;
	People = EGSPD::CPeople::Fyros;
	Male = true;
	Skeleton = "fy_hom_skel.skel";
	AnimPlayed = 0;
	Parts[Char3DPart_Chest].Name = "FY_HOM_underwear_gilet.shape";
	Parts[Char3DPart_Legs].Name = "FY_HOM_underwear_pantabottes.shape";
	Parts[Char3DPart_Arms].Name = "FY_HOM_underwear_armpad.shape";
	Parts[Char3DPart_Feet].Name = "FY_HOM_underwear_bottes.shape";
	Parts[Char3DPart_Face].Name = "FY_HOM_visage.shape";
	Parts[Char3DPart_Head].Name = "FY_HOM_cheveux_medium01.shape";
	Parts[Char3DPart_Hands].Name = "TR_HOM_underwear_hand.shape";
	Parts[Char3DPart_HandRightItem].Name = "";
	Parts[Char3DPart_HandLeftItem].Name = "";
	for (uint32 i = 0; i < NB_CHARACTER3D_PARTS; ++i)
	{
		Parts[i].Color = 0;
		Parts[i].Quality = -1;
	}
	Tattoo = 0;
	EyesColor = 0;
	HairColor = 0;
	CharHeight = 0.0f;
	ChestWidth = 0.0f;
	ArmsWidth = 0.0f;
	LegsWidth = 0.0f;
	BreastSize = 0.0f;
	HideFace = false;

	for (uint32 i = 0; i < NB_MORPH_TARGETS; ++i)
		MorphTarget[i] = 0.0f;
}

// ------------------------------------------------------------------------------------------------
static CGenderInfo *getGenderInfo (EGSPD::CPeople::TPeople ePeople, bool bMale)
{
	// Read in the race_stats forms the default equipement
	CSheetId RSid;
	switch (ePeople)
	{
		case EGSPD::CPeople::Tryker:	RSid = CSheetId("tryker.race_stats"); break;
		case EGSPD::CPeople::Matis:	RSid = CSheetId("matis.race_stats"); break;
		case EGSPD::CPeople::Zorai:	RSid = CSheetId("zorai.race_stats"); break;
		case EGSPD::CPeople::Fyros:
		default:
			RSid = CSheetId("fyros.race_stats"); break;
	}
	CRaceStatsSheet *pRSS = dynamic_cast<CRaceStatsSheet*>(SheetMngr.get (RSid));

	if (pRSS == NULL)
	{
		nlwarning ("cannot find sheet for people:%d male:%d", ePeople, bMale);
		return NULL;
	}

	// Choose default stuff is we are male or female
	CGenderInfo *pGI;
	if (bMale)
		pGI = &pRSS->GenderInfos[0];
	else
		pGI = &pRSS->GenderInfos[1];

	return pGI;
}

// ------------------------------------------------------------------------------------------------
void SCharacter3DSetup::setupDefault (EGSPD::CPeople::TPeople eRace, bool bMale)
{
	People = eRace;
	Male = bMale;

	CGenderInfo *pGI = getGenderInfo(eRace, bMale);
	if (pGI == NULL) return;

	Skeleton = pGI->Skelfilename;

	// Read all the default equipement
	for (sint32 i = 0; i < SLOTTYPE::NB_SLOT; ++i)
	{
		string ISstr = pGI->Items[i]; // All the strings are items
		if (!ISstr.empty())
		{
			CItemSheet *pIS = dynamic_cast<CItemSheet*>(SheetMngr.get(CSheetId(ISstr)));
			if (pIS != NULL)
			{
				sint32 cpIndex = convert_VisualSlot_To_Char3DPart ((SLOTTYPE::EVisualSlot)i);
				if (cpIndex != Char3DPart_INVALID)
				{
					Parts[cpIndex].Quality = pIS->MapVariant;
					if (Male)
						Parts[cpIndex].Name = pIS->getShape();
					else
						Parts[cpIndex].Name = pIS->getShapeFemale();
				}
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
void SCharacter3DSetup::setupFromCharacterSummary (const CCharacterSummary &cs)
{
	const SPropVisualA::SPropSubData &rPVA = cs.VisualPropA.PropertySubData;
	const SPropVisualB::SPropSubData &rPVB = cs.VisualPropB.PropertySubData;
	const SPropVisualC::SPropSubData &rPVC = cs.VisualPropC.PropertySubData;

	setupDefault (cs.People, (rPVA.Sex == 0));

	// Setup char3dParts with additionnal info
	setupFromCS_ModelCol (SLOTTYPE::CHEST_SLOT,			rPVA.JacketModel,		rPVA.JacketColor);
	setupFromCS_ModelCol (SLOTTYPE::LEGS_SLOT,			rPVA.TrouserModel,		rPVA.TrouserColor);
	setupFromCS_ModelCol (SLOTTYPE::HEAD_SLOT,			rPVA.HatModel,			rPVA.HatColor);

	// Should have to hide the face ?
	{
//		TChar3DPart part = convert_VisualSlot_To_Char3DPart (SLOTTYPE::HEAD_SLOT);
		CItemSheet *item = SheetMngr.getItem (SLOTTYPE::HEAD_SLOT, rPVA.HatModel);
		if ((item != NULL) && ((item->Family == ITEMFAMILY::ARMOR) || (item->Family == ITEMFAMILY::SHIELD)))
			HideFace = true;
		else
			HideFace = false;
	}

	setupFromCS_ModelCol (SLOTTYPE::ARMS_SLOT,			rPVA.ArmModel,			rPVA.ArmColor);
	// setupFromCS_Model (SLOTTYPE::FACE_SLOT,			?????????????);
	setupFromCS_ModelCol (SLOTTYPE::FEET_SLOT,			rPVB.FeetModel,			rPVB.FeetColor);
	setupFromCS_ModelCol (SLOTTYPE::RIGHT_HAND_SLOT,	rPVA.WeaponRightHand,	0);
	setupFromCS_ModelCol (SLOTTYPE::LEFT_HAND_SLOT,		rPVA.WeaponLeftHand,	0);
	// armor gloves are not displayed if character has the 'weapon' magician gloves
	{
		CItemSheet *item = SheetMngr.getItem (SLOTTYPE::RIGHT_HAND_SLOT, rPVA.WeaponRightHand);
		if( ! ((item != NULL)&&(item->ItemType == ITEM_TYPE::MAGICIAN_STAFF) ) )
			setupFromCS_ModelCol (SLOTTYPE::HANDS_SLOT,			rPVB.HandsModel,		rPVB.HandsColor);
	}
	Tattoo = rPVC.Tattoo;
	HairColor = rPVA.HatColor; // TODO : For the moment no diff between hair color and head color !!!
	EyesColor = rPVC.EyesColor;
	CharHeight = (rPVC.CharacterHeight - 7.0f) / 7.0f;
	ChestWidth = (rPVC.TorsoWidth - 7.0f) / 7.0f;
	ArmsWidth = (rPVC.ArmsWidth - 7.0f) / 7.0f;
	LegsWidth = (rPVC.LegsWidth - 7.0f) / 7.0f;
	BreastSize = (rPVC.BreastSize - 7.0f) / 7.0f;

	float MTmin, MTmax;
	CGenderInfo *pGI = getGenderInfo (cs.People, (rPVA.Sex == 0));
	if (pGI == NULL)
		return;
	MTmin = pGI->BlendShapeMin[0];
	MTmax = pGI->BlendShapeMax[0];
	if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
	MorphTarget[0] = rPVC.MorphTarget1 / 7.0f * (MTmax - MTmin) + MTmin;
	MTmin = pGI->BlendShapeMin[1];
	MTmax = pGI->BlendShapeMax[1];
	if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
	MorphTarget[1] = rPVC.MorphTarget2 / 7.0f * (MTmax - MTmin) + MTmin;
	MTmin = pGI->BlendShapeMin[2];
	MTmax = pGI->BlendShapeMax[2];
	if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
	MorphTarget[2] = rPVC.MorphTarget3 / 7.0f * (MTmax - MTmin) + MTmin;
	MTmin = pGI->BlendShapeMin[3];
	MTmax = pGI->BlendShapeMax[3];
	if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
	MorphTarget[3] = rPVC.MorphTarget4 / 7.0f * (MTmax - MTmin) + MTmin;
	MTmin = pGI->BlendShapeMin[4];
	MTmax = pGI->BlendShapeMax[4];
	if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
	MorphTarget[4] = rPVC.MorphTarget5 / 7.0f * (MTmax - MTmin) + MTmin;
	MTmin = pGI->BlendShapeMin[5];
	MTmax = pGI->BlendShapeMax[5];
	if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
	MorphTarget[5] = rPVC.MorphTarget6 / 7.0f * (MTmax - MTmin) + MTmin;
	MTmin = pGI->BlendShapeMin[6];
	MTmax = pGI->BlendShapeMax[6];
	if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
	MorphTarget[6] = rPVC.MorphTarget7 / 7.0f * (MTmax - MTmin) + MTmin;
	MTmin = pGI->BlendShapeMin[7];
	MTmax = pGI->BlendShapeMax[7];
	if (!ClientCfg.BlendShapePatched) { MTmin = 0.0f; MTmax = 100.0f; }
	MorphTarget[7] = rPVC.MorphTarget8 / 7.0f * (MTmax - MTmin) + MTmin;
}

// ------------------------------------------------------------------------------------------------
void SCharacter3DSetup::setupFromDataBase (const std::string &branchName)
{
	CCharacterSummary CS;
	setupCharacterSummaryFromDB(CS, branchName);
	setupFromCharacterSummary (CS);
}

// ------------------------------------------------------------------------------------------------
void SCharacter3DSetup::setupFromSERVERDataBase (uint8 nEntity)
{
	CCharacterSummary CS;
	setupCharacterSummaryFromSERVERDB(CS, nEntity);
	setupFromCharacterSummary (CS);
}

// ------------------------------------------------------------------------------------------------
void SCharacter3DSetup::setupDBFromCharacterSummary (const string &branchName, const CCharacterSummary &CS)
{
	setDB (branchName+":PEOPLE",				CS.People);

	setDB (branchName+":VPA:SEX",				CS.VisualPropA.PropertySubData.Sex);
	setDB (branchName+":VPA:JACKETMODEL",		CS.VisualPropA.PropertySubData.JacketModel);
	setDB (branchName+":VPA:JACKETCOLOR",		CS.VisualPropA.PropertySubData.JacketColor);
	setDB (branchName+":VPA:TROUSERMODEL",		CS.VisualPropA.PropertySubData.TrouserModel);
	setDB (branchName+":VPA:TROUSERCOLOR",		CS.VisualPropA.PropertySubData.TrouserColor);
	setDB (branchName+":VPA:WEAPONRIGHTHAND",	CS.VisualPropA.PropertySubData.WeaponRightHand);
	setDB (branchName+":VPA:WEAPONLEFTHAND",	CS.VisualPropA.PropertySubData.WeaponLeftHand);
	setDB (branchName+":VPA:ARMMODEL",			CS.VisualPropA.PropertySubData.ArmModel);
	setDB (branchName+":VPA:ARMCOLOR",			CS.VisualPropA.PropertySubData.ArmColor);
	setDB (branchName+":VPA:HATMODEL",			CS.VisualPropA.PropertySubData.HatModel);
	setDB (branchName+":VPA:HATCOLOR",			CS.VisualPropA.PropertySubData.HatColor);

	setDB (branchName+":VPB:NAME",				CS.VisualPropB.PropertySubData.Name);
	setDB (branchName+":VPB:HANDSMODEL",		CS.VisualPropB.PropertySubData.HandsModel);
	setDB (branchName+":VPB:HANDSCOLOR",		CS.VisualPropB.PropertySubData.HandsColor);
	setDB (branchName+":VPB:FEETMODEL",			CS.VisualPropB.PropertySubData.FeetModel);
	setDB (branchName+":VPB:FEETCOLOR",			CS.VisualPropB.PropertySubData.FeetColor);

	setDB (branchName+":VPC:MORPHTARGET1",		CS.VisualPropC.PropertySubData.MorphTarget1);
	setDB (branchName+":VPC:MORPHTARGET2",		CS.VisualPropC.PropertySubData.MorphTarget2);
	setDB (branchName+":VPC:MORPHTARGET3",		CS.VisualPropC.PropertySubData.MorphTarget3);
	setDB (branchName+":VPC:MORPHTARGET4",		CS.VisualPropC.PropertySubData.MorphTarget4);
	setDB (branchName+":VPC:MORPHTARGET5",		CS.VisualPropC.PropertySubData.MorphTarget5);
	setDB (branchName+":VPC:MORPHTARGET6",		CS.VisualPropC.PropertySubData.MorphTarget6);
	setDB (branchName+":VPC:MORPHTARGET7",		CS.VisualPropC.PropertySubData.MorphTarget7);
	setDB (branchName+":VPC:MORPHTARGET8",		CS.VisualPropC.PropertySubData.MorphTarget8);
	setDB (branchName+":VPC:EYESCOLOR",			CS.VisualPropC.PropertySubData.EyesColor);
	setDB (branchName+":VPC:TATTOO",			CS.VisualPropC.PropertySubData.Tattoo);
	setDB (branchName+":VPC:CHARACTERHEIGHT",	CS.VisualPropC.PropertySubData.CharacterHeight);
	setDB (branchName+":VPC:TORSOWIDTH",		CS.VisualPropC.PropertySubData.TorsoWidth);
	setDB (branchName+":VPC:ARMSWIDTH",			CS.VisualPropC.PropertySubData.ArmsWidth);
	setDB (branchName+":VPC:LEGSWIDTH",			CS.VisualPropC.PropertySubData.LegsWidth);
	setDB (branchName+":VPC:BREASTSIZE",		CS.VisualPropC.PropertySubData.BreastSize);
}

// ------------------------------------------------------------------------------------------------
void SCharacter3DSetup::setupCharacterSummaryFromDB (CCharacterSummary &CS, const string &branchName)
{
	CS.People = (EGSPD::CPeople::TPeople)getDB (branchName+":PEOPLE");

	CS.VisualPropA.PropertySubData.Sex				= getDB (branchName+":VPA:SEX");
	CS.VisualPropA.PropertySubData.JacketModel		= getDB (branchName+":VPA:JACKETMODEL");
	CS.VisualPropA.PropertySubData.JacketColor		= getDB (branchName+":VPA:JACKETCOLOR");
	CS.VisualPropA.PropertySubData.TrouserModel		= getDB (branchName+":VPA:TROUSERMODEL");
	CS.VisualPropA.PropertySubData.TrouserColor		= getDB (branchName+":VPA:TROUSERCOLOR");
	CS.VisualPropA.PropertySubData.WeaponRightHand	= getDB (branchName+":VPA:WEAPONRIGHTHAND");
	CS.VisualPropA.PropertySubData.WeaponLeftHand	= getDB (branchName+":VPA:WEAPONLEFTHAND");
	CS.VisualPropA.PropertySubData.ArmModel			= getDB (branchName+":VPA:ARMMODEL");
	CS.VisualPropA.PropertySubData.ArmColor			= getDB (branchName+":VPA:ARMCOLOR");
	CS.VisualPropA.PropertySubData.HatModel			= getDB (branchName+":VPA:HATMODEL");
	CS.VisualPropA.PropertySubData.HatColor			= getDB (branchName+":VPA:HATCOLOR");

	CS.VisualPropB.PropertySubData.Name				= getDB (branchName+":VPB:NAME");
	CS.VisualPropB.PropertySubData.HandsModel		= getDB (branchName+":VPB:HANDSMODEL");
	CS.VisualPropB.PropertySubData.HandsColor		= getDB (branchName+":VPB:HANDSCOLOR");
	CS.VisualPropB.PropertySubData.FeetModel		= getDB (branchName+":VPB:FEETMODEL");
	CS.VisualPropB.PropertySubData.FeetColor		= getDB (branchName+":VPB:FEETCOLOR");

	CS.VisualPropC.PropertySubData.MorphTarget1		= getDB (branchName+":VPC:MORPHTARGET1");
	CS.VisualPropC.PropertySubData.MorphTarget2		= getDB (branchName+":VPC:MORPHTARGET2");
	CS.VisualPropC.PropertySubData.MorphTarget3		= getDB (branchName+":VPC:MORPHTARGET3");
	CS.VisualPropC.PropertySubData.MorphTarget4		= getDB (branchName+":VPC:MORPHTARGET4");
	CS.VisualPropC.PropertySubData.MorphTarget5		= getDB (branchName+":VPC:MORPHTARGET5");
	CS.VisualPropC.PropertySubData.MorphTarget6		= getDB (branchName+":VPC:MORPHTARGET6");
	CS.VisualPropC.PropertySubData.MorphTarget7		= getDB (branchName+":VPC:MORPHTARGET7");
	CS.VisualPropC.PropertySubData.MorphTarget8		= getDB (branchName+":VPC:MORPHTARGET8");
	CS.VisualPropC.PropertySubData.EyesColor		= getDB (branchName+":VPC:EYESCOLOR");
	CS.VisualPropC.PropertySubData.Tattoo			= getDB (branchName+":VPC:TATTOO");
	CS.VisualPropC.PropertySubData.CharacterHeight	= getDB (branchName+":VPC:CHARACTERHEIGHT");
	CS.VisualPropC.PropertySubData.TorsoWidth		= getDB (branchName+":VPC:TORSOWIDTH");
	CS.VisualPropC.PropertySubData.ArmsWidth		= getDB (branchName+":VPC:ARMSWIDTH");
	CS.VisualPropC.PropertySubData.LegsWidth		= getDB (branchName+":VPC:LEGSWIDTH");
	CS.VisualPropC.PropertySubData.BreastSize		= getDB (branchName+":VPC:BREASTSIZE");
}

// ------------------------------------------------------------------------------------------------

void SCharacter3DSetup::setupCharacterSummaryFromSERVERDB (CCharacterSummary &cs, uint8 entityID)
{


	cs.VisualPropA = getDB ("SERVER:Entities:E"+NLMISC::toString(entityID)+
		":P"+NLMISC::toString(CLFECOMMON::PROPERTY_VPA));

	cs.VisualPropB = getDB ("SERVER:Entities:E"+NLMISC::toString(entityID)+
		":P"+NLMISC::toString(CLFECOMMON::PROPERTY_VPB));

	cs.VisualPropC = getDB ("SERVER:Entities:E"+NLMISC::toString(entityID)+
		":P"+NLMISC::toString(CLFECOMMON::PROPERTY_VPC));

	cs.People = EGSPD::CPeople::Fyros;
	CPlayerCL *pp = NULL;

	if ((pp=dynamic_cast<CPlayerCL*>(EntitiesMngr.entity(entityID))) == NULL)
	{
		pp=(CPlayerCL*)dynamic_cast<CPlayerR2CL*>(EntitiesMngr.entity(entityID));
	}
	if(pp)
	{
		cs.People = pp->people();
		cs.VisualPropA.PropertySubData.Sex = (pp->getGender() == GSGENDER::female);
	}
}

// ------------------------------------------------------------------------------------------------
TChar3DPart SCharacter3DSetup::convert_VisualSlot_To_Char3DPart (SLOTTYPE::EVisualSlot vs)
{
	switch (vs)
	{
		case SLOTTYPE::HIDDEN_SLOT:			return Char3DPart_INVALID;
		case SLOTTYPE::CHEST_SLOT:			return Char3DPart_Chest;
		case SLOTTYPE::LEGS_SLOT:			return Char3DPart_Legs;
		case SLOTTYPE::HEAD_SLOT:			return Char3DPart_Head;
		case SLOTTYPE::ARMS_SLOT:			return Char3DPart_Arms;
		case SLOTTYPE::FACE_SLOT:			return Char3DPart_Face;
		case SLOTTYPE::HANDS_SLOT:			return Char3DPart_Hands;
		case SLOTTYPE::FEET_SLOT:			return Char3DPart_Feet;
		case SLOTTYPE::RIGHT_HAND_SLOT:		return Char3DPart_HandRightItem;
		case SLOTTYPE::LEFT_HAND_SLOT:		return Char3DPart_HandLeftItem;
		case SLOTTYPE::NB_SLOT:				return Char3DPart_INVALID;
		default: break;
	}
	return Char3DPart_INVALID;
}

// ------------------------------------------------------------------------------------------------
SLOTTYPE::EVisualSlot SCharacter3DSetup::convert_Char3DPart_To_VisualSlot (TChar3DPart cp)
{
	switch (cp)
	{
		case Char3DPart_Chest:			return SLOTTYPE::CHEST_SLOT;
		case Char3DPart_Legs:			return SLOTTYPE::LEGS_SLOT;
		case Char3DPart_Head:			return SLOTTYPE::HEAD_SLOT;
		case Char3DPart_Arms:			return SLOTTYPE::ARMS_SLOT;
		case Char3DPart_Face:			return SLOTTYPE::FACE_SLOT;
		case Char3DPart_Hands:			return SLOTTYPE::HANDS_SLOT;
		case Char3DPart_Feet:			return SLOTTYPE::FEET_SLOT;
		case Char3DPart_HandRightItem:	return SLOTTYPE::RIGHT_HAND_SLOT;
		case Char3DPart_HandLeftItem:	return SLOTTYPE::LEFT_HAND_SLOT;
		case Char3DPart_INVALID:		return SLOTTYPE::NB_SLOT;
		default: break;
	}
	return SLOTTYPE::HIDDEN_SLOT;
}

// ------------------------------------------------------------------------------------------------
string SCharacter3DSetup::convert_VisualSlot_To_String (SLOTTYPE::EVisualSlot vs)
{
	switch (vs)
	{
		case SLOTTYPE::HIDDEN_SLOT:			return string("Hidden");
		case SLOTTYPE::CHEST_SLOT:			return string("Chest");
		case SLOTTYPE::LEGS_SLOT:			return string("Legs");
		case SLOTTYPE::HEAD_SLOT:			return string("Head");
		case SLOTTYPE::ARMS_SLOT:			return string("Arms");
		case SLOTTYPE::FACE_SLOT:			return string("Face");
		case SLOTTYPE::HANDS_SLOT:			return string("Hands");
		case SLOTTYPE::FEET_SLOT:			return string("Feet");
		case SLOTTYPE::RIGHT_HAND_SLOT:		return string("Hand Right Item");
		case SLOTTYPE::LEFT_HAND_SLOT:		return string("Hand Left Item");
		case SLOTTYPE::NB_SLOT:				return string("Number Of Slot");
		default: break;
	}
	return string("Invalid");
}

// ------------------------------------------------------------------------------------------------
void SCharacter3DSetup::setupFromCS_ModelCol (SLOTTYPE::EVisualSlot s, sint32 model, sint32 col)
{
	TChar3DPart part = convert_VisualSlot_To_Char3DPart (s);
	if (part == Char3DPart_INVALID) return;

	CItemSheet *item = SheetMngr.getItem (s, model);
	if (item != NULL)
	{
		// magician gloves are a weapon but displayed in hands slot(armor gloves)
		if( (s == SLOTTYPE::RIGHT_HAND_SLOT) && (item->ItemType == ITEM_TYPE::MAGICIAN_STAFF) )
		{
			Parts[part].Name = "none.shape";
			part = convert_VisualSlot_To_Char3DPart (SLOTTYPE::HANDS_SLOT);
		}

		Parts[part].Quality = item->MapVariant;
		if (Male)
			Parts[part].Name = item->getShape();
		else
			Parts[part].Name = item->getShapeFemale();

		// FX
		{
			Parts[part].AdvFx = item->FX.getAdvantageFX();
			Parts[part].StatFxNames.clear();
			Parts[part].StatFxBones.clear();
			Parts[part].StatFxOffss.clear();
			for (uint32 fx = 0; fx < item->FX.getNumStaticFX(); ++fx)
			{
				Parts[part].StatFxNames.push_back(item->FX.getStaticFXName(fx));
				Parts[part].StatFxBones.push_back(item->FX.getStaticFXBone(fx));
				Parts[part].StatFxOffss.push_back(item->FX.getStaticFXOffset(fx));
			}
		}

		if (part == Char3DPart_HandLeftItem)
		{
			if ((item->ItemType == ITEM_TYPE::SHIELD) ||  (item->ItemType == ITEM_TYPE::BUCKLER))
				LeftHandItemIsShield = true;
			else
				LeftHandItemIsShield = false;
		}
	}
	else
	{
		if ((part == Char3DPart_HandLeftItem) || (part == Char3DPart_HandRightItem))
			Parts[part].Name = "none.shape";
	}

	Parts[part].Color = col;
}

// ------------------------------------------------------------------------------------------------
uint64 SCharacter3DSetup::getDB (const string &name)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(name);
	if (pNL == NULL) return 0;
	return pNL->getValue64();
}

// ------------------------------------------------------------------------------------------------
void SCharacter3DSetup::setDB (const string &name, uint64 val)
{
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	CCDBNodeLeaf *pNL = NLGUI::CDBManager::getInstance()->getDbProp(name);
	if (pNL == NULL) return;
	pNL->setValue64(val);
}

// ------------------------------------------------------------------------------------------------
void DEBUG_DumpClothes()
{
	for (uint cp = 0; cp < NB_CHARACTER3D_PARTS; ++cp)
	if (cp != Char3DPart_Face)
	{
		SLOTTYPE::EVisualSlot vs = SCharacter3DSetup::convert_Char3DPart_To_VisualSlot((TChar3DPart)cp);
		string sTmp = SCharacter3DSetup::convert_VisualSlot_To_String(vs);
		nlinfo("*** PART *** : %s", sTmp.c_str());
		uint nNbItems = 0;
		if (cp == Char3DPart_HandRightItem)
			nNbItems = 1<<11;
		if ((cp == Char3DPart_Chest) || (cp == Char3DPart_Hands) || (cp == Char3DPart_Feet))
			nNbItems = 1<<9;
		if ((cp == Char3DPart_Legs) || (cp == Char3DPart_Arms) || (cp == Char3DPart_HandLeftItem))
			nNbItems = 1<<8;
		if (cp == Char3DPart_Head)
			nNbItems = 1<<7;
		for (uint it = 0; it < nNbItems; ++it)
		{
			CItemSheet *item = SheetMngr.getItem (vs, it);
			if (item == NULL)
			{
				//nlinfo("  val:%d UNKNOWN",it);
			}
			else
			{
				//nlinfo("  val:%d M[%s] F[%s]", it, item->Shape.c_str(), item->ShapeFemale.c_str());
				const CSheetManager::TEntitySheetMap &esm = SheetMngr.getSheets();
				CSheetManager::TEntitySheetMap::const_iterator	esmit = esm.begin();
				while (esmit != esm.end())
				{
					if (esmit->second.EntitySheet == item)
					{
						nlinfo("  val:%d item[%s]", it, esmit->first.toString().c_str() );
						break;
					}
					esmit++;
				}

			}
			nlSleep(1);
		}
	}
}

// ------------------------------------------------------------------------------------------------
// CCharacter3D
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------

CCharacter3D::CCharacter3D()
{
	_ClusterSystem = NULL;
	_Scene = NULL;
	_PlayListManager = NULL;
	_AnimationSet = NULL;
	_PlayList = NULL;
	_FacePlayList = NULL;
	// Clear the cache to make it work with 1st init
	_CurrentSetup.Skeleton = "";
	_CurrentSetup.AnimPlayed = -1;
	for (uint32 i = 0; i < NB_CHARACTER3D_PARTS; ++i)
	{
		_CurrentSetup.Parts[i].Name = "";
		_CurrentSetup.Parts[i].Color = -1;
		_CurrentSetup.Parts[i].Quality = -1;
	}
	_CurrentSetup.Tattoo = -1;
	_CurrentSetup.EyesColor = -1;
	_CurrentSetup.CharHeight = _CurrentSetup.ChestWidth = -20.0f;
	_CurrentSetup.ArmsWidth = _CurrentSetup.LegsWidth = _CurrentSetup.BreastSize = -20.0f;
	_PelvisPos.set(0.f,0.f,-20.0f);
	_CurPosX = _CurPosY = _CurPosZ = 0.0f;
	_CurRotX = _CurRotY = _CurRotZ = 0.0f;
	_NextBlinkTime = 0;
	_CopyAnim=false;
}

// ------------------------------------------------------------------------------------------------
CCharacter3D::~CCharacter3D()
{
	if (_Scene == NULL) return;

	// Delete animations first
	if (_PlayListManager != NULL)
	{
		if (_PlayList != NULL)
		{
			_PlayList->resetAllChannels();
			_PlayListManager->deletePlayList(_PlayList);
		}
		if (_FacePlayList != NULL)
		{
			_FacePlayList->resetAllChannels();
			_PlayListManager->deletePlayList(_FacePlayList);
		}
		_Scene->deletePlayListManager(_PlayListManager);
	}
	_AnimationSet= NULL;

	// delete instances
	for (uint32 i = 0; i < NB_CHARACTER3D_PARTS; ++i)
	{
		if (!_Instances[i].empty())
			_Scene->deleteInstance (_Instances[i]);

		for (uint32 fx = 0; fx < _InstancesFx[i].StaticFx.size(); ++fx)
			if (!_InstancesFx[i].StaticFx[fx].empty())
				_Scene->deleteInstance(_InstancesFx[i].StaticFx[fx]);
		_InstancesFx[i].StaticFx.clear();

		if (!_InstancesFx[i].AdvantageFx.empty())
			_Scene->deleteInstance (_InstancesFx[i].AdvantageFx);
	}
	// delete skeleton
	if(!_Skeleton.empty())
		_Scene->deleteSkeleton(_Skeleton);

	_Scene= NULL;
}

// ------------------------------------------------------------------------------------------------
bool CCharacter3D::init (UScene *pScene)
{
//	DEBUG_DumpClothes();

	if (_Scene != NULL) return true;
	_Scene = pScene;

	_PlayListManager = _Scene->createPlayListManager();
	if (!_PlayListManager)
	{
		nlwarning ("CCharacter3D : couldn't create playlist manager");
		return false;
	}


	// ANIMATIONS
	CInterfaceManager *pIM = CInterfaceManager::getInstance();
	COptionsAnimationSet	*pOAS= dynamic_cast<COptionsAnimationSet*>(CWidgetManager::getInstance()->getOptions("character_animations"));
	if(!pOAS || !pOAS->AnimationSet)
	{
		nlwarning("Not found <options> 'character_animations', or not of type 'animation_set'");
		return false;
	}

	if(ClientCfg.Light || !ClientCfg.EAMEnabled)
		_CopyAnim = false;

	// Retrieve the animation info
	if(!_CopyAnim)
	{
		resetAnimation (pOAS->AnimationSet);
		_AnimMale= pOAS->AnimMale;
		_AnimFemale= pOAS->AnimFemale;
	}
	else
	{
		if (EAM)
		{
			resetAnimation (EAM->getAnimationSet());
		}
	}

	return true;
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::resetAnimation (UAnimationSet *animSet)
{
	nlassert(animSet);

	if (_PlayList)
		_PlayList->resetAllChannels();
	if (_FacePlayList)
		_FacePlayList->resetAllChannels();
//	if (_PlayList != NULL) _PlayListManager->deletePlayList (_PlayList);
//	if (_FacePlayList != NULL) _PlayListManager->deletePlayList (_FacePlayList);

	_AnimationSet= animSet;

	if (_PlayList == NULL)
		_PlayList = _PlayListManager->createPlayList(_AnimationSet);
	if (!_PlayList)
	{
		nlwarning ("CCharacter3D : couldn't create play list");
		_Scene->deletePlayListManager (_PlayListManager);
		_PlayListManager = NULL;
		_AnimationSet = NULL;
		return;
	}

	if (_FacePlayList == NULL)
		_FacePlayList = _PlayListManager->createPlayList (_AnimationSet);
	if (!_FacePlayList)
	{
		nlwarning ("CCharacter3D : couldn't create face play list");
		// no face anim, but body anim is still available
		return;
	}
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::disableFaceMorphAndBlinks()
{
	if(_FacePlayList && _AnimationSet)
	{
		// disable eye blink animation (handled by ourselves)
		uint	id= _AnimationSet->getChannelIdByName("visage_100MorphFactor");
		if(id!=UAnimationSet::NotFound)
			_FacePlayList->enableChannel(id, false);
		// disable morph target (handled by ourselves)
		for(uint i=0;i<NB_MORPH_TARGETS;i++)
		{
			static const string baseName = "visage_00";
			id= _AnimationSet->getChannelIdByName(baseName + toString(i) + "MorphFactor");
			if(id!=UAnimationSet::NotFound)
				_FacePlayList->enableChannel(id, false);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::setup (const SCharacter3DSetup &c3ds)
{
	bool bSkeletonRebuilt = false;
	// Test with cache and call dressing/loading functions
	if (!c3ds.Skeleton.empty())
	if (c3ds.Skeleton != _CurrentSetup.Skeleton)
	{
		setSkeleton (c3ds.Skeleton);
		_CurrentSetup.Skeleton = c3ds.Skeleton;
		bSkeletonRebuilt = true;
		_Skeleton.setClusterSystem (_ClusterSystem);
		if (_PlayList)
		{
			_PlayList->registerTransform (_Skeleton);
			// disable pos animation
			uint	id= _AnimationSet->getChannelIdByName("pos");
			if(id!=UAnimationSet::NotFound)
				_PlayList->enableChannel(id, false);
		}
	}

	// Information that are additionnal
	_CurrentSetup.LeftHandItemIsShield = c3ds.LeftHandItemIsShield;
	_CurrentSetup.Male = c3ds.Male;

	// Setup instances
	uint32 i;

	for (i = 0; i < NB_CHARACTER3D_PARTS; ++i)
	{
		bool bInstanceRebuilt = false;
		bool bQualityRebuilt = false;

		// Create Instance
		if ((c3ds.Parts[i].Name != _CurrentSetup.Parts[i].Name) || (c3ds.Parts[i].AdvFx != _CurrentSetup.Parts[i].AdvFx))
		{
			// If face, unregister FacePlayList
			if(i==Char3DPart_Face && _FacePlayList)
				_FacePlayList->resetAllChannels();

			// rebuild this instance
			bInstanceRebuilt = true;
			createInstance ((TChar3DPart)i, c3ds.Parts[i]);
			_CurrentSetup.Parts[i].Name = c3ds.Parts[i].Name;
			_CurrentSetup.Parts[i].AdvFx = c3ds.Parts[i].AdvFx;
			_CurrentSetup.Parts[i].StatFxNames = c3ds.Parts[i].StatFxNames;
			_CurrentSetup.Parts[i].StatFxBones = c3ds.Parts[i].StatFxBones;
			_CurrentSetup.Parts[i].StatFxOffss = c3ds.Parts[i].StatFxOffss;

			// If face and instance created, reassign FacePlayList
			if(i==Char3DPart_Face && _FacePlayList && !_Instances[Char3DPart_Face].empty())
			{
				_FacePlayList->registerTransform(_Instances[Char3DPart_Face]);
				disableFaceMorphAndBlinks();
			}
		}

		// Quality
		if (c3ds.Parts[i].Quality != -1)
		if ((c3ds.Parts[i].Quality != _CurrentSetup.Parts[i].Quality) || bInstanceRebuilt)
		{
			if (!_Instances[i].empty())
			{
				_Instances[i].selectTextureSet((uint)c3ds.Parts[i].Quality);
				bQualityRebuilt = true;
			}
			_CurrentSetup.Parts[i].Quality = c3ds.Parts[i].Quality;
		}

		// Instance user color
		if (c3ds.Parts[i].Color != -1)
		if ((c3ds.Parts[i].Color != _CurrentSetup.Parts[i].Color) || bInstanceRebuilt || bQualityRebuilt)
		{
			if (!_Instances[i].empty())
			{
				ColorSlotManager.setInstanceSlot (	_Instances[i],
													1u, // Slot 1 is for user color
													c3ds.Parts[i].Color);
			}
			_CurrentSetup.Parts[i].Color = c3ds.Parts[i].Color;
		}

		// Instance skin color
		if (c3ds.People != EGSPD::CPeople::Undefined)
		if ((c3ds.People != _CurrentSetup.People) || bInstanceRebuilt || bQualityRebuilt)
		{
			if (!_Instances[i].empty())
			{
				ColorSlotManager.setInstanceSlot (	_Instances[i],
													0u, // Slot 0 is for skin
													peopleToSkin(c3ds.People));
			}
			// Here we do not update current setup people value to let other instances colorize too
		}

		// Special cases
		switch(i)
		{
			case Char3DPart_Face:
				// Setup tatoo
				if (c3ds.Tattoo != -1)
				if ((c3ds.Tattoo != _CurrentSetup.Tattoo) || bInstanceRebuilt)
				{
					if (!_Instances[Char3DPart_Face].empty())
						makeUp (_Instances[Char3DPart_Face], c3ds.Tattoo);
					_CurrentSetup.Tattoo = c3ds.Tattoo;
				}
				// Setup eyes color
				if (c3ds.EyesColor != -1)
				if ((c3ds.EyesColor != _CurrentSetup.EyesColor) ||bInstanceRebuilt)
				{
					if (!_Instances[Char3DPart_Face].empty())
						ColorSlotManager.setInstanceSlot (	_Instances[Char3DPart_Face],
															(uint)3, // slot 3 is for eyes colors
															c3ds.EyesColor );
					_CurrentSetup.EyesColor = c3ds.EyesColor;
				}
				// Setup morph targets
				if ((c3ds.MorphTarget[0] != _CurrentSetup.MorphTarget[0]) ||
					(c3ds.MorphTarget[1] != _CurrentSetup.MorphTarget[1]) ||
					(c3ds.MorphTarget[2] != _CurrentSetup.MorphTarget[2]) ||
					(c3ds.MorphTarget[3] != _CurrentSetup.MorphTarget[3]) ||
					(c3ds.MorphTarget[4] != _CurrentSetup.MorphTarget[4]) ||
					(c3ds.MorphTarget[5] != _CurrentSetup.MorphTarget[5]) ||
					(c3ds.MorphTarget[6] != _CurrentSetup.MorphTarget[6]) ||
					(c3ds.MorphTarget[7] != _CurrentSetup.MorphTarget[7]) ||
					bInstanceRebuilt || bSkeletonRebuilt)
				{
					if (!_Instances[Char3DPart_Face].empty())
					{
						for(uint k = 0; k < NB_MORPH_TARGETS; ++k)
						{
							static const char *baseName = "visage_00";
							_Instances[Char3DPart_Face].setBlendShapeFactor (baseName + toString(k),
														c3ds.MorphTarget[k], true);
							_CurrentSetup.MorphTarget[k] = c3ds.MorphTarget[k];
						}
					}
				}

				if (!_Instances[Char3DPart_Face].empty())
				{
					if (c3ds.HideFace)
						_Instances[Char3DPart_Face].hide();
					else
						_Instances[Char3DPart_Face].show();
				}

				_CurrentSetup.HideFace = c3ds.HideFace;

				// Setup hair color (for both part)
				if (c3ds.HairColor != -1)
					if ((c3ds.HairColor!= _CurrentSetup.HairColor) || bInstanceRebuilt)
					{
						if (!_Instances[Char3DPart_Face].empty())
							ColorSlotManager.setInstanceSlot (	_Instances[Char3DPart_Face],
																(uint)2, // slot 2 is for hair color
																c3ds.HairColor );
						//_CurrentSetup.HairColor = c3ds.HairColor;
					}
			break;
			case Char3DPart_Head:
				// Setup hair color
				if (c3ds.HairColor != -1)
				if ((c3ds.HairColor!= _CurrentSetup.HairColor) || bInstanceRebuilt)
				{
					if (!_Instances[Char3DPart_Head].empty())
						ColorSlotManager.setInstanceSlot (	_Instances[Char3DPart_Head],
															(uint)2, // slot 2 is for hair color
															c3ds.HairColor );
					_CurrentSetup.HairColor = c3ds.HairColor;
				}
			break;
			default:
			break;
		}

		// Bind instance to skeleton
		if (bInstanceRebuilt || bSkeletonRebuilt)
		{
			bindToSkeleton ((TChar3DPart)i);
		}
	}
	_CurrentSetup.People = c3ds.People; // Because not done for each instance

	// Setup gabarit
	bool bGabaritChanged = false;
	if ((c3ds.CharHeight != _CurrentSetup.CharHeight) ||
		(c3ds.ChestWidth != _CurrentSetup.ChestWidth) ||
		(c3ds.ArmsWidth != _CurrentSetup.ArmsWidth) ||
		(c3ds.LegsWidth != _CurrentSetup.LegsWidth) ||
		(c3ds.BreastSize != _CurrentSetup.BreastSize) ||

		bSkeletonRebuilt)
	{
		uint gender = _CurrentSetup.Male ? 0 : 1;
		float heightScale;
		GabaritSet.applyGabarit ( _Skeleton, gender, _CurrentSetup.People,
			                c3ds.CharHeight, c3ds.ChestWidth, c3ds.ArmsWidth, c3ds.LegsWidth, c3ds.BreastSize,
							&heightScale	);
		float refHeightScale = GabaritSet.getRefHeightScale(gender, _CurrentSetup.People);
		// dummy code, to avoid 1 frame big swap
		_PelvisPos.z = 1.f * heightScale;

		_CurrentSetup.CharHeight = c3ds.CharHeight;
		_CurrentSetup.ChestWidth = c3ds.ChestWidth;
		_CurrentSetup.ArmsWidth = c3ds.ArmsWidth;
		_CurrentSetup.LegsWidth = c3ds.LegsWidth;
		_CurrentSetup.BreastSize = c3ds.BreastSize;

		if(refHeightScale != 0.f)
			_CustomScalePos = heightScale/refHeightScale;
		else
			_CustomScalePos = 1.f;

		bGabaritChanged = true;
	}

	// Play an animation

	if (c3ds.AnimPlayed != -1)
	if ((c3ds.AnimPlayed != _CurrentSetup.AnimPlayed) || (bSkeletonRebuilt) || _CopyAnim)
		setAnim (c3ds.AnimPlayed);

	if (bSkeletonRebuilt || bGabaritChanged)
		animate(0.0);

	// If skeleton or gabarit has changed replace correctly the skeleton from feet reference point
	setPos (_CurPosX, _CurPosY, _CurPosZ);
	setRotEuler (_CurRotX, _CurRotY, _CurRotZ);

	// update skeleton pelvis pos
	if (!_Skeleton.empty())
		_Skeleton.setPos(_PelvisPos);
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::setAnim (uint animID)
{
	CCharacterCL * character = NULL;

	if(!_CopyAnim)
	{
		_CurrentSetup.AnimPlayed = animID;
		if (_CurrentSetup.Male)
		{
			if (animID >= _AnimMale.size()) return;
			animID = _AnimMale[animID].AnimId;
		}
		else
		{
			if (animID >= _AnimFemale.size()) return;
			animID = _AnimFemale[animID].AnimId;
		}
	}
	else
	{
		CInstance * selectedInst = getEditor().getSelectedInstance();
		if(!selectedInst) return;
		CEntityCL * entity = selectedInst->getEntity();
		if(!(entity && ((character=dynamic_cast<CCharacterCL*>(entity))!=NULL)))
			return;

		animID = character->playList()->getAnimation(MOVE);
		_CurrentSetup.AnimPlayed = animID;
	}


	float animSpeedFactor = 0.9f + 0.2f * NLMISC::frand(1);
	if (_PlayList)
	{
		if(_CopyAnim)
		{
			_PlayList->setTimeOrigin(MOVE, character->playList()->getTimeOrigin(MOVE));
			if(character->playList()->getAnimation(MOVE)!=_PlayList->getAnimation(MOVE))
			{
				_PlayList->setAnimation(MOVE, animID);
				_PlayList->setSpeedFactor(MOVE, character->playList()->getSpeedFactor(MOVE));
				_PlayList->setWrapMode(MOVE, character->playList()->getWrapMode(MOVE));
			}
		}
		else
		{
			_PlayList->setAnimation(MOVE, animID);
			_PlayList->setSpeedFactor(MOVE, animSpeedFactor);
			_PlayList->setTimeOrigin(MOVE, TimeInSec);
			_PlayList->setWrapMode(MOVE, UPlayList::Repeat);
		}
	}

	if (_FacePlayList)
	{
		uint	faceAnimId=UPlayList::empty;
		if(_AnimationSet && animID<_AnimationSet->getNumAnimation())
		{
			// build the anim name of the face
			string	faceAnimName= COptionsAnimationSet::getFaceAnimName(_AnimationSet->getAnimationName(animID));
			// find the face anim for this name
			faceAnimId= _AnimationSet->getAnimationIdByName(faceAnimName);
			if(faceAnimId==UAnimationSet::NotFound)
				faceAnimId= UPlayList::empty;
		}

		_FacePlayList->setAnimation(MOVE, faceAnimId);
		if(faceAnimId!=UPlayList::empty)
		{
			if(_CopyAnim)
			{
				_FacePlayList->setTimeOrigin(MOVE, character->facePlayList()->getTimeOrigin(MOVE));
				_FacePlayList->setSpeedFactor(MOVE, character->facePlayList()->getSpeedFactor(MOVE));
				_FacePlayList->setWrapMode(MOVE, character->facePlayList()->getWrapMode(MOVE));
			}
			else
			{
				_FacePlayList->setSpeedFactor(MOVE, animSpeedFactor);
				_FacePlayList->setTimeOrigin(MOVE, TimeInSec);
				_FacePlayList->setWrapMode(MOVE, UPlayList::Repeat);
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::animate (double globalTime)
{
	if (!_AnimationSet) return;
	_PlayListManager->animate (globalTime);

	animblink (globalTime);

	if (_CurrentSetup.AnimPlayed == -1) return;
	// take correct 3D animId
	uint	animID= _CurrentSetup.AnimPlayed;
	bool	applyRaceScalePos= true;

	if(!_CopyAnim)
	{
		if (_CurrentSetup.Male && animID < _AnimMale.size())
		{
			applyRaceScalePos = _AnimMale[animID].ApplyRaceScalePos;
			// animId is now the correct 3D animId
			animID = _AnimMale[animID].AnimId;
		}
		else if (!_CurrentSetup.Male && animID < _AnimFemale.size())
		{
			applyRaceScalePos = _AnimFemale[animID].ApplyRaceScalePos;
			// animId is now the correct 3D animId
			animID = _AnimFemale[animID].AnimId;
		}
		else
			return;
	}

	// get the animation
	if(animID==UAnimationSet::NotFound)
		return;
	UAnimation *pAnim = _AnimationSet->getAnimation (animID);
	if (pAnim == NULL) return;
	UTrack *pTrack = pAnim->getTrackByName("pos");
	CVector animPos;
	if (pTrack == NULL) return;

	// Compute animation time (wrapped)
	double wrappedTime=(globalTime-_PlayList->getTimeOrigin(0))*_PlayList->getSpeedFactor(0);
	// Mod repeat the time
	{
		float length=pAnim->getEndTime ()-pAnim->getBeginTime();
		if (wrappedTime>=0)
			wrappedTime=pAnim->getBeginTime()+(float)fmod ((float)wrappedTime, length);
		else
			wrappedTime=pAnim->getBeginTime()+(float)fmod ((float)wrappedTime, length)+length;
	}
	pTrack->interpolate((float)wrappedTime, animPos);

	// apply race scale pos only if animation need it
	if(applyRaceScalePos)
		animPos *= getGenderInfo(_CurrentSetup.People, _CurrentSetup.Male)->CharacterScalePos;
	// always apply custom scale pos
	animPos *= _CustomScalePos;
	_PelvisPos = animPos;
	// update skeleton pelvis pos
	if (!_Skeleton.empty())
	{
		_Skeleton.setPos(_PelvisPos);
		// update skeleton spawn script pos
		_Skeleton.setSSSWOPos(_Root.getMatrix().getPos());
		_Skeleton.setSSSWODir(_Root.getMatrix().getJ());
	}
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::setPos (float x, float y, float z)
{
	_CurPosX = x;
	_CurPosY = y;
	_CurPosZ = z;
	if (!_Root.empty())
		_Root.setPos (x, y, z);
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::setClusterSystem (NL3D::UInstanceGroup *pIG)
{
	_ClusterSystem = pIG;
	if (!_Skeleton.empty())
		_Skeleton.setClusterSystem(pIG);
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::setRotEuler (float rx, float ry, float rz)
{
	_CurRotX = rx;
	_CurRotY = ry;
	_CurRotZ = rz;
	if (!_Root.empty())
	{
		_Root.setTransformMode (UTransformable::RotEuler);
		_Root.setRotEuler (_CurRotX, _CurRotY, _CurRotZ);
	}
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::getHeadPos (float &x, float &y, float &z)
{
	x = y = z = 0;
	if (!_Skeleton.empty())
	{
		sint boneId = _Skeleton.getBoneIdByName("Bip01 Head");
		if (boneId == -1)
		{
			nlwarning ("bad bone name");
			return;
		}
		_Skeleton.forceComputeBone(boneId);
		UBone rBone = _Skeleton.getBone(boneId);
		const CMatrix &rM = rBone.getLastWorldMatrixComputed();
		CVector v;
		rM.getPos(v);
		x = v.x;
		y = v.y;
		z = v.z;
	}
	else
	{
		nlwarning ("no skeleton");
	}
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::setSkeleton (const string &filename)
{
	// Remove the old skeleton.
	if (!_Skeleton.empty())
	{
		// Must remove first any channels of _Skeleton registered into _PlayList
		if (_PlayList)
			_PlayList->resetAllChannels();
		_Scene->deleteSkeleton(_Skeleton);
		_Skeleton = NULL;
		_Root = NULL;
	}
	if (!_Root.empty())
		_Scene->deleteTransform(_Root);
	_Root = _Scene->createTransform();
	// Create the skeleton.
	_Skeleton = _Scene->createSkeleton(filename);
	if (_Skeleton.empty())
	{
		nlwarning ("CCharacter3D::setSkeleton : Skeleton %s can't be created.", filename.c_str());
		return;
	}

	_Skeleton.setPos (_PelvisPos);
	_Skeleton.changeMRMDistanceSetup (100.0f, 150.0f, 200.0f);
	_Skeleton.parent(_Root);
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::createInstance (TChar3DPart i, const SCharacter3DSetup::SCharacterPart &part)
{
	if (_Scene == NULL)
	{
		nlwarning ("CCharacter3D::createInstance : no scene setup.");
		return;
	}

	if (!_Instances[i].empty())
		_Scene->deleteInstance (_Instances[i]);

	if ((!part.Name.empty()) && (part.Name != "none.shape"))
		_Instances[i] = _Scene->createInstance (part.Name);

	// if cannot create output some errors
	if (_Instances[i].empty())
	{
		if ((i != Char3DPart_HandRightItem) && (i != Char3DPart_HandLeftItem))
			nlwarning ("CCharacter3D::createInstance : cannot create the instance : %s.", part.Name.c_str());
		return;
	}

	// FX Management

	// Advantage Fx
	if (!_InstancesFx[i].AdvantageFx.empty())
		_Scene->deleteInstance (_InstancesFx[i].AdvantageFx);

	if ((!part.AdvFx.empty()) && (part.AdvFx != "none.shape"))
	{
		_InstancesFx[i].AdvantageFx = _Scene->createInstance (part.AdvFx);
		if (_InstancesFx[i].AdvantageFx.empty())
		{
			nlwarning ("CCharacter3D::createInstance : cannot create the fx : %s.", part.AdvFx.c_str());
		}
		else
		{
			CMatrix mat = _Instances[i].getMatrix();
			mat.invert();
			mat *= _InstancesFx[i].AdvantageFx.getMatrix();
			_InstancesFx[i].AdvantageFx.setTransformMode(UTransformable::DirectMatrix);
			_InstancesFx[i].AdvantageFx.setMatrix(mat);
			_InstancesFx[i].AdvantageFx.parent(_Instances[i]);
		}
	}

	// Static Fx
	uint32 fx;
	for (fx = 0; fx < _InstancesFx[i].StaticFx.size(); ++fx)
		if (!_InstancesFx[i].StaticFx[fx].empty())
			_Scene->deleteInstance(_InstancesFx[i].StaticFx[fx]);
	_InstancesFx[i].StaticFx.clear();

	for (fx = 0; fx < part.StatFxNames.size(); ++fx)
		if ((!part.StatFxNames[fx].empty()) && (part.StatFxNames[fx] != "none.shape") &&
			(!part.StatFxBones[fx].empty()) && (part.StatFxBones[fx] != "none.shape"))
		{
			sint boneID = _Skeleton.getBoneIdByName(part.StatFxBones[fx]);
			if (boneID != -1)
			{
				UInstance instance = _Scene->createInstance(part.StatFxNames[fx]);
				if (!instance.empty())
				{
					instance.setTransformMode(UTransform::DirectMatrix);
					CMatrix mat;
					mat.setPos(part.StatFxOffss[fx]);
					instance.setMatrix(mat);
					_Skeleton.stickObject(instance, boneID);
					_InstancesFx[i].StaticFx.push_back(instance);
				}
				else
				{
					nlwarning("Can't create static fx %s sticked on bone %s", part.StatFxNames[fx].c_str(), part.StatFxBones[fx].c_str());
				}
			}
			else
			{
				nlwarning("Can't find bone %s for static fx %s", part.StatFxBones[fx].c_str(), part.StatFxNames[fx].c_str());
			}
		}
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::bindToSkeleton (TChar3DPart i)
{
	if (_Skeleton.empty())
	{
		nlwarning ("CCharacter3D::bindToSkeleton : no skeleton setup");
		return;
	}

	if (_Instances[i].empty())
	{
		if ((i != Char3DPart_HandRightItem) && (i != Char3DPart_HandLeftItem))
			nlinfo ("CCharacter3D::bindToSkeleton : no character part for %d", i);
		return;
	}

	switch (i)
	{
		case Char3DPart_HandRightItem:
		{
			sint rightHandBoneID = _Skeleton.getBoneIdByName ("box_arme");
			if (rightHandBoneID != -1)
				_Skeleton.stickObject (_Instances[i], rightHandBoneID);
		}
		break;

		case Char3DPart_HandLeftItem:
		{
			sint leftHandBoneID;
			// If this is a shield.
			if (_CurrentSetup.LeftHandItemIsShield)
				leftHandBoneID = _Skeleton.getBoneIdByName ("Box_bouclier");
			else
				leftHandBoneID = _Skeleton.getBoneIdByName ("box_arme_gauche");
			if (leftHandBoneID != -1)
				_Skeleton.stickObject (_Instances[i], leftHandBoneID);
		}
		break;

		default:
			if (!_Skeleton.bindSkin(_Instances[i]))
			{
				nlwarning ("CCharacter3D::bindToSkeleton: Cannot bind the instance : %d.", i);
				return;
			}
		break;
	}
}

// ------------------------------------------------------------------------------------------------
uint32 CCharacter3D::peopleToSkin (EGSPD::CPeople::TPeople people) const
{
	switch (people)
	{
		case EGSPD::CPeople::Matis:
			return 1;

		case EGSPD::CPeople::Tryker:
			return 2;

		case EGSPD::CPeople::Zorai:
			return 3;

		case EGSPD::CPeople::Fyros:
		default:
			return 0;
	}
}

// ------------------------------------------------------------------------------------------------
void CCharacter3D::animblink (double globalTime)
{
	float blend;

	// Some parameters
	static const double blinkTime = 0.1f;
	static const double minBlinkLength = 0.5f;
	static const double maxBlinkLength = 5.0f;

	// Next blink time is valid ?
	bool validTime = (_NextBlinkTime + blinkTime >= globalTime) && (_NextBlinkTime <= (globalTime + maxBlinkLength));

	// Blink end ?
	bool blinkEnd = (globalTime >= _NextBlinkTime + blinkTime);

	// Blink is finished or next blink time is invalid ?
	if ( blinkEnd || !validTime )
	{
		blend = 0;

		// Compute next time
		_NextBlinkTime = (((double)rand () / (double)RAND_MAX) * (maxBlinkLength - minBlinkLength) + minBlinkLength + (double)globalTime);
	}
	else
	{
		// Blink time ?
		if (globalTime >= _NextBlinkTime)
		{
			blend = 100.f;
		}
		else
		{
			// Do nothing
			return;
		}
	}

	// Set the blend shape
	if(!_Instances[Char3DPart_Face].empty())
		_Instances[Char3DPart_Face].setBlendShapeFactor ("visage_100", blend, true);
}

// ------------------------------------------------------------------------------------------------
CVector CCharacter3D::getBonePos (const string &boneName)
{
	CVector ret=CVector(0,0,0);
	sint boneId = _Skeleton.getBoneIdByName(boneName);
	if (boneId == -1) return ret;
	_Skeleton.forceComputeBone(boneId);
	UBone rBone = _Skeleton.getBone(boneId);
	const CMatrix &rM = rBone.getLastWorldMatrixComputed();
	rM.getPos(ret);
	return ret;
}

// ------------------------------------------------------------------------------------------------



