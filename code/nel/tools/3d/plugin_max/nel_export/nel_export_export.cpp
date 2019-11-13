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
#include "nel_export.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/3d/shape.h"
#include "nel/3d/animation.h"
#include "nel/3d/skeleton_shape.h"
#include "nel/3d/vegetable_shape.h"
#include "nel/3d/lod_character_shape.h"
#include "../nel_mesh_lib/export_nel.h"
#include "../nel_mesh_lib/export_appdata.h"


using namespace NL3D;
using namespace NLMISC;

// --------------------------------------------------

bool CNelExport::exportMesh (const std::string &sPath, INode& node, TimeValue time)
{
	// Result to return
	bool bRet = false;
	std::string tempFileName;
	std::string tempPathBuffer;

	try
	{
		tempPathBuffer = NLMISC::CPath::getTemporaryDirectory();
		NLMISC::CFile::getTemporaryOutputFilename(tempPathBuffer + "_nel_export_mesh_", tempFileName);

		// Eval the object a time
		ObjectState os = node.EvalWorldState(time);

		// Object exist ?
		if (os.obj)
		{
			// Skeleton shape
			CSkeletonShape * skeletonShape = NULL;
			TInodePtrInt *mapIdPtr=NULL;
			TInodePtrInt mapId;

			// If model skinned ?
			if (CExportNel::isSkin (node))
			{
				// Create a skeleton
				INode *skeletonRoot = CExportNel::getSkeletonRootBone(node);

				// Skeleton exist ?
				if (skeletonRoot)
				{
					// Build a skeleton
					skeletonShape = new CSkeletonShape();

					// Add skeleton bind pos info
					CExportNel::mapBoneBindPos boneBindPos;
					CExportNel::addSkeletonBindPos (node, boneBindPos);

					// Build the skeleton based on the bind pos information
					_ExportNel->buildSkeletonShape(*skeletonShape, *skeletonRoot, &boneBindPos, mapId, time);

					// Set the pointer to not NULL
					mapIdPtr=&mapId;

					// Erase the skeleton
					skeletonShape = NULL;
				}
			}

			DWORD t = timeGetTime();
			if (InfoLog)
				InfoLog->display("Beg buildShape %s \n", node.GetName());
			// Export in mesh format
			IShape *pShape = _ExportNel->buildShape(node, time, mapIdPtr, true);
			if (InfoLog)
				InfoLog->display("End buildShape in %d ms \n", timeGetTime()-t);

			// Conversion success ?
			if (pShape)
			{
				// Open a file
				COFile file;
				if (file.open(tempFileName))
				{
					try
					{
						// Create a streamable shape
						CShapeStream shapeStream(pShape);

						// Serial the shape
						shapeStream.serial(file);

						// Close the file
						file.close();

						// All is good
						bRet = true;
					}
					catch (...)
					{
						nlwarning("Shape serialization failed!");
						try
						{
							file.close();
						}
						catch (...)
						{

						}

						CFile::deleteFile(tempFileName);
					}
				}
				else
				{
					nlwarning("Failed to create file %s", tempFileName.c_str());
					if (_TerminateOnFileOpenIssues)
						nelExportTerminateProcess();
				}

				// Delete the pointer
				nldebug("Delete the pointer");
				try
				{
					bool tempBRet = bRet;
					bRet = false;
					delete pShape;
					bRet = tempBRet;
				}
				catch (...)
				{
					nlwarning("Failed to delete pShape pointer! Something might be wrong.");
					CFile::deleteFile(tempFileName);
					bRet = false;
				}

				// Verify the file
				nldebug("Verify exported shape file");
				try
				{
					bool tempBRet = bRet;
					bRet = false;
					CIFile vf;
					if (vf.open(tempFileName))
					{
						nldebug("File opened, size: %u", vf.getFileSize());
						CShapeStream s;
						s.serial(vf);
						nldebug("Shape serialized");
						vf.close();
						nldebug("File closed");
						delete s.getShapePointer();
						nldebug("Shape deleted");
						bRet = tempBRet;
					}
					else
					{
						nlwarning("Failed to open file: %s", tempFileName.c_str());
						if (_TerminateOnFileOpenIssues)
							nelExportTerminateProcess();
					}
				}
				catch (...)
				{
					nlwarning("Failed to verify shape. Must crash now.");
					CFile::deleteFile(tempFileName);
					bRet = false;
				}

			}
		}
	}
	catch (...)
	{
		nlwarning("Fatal exception at CNelExport::exportMesh.");
		bRet = false;
	}

	if (bRet)
	{
		CFile::deleteFile(sPath);
		CFile::moveFile(sPath, tempFileName);
		nlinfo("MOVE %s -> %s", tempFileName.c_str(), sPath.c_str());
	}

	return bRet;
}

// --------------------------------------------------

bool CNelExport::exportVegetable (const std::string &sPath, INode& node, TimeValue time)
{
	bool bRet=false;

	// Build a vegetable
	NL3D::CVegetableShape vegetable;
	if (_ExportNel->buildVegetableShape (vegetable, node, time))
	{
		// Open a file
		COFile file;
		if (file.open (sPath))
		{
			try
			{
				// Serial the shape
				vegetable.serial (file);

				// All is good
				bRet=true;
			}
			catch (const Exception &e)
			{
				// Message box
				const char *message = e.what();
				_ExportNel->outputErrorMessage ("Error during vegetable serialisation");
			}
		}
	}
	return bRet;
}

// --------------------------------------------------

bool CNelExport::exportAnim (const std::string &sPath, std::vector<INode*>& vectNode, TimeValue time, bool scene)
{
	// Result to return
	bool bRet=false;
	std::string tempFileName;
	std::string tempPathBuffer;

	try
	{
		tempPathBuffer = NLMISC::CPath::getTemporaryDirectory();
		NLMISC::CFile::getTemporaryOutputFilename(tempPathBuffer + "_nel_export_mesh_", tempFileName);

		// Create an animation file
		CAnimation animFile;

		// For each node to export
		for (uint n=0; n<vectNode.size(); n++)
		{
			// Get name
			std::string nodeName;

			// Get NEL3D_APPDATA_EXPORT_ANIMATION_PREFIXE_NAME
			int prefixe = CExportNel::getScriptAppData (vectNode[n], NEL3D_APPDATA_EXPORT_ANIMATION_PREFIXE_NAME, 0);

			// Set the name only if it is a scene animation
			if (scene || prefixe)
			{
				// try to get the prefix from the appData if present. If not, takes it from the node name
				nodeName = CExportNel::getScriptAppData (vectNode[n], NEL3D_APPDATA_INSTANCE_NAME, "");
				if (nodeName.empty()) // not found ?
				{
					nodeName=CExportNel::getName (*vectNode[n]);
				}
				nodeName+=".";
			}

			// Is a root ?
			bool root = vectNode[n]->GetParentNode () == _Ip->GetRootNode();

			// Add animation
			_ExportNel->addAnimation (animFile, *vectNode[n], nodeName.c_str(), root);
		}

		if (vectNode.size())
		{
			// Open a file
			COFile file;
			if (file.open (tempFileName))
			{
				try
				{
					nldebug("Serialize the animation");
					// Serial the animation
					animFile.serial (file);
					// Close the file
					file.close();
					// All is good
					bRet=true;
					// Verify the file
					nldebug("Verify exported anim file");
					try
					{
						bool tempBRet = bRet;
						bRet = false;
						CIFile vf;
						if (vf.open(tempFileName))
						{
							nldebug("File opened, size: %u", vf.getFileSize());
							CAnimation a;
							a.serial(vf);
							nldebug("Anim serialized");
							vf.close();
							nldebug("File closed");
							bRet = tempBRet;
						}
						else
						{
							nlwarning("Failed to open file: %s", tempFileName.c_str());
							bRet = false;
							if (_TerminateOnFileOpenIssues)
								nelExportTerminateProcess();
						}
					}
					catch (...)
					{
						nlwarning("Failed to verify shape. Must crash now.");
						CFile::deleteFile(tempFileName);
						bRet = false;
					}
				}
				catch (const Exception& e)
				{
					if (_ErrorInDialog)
						MessageBoxA (NULL, e.what(), "NeL export", MB_OK|MB_ICONEXCLAMATION);
					else
						nlwarning ("ERROR : %s", e.what ());
				}
			}
			else
			{
				if (_ErrorInDialog)
					MessageBox (NULL, _T("Can't open the file for writing."), _T("NeL export"), MB_OK|MB_ICONEXCLAMATION);
				else
					nlwarning ("ERROR : Can't open the file (%s) for writing", tempFileName.c_str());
				if (_TerminateOnFileOpenIssues)
					nelExportTerminateProcess();
			}
		}
	}
	catch (...)
	{
		nlwarning("Fatal exception at CNelExport::exportAnim.");
		bRet = false;
	}

	if (bRet)
	{
		CFile::deleteFile(sPath);
		CFile::moveFile(sPath, tempFileName);
		nlinfo("MOVE %s -> %s", tempFileName.c_str(), sPath.c_str());
	}
	return bRet;
}

// --------------------------------------------------

bool CNelExport::exportSkeleton	(const std::string &sPath, INode* pNode, TimeValue time)
{
	// Result to return
	bool bRet=false;

	// Build the skeleton format
	CSkeletonShape *skeletonShape=new CSkeletonShape();
	TInodePtrInt mapId;
	_ExportNel->buildSkeletonShape (*skeletonShape, *pNode, NULL, mapId, time);

	// Open a file
	COFile file;
	if (file.open (sPath))
	{
		try
		{
			// Create a streamable shape
			CShapeStream shapeStream (skeletonShape);

			// Serial the shape
			shapeStream.serial (file);

			// All is good
			bRet=true;
		}
		catch (const Exception &e)
		{
			nlwarning (e.what());
		}
	}

	// Delete the pointer
	delete skeletonShape;

	return bRet;
}

// --------------------------------------------------

bool CNelExport::exportLodCharacter (const std::string &sPath, INode& node, TimeValue time)
{
	// Result to return
	bool bRet=false;

	// Eval the object a time
	ObjectState os = node.EvalWorldState(time);

	// Object exist ?
	if (os.obj)
	{
		// Skeleton shape
		CSkeletonShape *skeletonShape=NULL;
		TInodePtrInt *mapIdPtr=NULL;
		TInodePtrInt mapId;

		// If model skinned ?
		if (CExportNel::isSkin (node))
		{
			// Create a skeleton
			INode *skeletonRoot=CExportNel::getSkeletonRootBone (node);

			// Skeleton exist ?
			if (skeletonRoot)
			{
				// Build a skeleton
				skeletonShape=new CSkeletonShape();

				// Add skeleton bind pos info
				CExportNel::mapBoneBindPos boneBindPos;
				CExportNel::addSkeletonBindPos (node, boneBindPos);

				// Build the skeleton based on the bind pos information
				_ExportNel->buildSkeletonShape (*skeletonShape, *skeletonRoot, &boneBindPos, mapId, time);

				// Set the pointer to not NULL
				mapIdPtr=&mapId;

				// Erase the skeleton
				if (skeletonShape)
					delete skeletonShape;
			}
		}

		// Conversion success ?
		CLodCharacterShapeBuild		lodBuild;
		if (_ExportNel->buildLodCharacter (lodBuild, node, time, mapIdPtr) )
		{
			// Open a file
			COFile file;
			if (file.open (sPath))
			{
				try
				{
					// Serial the shape
					lodBuild.serial (file);

					// All is good
					bRet=true;
				}
				catch (...)
				{
				}
			}
		}
	}
	return bRet;
}
