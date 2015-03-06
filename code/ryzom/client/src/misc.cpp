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
// Client
#include "misc.h"
#include "entity_animation_manager.h"
#include "entities.h"
#include "sheet_manager.h"
#include "interface_v3/interface_manager.h"
#include "continent_manager.h"
#include "world_database_manager.h"
#include "client_cfg.h"
#include "user_entity.h"
#include "net_manager.h"
#include "sky_render.h"
// Client Sheets
#include "client_sheets/item_sheet.h"
// 3D
#include "nel/3d/u_instance.h"
#include "nel/3d/u_instance_material.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/material.h"
// Misc
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/fast_floor.h"
#include "nel/misc/noise_value.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/system_utils.h"
// Game Share
#include "game_share/player_visual_properties.h"
#include "game_share/seeds.h"
// Georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"

///////////
// USING //
///////////
using namespace NL3D;
using namespace NLMISC;
using namespace std;
using namespace NLGEORGES;

////////////
// EXTERN //
////////////
extern CEntityAnimationManager	*EAM;
extern UDriver					*Driver;
extern UMaterial				GenericMat;


////////////
// GLOBAL //
////////////
const uint RecordVersion = 1;
std::set<std::string> LodCharactersNotFound;


//////////////
// FUNCTION //
//////////////
//-----------------------------------------------
// keepIn_NegPi_Pi :
//-----------------------------------------------
double keepIn_NegPi_Pi(double angle)
{
	if(angle > Pi)
		angle = -2*Pi+angle;
	else if(angle < -Pi)
		angle = 2*Pi+angle;

	return angle;
}// keepIn_NegPi_Pi //

//-----------------------------------------------
// angleBetween2Vect :
//-----------------------------------------------
double angleBetween2Vect(const CVectorD &from, const CVectorD &to)
{
	// Get the final local head Yaw.
	CVector vj = from;
	vj.z = 0;
	CVector vk(0,0,1);
	CVector vi = vj^vk;

	CMatrix mat;
	mat.setRot(vi,vj,vk,true);

	CVector localDir = mat.inverted() * to;
	return atan2(-localDir.x, localDir.y);
}// angleBetween2Vect //

//-----------------------------------------------
// makeUp :
// \param face : pointer on the face to make up (must not be null).
// \param idMakeUp : index of the make-up to apply.
// \warning This function does not check if 'face'  is valid.
//-----------------------------------------------
void makeUp(NL3D::UInstance face, sint idMakeUp)
{
	static const char *tattooStr = "visage_makeup";

	// look for tattoo texture
	uint numMat = face.getNumMaterials();
	std::string texFilename;
	for(uint k = 0; k < numMat; ++k)
	{
		UInstanceMaterial im = face.getMaterial(k);
		sint numTex = im.getLastTextureStage();
		for(sint l = 0; l <= numTex; ++l)
		{
			if (im.isTextureFile(l)) // one texture from a file ?
			{
				// see if it is the texture used for tattoos
				texFilename = im.getTextureFileName(l);
				// nlinfo("visage tex = %s", texFilename.c_str());
				std::string::size_type pos = texFilename.find(tattooStr, 0);
				if (pos != std::string::npos)
				{
					uint charIndex = (uint)(pos + strlen(tattooStr));
					if (texFilename.length() >= charIndex + 2)
					{
						texFilename[charIndex] = '0' + (unsigned char) (idMakeUp / 10);
						texFilename[charIndex + 1] = '0' + (unsigned char) (idMakeUp % 10);
						im.setTextureFileName(texFilename, l);
					}
				}
			}
		}
	}
}// makeUp //

//-----------------------------------------------
// qStart is the quaterion at t=0,
// qEnd is the quaterion at t=TAnimEnd,
// time is between 0 and 1. (1=> TAnimEnd)
//-----------------------------------------------
CQuat applyRotationFactor(CQuat in, float rotFactor, CQuat qStart, CQuat qEnd, float time)
{
	H_AUTO ( RZ_Client_Apply_Rotation_Factor )

	CQuat	qRotTotal1, qRotTotal2;
	qStart.invert();
	qRotTotal1= qEnd*qStart;
//	qRotTotal2.makeClosest(CQuat::Identity);
	qRotTotal2= CQuat::slerp(CQuat::Identity, qRotTotal1, rotFactor);

	// apply animation factor
//	qRotTotal1.makeClosest(CQuat::Identity);
//	qRotTotal2.makeClosest(CQuat::Identity);
	CQuat	qRotDelta1= CQuat::slerp(CQuat::Identity, qRotTotal1, time);
	CQuat	qRotDelta2= CQuat::slerp(CQuat::Identity, qRotTotal2, time);

	// remove normal rotation, and apply rotFactor-ed one.
	qRotDelta1.invert();
	return qRotDelta2 * qRotDelta1 * in;
}// applyRotationFactor //


//-----------------------------------------------
// computeShortestAngle :
// Compute the angle between a source and a destination using the shortest way.
// \param from : angle between -Pi and Pi;
// \param to : angle between -Pi and Pi;
//-----------------------------------------------
double computeShortestAngle(double from, double to)
{
	double difAngle = to - from;
	if(difAngle < -Pi)		// Angle in the wrong direction.
		difAngle += 2.0*Pi;
	else if(difAngle > Pi)	// Angle in the wrong direction.
		difAngle -= 2.0*Pi;

	return difAngle;
}// computeShortestAngle //

//-----------------------------------------------
// readStringArray :
//-----------------------------------------------
void readStringArray(const std::string &filename, NLGEORGES::UFormLoader *formLoader, NLMISC::CSmartPtr<NLGEORGES::UForm> &form, std::map<std::string, std::string> &container)
{
	// Read an array of strings
	if(formLoader)
	{
		form = formLoader->loadForm(filename.c_str());
		if(form)
		{
			// Get the root.
			const UFormElm& rootElmt = form->getRootNode();
			// Get animations.
			const UFormElm *elmt = 0;
			if(rootElmt.getNodeByName(&elmt, "array") == false)
				nlwarning("readStringArray: the node 'array' is not referenced.");
			// If the array is not empty (in fact exist).
			if(elmt)
			{
				// Get the array size
				uint arraySize;
				elmt->getArraySize(arraySize);
				// If there is at least 1 animation.
				if(arraySize > 0)
				{
					// Get all animation for the State.
					for(uint i = 0; i<arraySize; ++i)
					{
						// Get the Name
						std::string nodeName;
						std::string stringName;
						if(elmt->getArrayNodeName(nodeName, i))
						{
							if(elmt->getArrayValue(stringName, i))
								container.insert(make_pair(nodeName, stringName));
							else
								nlwarning("readStringArray: no string associated to the node '%d(%s)'.", i, nodeName.c_str());
						}
						else
							nlwarning("readStringArray: node '%d', index valid.", i);
					}
				}
			}
			else
				nlwarning("readStringArray: array node not allocated.");
		}
		else
			nlwarning("readStringArray: cannot create the form from file '%s'.", filename.c_str());
	}
	else
		nlwarning("readStringArray: the Loader is not allocated.");
}// readStringArray //

//-----------------------------------------------
// mode2Anim :
// Return the animset base name corresponding to the mode.
// \param mode : the mode to convert.
// \param result : this will be filed with the mode animset name.
// \return bool : 'true' if 'result' is filled, 'false' if left untouched.
//-----------------------------------------------
bool mode2Anim(MBEHAV::EMode mode, string &result)
{
	static bool init = false;
	static string mode2AnimArray[MBEHAV::NUMBER_OF_MODES];

	// Is the mode valid
	uint index = (uint)mode;
	if(index >= MBEHAV::NUMBER_OF_MODES)
		return false;

	// Initialize
	if(!init)
	{
		// Read mode2animset file
		std::map<std::string, std::string> mode2Animset;
		const std::string filename = "mode2animset.string_array";
		NLGEORGES::UFormLoader *formLoader = UFormLoader::createLoader();
		if(formLoader)
		{
			NLMISC::CSmartPtr<NLGEORGES::UForm> form;
			readStringArray(filename, formLoader, form, mode2Animset);
		}
		else
			nlwarning("mode2Anim: cannot create de loader");
		NLGEORGES::UFormLoader::releaseLoader(formLoader);
		// Initialize the static vector.
		//-------------------------------
		for(uint i=0; i<MBEHAV::NUMBER_OF_MODES; ++i)
		{
			const std::string modeName = MBEHAV::modeToString((MBEHAV::EMode)i);
			std::map<std::string, std::string>::const_iterator it = mode2Animset.find(modeName);
			if(it != mode2Animset.end())
			{
				mode2AnimArray[i] = (*it).second;
				if(mode2AnimArray[i].empty())
					nlwarning("mode2Anim: The mode '%d(%s)' has an empty animset associated.", i, modeName.c_str ());
			}
			// No animset for the mode.
			else
			{
				mode2AnimArray[i] = "";
				nlwarning("mode2Anim: no animset associated to the mode %d'%s'.", i, modeName.c_str());
			}
		}
		// Init Done now.
		init = true;
	}

	// Fill the result.
	// Check name.
	if(mode2AnimArray[index].empty())
		result = mode2AnimArray[MBEHAV::NORMAL];
	else
		result = mode2AnimArray[index];
	// Result filled.
	return true;
}// mode2Anim //

//-----------------------------------------------
// computeAnimSet :
// Compute the animation set to use according to weapons, mode and race.
// \param animSet : result pointer.
// \param mode : the mode.
// \param animSetBaseName : basic name to construc the complet name of the animSet.
// \param leftHand : animSet name for the left hand.
// \param rightHand : animSet name for the right hand.
// \param lookAtItemsInHands : compute animset according to items in hands or not.
// \return bool : 'true' if the new animation set is the right one. 'false' if the one choosen is not the right one.
//-----------------------------------------------
bool computeAnimSet(const CAnimationSet *&animSet, MBEHAV::EMode mode, const string &animSetBaseName, const CItemSheet *itemLeftHand, const CItemSheet *itemRightHand, bool lookAtItemsInHands)
{
	static std::set<std::string> UnknownAnimSet;

	if(EAM == 0)
	{
		if(!ClientCfg.Light)
		{
			nlwarning("computeAnimSet: EAM not allocated -> cannot compute the anim set.");
		}
		return false;
	}

	string animSetRight, animSetLeft;
	if(itemLeftHand)
		animSetLeft = itemLeftHand->getAnimSet();

	if(itemRightHand)
		animSetRight = itemRightHand->getAnimSet();

	// Get the animset name from the mode.
	string result;
	if(!mode2Anim(mode, result))
	{
		nlwarning("computeAnimSet: unknown mode '%d'.", mode);
		result = "default";
	}

	// Compute the name.
	if(lookAtItemsInHands)
		result = animSetBaseName + "_" + result + "_" + animSetRight + "_" + animSetLeft;
	else
		result = animSetBaseName + "_" + result + "__";

	// Get the animset.
	const CAnimationSet *animSetTmp = EAM->getAnimSet(result);
	if(animSetTmp)
	{
		animSet = animSetTmp;
		return true;
	}
	// Bad one try something else.
	else
	{
		// Up to 100 missing anim set (security).
		if(UnknownAnimSet.size() < 100)
		{
			if(UnknownAnimSet.insert(result).second)
				nlwarning("computeAnimSet: unknown Anim Set '%s' ('%u' unkowns).", result.c_str(), UnknownAnimSet.size());
		}
		// Try to compute the default one
		result = animSetBaseName + "_" + "default" + "__";
		animSetTmp = EAM->getAnimSet(result);
		if(animSetTmp)
			animSet = animSetTmp;
		else
		{
			// Up to 100 missing anim set (security).
			if(UnknownAnimSet.size() < 100)
			{
				if(UnknownAnimSet.insert(result).second)
					nlwarning("computeAnimSet: unknown Anim Set '%s' ('%u' unkowns).", result.c_str(), UnknownAnimSet.size());
			}
		}
	}

	// Not Well done.
	return false;
}// computeAnimSet //


//-----------------------------------------------
// dump :
// Create a file with information to debug.
//-----------------------------------------------
void dump(const std::string &name)
{
	// Write information to start as the version
	COFile fStart;
	if(fStart.open(name + "_start.rec", false, false))
	{
		CVectorD currentPos = UserEntity->pos();
		fStart.serialVersion(RecordVersion);
		fStart.serial(currentPos);
		// Close the File.
		fStart.close();
	}
	else
		nlwarning("dump: cannot open/create the file '%s_start.rec'.", name.c_str());

	// Write the DB
	IngameDbMngr.write(name + "_db.rec");
	// Open the file.
	COFile f;
	if(f.open(name + ".rec", false, false))
	{
		// Dump entities.
		EntitiesMngr.dump(f);

		// Dump Client CFG.
		ClientCfg.serial(f);

		// Close the File.
		f.close();
	}
	else
		nlwarning("dump: cannot open/create the file '%s.rec'.", name.c_str());


	// Open the file.
	if(f.open(name + ".xml", false, true))
	{
		// Create the XML stream
		COXml output;
		// Init
		if(output.init (&f, "1.0"))
		{
			// Open the XML Dump.
			output.xmlPush("XML");

			// Dump Client CFG.
			ClientCfg.serial(output);

			// Dump entities.
			EntitiesMngr.dumpXML(output);

			// Close the XML Dump.
			output.xmlPop();

			// Flush the stream, write all the output file
			output.flush();
		}
		else
			nlwarning("dump: cannot initialize '%s.xml'.", name.c_str());
		// Close the File.
		f.close();
	}
	else
		nlwarning("dump: cannot open/create the file '%s.xml'.", name.c_str());
}// dump //


//-----------------------------------------------
// loadDump :
// Create a file with the current state of the client (good to report a bug).
//-----------------------------------------------
void loadDump(const std::string &name)
{
	CVectorD currentPos;

	// Load information to start as the version
	CIFile fStart;
	if(fStart.open(name + "_start.rec", false))
	{
		fStart.serialVersion(RecordVersion);
		fStart.serial(currentPos);
		// Close the File.
		fStart.close();
	}
	else
		nlwarning("loadDump: cannot open the file '%s_start.rec'.", name.c_str());

	// Update the position for the vision.
	NetMngr.setReferencePosition(currentPos);

	// Select the closest continent from the new position.
	class CDummyProgress : public IProgressCallback
	{
		void progress (float /* value */) {}
	};
	CDummyProgress dummy;
	ContinentMngr.select(currentPos, dummy);

	// Load the DB
	IngameDbMngr.read(name + "_db.rec");

	// Open the file.
	CIFile f;
	if(f.open(name + ".rec", false))
	{
		// Dump entities.
		EntitiesMngr.dump(f);

		// Close the File.
		f.close();
	}
	else
		nlwarning("loadDump: cannot open '%s.rec'.", name.c_str());
}// loadDump //


//-----------------------------------------------
// getLodCharacterId
// Get the lod character id from the scene, according to LodCharacterName. Cached.
// -1 if id not found.
//-----------------------------------------------
sint getLodCharacterId(UScene &scene, const string &lodCharacterName)
{
	sint lodCharacterId = scene.getCLodShapeIdByName(lodCharacterName);
	// display a warning for bad character Id, only if name was setup in the sheet
	if(lodCharacterId==-1 && !lodCharacterName.empty() )
	{
		// Limited to 100 missing Lod to avoid memories problems
		if(LodCharactersNotFound.size() < 100)
		{
			// Insert and display a waring
			if(LodCharactersNotFound.insert(lodCharacterName).second)
				nlwarning("getLodCharacterId: Not found A Character LodCharacter in the Manager: %s", lodCharacterName.c_str());
		}
	}

	return lodCharacterId;
}// getLodCharacterId //


//-----------------------------------------------
// getItem :
//-----------------------------------------------
CItemSheet *getItem(const CGenderInfo &genderInfo, SLOTTYPE::EVisualSlot slot)
{
	CEntitySheet	*faceItem = SheetMngr.get(CSheetId(genderInfo.getItemName(slot)));
	return (dynamic_cast <CItemSheet *> (faceItem));
}// getItem //


//-----------------------------------------------
// getColorIndex :
//-----------------------------------------------
sint getColorIndex(const CGenderInfo &genderInfo, SLOTTYPE::EVisualSlot slot)
{
	CItemSheet *is = getItem(genderInfo, slot);
	if(is == 0)
		return 0;
	return is->Color;
}// getColorIndex //


//-----------------------------------------------
// buildPropVisualA :
//-----------------------------------------------
SPropVisualA buildPropVisualA(const CGenderInfo &genderInfo)
{
	SPropVisualA dest;
	dest.PropertyA = 0;
	// setup items
	dest.PropertySubData.JacketModel    = SheetMngr.getVSIndex(genderInfo.Items[SLOTTYPE::CHEST_SLOT],  SLOTTYPE::CHEST_SLOT);
	dest.PropertySubData.TrouserModel   = SheetMngr.getVSIndex(genderInfo.Items[SLOTTYPE::LEGS_SLOT], SLOTTYPE::LEGS_SLOT);
	dest.PropertySubData.ArmModel       = SheetMngr.getVSIndex(genderInfo.Items[SLOTTYPE::ARMS_SLOT], SLOTTYPE::ARMS_SLOT);
	dest.PropertySubData.HatModel       = SheetMngr.getVSIndex(genderInfo.Items[SLOTTYPE::HEAD_SLOT], SLOTTYPE::HEAD_SLOT);

	// setup colors (fixed or user)
	sint col = getColorIndex(genderInfo, SLOTTYPE::CHEST_SLOT);
	dest.PropertySubData.JacketColor    = (col == -1) ? 0 : col;
	//
	col = getColorIndex(genderInfo, SLOTTYPE::LEGS_SLOT);
	dest.PropertySubData.TrouserColor    = (col == -1) ? 0 : col;
	//
	col = getColorIndex(genderInfo, SLOTTYPE::ARMS_SLOT);
	dest.PropertySubData.ArmColor        = (col == -1) ? 0 : col;
	//
	col = getColorIndex(genderInfo, SLOTTYPE::HEAD_SLOT);
	dest.PropertySubData.HatColor        = (col == -1) ? 0 : col;

	// sheath are not used
	return dest;
}// buildPropVisualA //


//-----------------------------------------------
// buildPropVisualB :
//-----------------------------------------------
SPropVisualB buildPropVisualB(const CGenderInfo &genderInfo)
{
	// feet & hands
	SPropVisualB dest;
	dest.PropertyB = 0;
	// setup items
	dest.PropertySubData.HandsModel   = SheetMngr.getVSIndex(genderInfo.Items[SLOTTYPE::HANDS_SLOT],  SLOTTYPE::HANDS_SLOT);
	dest.PropertySubData.FeetModel    = SheetMngr.getVSIndex(genderInfo.Items[SLOTTYPE::FEET_SLOT],  SLOTTYPE::FEET_SLOT);

	// setup colors (fixed or user)
	sint col = getColorIndex(genderInfo, SLOTTYPE::HANDS_SLOT);
	dest.PropertySubData.HandsColor    = (col == -1) ? 0 : col;
	//
	col = getColorIndex(genderInfo, SLOTTYPE::FEET_SLOT);
	dest.PropertySubData.FeetColor    = (col == -1) ? 0 : col;

	return dest;
}// buildPropVisualB //




//-----------------------------------------------
// isUserColorSupported :
// Test whether user color is supported for this equipment
//-----------------------------------------------
bool isUserColorSupported(const CPlayerSheet::CEquipment &equip)
{
	NLMISC::CSheetId si;
	if(!si.buildSheetId(equip.Item))
		return false;

	CItemSheet *is = dynamic_cast<CItemSheet *>(SheetMngr.get(si));
	if(!is)
		return false;

	return is->Color == -1; // user color supported by the item
}// isUserColorSupported //

//-----------------------------------------------
// isUserColorSupported :
// Test whether user color is supported for a given visual slot
//-----------------------------------------------
bool isUserColorSupported(const CPlayerSheet &playerSheet, SLOTTYPE::EVisualSlot vs)
{
	switch(vs)
	{
		case SLOTTYPE::CHEST_SLOT:			return isUserColorSupported(playerSheet.Body);
		case SLOTTYPE::LEGS_SLOT:			return isUserColorSupported(playerSheet.Legs);
		case SLOTTYPE::HEAD_SLOT:			return isUserColorSupported(playerSheet.Head);
		case SLOTTYPE::ARMS_SLOT:			return isUserColorSupported(playerSheet.Arms);
		case SLOTTYPE::HANDS_SLOT:			return isUserColorSupported(playerSheet.Hands);
		case SLOTTYPE::FEET_SLOT:			return isUserColorSupported(playerSheet.Feet);
		case SLOTTYPE::RIGHT_HAND_SLOT:	return isUserColorSupported(playerSheet.ObjectInRightHand);
		case SLOTTYPE::LEFT_HAND_SLOT:	return isUserColorSupported(playerSheet.ObjectInLeftHand);
		default: break;
	}
	return false;
}// isUserColorSupported //


//-----------------------------------------------
// getColor :
//-----------------------------------------------
sint8 getColor(const CPlayerSheet::CEquipment &equip)
{
	NLMISC::CSheetId si;
	if(!si.buildSheetId(equip.Item))
		return -2;

	CItemSheet *is = dynamic_cast<CItemSheet *>(SheetMngr.get(si));
	if(!is)
		return -2;

	if(is->Color == -1) // user color supported by the item
	{
		return equip.Color;
	}
	else
	{
		return is->Color;
	}
}// getColor //

//-----------------------------------------------
// buildPropVisualA :
//-----------------------------------------------
SPropVisualA buildPropVisualA(const CPlayerSheet &playerSheet)
{
	SPropVisualA vp;
	// setup items
	vp.PropertySubData.JacketModel		= SheetMngr.getVSIndex(playerSheet.Body.Item,				SLOTTYPE::CHEST_SLOT);
	vp.PropertySubData.TrouserModel		= SheetMngr.getVSIndex(playerSheet.Legs.Item,				SLOTTYPE::LEGS_SLOT);
	vp.PropertySubData.ArmModel			= SheetMngr.getVSIndex(playerSheet.Arms.Item,				SLOTTYPE::ARMS_SLOT);
	vp.PropertySubData.HatModel			= SheetMngr.getVSIndex(playerSheet.Head.Item,				SLOTTYPE::HEAD_SLOT);
	vp.PropertySubData.WeaponRightHand	= SheetMngr.getVSIndex(playerSheet.ObjectInRightHand.Item,	SLOTTYPE::RIGHT_HAND_SLOT);
	vp.PropertySubData.WeaponLeftHand	= SheetMngr.getVSIndex(playerSheet.ObjectInLeftHand.Item,	SLOTTYPE::LEFT_HAND_SLOT);

	// setup colors
	vp.PropertySubData.JacketColor	= getColor(playerSheet.Body);
	vp.PropertySubData.TrouserColor	= getColor(playerSheet.Legs);
	vp.PropertySubData.ArmColor		= getColor(playerSheet.Arms);
	vp.PropertySubData.HatColor		= getColor(playerSheet.Head);

	return vp;
}// buildPropVisualA //

//-----------------------------------------------
// buildPropVisualB :
//-----------------------------------------------
SPropVisualB buildPropVisualB(const CPlayerSheet &playerSheet)
{
	SPropVisualB vp;
	// setup items
	vp.PropertySubData.Name = 0;
	vp.PropertySubData.HandsModel	= SheetMngr.getVSIndex(playerSheet.Hands.Item,	SLOTTYPE::HANDS_SLOT);
	vp.PropertySubData.FeetModel	= SheetMngr.getVSIndex(playerSheet.Feet.Item,	SLOTTYPE::FEET_SLOT);

	// setup colors
	vp.PropertySubData.HandsColor = getColor(playerSheet.Hands);
	vp.PropertySubData.FeetColor  = getColor(playerSheet.Feet);

	return vp;
}// buildPropVisualB //


//-----------------------------------------------
// drawBox :
// Draw a Box from 2 vectors.
//-----------------------------------------------
void drawBox(const CVector &vMin, const CVector &vMax, const CRGBA &color)
{
	CLineColor line;
	line.Color0 = color;
	line.Color1 = color;
	// Bottom quad
	line = CLine(CVector(vMin.x,vMin.y,vMin.z), CVector(vMax.x,vMin.y,vMin.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x,vMin.y,vMin.z), CVector(vMax.x,vMax.y,vMin.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x,vMax.y,vMin.z), CVector(vMin.x,vMax.y,vMin.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMin.x,vMax.y,vMin.z), CVector(vMin.x,vMin.y,vMin.z));
	Driver->drawLine(line, GenericMat);
	// Top quad
	line = CLine(CVector(vMin.x,vMin.y,vMax.z), CVector(vMax.x,vMin.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x,vMin.y,vMax.z), CVector(vMax.x,vMax.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x,vMax.y,vMax.z), CVector(vMin.x,vMax.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMin.x,vMax.y,vMax.z), CVector(vMin.x,vMin.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	// Sides Quad
	line = CLine(CVector(vMin.x,vMin.y,vMin.z), CVector(vMin.x,vMin.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x,vMin.y,vMin.z), CVector(vMax.x,vMin.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMax.x,vMax.y,vMin.z), CVector(vMax.x,vMax.y,vMax.z));
	Driver->drawLine(line, GenericMat);
	line = CLine(CVector(vMin.x,vMax.y,vMin.z), CVector(vMin.x,vMax.y,vMax.z));
	Driver->drawLine(line, GenericMat);
}// drawBox //


//-----------------------------------------------
// drawSphere :
// Draw a Sphere
//-----------------------------------------------
void drawSphere(const NLMISC::CVector &center, float radius, const NLMISC::CRGBA &color)
{
	const	uint	numSegs= 12;
	CLineColor line;
	line.Color0 = color;
	line.Color1 = color;
	// For all faces
	for(uint face=0;face<3;face++)
	{
		for(uint i=0;i<numSegs;i++)
		{
			float	angStart= float(i*2*Pi/numSegs);
			float	angEnd= float(((i+1)%numSegs)*2*Pi/numSegs);
			line.V0= radius*CVector(cosf(angStart), sinf(angStart), 0.f);
			line.V1= radius*CVector(cosf(angEnd), sinf(angEnd), 0.f);
			if(face==1)
			{
				swap(line.V0.x, line.V0.z);
				swap(line.V1.x, line.V1.z);
			}
			else if(face==2)
			{
				swap(line.V0.y, line.V0.z);
				swap(line.V1.y, line.V1.z);
			}
			line.V0+= center;
			line.V1+= center;
			Driver->drawLine(line, GenericMat);
		}
	}
}


//-----------------------------------------------
// getSeedsFromDB :
//-----------------------------------------------
void getSeedsFromDB(CSeeds &dest)
{
	CInterfaceManager *im =CInterfaceManager::getInstance();
	nlctassert(sizeof(CSeeds::TUInt) == 4); // excpect that the number of each seed type is encoded on 32 bits
											// if this assert at compile, change the following code
	string ls = CWidgetManager::getInstance()->getParser()->getDefine("money_1");
	string ms = CWidgetManager::getInstance()->getParser()->getDefine("money_2");
	string bs = CWidgetManager::getInstance()->getParser()->getDefine("money_3");
	string vbs = CWidgetManager::getInstance()->getParser()->getDefine("money_4");

	dest = CSeeds(NLGUI::CDBManager::getInstance()->getDbProp(ls)->getValue32(),
				  NLGUI::CDBManager::getInstance()->getDbProp(ms)->getValue32(),
				  NLGUI::CDBManager::getInstance()->getDbProp(bs)->getValue32(),
				  NLGUI::CDBManager::getInstance()->getDbProp(vbs)->getValue32());
} // getSeedsFromDB //


//-----------------------------------------------
// setVect :
// Change a 'direction' vector.
// \param vectToChange : the vector to change.
// \param vect : new vector to use.
// \param compute : adjust the param 'vect' to be valid or leave the old one unchanged if impossible.
// \param check : warning if the param 'vect' is not valid (vector Null) even with compute=true.
// \return bool : 'true' if the vectToChange has been filled, else 'false'.
//-----------------------------------------------
bool setVect(CVector &vectToChange, const CVector &vect, bool compute, bool check)
{
	// Compute the vector.
	if(compute)
	{
		CVector vectTmp(vect);
		// No need of the Z component for the vector.
		vectTmp.z = 0.0f;
		// Check vector.
		if(vectTmp != CVector::Null)
		{
			vectTmp.normalize();
			vectToChange = vectTmp;
			return true;
		}
		// Bad Vector -> vectToChange remains the same
		else
		{
			// Warning
			if(check)
				nlwarning("setVect: cannot compute the vector, keep the old one.");
			return false;
		}
	}

	// Bad Vector -> vectToChange remains the same
	if(vect == CVector::Null)
	{
		// Warning
		if(check)
			nlwarning("setVect: param is a vector Null, vectToChange remains the same.");
		return false;
	}

	// Set the new front vector.
	vectToChange = vect;
	return true;
}// setVect //

NLMISC::CRGBA interpClientCfgColor(const ucstring &src, ucstring &dest)
{
	CRGBA color = CRGBA::White;
	if (src.size() >= 3)
	{
		if (src[0] == (ucchar) '&')
		{
			ucstring::size_type nextPos = src.find((ucchar) '&', 1);
			if (nextPos != ucstring::npos)
			{
				std::string colorCode;
				colorCode.resize(nextPos - 1);
				for(uint k = 0; k < nextPos - 1; ++k)
				{
					colorCode[k] = tolower((char) src[k + 1]);
				}
				std::map<std::string, CClientConfig::SSysInfoParam>::const_iterator it = ClientCfg.SystemInfoParams.find(colorCode);
				if (it != ClientCfg.SystemInfoParams.end())
				{
					color = it->second.Color;
				}
				dest = src.substr(nextPos + 1);
			}
			else
			{
				dest = src;
			}
		}
		else
		{
			dest = src;
		}
	}
	else
	{
		dest = src;
	}
	return color;
}

std::string getStringCategory(const ucstring &src, ucstring &dest, bool alwaysAddSysByDefault)
{
	std::string str = getStringCategoryIfAny(src, dest);
	if (alwaysAddSysByDefault)
		return str.empty()?"SYS":str;
	else
		return str;
}


std::string getStringCategoryIfAny(const ucstring &src, ucstring &dest)
{
	std::string colorCode;
	if (src.size() >= 3)
	{
		uint startPos = 0;

		// Skip <NEW> or <CHG> if present at beginning
		ucstring preTag;
		const uint PreTagSize = 5;
		const ucstring newTag("<NEW>");
		if ( (src.size() >= PreTagSize) && (src.substr( 0, PreTagSize ) == newTag) )
		{
			startPos = PreTagSize;
			preTag = newTag;
		}
		const ucstring chgTag("<CHG>");
		if ( (src.size() >= PreTagSize) && (src.substr( 0, PreTagSize ) == chgTag) )
		{
			startPos = PreTagSize;
			preTag = chgTag;
		}

		if (src[startPos] == (ucchar) '&')
		{
			ucstring::size_type nextPos = src.find((ucchar) '&', startPos+1);
			if (nextPos != ucstring::npos)
			{
				uint codeSize = (uint)nextPos - startPos - 1;
				colorCode.resize( codeSize );
				for(uint k = 0; k < codeSize; ++k)
				{
					colorCode[k] = tolower((char) src[k + startPos + 1]);
				}
				ucstring destTmp;
				if ( startPos != 0 )
					destTmp = preTag; // leave <NEW> or <CHG> in the dest string
				destTmp += src.substr(nextPos + 1);
				dest = destTmp;
			}
			else
			{
				dest = src;
			}
		}
		else
		{
			dest = src;
		}
	}
	else
	{
		dest = src;
	}
	return colorCode;
}


// ***************************************************************************
sint ucstrnicmp(const ucstring &s0, uint p0, uint n0, const ucstring &s1)
{
	// start
	const ucchar	*start1= s1.c_str();
	uint			lenS1= (uint)s1.size();
	const ucchar	*start0= s0.c_str();
	uint			lenS0= (uint)s0.size();
	if(p0!=0)
	{
		if(p0<lenS0)
		{
			start0+= p0;
			lenS0-= p0;
		}
		else
		{
			start0+= lenS0;		// points to '\0'
			lenS0= 0;
		}
	}
	lenS0= min(lenS0, n0);

	// compare character to character
	while(lenS0>0 && lenS1>0)
	{
		ucchar	c0= toLower(*start0++);
		ucchar	c1= toLower(*start1++);
		if(c0!=c1)
			return c0<c1?-1:+1;
		lenS0--;
		lenS1--;
	}

	// return -1 if s1>s0, 1 if s0>s1, or 0 if equals
	if(lenS1>0)
		return -1;
	else if(lenS0>0)
		return 1;
	else
		return 0;
}


// *******************************************************************************************
float computeUniformNoise(const NLMISC::CNoiseValue &nv, const CVector &pos)
{
	NLMISC::OptFastFloorBegin();
	float value = nv.eval(pos);
	value = 10.f * fmodf(value, 0.1f); // make repartition more uniform
	NLMISC::OptFastFloorEnd();
	return value;
}


// ***************************************************************************
void computeCurrentFovAspectRatio(float &fov, float &ar)
{
	// compute the fov
	fov = (float)(ClientCfg.FoV*Pi/180.0);

	// get the screen aspect ratio from CFG
	ar = ClientCfg.ScreenAspectRatio;

	// if Driver is not created, we can't get current screen mode
	if (!Driver) return;

	// if windowed, must modulate aspect ratio by (WindowResolution / ScreenResolution)
	if(ClientCfg.Windowed)
	{
		uint32	wndW, wndH;
		Driver->getWindowSize(wndW, wndH);
		UDriver::CMode	mode;
		Driver->getCurrentScreenMode(mode);
		if(wndH)
		{
			// compute window aspect ratio
			float	arWnd= float(wndW) / float(wndH);
			if (ar == 0.f)
			{
				// auto mode, we are using window aspect ratio
				ar = arWnd;
			}
			else if (mode.Width && mode.Height)
			{
				// compute screen aspect ratio
				float	arScreen= float(mode.Width) / float(mode.Height);
				ar *= arWnd / arScreen;
			}
		}
	}
	// if fullscreen, must modulate aspect ratio by ScreenResolution
	else
	{
		if (ar == 0.f)
		{
			UDriver::CMode	mode;
			Driver->getCurrentScreenMode(mode);
			if(mode.Height)
			{
				ar = float(mode.Width) / float(mode.Height);
			}
		}
	}
}

// ***************************************************************************
void drawDisc(CBitmap &dest, float x, float y, float radius, const CRGBA &color, bool additif /*= false*/, uint numSegs /*= 127*/)
{
	CPolygon2D poly;
	poly.Vertices.resize(numSegs);
	for(uint k = 0; k < numSegs; ++k)
	{
		poly.Vertices[k].set(x + radius * (float) cos(k / (float) numSegs * 2 * Pi), y + radius * (float) sin(k / (float) numSegs * 2 * Pi));
	}
	CPolygon2D::TRasterVect rasters;
	sint minY;
	poly.computeOuterBorders(rasters, minY);
	sint maxY = std::min((sint) dest.getHeight(), (sint) rasters.size() + minY);
	for (sint y = std::max((sint) 0, minY); y < maxY; ++y)
	{
		nlassert(y >= 0 && y < (sint) dest.getHeight());
			sint minX = std::max((sint) 0, rasters[y - minY].first);
		sint maxX = std::min((sint) dest.getWidth(), rasters[y - minY].second);
		if (maxX > minX)
		{
			CRGBA *pt = (CRGBA *) &dest.getPixels(0)[0];
			pt += y * dest.getWidth() + minX;
			const CRGBA *endPt = pt + (maxX - minX);
			while (pt != endPt)
			{
				if (additif)
				{
					pt->add(*pt, color);
				}
				else
				{
					*pt = color;
				}
				++ pt;
			}
		}
	}
}


// *************************************************************************************************
NL3D::UScene *getSkyScene()
{
	if (ContinentMngr.cur() && !ContinentMngr.cur()->Indoor)
	{
		CSky &sky = ContinentMngr.cur()->CurrentSky;
		if (sky.getScene())
		{
			return sky.getScene();
		}
		else
		{
			return SkyScene; // old sky rendering
		}
	}
	return NULL;
}

// *************************************************************************************************
void setEmissive(NL3D::UInstance instance, const NLMISC::CRGBA &color)
{
	if (instance.empty()) return;
	for(uint k = 0; k < instance.getNumMaterials(); ++k)
	{
		NL3D::UInstanceMaterial mat = instance.getMaterial(k);
		mat.setEmissive(color);
	}
}

// *************************************************************************************************
void setDiffuse(NL3D::UInstance instance, bool onOff, const NLMISC::CRGBA &color)
{
	if (instance.empty()) return;
	for(uint k = 0; k < instance.getNumMaterials(); ++k)
	{
		NL3D::UInstanceMaterial mat = instance.getMaterial(k);
		if (mat.isLighted())
		{
			CRGBA src;
			if (onOff)
			{
				src = mat.getDiffuse();
				src.R = color.R;
				src.G = color.G;
				src.B = color.B;
			}
			else
			{
				NL3D::UMaterial			matShape = instance.getShape().getMaterial(k);
				src = matShape.getDiffuse();
				src.A = mat.getDiffuse().A;
			}
			mat.setDiffuse(src);
		}
		else
		{
			CRGBA src;
			if (onOff)
			{
				src = mat.getColor();
				src.R = color.R;
				src.G = color.G;
				src.B = color.B;
			}
			else
			{
				NL3D::UMaterial			matShape = instance.getShape().getMaterial(k);
				src = matShape.getColor();
				src.A = mat.getColor().A;
			}
			mat.setColor(src);
		}
	}
}



// *************************************************************************************************
void makeInstanceTransparent(UInstance	&inst, uint8 opacity, bool disableZWrite)
{
	UShape		shape= inst.getShape();
	if(shape.empty())
		return;
	uint	numMats= shape.getNumMaterials();
	if(numMats==0)
		return;
	if(numMats!=inst.getNumMaterials())
		return;

	// instance transparent or not?
	if (opacity == 255)
	{
		// reset default shape opacity / transparency
		inst.setOpacity(shape.getDefaultOpacity());
		inst.setTransparency(shape.getDefaultTransparency());
		inst.setBypassLODOpacityFlag(false);
	}
	else
	{
		// Will have some blend material => sure not to be rendered in Opaque pass
		inst.setOpacity(false);
		inst.setTransparency(true);
		inst.setBypassLODOpacityFlag(true); // these flags prevails over the current lods flags for multi-lod objects
	}

	// set all materials
	for (uint32 j = 0; j < numMats; ++j)
	{
		NL3D::UInstanceMaterial matInst = inst.getMaterial(j);
		NL3D::UMaterial			matShape= shape.getMaterial(j);

		// disalbe zwrite?
		if(disableZWrite)
			matInst.setZWrite(false);
		else
			matInst.setZWrite(matShape.getZWrite());

		// if no more transparent
		if (opacity == 255)
		{
			// reset to default
			matInst.setBlend(matShape.getBlend());
			matInst.setBlendFunc((NL3D::UInstanceMaterial::TBlend)matShape.getSrcBlend(),
				(NL3D::UInstanceMaterial::TBlend)matShape.getDstBlend());
			// if orginal material is opaque or additif and has no alpha test, then ensure restore last tex env if needed
			CMaterial *destInternalMat = matInst.getObjectPtr();
			if (!matShape.getBlend() && !matShape.getAlphaTest())
			{
				if (destInternalMat->getShader() == CMaterial::Normal)
				{
					CMaterial *srcInternalMat = matShape.getObjectPtr();
					uint numTex = 0;
					for (;numTex < 4 && srcInternalMat->getTexture(numTex) != NULL; ++numTex) {}
					if (numTex > 0)
					{
						if (srcInternalMat->getTexEnvMode(numTex - 1) != destInternalMat->getTexEnvMode(numTex - 1))
						{
							destInternalMat->setTexEnvMode(numTex - 1, srcInternalMat->getTexEnvMode(numTex - 1));
						}
					}
				}
			}
			if (destInternalMat->getShader() == CMaterial::Normal)
			{
				// if !lighted, restore color
				if (!destInternalMat->isLighted())
				{
					CMaterial *srcInternalMat = matShape.getObjectPtr();
					// restore alpha in color
					CRGBA color = destInternalMat->getColor();
					color.A = srcInternalMat->getColor().A;
					destInternalMat->setColor(color);
				}
			}
		}
		else
		{
			// Enable blend
			matInst.setBlend(true);
			// If default is ???/one or , then use a srcalpha/one  (eg: for Diamond-like weapons)
			if(matShape.getBlend() && (sint32)matShape.getDstBlend()==(sint32)NL3D::UInstanceMaterial::one)
				matInst.setBlendFunc(NL3D::UInstanceMaterial::srcalpha, NL3D::UInstanceMaterial::one);
			// else use a standard srcalpha/invsrcalpha
			else
				matInst.setBlendFunc(NL3D::UInstanceMaterial::srcalpha, NL3D::UInstanceMaterial::invsrcalpha);
			// if orginal material is opaque or additif and has no alpha test, then ensure that the alpha output is 'diffuse'
			CMaterial *internalMat = matInst.getObjectPtr();
			if (!matShape.getBlend() && !matShape.getAlphaTest())
			{
				if (internalMat->getShader() == CMaterial::Normal)
				{
					uint numTex = 0;
					for (;numTex < 4 && internalMat->getTexture(numTex) != NULL; ++numTex) {}
					if (numTex > 0)
					{
						internalMat->texEnvOpAlpha(numTex - 1, CMaterial::Replace);
						// if material is unlighted, then use the constant at this stage to set the alpha
						internalMat->texEnvArg0Alpha(numTex - 1, CMaterial::Diffuse, CMaterial::SrcAlpha);
					}
				}
			}
			if (internalMat->getShader() == CMaterial::Normal)
			{
				if (!internalMat->isLighted())
				{
					// replace alpha in color
					CRGBA color = internalMat->getColor();
					color.A = opacity;
					internalMat->setColor(color);
				}
			}
		}

		// suppose that default opacity is always 255
		if (matInst.isLighted())
		{
			matInst.setOpacity(opacity);
		}

		matInst.setAlphaTestThreshold(matShape.getAlphaTestThreshold()*((float)opacity)/255.0f);
	}
}

void setVideoMode(const UDriver::CMode &mode)
{
	UDriver::CMode oldMode, newMode = mode;
	oldMode.Windowed = true; // getCurrentScreenMode may fail if first init ...
	Driver->getCurrentScreenMode(oldMode);
	bool wasMaximized = isWindowMaximized();
	if (!Driver->setMode(newMode) && !newMode.Windowed)
	{
		// failed to switch to mode, fall back to windowed
		newMode.Windowed = true;
		ClientCfg.Windowed = true;
		ClientCfg.writeInt("FullScreen", 0);

		// set the window mode
		Driver->setMode(newMode);
	}
	bool isMaximized = isWindowMaximized();
	if (oldMode.Windowed && !newMode.Windowed) // going to fullscreen ?
	{
		/*CInterfaceManager *pIM = CInterfaceManager::getInstance();
		pIM->movePointerAbs((sint32) mode.Width / 2, (sint32) mode.Height / 2);
		Driver->setMousePos(0.5f, 0.5f);*/
	}
	else if ((!oldMode.Windowed || wasMaximized) && (newMode.Windowed && !isMaximized)) // leaving fullscreen ?
	{
		UDriver::CMode screenMode;

		uint32 posX = 0;
		uint32 posY = 0;

		if (Driver->getCurrentScreenMode(screenMode))
		{
			// position is not saved in config so center the window
			posX = (screenMode.Width - Driver->getWindowWidth())/2;
			posY = (screenMode.Height - Driver->getWindowHeight())/2;
		}

		Driver->setWindowPos(posX, posY);
	}
}

uint getCurrentColorDepth()
{
	if (Driver && Driver->isActive())
	{
		UDriver::CMode videoMode;
		Driver->getCurrentScreenMode(videoMode);
		if (!videoMode.Windowed)
		{
			return videoMode.Depth;
		}
	}

	return CSystemUtils::getCurrentColorDepth();
}

bool isWindowMaximized()
{
	UDriver::CMode screenMode;
	uint32 width, height;

	Driver->getWindowSize(width, height);

	return (Driver->getCurrentScreenMode(screenMode) && screenMode.Windowed &&
		screenMode.Width == width && screenMode.Height == height);
}

sint getRyzomModes(std::vector<NL3D::UDriver::CMode> &videoModes, std::vector<std::string> &stringModeList)
{
	// **** Init Video Modes
	Driver->getModes(videoModes);
	// Remove modes under 800x600 and get the unique strings
	sint i, j, nFoundMode = -1;
	for (i=0; i < (sint)videoModes.size(); ++i)
	{
		if ((videoModes[i].Width < 800) || (videoModes[i].Height < 600))
		{
			videoModes.erase(videoModes.begin()+i);
			--i;
		}
		else
		{
			bool bFound = false;
			string tmp = toString(videoModes[i].Width)+" x "+toString(videoModes[i].Height);
			for (j = 0; j < (sint)stringModeList.size(); ++j)
			{
				if (stringModeList[j] == tmp)
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
			{
				stringModeList.push_back(tmp);
				if ((videoModes[i].Width <= ClientCfg.Width) && (videoModes[i].Height <= ClientCfg.Height))
				{
					if (nFoundMode == -1)
					{
						nFoundMode = j;
					}
					else
					{
						if ((videoModes[i].Width >= videoModes[nFoundMode].Width) &&
							(videoModes[i].Height >= videoModes[nFoundMode].Height))
							nFoundMode = j;
					}
				}
			}
		}
	}
	
	// If no modes are available, fallback to windowed mode
	if (nFoundMode == -1)
	{
		nlwarning("Mode %ux%u not found, fall back to windowed", (uint)ClientCfg.Width, (uint)ClientCfg.Height);
		ClientCfg.Windowed = true;
		ClientCfg.writeInt("FullScreen", 0);
	}

	return nFoundMode;
}
