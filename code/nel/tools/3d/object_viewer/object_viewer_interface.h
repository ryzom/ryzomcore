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

#ifndef OBJECT_VIEWER_INTERFACE
#define OBJECT_VIEWER_INTERFACE

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

#ifndef OBJECT_VIEWER_EXPORT
#define OBJECT_VIEWER_EXPORT __declspec( dllimport )
#endif // OBJECT_VIEWER_EXPORT

// Increment this version number each time you distribute a new version of the dll.
#define OBJECT_VIEWER_VERSION 4

namespace NL3D
{
	class IShape;
	class CAnimation;
	class CLight;
	class CTransformShape;
	class CSkeletonModel;
	class CWaterPoolManager;
	class CCameraInfo;
	class CAnimationSet;
	class CInstanceGroup;
}

namespace NLMISC
{
	class CRGBA;
}

namespace NLPACS
{
	class	CRetrieverBank;
	class	CGlobalRetriever;
}

class IObjectViewer
{
public:
	virtual ~IObjectViewer () {}

	// Init the UI
	virtual bool initUI (HWND parent=NULL) = 0;

	// Test whether an instance of the viewer is running
	virtual bool isInstanceRunning() = 0;

	// Go. It shouldn't be called if there's an instance of the viewer that is running.
	virtual void go () = 0;

	// Release the UI
	virtual void releaseUI () = 0;

	// Add a mesh
	virtual uint addMesh(NL3D::IShape* pMeshShape, const std::string &meshName, uint skelIndex, const char* bindSkelName = NULL, bool createInstance = true) = 0;

	// Add a skel
	virtual uint addSkel (NL3D::IShape* pSkelShape, const std::string &skelName) = 0;

	// Add a camera
	virtual uint addCamera (const NL3D::CCameraInfo &cameraInfo, const std::string &cameraName) = 0;

	// remove all the instance from the scene
	virtual void					 removeAllInstancesFromScene() = 0;

	// Load a mesh
	virtual bool loadMesh (std::vector<std::string> &meshFilename, const std::string &skeleton) = 0;

	// Load a shape
	virtual void resetCamera () = 0;

	// Set single animation
	virtual void setSingleAnimation (NL3D::CAnimation* pAnim, const std::string &name, uint instance) = 0;

	// Set automatic animation
	virtual void setAutoAnimation (NL3D::CAnimationSet* pAnimSet) = 0;

	// Set ambient color
	virtual void setAmbientColor (const NLMISC::CRGBA& color) = 0;

	// Set background color
	virtual void setBackGroundColor (const NLMISC::CRGBA& color) = 0;

	// Set ambient color
	virtual void setLight (unsigned char id, const NL3D::CLight& light) = 0;

	/** set the water pool manager used by the object viewer. Must be the same than tyhe one of the dll which created the models 
	  * (because the 3d lib is duplicated : one version in the viewer, and one version in the exporter)
	  */
	virtual void setWaterPoolManager(NL3D::CWaterPoolManager &wpm) = 0;

	/** Add an InstanceGroup. ptr Will be deleted by objectViewer.
	 */
	virtual uint addInstanceGroup(NL3D::CInstanceGroup *ig) = 0;

	/** Setup Scene lighting System. Disabled by default
	 */
	virtual void setupSceneLightingSystem(bool enable, const NLMISC::CVector &sunDir, NLMISC::CRGBA sunAmbiant, NLMISC::CRGBA sunDiffuse, NLMISC::CRGBA sunSpecular) = 0;

	/** Scene lighting System: enable dynamic object testing. Give the ig the shape must be light_tested against
	 *	globalRetriever is not deleted by the ObjectViewer.
	 */
	virtual void enableDynamicObjectLightingTest(NLPACS::CGlobalRetriever *globalRetriever, NL3D::CInstanceGroup *ig) = 0;


	// Get instance
	static OBJECT_VIEWER_EXPORT IObjectViewer* getInterface (int version=OBJECT_VIEWER_VERSION);

	// Release instance
	static OBJECT_VIEWER_EXPORT void releaseInterface (IObjectViewer* view);
};

#endif OBJECT_VIEWER_INTERFACE
