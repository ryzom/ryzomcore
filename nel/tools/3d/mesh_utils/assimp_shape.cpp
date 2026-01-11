// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2015  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2016  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
//
// Author: Jan BOON (Kaetemi) <jan.boon@kaetemi.be>

#include <nel/misc/types_nl.h>
#include "assimp_shape.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#define NL_NODE_INTERNAL_TYPE aiNode
#define NL_SCENE_INTERNAL_TYPE aiScene
#include "scene_context.h"

#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/pipeline/tool_logger.h>

#include <nel/3d/mesh.h>

#include "assimp_material.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

// TODO: buildParticleSystem ??
// TODO: buildWaveMakerShape ??
// TODO: buildRemanence ??
// TODO: buildFlare ??
// Probably specific settings we can only do in meta editor on a dummy node..
// TODO: pacs prim

// TODO: buildWaterShape specifics when node has water material

// TODO: CMeshMultiLod::CMeshMultiLodBuild multiLodBuild; export_mesh.cpp ln 228
// TODO: LOD MRM

// TODO: Skinned - reverse transform by skeleton root bone to align?

/*inline CMatrix convMatrix(const aiMatrix4x4 &tf)
{
	CMatrix m;
	for (int i = 0; i < 16; ++i)
		m.set(&tf.a1);
	return m;
}*/

inline CVector convVector(const aiVector3D &av)
{
	return CVector(av.x, av.y, av.z); // COORDINATE CONVERSION
}

inline CRGBA convColor(const aiColor4D &ac)
{
	return CRGBA(ac.r * 255.99f, ac.g * 255.99f, ac.b * 255.99f, ac.a * 255.99f);
}

inline CUVW convUvw(const aiVector3D &av)
{
	return CUVW(av.x, -av.y, av.z); // UH OH COORDINATE CONVERSION ?! ONLY FOR TEXTURES !!
}

inline CQuat convQuat(const aiQuaternion &aq)
{
	return CQuat(aq.x, aq.y, aq.z, aq.w);
}

void assimpBuildBaseMesh(CMeshBase::CMeshBaseBuild &buildBaseMesh, CMeshUtilsContext &context, CNodeContext &nodeContext)
{
	const aiNode *node = nodeContext.InternalNode;
	// Reference CExportNel::buildBaseMeshInterface

	// Load materials
	buildBaseMesh.Materials.resize(node->mNumMeshes);

	for (unsigned int mi = 0; mi < node->mNumMeshes; ++mi)
	{
		const aiMesh *mesh = context.InternalScene->mMeshes[node->mMeshes[mi]];
		const aiMaterial *am = context.InternalScene->mMaterials[mesh->mMaterialIndex];

		aiString amname;
		if (am->Get(AI_MATKEY_NAME, amname) != aiReturn_SUCCESS)
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"Material used by node '%s' has no name", node->mName.C_Str()); // TODO: Maybe autogen names by index in mesh or node if this is actually a thing
			assimpMaterial(buildBaseMesh.Materials[mi], context, am);
		}
		else
		{
			buildBaseMesh.Materials[mi] = *context.SceneMeta.Materials[amname.C_Str()];
		}
	}

	// Positioning
	const aiMatrix4x4 &root = context.InternalScene->mRootNode->mTransformation;
	const aiMatrix4x4 &tf = nodeContext.InternalNode->mTransformation; // COORDINATE CONVERSION HERE INSTEAD OF PER VERTEX ??
	aiVector3D scaling;
	aiQuaternion rotation;
	aiVector3D position;
	tf.Decompose(scaling, rotation, position);
	buildBaseMesh.DefaultScale = convVector(scaling);
	buildBaseMesh.DefaultRotQuat = convQuat(rotation);
	buildBaseMesh.DefaultRotEuler = CVector(0, 0, 0);
	buildBaseMesh.DefaultPivot = CVector(0, 0, 0);
	buildBaseMesh.DefaultPos = convVector(position);
	if (buildBaseMesh.DefaultScale.x != 1.0f || buildBaseMesh.DefaultScale.y != 1.0f || buildBaseMesh.DefaultScale.z != 1.0f)
	{
		tlmessage(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
			"Node '%s' has a scaled transformation. This may be a mistake", node->mName.C_Str());
	}

	// Meta
	// dst.CollisionMeshGeneration = src.CollisionMeshGeneration;

	// TODO: Morph
}

bool assimpBuildMesh(CMesh::CMeshBuild &buildMesh, CMeshBase::CMeshBaseBuild &buildBaseMesh, CMeshUtilsContext &context, CNodeContext &nodeContext)
{
	// TODO
	// *** If the mesh is skined, vertices will be exported in world space.
	// *** If the mesh is not skined, vertices will be exported in offset space.

	// TODO Support skinning

	const aiNode *node = nodeContext.InternalNode;
	nlassert(node->mNumMeshes);

	// Basic validations before processing starts
	for (unsigned int mi = 0; mi < node->mNumMeshes; ++mi)
	{
		// TODO: Maybe needs to be the same count too for all meshes, so compare with mesh 0
		const aiMesh *mesh = context.InternalScene->mMeshes[node->mMeshes[mi]];
		if (mesh->GetNumColorChannels() > 2)
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"(%s) mesh->GetNumColorChannels() > 2", node->mName.C_Str());
			return false;
		}
		if (mesh->GetNumUVChannels() > CVertexBuffer::MaxStage)
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"(%s) mesh->GetNumUVChannels() > CVertexBuffer::MaxStage", node->mName.C_Str());
			return false;
		}
		if (!mesh->HasNormals())
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"(%s) !mesh->HasNormals()", node->mName.C_Str());
			return false;
		}
	}

	// Default vertex flags
	buildMesh.VertexFlags = CVertexBuffer::PositionFlag | CVertexBuffer::NormalFlag;

	// TODO: UV Channels routing to correct texture stage
	for (uint i = 0; i < CVertexBuffer::MaxStage; ++i)
		buildMesh.UVRouting[i] = i;

	// Meshes in assimp are separated per material, so we need to re-merge them for the mesh build process
	// This process also deduplicates vertices
	bool cleanupMesh = true;
	sint32 numVertices = 0;
	for (unsigned int mi = 0; mi < node->mNumMeshes; ++mi)
		numVertices += context.InternalScene->mMeshes[node->mMeshes[mi]]->mNumVertices;
	buildMesh.Vertices.resize(numVertices);
	numVertices = 0;
	map<CVector, sint32> vertexIdentifiers;
	vector<vector<sint32> > vertexRemapping;
	vertexRemapping.resize(node->mNumMeshes);
	for (unsigned int mi = 0; mi < node->mNumMeshes; ++mi)
	{
		const aiMesh *mesh = context.InternalScene->mMeshes[node->mMeshes[mi]];
		vertexRemapping[mi].resize(mesh->mNumVertices);
		for (unsigned int vi = 0; vi < mesh->mNumVertices; ++vi)
		{
			CVector vec = convVector(mesh->mVertices[vi]);
			map<CVector, sint32>::iterator vecit = vertexIdentifiers.find(vec);
			if (vecit == vertexIdentifiers.end())
			{
				buildMesh.Vertices[numVertices] = vec;
				if (cleanupMesh) vertexIdentifiers[vec] = numVertices; // Don't remap if we don't wan't to lose vertex indices
				vertexRemapping[mi][vi] = numVertices;
				++numVertices;
			}
			else
			{
				vertexRemapping[mi][vi] = vecit->second;
			}
		}
	}
	buildMesh.Vertices.resize(numVertices);

	// Process all faces
	// WONT IMPLEMENT: Radial faces generation... is linked to smoothing group... 
	// leave radial normals generation to modeling tool for now...
	sint32 numFaces = 0;
	for (unsigned int mi = 0; mi < node->mNumMeshes; ++mi)
		numFaces += context.InternalScene->mMeshes[node->mMeshes[mi]]->mNumFaces;
	buildMesh.Faces.resize(numFaces);
	numFaces = 0;
	unsigned int refNumColorChannels = context.InternalScene->mMeshes[node->mMeshes[0]]->GetNumColorChannels();
	unsigned int refNumUVChannels = context.InternalScene->mMeshes[node->mMeshes[0]]->GetNumUVChannels();
	for (unsigned int mi = 0; mi < node->mNumMeshes; ++mi)
	{
		const aiMesh *mesh = context.InternalScene->mMeshes[node->mMeshes[mi]];

		// Get channel numbers
		unsigned int numColorChannels = mesh->GetNumColorChannels();
		if (numColorChannels > 2)
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"Shape '%s' has too many color channels in mesh %i (%i channels found)", node->mName.C_Str(), mi, numColorChannels);
		}
		if (numColorChannels > 0)
		{
			buildMesh.VertexFlags |= CVertexBuffer::PrimaryColorFlag;
			if (numColorChannels > 1)
			{
				buildMesh.VertexFlags |= CVertexBuffer::SecondaryColorFlag;
			}
		}
		unsigned int numUVChannels = mesh->GetNumUVChannels();
		if (numUVChannels > CVertexBuffer::MaxStage)
		{
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"Shape '%s' has too many uv channels in mesh %i (%i channels found)", node->mName.C_Str(), mi, numUVChannels);
			numUVChannels = CVertexBuffer::MaxStage;
		}
		for (unsigned int ui = 0; ui < numUVChannels; ++ui)
			buildMesh.VertexFlags |= (CVertexBuffer::TexCoord0Flag << ui); // TODO: Coord UV tex stage rerouting

		// TODO: Channels do in fact differ between submeshes, so we need to correctly recount and reroute the materials properly
		if (numColorChannels != refNumColorChannels)
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"Shape '%s' mismatch of nb color channel in mesh '%i', please contact developer", node->mName.C_Str(), mi);
		if (numUVChannels != refNumUVChannels)
			tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
				"Shape '%s' mismatch of nb uv channel in mesh '%i', please contact developer", node->mName.C_Str(), mi);

		for (unsigned int fi = 0; fi < mesh->mNumFaces; ++fi)
		{
			const aiFace &af = mesh->mFaces[fi];
			if (af.mNumIndices != 3)
			{
				tlerror(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
					"(%s) Face %i on mesh %i has %i faces", node->mName.C_Str(), fi, mi, af.mNumIndices);
				continue; // return false; Keep going, just drop the face for better user experience
			}
			if (cleanupMesh)
			{
				if (vertexRemapping[mi][af.mIndices[0]] == vertexRemapping[mi][af.mIndices[1]]
					|| vertexRemapping[mi][af.mIndices[1]] == vertexRemapping[mi][af.mIndices[2]]
					|| vertexRemapping[mi][af.mIndices[2]] == vertexRemapping[mi][af.mIndices[0]])
					continue; // Not a triangle
			}
			CMesh::CFace &face = buildMesh.Faces[numFaces];
			face.MaterialId = mi;
			face.SmoothGroup = 0; // No smoothing groups (bitfield)
			face.Corner[0].Vertex = vertexRemapping[mi][af.mIndices[0]];
			face.Corner[1].Vertex = vertexRemapping[mi][af.mIndices[1]];
			face.Corner[2].Vertex = vertexRemapping[mi][af.mIndices[2]];
			face.Corner[0].Normal = convVector(mesh->mNormals[af.mIndices[0]]);
			face.Corner[1].Normal = convVector(mesh->mNormals[af.mIndices[1]]);
			face.Corner[2].Normal = convVector(mesh->mNormals[af.mIndices[2]]);
			// TODO: If we want normal maps, we need to add tangent vectors to CFace and build process
			// UV channels
			for (unsigned int ui = 0; ui < numUVChannels; ++ui) // TODO: UV Rerouting
			{
				face.Corner[0].Uvws[ui] = convUvw(mesh->mTextureCoords[ui][af.mIndices[0]]);
				face.Corner[1].Uvws[ui] = convUvw(mesh->mTextureCoords[ui][af.mIndices[1]]);
				face.Corner[2].Uvws[ui] = convUvw(mesh->mTextureCoords[ui][af.mIndices[2]]);
			}
			for (unsigned int ui = numUVChannels; ui < CVertexBuffer::MaxStage; ++ui)
			{
				face.Corner[0].Uvws[ui] = CUVW(0, 0, 0);
				face.Corner[1].Uvws[ui] = CUVW(0, 0, 0);
				face.Corner[2].Uvws[ui] = CUVW(0, 0, 0);
			}
			// Primary and secondary color channels
			if (numColorChannels > 0) // TODO: Verify
			{
				face.Corner[0].Color = convColor(mesh->mColors[0][af.mIndices[0]]);
				face.Corner[1].Color = convColor(mesh->mColors[0][af.mIndices[1]]);
				face.Corner[2].Color = convColor(mesh->mColors[0][af.mIndices[2]]);
			}
			else
			{
				face.Corner[0].Color = CRGBA(255, 255, 255, 255);
				face.Corner[1].Color = CRGBA(255, 255, 255, 255);
				face.Corner[2].Color = CRGBA(255, 255, 255, 255);
			}
			if (numColorChannels > 1) // TODO: Verify
			{
				face.Corner[0].Specular = convColor(mesh->mColors[1][af.mIndices[0]]);
				face.Corner[1].Specular = convColor(mesh->mColors[1][af.mIndices[1]]);
				face.Corner[2].Specular = convColor(mesh->mColors[1][af.mIndices[2]]);
			}
			else
			{
				face.Corner[0].Specular = CRGBA(255, 255, 255, 255);
				face.Corner[1].Specular = CRGBA(255, 255, 255, 255);
				face.Corner[2].Specular = CRGBA(255, 255, 255, 255);
			}
			// TODO: Color modulate, alpha, use color alpha for vp tree, etc
			++numFaces;
		}
	}
	if (numFaces != buildMesh.Faces.size())
	{
		tlmessage(context.ToolLogger, context.Settings.SourceFilePath.c_str(),
			"Removed %u degenerate faces in shape '%s'", (uint32)(buildMesh.Faces.size() - numFaces), node->mName.C_Str());
		buildMesh.Faces.resize(numFaces);
	}

	// clear for MRM info
	buildMesh.Interfaces.clear();
	buildMesh.InterfaceLinks.clear();

	// TODO: Export VP
	buildMesh.MeshVertexProgram = NULL;
	
	return true;
}

bool assimpShape(CMeshUtilsContext &context, CNodeContext &nodeContext)
{
	// Reference: export_mesh.cpp, buildShape
	nodeContext.Shape = NULL;

	const aiNode *node = nodeContext.InternalNode;
	nlassert(node->mNumMeshes);

	// Fill the build interface of CMesh
	CMeshBase::CMeshBaseBuild buildBaseMesh;
	assimpBuildBaseMesh(buildBaseMesh, context, nodeContext);

	CMesh::CMeshBuild buildMesh;
	if (!assimpBuildMesh(buildMesh, buildBaseMesh, context, nodeContext))
		return false;

	// Make a CMesh object
	CMesh *mesh = new CMesh();

	// Build the mesh with the build interface
	mesh->build(buildBaseMesh, buildMesh);

	// TODO
	// Reference: export_mesh.cpp, buildShape
	// Must be done after the build to update vertex links
	// Pass to buildMeshMorph if the original mesh is skinned or not
	// buildMeshMorph(buildMesh, node, time, nodeMap != NULL);
	// mesh->setBlendShapes(buildMesh.BlendShapes);

	// optimize number of material
	// mesh->optimizeMaterialUsage(materialRemap);

	// Store mesh in context
	nodeContext.Shape = mesh;
	return true;
}

/* end of file */
