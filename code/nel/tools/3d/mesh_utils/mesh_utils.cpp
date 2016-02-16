// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2015  Winch Gate Property Limited
// Author: Jan Boon <jan.boon@kaetemi.be>
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

#include <nel/misc/types_nl.h>
#include "mesh_utils.h"

#include <nel/misc/debug.h>
#include <nel/pipeline/tool_logger.h>
#include <nel/pipeline/project_config.h>
#include <nel/misc/sstring.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

#include <nel/3d/shape.h>
#include <nel/3d/mesh.h>
#include <nel/3d/texture_file.h>

#include "scene_meta.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#define NL_NODE_INTERNAL_TYPE aiNode
#define NL_SCENE_INTERNAL_TYPE aiScene
#include "scene_context.h"

#include "assimp_material.h"
#include "assimp_shape.h"

CMeshUtilsSettings::CMeshUtilsSettings()
{
	/*ShapeDirectory = "shape";
	IGDirectory = "ig";
	SkelDirectory = "skel";*/
}

void importShapes(CMeshUtilsContext &context, const aiNode *node)
{
	if (node != context.InternalScene->mRootNode)
	{
		CNodeContext &nodeContext = context.Nodes[node->mName.C_Str()];
		CNodeMeta &nodeMeta = context.SceneMeta.Nodes[node->mName.C_Str()];
		if (nodeMeta.ExportMesh == TMeshShape && nodeMeta.InstanceName.empty())
		{
			if (node->mNumMeshes)
			{
				nldebug("Shape '%s' found containing '%u' meshes", node->mName.C_Str(), node->mNumMeshes);
				assimpShape(context, nodeContext);
			}
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		importShapes(context, node->mChildren[i]);
}

void validateInternalNodeNames(CMeshUtilsContext &context, const aiNode *node)
{
	if (!node->mParent || node == context.InternalScene->mRootNode)
	{
		// do nothing
	}
	else if (node->mName.length == 0)
	{
		tlwarning(context.ToolLogger, context.Settings.SourceFilePath.c_str(), 
			"Node has no name");
	}
	else
	{
		CNodeContext &nodeContext = context.Nodes[node->mName.C_Str()];

		if (nodeContext.InternalNode && nodeContext.InternalNode != node)
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(), 
				"Node name '%s' appears multiple times", node->mName.C_Str());
		}
		else
		{
			nodeContext.InternalNode = node;
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		validateInternalNodeNames(context, node->mChildren[i]);
}

void flagAssimpBones(CMeshUtilsContext &context)
{
	// Find out which nodes are bones by checking the mesh meta info
	const aiScene *scene = context.InternalScene;
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		// nldebug("FOUND MESH '%s'\n", scene->mMeshes[i]->mName.C_Str());
		const aiMesh *mesh = scene->mMeshes[i];
		for (unsigned int j = 0; j < mesh->mNumBones; ++j)
		{
			CNodeContext &nodeContext = context.Nodes[mesh->mBones[j]->mName.C_Str()];
			if (!nodeContext.InternalNode)
			{
				tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(), 
					"Bone '%s' has no associated node", mesh->mBones[j]->mName.C_Str());
			}
			else
			{
				// Flag as bone
				nodeContext.IsBone = true;

				// Flag all parents as bones
				/*const aiNode *parent = nodeContext.InternalNode;
				while (parent = parent->mParent) if (parent->mName.length)
				{
					context.Nodes[parent->mName.C_Str()].IsBone = true;
				}*/
			}
		}
	}

	// Find out which nodes are bones by checking the animation info
	// TODO
}

void flagRecursiveBones(CMeshUtilsContext &context, CNodeContext &nodeContext, bool autoStop = false)
{
	nodeContext.IsBone = true;
	const aiNode *node = nodeContext.InternalNode;
	nlassert(node);
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		CNodeContext &ctx = context.Nodes[node->mName.C_Str()];
		if (autoStop && ctx.IsBone)
			continue;
		flagRecursiveBones(context, ctx);
	}
}

void flagMetaBones(CMeshUtilsContext &context)
{
	for (TNodeContextMap::iterator it(context.Nodes.begin()), end(context.Nodes.end()); it != end; ++it)
	{
		CNodeContext &ctx = it->second;
		CNodeMeta &meta = context.SceneMeta.Nodes[it->first];
		if (meta.ExportBone == TBoneForce)
			ctx.IsBone = true;
		else if (meta.ExportBone == TBoneRoot)
			flagRecursiveBones(context, ctx);
	}
}

void flagLocalParentBones(CMeshUtilsContext &context, CNodeContext &nodeContext)
{
	const aiNode *node = nodeContext.InternalNode;
}

void flagAllParentBones(CMeshUtilsContext &context, CNodeContext &nodeContext, bool autoStop = false)
{
	const aiNode *parent = nodeContext.InternalNode;
	while (parent = parent->mParent) if (parent->mName.length && parent != context.InternalScene->mRootNode)
	{
		CNodeContext &ctx = context.Nodes[parent->mName.C_Str()];
		if (autoStop && ctx.IsBone)
			break;
		ctx.IsBone = true;
	}
}

bool hasIndirectParentBone(CMeshUtilsContext &context, CNodeContext &nodeContext)
{
	const aiNode *parent = nodeContext.InternalNode;
	while (parent = parent->mParent) if (parent->mName.length && parent != context.InternalScene->mRootNode)
		if (context.Nodes[parent->mName.C_Str()].IsBone) return true;
	return false;
}

void flagExpandedBones(CMeshUtilsContext &context)
{
	switch (context.SceneMeta.SkeletonMode)
	{
	case TSkelLocal:
		for (TNodeContextMap::iterator it(context.Nodes.begin()), end(context.Nodes.end()); it != end; ++it)
		{
			CNodeContext &nodeContext = it->second;
			if (nodeContext.IsBone && hasIndirectParentBone(context, nodeContext))
				flagAllParentBones(context, nodeContext, true);
		}
		break;
	case TSkelRoot:
		for (TNodeContextMap::iterator it(context.Nodes.begin()), end(context.Nodes.end()); it != end; ++it)
		{
			CNodeContext &nodeContext = it->second;
			if (nodeContext.IsBone)
				flagAllParentBones(context, nodeContext, true);
		}
		break;
	case TSkelFull:
		for (TNodeContextMap::iterator it(context.Nodes.begin()), end(context.Nodes.end()); it != end; ++it)
		{
			CNodeContext &nodeContext = it->second;
			if (nodeContext.IsBone)
				flagAllParentBones(context, nodeContext, true);
		}
		for (TNodeContextMap::iterator it(context.Nodes.begin()), end(context.Nodes.end()); it != end; ++it)
		{
			CNodeContext &nodeContext = it->second;
			if (nodeContext.IsBone)
				flagRecursiveBones(context, nodeContext, true);
		}
		break;
	}
}

void exportShapes(CMeshUtilsContext &context)
{
	for (TNodeContextMap::iterator it(context.Nodes.begin()), end(context.Nodes.end()); it != end; ++it)
	{
		CNodeContext &nodeContext = it->second;
		if (nodeContext.Shape)
		{
			std::string shapePath = NLMISC::CPath::standardizePath(context.Settings.DestinationDirectoryPath, true) + it->first + ".shape";
			context.ToolLogger.writeDepend(NLPIPELINE::BUILD, shapePath.c_str(), "*");
			NLMISC::COFile f;
			if (f.open(shapePath, false, false, true))
			{
				try
				{
					NL3D::CShapeStream shapeStream(nodeContext.Shape);
					shapeStream.serial(f);
					f.close();
				}
				catch (...)
				{
					tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
						"Shape '%s' serialization failed!", it->first.c_str());
				}
			}
			if (NL3D::CMeshBase *mesh = dynamic_cast<NL3D::CMeshBase *>(nodeContext.Shape.getPtr()))
			{
				for (uint mi = 0; mi < mesh->getNbMaterial(); ++mi)
				{
					NL3D::CMaterial &mat = mesh->getMaterial(mi);
					for (uint ti = 0; ti < NL3D::IDRV_MAT_MAXTEXTURES; ++ti)
					{
						if (NL3D::ITexture *itex = mat.getTexture(ti))
						{
							if (NL3D::CTextureFile *tex = dynamic_cast<NL3D::CTextureFile *>(itex))
							{
								std::string fileName = tex->getFileName();
								std::string knownPath = NLMISC::CPath::lookup(fileName, false, false, false);
								if (!knownPath.empty())
								{
									context.ToolLogger.writeDepend(NLPIPELINE::RUNTIME, shapePath.c_str(), knownPath.c_str());
								}
								else
								{
									// TODO: Move this warning into nelmeta serialization so it's shown before export
									tlwarning(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
										"Texture '%s' referenced in material but not found in the database search paths", fileName.c_str());
								}
							}
						}
					}
				}
			}
		}
	}
}

// TODO: Separate load scene and save scene functions
int exportScene(const CMeshUtilsSettings &settings)
{
	CMeshUtilsContext context(settings);
	NLMISC::CFile::createDirectoryTree(settings.DestinationDirectoryPath);

	if (!settings.ToolDependLog.empty())
		context.ToolLogger.initDepend(settings.ToolDependLog);
	if (!settings.ToolErrorLog.empty())
		context.ToolLogger.initError(settings.ToolErrorLog);
	context.ToolLogger.writeDepend(NLPIPELINE::BUILD, "*", NLMISC::CPath::standardizePath(context.Settings.SourceFilePath, false).c_str()); // Base input file

	// Apply database configuration
	if (!NLPIPELINE::CProjectConfig::init(settings.SourceFilePath, 
		NLPIPELINE::CProjectConfig::DatabaseTextureSearchPaths,
		true))
	{
		tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(), "Unable to find database.cfg in input path or any of its parents.");
		// return EXIT_FAILURE; We can continue but the output will not be guaranteed...
	}

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(settings.SourceFilePath, 0
		| aiProcess_Triangulate 
		| aiProcess_ValidateDataStructure
		| aiProcess_GenNormals // Or GenSmoothNormals? TODO: Validate smoothness between material boundaries!
		); // aiProcess_SplitLargeMeshes | aiProcess_LimitBoneWeights
	if (!scene)
	{
		const char *errs = importer.GetErrorString();
		if (errs) tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(), "Assimp failed to load the scene: '%s'", errs);
		else tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(), "Unable to load scene");
		return EXIT_FAILURE;
	}
	// aiProcess_Triangulate
	// aiProcess_ValidateDataStructure: TODO: Catch Assimp error output stream
	// aiProcess_RemoveRedundantMaterials: Not used because we may override materials with NeL Material from meta
	// aiProcess_ImproveCacheLocality: TODO: Verify this does not modify vertex indices
	//scene->mRootNode->mMetaData

	context.InternalScene = scene;
	if (context.SceneMeta.load(context.Settings.SourceFilePath))
		context.ToolLogger.writeDepend(NLPIPELINE::BUILD, "*", context.SceneMeta.metaFilePath().c_str()); // Meta input file

	validateInternalNodeNames(context, context.InternalScene->mRootNode);

	// -- SKEL FLAG --
	flagAssimpBones(context);
	flagMetaBones(context);
	flagExpandedBones(context);
	// TODO
	// [
	// Only necessary in TSkelLocal
	// For each shape test if all the bones have the same root bones for their skeleton
	// 1) Iterate each until a different is found
	// 2) When a different root is found, connect the two to the nearest common bone
	// ]
	// -- SKEL FLAG --

	// First import materials
	assimpMaterials(context);

	// Import shapes
	importShapes(context, context.InternalScene->mRootNode);

	// Export shapes
	exportShapes(context);

	return EXIT_SUCCESS;
}

/* end of file */
