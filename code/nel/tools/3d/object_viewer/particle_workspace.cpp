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

#include "std_afx.h"
#include <shlwapi.h>
//
#include "particle_workspace.h"
#include "object_viewer.h"
//
#include "nel/3d/shape_bank.h"
#include "nel/3d/particle_system_model.h"
#include "nel/3d/particle_system_shape.h"
#include "nel/3d/skeleton_model.h"
//
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/file.h"
//



//***********************************************************************************************
CParticleWorkspace::CNode::CNode()
{
	_PS = NULL;
	_PSM = NULL;
	_ShapeBank = NULL;
	_Modified = false;
	_ParentSkel = NULL;
	_ResetAutoCount = false;
	_WS = NULL;
}

//***********************************************************************************************
CParticleWorkspace::CNode::~CNode()
{	
	unload();
	
}

//***********************************************************************************************
void CParticleWorkspace::CNode::memorizeState()
{
	nlassert(_WS);
	if (!_PS) return;
	_InitialPos.copySystemInitialPos(_PS);
}

//***********************************************************************************************
void CParticleWorkspace::CNode::restoreState()
{
	nlassert(_WS);
	if (!_PS) return;
	_InitialPos.restoreSystem();
}

//**************************************************************************************************************************
bool CParticleWorkspace::CNode::isStateMemorized() const
{
	return _InitialPos.isStateMemorized();
}

//**************************************************************************************************************************
void CParticleWorkspace::CNode::stickPSToSkeleton(NL3D::CSkeletonModel *skel,
												  uint bone,
												  const std::string &parentSkelName,
												  const std::string &parentBoneName)
{
	nlassert(_WS);
	if (!_PSM) return;
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

//**************************************************************************************************************************
void CParticleWorkspace::CNode::unstickPSFromSkeleton()
{
	nlassert(_WS);
	_ParentSkelName.clear();
	_ParentBoneName.clear();
	if (!_PSM) return;
	if (_ParentSkel)
	{
		_ParentSkel->detachSkeletonSon(_PSM);
		_ParentSkel = NULL;		
	}	
}

//***********************************************************************************************
void CParticleWorkspace::CNode::removeLocated(NL3D::CPSLocated *loc)
{	
	nlassert(_WS);
	if (_InitialPos.isStateMemorized())
	{	
		_InitialPos.removeLocated(loc);
	}
}

//***********************************************************************************************
void CParticleWorkspace::CNode::removeLocatedBindable(NL3D::CPSLocatedBindable *lb)
{	
	nlassert(_WS);
	if (_InitialPos.isStateMemorized())
	{
		_InitialPos.removeLocatedBindable(lb);
	}
}

//***********************************************************************************************
void CParticleWorkspace::CNode::setModified(bool modified)
{
	nlassert(_WS);
	if (_Modified == modified) return;		
	_Modified = modified;
	_WS->nodeModified(*this);
}

//***********************************************************************************************
void CParticleWorkspace::CNode::unload()
{
	nlassert(_WS);
	if (_PSM)
	{
		NL3D::CShapeBank *oldSB = NL3D::CNELU::Scene->getShapeBank();
		NL3D::CNELU::Scene->setShapeBank(_ShapeBank);
		NL3D::CNELU::Scene->deleteInstance(_PSM);
		NL3D::CNELU::Scene->setShapeBank(oldSB);
		_PSM = NULL;
	}
	delete _ShapeBank;	
	_ShapeBank = NULL;
	_PS = NULL;
	_ParentSkel = NULL;
}


//***********************************************************************************************
void CParticleWorkspace::CNode::setup(NL3D::CParticleSystemModel &psm)
{
	nlassert(_WS);
	psm.setTransformMode(NL3D::CTransform::DirectMatrix);
	psm.setMatrix(NLMISC::CMatrix::Identity);					
	nlassert(_WS->getObjectViewer());
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
	_WS->getObjectViewer()->getSceneRoot()->hrcLinkSon(&psm);
	NL3D::CParticleSystem  *ps  = psm.getPS();
	nlassert(ps);
	ps->setFontManager(_WS->getFontManager());
	ps->setFontGenerator(_WS->getFontGenerator());
	ps->stopSound();		
	// flush textures
	psm.Shape->flushTextures(*NL3D::CNELU::Driver, 0);	
}

//***********************************************************************************************
void CParticleWorkspace::CNode::setTriggerAnim(const std::string &anim)
{
	nlassert(_WS);
	if (anim == _TriggerAnim) return;
	_WS->touch();
	_TriggerAnim = anim;
}


//***********************************************************************************************
void CParticleWorkspace::CNode::createEmptyPS()
{	
	nlassert(_WS);
	NL3D::CParticleSystem emptyPS;	
	NL3D::CParticleSystemShape *pss = new NL3D::CParticleSystemShape;
	pss->buildFromPS(emptyPS);	
	CUniquePtr<NL3D::CShapeBank> sb(new NL3D::CShapeBank);
	std::string shapeName = NLMISC::CFile::getFilename(_RelativePath);
	sb->add(shapeName, pss);
	NL3D::CShapeBank *oldSB = NL3D::CNELU::Scene->getShapeBank();
	NL3D::CNELU::Scene->setShapeBank(sb.get());
	NL3D::CParticleSystemModel *psm = NLMISC::safe_cast<NL3D::CParticleSystemModel *>(NL3D::CNELU::Scene->createInstance(shapeName));
	nlassert(psm);	
	NL3D::CNELU::Scene->setShapeBank(oldSB);				
	setup(*psm);
	unload();
	// commit new values
	_PS = psm->getPS();
	_PSM = psm;
	_ShapeBank = sb.release();
	_Modified = false;
}

//***********************************************************************************************
void CParticleWorkspace::CNode::init(CParticleWorkspace *ws)		
{		
	nlassert(ws);
	_WS = ws;
}

//***********************************************************************************************
void CParticleWorkspace::CNode::setRelativePath(const std::string &relativePath)
{
	nlassert(_WS);
	_RelativePath = relativePath;
}

//***********************************************************************************************
void CParticleWorkspace::CNode::serial(NLMISC::IStream &f)
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

//***********************************************************************************************
void CParticleWorkspace::CNode::savePS()
{
	savePSAs(getFullPath());
}


//***********************************************************************************************
void CParticleWorkspace::CNode::savePSAs(const std::string &fullPath)
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

//***********************************************************************************************
std::string CParticleWorkspace::CNode::getFullPath() const
{
	nlassert(_WS);
	return NLMISC::CPath::makePathAbsolute(_RelativePath, _WS->getPath(), true);
}

//***********************************************************************************************
bool CParticleWorkspace::CNode::loadPS()
{	
	nlassert(_WS);
	// manually load the PS shape (so that we can deal with exceptions)
	NL3D::CShapeStream ss;
	NLMISC::CIFile inputFile;
	// collapse name
	inputFile.open(getFullPath());
	ss.serial(inputFile);
	CUniquePtr<NL3D::CShapeBank> sb(new NL3D::CShapeBank);
	std::string shapeName = NLMISC::CFile::getFilename(_RelativePath);
	sb->add(shapeName, ss.getShapePointer());
	NL3D::CShapeBank *oldSB = NL3D::CNELU::Scene->getShapeBank();
	NL3D::CNELU::Scene->setShapeBank(sb.get());
	NL3D::CTransformShape *trs = NL3D::CNELU::Scene->createInstance(shapeName);
	if (!trs)
	{
		NL3D::CNELU::Scene->setShapeBank(oldSB);
		return false;
	}
	NL3D::CParticleSystemModel *psm = dynamic_cast<NL3D::CParticleSystemModel *>(trs);
	if (!psm)
	{
		// Not a particle system
		NL3D::CNELU::Scene->deleteInstance(trs);
		return false;
	}
	NL3D::CNELU::Scene->setShapeBank(oldSB);		
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
	_OV = NULL;
	_Modified = false;
	_FontManager = NULL;
	_FontGenerator = NULL;
	_ModificationCallback = NULL;
}

//***********************************************************************************************
CParticleWorkspace::~CParticleWorkspace()
{
}

//***********************************************************************************************
void CParticleWorkspace::init(CObjectViewer *ov,
							  const std::string &filename,
							  NL3D::CFontManager	*fontManager, 
							  NL3D::CFontGenerator	*fontGenerator 
							 )
{	
	nlassert(!_OV);
	nlassert(ov);
	_OV = ov;
	_Filename = filename;
	_FontManager   = fontManager;
	_FontGenerator = fontGenerator;
}
//***********************************************************************************************
void CParticleWorkspace::setName(const std::string &name)
{
	_Name = name;
	touch();
}


//***********************************************************************************************
CParticleWorkspace::CNode *CParticleWorkspace::addNode(const std::string &filenameWithFullPath)
{	
	nlassert(_OV);				
	// Check that file is not already inserted 	
	std::string fileName = NLMISC::CFile::getFilename(filenameWithFullPath);
	for(uint k = 0; k < _Nodes.size(); ++k)
	{
		if (NLMISC::nlstricmp(_Nodes[k]->getFilename(), fileName) == 0) return NULL;		
	}

	// TODO: replace with NeL methods
	TCHAR resultPath[MAX_PATH];
	std::string dosPath = NLMISC::CPath::standardizeDosPath(getPath());
	std::string relativePath;
	if (!PathRelativePathTo(resultPath, nlUtf8ToTStr(dosPath), FILE_ATTRIBUTE_DIRECTORY, nlUtf8ToTStr(filenameWithFullPath), 0))
	{
		relativePath = filenameWithFullPath; 
	}
	else
	{
		relativePath = NLMISC::tStrToUtf8(resultPath);
	}

	if (relativePath.size() >= 2)
	{
		if (relativePath[0] == '\\' && relativePath[1] != '\\')
		{
			relativePath = relativePath.substr(1);
		}
	}

	CNode *newNode = new CNode;
	newNode->init(this);
	newNode->setRelativePath(relativePath);
	_Nodes.push_back(newNode);
	setModifiedFlag(true);
	return newNode;	
}

//***********************************************************************************************
void CParticleWorkspace::removeNode(uint index)
{
	nlassert(_OV);
	nlassert(index < _Nodes.size());
	_Nodes[index] = NULL; // delete the smart-ptr target
	_Nodes.erase(_Nodes.begin() + index);
	touch();
}

//***********************************************************************************************
void CParticleWorkspace::removeNode(CNode *ptr)
{
	sint index = getIndexFromNode(ptr);
	nlassert(index != -1);
	removeNode((uint) index);
}

//***********************************************************************************************
void CParticleWorkspace::save()
{
	NLMISC::COFile stream;
	stream.open(_Filename);
	NLMISC::COXml xmlStream;
	xmlStream.init(&stream);
	this->serial(xmlStream);
	clearModifiedFlag();
}

//***********************************************************************************************
void CParticleWorkspace::load()
{
	NLMISC::CIFile stream;
	stream.open(_Filename);
	NLMISC::CIXml xmlStream;
	xmlStream.init(stream);
	this->serial(xmlStream);
	clearModifiedFlag();
}

//***********************************************************************************************
void CParticleWorkspace::serial(NLMISC::IStream &f)
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
			_Nodes.push_back(new CNode());						
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

//***********************************************************************************************
std::string CParticleWorkspace::getPath() const
{
	return NLMISC::CPath::standardizePath(NLMISC::CFile::getPath(_Filename));
}

//***********************************************************************************************
sint CParticleWorkspace::getIndexFromNode(CNode *node) const
{
	nlassert(node);
	nlassert(node->getWorkspace() == this);
	for(uint k = 0; k < _Nodes.size(); ++k)
	{
		if (node == _Nodes[k]) return (sint) k;
	}
	return -1;
}

//***********************************************************************************************
bool CParticleWorkspace::containsFile(std::string filename) const
{
	for(uint k = 0; k < _Nodes.size(); ++k)
	{
		if (NLMISC::nlstricmp(filename, _Nodes[k]->getFilename()) == 0) return true;
	}
	return false;
}

//***********************************************************************************************
void CParticleWorkspace::nodeModified(CNode &node)
{
	nlassert(node.getWorkspace() == this);
	if (_ModificationCallback)
	{
		_ModificationCallback->nodeModifiedFlagChanged(node);
	}
}

//***********************************************************************************************
CParticleWorkspace::CNode *CParticleWorkspace::getNodeFromPS(NL3D::CParticleSystem *ps) const
{
	for(uint k = 0; k < _Nodes.size(); ++k)
	{
		if (_Nodes[k]->getPSPointer() == ps) return _Nodes[k];
	}
	return NULL;
}

//***********************************************************************************************
void CParticleWorkspace::setModifiedFlag(bool modified)
{
	if (_Modified == modified) return;
	_Modified = modified;
	if (_ModificationCallback) _ModificationCallback->workspaceModifiedFlagChanged(*this);
}

//***********************************************************************************************
// predicate for workspace sorting
class CParticleWorkspaceSorter
{
public:
	CParticleWorkspace::ISort *Sorter;
	bool operator()(const NLMISC::CSmartPtr<CParticleWorkspace::CNode> &lhs, const NLMISC::CSmartPtr<CParticleWorkspace::CNode> &rhs)
	{
		return Sorter->less(*lhs, *rhs);
	}
};

//***********************************************************************************************
void CParticleWorkspace::sort(ISort &predicate)
{
	CParticleWorkspaceSorter ws;
	ws.Sorter = &predicate;
	std::sort(_Nodes.begin(), _Nodes.end(), ws);
	setModifiedFlag(true);
}

//***********************************************************************************************
bool CParticleWorkspace::isContentModified() const
{
	for(uint k = 0; k < _Nodes.size(); ++k)
	{
		if (_Nodes[k]->isModified()) return true;
	}
	return false;
}

//***********************************************************************************************
void CParticleWorkspace::restickAllObjects(CObjectViewer *ov)
{
	for(uint k = 0; k < _Nodes.size(); ++k)
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
	}
}


