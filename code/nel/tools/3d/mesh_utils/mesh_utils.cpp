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
#include <nel/misc/tool_logger.h>
#include <nel/misc/sstring.h>

#include "database_config.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

CMeshUtilsSettings::CMeshUtilsSettings()
{
	/*ShapeDirectory = "shape";
	IGDirectory = "ig";
	SkelDirectory = "skel";*/
}

struct CNodeContext
{
	CNodeContext() :
		AssimpNode(NULL),
		IsBone(false)
	{

	}

	const aiNode *AssimpNode;
	bool IsBone;
};

struct CMeshUtilsContext
{
	CMeshUtilsContext(const CMeshUtilsSettings &settings) : Settings(settings), AssimpScene(NULL)
	{

	}

	const CMeshUtilsSettings &Settings;
	
	NLMISC::CToolLogger ToolLogger;

	const aiScene *AssimpScene;
	std::map<NLMISC::CSString, CNodeContext> Nodes;
	std::map<const aiMesh *, NLMISC::CSString> MeshNames; // Maps meshes to a node name ********************* todo ***************
};

struct CNodeMeta
{
	// TODO
};
static const CNodeMeta g_DefaultMeta;

void importNode(CMeshUtilsContext &context, const aiNode *node, const CNodeMeta *meta)
{
	if (node->mNumMeshes)
	{
		// TODO
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		importNode(context, node->mChildren[i], &g_DefaultMeta);
}

void validateAssimpNodeNames(CMeshUtilsContext &context, const aiNode *node)
{
	if (!node->mParent || node == context.AssimpScene->mRootNode)
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

		if (nodeContext.AssimpNode && nodeContext.AssimpNode != node)
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(), 
				"Node name '%s' appears multiple times", node->mName.C_Str());
		}
		else
		{
			nodeContext.AssimpNode = node;
		}
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
		validateAssimpNodeNames(context, node->mChildren[i]);
}

void flagAssimpBones(CMeshUtilsContext &context)
{
	// Find out which nodes are bones by checking the mesh meta info
	const aiScene *scene = context.AssimpScene;
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		// nldebug("FOUND MESH '%s'\n", scene->mMeshes[i]->mName.C_Str());
		const aiMesh *mesh = scene->mMeshes[i];
		for (unsigned int j = 0; j < mesh->mNumBones; ++j)
		{
			CNodeContext &nodeContext = context.Nodes[mesh->mBones[j]->mName.C_Str()];
			if (!nodeContext.AssimpNode)
			{
				tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(), 
					"Bone '%s' has no associated node", mesh->mBones[j]->mName.C_Str());
			}
			else
			{
				// Flag as bone
				nodeContext.IsBone = true;

				// Flag all parents as bones
				/*const aiNode *parent = nodeContext.AssimpNode;
				while (parent = parent->mParent) if (parent->mName.length)
				{
					context.Nodes[parent->mName.C_Str()].IsBone = true;
				}*/
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
	context.ToolLogger.writeDepend(NLMISC::BUILD, "*", context.Settings.SourceFilePath.c_str()); // Base input file

	// Apply database configuration
	CDatabaseConfig::init(settings.SourceFilePath);
	CDatabaseConfig::initTextureSearchDirectories();

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(settings.SourceFilePath, aiProcess_Triangulate | aiProcess_ValidateDataStructure); // aiProcess_SplitLargeMeshes | aiProcess_LimitBoneWeights
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

	context.AssimpScene = scene;

	validateAssimpNodeNames(context, context.AssimpScene->mRootNode);
	flagAssimpBones(context);

	importNode(context, scene->mRootNode, &g_DefaultMeta);

	return EXIT_SUCCESS;
}

/* end of file */
