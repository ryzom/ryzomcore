// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/common.h"
#include "nel/3d/scene_group.h"
#include "nel/3d/zone.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/3d/register_3d.h"
#include "nel/3d/mesh.h"
#include "nel/3d/mesh_mrm.h"
#include "nel/3d/mesh_mrm_skinned.h"
#include "nel/3d/mesh_multi_lod.h"

#ifdef NL_OS_WINDOWS
#include <conio.h>
#endif // NL_OS_WINDOWS

#ifdef NL_OS_UNIX
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

int _getch( ) {
struct termios oldt,
newt;
int ch;
tcgetattr( STDIN_FILENO, &oldt );
newt = oldt;
newt.c_lflag &= ~( ICANON | ECHO );
tcsetattr( STDIN_FILENO, TCSANOW, &newt );
ch = getchar();
tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
return ch;
}
#endif // NL_OS_UNIX


using	namespace std;
using	namespace NLMISC;
using	namespace NL3D;


// ***************************************************************************
void	displayGeom(FILE *logStream, const CMeshGeom &geom)
{
	uint	i,j;
	uint	numFaces=0;
	for(i=0;i<geom.getNbMatrixBlock();i++)
	{
		for(j=0;j<geom.getNbRdrPass(i);j++)
		{
			numFaces+= geom.getRdrPassPrimitiveBlock(i,j).getNumIndexes()/3;
		}
	}
	fprintf(logStream, "Standard Mesh %s\n", geom.isSkinned()?"Skinned":"" );
	fprintf(logStream, "  NumFaces: %d\n", numFaces );
	fprintf(logStream, "  NumVertices: %d\n", geom.getVertexBuffer().getNumVertices() );
	uint	nbBS= geom.getNbBlendShapes();
	if(nbBS)
		fprintf(logStream, "  NumBlendShapes: %d\n", nbBS );
	IMeshVertexProgram	*mvp= geom.getMeshVertexProgram();
	if(mvp)
		fprintf(logStream, "  MeshVertexProgram: %s\n", typeid(*mvp).name() );
	
}

void	displayMRMGeom(FILE *logStream, const CMeshMRMGeom &geom)
{
	uint	i,j;
	uint	numFaces=0;
	uint	numFacesLodMax=0;
	for(i=0;i<geom.getNbLod();i++)
	{
		for(j=0;j<geom.getNbRdrPass(i);j++)
		{
			uint	nPassFaces= geom.getRdrPassPrimitiveBlock(i,j).getNumIndexes()/3;
			numFaces+= nPassFaces;
			if(i==geom.getNbLod()-1)
				numFacesLodMax+= nPassFaces;
		}
	}
	fprintf(logStream, "MRM Mesh %s\n", geom.isSkinned()?"Skinned":"" );
	fprintf(logStream, "  NumFaces(Max Lod): %d\n", numFacesLodMax );
	fprintf(logStream, "  NumFaces(Sum all Lods): %d\n", numFaces );
	fprintf(logStream, "  NumVertices(Sum all Lods): %d\n", geom.getVertexBuffer().getNumVertices() );
	fprintf(logStream, "  NumShadowSkinVertices: %d\n", geom.getNumShadowSkinVertices() );
	fprintf(logStream, "  Skinned: %s\n", geom.isSkinned()?"true":"false" );
	uint	nbBS= geom.getNbBlendShapes();
	if(nbBS)
		fprintf(logStream, "  NumBlendShapes: %d\n", nbBS );
}


void	displayMRMSkinnedGeom(FILE *logStream, const CMeshMRMSkinnedGeom &geom)
{
	uint	i,j;
	uint	numFaces=0;
	uint	numFacesLodMax=0;
	for(i=0;i<geom.getNbLod();i++)
	{
		for(j=0;j<geom.getNbRdrPass(i);j++)
		{
			CIndexBuffer block;
			geom.getRdrPassPrimitiveBlock(i,j,block);
			uint	nPassFaces= block.getNumIndexes()/3;
			numFaces+= nPassFaces;
			if(i==geom.getNbLod()-1)
				numFacesLodMax+= nPassFaces;
		}
	}
	fprintf(logStream, "MRM Skinned Mesh\n");
	fprintf(logStream, "  NumFaces(Max Lod): %d\n", numFacesLodMax );
	fprintf(logStream, "  NumFaces(Sum all Lods): %d\n", numFaces );
	CVertexBuffer VB;
	geom.getVertexBuffer(VB);
	fprintf(logStream, "  NumVertices(Sum all Lods): %d\n", VB.getNumVertices() );
	fprintf(logStream, "  NumShadowSkinVertices: %d\n", geom.getNumShadowSkinVertices() );
}


uint	MaxNumLightMap= 0;
void	displayMeshBase(FILE *logStream, CMeshBase *meshBase)
{
	uint	nMat= meshBase->getNbMaterial();
	uint	nLms= (uint)meshBase->_LightInfos.size();
	MaxNumLightMap= max(MaxNumLightMap, nLms);
	if(nLms)
	{
		fprintf(logStream, "The Mesh has %d lightmaps for %d Materials\n", nLms, nMat );
		for(uint i=0;i<nLms;i++)
		{
			uint32		lg= meshBase->_LightInfos[i].LightGroup;
			string		al= meshBase->_LightInfos[i].AnimatedLight;
			fprintf(logStream, "  LightGroup=%d; AnimatedLight='%s'; mat/stage: ", lg, al.c_str());
			std::list<CMeshBase::CLightMapInfoList::CMatStage>::iterator	it= meshBase->_LightInfos[i].StageList.begin();
			while(it!=meshBase->_LightInfos[i].StageList.end())
			{
				fprintf(logStream, "%d/%d, ", it->MatId, it->StageId);
				it++;
			}
			fprintf(logStream, "\n");
		}
	}
	else
	{
		fprintf(logStream, "The Mesh has %d Materials\n", nMat );
	}

	fprintf(logStream, "The mesh has a LodCharacterTexture: %s\n", meshBase->getLodCharacterTexture()?"true":"false");
}


// ***************************************************************************
/// Dispaly info for file in stdout
void	displayInfoFileInStream(FILE *logStream, const char *fileName, const set<string> &options, bool displayShortFileName)
{
	if(fileName==NULL)
		return;

	bool ms = options.find ("-ms") != options.end();
	bool vi = options.find ("-vi") != options.end();
	bool vl = options.find ("-vl") != options.end();
	bool vp = options.find ("-vp") != options.end();
	bool veil = options.find ("-veil") != options.end();

	// Special option.
	if( ms )
	{
		if(strstr(fileName, ".shape"))
		{
			// read the skeleton.
			CIFile	file(fileName);
			CShapeStream	shapeStream;
			file.serial(shapeStream);

			// Test Type
			CMesh			*mesh= dynamic_cast<CMesh*>(shapeStream.getShapePointer());

			// Mesh ??
			if( mesh )
			{
				if( mesh->getMeshGeom().isSkinned() )
				{
					fprintf(logStream, "%s is Skinned, but without MRM!!!\n", fileName);
				}
			}

			// release
			delete shapeStream.getShapePointer();
			shapeStream.setShapePointer(NULL);
		}
	}
	// Std Way.
	else
	{
		// some general info.
		if(displayShortFileName)
		{
			string	sfn= CFile::getFilename(fileName);
			fprintf(logStream, "File: %s\n", sfn.c_str());
		}
		else
		{
			fprintf(logStream, "File: %s\n", fileName);
		}
		fprintf(logStream, "***********\n\n");

		if(strstr(fileName, ".zone"))
		{
			// read the zone.
			CIFile	file(fileName);
			CZone	zone;
			file.serial(zone);

			// retreive info on Zone
			CZoneInfo	zoneInfo;
			zone.retrieve(zoneInfo);

			// display Info on the zone:
			fprintf(logStream, "  Num Patchs: %d\n", zone.getNumPatchs() );
			fprintf(logStream, "  Num PointLights: %u\n", (uint)zoneInfo.PointLights.size() );
			if (vl)
			{
				fprintf(logStream, "  Lights\n");
				uint k;
				for(k = 0; k < zoneInfo.PointLights.size(); ++k)
				{
					const CPointLightNamed &pl = zoneInfo.PointLights[k];
					CRGBA diffuse = pl.getDiffuse();
					CRGBA defaultDiffuse = pl.getDefaultDiffuse ();
					fprintf(logStream, "    light group = %d, anim = \"%s\" x=%.1f, y=%.1f, z=%.1f, r=%d, g=%d, b=%d, dr=%d, dg=%d, db=%d\n", pl.LightGroup, 
						pl.AnimatedLight.c_str(), pl.getPosition().x, pl.getPosition().y, pl.getPosition().z, 
						diffuse.R, diffuse.G, diffuse.B, defaultDiffuse.R, defaultDiffuse.G, defaultDiffuse.B);
				}
			}
			if (vp)
			{
				CZoneInfo zoneInfo;
				zone.retrieve (zoneInfo);

				// Patch information
				uint k;
				for(k = 0; k < zoneInfo.Patchs.size(); ++k)
				{
					fprintf(logStream, "   Patch %d, S %d, T %d, smooth flags %d %d %d %d, corner flags %d %d %d %d\n",
						k, 
						zoneInfo.Patchs[k].OrderS,
						zoneInfo.Patchs[k].OrderT,
						zoneInfo.Patchs[k].getSmoothFlag (0),
						zoneInfo.Patchs[k].getSmoothFlag (1),
						zoneInfo.Patchs[k].getSmoothFlag (2),
						zoneInfo.Patchs[k].getSmoothFlag (3),
						zoneInfo.Patchs[k].getCornerSmoothFlag(0),
						zoneInfo.Patchs[k].getCornerSmoothFlag(1),
						zoneInfo.Patchs[k].getCornerSmoothFlag(2),
						zoneInfo.Patchs[k].getCornerSmoothFlag(3));
					uint l;
					for (l=0; l<4; l++)
					{
						fprintf(logStream, "    Bind edge %d, NPatchs %d, ZoneId %d, Next %d %d %d %d, Edge %d %d %d %d\n",
							l, 
							zoneInfo.Patchs[k].BindEdges[l].NPatchs,
							zoneInfo.Patchs[k].BindEdges[l].ZoneId,
							zoneInfo.Patchs[k].BindEdges[l].Next[0],
							zoneInfo.Patchs[k].BindEdges[l].Next[1],
							zoneInfo.Patchs[k].BindEdges[l].Next[2],
							zoneInfo.Patchs[k].BindEdges[l].Next[3],
							zoneInfo.Patchs[k].BindEdges[l].Edge[0],
							zoneInfo.Patchs[k].BindEdges[l].Edge[1],
							zoneInfo.Patchs[k].BindEdges[l].Edge[2],
							zoneInfo.Patchs[k].BindEdges[l].Edge[3]);
					}
				}
			}
		}
		else if(strstr(fileName, ".ig"))
		{
			// read the ig.
			CIFile	file(fileName);
			CInstanceGroup	ig;
			file.serial(ig);

			// display Info on the ig:
			CVector gpos = ig.getGlobalPos();
			fprintf(logStream, "  Global pos : x = %.1f, y = %.1f, z =%.1f\n", gpos.x, gpos.y, gpos.z);
			fprintf(logStream, "  Num Instances: %d\n", ig.getNumInstance() );
			fprintf(logStream, "  Num PointLights: %u\n", (uint)ig.getPointLightList().size() );
			fprintf(logStream, "  Realtime sun contribution = %s\n", ig.getRealTimeSunContribution() ? "on" : "off");
			if (vi)
			{
				fprintf(logStream, "  Instances:\n");
				uint k;
				for(k = 0; k < ig._InstancesInfos.size(); ++k)
				{
					fprintf(logStream, "    Instance %3d: shape = %s, name = %s, x = %.1f, y = %.1f, z = %.1f, sx = %.1f, sy = %.1f, sz = %.1f\n", k, ig._InstancesInfos[k].Name.c_str(), ig._InstancesInfos[k].InstanceName.c_str(), ig._InstancesInfos[k].Pos.x + gpos.x, ig._InstancesInfos[k].Pos.y + gpos.y, ig._InstancesInfos[k].Pos.z + gpos.z, ig._InstancesInfos[k].Scale.x, ig._InstancesInfos[k].Scale.y, ig._InstancesInfos[k].Scale.z);
				}
			}
			if (vl)
			{
				fprintf(logStream, "  Lights:\n");
				uint k;
				for(k = 0; k < ig.getNumPointLights(); ++k)
				{
					const CPointLightNamed &pl = ig.getPointLightNamed(k);
					CRGBA diffuse = pl.getDiffuse();
					CRGBA defaultDiffuse = pl.getDefaultDiffuse ();
					fprintf(logStream, "    Light %3d: Light group = %d, anim = \"%s\" x=%.1f, y=%.1f, z=%.1f, r=%d, g=%d, b=%d, dr=%d, dg=%d, db=%d\n", k, pl.LightGroup, 
						pl.AnimatedLight.c_str(), pl.getPosition().x + gpos.x, pl.getPosition().y + gpos.y, pl.getPosition().z + gpos.z,
						diffuse.R, diffuse.G, diffuse.B, defaultDiffuse.R, defaultDiffuse.G, defaultDiffuse.B);
				}
			}
			if (veil)
			{
				fprintf(logStream, "  Instances Bound To Lights:\n");
				fprintf(logStream, "    WordList:\n");
				fprintf(logStream, "    'StaticLight Not Computed' means the instance has a ASP flag or the ig is not yet lighted\n");
				fprintf(logStream, "    If lighted, for each instance, the format is 'SunContribution(8Bit) - idLight0;idLight1 (or NOLIGHT) - LocalAmbientId (or GLOBAL_AMBIENT)' \n");
				fprintf(logStream, "    DCS means the instance don't cast shadow (used in the lighter)\n");
				fprintf(logStream, "    DCSINT Same but very special for ig_lighter only\n");
				fprintf(logStream, "    DCSEXT Same but very special for zone_lighter and zone_ig_lighter only\n");
				fprintf(logStream, "    ASP means the instance AvoidStaticLightPreCompute (used in the lighter)\n");
				fprintf(logStream, "  -------------------------------------------------------------\n");
				uint k;
				for(k = 0; k < ig._InstancesInfos.size(); ++k)
				{
					CInstanceGroup::CInstance	&instance= ig._InstancesInfos[k];
					fprintf(logStream, "    Instance %3d: ", k);
					if(!instance.StaticLightEnabled)
						fprintf(logStream, " StaticLight Not Computed.");
					else
					{
						fprintf(logStream, " %3d - ", instance.SunContribution);
						if(instance.Light[0]==0xFF)
							fprintf(logStream, "NOLIGHT - ");
						else
						{
							fprintf(logStream, "%3d;", instance.Light[0]);
							if(instance.Light[1]!=0xFF)
								fprintf(logStream, "%3d", instance.Light[1]);
							else
							  fprintf(logStream, "   "); //, instance.Light[1]);
							fprintf(logStream, " - ");
						}
						if(instance.LocalAmbientId==0xFF)
							fprintf(logStream, "GLOBAL_AMBIENT.  ");
						else
							fprintf(logStream, "%d.  ", instance.LocalAmbientId);
					}
					if(instance.DontCastShadow)
						fprintf(logStream, "DCS,");
					if(instance.DontCastShadowForInterior)
						fprintf(logStream, "DCSINT,");
					if(instance.DontCastShadowForExterior)
						fprintf(logStream, "DCSEXT,");
					if(instance.AvoidStaticLightPreCompute)
						fprintf(logStream, "ASP,");

					fprintf(logStream, "\n");
				}
			}
		}
		else if(strstr(fileName, ".skel"))
		{
			// read the skeleton.
			CIFile	file(fileName);
			CShapeStream	shapeStream;
			file.serial(shapeStream);
			CSkeletonShape	*skel= dynamic_cast<CSkeletonShape*>(shapeStream.getShapePointer());

			if(skel)
			{
				vector<CBoneBase>	bones;
				skel->retrieve(bones);
				// Display Bone Infos.
				fprintf(logStream, "Num Bones: %u\n", (uint)bones.size());
				for(uint i=0; i<bones.size(); i++)
				{

					// get parent
					sint32	parent = bones[i].FatherId;

					// get parent
					bool	inheritScale = bones[i].UnheritScale;

					// get default pos.
					CVector	pos = bones[i].DefaultPos.getDefaultValue();

					// get default rotquat.
					CQuat	rotQuat = bones[i].DefaultRotQuat.getDefaultValue();

					// get default scale.
					CVector	scale = bones[i].DefaultScale.getDefaultValue();

					// get inv bind pos.
					CMatrix	invBindPos = bones[i].InvBindPos;
					CVector invBindPosI = invBindPos.getI();
					CVector invBindPosJ = invBindPos.getJ();
					CVector invBindPosK = invBindPos.getK();
					CVector invBindPosT = invBindPos.getPos();

					// print info
					fprintf(logStream, "Bone %2d. %s.\n", i, bones[i].Name.c_str());
					fprintf(logStream, "   Parent:          %d\n", parent );
					fprintf(logStream, "   InheritScale:    %d\n", inheritScale );
					fprintf(logStream, "   Position:        (%2.3f, %2.3f, %2.3f)\n", 
						pos.x, pos.y, pos.z);
					fprintf(logStream, "   RotQuat:         (%2.3f, %2.3f, %2.3f, %2.3f)\n", 
						rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w);
					fprintf(logStream, "   Scale:           (%2.3f, %2.3f, %2.3f)\n",
						scale.x, scale.y, scale.z);
					fprintf(logStream, "   InvBindPos: I:   (%2.3f, %2.3f, %2.3f)\n", 
						invBindPosI.x, invBindPosI.y, invBindPosI.z);
					fprintf(logStream, "   InvBindPos: J:   (%2.3f, %2.3f, %2.3f)\n", 
						invBindPosJ.x, invBindPosJ.y, invBindPosJ.z);
					fprintf(logStream, "   InvBindPos: K:   (%2.3f, %2.3f, %2.3f)\n", 
						invBindPosK.x, invBindPosK.y, invBindPosK.z);
					fprintf(logStream, "   InvBindPos: Pos: (%2.3f, %2.3f, %2.3f)\n", 
						invBindPosT.x, invBindPosT.y, invBindPosT.z);
					
				}
			}
			else
			{
				fprintf(logStream, "Bad Skel file\n");
			}

			// release
			delete shapeStream.getShapePointer();
			shapeStream.setShapePointer(NULL);
		}
		else if(strstr(fileName, ".shape"))
		{
			// read the shape.
			CIFile	file(fileName);
			CShapeStream	shapeStream;
			file.serial(shapeStream);

			// Test Type
			CMesh			*mesh= dynamic_cast<CMesh*>(shapeStream.getShapePointer());
			CMeshMRM		*meshMRM= dynamic_cast<CMeshMRM*>(shapeStream.getShapePointer());
			CMeshMRMSkinned *meshMRMSkinned= dynamic_cast<CMeshMRMSkinned*>(shapeStream.getShapePointer());
			CMeshMultiLod	*meshMulti= dynamic_cast<CMeshMultiLod*>(shapeStream.getShapePointer());

			// Material infos
			CMeshBase		*meshBase= dynamic_cast<CMeshBase*>(shapeStream.getShapePointer());
			if(meshBase)
			{
				displayMeshBase(logStream, meshBase);
			}

			// Mesh ??
			if( mesh )
			{
				displayGeom(logStream, mesh->getMeshGeom());
			}
			// MRM ??
			else if( meshMRM )
			{
				displayMRMGeom(logStream, meshMRM->getMeshGeom());
			}
			else if( meshMRMSkinned )
			{
				displayMRMSkinnedGeom(logStream, meshMRMSkinned->getMeshGeom());
			}
			// MultiLod??
			else if( meshMulti )
			{
				uint	numSlots= meshMulti->getNumSlotMesh ();
				fprintf(logStream, "  Num Lods: %d\n", numSlots );
				if(numSlots)
				{
					const CMeshGeom		*meshGeom= dynamic_cast<const CMeshGeom*>(&(meshMulti->getMeshGeom(0)));
					const CMeshMRMGeom	*meshMRMGeom= dynamic_cast<const CMeshMRMGeom*>(&(meshMulti->getMeshGeom(0)));
					if( meshGeom )
						displayGeom(logStream, *meshGeom);
					else if( meshMRMGeom )
						displayMRMGeom(logStream, *meshMRMGeom);
				}
			}
			else
			{
				fprintf(logStream, "Unsupported .shape type for display info\n");
			}

			// release
			delete shapeStream.getShapePointer();
			shapeStream.setShapePointer(NULL);
		}
		else if(strstr(fileName, ".anim"))
		{
			// read the shape.
			CIFile	file(fileName);
			CAnimation	anim;
			file.serial(anim);

			// Enum the tracks
			std::set<std::string> tracks;
			anim.getTrackNames (tracks);
			std::set<std::string>::iterator ite = tracks.begin();
			while (ite != tracks.end())
			{
				// Track name
				fprintf(logStream, "Track name=%s", ite->c_str());

				uint trackId = anim.getIdTrackByName (*ite);
				ITrack *track = anim.getTrack (trackId);
				if (track)
				{
					fprintf(logStream, " type=%s", typeid(*track).name());

					UTrackKeyframer *keyFramer = dynamic_cast<UTrackKeyframer*> (track);
					if (keyFramer)
					{
						TAnimationTime begin = track->getBeginTime ();
						TAnimationTime end = track->getEndTime ();
						std::vector<TAnimationTime> keys;
						keyFramer->getKeysInRange(begin, end, keys);
						if (!keys.empty())
						{
							float fvalue;
							sint32 ivalue;
							CRGBA cvalue;
							CVector vvalue;
							CQuat qvalue;
							string svalue;
							bool bvalue;
							uint i;
							if (track->interpolate (begin, fvalue))
							{
								fprintf(logStream, " floats\n");
								for (i=0; i<keys.size(); i++)
								{
									if (track->interpolate (keys[i], fvalue))
										fprintf(logStream, "\tKey %d : time=%f, value=%f\n", i, keys[i], fvalue);
								}
							}
							else if (track->interpolate (begin, ivalue))
							{
								fprintf(logStream, " integers\n");
								for (i=0; i<keys.size(); i++)
								{
									if (track->interpolate (keys[i], ivalue))
										fprintf(logStream, "\tKey %d : time=%f, value=%d\n", i, keys[i], ivalue);
								}
							}
							else if (track->interpolate (begin, cvalue))
							{
								fprintf(logStream, " color\n");
								for (i=0; i<keys.size(); i++)
								{
									if (track->interpolate (keys[i], cvalue))
										fprintf(logStream, "\tKey %d : time=%f, r=%d, g=%d, b=%d, a=%d\n", i, keys[i], cvalue.R, cvalue.G, cvalue.B, cvalue.A);
								}
							}
							else if (track->interpolate (begin, vvalue))
							{
								fprintf(logStream, " vector\n");
								for (i=0; i<keys.size(); i++)
								{
									if (track->interpolate (keys[i], vvalue))
										fprintf(logStream, "\tKey %d : time=%f, x=%f, y=%f, z=%f\n", i, keys[i], vvalue.x, vvalue.y, vvalue.z);
								}
							}
							else if (track->interpolate (begin, qvalue))
							{
								fprintf(logStream, " quaternion\n");
								for (i=0; i<keys.size(); i++)
								{
									if (track->interpolate (keys[i], qvalue))
										fprintf(logStream, "\tKey %d : time=%f, x=%f, y=%f, z=%f, w=%f\n", i, keys[i], qvalue.x, qvalue.y, qvalue.z, qvalue.w);
								}
							}
							else if (track->interpolate (begin, svalue))
							{
								fprintf(logStream, " string\n");
								for (i=0; i<keys.size(); i++)
								{
									if (track->interpolate (keys[i], svalue))
										fprintf(logStream, "\tKey %d : time=%f, value=%s\n", i, keys[i], svalue.c_str());
								}
							}
							else if (track->interpolate (begin, bvalue))
							{
								fprintf(logStream, " bool\n");
								for (i=0; i<keys.size(); i++)
								{
									if (track->interpolate (keys[i], bvalue))
										fprintf(logStream, "\tKey %d : time=%f, value=%s\n", i, keys[i], bvalue?"true":"false");
								}
							}
						}
					}
				}

				ite++;
			}
		}
		else
		{
			fprintf(logStream, "unsupported format\n");
		}
	}
}


// ***************************************************************************
// dispaly info for a file.
void		displayInfoFile(FILE *logStream, const char *fileName, const set<string> &options, bool displayShortFileName)
{
	// Display on screen.
	displayInfoFileInStream(stdout, fileName,options, displayShortFileName);
	// Display in log
	if(logStream)
		displayInfoFileInStream(logStream, fileName,options, displayShortFileName);
}


// ***************************************************************************
/// Dispaly info cmd line
int		main(int argc, const char *argv[])
{
	registerSerial3d();

	if(argc<2)
	{
		puts("Usage: ig_info file.??? [opt]");
		puts("Usage: ig_info directory [opt]");
		puts("    For now, only .ig, .zone, .skel, .shape are supported");
		puts("    Results are displayed too in \"c:/temp/file_info.log\" ");
		puts("    [opt] can get: ");
		puts("    -ms display only a Warning if file is a .shape and is a Mesh, skinned, but without MRM");
		puts("    -vi verbose instance information");
		puts("    -vl verbose light information");
		puts("    -vp verbose patche information");
		puts("    -veil verbose instances bound to light extra information");
		puts("Press any key");
		_getch();
		return -1;
	}

	// Parse options.
	set<string> options;
	int i;
	for (i=2; i<argc; i++)
		options.insert (argv[i]);

	// Open log
	FILE *logStream = nlfopen(getLogDirectory() + "file_info.log", "wt");

	// parse dir or file ??
	const char *fileName= argv[1];
	if(CFile::isDirectory(fileName))
	{
		// dir all files.
		std::vector<std::string>	listFile;
		CPath::getPathContent (fileName, false, false, true, listFile);

		fprintf(stdout,		"Scanning Directory '%s' .........\n\n\n", fileName);
		if(logStream)
			fprintf(logStream,	"Scanning Directory '%s' .........\n\n\n", fileName);

		// For all files.
		for(uint i=0;i<listFile.size();i++)
		{
			displayInfoFile(logStream, listFile[i].c_str(), options, true);
		}

		// display info for lightmaps
		fprintf(stdout,		"\n\n ************** \n I HAVE FOUND AT MAX %d LIGHTMAPS IN A SHAPE\n", MaxNumLightMap);
		if(logStream)
			fprintf(logStream,	"\n\n ************** \n I HAVE FOUND AT MAX %d LIGHTMAPS IN A SHAPE\n", MaxNumLightMap);
	}
	else
	{
		displayInfoFile(logStream, fileName, options, false);
	}


	// close log
	if(logStream)
		fclose(logStream);


	puts("Press any key");
	_getch();
}
