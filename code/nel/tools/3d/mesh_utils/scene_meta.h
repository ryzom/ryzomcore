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

#ifndef NL_SCENE_META_H
#define NL_SCENE_META_H
#include <nel/misc/types_nl.h>

#include <nel/misc/sstring.h>
#include <nel/misc/smart_ptr.h>

#include <nel/3d/material.h>

namespace NLMISC {
	class IStream;
}

namespace NL3D {
	class CMaterial;
}

enum TMesh
{
	TMeshDisabled = 0,
	TMeshShape = 1,
	TMeshCollisionInt = 2,
	TMeshCollisionExt = 3,
	TMeshZone = 4,
	TMeshPortal = 5,
	TMeshCluster = 6,
};

enum TBone
{
	TBoneAuto = 0,
	TBoneForce = 1, // Force this node to be part of a skeleton
	TBoneRoot = 2, // Make this node the skeleton root, it will be exported using the scene name. There can only be one (editor should keep track and disable)
};

struct CNodeMeta
{
	CNodeMeta();

	bool AddToIG; // Add this node to an instance group
	TMesh ExportMesh;
	TBone ExportBone;

	std::string InstanceShape;
	std::string InstanceName;
	std::string InstanceGroupName;

	bool AutoAnim;
	// std::vector<NLMISC::CSString> Materials; // In case there's an issue with nameless materials in some format... Map to material entirely in the meta editor.

	void serial(NLMISC::IStream &s);
};

enum TSkel
{
	TSkelLocal = 0, // Export smallest skeleton possible from connected bones
	TSkelRoot = 1, // Export skeleton from a direct child node in the scene root node
	TSkelFull = 2, // Include all connected child nodes in the skeleton
};

typedef std::map<NLMISC::CSString, NLMISC::CSmartPtr<NL3D::CMaterial> > TMaterialMap;
struct CSceneMeta
{
	CSceneMeta();

	bool ImportShape;
	bool ImportSkel;
	bool ImportAnim;
	bool ImportCmb;
	bool ImportIG;

	bool ExportDefaultIG; // Export a default instance group from nodes the scene that do not have an instance group set
	TSkel SkeletonMode;

	std::map<NLMISC::CSString, CNodeMeta> Nodes;
	TMaterialMap Materials;

	const std::string &metaFilePath() const { return m_MetaFilePath; }

	bool load(const std::string &filePath);
	void save();
	void serial(NLMISC::IStream &s);

private:
	std::string m_MetaFilePath;

};

#endif /* NL_SCENE_META_H */

/* end of file */
