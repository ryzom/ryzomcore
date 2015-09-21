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
#include "scene_meta.h"

#include <nel/misc/debug.h>
#include <nel/misc/stream.h>
#include <nel/misc/file.h>

#include <nel/3d/material.h>

using namespace std;
using namespace NLMISC;

CNodeMeta::CNodeMeta() :
	AddToIG(true),
	ExportMesh(TMeshShape),
	ExportBone(TBoneAuto),
	AutoAnim(false)
{

}

void CNodeMeta::serial(NLMISC::IStream &s)
{
	uint version = s.serialVersion(1);
	s.serial(AddToIG);
	s.serial((uint32 &)ExportMesh);
	s.serial((uint32 &)ExportBone);
	s.serial(InstanceShape);
	s.serial(InstanceName);
	s.serial(InstanceGroupName);
	s.serial(AutoAnim);
}

CSceneMeta::CSceneMeta() :
	ImportShape(true),
	ImportSkel(true),
	ImportAnim(true),
	ImportCmb(true),
	ImportIG(true),
	ExportDefaultIG(false),
	SkeletonMode(TSkelRoot)
{
	
}

bool CSceneMeta::load(const std::string &filePath)
{
	m_MetaFilePath = NLMISC::CPath::standardizePath(filePath + ".nelmeta", false);
	if (CFile::fileExists(m_MetaFilePath))
	{
		CIFile f(m_MetaFilePath);
		serial(f);
		f.close();
		return true;
	}
	return false;
}

void CSceneMeta::save()
{
	COFile f(m_MetaFilePath, false, false, true);
	serial(f);
	f.close();
}

void CSceneMeta::serial(NLMISC::IStream &s)
{
	uint version = s.serialVersion(1);

	s.serial(ImportShape);
	s.serial(ImportSkel);
	s.serial(ImportAnim);
	s.serial(ImportCmb);
	s.serial(ImportIG);

	s.serial(ExportDefaultIG);
	s.serial((uint32 &)SkeletonMode);

	s.serialCont(Nodes);
	s.serialPtrCont(Materials);
}

/* end of file */
