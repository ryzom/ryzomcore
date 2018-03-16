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

#ifndef NL_EXPORT_NEL_H
#define NL_EXPORT_NEL_H

#include "nel/misc/types_nl.h"
#include "nel/3d/mesh.h"
#include "nel/3d/material.h"
#include "nel/3d/mesh_vertex_program.h"
#include "nel/3d/camera.h"
#include "nel/3d/key.h"
#include "nel/3d/track_keyframer.h"
#include "nel/3d/bone.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/animation_time.h"
#include "nel/pacs/collision_mesh_build.h"

#define UVGEN_MISSING (-1)
#define UVGEN_REFLEXION (-2)
#define FLOAT_EPSILON 0.001

#define NEL_MTL_A						0x64c75fec
#define NEL_MTL_B						0x222b9eb9
#define NEL_LIGHT_CLASS_ID_A			0x36e3181f
#define NEL_LIGHT_CLASS_ID_B			0x3ac24049
#define NEL_PACS_BOX_CLASS_ID_A			0x7f374277
#define NEL_PACS_BOX_CLASS_ID_B			0x5d3971df
#define NEL_PACS_CYL_CLASS_ID_A			0x62a56810
#define NEL_PACS_CYL_CLASS_ID_B			0x4b3d601c
#define NEL_PARTICLE_SYSTEM_CLASS_ID	0x58ce2893
#define NEL_FLARE_CLASS_ID_A			0x4e913532
#define NEL_FLARE_CLASS_ID_B			0x3c2f2307
#define NEL_WAVE_MAKER_CLASS_ID_A		0x77e24828
#define NEL_WAVE_MAKER_CLASS_ID_B		0x329a1de5
#define NEL_REMANENT_SEGMENT_CLASS_ID_A 0x72f3588b
#define NEL_REMANENT_SEGMENT_CLASS_ID_B 0x6eda0a52

#define MAX_MAX_TEXTURE					8

#define MAX_MORPHER_CLASS_ID			Class_ID(0x17bb6854, 0xa5cba2a3)



// ***************************************************************************

enum TNelValueType
{
	typeFloat,
	typePos,
	typeScale,
	typeRotation,
	typeColor,
	typeInt,
	typeBoolean,
	typeString,
	typeMatrix,
};

// ***************************************************************************

enum TNelScriptValueType
{
	scriptNothing,
	scriptFloat,
	scriptBool,
	scriptNode,
};

// ***************************************************************************

namespace NL3D
{
	class CAnimation;
	class ITrack;
	class CTrackKeyFramerConstBool;
	class CSkeletonShape;
	class CMRMParameters;
	class IMeshGeom;
	class CInstanceGroup;
	class CVegetableShape;
	class CLodCharacterShapeBuild;
	class CShapeBank;
	class IDriver;
	class CLandscape;
	class CTextureCube;
};


namespace NLPACS
{
	class CRetrieverBank;
	class CGlobalRetriever;
	class CPrimitiveBlock;
};

namespace NLMISC
{
	class CAABBox;
}


// ***************************************************************************
// Interface for feed back during calculation
class IProgress
{
public:
	virtual void setLine (uint32 LineNumber, std::string &LineText)=0;
	virtual void update()=0;
};

// ***************************************************************************
struct CExportNelOptions
{
	bool bShadow;
	bool bExportLighting;
	bool bExportBgColor;
	bool OutputLightmapLog;
	std::string sExportLighting;
	sint32 nExportLighting;
	float rLumelSize;
	sint32 nOverSampling;
	bool bExcludeNonSelected;
	IProgress *FeedBack;
	bool bShowLumel;
	bool bTestSurfaceLighting;
	float SurfaceLightingCellSize;
	float SurfaceLightingDeltaZ;


	CExportNelOptions::CExportNelOptions()
	{
		// If no configuration file
		bShadow = false;
		bExportLighting = false;
		bExportBgColor = true;
		OutputLightmapLog = false;
		sExportLighting = "c:\\temp";
		nExportLighting = 0; // Normal lighting
		rLumelSize = 0.25f;
		nOverSampling = 1;
		bExcludeNonSelected = false;
		FeedBack = NULL;
		bShowLumel = false;
		bTestSurfaceLighting= true;
		SurfaceLightingCellSize= 1.5f;
		SurfaceLightingDeltaZ= 0.8f;
	}

	void serial(NLMISC::IStream& stream)
	{
		sint version = stream.serialVersion (6);

		// Check version
		switch (version)
		{
		case 6:
			stream.serial (OutputLightmapLog);
		case 5:
		{
			bool	fake= false;
			stream.serial (fake);
		}
		case 4:
			stream.serial (SurfaceLightingDeltaZ);
		case 3:
			stream.serial (bTestSurfaceLighting);
			stream.serial (SurfaceLightingCellSize);
		case 2:
			stream.serial (bExportBgColor);
		case 1:
			stream.serial (bShowLumel);
			stream.serial (bShadow);
			stream.serial (bExportLighting);
			stream.serial (sExportLighting);
			stream.serial (nExportLighting);
			stream.serial (rLumelSize);
			stream.serial (nOverSampling);
			stream.serial (bExcludeNonSelected);
		}
	}
};

// ***************************************************************************

class CExportDesc;

// ***************************************************************************

typedef std::map<INode*, sint32> TInodePtrInt;


// ***************************************************************************
/**	Descriptor of a WindTree VertexProgram AppData
 */
class	CVPWindTreeAppData
{
public:
	enum	{HrcDepth= 3, NumTicks=100};

	/// Scale value for sliders
	float		FreqScale;
	float		DistScale;

	/// Frequency of the wind for 3 Hierachy levels. Slider Value
	int			Frequency[HrcDepth];
	/// Additional frequency, multiplied by the globalWindPower. Slider Value
	int			FrequencyWindFactor[HrcDepth];
	/// Power of the wind on XY. Mul by globalWindPower. Slider Value
	int			DistXY[HrcDepth];
	/// Power of the wind on Z. Mul by globalWindPower. Slider Value
	int			DistZ[HrcDepth];
	/// Bias result of the cosinus: f= cos(time)+bias. Slider Value
	int			Bias[HrcDepth];

	/// BST_CHECKED if want Specular Lighting.
	int			SpecularLighting;
};


// ***************************************************************************
/// Skeleton Spawn Script build, used at addAnimation()
class CSSSBuild
{
public:
	struct CKey
	{
		std::string				Value;
		NL3D::TAnimationTime	Time;
	};
	struct CBoneScript
	{
		// The name of the bone on which the track is bound
		std::string			BoneName;
		// The Temp Track definition
		std::vector<CKey>	Track;
	};
	std::vector<CBoneScript>	Bones;
	
	// if not empty, compile all bone scripts, and add to the animation of the skeleton
	void	compile(NL3D::CAnimation &dest, const char* sBaseName);
};


// ***************************************************************************

/**
 * 3dsmax to NeL export interface for other things that landscape.
 * \author Cyril Corvazier
 * \author Nevrax France
 * \date 2000
 */
class CExportNel
{
private:
	class	CAnimationBuildCtx;
public:
	enum
	{
		NoError=0,
		VertexWithoutWeight,
		InvalidSkeleton,
		CodeCount
	};

	static const char* ErrorMessage[CodeCount];

	typedef std::map<INode*, Matrix3> mapBoneBindPos;

	/// Constructor
	CExportNel (bool errorInDialog, bool view, bool absolutePath, Interface *ip, std::string errorTitle, CExportNelOptions *opt);

	// *********************
	// *** Export mesh
	// *********************

	/**
	  * Build a NeL mesh
	  *
	  * skeletonShape must be NULL if no bones.
	  */
	NL3D::IShape*					buildShape (INode& node, TimeValue time, const TInodePtrInt *nodeMap, 
												bool buildLods);

	/**
	  * Build a NeL meshBuild
	  * 
	  * This method does not care of the skeletonShape
	  * if isMorphTarget is true, no "mrm/normal mesh interface" is built
	  */
	NL3D::CMesh::CMeshBuild*		createMeshBuild(INode& node, TimeValue tvTime, NL3D::CMesh::CMeshBaseBuild*& baseBuild, const NLMISC::CMatrix &finalSpace = NLMISC::CMatrix::Identity, bool isMorphTarget= false);

	/** Test wether the node has app datas specifying interface meshs.
	  * \see applyInterfaceToMeshBuild
	  */ 
	static bool						useInterfaceMesh(INode &node);


	/** Use interface mesh from a max file to unify normal at extremities of a mesh
	  * Example : a character is sliced in slots, each parts being stored in a file.
	  * The normals at the junction part are incorrect.
	  * An "interface" is a polygon that is used to unify normal between various meshs 
	  * IMPORTANT : the meshbuild should have been exported in WORLD SPACE
	  * Note : the name of the max file that contains the name of the interface is stored in an app data attached to this node
	  * \param node the node from which datas must be retrieved (name of the .max file containing the interfaces)
	  * \param meshBuildToModify The mesh build whose normal will be modified
  	  * \param toWorldMat a matrix to put the meshbuild vertices into worldspace	  
	  * \param tvTime time aty which evaluate the mesh
	  */
	void							applyInterfaceToMeshBuild(INode &node, NL3D::CMesh::CMeshBuild &meshBuildToModify,
															  const NLMISC::CMatrix &toWorldMat,															  
															  TimeValue tvTime);

	/** This takes a max mesh, and select the vertices that match vertices of a mesh interface
	  * This has no effect if the mesh has no app datas specifying a mesh interface
	  * \see applyInterfaceToMeshBuild
	  * \return true if the operation succeed
	  */ 
	bool							selectInterfaceVertices(INode &node, TimeValue time);	

	/**
	  * Build a NeL instance group
	  */
	NL3D::CInstanceGroup*			buildInstanceGroup(const std::vector<INode*>& vectNode, std::vector<INode*>& resultInstanceNode, TimeValue tvTime);

	/**
	  * Build a complete NeL scene with objects attached to the scene root node
	  *
	  * \param scene is the scene to build
	  * \param shapeBank is the shape bank to use with the scene
	  * \param tvTime if the time to use to build the scene
	  * \param options is the options structure to use to build the scene
	  * \param landscape is a pointer ona landscape created with the scene. Can be NULL if you dan't want to build landscape zones.
	  * \param progress is the progress bar to use to display build progression. Can be NULL if no prgoress bar is needed
	  * \param buildHidden If it is true, build hidden nodes
	  * \param onlySelected If it is true, build only selected nodes
	  * \param buildLods If it is true, build lod of objects
	  */
	void							buildScene (NL3D::CScene &scene, NL3D::CShapeBank &shapeBank, NL3D::IDriver &driver, TimeValue tvTime, 
												NL3D::CLandscape *landscape, IProgress *progress, bool buildHidden, bool onlySelected, bool buildLods);

	/**
	  * Build a NeL camera
	  */
	void							buildCamera(NL3D::CCameraInfo &cameraInfo, INode& node, TimeValue time);

	/**
	  * Return true if it is a mesh.
	  *
	  * skeletonShape must be NULL if no bones.
	  *
	  *	if excludeCollision then return false if the mesh is a collision (NL3D_APPDATA_COLLISION)
	  *	else don't test NL3D_APPDATA_COLLISION.
	  */
	static bool						isMesh (INode& node, TimeValue time, bool excludeCollision= true);
	static bool						isCamera (INode& node, TimeValue time);
	static bool						isDummy (INode& node, TimeValue time);
	static bool						isVegetable (INode& node, TimeValue time);

	/** Compute an  aabbox of a mesh, in world.
	  * \return true if the conversion succeed.
	  */ 
	static bool						buildMeshAABBox(INode &node, NLMISC::CAABBox &dest, TimeValue time);

	/**
	  * Return true if the node is a mesh and has a Nel_Material attached to it
	  */
	static bool						hasLightMap (INode& node, TimeValue time);
	void							deleteLM (INode& node);
	bool							calculateLM (NL3D::CMesh::CMeshBuild *pZeMeshBuild, 
												NL3D::CMeshBase::CMeshBaseBuild *pZeMeshBaseBuild,
												INode& ZeNode, 
												TimeValue tvTime, uint firstMaterial, bool outputLightmapLog);

	bool							calculateLMRad(NL3D::CMesh::CMeshBuild *pZeMeshBuild, 
												NL3D::CMeshBase::CMeshBaseBuild *pZeMeshBaseBuild,
												INode& ZeNode, 
												TimeValue tvTime);
	

	// *********************
	// *** Export animation
	// *********************

	// Add animation track of this node into the animation object pass in parameter.
	void							addAnimation (NL3D::CAnimation& animation, INode& node, const char* sBaseName, bool root);

	// Build a NeL track with a 3dsmax node and a controller.
	NL3D::ITrack*					buildATrack (NL3D::CAnimation& animation, Control& c, TNelValueType type, Animatable& node, const CExportDesc& desc, 
												CAnimationBuildCtx	*animBuildCtx, bool bodyBiped=false);

	// Build a Nel bool track from a On/Off max Controller (doesn't work with buildATRack, which require a keyframer interface
	// , which isn't provided by an on / off controller)
	static NL3D::CTrackKeyFramerConstBool*			buildOnOffTrack(Control& c);

	// Add tracks for particle systems
	void							addParticleSystemTracks(NL3D::CAnimation& animation, INode& node, const char* parentName) ;

	// Add tracks for the bone and its children (recursive)
	void							addBoneTracks (NL3D::CAnimation& animation, INode& node, const char* parentName, 
										CAnimationBuildCtx	*animBuildCtx, bool root, CSSSBuild &ssBuilder);

	// Add biped tracks
	void							addBipedNodeTracks (NL3D::CAnimation& animation, INode& node, const char* parentName,
										CAnimationBuildCtx	*animBuildCtx, bool root, CSSSBuild &ssBuilder);


	// Add a note track. It tackes the first note track of the object
	static void						addNoteTrack(NL3D::CAnimation& animation, INode& node);

	// Add a SkeletonSpawnScript track. It takes the first note track of the object
	static void						addSSSTrack(CSSSBuild	&ssBuilder, INode& node);
	
	// Build a Nel String track from the first NoteTrack
	static NL3D::CTrackKeyFramerConstString*		buildFromNoteTrack(INode& node);

	// Convert keyframe methods
	static void						buildNelKey (NL3D::CKeyFloat& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyInt& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBool& nelKey, ILinFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyVector& nelKey, ILinPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyRGBA& nelKey, ILinPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyQuat& nelKey, ILinRotKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyVector& nelKey, ILinScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);

	static void						buildNelKey (NL3D::CKeyBezierFloat& nelKey, IBezFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBool& nelKey, IBezFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBezierVector& nelKey, IBezPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBezierQuat& nelKey, IBezQuatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBezierVector& nelKey, IBezScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);

	static void						buildNelKey (NL3D::CKeyTCBFloat& nelKey, ITCBFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyBool& nelKey, ITCBFloatKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyTCBVector& nelKey, ITCBPoint3Key& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyTCBQuat& nelKey, ITCBRotKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);
	static void						buildNelKey (NL3D::CKeyTCBVector& nelKey, ITCBScaleKey& maxKey, float ticksPerSecond, const CExportDesc& desc, Control& c);	


	// Create the transform matrix tracks
	void							createBipedKeyFramer (NL3D::ITrack *&nelRot, NL3D::ITrack *&nelPos, bool isRot, bool isPos, 
														float ticksPerSecond, const Interval& range, int oRT, const CExportDesc& desc, 
														INode& node, CAnimationBuildCtx	*animBuildCtx);

	// convert to nel time value
	static NL3D::TAnimationTime		convertTime (TimeValue time);

	// ************************************
	// *** Export skeleton and skinning
	// ************************************

	/* 
	 * Build a skeleton shape
	 *
	 * mapBindPos is the pointer of the map of bind pos by bone. Can be NULL if the skeleton is already in the bind pos.
	 */
	void							buildSkeletonShape (NL3D::CSkeletonShape& skeletonShape, INode& node, mapBoneBindPos* mapBindPos, 
														TInodePtrInt& mapId, TimeValue time);

	// Build an array of CBoneBase
	void							buildSkeleton (std::vector<NL3D::CBoneBase>& bonesArray, INode& node, mapBoneBindPos* mapBindPos, 
														TInodePtrInt& mapId, std::set<std::string> &nameSet, 
														TimeValue time, sint32& idCount, sint32 father=-1);

	/*
	 * Add Skinning data into the build structure.
	 * You must call first isSkin. If isSkin return true, then you can call this method.
	 * Can return the following errors:
	 *
	 * skeletonShape must be NULL if no bones.
	 * 
	 * NoError
	 * VertexWithoutWeight
	 * InvalidSkeleton
	 */
	static uint						buildSkinning (NL3D::CMesh::CMeshBuild& buildMesh, const TInodePtrInt& skeletonShape, INode& node);

	// Return true if the mesh is a skin, else return false.
	static bool						isSkin (INode& node);

	// Return the root node of the skeleton attached to the node. Return NULL if no skeleton.
	static INode*					getSkeletonRootBone (INode& node);

	// Add bind pose matrix of the bone used by the model in the map.
	static void						addSkeletonBindPos (INode& node, mapBoneBindPos& boneBindPos);

	// Enable / disable the skin modifier
	static void						enableSkinModifier (INode& node, bool enable);

	/// return true if the bone must unherit his father scale. Special cases for biped, biped father etc...
	bool							getNELUnHeritFatherScale(INode &node);

	/**	This method do the same thing as getLocalMatrix() mut manages complexs cases of Scale unheritance,
		strange Biped node Scale stuff, etc.... 
	 */
	void							getNELBoneLocalTM(INode &node, TimeValue time,
		NLMISC::CVector &nelScale, NLMISC::CQuat &nelQuat, NLMISC::CVector &nelPos);

	/// Physique Mirroring
	bool							mirrorPhysiqueSelection(INode &node, TimeValue time, const std::vector<uint> &vertIn, 
		float threshold);
	
	// **************
	// *** Export Lod
	// **************

	void							addChildLodNode (std::set<INode*> &lodListToExclude, INode *current = NULL);
	void							addParentLodNode (INode &node, std::set<INode*> &lodListToExclude, INode *current = NULL);

	// *********************
	// *** Export collision
	// *********************

	/** Export a CCollisionMeshBuild from a list of node.
	 *	NB: do not check NEL3D_APPDATA_COLLISION.
	 */
	NLPACS::CCollisionMeshBuild*	createCollisionMeshBuild(std::vector<INode *> &nodes, TimeValue tvTime);

	/** Export a list of CCollisionMeshBuild from a list of node, grouped by igName. User must delete the meshBuilds.
	 *	meshBuildList is a vector of tuple igName-Cmb
	 *	NB: check NEL3D_APPDATA_COLLISION. if not set, the node is not exported
	 */
	bool							createCollisionMeshBuildList(std::vector<INode *> &nodes, TimeValue time, 
																	std::vector<std::pair<std::string, NLPACS::CCollisionMeshBuild*> > &meshBuildList);

	/**
	 *	Parse all the scene and build retrieverBank and globalRetriever. NULL if no collisions mesh found.
	 *	\param retIgName eg: if IgName of a collisionMesh "col_pipo_1" is found in the scene, and if 
	 *	igNamePrefix=="col_" and igNameSuffix=="_", then retIgName returned is "pipo".
	 *	If different igName may match, result is undefined (random).
	 */
	void							computeCollisionRetrieverFromScene(TimeValue time, 
																	NLPACS::CRetrieverBank *&retrieverBank, NLPACS::CGlobalRetriever *&globalRetriever,
																	const char *igNamePrefix, const char *igNameSuffix, std::string &retIgName);

	/**
	  * Retrieve the Z rotation in radian of an object with its I matrix vector, assuming that the K matrix vector is near CVector::K
	  */
	static float getZRot (const NLMISC::CVector &i);

	/**
	  * Build a primtive block with an array of inode. Return false if one of the object is not a PACS primitive object.
	  */
	bool							buildPrimitiveBlock (TimeValue time, std::vector<INode*> objects, NLPACS::CPrimitiveBlock &primitiveBlock);

	// *********************
	// *** Export misc
	// *********************

	// Transforme a 3dsmax view matrix to camera matrix.
	static Matrix3					viewMatrix2CameraMatrix (const Matrix3& viewMatrix);

	// Convert a 3dsmax matrix in NeL matrix
	static void						convertMatrix (NLMISC::CMatrix& nelMatrix, const Matrix3& maxMatrix);

	// Convert a NeL matrix in 3dsmax matrix
	static void						convertMatrix (Matrix3& maxMatrix, const NLMISC::CMatrix& nelMatrix);

	/// Convert a 3dsmax uv matrix to a nel uv matrix
	static void						uvMatrix2NelUVMatrix (const Matrix3& uvMatrix, NLMISC::CMatrix &dest);

	// Convert a 3dsmax vector in NeL vector
	static void						convertVector (NLMISC::CVector& nelVector, const Point3& maxVector);

	// Get local node matrix
	static void						getLocalMatrix (Matrix3& localMatrix, INode& node, TimeValue time);

	// Decompose a 3dsmax matrix in NeL translation, scale, quat.
	static void						decompMatrix (NLMISC::CVector& nelScale, NLMISC::CQuat& nelRot, NLMISC::CVector& nelPos, 
													const Matrix3& maxMatrix);
	// Convert a 3dsmax color in NeL color
	static void						convertColor (NLMISC::CRGBA& nelColor, const Color& maxColor);

	// Return true if the node as the classId classid or is derived from a class with classId class id
	static bool						isClassIdCompatible (Animatable& node, Class_ID& classId);

	// Return the pointer on ther subanim with the name sName of the node. If it doesn't exist, return NULL.
	static Animatable*				getSubAnimByName (Animatable& node, const char* sName);

	// Get the node name
	static std::string				getName (MtlBase& mtl);

	// Get the node name
	static std::string				getName (INode& node);

	// Get the NEL node name. ie either NEL3D_APPDATA_INSTANCE_SHAPE appData or just GetName().
	static std::string				getNelObjectName (INode& node);

	// Get lights
	void							getLights (std::vector<NL3D::CLight>& vectLight, TimeValue time, INode* node=NULL);

	// Get the root node of the scene
	INode						   *getRootNode() const;
	// Get All node (objects only) of a hierarchy. NULL => all the scene
	void							getObjectNodes (std::vector<INode*>& vectNode, TimeValue time, INode* node=NULL);

	// *** Paramblock2 access

	// Return the pointer on the subanim with the name sName of the node. If it doesn't exist, return NULL.
	static Control*					getControlerByName (Animatable& node, const char* sName);

	// Return the pointer on the subanim with the name sName of the node. If it doesn't exist, return NULL.
	// Support only those type:
	// TYPE_FLOAT
	// TYPE_INT
	// TYPE_POINT3
	// TYPE_BOOL
	static bool						getValueByNameUsingParamBlock2 (Animatable& node, const char* sName, 
																	ParamType2 type, void *pValue, TimeValue time, bool verbose = true);

	// Get the first modifier in the pipeline of a node by its class identifier
	static Modifier*				getModifier (INode* pNode, Class_ID modCID);

	// Get the ambient value
	NLMISC::CRGBA					getAmbientColor (TimeValue time);

	// Get the backgorund value
	NLMISC::CRGBA					getBackGroundColor (TimeValue time);

	// Get the light group
	static uint						getLightGroup (INode *node);

	// Get the light animation name
	static std::string				getAnimatedLight (INode *node);

	// *** Script access

	// Eval a scripted function
	static bool						scriptEvaluate (const char *script, void *out, TNelScriptValueType type);

	// *** Appdata access

	// Get an appData float
	static float 					getScriptAppData (Animatable *node, uint32 id, float def);

	// Set an appData float
	static void						setScriptAppData (Animatable *node, uint32 id, float value);

	// Get an appData integer
	static int						getScriptAppData (Animatable *node, uint32 id, int def);

	// Set an appData integer
	static void						setScriptAppData (Animatable *node, uint32 id, int value);

	// Get an appData integer
	static std::string				getScriptAppData (Animatable *node, uint32 id, const std::string& def);

	// Set an appData integer
	static void						setScriptAppData (Animatable *node, uint32 id, const std::string& value);

	// Get an appData RGBA
	static NLMISC::CRGBA			getScriptAppData (Animatable *node, uint32 id, NLMISC::CRGBA def);
	
	// Set an appData RGBA
	static void						setScriptAppData (Animatable *node, uint32 id, NLMISC::CRGBA val);
	
	// Output error message
	void							outputErrorMessage (const std::string &message);
	void							outputWarningMessage(const std::string &message);


	// Get an appData VertexProgram WindTree (ensure same default values for all retrieve).
	static void						getScriptAppDataVPWT (Animatable *node, CVPWindTreeAppData &apd);

	// Set an appData VertexProgram WindTree.
	static void						setScriptAppDataVPWT (Animatable *node, const CVPWindTreeAppData &apd);

	/** private func : this convert a polygon expressed as a max mesh into a list of ordered vectors.
	  * This also gives an average normal by averaging faces normals.
	  */
	static void						maxPolygonMeshToOrderedPoly(Mesh &mesh, std::vector<NLMISC::CVector> &dest, const NLMISC::CMatrix &basis, NLMISC::CVector &avgNormal);


	// ********************
	// *** Export Vegetable
	// ********************

	/* 
	 * Build a skeleton shape
	 *
	 * mapBindPos is the pointer of the map of bind pos by bone. Can be NULL if the skeleton is already in the bind pos.
	 */
	bool							buildVegetableShape (NL3D::CVegetableShape& skeletonShape, INode& node, TimeValue time);


	// ********************
	// *** Export Lod Character
	// ********************

	/* is this node a lod character ???
	 */
	static bool						isLodCharacter (INode& node, TimeValue time);

	/* 
	 * Build a lod character from a node
	 */
	bool							buildLodCharacter (NL3D::CLodCharacterShapeBuild& lodBuild, INode& node, TimeValue time, const TInodePtrInt *nodeMap);

	// *************
	// *** Misc  ***
	// *************

	// get a ptr to max interface
	Interface					  *getInterface() const { return _Ip; }	

	/// Return true if error must be in dialog, false if error must not stop the process.
	bool		isErrorInDialog () const;

private:

	// A class that describe how build a NeL material with a max one
	class CMaterialDesc
	{
	public:
		friend class CExportNel;

		// Default constructor init _IndexInMaxMaterial to MISSING_VALUE, _UVMatrix to indentity and clamp to 0, 0, 1, 1
		CMaterialDesc ()
		{
			_IndexInMaxMaterial=UVGEN_MISSING;
			_IndexInMaxMaterialAlternative=UVGEN_MISSING;
			_UVMatrix.IdentityMatrix();
			_CropU=0.f;
			_CropV=0.f;
			_CropW=1.f;
			_CropH=1.f;
		}
		const Matrix3 &getUVMatrix() const { return _UVMatrix; }
	private:
		// *** Data

		// Index in material
		sint		_IndexInMaxMaterial;
		sint		_IndexInMaxMaterialAlternative;

		// Matrix for UVs
		Matrix3		_UVMatrix;

		// Crop region
		float		_CropU;
		float		_CropV;
		float		_CropW;
		float		_CropH;
	};

	// Max material info
	class CMaxMaterialInfo
	{
	public:
		// Default constructor
		CMaxMaterialInfo ()
		{
			AlphaVertex = false;
			ColorVertex = false;
			AlphaVertexChannel = 0;
			TextureMatrixEnabled = false;
			uint i;
			for (i=0; i<MAX_MAX_TEXTURE; i++)
				UVRouting[i] = 0xff;
		};

		// Remap UV channel
		std::vector<CMaterialDesc>					RemapChannel;

		// Material names
		std::string									MaterialName;

		// Alpha vertex in this material
		bool										AlphaVertex;

		/* UV channel routing : 
		 * (UVRouting[i] == 0xff)	UV channel i is not needed
		 * (UVRouting[i] != 0xff)	UV channel i is needed for the material
		 * (UVRouting[i] == i)		UV channel i is present in the vertex buffer
		 * (UVRouting[i] != i)		UV channel i is not present in the vertex buffer. It is routed to another UV channel.*/
		uint8										UVRouting[MAX_MAX_TEXTURE];

		// Color vertex in this material
		bool										ColorVertex;

		// Alpha vertex channel for this material
		uint										AlphaVertexChannel;

		// allow to export a user texture matrix
		bool										TextureMatrixEnabled;
	};

	// Max base build structure
	class CMaxMeshBaseBuild
	{
	public:
		CMaxMeshBaseBuild ()
		{
			NeedVertexColor = false;
			uint i;
			for (i=0; i<MAX_MAX_TEXTURE; i++)
				UVRouting[i] = 0xff;
		}

		// First material in the array
		uint										FirstMaterial;

		// Num of materials
		uint										NumMaterials;

		/* UV channel routing : 
		 * (UVRouting[i] == 0xff)	UV channel i is not needed
		 * (UVRouting[i] != 0xff)	UV channel i is needed for the material
		 * (UVRouting[i] == i)		UV channel i is present in the vertex buffer
		 * (UVRouting[i] != i)		UV channel i is not present in the vertex buffer. It is routed to another UV channel.*/
		uint8										UVRouting[MAX_MAX_TEXTURE];

		// Need vertex color
		bool										NeedVertexColor;

		// Remap UV channel
		std::vector<CMaxMaterialInfo>				MaterialInfo;
	};

	// *********************
	// *** Export mesh
	// *********************

	// Get 3ds UVs channel used by a texmap and make a good index channel
	// Can return an interger in [0; MAX_MESHMAPS-1] or on of the following value: UVGEN_REFLEXION, UVGEN_MISSING
	static int						getVertMapChannel (Texmap& texmap, Matrix3& channelMatrix, TimeValue time);

	// Build a CLight
	static bool						buildLight (GenLight &maxLight, NL3D::CLight& nelLight, INode& node, TimeValue time);

	/**
	  * Build a NeL base mesh interface
	  *
	  * if skeletonShape is NULL, no skinning is exported.
	  */
	void							buildBaseMeshInterface (NL3D::CMeshBase::CMeshBaseBuild& buildMesh, CMaxMeshBaseBuild& maxBaseBuild, INode& node, 
															TimeValue time, const NLMISC::CMatrix& basis);

	/**
	  * Build a NeL mesh interface
	  *
	  * if skeletonShape is NULL, no skinning is exported.
	  * if isMorphTarget is true, no "mrm/normal mesh interface" is built
	  */
	void							buildMeshInterface (TriObject &tri, NL3D::CMesh::CMeshBuild& buildMesh, const NL3D::CMeshBase::CMeshBaseBuild& buildBaseMesh, 
														const CMaxMeshBaseBuild& maxBaseBuild, INode& node, TimeValue time, const TInodePtrInt* nodeMap, 
														const NLMISC::CMatrix& newBasis=NLMISC::CMatrix::Identity, 
														const NLMISC::CMatrix& finalSpace=NLMISC::CMatrix::Identity,
														bool isMorphTarget= false);


	/**
	  * Get all the blend shapes from a node in the meshbuild form
	  */
	void							getBSMeshBuild (std::vector<NL3D::CMesh::CMeshBuild*> &bsList, INode &node, TimeValue time, bool skined);
	
	/**
	  * Build a NeL mrm parameters block
	  */
	static void						buildMRMParameters (Animatable& node, NL3D::CMRMParameters& params);

	/**
	  * Build a mesh geom with a node
	  */
	NL3D::IMeshGeom					*buildMeshGeom (INode& node, TimeValue time, const TInodePtrInt *nodeMap,
													NL3D::CMeshBase::CMeshBaseBuild &buildBaseMesh, std::vector<std::string>& listMaterialName,
													bool& isTransparent, bool& isOpaque, const NLMISC::CMatrix& parentMatrix);
	/**
	  * Build the mesh morpher info in the mesh geom	  */
	void							buildMeshMorph (NL3D::CMesh::CMeshBuild& buildMesh, INode &node, TimeValue time, bool skined);

	// Get the normal of a face for a given corner in localSpace
	static Point3					getLocalNormal (int face, int corner, Mesh& mesh);

	// Build a water shape. The given node must have a water materiel, an assertion is raised otherwise
	NL3D::IShape					*buildWaterShape(INode& node, TimeValue time);

	// build a wave maker shape
	NL3D::IShape					*buildWaveMakerShape(INode& node, TimeValue time);

	

	// *********************
	// ***  Export FXs   ***
	// *********************
	// Build shape from a particle system node
	NL3D::IShape					*buildParticleSystem(INode& node, TimeValue time);
	// Build shape from a remanence node
	NL3D::IShape					*buildRemanence(INode& node, TimeValue time);
	// Build shape from a flare node
	NL3D::IShape					*buildFlare(INode& node, TimeValue time);


	// *********************
	// *** Export material
	// *********************

	/** Test wether the given max node has a water material. A water object should only have one material, and must have planar, convex geometry.
	* Morevover, the mapping should only have scale and offsets, no rotation
	*/
	static bool						hasWaterMaterial(INode& node, TimeValue time);

	// Build an array of NeL material corresponding with max material at this node.
	void							buildMaterials (std::vector<NL3D::CMaterial>& Materials, CMaxMeshBaseBuild& maxBaseBuild, INode& node, 
													TimeValue time);

	// Build a NeL material corresponding with a max material.
	void							buildAMaterial (NL3D::CMaterial& material, CMaxMaterialInfo& materialInfo, Mtl& mtl, TimeValue time);

	// Build a NeL texture corresponding with a max Texmap.
	NL3D::ITexture*					buildATexture (Texmap& texmap, CMaterialDesc& remap3dsTexChannel, TimeValue time, bool forceCubic=false);

	/// Build a NeL texture cube from a Reflect/refract map containing 6 textures.
	NL3D::CTextureCube				*buildTextureCubeFromReflectRefract(Texmap &texmap, TimeValue time);

	/// Build a NeL texture cube from a Composite map containing 6 textures. (note: no re-ordering is done, because it didn't reorder composite cube in previous version either).
	NL3D::CTextureCube				*buildTextureCubeFromComposite(Texmap &texmap, TimeValue time);

	/// Build a NeL texture cube from a single texture.
	NL3D::CTextureCube				*buildTextureCubeFromTexture(Texmap &texmap, TimeValue time);

public:
	 /** Return true if a mesh has a material whose shader requires a specific vertex shader to work (for example, per-pixel lighting).
	   * This also stores the result in shader
	   */
	 static bool					hasMaterialWithShaderForVP(INode &node, TimeValue time, NL3D::CMaterial::TShader &shader);

	/// Test wether the given material need a specific vertex program to work correctly
	 static	bool                      needVP(Mtl &mat, TimeValue time, NL3D::CMaterial::TShader &shader);
private:

	/** Build a mesh vertex program associated with the given shader
	  * For now there can be only one vertex program per mesh, so you must specify the shader this v.p must be built for.
	  * The meshBuild may be modfied by this operation (to add tangent space infos for example)
	  * \return The vertex program or NULL if the op failed, or if there's no need for a v.p.
	  */
	 static NL3D::IMeshVertexProgram           *buildMeshMaterialShaderVP(NL3D::CMaterial::TShader shader, NL3D::CMesh::CMeshBuild *mb);

	/// Test wether a material need a vertex program

	// *********************
	// *** Export Animation
	// *********************

	// Add tracks for the node
	void							addNodeTracks (NL3D::CAnimation& animation, INode& node, const char* parentName,
													CAnimationBuildCtx	*animBuildCtx, bool root, CSSSBuild &ssBuilder,
													bool bodyBiped=false);

	// Add tracks for the light
	void							addLightTracks (NL3D::CAnimation& animation, INode& node, const char* parentName);

	// Add tracks for the morphing
	void							addMorphTracks (NL3D::CAnimation& animation, INode& node, const char* parentName);
	
	// Add tracks for the object
	void							addObjTracks (NL3D::CAnimation& animation, Object& obj, const char* parentName);

	// Add tracks for the material
	void							addMtlTracks (NL3D::CAnimation& animation, Mtl& mtl, const char* parentName);

	// Add tracks for the texture
	void							addTexTracks (NL3D::CAnimation& animation, Texmap& tex, uint stage, const char* parentName);


	// Get a biped key parameter using script
	bool							getBipedKeyInfo (const char* nodeName, const char* paramName, uint key, float& res);

	// Get inplace biped mode
	bool							getBipedInplaceMode (const char* nodeName, const char* inplaceFunction, 
															bool &res);

	// Change inplace biped mode
	bool							setBipedInplaceMode (const char* nodeName, const char* inplaceFunction, 
															bool onOff);

	// Current context for addAnimation()
	class	CAnimationBuildCtx
	{
	public:
		struct	CBipedKey
		{
			TimeValue			Time;
			// The correct Nel Local TM
			NLMISC::CVector		Pos;
			NLMISC::CQuat		Quat;
			NLMISC::CVector		Scale;
		};

		struct	CBipedNode
		{
			INode							*Node;
			// Oversampled position
			std::vector<CBipedKey>			Keys;
		};

		// The animation range for Biped Nodes. BipedRangeMin and BipedRangeMax are inclusive
		TimeValue					BipedRangeMin;
		TimeValue					BipedRangeMax;

		// All biped nodes we must export.
		std::vector<CBipedNode>		BipedNodes;

	public:
		CAnimationBuildCtx();
		
		bool			hasBipedNodes() const {return BipedNodes.size()>0;}

		// compile the bipedMap from BipedNodes, and other infos
		void			compileBiped();

		// From a node, get its associated CBipedNode in the BipedNodes array.
		CBipedNode		*getBipedNodeInfo(INode *node);

		/* return true if this node must export Track Position. It returns true for bodyBiped and LClavicle and RClavicle
			limb for now.
			NB: We use script to now if the node is a clavicle (ie not hack-tested by node name)
			But this slow test is done in compileBiped().
		*/
		bool			mustExportBipedBonePos(INode *node);

	private:
		// Map from INode * to CBiped Id in BipedNodes
		typedef	std::map<INode*, uint>	TBipedMap;
		TBipedMap						_BipedMap;

		// List of node which must export their position.
		typedef	std::set<INode*>		TBipedSet;
		TBipedSet						_ExportBipedBonePosSet;

		// Add a limb node to the _ExportBipedBonePosSet. eg: #larm.
		void							addLimbNodeToExportPos(INode *rootNode, const char *limbId);
	};

	// Bkup ctx of a biped bone.
	struct	CBipedNodePlaceMode
	{
		INode				*Node;
		bool				InPlaceMode;
		bool				InPlaceYMode;
		bool				InPlaceXMode;
	};


	// 
	void			buildBipedInformation(CAnimationBuildCtx &animBuildCtx, INode &node);
	// 
	void			overSampleBipedAnimation(CAnimationBuildCtx &animBuildCtx, uint overSampleValue);


	/// return the Nel Scale Reference node if any.
	INode	*getNELScaleReferenceNode(INode &node);

	/// return true if the node has a biped controller (either body or slave).
	bool		isBipedNode(INode &node);

	/// As part of getNELBoneLocalTM.
	void		getNELBoneLocalScale(INode &node, TimeValue time, NLMISC::CVector &nelScale);

private:

	// Pointer on the interface
	Interface						*_Ip;

	// Texture are built path absolute
	bool							_AbsolutePath;

	// Build to view the scene
	bool							_View;

	// Errors goes in dialog
	bool							_ErrorInDialog;

	// Error title
	std::string						_ErrorTitle;

	// Build options
	CExportNelOptions				_Options;
};

/** replacment for sprintf scanf (because of localisation in max)
  */
float		toFloatMax(const std::string &src);
float		toFloatMax(const TCHAR *src);
// Same as to float max, but returns true if succeed
bool		toFloatMax(const std::string &src, float &dest);
bool		toFloatMax(const TCHAR *src, float &dest);
std::string toStringMax(float value);
std::string toStringMax(int value);




#endif // NL_EXPORT_NEL_H

/* End of export_nel.h */
