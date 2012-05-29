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


#include "stdpch.h"
#include "particle_node.h"

// NeL includes
#include <nel/3d/shape_bank.h>
#include <nel/3d/particle_system_model.h>
#include <nel/3d/particle_system_shape.h>

#include <nel/misc/o_xml.h>
#include <nel/misc/i_xml.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

// Project includes
#include "modules.h"

using namespace NLMISC;
using namespace NL3D;

namespace NLQT
{

CWorkspaceNode::CWorkspaceNode()
{
	_PS = NULL;
	_PSM = NULL;
	_ShapeBank = NULL;
	_Modified = false;
	_ParentSkel = NULL;
	_ResetAutoCount = false;
	_WS = NULL;
}

CWorkspaceNode::~CWorkspaceNode()
{
	unload();
}

void CWorkspaceNode::memorizeState()
{
	nlassert(_WS);
	if (!_PS)
		return;
	_InitialPos.copySystemInitialPos(_PS);
}

void CWorkspaceNode::restoreState()
{
	nlassert(_WS);
	if (!_PS)
		return;
	_InitialPos.restoreSystem();
}

bool CWorkspaceNode::isStateMemorized() const
{
	return _InitialPos.isStateMemorized();
}

void CWorkspaceNode::stickPSToSkeleton(NL3D::CSkeletonModel *skel,
									   uint bone,
									   const std::string &parentSkelName,
									   const std::string &parentBoneName)
{
	nlassert(_WS);
	if (!_PSM)
		return;
	unstickPSFromSkeleton();
	_ParentSkelName = parentSkelName;
	_ParentBoneName = parentBoneName;
	if (skel)
	{
		skel->stickObject(_PSM, bone);
		_PSM->setMatrix(NLMISC::CMatrix::Identity);
		_ParentSkel = skel;
	}
	if (_WS->getModificationCallback())
	{
		_WS->getModificationCallback()->nodeSkelParentChanged(*this);
	}
}

void CWorkspaceNode::unstickPSFromSkeleton()
{
	nlassert(_WS);
	_ParentSkelName = "";
	_ParentBoneName = "";
	if (!_PSM) return;
	if (_ParentSkel)
	{
		_ParentSkel->detachSkeletonSon(_PSM);
		_ParentSkel = NULL;
	}
}

void CWorkspaceNode::removeLocated(NL3D::CPSLocated *loc)
{
	nlassert(_WS);
	if (_InitialPos.isStateMemorized())
	{
		_InitialPos.removeLocated(loc);
	}
}

void CWorkspaceNode::removeLocatedBindable(NL3D::CPSLocatedBindable *lb)
{
	nlassert(_WS);
	if (_InitialPos.isStateMemorized())
	{
		_InitialPos.removeLocatedBindable(lb);
	}
}

void CWorkspaceNode::setModified(bool modified)
{
	nlassert(_WS);
	if (_Modified == modified) return;
	_Modified = modified;
	_WS->nodeModified(*this);
}

void CWorkspaceNode::unload()
{
	nlassert(_WS);
	if (_PSM)
	{
		NL3D::CShapeBank *oldSB = Modules::psEdit().getScene()->getShapeBank();
		Modules::psEdit().getScene()->setShapeBank(_ShapeBank);
		Modules::psEdit().getScene()->deleteInstance(_PSM);
		Modules::psEdit().getScene()->setShapeBank(oldSB);
		_PSM = NULL;
	}
	delete _ShapeBank;
	_ShapeBank = NULL;
	_PS = NULL;
	_ParentSkel = NULL;
}

void CWorkspaceNode::setup(NL3D::CParticleSystemModel &psm)
{
	nlassert(_WS);
	psm.setTransformMode(NL3D::CTransform::DirectMatrix);
	psm.setMatrix(NLMISC::CMatrix::Identity);
	psm.setEditionMode(true); // this also force the system instanciation
	for(uint k = 0; k < NL3D::MaxPSUserParam; ++k)
	{
		psm.bypassGlobalUserParamValue(k);
	}
	psm.enableAutoGetEllapsedTime(false);
	psm.setEllapsedTime(0.f); // system is paused
	// initialy, the ps is hidden
	psm.hide();
	// link to the root for manipulation
	// !!!!!!!!!!!!!!!!!!!
	//_WS->getObjectViewer()->getSceneRoot()->hrcLinkSon(&psm);
	NL3D::CParticleSystem  *ps  = psm.getPS();
	nlassert(ps);
	ps->setFontManager(Modules::psEdit().getFontManager());
	ps->setFontGenerator(Modules::psEdit().getFontGenerator());
	ps->stopSound();
	// flush textures
	psm.Shape->flushTextures(*Modules::psEdit().getDriver(), 0);
}

void CWorkspaceNode::setTriggerAnim(const std::string &anim)
{
	nlassert(_WS);
	if (anim == _TriggerAnim) return;
	_WS->touch();
	_TriggerAnim = anim;
}

void CWorkspaceNode::createEmptyPS()
{
	nlassert(_WS);
	NL3D::CParticleSystem emptyPS;
	NL3D::CParticleSystemShape *pss = new NL3D::CParticleSystemShape;
	pss->buildFromPS(emptyPS);
	std::auto_ptr<NL3D::CShapeBank> sb(new NL3D::CShapeBank);
	std::string shapeName = NLMISC::CFile::getFilename(_RelativePath);
	sb->add(shapeName, pss);
	NL3D::CShapeBank *oldSB = Modules::psEdit().getScene()->getShapeBank();
	Modules::psEdit().getScene()->setShapeBank(sb.get());
	NL3D::CParticleSystemModel *psm = NLMISC::safe_cast<NL3D::CParticleSystemModel *>(Modules::psEdit().getScene()->createInstance(shapeName));
	nlassert(psm);
	Modules::psEdit().getScene()->setShapeBank(oldSB);
	setup(*psm);
	unload();
	// commit new values
	_PS = psm->getPS();
	_PSM = psm;
	_ShapeBank = sb.release();
	_Modified = false;
}

void CWorkspaceNode::init(CParticleWorkspace *ws)
{
	nlassert(ws);
	_WS = ws;
}

void CWorkspaceNode::setRelativePath(const std::string &relativePath)
{
	nlassert(_WS);
	_RelativePath = relativePath;
}

void CWorkspaceNode::serial(NLMISC::IStream &f)
{
	nlassert(_WS);
	f.xmlPush("PROJECT_FILE");
	sint version = f.serialVersion(2);
	f.xmlSerial(_RelativePath, "RELATIVE_PATH");
	if (version >= 1)
	{
		f.xmlSerial(_TriggerAnim, "TRIGGER_ANIMATION");
	}
	if (version >= 2)
	{
		f.xmlSerial(_ParentSkelName, "PARENT_SKEL_NAME");
		f.xmlSerial(_ParentBoneName, "PARENT_BONE_NAME");
	}
	f.xmlPop();
}

void CWorkspaceNode::savePS() throw(NLMISC::EStream)
{
	savePSAs(getFullPath());
}

void CWorkspaceNode::savePSAs(const std::string &fullPath) throw(NLMISC::EStream)
{
	nlassert(_WS);
	if (!_PS) return;
	// build a shape from our system, and save it
	NL3D::CParticleSystemShape psc;
	psc.buildFromPS(*_PS);
	NL3D::CShapeStream st(&psc);
	NLMISC::COFile oFile(fullPath);
	oFile.serial(st);
}

std::string CWorkspaceNode::getFullPath() const
{
	nlassert(_WS);
	return _WS->getPath() + _RelativePath.c_str();
}

bool CWorkspaceNode::loadPS() throw(NLMISC::EStream)
{
	nlassert(_WS);
	// manually load the PS shape (so that we can deal with exceptions)
	NL3D::CShapeStream ss;
	NLMISC::CIFile inputFile;
	// collapse name
	inputFile.open(getFullPath());
	ss.serial(inputFile);
	std::auto_ptr<NL3D::CShapeBank> sb(new NL3D::CShapeBank);
	std::string shapeName = NLMISC::CFile::getFilename(_RelativePath);
	sb->add(shapeName, ss.getShapePointer());
	NL3D::CShapeBank *oldSB = Modules::psEdit().getScene()->getShapeBank();
	Modules::psEdit().getScene()->setShapeBank(sb.get());
	NL3D::CTransformShape *trs = Modules::psEdit().getScene()->createInstance(shapeName);
	if (!trs)
	{
		Modules::psEdit().getScene()->setShapeBank(oldSB);
		return false;
	}
	NL3D::CParticleSystemModel *psm = dynamic_cast<NL3D::CParticleSystemModel *>(trs);
	if (!psm)
	{
		// Not a particle system
		Modules::psEdit().getScene()->deleteInstance(trs);
		return false;
	}
	if (oldSB)
	{
		Modules::psEdit().getScene()->setShapeBank(oldSB);
	}
	setup(*psm);
	unload();
	// commit new values
	_PS = psm->getPS();
	_PSM = psm;
	_ShapeBank = sb.release();
	_Modified = false;
	return true;
}


//***********************************************************************************************
CParticleWorkspace::CParticleWorkspace()
{
	_Modified = false;
	_ModificationCallback = NULL;
}

CParticleWorkspace::~CParticleWorkspace()
{
	for (size_t i = 0; i < _Nodes.size(); ++i)
		delete _Nodes[i];
	_Nodes.clear();
}

void CParticleWorkspace::init(const std::string &filename)
{
	_Filename = filename;
}

void CParticleWorkspace::setName(const std::string &name)
{
	_Name = name;
	touch();
}

void CParticleWorkspace::setFileName(const std::string &fileName)
{
	_Filename = fileName;
}

std::string CParticleWorkspace::getFilename() const
{
	return CFile::getFilename(_Filename);
}

CWorkspaceNode *CParticleWorkspace::addNode(const std::string &filenameWithFullPath)  throw( NLMISC::Exception)
{
	// Check that file is not already inserted
	std::string fileName = NLMISC::CFile::getFilename(filenameWithFullPath);
	for(uint k = 0; k < _Nodes.size(); ++k)
	{
		if (NLMISC::nlstricmp(_Nodes[k]->getFilename(), fileName) == 0) return NULL;
	}
//	char resultPath[MAX_PATH];
//	std::string dosPath = NLMISC::CPath::standardizeDosPath(getPath());
	std::string relativePath;
//	if (!PathRelativePathTo(resultPath, dosPath.c_str(), FILE_ATTRIBUTE_DIRECTORY, filenameWithFullPath.c_str(), 0))
//	{
	relativePath = filenameWithFullPath;
//	}
//	else
//	{
//		relativePath = resultPath;
//	}
	if (relativePath.size() >= 2)
	{
		if (relativePath[0] == '\\' && relativePath[1] != '\\')
		{
			relativePath = relativePath.substr(1);
		}
	}
	CWorkspaceNode *newNode = new CWorkspaceNode();
	newNode->init(this);
	newNode->setRelativePath(relativePath);
	_Nodes.push_back(newNode);
	setModifiedFlag(true);
	return newNode;
}

void CParticleWorkspace::removeNode(uint index)
{
	nlassert(index < _Nodes.size());
	_Nodes[index] = NULL; // delete the smart-ptr target
	delete _Nodes[index];
	_Nodes.erase(_Nodes.begin() + index);
	touch();
}

void CParticleWorkspace::removeNode(CWorkspaceNode *ptr)
{
	sint index = getIndexFromNode(ptr);
	nlassert(index != -1);
	removeNode((uint) index);
}

void CParticleWorkspace::save() throw(NLMISC::EStream)
{
	NLMISC::COFile stream;
	stream.open(_Filename);
	NLMISC::COXml xmlStream;
	xmlStream.init(&stream);
	this->serial(xmlStream);
	clearModifiedFlag();
}

void CParticleWorkspace::load() throw(NLMISC::EStream)
{
	NLMISC::CIFile stream;
	stream.open(_Filename);
	NLMISC::CIXml xmlStream;
	xmlStream.init(stream);
	this->serial(xmlStream);
	clearModifiedFlag();
}

void CParticleWorkspace::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.xmlPush("PARTICLE_WORKSPACE");
	f.serialVersion(0);
	f.xmlSerial(_Name, "NAME");
	f.xmlPush("PS_LIST");
	uint32 numNodes = (uint32)_Nodes.size();
	// TODO : avoid to store the number of nodes
	f.xmlSerial(numNodes, "NUM_NODES");
	if (f.isReading())
	{
		for(uint k = 0; k < numNodes; ++k)
		{
			_Nodes.push_back(new CWorkspaceNode());
			_Nodes.back()->init(this);
			f.serial(*_Nodes.back());
		}
	}
	else
	{
		for(uint k = 0; k < numNodes; ++k)
		{
			f.serial(*_Nodes[k]);
		}
	}
	f.xmlPop();
	f.xmlPop();
}

std::string CParticleWorkspace::getPath() const
{
	return NLMISC::CPath::standardizePath(NLMISC::CFile::getPath(_Filename));
}

sint CParticleWorkspace::getIndexFromNode(CWorkspaceNode *node) const
{
	nlassert(node);
	nlassert(node->getWorkspace() == this);
	for(uint k = 0; k < _Nodes.size(); ++k)
	{
		if (node == _Nodes[k]) return (sint) k;
	}
	return -1;
}

bool CParticleWorkspace::containsFile(std::string filename) const
{
	for(uint k = 0; k < _Nodes.size(); ++k)
	{
		if (NLMISC::nlstricmp(filename, _Nodes[k]->getFilename()) == 0) return true;
	}
	return false;
}

void CParticleWorkspace::nodeModified(CWorkspaceNode &node)
{
	nlassert(node.getWorkspace() == this);
	if (_ModificationCallback)
	{
		_ModificationCallback->nodeModifiedFlagChanged(node);
	}
}

CWorkspaceNode *CParticleWorkspace::getNodeFromPS(NL3D::CParticleSystem *ps) const
{
	for(uint k = 0; k < _Nodes.size(); ++k)
	{
		if (_Nodes[k]->getPSPointer() == ps) return _Nodes[k];
	}
	return NULL;
}

void CParticleWorkspace::setModifiedFlag(bool modified)
{
	if (_Modified == modified) return;
	_Modified = modified;
	if (_ModificationCallback) _ModificationCallback->workspaceModifiedFlagChanged(*this);
}

bool CParticleWorkspace::isContentModified() const
{
	for(uint k = 0; k < _Nodes.size(); ++k)
	{
		if (_Nodes[k]->isModified()) return true;
	}
	return false;
}

void CParticleWorkspace::restickAllObjects()
{
	/*	for(uint k = 0; k < _Nodes.size(); ++k)
		{
			std::string parentSkelName = _Nodes[k]->getParentSkelName();
			std::string parentBoneName = _Nodes[k]->getParentBoneName();
			//
			_Nodes[k]->unstickPSFromSkeleton();
			if (!parentSkelName.empty())
			// find instance to stick to in the scene
			for(uint l = 0; l < ov->getNumInstance(); ++l)
			{
				CInstanceInfo *ii = ov->getInstance(l);
				if (ii->TransformShape && ii->Saved.ShapeFilename == parentSkelName)
				{
					NL3D::CSkeletonModel *skel = dynamic_cast<NL3D::CSkeletonModel *>(ii->TransformShape);
					if (skel)
					{
						sint boneID = skel->getBoneIdByName(parentBoneName);
						if (boneID != -1)
						{
							_Nodes[k]->stickPSToSkeleton(skel, (uint) boneID, parentSkelName, parentBoneName);
							break;
						}
					}
				}
			}
		}*/
}

} /* namespace NLQT */