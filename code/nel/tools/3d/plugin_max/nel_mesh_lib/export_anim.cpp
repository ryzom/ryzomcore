// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdafx.h"
#include "export_nel.h"
#include "nel/3d/camera.h"
#include "nel/3d/transform.h"
#include "nel/3d/animation.h"
#include "nel/3d/animated_material.h"
#include "nel/3d/key.h"
#include "nel/3d/track.h"
#include "nel/3d/particle_system_model.h"
#include "nel/3d/skeleton_model.h"
#include "nel/misc/algo.h"
#include <notetrck.h>
#include <functional>

#include "calc_lm.h"
#include "export_appdata.h"

using namespace NLMISC;
using namespace NL3D;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

static Class_ID DefNoteTrackClassID(NOTETRACK_CLASS_ID, 0);

#define BOOL_CONTROL_CLASS_ID 0x984b8d27


// --------------------------------------------------

class CExportDesc
{
public:

	// Default cstr;
	CExportDesc ()
	{
		reset ();
	}

	// Reset the desc
	void reset ()
	{
		Specular=false;
	}


	// This value is specular
	bool		Specular;
	StdMat2		*Material;
};


// --------------------------------------------------

// OverSamples fo BIPED animation export.
#define NL3D_BIPED_OVERSAMPLING 30

// Add track in this animation
void CExportNel::addAnimation (CAnimation& animation, INode& node, const char* sBaseName, bool root)
{
	// Get the TM controler
	Control *transform=node.GetTMController();


	// Build information for this nodes and his sons.
	CAnimationBuildCtx	animBuildCtx;
	// Build the biped information.
	buildBipedInformation(animBuildCtx, node);
	animBuildCtx.compileBiped();
	// If it is a biped, over Samples now along the animation, and get Keys values.
	if(animBuildCtx.hasBipedNodes())
	{
		overSampleBipedAnimation(animBuildCtx, NL3D_BIPED_OVERSAMPLING);
	}

	// For Skeleton Spawn Script
	CSSSBuild	ssBuilder;

	// Is it a biped node ?
	if (transform && (transform->ClassID() == BIPBODY_CONTROL_CLASS_ID))
	{
		// Export biped skeleton animation
		addBipedNodeTracks (animation, node, sBaseName, &animBuildCtx, root, ssBuilder);
	}	
	else
	{
		// Add node tracks
		addNodeTracks (animation, node, sBaseName, NULL, root, ssBuilder);

		// Get the object pointer
		Object* obj=node.GetObjectRef();

		// Export the object if it exists
		if (obj)
			addObjTracks (animation, *obj, sBaseName);

		// Export material tracks of this object
		int exportNodeMaterial = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_EXPORT_ANIMATED_MATERIALS, 0);
		{
			if (exportNodeMaterial)
			{			
				Mtl* mtl=node.GetMtl();
				if (mtl)
				{
					// Add material tracks in the animation
					addMtlTracks (animation, *mtl, sBaseName);
				}
			}
		}

		// Add light tracks
		addLightTracks (animation, node, sBaseName);

		// Add particle system tracks
		addParticleSystemTracks(animation, node, sBaseName);

		// Add morph tracks
		addMorphTracks (animation, node, sBaseName);

		// Add bones track
		uint childrenCont=(uint)node.NumberOfChildren();
		for (uint children=0; children<childrenCont; children++)
		{
			INode *child=node.GetChildNode(children);
			addBoneTracks (animation, *child, sBaseName, &animBuildCtx, false, ssBuilder);
		}
	}

	// check for note track export (a string track used to create events)
	int exportNoteTrack = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_EXPORT_NOTE_TRACK, 0);	
	if (exportNoteTrack)
	{
		addNoteTrack(animation, node);		
	}

	// Compile the SkeletonSpawnScript builder
	ssBuilder.compile(animation, sBaseName);
}


// --------------------------------------------------
void CExportNel::addNoteTrack(NL3D::CAnimation& animation, INode& node)
{
	NL3D::CTrackKeyFramerConstString*	st= buildFromNoteTrack(node);
	
	if(st)
		animation.addTrack(std::string("NoteTrack"), st);		
}


// --------------------------------------------------
void CExportNel::addSSSTrack(CSSSBuild	&ssBuilder, INode& node)
{
	CSSSBuild::CBoneScript	bs;
	bs.BoneName= getName (node);

	// Build from note Track (first one)
	NoteTrack *nt = node.GetNoteTrack(0);
    if(nt && (nt->ClassID() == DefNoteTrackClassID))
    {
        DefNoteTrack &dnt = *(DefNoteTrack *)nt;
        int noteCount = dnt.keys.Count();
		float firstDate = 0, lastDate = 0;
		
		// build bs.Track
		bs.Track.reserve(noteCount);
        for(int noteIndex = 0; noteIndex < noteCount; ++noteIndex)
        {
            NoteKey *note = dnt.keys[noteIndex];
			
            if(note)
            {
				CSSSBuild::CKey		ks;
				ks.Value = MaxTStrToUtf8(note->note);
				ks.Time= CExportNel::convertTime (note->time);
				bs.Track.push_back(ks);
            }
        }
		
		// if some key, add to the builder
		if(!bs.Track.empty())
			ssBuilder.Bones.push_back(bs);
	}
}

// --------------------------------------------------
NL3D::CTrackKeyFramerConstString*		CExportNel::buildFromNoteTrack(INode& node)
{
	// check for the first Note Track		
	NoteTrack *nt = node.GetNoteTrack(0);
	
    if(nt && (nt->ClassID() == DefNoteTrackClassID))
    {
		
		CTrackKeyFramerConstString *st = new CTrackKeyFramerConstString;
		
        DefNoteTrack &dnt = *(DefNoteTrack *)nt;
        int noteCount = dnt.keys.Count();
		float firstDate = 0, lastDate = 0;
		
        for(int noteIndex = 0; noteIndex < noteCount; ++noteIndex)
        {
			
            NoteKey *note = dnt.keys[noteIndex];
			
            if(note)
            {
				CKeyString ks;
				if (noteIndex == 0)
				{
					firstDate = CExportNel::convertTime (note->time);
				}				
				ks.Value = MaxTStrToUtf8(note->note);
				lastDate = CExportNel::convertTime (note->time);
				st->addKey(ks , lastDate );
				
            }
        }
		st->unlockRange (firstDate, lastDate);

		return st;
	}
	else
		return NULL;
}


// --------------------------------------------------
void CExportNel::addParticleSystemTracks(CAnimation& animation, INode& node, const char* parentName)
{	
	Class_ID  clid = node.GetObjectRef()->ClassID() ;
	/// is this a particle system ?
	if (clid.PartA() != NEL_PARTICLE_SYSTEM_CLASS_ID)
		return ;

	// Export desc
	CExportDesc desc;
	const char *paramName[] = { "PSParam0", "PSParam1", "PSParam2", "PSParam3" } ;
	

	for (uint k = 0 ; k < 4 ; ++k)
	{
		Control *ctrl = getControlerByName(node, paramName[k]) ;
		if (ctrl)
		{
			ITrack *pTrack=buildATrack (animation, *ctrl, typeFloat, node, desc, NULL);
			if (pTrack)
			{
				std::string name=parentName+std::string (NL3D::CParticleSystemModel::getPSParamName((uint) NL3D::CParticleSystemModel::PSParam0 + k));
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
					pTrack = NULL;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}	
	}

	Control *ctrl = getControlerByName(node, "PSTrigger");
	if (ctrl)
	{
		ITrack *pTrack = buildOnOffTrack(*ctrl);
		std::string name=parentName+std::string("PSTrigger");
		if (animation.getTrackByName (name.c_str()))
		{
			delete pTrack;
			pTrack = NULL;
		}
		else
		{
			animation.addTrack(name, pTrack);
		}
	}
}


// --------------------------------------------------

void CExportNel::addNodeTracks (CAnimation& animation, INode& node, const char* parentName,
								CAnimationBuildCtx	*animBuildCtx, bool root, CSSSBuild &ssBuilder, bool bodyBiped)
{
	// Tmp track*
	ITrack *pTrack;

	// Tmp name
	std::string name;

	// Export desc
	CExportDesc desc;

	// Export scale
	desc.reset();

	// Get the transformation controler
	Control *transform=node.GetTMController();

	// It exists ?
	if (transform)
	{
		// Get the Scale controler
		Control *c=transform->GetScaleController ();
		if (c)
		{
			// Build the track
			pTrack=buildATrack (animation, *c, typeScale, node, desc, animBuildCtx, bodyBiped);
			if (pTrack)
			{
				name=parentName+std::string (ITransformable::getScaleValueName());
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
					pTrack = NULL;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}

		// Export rotation
		desc.reset();

		// Get the Rotation controler
		c=transform->GetRotationController ();
		if (c)
		{
			pTrack=buildATrack (animation, *c, typeRotation, node, desc, animBuildCtx, bodyBiped);
			if (pTrack)
			{
				// Choose the good name for this track
				name=parentName+std::string (ITransformable::getRotQuatValueName());
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
					pTrack = NULL;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}

		// Export position
		desc.reset();

		// Get the Position controler
		c=transform->GetPositionController ();
		if (c)
		{
			pTrack=buildATrack (animation, *c, typePos, node, desc, animBuildCtx, bodyBiped);
			if (pTrack)
			{
				// Choose the good name for this track
				name=parentName+std::string (ITransformable::getPosValueName());
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
					pTrack = NULL;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}
		else
		{
			// try to find a biped controller for position
			pTrack=buildATrack (animation, *transform, typePos, node, desc, animBuildCtx, bodyBiped);
			if (pTrack)
			{
				// Choose the good name for this track
				name=parentName+std::string (ITransformable::getPosValueName());
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
					pTrack = NULL;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}

		// Export roll for camera
		desc.reset();

		if (isCamera (node, 0))
		{
			// Get the Roll controler
			c=transform->GetRollController ();
			if (c)
			{
				pTrack=buildATrack (animation, *c, typeFloat, node, desc, animBuildCtx, bodyBiped);
				if (pTrack)
				{
					name=parentName+std::string (CCamera::getRollValueName());
					if (animation.getTrackByName (name.c_str()))
					{
						delete pTrack;
						pTrack = NULL;
					}
					else
					{
						animation.addTrack (name.c_str(), pTrack);
					}
				}
			}

			// Export target position for camera
			desc.reset();

			// Get the target controler
			INode *target = node.GetTarget ();
			if (target)
			{
				// Get the transformation controler
				Control *targetTransform = target->GetTMController();

				// Get the Position controler
				Control *targetC = targetTransform->GetPositionController ();
				if (targetC)
				{
					pTrack=buildATrack (animation, *targetC, typePos, *target, desc, animBuildCtx, bodyBiped);
					if (pTrack)
					{
						// Choose the good name for this track
						name=parentName+std::string (CCamera::getTargetValueName());
						if (animation.getTrackByName (name.c_str()))
						{
							delete pTrack;
							pTrack = NULL;
						}
						else
						{
							animation.addTrack (name.c_str(), pTrack);
						}
					}
				}
			}
		}
	}

	// check for SkeletonSpawnScript track export
	int exportSSS = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_EXPORT_SSS_TRACK, 0);	
	if(exportSSS)
	{
		addSSSTrack(ssBuilder, node);
	}
}

// --------------------------------------------------

void CExportNel::addBipedNodeTracks (CAnimation& animation, INode& node, const char* parentName, CAnimationBuildCtx	*animBuildCtx, bool root, CSSSBuild &ssBuilder)
{
	// Get the matrix controler
	Control *transform=node.GetTMController();
	nlassert (transform);

	// Biped node
	if ((transform->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
		(transform->ClassID() == BIPBODY_CONTROL_CLASS_ID))
	{
		// Body biped ?
		bool bodyBiped = ((transform->ClassID() == BIPBODY_CONTROL_CLASS_ID) != 0);

		// Create a track name
		std::string name;
		
		// Choose the good name for this track
		if (!root)
			name=parentName + getName (node) + ".";
		else
			name=parentName;

		// Export keyframes
		addNodeTracks (animation, node, name.c_str(), animBuildCtx, root, ssBuilder, bodyBiped);

		// Add child tracks
		uint childrenCont=(uint)node.NumberOfChildren();
		for (uint children=0; children<childrenCont; children++)
		{
			INode *child=node.GetChildNode(children);
			addBipedNodeTracks (animation, *child, parentName, animBuildCtx, false, ssBuilder);
		}
	}
	else
	{
		// Add normal tracks
		addBoneTracks (animation, node, parentName, animBuildCtx, root, ssBuilder);
	}
}

// --------------------------------------------------

void CExportNel::addBoneTracks (NL3D::CAnimation& animation, INode& node, const char* parentName, CAnimationBuildCtx *animBuildCtx, bool root, CSSSBuild &ssBuilder)
{
	// Create a track name
	std::string name=parentName + getName (node) + ".";

	// Get the TM controler
	Control *transform=node.GetTMController();

	// Is it a biped node ?
	if (transform && (transform->ClassID() == BIPBODY_CONTROL_CLASS_ID))
	{
		// Export biped skeleton animation
		addBipedNodeTracks (animation, node, parentName, animBuildCtx, root, ssBuilder);
	}
	else
	{
		// Go for normal export!
		addNodeTracks (animation, node, name.c_str(), NULL, root, ssBuilder);
	}

	// Recursive call
	uint childrenCont=(uint)node.NumberOfChildren();
	for (uint children=0; children<childrenCont; children++)
	{
		INode *child=node.GetChildNode(children);
		addBoneTracks (animation, *child, parentName, animBuildCtx, false, ssBuilder);
	}
}

// --------------------------------------------------

void CExportNel::addLightTracks (NL3D::CAnimation& animation, INode& node, const char* parentName)
{
	CExportDesc desc;

	ObjectState os = node.EvalWorldState(0);
    Object *obj = os.obj;

	// Check if there is an object
	if (!obj)
		return;

	// Get a GenLight from the node
	if( ! (obj->SuperClassID()==LIGHT_CLASS_ID) )
		return;

	// Create a track name
	std::string name = CExportNel::getAnimatedLight (&node);
	name = "LightmapController." + name;

	int bAnimated = CExportNel::getScriptAppData (&node, NEL3D_APPDATA_LM_ANIMATED, 0);
	if( bAnimated )
	{
		Control *c = getControlerByName(node,"Color");
		if( c )
		{
			ITrack *pTrack=buildATrack (animation, *c, typeColor, node, desc, NULL);
			if (pTrack)
			{
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
					pTrack = NULL;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}
	}
}

// --------------------------------------------------

void CExportNel::addMorphTracks (NL3D::CAnimation& animation, INode& node, const char* parentName)
{
	CExportDesc desc;
	Modifier *pMorphMod = getModifier (&node, MAX_MORPHER_CLASS_ID);

	if (pMorphMod == NULL)
		return;

	uint32 i;

	for (i = 0; i < 100; ++i)
	{
		INode *pNode = (INode*)pMorphMod->GetReference (101+i);
		if (pNode == NULL)
			continue;
		std::string name = parentName;
		name += MCharStrToUtf8(pNode->GetName());
		name += "MorphFactor";
		
		IParamBlock *pb = (IParamBlock*)(pMorphMod->SubAnim (i+1));
		Control *c = pb->GetController (0);
		
		if (c != NULL)
		{
			ITrack *pTrack = buildATrack (animation, *c, typeFloat, node, desc, NULL);
			if (pTrack)
			{
				if (animation.getTrackByName (name.c_str()))
				{
					delete pTrack;
					pTrack = NULL;
				}
				else
				{
					animation.addTrack (name.c_str(), pTrack);
				}
			}
		}
	}
}


// --------------------------------------------------

void CExportNel::addObjTracks (CAnimation& animation, Object& obj, const char* parentName)
{
	// Export fov for camera
	CExportDesc desc;
	desc.reset();

	// Get the FOV controler
	Control *c=getControlerByName (obj, "FOV");
	if (c)
	{
		ITrack *pTrack=buildATrack (animation, *c, typeFloat, obj, desc, NULL);
		if (pTrack)
		{
			std::string name=parentName+std::string (CCamera::getFovValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}
}

// --------------------------------------------------

void CExportNel::addMtlTracks (CAnimation& animation, Mtl& mtl, const char* parentName)
{	

	// Material name
	std::string mtlName=std::string(parentName)+getName (mtl)+".";

	// Tmp track*
	ITrack *pTrack;

	// Tmp name
	std::string name;

	// Export desc
	CExportDesc desc;

	// *** Export ambient
	desc.reset();

	// Get a controller pointer
	Control* c=getControlerByName (mtl, "ambient");
	if (c)
	{
		// Build a track for this controller
		pTrack=buildATrack (animation, *c, typeColor, mtl, desc, NULL);
		if (pTrack)
		{
			// Add it in the animation
			name=mtlName+std::string (CAnimatedMaterial::getAmbientValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}

	// *** Export diffuse
	desc.reset();

	// Get a controller pointer
	c=getControlerByName (mtl, "diffuse");
	if (c)
	{
		// Build a track for this controller
		pTrack=buildATrack (animation, *c, typeColor, mtl, desc, NULL);
		if (pTrack)
		{
			// Add it in the animation
			name=mtlName+std::string (CAnimatedMaterial::getDiffuseValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}

	// *** Export specular
	desc.reset();
	desc.Specular=true;
	desc.Material=(StdMat2*)&mtl;

	// Get a controller pointer
	c=getControlerByName (mtl, "specular");
	if (c)
	{
		// Build a track for this controller
		pTrack=buildATrack (animation, *c, typeColor, mtl, desc, NULL);
		if (pTrack)
		{
			// Add it in the animation
			name=mtlName+std::string (CAnimatedMaterial::getSpecularValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}

	// *** Export emissive
	desc.reset();

	// Get a controller pointer
	c=getControlerByName (mtl, "selfIllumColor");
	if (c)
	{
		// Build a track for this controller
		pTrack=buildATrack (animation, *c, typeColor, mtl, desc, NULL);
		if (pTrack)
		{
			// Add it in the animation
			name=mtlName+std::string (CAnimatedMaterial::getEmissiveValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}
     
	// *** Export opacity
	desc.reset();

	// Get a controller pointer
	c=getControlerByName (mtl, "opacity");
	if (c)
	{
		// Build a track for this controller
		pTrack=buildATrack (animation, *c, typeFloat, mtl, desc, NULL);
		if (pTrack)
		{
			// Add it in the animation
			name=mtlName+std::string (CAnimatedMaterial::getOpacityValueName());
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}
	

	// Export sub materials tracks
	int s;
	int subMtl=mtl.NumSubMtls();
	for (s=0; s<subMtl; s++)
		addMtlTracks (animation, *mtl.GetSubMtl(s), parentName);

	// Export sub texmaps tracks
	/*subMtl=mtl.NumSubTexmaps();
	for (s=0; s<subMtl; s++)
		addTexTracks (animation, *mtl.GetSubTexmap(s), parentName);*/

	// *** export textures matrix animation if enabled

	/// test wether texture matrix animation should be exported
	int bExportTexMatAnim;
	CExportNel::getValueByNameUsingParamBlock2 (mtl, "bExportTextureMatrix", (ParamType2)TYPE_BOOL, &bExportTexMatAnim, 0);

	if (bExportTexMatAnim != 0)
	{
		for (uint i=0; i<IDRV_MAT_MAXTEXTURES; i++)
		{			
			// Get the enable flag name
			char enableSlotName[100];
			smprintf (enableSlotName, 100, "bEnableSlot_%d", i+1);

			// Get the enable flag 
			int bEnableSlot = 0;
			CExportNel::getValueByNameUsingParamBlock2 (mtl, enableSlotName, (ParamType2)TYPE_BOOL, &bEnableSlot, 0);
			if (bEnableSlot)
			{
				// Get the texture arg name
				char textureName[100];
				smprintf (textureName, 100, "tTexture_%d", i+1);

				// Get the texture pointer
				Texmap *pTexmap = NULL;
				CExportNel::getValueByNameUsingParamBlock2 (mtl, textureName, (ParamType2)TYPE_TEXMAP, &pTexmap, 0);
				if (pTexmap)
				{
					addTexTracks(animation, *pTexmap, i, mtlName.c_str());
				}
			}
		}
	}
}

// --------------------------------------------------

void CExportNel::addTexTracks (CAnimation& animation, Texmap& tex, uint stage, const char* parentName)
{
	// Texmap name
	

	// Tmp track*
	ITrack *pTrack;

	std::string name;

	CExportDesc desc;


	/// export the u translation
	// Get a controller pointer
	Control* c = getControlerByName (tex, "U Offset");
	if (c)
	{
		desc.reset();
		pTrack=buildATrack (animation, *c, typeFloat, tex, desc, NULL);
		if (pTrack)
		{
			name = parentName + std::string (CAnimatedMaterial::getTexMatUTransName(stage));
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}


	/// export the v translation
	// Get a controller pointer
                                    	c=getControlerByName (tex, "V Offset");
	if (c)
	{
		desc.reset();
		pTrack=buildATrack (animation, *c, typeFloat, tex, desc, NULL);
		if (pTrack)
		{
			name = parentName + std::string (CAnimatedMaterial::getTexMatVTransName(stage));
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}


	/// export the u scale
	// Get a controller pointer
	c = getControlerByName (tex, "U Tiling");
	if (c)
	{
		desc.reset();
		pTrack=buildATrack (animation, *c, typeFloat, tex, desc, NULL);
		if (pTrack)
		{
			name = parentName + std::string (CAnimatedMaterial::getTexMatUScaleName(stage));
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}


	/// export the v scale
	// Get a controller pointer
	c=getControlerByName (tex, "V Tiling");
	if (c)
	{
		desc.reset();
		pTrack=buildATrack (animation, *c, typeFloat, tex, desc, NULL);
		if (pTrack)
		{
			name = parentName + std::string (CAnimatedMaterial::getTexMatVScaleName(stage));
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}


	/// export the w rotation
	// Get a controller pointer
	c=getControlerByName (tex, "W Angle");
	if (c)
	{
		desc.reset();
		pTrack=buildATrack (animation, *c, typeFloat, tex, desc, NULL);
		if (pTrack)
		{
			name = parentName + std::string (CAnimatedMaterial::getTexMatWRotName(stage));
			if (animation.getTrackByName (name.c_str()))
			{
				delete pTrack;
				pTrack = NULL;
			}
			else
			{
				animation.addTrack (name.c_str(), pTrack);
			}
		}
	}
}



// --------------------------------------------------

// Build nel keys
void CExportNel::buildNelKey (CKeyFloat& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=maxKey.val;
}

// --------------------------------------------------

void CExportNel::buildNelKey (CKeyInt& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=(sint32)maxKey.val;
}

// --------------------------------------------------

void CExportNel::buildNelKey (CKeyBool& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=fabs(maxKey.val)<=FLOAT_EPSILON;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyVector& nelKey, ILinPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.x=maxKey.val.x;
	nelKey.Value.y=maxKey.val.y;
	nelKey.Value.z=maxKey.val.z;

	// Specular
	if (desc.Specular)
	{
		float shininess=desc.Material->GetShinStr(maxKey.time);
		nelKey.Value.x*=shininess;
		nelKey.Value.y*=shininess;
		nelKey.Value.z*=shininess;
	}
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyRGBA& nelKey, ILinPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.R=(uint8)maxKey.val.x;
	nelKey.Value.G=(uint8)maxKey.val.y;
	nelKey.Value.B=(uint8)maxKey.val.z;
	nelKey.Value.A=255;

	// Specular
	if (desc.Specular)
	{
		float shininess=desc.Material->GetShinStr(maxKey.time);
		clamp (shininess, 0.f, 1.f);
		nelKey.Value.modulateFromui (nelKey.Value, (uint8)(shininess*255.f));
	}
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyQuat& nelKey, ILinRotKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.w=-maxKey.val.w;
	nelKey.Value.x=maxKey.val.x;
	nelKey.Value.y=maxKey.val.y;
	nelKey.Value.z=maxKey.val.z;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyVector& nelKey, ILinScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	// Make a scale matrix
	Matrix3 srtm, stm, mat;
	maxKey.val.q.MakeMatrix(srtm);
	stm = ScaleMatrix(maxKey.val.s);
	mat = Inverse(srtm) * stm * srtm;

	// Get a NeL matrix
	CMatrix scaleMatrix;
	convertMatrix (scaleMatrix, mat);
	
	// Export it
	nelKey.Value.x=scaleMatrix.getI().x;
	nelKey.Value.y=scaleMatrix.getJ().y;
	nelKey.Value.z=scaleMatrix.getK().z;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBezierFloat& nelKey, IBezFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=maxKey.val;
	nelKey.InTan=maxKey.intan;
	nelKey.OutTan=maxKey.outtan;
	
	// Step mode ?
	if (GetOutTanType(maxKey.flags)==BEZKEY_STEP)
		nelKey.Step=true;
	else
		nelKey.Step=false;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBool& nelKey, IBezFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=fabs(maxKey.val)<=FLOAT_EPSILON;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBezierVector& nelKey, IBezPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.x=maxKey.val.x;
	nelKey.Value.y=maxKey.val.y;
	nelKey.Value.z=maxKey.val.z;
	nelKey.InTan.x=ticksPerSecond*maxKey.intan.x;
	nelKey.InTan.y=ticksPerSecond*maxKey.intan.y;
	nelKey.InTan.z=ticksPerSecond*maxKey.intan.z;
	nelKey.OutTan.x=ticksPerSecond*maxKey.outtan.x;
	nelKey.OutTan.y=ticksPerSecond*maxKey.outtan.y;
	nelKey.OutTan.z=ticksPerSecond*maxKey.outtan.z;
	
	// Step mode ?
	if (GetOutTanType(maxKey.flags)==BEZKEY_STEP)
		nelKey.Step=true;
	else
		nelKey.Step=false;

	// Specular
	if (desc.Specular)
	{
		float shininess=desc.Material->GetShinStr(maxKey.time);
		nelKey.Value.x*=shininess;
		nelKey.Value.y*=shininess;
		nelKey.Value.z*=shininess;
	}
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBezierQuat& nelKey, IBezQuatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.w=-maxKey.val.w;
	nelKey.Value.x=maxKey.val.x;
	nelKey.Value.y=maxKey.val.y;
	nelKey.Value.z=maxKey.val.z;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBezierVector& nelKey, IBezScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	// Make a scale matrix
	Matrix3 srtm, stm, mat;
	maxKey.val.q.MakeMatrix(srtm);
	stm = ScaleMatrix(maxKey.val.s);
	mat = Inverse(srtm) * stm * srtm;

	// Get a NeL matrix
	CMatrix scaleMatrix;
	convertMatrix (scaleMatrix, mat);

	// Export it
	nelKey.Value.x=scaleMatrix.getI().x;
	nelKey.Value.y=scaleMatrix.getJ().y;
	nelKey.Value.z=scaleMatrix.getK().z;
	nelKey.InTan.x=maxKey.intan.x;
	nelKey.InTan.y=maxKey.intan.y;
	nelKey.InTan.z=maxKey.intan.z;
	nelKey.OutTan.x=maxKey.outtan.x;
	nelKey.OutTan.y=maxKey.outtan.y;
	nelKey.OutTan.z=maxKey.outtan.z;
	
	// Step mode ?
	if (GetOutTanType(maxKey.flags)==BEZKEY_STEP)
		nelKey.Step=true;
	else
		nelKey.Step=false;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyTCBFloat& nelKey, ITCBFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=maxKey.val;
	nelKey.Tension=maxKey.tens;
	nelKey.Continuity=maxKey.cont;
	nelKey.Bias=maxKey.bias;
	nelKey.EaseTo=maxKey.easeIn;
	nelKey.EaseFrom=maxKey.easeOut;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyBool& nelKey, ITCBFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value=fabs(maxKey.val)<=FLOAT_EPSILON;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyTCBVector& nelKey, ITCBPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.x=maxKey.val.x;
	nelKey.Value.y=maxKey.val.y;
	nelKey.Value.z=maxKey.val.z;
	nelKey.Tension=maxKey.tens;
	nelKey.Continuity=maxKey.cont;
	nelKey.Bias=maxKey.bias;
	nelKey.EaseTo=maxKey.easeIn;
	nelKey.EaseFrom=maxKey.easeOut;

	// Specular
	if (desc.Specular)
	{
		float shininess=desc.Material->GetShinStr(maxKey.time);
		nelKey.Value.x*=shininess;
		nelKey.Value.y*=shininess;
		nelKey.Value.z*=shininess;
	}
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyTCBQuat& nelKey, ITCBRotKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	nelKey.Value.Angle=-maxKey.val.angle;
	nelKey.Value.Axis.x=maxKey.val.axis.x;
	nelKey.Value.Axis.y=maxKey.val.axis.y;
	nelKey.Value.Axis.z=maxKey.val.axis.z;
	nelKey.Tension=maxKey.tens;
	nelKey.Continuity=maxKey.cont;
	nelKey.Bias=maxKey.bias;
	nelKey.EaseTo=maxKey.easeIn;
	nelKey.EaseFrom=maxKey.easeOut;
}

// --------------------------------------------------

void CExportNel::buildNelKey (NL3D::CKeyTCBVector& nelKey, ITCBScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c)
{
	// Make a scale matrix
	Matrix3 srtm, stm, mat;
	maxKey.val.q.MakeMatrix(srtm);
	stm = ScaleMatrix(maxKey.val.s);
	mat = Inverse(srtm) * stm * srtm;

	// Get a NeL matrix
	CMatrix scaleMatrix;
	convertMatrix (scaleMatrix, mat);
	
	// Export it
	nelKey.Value.x=scaleMatrix.getI().x;
	nelKey.Value.y=scaleMatrix.getJ().y;
	nelKey.Value.z=scaleMatrix.getK().z;
	nelKey.Tension=maxKey.tens;
	nelKey.Continuity=maxKey.cont;
	nelKey.Bias=maxKey.bias;
	nelKey.EaseTo=maxKey.easeIn;
	nelKey.EaseFrom=maxKey.easeOut;
}

//--------------------------------------------------------


// Build a Nel bool track from a On/Off max Controller (doesn't work with buildATRack, which require a keyframer interface
// , which isn't provided by an on / off controller)
 NL3D::CTrackKeyFramerConstBool*			CExportNel::buildOnOffTrack(Control& c)
 {
	// make sure this is the controler we want
//	nlassert(c.ClassID() == Class_ID(BOOL_CONTROL_CLASS_ID, 0));	
	float value = 0.f;
	
	CTrackKeyFramerConstBool *track = new CTrackKeyFramerConstBool;

	// ** Get the range of the controler
	Interval range=c.GetTimeRange (TIMERANGE_ALL);

	// ** Get the out of range type
	int oRT=c.GetORT (ORT_AFTER);

	// Set the range
	if ((!(FOREVER==range))&&(!(NEVER==range)))
		track->unlockRange (CExportNel::convertTime (range.Start()), CExportNel::convertTime (range.End()));

	// Set the out of range type
	switch (oRT)
	{
	case ORT_LOOP:
		track->setLoopMode(true);
		break;
	case ORT_CONSTANT:
	case ORT_CYCLE:
	default:
		track->setLoopMode(false);
		break;
	}

	// Enum the keys
	int numKeys = c.NumKeys();
	float firstKey;
	float lastKey;
	for (int i=0; i<numKeys; i++) 
	{		
		// First key ?
		if (i==0)
			firstKey = convertTime (c.GetKeyTime(i));

		// Last key ?
		lastKey = convertTime (c.GetKeyTime(i));

		// Allocate the key
		CKey<bool> nelKey;		
		c.GetValue(c.GetKeyTime(i), &value, range, CTRL_ABSOLUTE);
		nelKey.Value = value > 0.f;
		track->addKey(nelKey, convertTime (c.GetKeyTime(i)));		
	}

	// Invalid interval ? Take the interval of the keyfarmer
	if ((FOREVER==range)||(NEVER==range))
		track->unlockRange (firstKey, lastKey);

	
	return track;
 }



// --------------------------------------------------

// Create a keyframer
template<class TTracker, class TKey, class TMaxKey>
ITrack* createKeyFramer (IKeyControl *ikeys, TTracker*, TKey*, TMaxKey*, float ticksPerSecond, const Interval& range, int oRT, 
						 const CExportDesc& desc, Control &c)
{
	// Allocate the tracker
	TTracker *pLinTrack=new TTracker;

	// Set the range
	if ((!(FOREVER==range))&&(!(NEVER==range)))
		pLinTrack->unlockRange (CExportNel::convertTime (range.Start()), CExportNel::convertTime (range.End()));

	// Set the out of range type
	switch (oRT)
	{
	case ORT_LOOP:
		pLinTrack->setLoopMode(true);
		break;
	case ORT_CONSTANT:
	case ORT_CYCLE:
	default:
		pLinTrack->setLoopMode(false);
		break;
	}

	// Enum the keys
	TMaxKey key;
	int numKeys = ikeys->GetNumKeys();
	float firstKey;
	float lastKey;
	for (int i=0; i<numKeys; i++) 
	{
		// Get the key
		ikeys->GetKey(i, &key);

		// First key ?
		if (i==0)
			firstKey=CExportNel::convertTime (key.time);

		// Last key ?
		lastKey=CExportNel::convertTime (key.time);

		// Allocate the key
		TKey nelKey;

		// Build the key
		CExportNel::buildNelKey (nelKey, key, ticksPerSecond, desc, c);

		// Add the good key
		pLinTrack->addKey (nelKey, CExportNel::convertTime (key.time));
	}

	// Invalid interval ? Take the inteval of the keyfarmer
	if ((FOREVER==range)||(NEVER==range))
		pLinTrack->unlockRange (firstKey, lastKey);


	// Return the keyframer
	return pLinTrack;
}

// --------------------------------------------------

class CDoomyKey : public IKey
{
	char toto[2048];
};

// --------------------------------------------------

// Create the matrix tracks
void CExportNel::createBipedKeyFramer (ITrack *&nelRot, ITrack *&nelPos, bool isRot, bool isPos, float ticksPerSecond, 
									   const Interval& range, int oRT, const CExportDesc& desc, INode& node, 
									   CAnimationBuildCtx	*animBuildCtx)
{
	/*
		Yoyo:
		The New version of this exporter over sample All frame along the animation.
		A CTrackSampleQuat/CTrackSampleVector will be used late in process (at compression time).
	*/

	nelRot=NULL;
	nelPos=NULL;

	// Type of the track
	typedef CTrackKeyFramerLinearVector	TMPos;
	typedef CTrackKeyFramerLinearQuat	TMRot;
	typedef CKeyVector					TMPosKey;
	typedef CKeyQuat					TMRotKey;

	// Find the Biped node info in the build ctx.
	nlassert(animBuildCtx);
	CAnimationBuildCtx::CBipedNode	*bipedNodeInfo= animBuildCtx->getBipedNodeInfo(&node);
	nlassert(bipedNodeInfo);

	// Number of keys
	uint numKeys = bipedNodeInfo->Keys.size();

	// No null keys ?
	if (numKeys!=0)
	{
		// Allocate the keyframer
		if(isRot)			
			nelRot=new TMRot;
		if (isPos)
			nelPos=new TMPos;

		// Set the range
		if ((!(FOREVER==range))&&(!(NEVER==range)))
		{
			if (isRot)
				((TMRot*)nelRot)->unlockRange (CExportNel::convertTime (range.Start()), CExportNel::convertTime (range.End()));
			if (isPos)
				((TMPos*)nelPos)->unlockRange (CExportNel::convertTime (range.Start()), CExportNel::convertTime (range.End()));
		}

		// Set the out of range type
		switch (oRT)
		{
		case ORT_LOOP:
			if (isRot)
				((TMRot*)nelRot)->setLoopMode(true);
			if (isPos)
				((TMPos*)nelPos)->setLoopMode(true);
			break;
		case ORT_CONSTANT:
		case ORT_CYCLE:
		default:
			if (isRot)
				((TMRot*)nelRot)->setLoopMode(false);
			if (isPos)
				((TMPos*)nelPos)->setLoopMode(false);
			break;
		}

		// Enum the keys
		float firstKey;
		float lastKey;
		CQuat previous;
		for(uint key=0; key<numKeys; key++)
		{
			CAnimationBuildCtx::CBipedKey	&bipedKey= bipedNodeInfo->Keys[key];

			// First key ?
			if (key==0)
				firstKey=convertTime (bipedKey.Time);

			// Last key ?
			lastKey=convertTime (bipedKey.Time);

			// Allocate the key
			TMRotKey		theRot;
			TMPosKey		thePos;
	
			// Get Pos/Rot/Scale from overSampled values in the animBuildCtx
			CQuat tmpQuat= bipedKey.Quat;
			thePos.Value= bipedKey.Pos;
			tmpQuat.normalize();

			// Make closest with previous
			if (key>0)
			{
				tmpQuat.makeClosest (previous);
			}

			// Set previous
			previous=tmpQuat;

			// Copy to key
			theRot.Value= tmpQuat;

			// Add the good keys
			if (isRot)
				((TMRot*)nelRot)->addKey (theRot, convertTime (bipedKey.Time));
			if (isPos)
				((TMPos*)nelPos)->addKey (thePos, convertTime (bipedKey.Time));
		}

		// Invalid interval ? Take the inteval of the keyfarmer
		if ((FOREVER==range)||(NEVER==range))
		{
			if (isRot)
				((TMRot*)nelRot)->unlockRange (firstKey, lastKey);
			if (isPos)
				((TMPos*)nelPos)->unlockRange (firstKey, lastKey);
		}
	}
}

// --------------------------------------------------

NL3D::TAnimationTime	CExportNel::convertTime (TimeValue time)
{
	return (float)time/((float)GetTicksPerFrame()*(float)GetFrameRate());
}

// --------------------------------------------------

// Build a NeL track with a 3dsmax controler.
ITrack* CExportNel::buildATrack (CAnimation& animation, Control& c, TNelValueType type, Animatable& node, const CExportDesc& desc,
								 CAnimationBuildCtx	*animBuildCtx, bool bodyBiped)
{
	// What kind of controler?

	/*
	 * Those controlers are supported:
	 *
	 * LININTERP_FLOAT_CLASS_ID
	 * LININTERP_POSITION_CLASS_ID
	 * LININTERP_ROTATION_CLASS_ID
	 * LININTERP_SCALE_CLASS_ID
	 * HYBRIDINTERP_FLOAT_CLASS_ID
	 * HYBRIDINTERP_POSITION_CLASS_ID
	 * HYBRIDINTERP_ROTATION_CLASS_ID
	 * HYBRIDINTERP_POINT3_CLASS_ID
	 * HYBRIDINTERP_SCALE_CLASS_ID
	 * TCBINTERP_FLOAT_CLASS_ID
	 * TCBINTERP_POSITION_CLASS_ID
	 * TCBINTERP_ROTATION_CLASS_ID
	 * TCBINTERP_POINT3_CLASS_ID
	 * TCBINTERP_SCALE_CLASS_ID
	 * BIPSLAVE_CONTROL_CLASS_ID
	 * BIPBODY_CONTROL_CLASS_ID
	 *
	 * Thoses controlers are not supported:
	 *
	 * LOOKAT_CONTROL_CLASS_ID
	 * PATH_CONTROL_CLASS_ID
	 * EXPR_POS_CONTROL_CLASS_ID
	 * EXPR_P3_CONTROL_CLASS_ID
	 * EXPR_FLOAT_CONTROL_CLASS_ID
	 * EXPR_SCALE_CONTROL_CLASS_ID
	 * EXPR_ROT_CONTROL_CLASS_ID
	 * SURF_CONTROL_CLASSID
	 * LINKCTRL_CLASSID
	 *
	 * May be supported in futur:
	 *
	 * EULER_CONTROL_CLASS_ID
	 * FLOATNOISE_CONTROL_CLASS_ID
	 * POSITIONNOISE_CONTROL_CLASS_ID
	 * POINT3NOISE_CONTROL_CLASS_ID
	 * ROTATIONNOISE_CONTROL_CLASS_ID
	 * SCALENOISE_CONTROL_CLASS_ID
	 * HYBRIDINTERP_COLOR_CLASS_ID
	 * FOOTPRINT_CLASS_ID
	 */

	ITrack* pTrack=NULL;

	// ** Get the 3dsmax key control.
	IKeyControl *ikeys=GetKeyControlInterface ((&c));

	// ** Get number of ticks per second
	float ticksPerSecond=(float)TIME_TICKSPERSEC;

	// ** Get the range of the controler
	Interval range=c.GetTimeRange (TIMERANGE_ALL);

	// ** Get the out of range type
	int oRT=c.GetORT (ORT_AFTER);

	// ** Check all keyframer if it is one.
	if (ikeys)
	{
		if (ikeys->GetNumKeys()>0)
		{
			// ** Linear nel keyframe

			if (c.ClassID()==Class_ID(LININTERP_FLOAT_CLASS_ID,0))
			{
				// For float
				if (type==typeFloat)
					pTrack=createKeyFramer<CTrackKeyFramerLinearFloat, CKeyFloat, ILinFloatKey> (ikeys,
						(CTrackKeyFramerLinearFloat*)NULL, (CKeyFloat*)NULL, (ILinFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For int
				if (type==typeInt)
					pTrack=createKeyFramer<CTrackKeyFramerLinearInt, CKeyInt, ILinFloatKey> (ikeys,
						(CTrackKeyFramerLinearInt*)NULL, (CKeyInt*)NULL, (ILinFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For bool
				if (type==typeBoolean)
					pTrack=createKeyFramer<CTrackKeyFramerConstBool, CKeyBool, ILinFloatKey> (ikeys,
						(CTrackKeyFramerConstBool*)NULL, (CKeyBool*)NULL, (ILinFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(LININTERP_POSITION_CLASS_ID,0))
			{
				// For vector
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerLinearVector, CKeyVector, ILinPoint3Key> (ikeys,
						(CTrackKeyFramerLinearVector*)NULL, (CKeyVector*)NULL, (ILinPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For color
				if (type==typeColor)
					pTrack=createKeyFramer<CTrackKeyFramerLinearRGBA, CKeyRGBA, ILinPoint3Key> (ikeys,
						(CTrackKeyFramerLinearRGBA*)NULL, (CKeyRGBA*)NULL, (ILinPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(LININTERP_ROTATION_CLASS_ID,0))
			{
				// For quaternion
				if (type==typeRotation)
					pTrack=createKeyFramer<CTrackKeyFramerLinearQuat, CKeyQuat, ILinRotKey> (ikeys,
						(CTrackKeyFramerLinearQuat*)NULL, (CKeyQuat*)NULL, (ILinRotKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(LININTERP_SCALE_CLASS_ID,0))
			{
				// For scale
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerLinearVector, CKeyVector, ILinScaleKey> (ikeys,
						(CTrackKeyFramerLinearVector*)NULL, (CKeyVector*)NULL, (ILinScaleKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}

			// ** Bezier

			if (c.ClassID()==Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID,0))
			{
				// For float
				if (type==typeFloat)
					pTrack=createKeyFramer<CTrackKeyFramerBezierFloat, CKeyBezierFloat, IBezFloatKey> (ikeys,
						(CTrackKeyFramerBezierFloat*)NULL, (CKeyBezierFloat*)NULL, (IBezFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For int
				if (type==typeInt)
					pTrack=createKeyFramer<CTrackKeyFramerBezierInt, CKeyBezierFloat, IBezFloatKey> (ikeys,
						(CTrackKeyFramerBezierInt*)NULL, (CKeyBezierFloat*)NULL, (IBezFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For bool
				if (type==typeBoolean)
					pTrack=createKeyFramer<CTrackKeyFramerConstBool, CKeyBool, IBezFloatKey> (ikeys,
						(CTrackKeyFramerConstBool*)NULL, (CKeyBool*)NULL, (IBezFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if ((c.ClassID()==Class_ID(HYBRIDINTERP_POSITION_CLASS_ID,0))||
				(c.ClassID()==Class_ID(HYBRIDINTERP_POINT3_CLASS_ID,0)))
			{
				// For vector
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerBezierVector, CKeyBezierVector, IBezPoint3Key> (ikeys,
						(CTrackKeyFramerBezierVector*)NULL, (CKeyBezierVector*)NULL, (IBezPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For color
				if (type==typeColor)
					pTrack=createKeyFramer<CTrackKeyFramerBezierRGBA, CKeyBezierVector, IBezPoint3Key> (ikeys,
						(CTrackKeyFramerBezierRGBA*)NULL, (CKeyBezierVector*)NULL, (IBezPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID,0))
			{
				// For quaternion
				if (type==typeRotation)
					pTrack=createKeyFramer<CTrackKeyFramerBezierQuat, CKeyBezierQuat, IBezQuatKey> (ikeys,
						(CTrackKeyFramerBezierQuat*)NULL, (CKeyBezierQuat*)NULL, (IBezQuatKey *)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(HYBRIDINTERP_SCALE_CLASS_ID,0))
			{
				// For scale
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerBezierVector, CKeyBezierVector, IBezScaleKey> (ikeys,
						(CTrackKeyFramerBezierVector*)NULL, (CKeyBezierVector*)NULL, (IBezScaleKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(HYBRIDINTERP_COLOR_CLASS_ID,0))
			{
			}

			// ** TCB

			if (c.ClassID()==Class_ID(TCBINTERP_FLOAT_CLASS_ID,0))
			{
				// For float
				if (type==typeFloat)
					pTrack=createKeyFramer<CTrackKeyFramerTCBFloat, CKeyTCBFloat, ITCBFloatKey> (ikeys,
						(CTrackKeyFramerTCBFloat*)NULL, (CKeyTCBFloat*)NULL, (ITCBFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For int
				if (type==typeInt)
					pTrack=createKeyFramer<CTrackKeyFramerTCBInt, CKeyTCBFloat, ITCBFloatKey> (ikeys,
						(CTrackKeyFramerTCBInt*)NULL, (CKeyTCBFloat*)NULL, (ITCBFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For bool
				if (type==typeBoolean)
					pTrack=createKeyFramer<CTrackKeyFramerConstBool, CKeyBool, ITCBFloatKey> (ikeys,
						(CTrackKeyFramerConstBool*)NULL, (CKeyBool*)NULL, (ITCBFloatKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if ((c.ClassID()==Class_ID(TCBINTERP_POSITION_CLASS_ID,0))||
				(c.ClassID()==Class_ID(TCBINTERP_POINT3_CLASS_ID,0)))
			{
				// For vector
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerTCBVector, CKeyTCBVector, ITCBPoint3Key> (ikeys,
						(CTrackKeyFramerTCBVector*)NULL, (CKeyTCBVector*)NULL, (ITCBPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);

				// For color
				if (type==typeColor)
					pTrack=createKeyFramer<CTrackKeyFramerTCBRGBA, CKeyTCBVector, ITCBPoint3Key> (ikeys,
						(CTrackKeyFramerTCBRGBA*)NULL, (CKeyTCBVector*)NULL, (ITCBPoint3Key*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(TCBINTERP_ROTATION_CLASS_ID,0))
			{
				// For quaternion
				if (type==typeRotation)
					pTrack=createKeyFramer<CTrackKeyFramerTCBQuat, CKeyTCBQuat, ITCBRotKey> (ikeys,
						(CTrackKeyFramerTCBQuat*)NULL, (CKeyTCBQuat*)NULL, (ITCBRotKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}
			if (c.ClassID()==Class_ID(TCBINTERP_SCALE_CLASS_ID,0))
			{
				// For scale
				if ((type==typePos)||(type==typeScale))
					pTrack=createKeyFramer<CTrackKeyFramerTCBVector, CKeyTCBVector, ITCBScaleKey> (ikeys,
						(CTrackKeyFramerTCBVector*)NULL, (CKeyTCBVector*)NULL, (ITCBScaleKey*)NULL, ticksPerSecond, range, oRT, desc, c);
			}			
		}
	}

	// No keyframer controler
	if ((c.ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
		bodyBiped)
	{
		NL3D::ITrack	*doomy;

		// For rotation
		if (type==typeRotation)
		{
			if (animBuildCtx)
			{
				createBipedKeyFramer (pTrack, doomy, true, false, ticksPerSecond, range, oRT, desc, *(INode*)&node, 
									animBuildCtx);
			}
		}
	}

	//if a biped body, or if must export his position track (clavicle...)
	if (bodyBiped || ( animBuildCtx && animBuildCtx->mustExportBipedBonePos((INode*)&node) ) )
	{
		NL3D::ITrack	*doomy;

		// For position
		if (type==typePos)
		{
			// 
			// Create key set
			if (animBuildCtx)
			{
				createBipedKeyFramer (doomy, pTrack, false, true, ticksPerSecond, range, oRT, desc, *(INode*)&node, 
										animBuildCtx);
			}
			else
			{
				MessageBox (NULL, _T("Warning: no pos track exported!"), _T("Tmp NEL"), MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}
	
	// Return the track
	return pTrack;
}

// --------------------------------------------------

bool CExportNel::getBipedKeyInfo (const char* nodeName, const char* paramName, uint key, float& res)
{
	// Make the script sequence
	char script[512];
	_snprintf (script, 512, "(biped.getKey $'%s'.controller %d).%s", nodeName, key+1, paramName);

	// Call the script
	return scriptEvaluate (script, &res, scriptFloat);
}

// --------------------------------------------------

bool CExportNel::getBipedInplaceMode (const char* nodeName, const char* inplaceFunction, bool &res)
{
	// Make the script sequence
	char script[512];
	_snprintf (script, 512, "$'%s'.controller.%s", nodeName, inplaceFunction);

	// Call the script
	return scriptEvaluate (script, &res, scriptBool);
}

// --------------------------------------------------

bool CExportNel::setBipedInplaceMode (const char* nodeName, const char* inplaceFunction, bool onOff)
{
	// Make the script sequence
	char script[512];
	_snprintf (script, 512, "$'%s'.controller.%s = %s", nodeName, inplaceFunction, onOff ? "true" : "false");

	// Call the script
	return scriptEvaluate (script, NULL, scriptNothing);
}

// --------------------------------------------------



// ***************************************************************************
// ***************************************************************************
// Biped Oversampling.
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void				CExportNel::buildBipedInformation(CAnimationBuildCtx &animBuildCtx, INode &node)
{
	// Parse this node and his children

	// Get the TM controler
	Control *transform=node.GetTMController();

	// Is it a biped node ?
	if ( transform && 
		(transform->ClassID() == BIPBODY_CONTROL_CLASS_ID || transform->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) 
		)
	{
		// Yes, Add the node
		//===============
		CAnimationBuildCtx::CBipedNode	bipNode;
		bipNode.Node= &node;

		// add it to the array.
		animBuildCtx.BipedNodes.push_back(bipNode);


		// Update Ctx anim range
		//===============
		// Get the 3dsmax key control.
		IKeyControl *ikeys=GetKeyControlInterface (transform);

		// If has keys
		if(ikeys && ikeys->GetNumKeys()>0)
		{
			CDoomyKey key;

			// Get the first key
			ikeys->GetKey(0, &key);
			animBuildCtx.BipedRangeMin= std::min(animBuildCtx.BipedRangeMin, key.time);

			// Get the last key
			ikeys->GetKey(ikeys->GetNumKeys()-1, &key);
			animBuildCtx.BipedRangeMax= std::max(animBuildCtx.BipedRangeMax, key.time);
		}
	}

	// Parse children
	uint childrenCount=(uint)node.NumberOfChildren();
	for (uint children=0; children<childrenCount; children++)
	{
		INode *child=node.GetChildNode(children);
		buildBipedInformation (animBuildCtx, *child);
	}

}

// ***************************************************************************
void				CExportNel::overSampleBipedAnimation(CAnimationBuildCtx &animBuildCtx, uint overSampleValue)
{
	uint	i;

	// Prepare Biped bones for animation.
	//=========================

	// Bkup every bone which need to be reseted.
	std::vector<CBipedNodePlaceMode>	nodeToResetPlaceMode;

	// For all biped nodes.
	for(i=0; i<animBuildCtx.BipedNodes.size(); i++)
	{
		INode	*node= animBuildCtx.BipedNodes[i].Node;
		// Get the matrix controler
		Control *transform= node->GetTMController();
		nlassert (transform);
		nlassert (	(transform->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
					(transform->ClassID() == BIPBODY_CONTROL_CLASS_ID) );

		// bkup/reset InPlaceMode.
		if(transform->ClassID() == BIPBODY_CONTROL_CLASS_ID)
		{
			// Backup inplace mode
			CBipedNodePlaceMode		pm;
			pm.Node= node;
			// Backup inplace mode states
			bool res = getBipedInplaceMode (getName (*node).c_str(), "inPlaceYMode", pm.InPlaceYMode);
			nlassert (res);
			res = getBipedInplaceMode (getName (*node).c_str(), "inPlaceXMode", pm.InPlaceXMode);
			nlassert (res);
			res = getBipedInplaceMode (getName (*node).c_str(), "inPlaceMode", pm.InPlaceMode);
			nlassert (res);

			// If the node is in place mode
			if( pm.InPlaceYMode || pm.InPlaceXMode || pm.InPlaceMode )
			{
				// add to the array for restore
				nodeToResetPlaceMode.push_back(pm);

				// No inplace mode
				setBipedInplaceMode (getName (*node).c_str(), "inPlaceMode", false);
				setBipedInplaceMode (getName (*node).c_str(), "inPlaceXMode", false);
				setBipedInplaceMode (getName (*node).c_str(), "inPlaceYMode", false);
			}
		}

		//Get the Biped Export Interface from the controller 
		IBipedExport *BipIface = (IBipedExport *) transform->GetInterface(I_BIPINTERFACE);
		nlassert (BipIface);

		// Remove the non uniform scale
		BipIface->RemoveNonUniformScale(1);
	}

	// Oversample
	//=========================

	// Step for interpolation
	int interpolationStep=(overSampleValue*GetTicksPerFrame())/GetFrameRate();

	// get the number of keys to create. (ceil() and +1)
	uint	numKeys= 1 + (interpolationStep-1 + animBuildCtx.BipedRangeMax - animBuildCtx.BipedRangeMin) / interpolationStep;
	// Reserve enough space for each Biped key.
	for(i=0;i<animBuildCtx.BipedNodes.size();i++)
	{
		animBuildCtx.BipedNodes[i].Keys.reserve(numKeys);
	}

	// For each bone, true if must export his Pos track.
	std::vector<bool>	bpExportPos;
	bpExportPos.resize(animBuildCtx.BipedNodes.size());
	for(i=0;i<animBuildCtx.BipedNodes.size();i++)
	{
		bpExportPos[i]= animBuildCtx.mustExportBipedBonePos(animBuildCtx.BipedNodes[i].Node);
	}

	// For all time Values usefull for the Biped.
	TimeValue	time;
	for(time= animBuildCtx.BipedRangeMin; time<=animBuildCtx.BipedRangeMax; time+= interpolationStep)
	{
		time= std::min(animBuildCtx.BipedRangeMax, time);

		/* Do this in this way, so the Biped will be evaluated in Max only once per frame.
		*/

		// For all biped node, get their LocalMatrix.
		for(i=0;i<animBuildCtx.BipedNodes.size();i++)
		{
			CAnimationBuildCtx::CBipedNode		&bpNode= animBuildCtx.BipedNodes[i];

			// Build the key
			CAnimationBuildCtx::CBipedKey		bpKey;
			bpKey.Time= time;

			// For faster interpolation, use simpler getLocalMatrix when possible
			if( !bpExportPos[i] )
			{
				// can use it because RotQuat is the same with this method or with getNELBoneLocalTM()
				Matrix3 localTM (TRUE);
				getLocalMatrix (localTM, *bpNode.Node, time);
				// NB: with this method the NEL local pos and local scale are false, but doesn't matter since not used...
				decompMatrix (bpKey.Scale, bpKey.Quat, bpKey.Pos, localTM);
				bpKey.Quat.normalize();
			}
			else
			{
				// Get the NEL local TM, with correct position
				getNELBoneLocalTM(*bpNode.Node, time, bpKey.Scale, bpKey.Quat, bpKey.Pos);
			}

			// Add the key.
			bpNode.Keys.push_back(bpKey);
		}
	}

	// Reset Biped bones 
	//=========================

	// For all biped nodes.
	for(i=0; i<animBuildCtx.BipedNodes.size(); i++)
	{
		INode	*node= animBuildCtx.BipedNodes[i].Node;
		// Get the matrix controler
		Control *transform= node->GetTMController();
		nlassert (transform);
		nlassert (	(transform->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
					(transform->ClassID() == BIPBODY_CONTROL_CLASS_ID));

		//Get the Biped Export Interface from the controller 
		IBipedExport *BipIface = (IBipedExport *) transform->GetInterface(I_BIPINTERFACE);
		nlassert (BipIface);

		// Restore the non uniform scale
		BipIface->RemoveNonUniformScale(0);
	}

	// restore InPlaceMode for nodes which have been reset
	for(i=0; i<nodeToResetPlaceMode.size(); i++)
	{
		CBipedNodePlaceMode		&pm= nodeToResetPlaceMode[i];
		INode					*node= pm.Node;

		setBipedInplaceMode (getName (*node).c_str(), "inPlaceYMode", pm.InPlaceYMode);
		setBipedInplaceMode (getName (*node).c_str(), "inPlaceXMode", pm.InPlaceXMode);
		setBipedInplaceMode (getName (*node).c_str(), "inPlaceMode", pm.InPlaceMode);
	}


	// TestYoyo. To see what Pos are animated. Strange biped stuff
	//=========================
	// NB: 	for all node, you must _ExportBipedBonePosSet.insert(node) in compileBiped() to test it correctly
	/*for(i=0; i<animBuildCtx.BipedNodes.size(); i++)
	{
		CAnimationBuildCtx::CBipedNode		&bpNode= animBuildCtx.BipedNodes[i];
		if(bpNode.Keys.size()==0)
			return;

		// Test all track pos
		CVector		posRef= bpNode.Keys[0].Pos;
		for(uint j=1;j<bpNode.Keys.size();j++)
		{
			if( (bpNode.Keys[j].Pos-posRef).norm()>0.001 )
			{
				std::string	name= getName(*bpNode.Node);
				nlinfo("BipedNode %s has a Pos animation", name.c_str());
				break;
			}
		}
	}*/

}


// ***************************************************************************
CExportNel::CAnimationBuildCtx::CAnimationBuildCtx()
{
	BipedRangeMin= INT_MAX;
	BipedRangeMax= 0;
}

// ***************************************************************************
void				CExportNel::CAnimationBuildCtx::compileBiped()
{
	uint	i;

	// Build _BipedMap
	_BipedMap.clear();
	for(i=0;i<BipedNodes.size();i++)
	{
		// Add entry in map.
		_BipedMap[BipedNodes[i].Node]= i;
	}

	// Build set of bones to Export Pos Track.
	_ExportBipedBonePosSet.clear();
	for(i=0;i<BipedNodes.size();i++)
	{
		INode	*node= BipedNodes[i].Node;

		// Get the matrix controler
		Control *transform= node->GetTMController();
		nlassert (transform);
		// If this is node is a Biped controller.
		if( transform->ClassID() == BIPBODY_CONTROL_CLASS_ID )
		{
			// Add him to the set.
			_ExportBipedBonePosSet.insert(node);

			// Must add some important limbs of this biped too (Biped stuff => sometimes pos are animated).
			addLimbNodeToExportPos(node, "#larm");
			addLimbNodeToExportPos(node, "#rarm");
			addLimbNodeToExportPos(node, "#spine");
			addLimbNodeToExportPos(node, "#tail");
		}
	}
}


// ***************************************************************************
void				CExportNel::CAnimationBuildCtx::addLimbNodeToExportPos(INode *rootNode, const char *limbId)
{
	char script[512];
	INode		*limbNode= NULL;

	// evaluate script.
	_snprintf (script, 512, "biped.getNode $'%s' %s", CExportNel::getName(*rootNode).c_str(), limbId);
	// If script is ok.
	if( CExportNel::scriptEvaluate (script, &limbNode, scriptNode) )
	{
		// add to the array if success
		if(limbNode)
			_ExportBipedBonePosSet.insert(limbNode);
	}
}


// ***************************************************************************
CExportNel::CAnimationBuildCtx::CBipedNode		*CExportNel::CAnimationBuildCtx::getBipedNodeInfo(INode *node)
{
	TBipedMap::const_iterator	it= _BipedMap.find(node);
	if(it==_BipedMap.end())
		return NULL;
	else
		return &BipedNodes[it->second];
}


// ***************************************************************************
bool				CExportNel::CAnimationBuildCtx::mustExportBipedBonePos(INode *node)
{
	// true if found in the set.
	return (_ExportBipedBonePosSet.find(node)!=_ExportBipedBonePosSet.end());
}


// ***************************************************************************
struct CSpawnCmd 
{
	std::string		Cmd;
	std::string		Shape;
	std::string		Bone;

	bool	operator<(const CSpawnCmd &o) const
	{
		if(Shape!=o.Shape)
			return Shape<o.Shape;
		if(Cmd!=o.Cmd)
			return Cmd<o.Cmd;
		return Bone<o.Bone;
	}
};
struct CSpawnObject
{
	uint	FinalId;
	bool	Spawned;

	CSpawnObject()
	{
		FinalId= 0;
		Spawned= false;
	}
};


static void commitSSSKey(NL3D::TAnimationTime keyTime, std::map<CSpawnCmd, CSpawnObject> &objects, CTrackKeyFramerConstString *finalTrack, bool &insertedAt0)
{
	CKeyString		keyValue;
	
	// The final script is "state script" oriented
	// ie, generate a line for each spawned object, not for each command!
	std::map<CSpawnCmd, CSpawnObject>::iterator		it;
	for(it= objects.begin();it!=objects.end();it++)
	{
		// if the object is spawn, add a line
		if(it->second.Spawned)
		{
			if(it->first.Cmd=="lspawn")
			{
				keyValue.Value+= toString("objl %d %s %s\n", it->second.FinalId, it->first.Shape.c_str(), it->first.Bone.c_str());
			}
			else if(it->first.Cmd=="wspawn")
			{
				keyValue.Value+= toString("objw %d %s\n", it->second.FinalId, it->first.Shape.c_str());
			}
		}
	}

	// add to the track
	finalTrack->addKey(keyValue, keyTime);

	if(keyTime==0)
		insertedAt0= true;
}

void				CSSSBuild::compile(NL3D::CAnimation &dest, const char* sBaseName)
{
	// no script?
	if(Bones.empty())
		return;

	/*	Take each script at each bone, and generate only ONE script (the global script), 
		that will be attached to the animation
	*/

	// list of unique object
	std::map<CSpawnCmd, CSpawnObject>			objects;
	uint										numObjs= 0;

	// timeline of cmds
	std::multimap<NL3D::TAnimationTime, CSpawnCmd>	keys;


	// **** first pass. compute set of objects, and generate keys timeline
	uint i;
	for(i=0;i<Bones.size();i++)
	{
		CSSSBuild::CBoneScript				&bone= Bones[i];
		// "compile" script at each key
		for(uint j=0;j<bone.Track.size();j++)
		{
			std::string		&script= bone.Track[j].Value;
			// at each key, there can't be the same shape to create
			std::set<CSpawnCmd>	keySet;
			
			// parse each line of the script
			static std::vector<std::string>		lines;
			lines.clear();
			splitString(script, "\n", lines);
			for(uint k=0;k<lines.size();k++)
			{
				std::string	&line= lines[k];

				// remove any '\r'
				line.resize( std::remove_if(line.begin(), line.end(), std::bind2nd(std::equal_to<char>(), '\r') )
					- line.begin() );

				// parse
				static std::vector<std::string>		words;
				words.clear();
				splitString(line, " ", words);
				
				if(words.size()>=2)
				{
					bool	ok= false;
					if(words[0]=="wspawn")		ok= true;
					else if(words[0]=="lspawn")	ok= true;
					else if(words[0]=="wkill")		ok= true;
					else if(words[0]=="lkill")	ok= true;
					
					// if valid command
					if(ok)
					{
						CSpawnCmd	oi;
						oi.Cmd= words[0];
						oi.Shape= words[1];
						// lower to avoid case error, and append .shape
						strlwr(oi.Shape);
						if(oi.Shape.find('.')==std::string::npos)
							oi.Shape+= ".shape";
						// bone name
						oi.Bone= bone.BoneName;
						// valid if the command does not exist in the current key
						if(keySet.insert(oi).second)
						{
							// if the command is not a kill, insert in the object list
							if(oi.Cmd!="wkill" && oi.Cmd!="lkill")
							{
								// insert if not already done
								if(objects.find(oi)==objects.end())
								{
									objects[oi].FinalId= numObjs++;
								}
							}
							// insert a cmd key at this key time
							keys.insert(std::make_pair(bone.Track[j].Time, oi));
						}
					}
				}
			}
		}
	}

	// **** second pass, genereate the global script animation
	// create the final track
	CTrackKeyFramerConstString		*finalTrack= new CTrackKeyFramerConstString;
	// parse each key in the chronogical order
	NL3D::TAnimationTime	precTime= 0;
	bool					firstKey= true;
	bool					insertedAt0= false;
	std::multimap<NL3D::TAnimationTime, CSpawnCmd>::iterator		kit;
	for(kit= keys.begin();kit!=keys.end();kit++)
	{
		NL3D::TAnimationTime	keyTime= kit->first;
		CSpawnCmd				keyValue= kit->second;


		// ---- if the keyTime has changed, then commit the last key
		if(!firstKey && precTime!=keyTime)
		{
			commitSSSKey(precTime, objects, finalTrack, insertedAt0);
			// new key to create
			precTime= keyTime;
		}
		// no more first key
		if(firstKey)
			firstKey= false;
		// for next loop
		precTime= keyTime;
		

		// ---- update object state, according to the key cmd
		std::map<CSpawnCmd, CSpawnObject>::iterator		oit;
		bool	isSpawn;
		if(keyValue.Cmd=="wspawn" || keyValue.Cmd=="lspawn")
			isSpawn= true;
		else
		{
			isSpawn= false;
			// Find the associated object if a kill => replace kill by spawn
			if(keyValue.Cmd=="wkill")	keyValue.Cmd="wspawn";
			if(keyValue.Cmd=="lkill")	keyValue.Cmd="lspawn";
		}
		oit= objects.find(keyValue);
		// NB: may not be found, if a kill has a bad ShapeName
		if(oit!=objects.end())
		{
			// spawn or kill?
			if(isSpawn)
				oit->second.Spawned= true;
			else
				oit->second.Spawned= false;
		}
	}

	// if some keys added
	if(!firstKey)
	{
		// comit the last key
		commitSSSKey(precTime, objects, finalTrack, insertedAt0);

		// If none has been inserted at 0, must add an empty key at 0
		if(!insertedAt0)
		{
			CKeyString		keyValue;
			finalTrack->addKey(keyValue, 0);
		}
	}
	else
	{
		// no keys added
		delete finalTrack;
		finalTrack = NULL;
	}
		

	// **** Add to the animation
	if(finalTrack)
	{
		// ---- add the track
		std::string		name= std::string(sBaseName) + NL3D::CSkeletonModel::getSpawnScriptValueName();
		dest.addTrack(name, finalTrack);

		// ---- must also inform the animation of all shapes it can spawn
		std::set<std::string>	shapeSet;
		std::map<CSpawnCmd, CSpawnObject>::iterator		oit;

		// remove shape duplicate from cmd
		for(oit= objects.begin();oit!=objects.end();oit++)
			shapeSet.insert(oit->first.Shape);

		// then for each, insert in the animation
		std::set<std::string>::iterator		it;
		for(it=shapeSet.begin();it!=shapeSet.end();it++)
			dest.addSSSShape(*it);
	}
}



