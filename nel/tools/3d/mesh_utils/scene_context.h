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

#ifndef NL_SCENE_CONTEXT_H
#define NL_SCENE_CONTEXT_H
#include <nel/misc/types_nl.h>

#include "mesh_utils.h"
#include "scene_meta.h"

#include <nel/misc/sstring.h>
#include <nel/pipeline/tool_logger.h>
#include <nel/misc/smart_ptr.h>
#include <nel/misc/matrix.h>

#include <nel/3d/shape.h>

#ifndef NL_NODE_INTERNAL_TYPE
#define NL_NODE_INTERNAL_TYPE void
#endif
#ifndef NL_SCENE_INTERNAL_TYPE
#define NL_SCENE_INTERNAL_TYPE void
#endif

namespace NL3D {
	class IShape;
	class CMaterial;
}

struct CNodeContext
{
	CNodeContext() :
		InternalNode(NULL),
		IsBone(false)
	{

	}

	const NL_NODE_INTERNAL_TYPE *InternalNode;
	bool IsBone;

	// NLMISC::CMatrix Transform; // TODO
	NLMISC::CSmartPtr<NL3D::IShape> Shape;
};

typedef std::map<NLMISC::CSString, CNodeContext> TNodeContextMap;
struct CMeshUtilsContext
{
	CMeshUtilsContext(const CMeshUtilsSettings &settings) : Settings(settings), InternalScene(NULL)
	{

	}

	const CMeshUtilsSettings &Settings;

	NLPIPELINE::CToolLogger ToolLogger;

	const NL_SCENE_INTERNAL_TYPE *InternalScene;
	CSceneMeta SceneMeta;

	TNodeContextMap Nodes; // Impl note: Should never end up containing the scene root node.
	// std::map<const aiMesh *, NLMISC::CSString> MeshNames; // Maps meshes to a node name ********************* todo ***************
};

#endif /* NL_SCENE_CONTEXT_H */

/* end of file */
