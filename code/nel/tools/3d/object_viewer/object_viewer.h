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

#if !defined(AFX_OBJECT_VIEWER_H__9B22CB84_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
#define AFX_OBJECT_VIEWER_H__9B22CB84_1929_11D5_9CD4_0050DAC3A412__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define REGKEY_OBJ_VIEW "Software\\Nevrax\\nel\\object_viewer"
#define REGKEY_OBJ_VIEW_OPENGL_WND "Software\\Nevrax\\nel\\object_viewer\\opengl_wnd"
#define REGKEY_OBJ_VIEW_SCENE_DLG "Software\\Nevrax\\nel\\object_viewer\\scene_dlg"
#define REGKEY_OBJ_VIEW_ANIMATION_DLG "Software\\Nevrax\\nel\\object_viewer\\animation_dlg"
#define REGKEY_OBJ_VIEW_ANIMATION_SET_DLG "Software\\Nevrax\\nel\\object_viewer\\animation_set_dlg"
#define REGKEY_OBJ_VIEW_SLOT_DLG "Software\\Nevrax\\nel\\object_viewer\\slot_dlg"
#define REGKEY_OBJ_PARTICLE_DLG "Software\\Nevrax\\nel\\object_viewer\\particle_dlg"
#define REGKEY_OBJ_DAYNIGHT_DLG "Software\\Nevrax\\nel\\object_viewer\\daynight_dlg"
#define REGKEY_OBJ_WATERPOOL_DLG "Software\\Nevrax\\nel\\object_viewer\\daynight_dlg"
#define REGKEY_OBJ_VIEW_VEGETABLE_DLG "Software\\Nevrax\\nel\\object_viewer\\vegetable_dlg"
#define REGKEY_OBJ_GLOBAL_WIND_DLG "Software\\Nevrax\\nel\\object_viewer\\global_wind_dlg"
#define REGKEY_OBJ_SOUND_ANIM_DLG "Software\\Nevrax\\nel\\object_viewer\\sound_anim_dlg"
#define REGKEY_OBJ_LIGHT_GROUP_DLG "Software\\Nevrax\\nel\\object_viewer\\light_group_dlg"
#define REGKEY_CHOOSE_FRAME_DELAY_DLG "Software\\Nevrax\\nel\\object_viewer\\choose_frame_delay_dlg"
#define REGKEY_CHOOSE_BG_COLOR_DLG "Software\\Nevrax\\nel\\object_viewer\\choose_bg_color_dlg"
#define REGKEY_CHOOSE_SUN_COLOR_DLG "Software\\Nevrax\\nel\\object_viewer\\choose_sun_color_dlg"
#define REGKEY_SKELETON_SCALE_DLG "Software\\Nevrax\\nel\\object_viewer\\skeleton_scale_dlg"
#define REGKEY_TUNE_MRM_DLG "Software\\Nevrax\\nel\\object_viewer\\tune_mrm_dlg"



#include "resource.h"
#include "object_viewer_interface.h"
#include "main_dlg.h"
#include "animation_set_dlg.h"
#include "animation_dlg.h"

#include "nel/3d/animation_set.h"
#include "nel/3d/channel_mixer.h"
#include "nel/3d/shape.h"
#include "nel/3d/event_mouse_listener.h"
#include "nel/3d/light.h"
#include "nel/3d/font_manager.h"
#include "nel/3d/cloud_scape.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/stream.h"

#include "nel/3d/logic_info.h"
#include "nel/pacs/u_global_position.h"

namespace NL3D
{
class CFontGenerator;
class CWaterPoolManager;
class CTileVegetableDesc;
class CLandscapeModel;
class CVisualCollisionManager;
class CVisualCollisionEntity;
}

class CMainFrame;

class CParticleDlg;
class CDayNightDlg;
class CWaterPoolEditor;
class CVegetableDlg;
class CGlobalWindDlg;
class CSoundAnimDlg;
class CLightGroupFactor;
class CChooseFrameDelay;
class CChooseBGColorDlg;
class CChooseSunColorDlg;
class CSkeletonScaleDlg;
class CTuneMrmDlg;



/////////////////////////////////////////////////////////////////////////////
// CObject_viewerApp
// See object_viewer.cpp for the implementation of this class
//

class CObject_viewerApp : public CWinApp
{
public:
	CObject_viewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObject_viewerApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CObject_viewerApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

class CSlotInfo
{
public:
	CSlotInfo ();
	std::string		Animation;
	std::string		Skeleton;
	sint32			Offset;
	sint32			StartTime;
	sint32			EndTime;
	float			StartBlend;
	float			EndBlend;
	float			Smoothness;
	float			SpeedFactor;
	sint32			ClampMode;
	bool			SkeletonInverted;
	bool			Enable;

	// Serial
	void serial (NLMISC::IStream& s);
};

/////////////////////////////////////////////////////////////////////////////

class CInstanceSave
{
public:
	CInstanceSave ();

	// Play list of this object
	std::vector<std::string>	PlayList;

	// Slot info for this object
	CSlotInfo					SlotInfo[NL3D::CChannelMixer::NumAnimationSlot];

	// Camera info
	NL3D::CCameraInfo			CameraInfo;

	// Camera ?
	bool						Camera;

	// Input file
	std::string					ShapeFilename;

	// Skeleton id
	uint32						SkeletonId;

	// Bin bone name
	std::string					BindBoneName;

	// Is a skeleton
	bool						IsSkeleton;

	// Animation input file
	std::vector<std::string>	AnimationFileName;

	// Skeleton weight input file
	std::vector<std::string>	SWTFileName;

	// Serial
	void serial (NLMISC::IStream& s);
};

/////////////////////////////////////////////////////////////////////////////

class CInstanceInfo
{
public:
	CInstanceInfo ();
	~CInstanceInfo ();

	// Camera pointer
	NL3D::CCamera				*Camera;

	// Transform shape pointer
	NL3D::CTransformShape		*TransformShape;

	// Animation set of this instance
	NL3D::CAnimationSet			AnimationSet;

	// Channel mixer of this instance
	NL3D::CChannelMixer			ChannelMixer;

	// True, must delete this shape
	bool						MustDelete;

	// Save information
	CInstanceSave				Saved;

	// The current playlist
	NL3D::CAnimationPlaylist	Playlist;

	// Set an animation playlist
	void						setAnimationPlaylist (float frameRate);
	
};

/////////////////////////////////////////////////////////////////////////////

class CObjectViewer : public IObjectViewer
{
public:
	// Name of particle workspace, just used to save config file
	std::string									 ParticleWorkspaceFilename;
	friend class CMainFrame;
	friend class CAnimationSetDlg;
public:
	CObjectViewer ( );
	virtual ~CObjectViewer ();

	// Init the UI
	bool initUI (HWND parent=NULL);
	

	// Go
	void go ();

	virtual bool	isInstanceRunning() { return _InstanceRunning; }

	// Release the UI
	void releaseUI ();

	// Set single animtion.
	void setSingleAnimation (NL3D::CAnimation*	pAnim, const char* name, uint instance);

	// Set automatic animation
	void setAutoAnimation (NL3D::CAnimationSet* pAnimSet);

	// Add a mesh
	uint addMesh (NL3D::IShape* pMeshShape, const char* meshName, uint skelIndex, const char* bindSkelName = NULL, bool createInstance = true);

	// Add a camera
	uint addCamera (const NL3D::CCameraInfo &cameraInfo, const char* cameraName);

	// Add a skel  
	uint addSkel (NL3D::IShape* pSkelShape, const char* skelName);

	// remove all instances from the scene
	void		 removeAllInstancesFromScene();

	// Start / stop fxs (remanence for now..)
	void		 enableFXs(bool enabled);	


	/// Force all the instances of the scene to use the given texture set (if available)
	void activateTextureSet(uint index);

	/// Select random textures sets
	void shuffleTextureSet();

	// Load a mesh
	bool loadMesh (std::vector<std::string> &meshFilename, const char* skeleton="");

	// Load an instance group
	bool loadInstanceGroup(const char *igFilename);

	// Set ambient color
	void setAmbientColor (const NLMISC::CRGBA& color);

	// Set ambient color
	void setLight (unsigned char id, const NL3D::CLight& light);

	// Get edited object, 0xffffffff if no object edited
	uint getEditedObject ();

	// Set edited object, 0xffffffff if no object edited
	void setEditedObject (uint selected);
	
	// Get an instance
	CInstanceInfo *getInstance (uint instance);

	// Get the slot dialog
	CMainDlg *getSlotDlg ();

	// Get number of instances
	uint getNumInstance () const;

	// Add an animation
	void addAnimation (NL3D::CAnimation* anim, const char* filename, const char* name, uint instance);

	// Update all objects that depend on the animation set
	void refreshAnimationListeners();

	// Load a shape
	void resetCamera ();

	// get the mouse listener
	NL3D::CEvent3dMouseListener &getMouseListener(void) { return _MouseListener ; }
	const NL3D::CEvent3dMouseListener &getMouseListener(void) const { return _MouseListener ; }

	// get the particle dialog
	CParticleDlg *getParticleDialog(void) const { return _ParticleDlg ; }	

	// get the frame delay dialog
	CChooseFrameDelay *getFrameDelayDlg() const { return _ChooseFrameDelayDlg; }

	// Load animation
	void loadAnimation (const char* fileName, uint instance);

	// Load a skeleton template
	void loadSWT (const char* fileName, uint instance);


	/// Not exported
	// @{

	void setAnimTime (float animStart, float animEnd);

	// Reset the slots
	void resetSlots (uint instance);

	// Reinit and refill the channel mixer channels
	void reinitChannels ();

	// Return the frame rate
	float getFrameRate ();

	// Serial the config
	void serial (NLMISC::IStream& f);

	/// set the backgournd color
	void setBackGroundColor(const NLMISC::CRGBA& col) { _BackGroundColor = col ; }

	/// get the background color
	NLMISC::CRGBA getBackGroundColor(void) { return _BackGroundColor ; }

	/// Get font manager
	NL3D::CFontManager		*getFontManager () { return &_FontManager; };

	/// Get font generator
	NL3D::CFontGenerator	*getFontGenerator () { return _FontGenerator; };
	// @}

	/** an interface for objects that want to be called during the loop
	  * First use : display of an optional bbox for a particle system
	  */
	struct IMainLoopCallBack
	{
		/// this will be called each time the main loop is processed
		virtual void goPostRender() = 0;
		virtual void goPreRender() = 0;
	} ;

	/** add an object that will be notified each time a frame is processed
	  * \see removeMainLoopCallBack()
	  */
	void registerMainLoopCallBack(IMainLoopCallBack *i)  ;

	/// remove an object that was registered with registerMainLoopCallBack()
	void removeMainLoopCallBack(IMainLoopCallBack *i) ;
	  
	/// set the lag in milliseconds between each frame; 0 is the default
	void setFrameDelay(uint32 milliSec) { _FrameDelay = milliSec; }

	/// get the frame delay in milliseconds
	uint32 getFrameDelay() const		  { return _FrameDelay; }

	/// eval sound tracks
	void evalSoundTrack (float lastTime, float currentTime);

	/// Setup the playlist with the playlist
	void setupPlaylist (float time);
		
	/// Enable disable channels
	void enableChannels ();
		
	/// Setup transform positions
	void setupPositions ();

	/// Manage matrix increment
	void addTransformation (NLMISC::CMatrix &current, NL3D::CAnimation *anim, float begin, float end, NL3D::ITrack *posTrack, NL3D::ITrack *rotquatTrack, 
									   NL3D::ITrack *nextPosTrack, NL3D::ITrack *nextRotquatTrack, bool removeLast);

	/// inherited from CObjectViewerInterface
	void setWaterPoolManager(NL3D::CWaterPoolManager &wpm) { _Wpm = &wpm; }

	NL3D::CWaterPoolManager &getWaterPoolManager() { return *_Wpm; }

	// Reload textures
	void reloadTextures ();

	/** inherited from CObjectViewerInterface
	 */
	virtual uint addInstanceGroup(NL3D::CInstanceGroup *ig);

	/** inherited from CObjectViewerInterface
	 */
	virtual void setupSceneLightingSystem(bool enable, const NLMISC::CVector &sunDir, NLMISC::CRGBA sunAmbiant, NLMISC::CRGBA sunDiffuse, NLMISC::CRGBA sunSpecular);

	/** inherited from CObjectViewerInterface
	 */
	virtual void enableDynamicObjectLightingTest(NLPACS::CGlobalRetriever *globalRetriever, NL3D::CInstanceGroup *ig);

	/// \name Landscape Vegetable Edition
	// @{

	/// true if landscape is created
	bool		isVegetableLandscapeCreated() const {return _VegetableLandscape!=NULL;}

	/// load the landscape with help of setup in object_viewer.cfg. return true if OK.
	bool		createVegetableLandscape();

	/// if created, show the landscape
	void		showVegetableLandscape();
	/// if created, hide the landscape
	void		hideVegetableLandscape();

	/// display vegetable with landscape
	void		enableLandscapeVegetable(bool enable);

	/// refresh the vegetables in landscape, with the new vegetableSet
	void		refreshVegetableLandscape(const NL3D::CTileVegetableDesc &tvdesc);


	/// get vegetable Wind wetup.
	float		getVegetableWindPower() const {return _VegetableWindPower;}
	float		getVegetableWindBendStart() const {return _VegetableWindBendMin;}
	float		getVegetableWindFrequency() const {return _VegetableWindFreq;}
	/// set vegetable Wind wetup (updat view if possible)
	void		setVegetableWindPower(float w);
	void		setVegetableWindBendStart(float w);
	void		setVegetableWindFrequency(float w);

	/// if enable, snap the camera to the ground of the landscape.
	void		snapToGroundVegetableLandscape(bool enable);

	// @}


	/// Get/Set global Scene wind setup
	float		getGlobalWindPower() const;
	void		setGlobalWindPower(float w);

	/// Get the scene roor for manip.
	NL3D::CTransform	*getSceneRoot() const {return _SceneRoot;}

	// Shoot the scene
	void		shootScene ();

	// Camera methods

	// Return -1 if no current camera
	sint		getCurrentCamera () const;

	// -1 If no current camera
	void		setCurrentCamera (sint currentCamera);
	
	// Get the instance id of a camera
	uint		getCameraInstance (uint cameraId) const;

	// Get the camera count
	uint		getNumCamera () const;

	// Reinit camera
	void		initCamera ();

	/** popup a dialog to choose a bone in the scene
	  * \return true is a bone has been selected
	  */
	bool		chooseBone(const std::string &caption, NL3D::CSkeletonModel *&skel, uint &boneIndex, std::string *skelName = NULL, std::string *boneName = NULL);

	// test if there's a skeleton in the scene
	bool		isSkeletonPresent() const;

	// get the main frame
	CMainFrame *getMainFrame() const { return _MainFrame; }	

	const NLMISC::CMatrix &getFXUserMatrix() const { return _FXUserMatrix; }
	void setFXUserMatrix(const NLMISC::CMatrix &fxUserMatrix) {	_FXUserMatrix = fxUserMatrix; }

	void		setSceneMatrixVisible(bool visible) { _SceneMatrixVisible = visible; }
	bool		getSceneMatrixVisible() const { return _SceneMatrixVisible; }
	void		setFXMatrixVisible(bool visible) { _FXMatrixVisible = visible; }
	bool		getFXMatrixVisible() const { return _FXMatrixVisible; }
	void		setFXUserMatrixVisible(bool visible) { _FXUserMatrixVisible = visible; }
	bool		getFXUserMatrixVisible() const { return _FXUserMatrixVisible; }
	void		setOcclusionTestMeshsVisible(bool visible) { _OcclusionTestMeshsVisible = visible; }
	bool		getOcclusionTestMeshsVisible() const { return _OcclusionTestMeshsVisible; }

	

private:

	CMainFrame									*_MainFrame;
	CAnimationDlg								*_AnimationDlg;
	CMainDlg									*_SlotDlg;
	CAnimationSetDlg							*_AnimationSetDlg;
	CParticleDlg								*_ParticleDlg;
	CDayNightDlg								*_DayNightDlg;
	CWaterPoolEditor							*_WaterPoolDlg;
	CVegetableDlg								*_VegetableDlg;
	CSoundAnimDlg								*_SoundAnimDlg;
	CLightGroupFactor							*_LightGroupDlg;
	CChooseFrameDelay							*_ChooseFrameDelayDlg;
	CChooseBGColorDlg							*_ChooseBGColorDlg;
	CChooseSunColorDlg							*_ChooseSunColorDlg;
	CSkeletonScaleDlg							*_SkeletonScaleDlg;
	CTuneMrmDlg									*_TuneMRMDlg;
	uint32										_SelectedObject;
	sint										_CurrentCamera;

	// FX user matrix
	NLMISC::CMatrix								_FXUserMatrix;

	// Vector of camera
	std::vector<uint>							_Cameras;

	// Vector of loaded instance
	std::vector<CInstanceInfo*>					_ListInstance;

	CGlobalWindDlg								*_GlobalWindDlg;
	NL3D::CEvent3dMouseListener					_MouseListener;
	NLMISC::CRGBA								_HotSpotColor;
	float										_HotSpotSize;
	NLMISC::CRGBA								_BackGroundColor;
	NLMISC::CVector								_SceneCenter;
	// List of static InstanceGroup.
	std::vector<class NL3D::CInstanceGroup*>	_ListIG;

	// Font mgt
	NL3D::CFontManager							_FontManager;
	NL3D::CFontGenerator						*_FontGenerator;	
	std::string									_FontPath;
	std::vector<IMainLoopCallBack *>			_CallBackList;
	uint32										_FrameDelay; 
	float										_CameraFocal;	
	float										_LastTime;
	NL3D::CWaterPoolManager						*_Wpm;

	/// \name Vegetable Edition
	// @{
	/// Our landscape
	NL3D::CLandscapeModel						*_VegetableLandscape;

	// File info to build it
	std::string									_VegetableLandscapeTileBank;
	std::string									_VegetableLandscapeTileFarBank;
	std::vector<std::string>					_VegetableLandscapeZoneNames;

	// Misc.
	float										_VegetableLandscapeThreshold;
	float										_VegetableLandscapeTileNear;
	NLMISC::CRGBA								_VegetableLandscapeAmbient;
	NLMISC::CRGBA								_VegetableLandscapeDiffuse;
	std::string									_VegetableTexture;
	NLMISC::CRGBA								_VegetableAmbient;
	NLMISC::CRGBA								_VegetableDiffuse;
	NLMISC::CVector								_VegetableLightDir;
	// Vegetable wind.
	NLMISC::CVector								_VegetableWindDir;
	float										_VegetableWindFreq;
	float										_VegetableWindPower;
	float										_VegetableWindBendMin;

	bool										_VegetableEnabled;

	// Collision
	bool										_VegetableSnapToGround;
	float										_VegetableSnapHeight;
	NL3D::CVisualCollisionManager				*_VegetableCollisionManager;
	NL3D::CVisualCollisionEntity				*_VegetableCollisionEntity;

	// load cfg.
	void		loadVegetableLandscapeCfg(NLMISC::CConfigFile &cf);

	// @}

	// Lightigng Setup (from cfg).
	bool										_SceneLightEnabled;
	NLMISC::CVector								_SceneLightSunDir;
	NLMISC::CRGBA								_SceneLightSunAmbiant;
	NLMISC::CRGBA								_SceneLightSunDiffuse;
	NLMISC::CRGBA								_SceneLightSunSpecular;

	/// \name dynamic object lighting testing
	// @{
	NLPACS::CGlobalRetriever					*_GlobalRetriever;
	NL3D::CTransformShape						*_ObjectLightTest;
	std::string									_ObjectLightTestShape;
	// The matrix not snapped by pacs.
	NLMISC::CMatrix								_ObjectLightTestMatrix;

	bool										_FXMatrixVisible;
	bool										_FXUserMatrixVisible;
	bool										_SceneMatrixVisible;
	bool										_OcclusionTestMeshsVisible;

	// the lightInfo linked to the ig.
	class	COVLogicInfo : public NL3D::ILogicInfo
	{
	public:
		NLPACS::UGlobalPosition		GPos;
		NL3D::CInstanceGroup		*Ig;
		NLPACS::CGlobalRetriever	*GlobalRetriever;

		virtual void	getStaticLightSetup(NLMISC::CRGBA sunAmbient, std::vector<NL3D::CPointLightInfluence> &pointLightList, uint8 &sunContribution, NLMISC::CRGBA &ambient);
	};
	// instnace
	COVLogicInfo								_ObjectLightTestLogicInfo;

	// @}

	static bool									_InstanceRunning;

	// Scale the pos of the skeleton
	float										_CharacterScalePos;

	// Driver in direct3d ?
	bool										_Direct3d;

	// Fog
	bool										_Fog;
	float										_FogStart;
	float										_FogEnd;
	NLMISC::CRGBA								_FogColor;

	// Cloud scape
	NL3D::SCloudScapeSetup						_CSS;
	NL3D::CCloudScape							*_CS;

	// The root of all objects added to the scene. Rotated for user convenience
	NL3D::CTransform							*_SceneRoot;

	

	// load the config file
	void	loadConfigFile();
	// get path of the dll
	std::string getModulePath() const;
	// load driver name from the config file
	void	loadDriverName();

	void	drawFXUserMatrix();	
	void	drawFXMatrix();
	void	drawSceneMatrix();

	void	drawNamedMatrix(const NLMISC::CMatrix &matrix, const std::string &name, NLMISC::CRGBA color, float textZOffset, float testSize);
};

void setRegisterWindowState (const CWnd *pWnd, const char* keyName);
void getRegisterWindowState (CWnd *pWnd, const char* keyName, bool resize);

/** display a localized message box. Caption an message text are retrieved from the string table
  * \param parentWindow    Pointer to initiator of the message box
  * \param messageStringID ID in string table of the message box content
  * \param captionStringID ID in string table of the message box caption
  * \param nType		   Type of message box (the same as the last parameter of ::MessageBox)
  * \return				   Same value than ::MessageBox
  */
int localizedMessageBox(HWND parentWindow, int messageStringID, int captionStringID, UINT nType);
int localizedMessageBox(HWND parentWindow, const char *message, int captionStringID, UINT nType);

// Return a string from the string table
CString getStrRsc(uint stringID);


bool browseFolder(const CString &caption, CString &destFolder, HWND parent);


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECT_VIEWER_H__9B22CB84_1929_11D5_9CD4_0050DAC3A412__INCLUDED_)
