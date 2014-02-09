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

#include "stdafx.h"
#include "export_nel.h"
#include "export_appdata.h"
#include "calc_lm.h"
#include "nel/misc/path.h"
#include "nel/3d/texture_file.h"
#include <modstack.h> // class IDerivedObject
#include <decomp.h> // class IDerivedObject
#include <locale.h>

using namespace NLMISC;
using namespace NL3D;

#ifdef _STLPORT_VERSION
namespace std
{
	float fabsf(float f) { return ::fabsf(f); }
	double fabsl(double f) { return ::fabsl(f); }
}
#endif

// ***************************************************************************

// --------------------------------------------------

const char* CExportNel::ErrorMessage[CodeCount]=
{
	"No error.",
	"Some vertices of the skin are not assigned.",
	"The skin doesn't use the same skeleton.",
};

// --------------------------------------------------

CExportNel::CExportNel (bool errorInDialog, bool view, bool absolutePath, Interface *ip, std::string errorTitle, CExportNelOptions *opt)
{
	_Ip = ip;
	_AbsolutePath = absolutePath;
	_View = view;
	_ErrorInDialog = errorInDialog;
	_ErrorTitle = errorTitle;

	// No options ?
	if (opt)
		_Options = *opt;
}

// --------------------------------------------------

/// Transforme a 3dsmax view matrix to camera matrix.
Matrix3 CExportNel::viewMatrix2CameraMatrix (const Matrix3& viewMatrix)
{
	// Init the return matrix
	Matrix3	mat;
	mat.Invert();

	// Get invert viewMatrix
	// We invert view matrix to get camera matrix in viewport space
	Matrix3	invertViewMatrix=viewMatrix;
	invertViewMatrix.Invert();

	// This matrix is used to compute the coordinates to the viewport space. Take ths invert.
	Matrix3 viewportInvert;
	viewportInvert.SetRow (0, Point3(1.f, 0.f, 0.f));
	viewportInvert.SetRow (1, Point3(0.f, 0.f, 1.f));
	viewportInvert.SetRow (2, Point3(0.f, -1.f, 0.f));
	viewportInvert.SetRow (3, Point3(0.f, 0.f, 0.f));
	viewportInvert.Invert();

	// Compose with invert viewport matrix
	mat=viewportInvert*invertViewMatrix;

	// return the computed matrix
	return mat;
}


// --------------------------------------------------
// ConvertMatrix a 3dsmax vector in NeL matrix
void CExportNel::convertVector (CVector& nelVector, const Point3& maxVector)
{
	nelVector.x = maxVector.x;	
	nelVector.y = maxVector.y;	
	nelVector.z = maxVector.z;	
}


// --------------------------------------------------

// ConvertMatrix a 3dsmax matrix in NeL matrix
void CExportNel::convertMatrix (CMatrix& nelMatrix, const Matrix3& maxMatrix)
{
	// Basis vector
	CVector I, J, K, P;

	// Build the rot matrix
	I.x= maxMatrix.GetRow(0).x;
	I.y= maxMatrix.GetRow(0).y;
	I.z= maxMatrix.GetRow(0).z;
	J.x= maxMatrix.GetRow(1).x;
	J.y= maxMatrix.GetRow(1).y;
	J.z= maxMatrix.GetRow(1).z;
	K.x= maxMatrix.GetRow(2).x;
	K.y= maxMatrix.GetRow(2).y;
	K.z= maxMatrix.GetRow(2).z;

	// Build the translation vector
	P.x= maxMatrix.GetTrans().x;
	P.y= maxMatrix.GetTrans().y;
	P.z= maxMatrix.GetTrans().z;

	// *** Build the NeL matrix

	// Set it to identity to have the good flags in it
	nelMatrix.identity();

	// Set the rotation part
	nelMatrix.setRot(I, J, K);

	// Set the position part
	nelMatrix.setPos(P);
}

// --------------------------------------------------

// ConvertMatrix a 3dsmax matrix in NeL matrix
void CExportNel::convertMatrix (Matrix3& maxMatrix, const CMatrix& nelMatrix)
{
	// Basis vector
	Point3 I, J, K, P;

	// Build the rot matrix
	I.x= nelMatrix.getI().x;
	I.y= nelMatrix.getI().y;
	I.z= nelMatrix.getI().z;
	J.x= nelMatrix.getJ().x;
	J.y= nelMatrix.getJ().y;
	J.z= nelMatrix.getJ().z;
	K.x= nelMatrix.getK().x;
	K.y= nelMatrix.getK().y;
	K.z= nelMatrix.getK().z;

	// Build the translation vector
	P.x= nelMatrix.getPos().x;
	P.y= nelMatrix.getPos().y;
	P.z= nelMatrix.getPos().z;

	// *** Build the NeL matrix

	// Set it to identity to have the good flags in it
	maxMatrix.IdentityMatrix();

	// Set the rotation part
	maxMatrix.SetRow (0, I);
	maxMatrix.SetRow (1, J);
	maxMatrix.SetRow (2, K);

	// Set the position part
	maxMatrix.SetTrans (P);
}

// --------------------------------------------------

// Convert a 3dsmax color in NeL color
void CExportNel::convertColor (CRGBA& nelColor, const Color& maxColor)
{
	// Convert from max to Nel
	float fR=maxColor.r*255.f+0.5f;
	float fG=maxColor.g*255.f+0.5f;
	float fB=maxColor.b*255.f+0.5f;
	clamp (fR, 0.f, 255.f);
	clamp (fG, 0.f, 255.f);
	clamp (fB, 0.f, 255.f);
	nelColor.R=(uint8)fR;
	nelColor.G=(uint8)fG;
	nelColor.B=(uint8)fB;

	// Alpha
	nelColor.A=255;
}

// --------------------------------------------------

// Return true if the texmap is a texture file else false
bool CExportNel::isClassIdCompatible (Animatable& anim, Class_ID& classId)
{
	// Check if it is a file texmap
	if	(
			(anim.ClassID() == classId)										||
			((anim.SuperClassID() == classId.PartA())&&(classId.PartB()==0))
		)
	{
		return true;
	}
	return false;
}

// --------------------------------------------------

// Return the pointer on the subanim with the name sName of the node. If it doesn't exist, return NULL.
Animatable*	CExportNel::getSubAnimByName (Animatable& node, const char* sName)
{
	// Search for the subanim
	for (int nSub=0; nSub<node.NumSubs(); nSub++)
	{
		// Get the name
		TSTR sSubName=node.SubAnimName(nSub);

		// Good name?
		if (strcmp (sSubName, sName)==0)
		{
			// ok, return this subanim
			return node.SubAnim(nSub);
		}

		// Recurse in the sub anim..
		Animatable* subSearch=getSubAnimByName (*node.SubAnim(nSub), sName);

		// Found? return it, else continue
		if (subSearch)
			return subSearch;
	}
	// not found
	return NULL;
}

// --------------------------------------------------

// Return the pointer on the subanim with the name sName of the node. If it doesn't exist, return NULL.
Control* CExportNel::getControlerByName (Animatable& node, const char* sName)
{
	// For debug
	TSTR nodeName;
	node.GetClassName (nodeName);

	// for all parameters block in this node
	for (int nBlock=0; nBlock<node.NumParamBlocks(); nBlock++)
	{
		// Get the paramblock2 pointer
		IParamBlock2 *param=node.GetParamBlock(nBlock);

		// Search for the subanim
		for (int nSub=0; nSub<param->NumParams(); nSub++)
		{
			// Index of the parameter
			ParamID id=param->IndextoID(nSub);

			// Get the paramDef
			ParamDef& paramDef=param->GetParamDef(id);

			// Good name?
			if (strcmp (paramDef.int_name, sName)==0)
			{
				// ok, return this subanim
#if MAX_VERSION_MAJOR >= 14
				return param->GetControllerByID(id);
#else
				return param->GetController(id);
#endif
			}
		}
	}

	// Get num sub anim
	uint numSubAnim=node.NumSubs();

	// Visit sub anim
	for (uint s=0; s<numSubAnim; s++)
	{
		// ok, return this subanim controler
		if (node.SubAnim(s))
		{
			// Sub anim name
			TSTR name=node.SubAnimName (s);
			if (strcmp (name, sName)==0)
			{
				// Get the controller pointer of this sub anim
				Control* c=GetControlInterface (node.SubAnim(s));
				if (c)
					return c;
			}
		}
		// If a sub anim is here, go to visit it.
		if (node.SubAnim(s))
		{
			// Get the ctrl for sub anim
			Control* c=getControlerByName (*node.SubAnim(s), sName);

			// If it exists, return it
			if (c)
				return c;
		}
	}

	// not found
	return NULL;
}

// --------------------------------------------------

bool getValueByNameUsingParamBlock2Internal (Animatable& node, const char* sName, ParamType2 type, void *pValue, TimeValue tvTime)
{
	// for all parameters block in this node
	for (int nBlock=0; nBlock<node.NumParamBlocks(); nBlock++)
	{
		// Get the paramblock2 pointer
		IParamBlock2 *param=node.GetParamBlock(nBlock);

		// Search for the subanim
		for (int nSub=0; nSub<param->NumParams(); nSub++)
		{
			// Index of the parameter
			ParamID id=param->IndextoID(nSub);

			// Get the paramDef
			ParamDef& paramDef=param->GetParamDef(id);

			// Good name?
			if (strcmp (paramDef.int_name, sName)==0)
			{
				// Check this value is good type
				ParamType2 paramType = param->GetParameterType(id);
				bool typeOk;
				switch (type)
				{
					case TYPE_FILENAME:
					case TYPE_STRING:
						typeOk = (paramType == TYPE_FILENAME) || (paramType == TYPE_STRING);
						break;
					case TYPE_FILENAME_TAB:
					case TYPE_STRING_TAB:
						typeOk = (paramType == TYPE_FILENAME_TAB) || (paramType == TYPE_STRING_TAB);
						break;
					default:
						typeOk = (paramType == type);
						break;
				}
				if (typeOk)
				{
					// Get the value
					Interval ivalid=FOREVER;
					BOOL bRes=FALSE;
					switch (type)
					{
						case TYPE_PCNT_FRAC:
						case TYPE_FLOAT:
							bRes=param->GetValue(id, tvTime, *(float*)pValue, ivalid);
						break;
						case TYPE_BOOL:
						case TYPE_INT:
							bRes=param->GetValue(id, tvTime, *(int*)pValue, ivalid);
						break;
						case TYPE_RGBA:
						case TYPE_POINT3:
							bRes=param->GetValue(id, tvTime, *(Point3*)pValue, ivalid);
						break;
						case TYPE_FILENAME:
						case TYPE_STRING:
							*(std::string*)pValue = param->GetStr (id, tvTime);
							bRes = TRUE;
						break;
						case TYPE_FILENAME_TAB:
						case TYPE_STRING_TAB:
						{
							std::vector<std::string> &rTab = *(std::vector<std::string>*)pValue;
							uint total = param->Count (id);
							rTab.resize(total);
							for( uint i = 0; i < total; ++i )
								rTab[i] = param->GetStr (id, tvTime, i);
							bRes = TRUE;
						}
						break;
						case TYPE_BOOL_TAB:
						{
							std::vector<bool> &rTab = *(std::vector<bool>*)pValue;
							uint total = param->Count (id);
							rTab.resize(total);
							for( uint i = 0; i < total; ++i )
								rTab[i] = param->GetInt(id, tvTime, i) ? true : false;
							bRes = TRUE;
						}
						break;
						case TYPE_TEXMAP:
						{
							bRes=param->GetValue(id, tvTime, *(Texmap**)pValue, ivalid);
						}
						break;
					}

					// Get successful ?
					if (bRes)
						return true;
				}
				else
				{
					nlwarning("Invalid type specified for pblock2 value with name '%s', given type '%u', found '%u'", 
						sName, (uint32)type, (uint32)paramType);
				}
			}
		}
	}

		// Get num sub anim
	uint numSubAnim=node.NumSubs();
	// Visit sub anim
	for (uint s=0; s<numSubAnim; s++)
	{
		// If a sub anim is here, go to visit it.
		if (node.SubAnim(s))
		{
			// Sub anim name
			TSTR subName = node.SubAnimName(s);

			// Get the ctrl for sub anim
			if( getValueByNameUsingParamBlock2Internal (*node.SubAnim(s), sName, type, pValue, tvTime) )
				return true;
		}
	}

	// not found
	return false;
}

// --------------------------------------------------

// Return the pointer on the subanim with the name sName of the node. If it doesn't exist, return NULL.
bool CExportNel::getValueByNameUsingParamBlock2 (Animatable& node, const char* sName, ParamType2 type, void *pValue, TimeValue tvTime, bool verbose)
{
	if (getValueByNameUsingParamBlock2Internal (node, sName, type, pValue, tvTime))
	{
		return true;
	}
	else
	{
		// nlwarning ("FAILED Can't find ParamBlock named '%s'", sName);
		return false;
	}
}

// --------------------------------------------------

// Get the first modifier in the pipeline of a node by its class identifier
Modifier* CExportNel::getModifier (INode* pNode, Class_ID modCID)
{
	Object* pObj = pNode->GetObjectRef();

	if (!pObj)
		return NULL;

	ObjectState os = pNode->EvalWorldState(0);
	if (os.obj && (os.obj->SuperClassID() != GEOMOBJECT_CLASS_ID) && (os.obj->SuperClassID() != LIGHT_CLASS_ID) )
	{
		return NULL;
	}

	// For all derived objects (can be > 1)
	while (pObj && (pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID))
	{
		IDerivedObject* pDObj = (IDerivedObject*)pObj;
		int m;
		int nNumMods = pDObj->NumModifiers();
		// Step through all modififers and verify the class id
		for (m=0; m<nNumMods; ++m) 
		{
			Modifier* pMod = pDObj->GetModifier(m);
			if (pMod) 
			{
				if (pMod->ClassID() == modCID) 
				{
					// Match! Return it
					return pMod;
				}
			}
		}
		pObj = pDObj->GetObjRef();
	}

	return NULL;
}

// --------------------------------------------------

// Get the material name
std::string	CExportNel::getName (MtlBase& mtl)
{
	// Return its name
	TSTR name;
	name=mtl.GetName();
	return std::string (name);
}

// --------------------------------------------------

// Get the node name
std::string	CExportNel::getName (INode& mtl)
{
	// Return its name
	TCHAR* name=mtl.GetName();
	return std::string (name);
}


// --------------------------------------------------

// Get the NEL node name
std::string		CExportNel::getNelObjectName (INode& node)
{
	// Workaround for FX (don't know why, but the AppData are not copied when FX are duplicated, so try to get the name in another way)
	// If this is a particle system, try to get the name of the shape.from the param blocks
	ObjectState os = node.EvalWorldState(0);
    Object *obj = os.obj;
	// Check if there is an object
	if (obj)
	{
		Class_ID  clid = obj->ClassID();
		// is the object a particle system ? (we do this defore meshs, because for now there is a mesh in max scenes to say where a particle system is...)		
		if (clid.PartA() == NEL_PARTICLE_SYSTEM_CLASS_ID)
		{
			std::string shapeName;
			if (CExportNel::getValueByNameUsingParamBlock2(node, "ps_file_name", (ParamType2) TYPE_STRING, &shapeName, 0))
			{
				return NLMISC::CFile::getFilename(shapeName);
			}			
		}
	}
	// Try to get an APPDATA for the name of the object			
	AppDataChunk *ad = node.GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, NEL3D_APPDATA_INSTANCE_SHAPE);
	if (ad&&ad->data)
	{
		if (::strlen((const char *) ad->data) != 0)
		{		
			// Get the name of the object in the APP data
			return (const char*)ad->data;
		}
		else
		{
			return node.GetName();
		}
	}
	else
	{
		ObjectState os = node.EvalWorldState(0);
		Object *obj = os.obj;
		if (obj)
		{
			ad = obj->GetAppDataChunk (MAXSCRIPT_UTILITY_CLASS_ID, UTILITY_CLASS_ID, NEL3D_APPDATA_INSTANCE_SHAPE);
			if (ad&&ad->data)
			{
				if (::strlen((const char *) ad->data) != 0)
				{				
					// get file name only
					char fName[_MAX_FNAME];
					char ext[_MAX_FNAME];
					::_splitpath((const char*)ad->data, NULL, NULL, fName, ext) ;						
					return std::string(fName + std::string(ext));
				}
				else
				{
					return node.GetName();
				}
			}
			else
			{
				// Extract the node name
				return node.GetName();
			}
		}
		else
		{		
			// Extract the node name
			return node.GetName();
		}
	}
}

// --------------------------------------------------

void CExportNel::decompMatrix (NLMISC::CVector& nelScale, NLMISC::CQuat& nelRot, NLMISC::CVector& nelPos, 
								const Matrix3& maxMatrix)
{
	// Use decomp part of the max SDK
	AffineParts parts;
	decomp_affine(maxMatrix, &parts);

	// Check
	Matrix3 srtm, rtm, ptm, stm, ftm;
	ptm.IdentityMatrix();
	ptm.SetTrans(parts.t);
	parts.q.MakeMatrix(rtm);
	parts.u.MakeMatrix(srtm);
	stm = ScaleMatrix(parts.k);
	ftm = ScaleMatrix(Point3(parts.f,parts.f,parts.f)); 
	Matrix3 mat = Inverse(srtm) * stm * srtm * rtm * ftm * ptm;

	// Set the translation
	nelPos.x=parts.t.x;
	nelPos.y=parts.t.y;
	nelPos.z=parts.t.z;

	// Set the rotation
	nelRot.x=parts.q.x;
	nelRot.y=parts.q.y;
	nelRot.z=parts.q.z;
	nelRot.w=-parts.q.w;

	// Make a scale matrix
	parts.u.MakeMatrix(srtm);
	stm = ScaleMatrix(parts.k);
	mat = Inverse(srtm) * stm * srtm;

	// Get a NeL matrix
	CMatrix scaleMatrix;
	convertMatrix (scaleMatrix, mat);

	// Take the scale of this matrix, not very smart but..
	nelScale.x=parts.f*scaleMatrix.getI().x;
	nelScale.y=parts.f*scaleMatrix.getJ().y;
	nelScale.z=parts.f*scaleMatrix.getK().z;
}

// --------------------------------------------------

void CExportNel::getLocalMatrix (Matrix3& localMatrix, INode& node, TimeValue time)
{
	INode *parent;
	Matrix3 parentTM, nodeTM;
	nodeTM = node.GetNodeTM (time);
	parent = node.GetParentNode ();
	if (parent)
	{
		parentTM = parent->GetNodeTM (time);
		localMatrix = nodeTM*Inverse (parentTM);
	}
	else
		localMatrix = nodeTM;
}

// --------------------------------------------------

bool CExportNel::isMesh (INode& node, TimeValue time, bool excludeCollision)
{
	// Result false by default
	bool bRet=false;

	// Eval the object a time
	ObjectState os = node.EvalWorldState(time);

	// Object exist ?
	if (os.obj)
	{
		// Object can convert itself to NeL patchmesh ?
		if (os.obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
			bRet=true;
	}

	// Want to exclude collision Mesh??
	if(excludeCollision)
	{
		// Object is flagged as a collision?
		int	bCol= getScriptAppData(&node, NEL3D_APPDATA_COLLISION, BST_UNCHECKED);
		if(bCol == BST_CHECKED)
			bRet= false;
	}

	// Return result
	return bRet;
}

// --------------------------------------------------

bool CExportNel::isCamera (INode& node, TimeValue time)
{
	// Result false by default
	bool bRet=false;

	// Eval the object a time
	ObjectState os = node.EvalWorldState(time);

	// Object exist ?
	if (os.obj)
	{
		// Object can convert itself to NeL patchmesh ?
		if (os.obj->CanConvertToType(Class_ID(LOOKAT_CAM_CLASS_ID, 0)))
			bRet=true;
	}

	// Return result
	return bRet;
}

// --------------------------------------------------

bool	CExportNel::isDummy (INode& node, TimeValue time)
{
	ObjectState os = node.EvalWorldState(time);
	return os.obj->ClassID().PartA() == DUMMY_CLASS_ID;
}


// --------------------------------------------------

bool CExportNel::hasLightMap (INode& node, TimeValue time)
{
	if( !isMesh(node,time) )
		return false;

	int nMaterialCount=0;

	// Get primary material pointer of the node
	Mtl* pNodeMat=node.GetMtl();

	// If NULL, no material at all at this node
	if (pNodeMat==NULL)
		return false;

	// Number of sub material at in this material
	nMaterialCount=pNodeMat->NumSubMtls();

	// If it is a multisub object, export all its sub materials
	if (nMaterialCount>0)
	{
		// Check all the sub materials
		for (int nSub=0; nSub<nMaterialCount; nSub++)
		{
			// Get a pointer on the sub material
			Mtl* pSub=pNodeMat->GetSubMtl(nSub);

			// Should not be NULL
			nlassert (pSub);

			// Is there a lightmap handling wanted
			int bLightMap = 0; // false
			CExportNel::getValueByNameUsingParamBlock2 (*pSub, "bLightMap", (ParamType2)TYPE_BOOL, &bLightMap, 0);
			if (bLightMap)
				return true;
		}
	}
	// Else check only this material
	else
	{
		int bLightMap = 0; // false
		CExportNel::getValueByNameUsingParamBlock2 (*pNodeMat, "bLightMap", (ParamType2)TYPE_BOOL, &bLightMap, 0);
		if (bLightMap)
			return true;
	}
	return false;
}

// --------------------------------------------------

void CExportNel::outputErrorMessage (const char *message)
{
	if (_ErrorInDialog)
	{
		MessageBox (_Ip->GetMAXHWnd(), message, _ErrorTitle.c_str(), MB_OK|MB_ICONEXCLAMATION);
	}
	mprintf (message);
	mprintf ("\n");

	nlwarning ("Error in max file %s : ", _Ip->GetCurFilePath());
	nlwarning (message);
}

// --------------------------------------------------

void CExportNel::outputWarningMessage (const char *message)
{
	if (_ErrorInDialog)
	{
		MessageBox (_Ip->GetMAXHWnd(), message, _ErrorTitle.c_str(), MB_OK|MB_ICONEXCLAMATION);
	}
	mprintf (message);
	mprintf ("\n");

	nlwarning ("Warning in max file %s : ", _Ip->GetCurFilePath());
	nlwarning (message);
}

// --------------------------------------------------

bool CExportNel::isVegetable (INode& node, TimeValue time)
{
	return CExportNel::getScriptAppData (&node, NEL3D_APPDATA_VEGETABLE, BST_UNCHECKED) != BST_UNCHECKED;
}

// --------------------------------------------------

bool CExportNel::isLodCharacter (INode& node, TimeValue time)
{
	return CExportNel::getScriptAppData (&node, NEL3D_APPDATA_CHARACTER_LOD, BST_UNCHECKED) != BST_UNCHECKED;
}

// --------------------------------------------------

void CExportNel::addChildLodNode (std::set<INode*> &lodListToExclude, INode *current)
{
	// First node ?
	if (current == NULL)
		current = _Ip->GetRootNode();

	// Get child count
	uint lodCount = getScriptAppData (current, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
	for (uint lod=0; lod<lodCount; lod++)
	{
		// Get lod name
		std::string lodName = getScriptAppData (current, NEL3D_APPDATA_LOD_NAME+lod, "");
		if (lodName != "")
		{
			// Get the lod by name
			INode *lodNode = _Ip->GetINodeByName (lodName.c_str());
			if (lodNode)
			{
				// Insert it in the set
				lodListToExclude.insert (lodNode);
			}
		}
	}

	// Scan child nodes
	for ( uint i = 0; i < (uint)current->NumberOfChildren(); ++i )
		addChildLodNode ( lodListToExclude, current->GetChildNode(i) );
}

// --------------------------------------------------

void CExportNel::addParentLodNode (INode &child, std::set<INode*> &lodListToExclude, INode *parent)
{
	// First node ?
	if (parent == NULL)
		parent = _Ip->GetRootNode();

	// Get its child lod 
	uint lodCount = getScriptAppData (parent, NEL3D_APPDATA_LOD_NAME_COUNT, 0);
	for (uint lod=0; lod<lodCount; lod++)
	{
		// Get lod name
		std::string lodName = getScriptAppData (parent, NEL3D_APPDATA_LOD_NAME+lod, "");
		if (lodName != "")
		{
			// Get the lod by name
			INode *lodNode = _Ip->GetINodeByName (lodName.c_str());
			if (lodNode == &child)
			{
				// Insert it in the set
				lodListToExclude.insert (parent);
				break;
			}
		}
	}

	// Scan child nodes
	for ( uint i = 0; i < (uint)parent->NumberOfChildren(); ++i )
		addParentLodNode ( child, lodListToExclude, parent->GetChildNode(i) );
}

// --------------------------------------------------
/// Transform a 3dsmax uv matrix to a nel uv matrix
void CExportNel::uvMatrix2NelUVMatrix (const Matrix3& maxMatrix, NLMISC::CMatrix &dest)
{
		// Basis vector
	CVector I, J, K, P;

	// Build the rot matrix
	I.x= maxMatrix.GetRow(0).x;
	I.y= maxMatrix.GetRow(0).y;
	I.z= maxMatrix.GetRow(0).z;
	J.x= maxMatrix.GetRow(1).x;
	J.y= maxMatrix.GetRow(1).y;
	J.z= maxMatrix.GetRow(1).z;
	K.x= maxMatrix.GetRow(2).x;
	K.y= maxMatrix.GetRow(2).y;
	K.z= maxMatrix.GetRow(2).z;

	// Build the translation vector
	P.x= maxMatrix.GetTrans().x;
	P.y= maxMatrix.GetTrans().y;
	P.z= maxMatrix.GetTrans().z;

	// *** Build the NeL matrix

	// Set it to identity to have the good flags in it
	dest.identity();

	// Set the rotation part
	dest.setRot(I, J, K); 

	// Set the position part
	dest.setPos(P);	

	// transfo matrix
	
	CMatrix convert;
	convert.setRot(CVector::I, -CVector::J, CVector::K);
	convert.setPos(CVector::J);
	dest = convert * dest * convert; // exported v are already inverted therefore the conversion
}


// --------------------------------------------------
void CExportNel::getObjectNodes (std::vector<INode*>& vectNode, TimeValue time, INode* node)
{
	// Get the root node
	if (node==NULL)
		node=_Ip->GetRootNode();

	// Get a pointer on the object's node
    ObjectState os = node->EvalWorldState(time);
    Object *obj = os.obj;

	// Check if there is an object
	if (obj)
	{
		// Append this node.
		vectNode.push_back(node);
	}

	// Recurse sub node
	for (int i=0; i<node->NumberOfChildren(); i++)
		getObjectNodes (vectNode, time, node->GetChildNode(i));
}

// --------------------------------------------------
INode *CExportNel::getRootNode() const
{
	return _Ip->GetRootNode();
}

// ***********************************************************************************************

std::string		CExportNel::getAnimatedLight (INode *node)
{
	std::string		ret = CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_ANIMATED_LIGHT, NEL3D_APPDATA_LM_ANIMATED_LIGHT_DEFAULT);
	if (ret == "Sun")
		ret = "";
	if (ret == "GlobalLight")
		ret = "";
	if (ret == "(Use NelLight Modifier)")
		ret = "";

	return ret;
}

// ***********************************************************************************************

uint CExportNel::getLightGroup (INode *node)
{
	return CExportNel::getScriptAppData (node, NEL3D_APPDATA_LM_LIGHT_GROUP, NEL3D_APPDATA_LM_LIGHT_GROUP_DEFAULT);
}

// ***********************************************************************************************

// a segment (used by maxPolygonMeshToOrderedPoly)
struct CMaxMeshSeg
{
	uint V0, V1;
	// for map insertion
	bool operator<(const CMaxMeshSeg &other) const
	{ 
		uint lv0 = std::min(V0, V1);
		uint lv1 = std::max(V0, V1);

		uint rv0 = std::min(other.V0, other.V1);
		uint rv1 = std::max(other.V0, other.V1);


		if (lv0 != rv0) return lv0 < rv0;
		return lv1 < rv1;
	}
	bool operator==(const CMaxMeshSeg &other) const
	{
		return !(*this < other) && !(other < *this);
	}
	CMaxMeshSeg(uint v0, uint v1) : V0(v0),
							 V1(v1)				
	{			
	}
};

// private : predicate to ordonate segments (used by maxPolygonMeshToOrderedPoly)
struct CPredNextSegOf
{
	uint Prev;
	CPredNextSegOf(uint prev) : Prev(prev) {}
	bool operator()(const CMaxMeshSeg &value) const { return value.V0 == Prev || value.V1 == Prev; }		
};

/// Get normal of a max triangle in nel format
static NLMISC::CVector getMaxFaceNormal(const Mesh &m, const NLMISC::CMatrix &basis, uint faceIndex)
{
	CVector corner[3];
	for(uint k = 0; k < 3; ++k)
	{
		CExportNel::convertVector(corner[k], m.verts[m.faces[faceIndex].v[k]]);
		corner[k]= basis * corner[k];
	}
	CVector normal = (corner[1] - corner[0]) ^ (corner[2] - corner[1]);
	normal.normalize();
	return normal;
}
  



// --------------------------------------------------
// This convert a polygon expressed as a max mesh into a list of ordered vectors
void CExportNel::maxPolygonMeshToOrderedPoly(Mesh &mesh, std::vector<NLMISC::CVector> &dest, const NLMISC::CMatrix &basis, NLMISC::CVector &avgNormal)
{
	/// We use a very simple (but slow) algo : examine for each segment how many tris share it. If it is one then it is a border seg	 
	/// Then, just order segments
	
	typedef std::map<CMaxMeshSeg, uint> TSegMap;		
		
	avgNormal.set(0, 0, 0);

	/////////////////////////////////////////////////////////////
	// count the number of ref by a triangle for each segment  //
	/////////////////////////////////////////////////////////////

	TSegMap segs;
	uint k;
	for(k = 0; k < (uint) mesh.getNumFaces(); ++k)
	{
		avgNormal += getMaxFaceNormal(mesh, basis, k);
		for(uint l = 0; l < 3; ++l)
		{
			CMaxMeshSeg seg(mesh.faces[k].v[l], mesh.faces[k].v[(l + 1) % 3]);
			TSegMap::iterator it = segs.find(seg);
			if (it != segs.end())
			{
				++ it->second;
			}
			else
			{
				// create a new entry
				segs[seg] = 1;
			}
		}
	}

	avgNormal.normalize();


	////////////////////////////////////////
	// keep segments for which nbref is 1 //
	////////////////////////////////////////

	typedef std::list<CMaxMeshSeg> TSegList;	
	TSegList borderSegs;	
	for(TSegMap::const_iterator it = segs.begin(); it != segs.end(); ++it)
	{
		if (it->second == 1) borderSegs.push_back(it->first);
	}


	
	dest.clear();
	if (borderSegs.empty()) return;


	///////////////////////
	// ordonate segments //
	///////////////////////

	NLMISC::CVector pos;
	CExportNel::convertVector(pos, mesh.verts[borderSegs.begin()->V0]);
	dest.push_back(basis * pos);
	uint nextToFind = borderSegs.begin()->V1;
	borderSegs.pop_front();
	for(;;)
	{
		TSegList::iterator nextSeg = std::find_if(borderSegs.begin(), borderSegs.end(), CPredNextSegOf(nextToFind));
		if (nextSeg == borderSegs.end()) return;					
		CExportNel::convertVector(pos, mesh.verts[nextSeg->V0 == nextToFind ? nextSeg->V0 : nextSeg->V1]);
		dest.push_back(basis * pos);	
		nextToFind = (nextSeg->V0 == nextToFind) ? nextSeg->V1 : nextSeg->V0;
		borderSegs.erase(nextSeg);
	}


}



static std::string OldDecimalSeparatorLocale;

static void setDecimalSeparatorAsPoint()
{				
	OldDecimalSeparatorLocale = ::setlocale(LC_NUMERIC, NULL);
	::setlocale(LC_NUMERIC, "English");	
}

static void restoreDecimalSeparator()
{	
	::setlocale(LC_NUMERIC, OldDecimalSeparatorLocale.c_str());		
}


///=======================================================================
float toFloatMax(const char *src)
{
	float result = 0.f;
	if (toFloatMax(src, result)) return result;
	return 0.f;
}


bool		toFloatMax(const char *src, float &dest)
{	
	setDecimalSeparatorAsPoint();
	std::string str(src);
	std::string::size_type pointPos = str.find_first_of(",");
	if (pointPos != std::string::npos)
	{
		str[pointPos] ='.';
	}
	if (::sscanf(str.c_str(), "%g", &dest) == 1)
	{
		restoreDecimalSeparator();
		return true;
	}
	restoreDecimalSeparator();
	return false;

/*	if (!src || *src == '\0') return false;
	float result = 0.f;
	float sgn = 1.f;
	while (*src == ' ') ++src;
	if (*src == '-') { sgn = -1.f; ++src; }
	while (*src == ' ') ++src;
	bool numberFound = false;
	while (::isdigit(*src))
	{
		numberFound = true;
		result *= 10.f;
		result += (float) (*src - '0');
		++src;
	}
	if (!(*src == '.' || *src == ',')) 
	{	
		if (numberFound)
		{		
			dest = sgn * result;
			return true;
		}
		else
			return false;
	}
	++src;		
	if (!::isdigit(*src))
	{
		if (!numberFound)
		{
			return false;
		}
	}
	float addValue = 0.1f;	
	while (::isdigit(*src))
	{	
		result += addValue * (float) (*src - '0');
		addValue *= 0.1f;
		++ src;
	}
	dest = sgn * result;
	return true;*/
}


///=======================================================================
std::string toStringMax(float value)
{
	char result[256];
	setDecimalSeparatorAsPoint();
	::sprintf(result, "%g", value);
	restoreDecimalSeparator();
	return result;

/*	char result[256];
	sprintf(result, "%g", value);
	char *point = strchr(result, ',');
	if (point)
	{
		*point = '.';
	}
	return std::string(result); */
/*	double intPart;
	value = (float) ::modf(value, &intPart);
	bool positive = value >= 0.f && intPart >= 0.f;
	value = ::fabsf(value);
	intPart = ::fabs(intPart);
	std::string result = toString(intPart);
	if (value == 0.f) return positive ? result : "-" + result;
	result += ".";
	float frac;
	do
	{
		value *= 10.f;
		frac = (float) ::modf((double) value, &intPart);		
		result += toString((int) intPart % 10);		
	}
	while (frac != 0.f);
	return positive ? result : "-" + result;*/
}
///=======================================================================
std::string toStringMax(int value)
{	
	char result[256];
	setDecimalSeparatorAsPoint();
	::sprintf(result, "%d", value);
	restoreDecimalSeparator();
	return result;

	/*char str[256];
	::sprintf(str, "%d", value);	
	return str;*/
}
///=======================================================================
void CExportNel::buildCamera(NL3D::CCameraInfo &cameraInfo, INode& node, TimeValue time)
{
	// Eval the object a time
	ObjectState os = node.EvalWorldState(time);

	// Object exist ?
	if (os.obj)
	{
		// Object can convert itself to NeL patchmesh ?
		GenCamera *genCamera = (GenCamera *)os.obj->ConvertToType(time, Class_ID (LOOKAT_CAM_CLASS_ID, 0));
		if (genCamera)
		{
			INode *target = node.GetTarget();
			if (target)
			{
				cameraInfo.TargetMode = true;
				cameraInfo.UseFov = true;

				bool deleteIt = genCamera != os.obj;

				// Camera position
				Point3 pos=node.GetNodeTM(time).GetTrans ();
				CVector position;
				position.x=pos.x;
				position.y=pos.y;
				position.z=pos.z;
				cameraInfo.Pos = position;

				// Target position
				pos=target->GetNodeTM(time).GetTrans ();
				position.x=pos.x;
				position.y=pos.y;
				position.z=pos.z;
				cameraInfo.TargetPos = position;

				// Set the roll parameter
				cameraInfo.Roll = 0;

				// Set the fov
				cameraInfo.Fov = genCamera->GetFOV(time);

				if (deleteIt)
					genCamera->DeleteThis();
				genCamera = NULL;
			}
		}
	}
}

bool		CExportNel::isErrorInDialog () const
{
	return _ErrorInDialog;
}


