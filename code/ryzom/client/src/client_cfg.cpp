// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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




/////////////
// INCLUDE //
/////////////
#include "stdpch.h"
// Misc.
#include "nel/misc/config_file.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/i18n.h"
// Client.
#include "client_cfg.h"
#include "entities.h"
#include "cursor_functions.h"
#include "debug_client.h"
#include "view.h"	// For the cameraDistance funtion
#include "user_entity.h"
#include "misc.h"

// 3D Interface.
#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
// Game Share.
#include "game_share/time_weather_season/time_and_season.h"
#include "game_share/ryzom_version.h"

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif // HAVE_CONFIG_H

#ifdef NL_OS_MAC
#include "app_bundle_utils.h"
#endif // NL_OS_MAC

#include <locale.h>

///////////
// MACRO //
///////////
//-----------------------------------------------
/// Macro to read a Bool from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_BOOL(variableName)											\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asInt() ? true : false;		\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read an Int from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_INT(variableName)											\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asInt();						\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a Float from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_FLOAT(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asFloat();						\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a Float from the CFG.
/// cfgVariableName : Variable Name to Read.
/// variableName : Variable Name to Set.
//-----------------------------------------------
#define _READ_FLOAT2(cfgVariableName,variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#cfgVariableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asFloat();						\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#cfgVariableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a Double from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_DOUBLE(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asDouble();					\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a String from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_STRING(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = varPtr->asString();					\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a CVector from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_CVECTOR(variableName)										\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	if(varPtr)															\
	{																	\
		/* Check params */												\
		if(varPtr->size()==3)											\
		{																\
			ClientCfg.variableName.x = varPtr->asFloat(0);				\
			ClientCfg.variableName.y = varPtr->asFloat(1);				\
			ClientCfg.variableName.z = varPtr->asFloat(2);				\
		}																\
		else															\
			cfgWarning("CFG: Bad params for '"#variableName"' !!!");		\
	}																	\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\


//-----------------------------------------------
/// Macro to read an Enum, as int from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_ENUM_ASINT(type, variableName)							\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
		ClientCfg.variableName = (type)varPtr->asInt();					\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
/// Macro to read a String Vector from the CFG.
/// variableName : Variable Name to Read and Set.
//-----------------------------------------------
#define _READ_STRINGVECTOR(variableName)								\
	/* Read the Variable Value From Script */							\
	varPtr = ClientCfg.ConfigFile.getVarPtr(#variableName);				\
	/* Value found, set the Variable */									\
	if(varPtr)															\
	{																	\
		ClientCfg.variableName.clear ();								\
		int iSz = varPtr->size();										\
		ClientCfg.variableName.reserve(iSz);							\
		for (int i = 0; i < iSz; i++)									\
			ClientCfg.variableName.push_back(varPtr->asString(i));		\
	}																	\
	/* Use the Default Value */											\
	else																\
		cfgWarning("CFG: Default value used for '"#variableName"' !!!");	\

//-----------------------------------------------
// Macro for the dev version
//-----------------------------------------------

#if !FINAL_VERSION
#define READ_BOOL_DEV(variableName) _READ_BOOL(variableName)
#define READ_INT_DEV(variableName) _READ_INT(variableName)
#define READ_FLOAT_DEV(variableName) _READ_FLOAT(variableName)
#define READ_FLOAT2_DEV(cfgVariableName,variableName) _READ_FLOAT2(cfgVariableName,variableName)
#define READ_DOUBLE_DEV(variableName) _READ_DOUBLE(variableName)
#define READ_STRING_DEV(variableName) _READ_STRING(variableName)
#define READ_CVECTOR_DEV(variableName) _READ_CVECTOR(variableName)
#define READ_ENUM_ASINT_DEV(type, variableName) _READ_ENUM_ASINT(type, variableName)
#define READ_STRINGVECTOR_DEV(variableName) _READ_STRINGVECTOR(variableName)
#else // !FINAL_VERSION
#define READ_BOOL_DEV(variableName)
#define READ_INT_DEV(variableName)
#define READ_FLOAT_DEV(variableName)
#define READ_FLOAT2_DEV(cfgVariableName,variableName)
#define READ_DOUBLE_DEV(variableName)
#define READ_STRING_DEV(variableName)
#define READ_CVECTOR_DEV(variableName)
#define READ_ENUM_ASINT_DEV(type, variableName)
#define READ_STRINGVECTOR_DEV(variableName)
#endif // !FINAL_VERSION

//-----------------------------------------------
// Macro for the dev & final version
//-----------------------------------------------

#define READ_BOOL_FV(variableName) _READ_BOOL(variableName)
#define READ_INT_FV(variableName) _READ_INT(variableName)
#define READ_FLOAT_FV(variableName) _READ_FLOAT(variableName)
#define READ_FLOAT2_FV(cfgVariableName,variableName) _READ_FLOAT2(cfgVariableName,variableName)
#define READ_DOUBLE_FV(variableName) _READ_DOUBLE(variableName)
#define READ_STRING_FV(variableName) _READ_STRING(variableName)
#define READ_CVECTOR_FV(variableName) _READ_CVECTOR(variableName)
#define READ_ENUM_ASINT_FV(type, variableName) _READ_ENUM_ASINT(type, variableName)
#define READ_STRINGVECTOR_FV(variableName) _READ_STRINGVECTOR(variableName)

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NL3D;


////////////
// GLOBAL //
////////////
CClientConfig LastClientCfg;
CClientConfig ClientCfg;
const string ConfigFileName = "client.cfg";


////////////
// EXTERN //
////////////
#ifndef RZ_NO_CLIENT
extern NL3D::UScene		*Scene;
extern NL3D::UDriver	*Driver;
extern CRyzomTime		RT;
extern string	Cookie;
extern string	FSAddr;
#endif

/////////////
// METHODS //
/////////////

// diplay only one time warning "Default values...."
static bool	DisplayCFGWarning= false;
static void	cfgWarning(const char *s)
{
	if(DisplayCFGWarning)
		nlwarning(s);
}

//---------------------------------------------------
// CClientConfig :
// Constructor.
//---------------------------------------------------
CClientConfig::CClientConfig()
{
	IsInvalidated		= false;

	TestBrowser			= false;
	Light				= false;					// Default is no client light version
	SaveConfig			= false;

	PositionX			= 0;
	PositionY			= 0;
	Frequency			= 60;

	SkipIntro			= false;
	SelectCharacter		= -1;						// Default is no auto select
	SelectedSlot		= 0;						// Default is slot 0

	Windowed			= false;					// Default is windowed mode.
	Width				= 800;						// Default Width for the window.
	Height				= 600;						// Default Height for the window.
	Depth				= 32;						// Default Bit per Pixel.
	Driver3D			= DrvAuto;					// Select best driver depending on hardware.
	Contrast			= 0.f;						// Default Monitor Contrast.
	Luminosity			= 0.f;						// Default Monitor Luminosity.
	Gamma				= 0.f;						// Default Monitor Gamma.

	VREnable			= false;
	VRDisplayDevice		= "Auto";
	VRDisplayDeviceId	= "";

	Local				= false;					// Default is Net Mode.
	FSHost				= "";						// Default Host.

#if 1 // Yubo hack
	// The order is important here, because in a layer, global texture are rendered through this order
	TexturesInterface.push_back("texture_interfaces_v3");
	// DXTC contain all items and bricks bitmaps, they must come after standard texture
	TexturesInterface.push_back("new_texture_interfaces_dxtc");
	// Added icons by Yubo's Team 2009
	TexturesInterface.push_back("texture_extra");
#else
	TexturesInterface.push_back("texture_interfaces_v3");
	TexturesInterfaceDXTC.push_back("texture_interfaces_dxtc");
#endif

	TexturesOutGameInterface.push_back("texture_interfaces_v3_outgame_ui");

	TexturesLoginInterface.push_back("texture_interfaces_v3_login");

	DisplayAccountButtons = true;
	CreateAccountURL	= "https://secure.ryzom.com/signup/from_client.php";
	ConditionsTermsURL	= "https://secure.ryzom.com/signup/terms_of_use.php";
	EditAccountURL		= "https://secure.ryzom.com/payment_profile/index.php";
	BetaAccountURL		= "http://www.ryzom.com/profile";
	ForgetPwdURL		= "https://secure.ryzom.com/payment_profile/lost_secure_password.php";
	FreeTrialURL		= "http://www.ryzom.com/join/?freetrial=1";
	LoginSupportURL		= "http://www.ryzom.com/en/support.html";
	Position			= CVector(0.f, 0.f, 0.f);	// Default Position.
	Heading				= CVector(0.f, 1.f, 0.f);	// Default Heading.
	EyesHeight			= 1.5f;						// Default User Eyes Height.
	Walk				= 1.66f;						// Default Velocity for the Walk.
	Run					= 6.0f;						// Default Velocity for the Run.
	Fly					= 25.0f;					// Default Velocity for the Fly.
	DmWalk				= 6.0f;						// Default Velocity for the Walk in Ring/DM or Ring/Editor.
	DmRun				= 20.0f;						// Default Velocity for the Run in Ring/DM or Ring/Editor.

	FlyAccel			= 1000.f;					// Default Acceleration for the fly, in m.s-2

	AllowDebugCommands	= false;					// Add debug commands at startup

	ForceDeltaTime		= 0;						// Default ForceDeltaTime, disabled by default

	HardwareCursor			= true;					// Default HardwareCursor
	HardwareCursorScale     = 0.85f;
	CursorSpeed				= 1.f;					// Default CursorSpeed
	CursorAcceleration		= 0;					// Default CursorAcceleration
	FreeLookSpeed			= 0.001f;				// Default FreeLookSpeed
	FreeLookAcceleration    = 0;					// Default FreeLookAcceleration
	FreeLookSmoothingPeriod = 0.f;                  // when in absolute mode, free look factor is used instead of speed, the mouse gives the absolute angle
	FreeLookInverted		= false;
	AutomaticCamera			= true;
	DblClickMode			= true;					// when in dbl click mode, a double click is needed to execute default contextual action
	AutoEquipTool			= true;					// when true player will auto-equip last used weapon or forage tool when doing an action

	BGColor				= CRGBA(100,100,255);		// Default Background Color.
	LandscapeTileNear	= 50.0f;					// Default Landscape Tile Near.
	LandscapeThreshold	= 1000.0f;					// Default Landscape Threshold.
	Vision				= 500.f;					// Player vision.
	Vision_min			= 200.f;					// Player vision min.
	Vision_max			= 800.f;					// Player vision max.
	SkinNbMaxPoly		= 40000;
	FxNbMaxPoly			= 10000;
	Cloud				= true;
	CloudQuality		= 160.0f;
	CloudUpdate			= 1;
	NbMaxSkeletonNotCLod= 20;
	CharacterFarClip	= 200.f;
	ScreenAspectRatio	= 0.f;						// Default commmon Screen Aspect Ratio (no relation with the resolution) - 0.f = auto
	FoV					= 75.f;						// Default value for the FoV.
	ForceDXTC			= false;					// Default is no DXTC Compression.
	AnisotropicFilter	= 0;						// Default is disabled (-1 = maximum value, 0 = disabled, 1+ = enabled)
	DivideTextureSizeBy2= false;					// Divide texture by 2
	DisableVtxProgram	= false;					// Disable Hardware Vertex Program.
	DisableVtxAGP		= false;					// Disable Hardware Vertex AGP.
	DisableTextureShdr	= false;					// Disable Hardware Texture Shader.
	MicroVeget			= true;						// Default is with MicroVeget.
	MicroVegetDensity	= 100.0f;
	HDEntityTexture		= false;
	HDTextureInstalled	= false;
	Fog					= true;						// Fog is on by default
	WaitVBL				= false;

	FXAA				= true;

	Bloom				= true;
	SquareBloom			= true;
	DensityBloom		= 255.f;

	GlobalWindPower		= 0.10f;					// Default is 0.25
	GlobalWindDirection	= CVector(1,0,0);			// Default direction is X>0

	MovieShooterMemory	= 0;						// MovieShooter disabled
	MovieShooterFramePeriod = 0.040f;				// default is 25 fps
	MovieShooterBlend	= false;
	MovieShooterFrameSkip = 0;						// default not skip frame
	MovieShooterPrefix = "shot_";

	CameraRecorderPrefix = "cam_rec";
	CameraRecorderBlend  = true;

	ScreenShotWidth		= 0;
	ScreenShotHeight	= 0;
	ScreenShotFullDetail = true;
	ScreenShotZBuffer	= false;

	MaxNumberOfTimedFXInstances = 20;
	SelectionFX = "sfx_selection_mouseover.ps";
	MouseOverFX = "sfx_selection_mouseover.ps";
	SelectionFXSize = 0.8f;

#if RZ_USE_PATCH
	PatchWanted = true;
#else
	PatchWanted = false;
#endif

	PatchUrl.clear();
	PatchletUrl.clear();
	PatchVersion.clear();
	PatchServer.clear();

	WebIgMainDomain = "atys.ryzom.com";
	WebIgTrustedDomains.push_back(WebIgMainDomain);

	RingReleaseNotePath = "http://"+WebIgMainDomain+"/releasenotes_ring/index.php";
	ReleaseNotePath = "http://"+WebIgMainDomain+"/releasenotes/index.php";


	///////////////
	// ANIMATION //
	// With a bigger angle, rotation is animated.
	AnimatedAngleThreshold	= 25.0;					//
	BlendFrameNumber		= 5;					//
	DestThreshold			= 0.01;					// Destination Threshold
	PositionLimiterRadius	= 0.1;
	SignificantDist			= 0.0001;				// Significant Distance
	///////////
	// SOUND //
	SoundOn				= true;						// Default is with sound.
	DriverSound			= SoundDrvAuto;
	SoundForceSoftwareBuffer = true;
	SoundOutGameMusic	= "Main Menu Loop.ogg";
	SoundSFXVolume		= 1.f;
	SoundGameMusicVolume	= 1.f;
	SoundTPFade			= 500;
	EnableBackgroundMusicTimeConstraint = true;

	SoundPackedSheetPath= "data/sound/";			// Default path for sound packed sheets
	SampleBankDir		= "data/sound/samplebanks";	// Default path for samples
	UserEntitySoundLevel = 0.5f;					// Default volume for sound in 1st person
	UseEax				= true;						// Default to use EAX;
	UseADPCM			= false;					// Defualt to PCM sample, NO ADPCM
	MaxTrack			= 32;						// DEfault to 32 track

	ColorShout			= CRGBA(150,0,0,255);		// Default Shout color.
	ColorTalk			= CRGBA(255,255,255,255);	// Default Talk color.

//	PreDataPath.push_back("data/gamedev/language/");	// Default Path for the language data

//	DataPath.push_back("data/");					// Default Path for the Data.
//	DataPath.push_back("data_leveldesign/");		// Default Path for the Level Design Directory.
//	DataPath.push_back("data_common/");				// Default Path for the Level Design Directory.

	DataPathNoRecurse.push_back("data_leveldesign/leveldesign/Game_Elem");

	UpdatePackedSheetPath.push_back("data_leveldesign");

	UpdatePackedSheet	= false;						// Update packed sheet if needed

	EndScreenTimeOut	= 0.f;						// Default time out for the screen at the end of the application.
	Loading_BG			= "loading_bg.tga";			// Default name for the loading background file.
	LoadingFreeTrial_BG	= "loading_free_trial_bg.tga";			// Default name for the loading background file in FreeTrial mode.
	Launch_BG			= "launcher_bg.tga";			// Default name for the launch background file.
	TeleportKami_BG		= "teleport_kami_bg.tga";
	TeleportKaravan_BG	= "teleport_caravan_bg.tga";
	Elevator_BG			= "elevator_bg.tga";		// Default name for the loading background file.
	ResurectKami_BG		= "resurect_kami_bg.tga";
	ResurectKaravan_BG	= "resurect_caravane_bg.tga";
	End_BG				= "end_bg.tga";				// Default name for the last background file.
	IntroNevrax_BG		= "launcher_nevrax.tga";
	IntroNVidia_BG		= "launcher_nvidia.tga";

	TipsY				= 0.07f;
	TeleportInfoY		= 0.23f;

	SceneName			= "";
	IdFilePath			= "sheet_id.bin";
	PacsPrimDir.push_back("data/3d/");

	Shadows				= true;						// Draw Shadows by default.
	ShadowsClipFar		= 10.f;						// Shadows are disabled after this distance.
	ShadowsLodDist		= 3.f;						// Shadows draw with just 1 part after this distance.
	ShadowZDirClampLandscape	= -0.5f;			// On landscape, allow a minimum Phi angle of -30 degrees
	ShadowZDirClampInterior		= -0.86f;			// On Interior, allow a minimum Phi angle of -60 degrees
	ShadowZDirClampSmoothSpeed	= 0.5f;
	ShadowMaxDepthLandscape		= 8.f;
	ShadowMaxDepthInterior		= 2.f;
	ShadowMaxDepthSmoothSpeed	= 6.f;


	Names				= false;					// Don't draw Names by default.

	Sleep				= -1;						// Default : client does not sleep.
	ProcessPriority		= 0;						// Default : NORMAL
	ShowPath			= false;					// Default : do not display the path.
	DrawBoxes			= false;					// Default : Do not draw the selection.

	UserSheet			= "fyros.race_stats";	// Default sheet used.
	Sex					= GSGENDER::male;			// Default : male.

	PrimitiveHeightAddition	= 2.f;					// Default : 2.f

	AttackDist			= 0.5f;
	RyzomDay			= 0;
	RyzomTime			= 0.0f;

	ManualWeatherSetup  = false;

	ChaseReactionTime	= 0.4f;						// Reaction time before chasing someone.

	TimeToAdjustCamera	= 1000.0;
	ChangeDirAngle		= 1.70f;				// Default Angle.

	GuildSymbolSize		= 0.3f;						// Default Guild Symbol Size.
	SelectionDist		= 150.f;					// Default dist in meter.
	SelectionOutBBoxWeight	= 16.f;					// Default factor
	LootDist			= 4.f;						// Default loot/harvest distance (in meter).
	SpaceSelectionDist	= 50.f;
	SpaceSelectionMaxCycle	= 1;

	ForceLanguage		= false;
	LanguageCode		= "en";						// Default to english
	DebugStringManager	= false;					// Default to no debug

	// TUNING
	WaterOffset			= -1.085f;
	FyrosWaterOffset	= -1.0f;
	MatisWaterOffset	= -1.08f;
	TrykerWaterOffset	= -0.88f;
	ZoraiWaterOffset	= -1.25f;
	WaterOffsetCreature = -1.6f;
	TimeToRemoveCol		= 1000;
	MoveToTimeToStopStall = 500;
	TPVMinPitch			= -1.5f;
	TPVMaxPitch			= 1.5f;
	MaxHeadTargetDist	= 30.0f;
	DeadFXName			= "misc_dead.ps";
	ImpactFXName		= "impact.ps";
	SkillUpFXName		= "misc_levelup.ps";
	MinDistFactor		= 0.5;
	NameScale			= 1.0f;						// Default Scale to display Names.
	NamePos				= 0.02f;
	NameFontSize		= 20;
	MaxNameDist			= 50.0f;
	ConstNameSizeDist	= 20.0f;
	StaticNameHeight	= true;
	BarsHeight			= 0.002f;
	BarsWidth			= 0.05f;
	FightAreaSize		= 1.5;
	BubbleZBias			= -3.f;
	ForageInterfaceZBias= -10.f;
	FyrosScale			= 1.0f;
	MatisScale			= 1.08f;
	TrykerScale			= 0.88f;
	ZoraiScale			= 1.25f;
	EnableRacialAnimation = true;

	// OPTIONS
	RunAtTheBeginning	= true;
	SelectWithRClick	= false;					// Default right click cannot select.
	RotKeySpeedMax		= 3.0f;
	RotKeySpeedMin		= 1.0f;
	RotAccel			= 0.001f;
	PutBackItems		= false;
	ShowNameUnderCursor	= true;
	ShowNameSelected	= true;
	ShowNameBelowDistanceSqr	= 20.f * 20.f;
	ForceIndoorFPV		= false;
	FollowOnAtk			= true;
	AtkOnSelect			= false;
	TransparentUnderCursor = false;

	// PREFERENCES
	FPV					= false;
	CameraHeight		= 2.5f;
	CameraDistance		= 3.0f;
	CameraDistStep		= 1.0f;
	CameraDistMin		= 1.0f;
	CameraDistMax		= 100.0f;
	DmCameraDistMax		= 25.0f;
	CameraAccel			= 0.2f;
	CameraSpeedMin		= 0.2f;
	CameraSpeedMax		= 1.0f;
	CameraResetSpeed	= 2.0f;

	// VERBOSES
	VerboseVP				= false;
	VerboseAnimUser			= false;
	VerboseAnimSelection	= false;
	VerboseAllTraffic		= false;

	// DEBUG
	DisplayMissingAnimFile = false;
	DefaultEntity = "ccafb1.creature";
	RestrainPI = true;
	DumpVSIndex = false;
	HelpFontSize  = 12;
	HelpFontColor = CRGBA(255,255,255);
	HelpLineStep  = 0.025f;
	DebugFontSize  = 16;
	DebugFontColor = CRGBA(250, 250, 250);
	DebugLineStep  = 0.02f;
	HeadOffset     = CVector(0.f, 0.f, 0.f);
	Check          = false;
	BlendForward   = true;
	//
	FPExceptions			= false;				// Disable Floating Point Exceptions.
	DisplayWeapons			= false;				// Do not dusplay weapons in first person.
	NeedComputeVS			= false;				// Do not need to compute Visual Slots.
	//
	GroundFXMaxDist         = 40.f;
	GroundFXMaxNB           = 10;
	GroundFXCacheSize       = 10;
	//
	AutoReloadFiles = false;
	BlendShapePatched = true;
	ExtendedCommands = false;

	WaterEnvMapUpdateTime = 1.f;

	NumFrameForProfile = 0;
	SimulateServerTick = false;
	TimerMode= 0;

	KlientChatPort= "";

	PreCacheShapes = true;
	ResetShapeBankOnRetCharSelect = false;
	LandscapeEnabled = true;
	VillagesEnabled = true;
	EAMEnabled = true;
	CacheUIParsing =  false;
	MicroLifeEnabled = true;

	SkipEULA = false;

	StageLCTUsage= StageUsePosOnlyLCT;

	SimulatePacketLossRatio= 0;

	CheckMemoryEveryNFrame= -1;
	LogMemoryAllocation=false;
	LogMemoryAllocationSize=8;

	DamageShieldEnabled = false;

	LevelDesignEnabled = false;

	R2Mode = true;
	R2EDEnabled = false;
	R2EDExtendedDebug = false;
	R2EDVerboseParseTime = false;
	R2EDDontReparseUnchangedUIFiles = false;
	R2EDLightPalette = false;
	R2EDAutoSaveWait = 60*5;
	R2EDAutoSaveSlot = 9;
	R2EDMustVerifyRingAccessWhileLoadingAnimation = false;
	R2EDUseVerboseRingAccess = false;
	R2EDDssNetwork = 3;

	DamageShieldEnabled = false;

	AllowDebugLua = false;
	LoadLuaDebugger = false;
	DisplayLuaDebugInfo = false;
	BeepWhenLaunched = false;



	LoadingStringCount = 0;

	LuaDebugInfoGotoButtonEnabled = false;

	FogDistAndDepthLookupBias = 0.f;

	R2EDLoadDynamicFeatures	= 0;

	CheckR2ScenarioMD5 = true;

	DisplayTPReason = false;

	//TPCancelButtonX = 988;
	//TPCancelButtonY = 138;
	//TPQuitButtonX = 8;
	//TPQuitButtonY = 138;
	TPButtonW = 32;
	TPButtonH = 32;

	//SAVE
	ScenarioSavePath = "./my_scenarios/";

	R2EDClippedEntityBlendTime = 0.18f;

	BackgroundDownloader = false;

}// CClientConfig //


//---------------------------------------------------
// load :
// Load the client config file.
//---------------------------------------------------
void CClientConfig::setValuesOnFileChange()
{
	//	ClientCfg.ConfigFile.print (InfoLog);

	// display an info only when the file change
	nlinfo ("reloading the config file!");

	setValues();
}

//---------------------------------------------------
// load :
// Load the client config file.
//---------------------------------------------------
void CClientConfig::setValues()
{
	CConfigFile::CVar *varPtr = 0;
	static bool	firstTimeSetValues= true;

	//////////////////////
	// INTERFACE CONFIG //
	// input files
	READ_STRING_FV(XMLInputFile)


	READ_BOOL_FV(SkipIntro);
	READ_BOOL_DEV(SkipEULA);
	READ_INT_DEV(SelectCharacter);

	READ_INT_FV(SelectedSlot);
	if( ClientCfg.SelectedSlot>4 )
	{
		ClientCfg.SelectedSlot = 0;
	}

	// interface textures login menus
	READ_STRINGVECTOR_FV(TexturesLoginInterface);
	READ_STRINGVECTOR_FV(TexturesLoginInterfaceDXTC);

	// interface textures outgame menus
	READ_STRINGVECTOR_FV(TexturesOutGameInterface);
	READ_STRINGVECTOR_FV(TexturesOutGameInterfaceDXTC);

	// interface textures ingame and r2
	READ_STRINGVECTOR_FV(TexturesInterface);
	READ_STRINGVECTOR_FV(TexturesInterfaceDXTC);

	// interface files login menus
	READ_STRINGVECTOR_FV(XMLLoginInterfaceFiles);

	// interface files outgame menus
	READ_STRINGVECTOR_FV(XMLOutGameInterfaceFiles);

	// interface files
	READ_STRINGVECTOR_FV(XMLInterfaceFiles);

	// r2ed interfaces
	READ_STRINGVECTOR_FV(XMLR2EDInterfaceFiles);

	// logos
	READ_STRINGVECTOR_FV(Logos);

	// browser test mode
	READ_BOOL_DEV(TestBrowser);
	READ_STRING_DEV(TestBrowserUrl);

	// ClientLight
#if !FINAL_VERSION
	varPtr = ClientCfg.ConfigFile.getVarPtr ("ClientLight");
	if (varPtr)
		ClientCfg.Light = varPtr->asInt() ? true : false;
#endif // !FINAL_VERSION


	READ_BOOL_DEV(LandscapeEnabled)
	READ_BOOL_DEV(VillagesEnabled)
	READ_BOOL_DEV(EAMEnabled)
	READ_BOOL_DEV(CacheUIParsing)
	READ_BOOL_DEV(MicroLifeEnabled)

	READ_BOOL_DEV(LevelDesignEnabled)

	///////////////////
	// WINDOW CONFIG //
	// Mode.

	// SaveConfig
	READ_BOOL_FV(SaveConfig)

	// Window Positon
	READ_INT_FV(PositionX)
	READ_INT_FV(PositionY)

	// Window frequency
	READ_INT_FV(Frequency)

	CConfigFile::CVar *pcvFullScreen = ClientCfg.ConfigFile.getVarPtr("FullScreen");
	if( pcvFullScreen )
	{
		ClientCfg.Windowed = pcvFullScreen->asInt() ? false : true;
	}
	else
		cfgWarning("Default value used for 'Fullscreen'");
	// Width
	READ_INT_FV(Width)
	// Height
	READ_INT_FV(Height)
	// Depth : Bit Per Pixel
	READ_INT_FV(Depth)
	// Contrast
	READ_FLOAT_FV(Contrast)
	// Luminosity
	READ_FLOAT_FV(Luminosity)
	// Gamma
	READ_FLOAT_FV(Gamma)
	// 3D Driver
	varPtr = ClientCfg.ConfigFile.getVarPtr ("Driver3D");
	if (varPtr)
	{
		if (nlstricmp(varPtr->asString(), "Auto") == 0 || nlstricmp(varPtr->asString(), "0") == 0) ClientCfg.Driver3D = CClientConfig::DrvAuto;
		else if (nlstricmp(varPtr->asString(), "OpenGL") == 0 || nlstricmp(varPtr->asString(), "1") == 0) ClientCfg.Driver3D = CClientConfig::OpenGL;
		else if (nlstricmp(varPtr->asString(), "Direct3D") == 0 || nlstricmp(varPtr->asString(), "2") == 0) ClientCfg.Driver3D = CClientConfig::Direct3D;
		else if (nlstricmp(varPtr->asString(), "OpenGLES") == 0 || nlstricmp(varPtr->asString(), "3") == 0) ClientCfg.Driver3D = CClientConfig::OpenGLES;
	}
	else
		cfgWarning ("Default value used for 'Driver3D' !!!");

	READ_BOOL_FV(VREnable)
	READ_STRING_FV(VRDisplayDevice)
	READ_STRING_FV(VRDisplayDeviceId)

	////////////
	// INPUTS //
	READ_BOOL_FV(HardwareCursor)
	READ_FLOAT_FV(HardwareCursorScale)
	READ_FLOAT_FV(CursorSpeed)
	READ_INT_FV(CursorAcceleration)
	READ_FLOAT_FV(FreeLookSpeed)
	READ_INT_FV(FreeLookAcceleration)
	READ_FLOAT_FV(FreeLookSmoothingPeriod)
	READ_BOOL_FV(FreeLookInverted)
	READ_BOOL_FV(AutomaticCamera)
	READ_BOOL_FV(DblClickMode)
	READ_BOOL_FV(AutoEquipTool)

	/////////
	// NET //
#if !FINAL_VERSION
	// Local : local mode or network mode
	READ_BOOL_DEV(Local)
#endif // FINAL_VERSION
	// FSHost
	READ_STRING_FV(FSHost)

	READ_BOOL_DEV(DisplayAccountButtons)
	READ_STRING_DEV(CreateAccountURL)
	READ_STRING_DEV(EditAccountURL)
	READ_STRING_DEV(ConditionsTermsURL)
	READ_STRING_DEV(BetaAccountURL)
	READ_STRING_DEV(ForgetPwdURL)
	READ_STRING_DEV(FreeTrialURL)
	READ_STRING_DEV(LoginSupportURL)

#ifndef RZ_NO_CLIENT
	// if cookie is not empty, it means that the client was launch
	// by the nel_launcher, so it can't be local
	if(!Cookie.empty())
	{
		nlassert (!FSAddr.empty());
		ClientCfg.Local = false;
	}
	else
	{
		nlassert (FSAddr.empty());
	}
#endif

	/////////////////
	// USER ENTITY //
	READ_CVECTOR_DEV(Position)
	READ_CVECTOR_DEV(Heading)
	// EyesHeight
	READ_FLOAT_DEV(EyesHeight)

	// Walk
	READ_FLOAT_DEV(Walk)
	// Run
	READ_FLOAT_DEV(Run)

	//When editing or Dm ing a session in ring mode the player move quicker
	// DM Walk
	READ_FLOAT_DEV(DmWalk)
	// DM Run
	READ_FLOAT_DEV(DmRun)



	// Fly
	READ_FLOAT_DEV(Fly)
	READ_FLOAT_DEV(FlyAccel)

	READ_BOOL_FV(AllowDebugCommands)

	// ForceDeltaTime
	READ_INT_DEV(ForceDeltaTime)

	////////////
	// RENDER //
	// Background Color
#if !FINAL_VERSION
	CConfigFile::CVar *pcvBackColor = ClientCfg.ConfigFile.getVarPtr("Background");
	if( pcvBackColor && (pcvBackColor->size()==3) )
	{
		ClientCfg.BGColor.R = pcvBackColor->asInt(0);
		ClientCfg.BGColor.G = pcvBackColor->asInt(1);
		ClientCfg.BGColor.B = pcvBackColor->asInt(2);
		ClientCfg.BGColor.A = 255;
	}
	else
		cfgWarning("Default value used for 'Background'");
#endif // !FINAL_VERSION
	// LandscapeTileNear
	READ_FLOAT_FV(LandscapeTileNear)
	// LandscapeThreshold
	READ_FLOAT_FV(LandscapeThreshold)
	// to be backward compatible, suppose a value<1 is for the old System!!! => invert it!
	if( ClientCfg.LandscapeThreshold<1)
	{
		ClientCfg.LandscapeThreshold= 1.f/ ClientCfg.LandscapeThreshold;
		// must re-write in the CFG database, to be ok with the in-game configurator
		varPtr = ClientCfg.ConfigFile.getVarPtr("LandscapeThreshold");
		if(varPtr)
			varPtr->forceAsDouble(ClientCfg.LandscapeThreshold);
	}
	// Vision
	READ_FLOAT_FV(Vision)
	READ_FLOAT_FV(Vision_min)
	READ_FLOAT_FV(Vision_max)
	// SkinNbMaxPoly
	READ_INT_FV(SkinNbMaxPoly)
	// FxNbMaxPoly
	READ_INT_FV(FxNbMaxPoly)
	READ_BOOL_FV(Cloud)
	READ_FLOAT_FV(CloudQuality)
	READ_INT_FV(CloudUpdate)
	// NbMaxSkeletonNotCLod
	READ_INT_FV(NbMaxSkeletonNotCLod)
	// CharacterFarClip
	READ_FLOAT_FV(CharacterFarClip)

	// Bloom
	READ_BOOL_FV(Bloom)
	READ_BOOL_FV(SquareBloom)
	READ_FLOAT_FV(DensityBloom)

	// FXAA
	READ_BOOL_FV(FXAA)

	// ScreenAspectRatio.
	READ_FLOAT_FV(ScreenAspectRatio)
	// FoV.
	READ_FLOAT_FV(FoV)
	// ForceDXTC
	READ_BOOL_FV(ForceDXTC)
	// AnisotropicFilter
	READ_INT_FV(AnisotropicFilter)
	// DivideTextureSizeBy2
	READ_BOOL_FV(DivideTextureSizeBy2)
	// DisableVtxProgram
	READ_BOOL_FV(DisableVtxProgram)
	// DisableVtxAGP
	READ_BOOL_FV(DisableVtxAGP)
	// DisableTextureShdr
	READ_BOOL_FV(DisableTextureShdr)
	// MicroVeget
	READ_BOOL_FV(MicroVeget)
	// MicroVeget Density
	READ_FLOAT_FV(MicroVegetDensity)
	// GlobalWindPower: Global Wind Power
	READ_FLOAT_DEV(GlobalWindPower)
	// Global Wind Direction
	READ_CVECTOR_DEV(GlobalWindDirection)
	// HDEntityTexture
	READ_BOOL_FV(HDEntityTexture)
	// HDTextureInstalled
	READ_BOOL_FV(HDTextureInstalled)

	// Fog
	READ_BOOL_DEV(Fog)

	// WaitVBL
	READ_BOOL_FV(WaitVBL)

	READ_INT_DEV(TimerMode)

	// MovieShooterMemory
	CConfigFile::CVar *pcv = ClientCfg.ConfigFile.getVarPtr("MovieShooterMemory");
	if( pcv )
	{
		ClientCfg.MovieShooterMemory = pcv->asInt();
		// Transform to octet
		ClientCfg.MovieShooterMemory *= 1024*1024;
	}
	else
		cfgWarning("'MovieShooterMemory' not found => MovieShooter Disabled");
	// MovieShooterPath
	READ_STRING_FV(MovieShooterPath)
	// MovieShooterFramePeriod
	READ_FLOAT_FV(MovieShooterFramePeriod)
	// MovieShooterBlend
	READ_BOOL_FV(MovieShooterBlend)
	// MovieShooterFrameSkip
	READ_INT_FV(MovieShooterFrameSkip)

	// Camera Recorder
	READ_STRING_FV(CameraRecorderPath)
	READ_STRING_FV(CameraRecorderPrefix)
	READ_BOOL_FV(CameraRecorderBlend)

	// Screenshot
	READ_INT_FV(ScreenShotWidth)
	READ_INT_FV(ScreenShotHeight)
	READ_BOOL_FV(ScreenShotFullDetail)
	READ_BOOL_FV(ScreenShotZBuffer)

	/////////////////////////
	// NEW PATCHING SYSTEM //
	READ_BOOL_FV(PatchWanted)
	READ_STRING_DEV(PatchUrl)
	READ_STRING_DEV(PatchVersion)
	READ_STRING_DEV(RingReleaseNotePath)
	READ_STRING_DEV(ReleaseNotePath)
	READ_STRING_FV(PatchServer)

	/////////////////////////
	// NEW PATCHLET SYSTEM //
	READ_STRING_FV(PatchletUrl)

	///////////
	// WEBIG //
	READ_STRING_FV(WebIgMainDomain);
	READ_STRINGVECTOR_FV(WebIgTrustedDomains);

	///////////////
	// ANIMATION //
	// AnimatedAngleThreshold
	READ_DOUBLE_DEV(AnimatedAngleThreshold)
	// BlendFrameNumber
	READ_INT_DEV(BlendFrameNumber)
	// DestThreshold
	READ_DOUBLE_DEV(DestThreshold)
	// PositionLimiterRadius
	READ_DOUBLE_DEV(PositionLimiterRadius)
	// SignificantDist
	READ_DOUBLE_DEV(SignificantDist)
	// Stage LCT usage
	READ_ENUM_ASINT_DEV(TStageLCTUsage, StageLCTUsage)

	////////////
	// TUNING //
	// Water Offset
	READ_FLOAT_DEV(WaterOffset)

#if !FINAL_VERSION
	READ_FLOAT_DEV(FyrosWaterOffset)
	READ_FLOAT_DEV(MatisWaterOffset)
	READ_FLOAT_DEV(TrykerWaterOffset)
	READ_FLOAT_DEV(ZoraiWaterOffset)
#endif // FINAL_VERSION

	// Water Offset for creature
	READ_FLOAT_DEV(WaterOffsetCreature)
	// TimeToRemoveCol
	READ_INT_DEV(TimeToRemoveCol)
	// MoveToTimeToStopStall
	READ_INT_DEV(MoveToTimeToStopStall)
	// TimeToAdjustCamera
	READ_DOUBLE_DEV(TimeToAdjustCamera)
	// ChangeDirAngle
	READ_DOUBLE_DEV(ChangeDirAngle)
	// GuildSymbolSize
	READ_FLOAT_DEV(GuildSymbolSize)
	// SelectionDist
	READ_FLOAT_DEV(SelectionDist)
	// SelectionOutBBoxWeight
	READ_FLOAT_DEV(SelectionOutBBoxWeight)
	// LootDist
	READ_FLOAT_DEV(LootDist)
	// SpaceSelectionDist
	READ_FLOAT_DEV(SpaceSelectionDist)
	// SpaceSelectionMaxCycle
	READ_INT_DEV(SpaceSelectionMaxCycle)
	// Third Person View Min Pitch.
	READ_FLOAT_DEV(TPVMinPitch)
	// Third Person View Max Pitch.
	READ_FLOAT_DEV(TPVMaxPitch)
	// The character look at the target before this distance.
	READ_FLOAT_DEV(MaxHeadTargetDist)
	// FX played when dead
	READ_STRING_DEV(DeadFXName)
	// FX played for each impact
	READ_STRING_DEV(ImpactFXName)
	// FX Played at skill up
	READ_STRING_DEV(SkillUpFXName)
	// MinDistFactor
	READ_DOUBLE_DEV(MinDistFactor)
	// NameScale
	READ_FLOAT_DEV(NameScale)
	// NamePos
	READ_FLOAT_DEV(NamePos)
	// NameFontSize
	READ_INT_DEV(NameFontSize)
	// MaxNameDist
	READ_FLOAT_DEV(MaxNameDist)
	// ConstNameSizeDist
	READ_FLOAT_DEV(ConstNameSizeDist)
	// StaticNameHeight
	READ_BOOL_FV(StaticNameHeight)
	// BarsHeight
	READ_FLOAT_DEV(BarsHeight)
	// BarsWidth
	READ_FLOAT_DEV(BarsWidth)
	// DisplayWeapons
	READ_BOOL_FV(DisplayWeapons)
	// FightAreaSize
	READ_DOUBLE_DEV(FightAreaSize)
	// AttackDist
	READ_FLOAT_DEV(AttackDist)
	// BubbleZBias
	READ_FLOAT_DEV(BubbleZBias);
	// ForageInterfaceZBias
	READ_FLOAT_DEV(ForageInterfaceZBias);

	// EnableRacialAnimation
	READ_BOOL_FV(EnableRacialAnimation);

#if !FINAL_VERSION
	READ_FLOAT_DEV(FyrosScale);
	READ_FLOAT_DEV(MatisScale);
	READ_FLOAT_DEV(TrykerScale);
	READ_FLOAT_DEV(ZoraiScale);

#endif // FINAL_VERSION

	//////////////////
	// SOUND CONFIG //
	// SoundOn
	READ_BOOL_FV(SoundOn)
	// Sound Driver
	varPtr = ClientCfg.ConfigFile.getVarPtr ("DriverSound");
	if (varPtr)
	{
		if (nlstricmp(varPtr->asString(), "Auto") == 0) ClientCfg.DriverSound = CClientConfig::SoundDrvAuto;
		else if (nlstricmp(varPtr->asString(), "FMod") == 0) ClientCfg.DriverSound = CClientConfig::SoundDrvFMod;
		else if (nlstricmp(varPtr->asString(), "OpenAL") == 0) ClientCfg.DriverSound = CClientConfig::SoundDrvOpenAL;
		else if (nlstricmp(varPtr->asString(), "DirectSound") == 0) ClientCfg.DriverSound = CClientConfig::SoundDrvDirectSound;
		else if (nlstricmp(varPtr->asString(), "XAudio2") == 0) ClientCfg.DriverSound = CClientConfig::SoundDrvXAudio2;
	}
	else
		cfgWarning ("Default value used for 'DriverSound' !!!");
	// SoundForceSoftwareBuffer
	READ_BOOL_FV(SoundForceSoftwareBuffer);
	// SoundOutGameMusic
	READ_STRING_DEV(SoundOutGameMusic)
	// SoundSFXVolume
	READ_FLOAT_FV(SoundSFXVolume);
	// SoundGameMusicVolume
	READ_FLOAT_FV(SoundGameMusicVolume);
	// SoundTPFade
	READ_INT_DEV(SoundTPFade);
	// EnableBackgroundMusicTimeConstraint
	READ_BOOL_DEV(EnableBackgroundMusicTimeConstraint);
	// SoundPackedSheetPath
	READ_STRING_DEV(SoundPackedSheetPath)
	// SampleBankDir
	READ_STRING_DEV(SampleBankDir)
	// UserEntitySoundLevel : UserEntity sound level
	READ_FLOAT_FV(UserEntitySoundLevel)
	// Use EAX
	READ_BOOL_FV(UseEax)
	// UseADPCM
	READ_BOOL_DEV(UseADPCM)
	// Max track
	READ_INT_FV(MaxTrack)

	/////////////////
	// USER COLORS //
	// Shout Color
	CConfigFile::CVar *pcvColorShout = ClientCfg.ConfigFile.getVarPtr("ColorShout");
	if( pcvColorShout && (pcvColorShout->size() == 3) )
	{
		ClientCfg.ColorShout.R = pcvColorShout->asInt(0);
		ClientCfg.ColorShout.G = pcvColorShout->asInt(1);
		ClientCfg.ColorShout.B = pcvColorShout->asInt(2);
		ClientCfg.ColorShout.A = 255;
	}
	else
		cfgWarning("Default value used for 'ColorShout'");

	// Talk Color
	CConfigFile::CVar *pcvColorTalk = ClientCfg.ConfigFile.getVarPtr("ColorTalk");
	if( pcvColorTalk && (pcvColorTalk->size() == 3) )
	{
		ClientCfg.ColorTalk.R = pcvColorTalk->asInt(0);
		ClientCfg.ColorTalk.G = pcvColorTalk->asInt(1);
		ClientCfg.ColorTalk.B = pcvColorTalk->asInt(2);
		ClientCfg.ColorTalk.A = 255;
	}
	else
		cfgWarning("Default value used for 'ColorTalk'");

	//////////
	// MISC //
	// Pre Data Path.
	READ_STRINGVECTOR_FV(PreDataPath);

	// Data Path.
	READ_STRINGVECTOR_FV(DataPath);

	// List of files that trigger R2ED reload when touched
	READ_STRINGVECTOR_FV(R2EDReloadFiles);

	// Data Path no recurse.
	READ_STRINGVECTOR_FV(DataPathNoRecurse);

	// Update packed sheet Path
	READ_STRINGVECTOR_FV(UpdatePackedSheetPath);

	// UpdatePackedSheet
	READ_BOOL_DEV(UpdatePackedSheet)

	// EndScreenTimeOut
	READ_FLOAT_DEV(EndScreenTimeOut)
	// Backgrounds
	READ_STRING_FV(Loading_BG)
	READ_STRING_FV(LoadingFreeTrial_BG)
	READ_STRING_FV(Launch_BG)
	READ_STRING_FV(TeleportKami_BG)
	READ_STRING_FV(TeleportKaravan_BG)
	READ_STRING_FV(Elevator_BG)
	READ_STRING_FV(ResurectKami_BG)
	READ_STRING_FV(ResurectKaravan_BG)
	READ_STRING_FV(End_BG)
	READ_STRING_FV(IntroNevrax_BG)
	READ_STRING_FV(IntroNVidia_BG)

	READ_FLOAT_DEV(TipsY)
	READ_FLOAT_DEV(TeleportInfoY)
	// SceneName
	READ_STRING_DEV(SceneName)
	// IdFile Path
	READ_STRING_DEV(IdFilePath)

	// PacsPrimDir
	READ_STRINGVECTOR_DEV(PacsPrimDir);

	/////////////
	// FILTERS //
	createDebug ();
	CConfigFile::CVar *pcvTmp = ClientCfg.ConfigFile.getVarPtr("NegFiltersDebug");
	if( pcvTmp )
	{
		int iSz = pcvTmp->size();
		for(int k = 0; k < iSz; ++k)
		{
			DebugLog->addNegativeFilter (pcvTmp->asString(k).c_str());
		}
	}
	else
		cfgWarning("Default value used for 'NegFiltersDebug'");

	pcvTmp = ClientCfg.ConfigFile.getVarPtr("NegFiltersInfo");
	if( pcvTmp )
	{
		int iSz = pcvTmp->size();
		for(int k = 0; k < iSz; ++k)
		{
			InfoLog->addNegativeFilter (pcvTmp->asString(k).c_str());
		}
	}
	else
		cfgWarning("Default value used for 'NegFiltersInfo'");

	pcvTmp = ClientCfg.ConfigFile.getVarPtr("NegFiltersWarning");
	if( pcvTmp )
	{
		int iSz = pcvTmp->size();
		for(int k = 0; k < iSz; ++k)
		{
			WarningLog->addNegativeFilter (pcvTmp->asString(k).c_str());
		}
	}
	else
		cfgWarning("Default value used for 'NegFiltersWarning'");

	// Script Files
	READ_STRINGVECTOR_FV(StartCommands);

	/////////////
	// OPTIONS //
	// Colors for system infos
	ClientCfg.SystemInfoParams.clear();
	CConfigFile::CVar *sic = ClientCfg.ConfigFile.getVarPtr("SystemInfoColors");
	if (!sic)
	{
		cfgWarning("Can't read SystemInfoColors, all colors defaulting to white");
	}
	else
	{
		if (sic->size() & 1)
		{
			cfgWarning("Expecting odd size for SystemInfoColors. Last entry ignored.");
		}
		uint numCol = sic->size() >> 1;
		for(uint k = 0; k < numCol; ++k)
		{
			uint r, g, b, a;
			char mode[64];
			char fx[64];
			fx[0]='\0';
			if (sscanf(sic->asString((2 * k) + 1).c_str(), "%d %d %d %d %s %s", &r, &g, &b, &a, mode, fx) < 5)
			{
				if(DisplayCFGWarning)
					nlwarning("Can't parse color for entry %s", sic->asString(2 * k).c_str());
			}
			else
			{
				SSysInfoParam p;

				p.Color = CRGBA(r, g, b, a);
				p.Mode = SSysInfoParam::Normal;
				p.SysInfoFxName = string(fx);
				if (stricmp(mode, "over") == 0)	p.Mode = SSysInfoParam::Over;
				else if (stricmp(mode, "overonly") == 0) p.Mode = SSysInfoParam::OverOnly;
				else if (stricmp(mode, "center") == 0)	p.Mode = SSysInfoParam::Center;
				else if (stricmp(mode, "centeraround") == 0)	p.Mode = SSysInfoParam::CenterAround;
				else if (stricmp(mode, "around") == 0)	p.Mode = SSysInfoParam::Around;

				ClientCfg.SystemInfoParams[toLower(sic->asString(2 * k))] = p;
			}
		}
	}

#ifndef RZ_NO_CLIENT
	// printf commands in loading screens
	ClientCfg.PrintfCommands.clear();
	ClientCfg.PrintfCommandsFreeTrial.clear();
	std::vector< std::string > printfCommands(2);
	printfCommands[0] = "PrintfCommands";
	printfCommands[1] = "PrintfCommandsFreeTrial";
	for(uint p=0; p<2; p++)
	{
		CConfigFile::CVar *pc = ClientCfg.ConfigFile.getVarPtr(printfCommands[p].c_str());
		if (pc)
		{
			if( pc->size()%5 == 0 && pc->size() >= 5)
			{
				for (uint i = 0; i < pc->size(); i+=5)
				{
					SPrintfCommand pcom;
					pcom.X = pc->asInt(i);
					pcom.Y = pc->asInt(i+1);
					pcom.Color = CRGBA::stringToRGBA( pc->asString(i+2).c_str() );
					pcom.FontSize = pc->asInt(i+3);
					pcom.Text = pc->asString(i+4);

					if(p==0) ClientCfg.PrintfCommands.push_back( pcom );
					else ClientCfg.PrintfCommandsFreeTrial.push_back( pcom );
				}
			}
			else
			{
				cfgWarning(("Missing or too many parameters in " + printfCommands[p]).c_str());
			}
		}
	}
#endif

	READ_INT_FV(LoadingStringCount)

	// DEBUG MEMORY
	READ_INT_DEV(CheckMemoryEveryNFrame)
	READ_BOOL_DEV(LogMemoryAllocation)
	READ_INT_DEV(LogMemoryAllocationSize)

	// SelectWithRClick
	READ_BOOL_FV(SelectWithRClick)
	READ_BOOL_DEV(RunAtTheBeginning)
	READ_FLOAT_FV(RotKeySpeedMax)
	READ_FLOAT_FV(RotKeySpeedMin)
	READ_FLOAT_FV(RotAccel)
	READ_BOOL_DEV(PutBackItems)
	READ_BOOL_DEV(ShowNameUnderCursor)
	READ_BOOL_DEV(ShowNameSelected)
	READ_FLOAT_DEV(ShowNameBelowDistanceSqr)
	READ_BOOL_DEV(ForceIndoorFPV)
	READ_BOOL_FV(FollowOnAtk);
	READ_BOOL_FV(AtkOnSelect);
	READ_BOOL_DEV(TransparentUnderCursor);


	/////////////////
	// PREFERENCES //
	// Read the view mode at load time only, cause prefer keep ingame player setup
	if(firstTimeSetValues)
	{
		READ_BOOL_FV(FPV)
	}
	READ_FLOAT_FV(CameraDistStep)
	READ_FLOAT_FV(CameraDistMin)
	READ_FLOAT_FV(CameraDistMax)
	READ_FLOAT_FV(DmCameraDistMax)
	READ_FLOAT_FV(CameraAccel)
	READ_FLOAT_FV(CameraSpeedMin)
	READ_FLOAT_FV(CameraSpeedMax)
	READ_FLOAT_FV(CameraResetSpeed)
	// Read the camera height and distance at load time only, cause prefer keep ingame player setup
	if(firstTimeSetValues)
	{
		READ_FLOAT_FV(CameraHeight)
		READ_FLOAT_FV(CameraDistance)
	}


	/////////////
	// SHADOWS //
	// Shadows : Get Shadows state
	READ_BOOL_FV(Shadows)
	// ShadowsClipFar : Get Shadows Clip Far.
	READ_FLOAT_DEV(ShadowsClipFar)
	// ShadowsLodDist : Get Shadows Lod Distance.
	READ_FLOAT_DEV(ShadowsLodDist)
	// ZClamp
	READ_FLOAT_DEV(ShadowZDirClampLandscape);
	READ_FLOAT_DEV(ShadowZDirClampInterior);
	READ_FLOAT_DEV(ShadowZDirClampSmoothSpeed);
	// MaxDepth
	READ_FLOAT_DEV(ShadowMaxDepthLandscape);
	READ_FLOAT_DEV(ShadowMaxDepthInterior);
	READ_FLOAT_DEV(ShadowMaxDepthSmoothSpeed);


	////////////////
	// GROUND FXS //
	////////////////
	READ_FLOAT_DEV(GroundFXMaxDist)
	READ_INT_DEV(GroundFXMaxNB)
	READ_INT_DEV(GroundFXCacheSize)

	// Names : Get Names state
	READ_BOOL_FV(Names)
	// Sleep
	READ_INT_FV(Sleep)
	// ProcessPriority
	READ_INT_FV(ProcessPriority)
	// ShowPath : Get the ShowPath value.
	READ_BOOL_DEV(ShowPath)
	// UserSheet : Get the sheet to used for the use rin Local mode.
	READ_STRING_DEV(UserSheet)
#if !FINAL_VERSION
	// Sex
	varPtr = ClientCfg.ConfigFile.getVarPtr("Sex");
	if(varPtr)
		ClientCfg.Sex = (GSGENDER::EGender)varPtr->asInt();
	else
		cfgWarning("Default value used for 'Sex' !!!");
#endif // FINAL_VERSION
	// PrimitiveHeightAddition
	READ_BOOL_DEV(PrimitiveHeightAddition)
	// DrawBoxes
	READ_BOOL_DEV(DrawBoxes)


	// ChaseReactionTime
	READ_FLOAT_DEV(ChaseReactionTime)
	// RyzomTime
	READ_FLOAT_DEV(RyzomTime)
	// RyzomDay
	READ_INT_DEV(RyzomDay)
	// ManualWeatherSetup
	READ_BOOL_DEV(ManualWeatherSetup)
	// LanguageCode
	READ_BOOL_DEV(ForceLanguage)
	if (!ClientCfg.ForceLanguage)
	{
		READ_STRING_FV(LanguageCode)
	}
	// DebugStringManager
	READ_BOOL_DEV(DebugStringManager)

	// LastLogin
	READ_STRING_FV(LastLogin)

	//////////////
	// VERBOSES //
	READ_BOOL_DEV(VerboseVP)
	READ_BOOL_DEV(VerboseAnimUser)
	READ_BOOL_DEV(VerboseAnimSelection)
	READ_BOOL_DEV(VerboseAllTraffic)

	READ_STRING_DEV(LigoPrimitiveClass);

	///////////
	// DEBUG //
	READ_INT_DEV(SimulatePacketLossRatio)
	READ_BOOL_DEV(PreCacheShapes)
	READ_BOOL_FV(ResetShapeBankOnRetCharSelect)
	READ_BOOL_DEV(DisplayMissingAnimFile)
	READ_STRING_DEV(DefaultEntity)
	READ_BOOL_DEV(RestrainPI)
	READ_BOOL_DEV(DumpVSIndex)
	READ_INT_DEV(HelpFontSize)
#if !FINAL_VERSION
	// HelpFontColor
	{
		CConfigFile::CVar *cvHelpFontColor = ClientCfg.ConfigFile.getVarPtr("HelpFontColor");
		if(cvHelpFontColor && cvHelpFontColor->size() == 3)
		{
			ClientCfg.HelpFontColor.R = cvHelpFontColor->asInt(0);
			ClientCfg.HelpFontColor.G = cvHelpFontColor->asInt(1);
			ClientCfg.HelpFontColor.B = cvHelpFontColor->asInt(2);
			ClientCfg.HelpFontColor.A = 255;
		}
		else
			cfgWarning("Default value used for 'HelpFontColor' !!!");
	}
#endif // !FINAL_VERSION
	// HelpLineStep
	READ_FLOAT_DEV(HelpLineStep)

	READ_INT_DEV(DebugFontSize)
#if !FINAL_VERSION
	// DebugFontColor
	CConfigFile::CVar *pcvDebugFontColor = ClientCfg.ConfigFile.getVarPtr("DebugFontColor");
	if( pcvDebugFontColor && (pcvDebugFontColor->size() == 3) )
	{
		ClientCfg.DebugFontColor.R = pcvDebugFontColor->asInt(0);
		ClientCfg.DebugFontColor.G = pcvDebugFontColor->asInt(1);
		ClientCfg.DebugFontColor.B = pcvDebugFontColor->asInt(2);
		ClientCfg.DebugFontColor.A = 255;
	}
	else
		cfgWarning("Default value used for 'DebugFontColor'");
#endif // !FINAL_VERSION
	READ_FLOAT_DEV(DebugLineStep)

	// HeadOffset
	READ_CVECTOR_DEV(HeadOffset)
	READ_BOOL_DEV(FPExceptions)
	READ_BOOL_DEV(NeedComputeVS)
	READ_BOOL_DEV(Check)
	READ_BOOL_DEV(UsePACSForAll)
	READ_FLOAT_DEV(WaterEnvMapUpdateTime)
	READ_BOOL_DEV(BlendForward)

	ClientCfg.ZCPacsPrim = "gen_bt_col_ext.pacs_prim";
	READ_STRING_DEV(ZCPacsPrim)

	READ_BOOL_FV(AutoReloadFiles)
	READ_BOOL_DEV(BlendShapePatched)

	READ_INT_DEV(MaxNumberOfTimedFXInstances);

	READ_STRING_DEV(SelectionFX);
	READ_STRING_DEV(MouseOverFX);
	READ_FLOAT_DEV(SelectionFXSize);

	if(ClientCfg.ConfigFile.exists("ExtendedCommands") && ClientCfg.ConfigFile.getVar("ExtendedCommands").asString() == "Enable")
		ClientCfg.ExtendedCommands = true;


	READ_BOOL_DEV(R2Mode);
	if (ClientCfg.Local) // R2EDEnabled is now set by the server
	{
		READ_BOOL_DEV(R2EDEnabled)
	}
	READ_INT_FV(R2EDDssNetwork)


	if (ClientCfg.Local)
	{
		ClientCfg.R2EDDssNetwork = 1;
	}

	READ_BOOL_DEV(R2EDExtendedDebug)
	#if FINAL_VERSION
   {
	   CConfigFile::CVar *var = ClientCfg.ConfigFile.getVarPtr("R2EDExtendedDebug");
	   if (var)
	   {
		   var->setAsInt(0);
	   }
	   // else no-op -> will resolve to 'nil' into lua
   }
   #endif


	READ_BOOL_DEV(R2EDVerboseParseTime)
	READ_BOOL_DEV(R2EDDontReparseUnchangedUIFiles)
	READ_BOOL_DEV(R2EDLightPalette)
	READ_INT_FV(R2EDAutoSaveWait)
	READ_INT_FV(R2EDAutoSaveSlot)
	READ_BOOL_DEV(R2EDMustVerifyRingAccessWhileLoadingAnimation)
	READ_BOOL_FV(R2EDUseVerboseRingAccess)



	//////////
	// TEMP //
	// Array with the name of all offensive impact FXs.
	READ_STRINGVECTOR_DEV(OffImpactFX);

#ifndef RZ_NO_CLIENT

	//////////
	// INIT //
	// FPU
#ifdef NL_OS_WINDOWS
	if(ClientCfg.FPExceptions)
		_control87(_EM_DENORMAL/*|_EM_INVALID|_EM_ZERODIVIDE|_EM_OVERFLOW*/|_EM_UNDERFLOW|_EM_INEXACT, _MCW_EM);
	else
		_control87(_EM_INVALID|_EM_DENORMAL|_EM_ZERODIVIDE|_EM_OVERFLOW|_EM_UNDERFLOW|_EM_INEXACT, _MCW_EM);

	// Set the process priority
	static DWORD priority[6]=
	{
		0x40,	// IDLE_PRIORITY_CLASS
		0x4000, // BELOW_NORMAL_PRIORITY_CLASS
		0x20,	// NORMAL_PRIORITY_CLASS
		0x8000, // ABOVE_NORMAL_PRIORITY_CLASS
		0x80,	// HIGH_PRIORITY_CLASS
		0x100,	// REALTIME_PRIORITY_CLASS
	};

	int index = ClientCfg.ProcessPriority+2;
	clamp(index, 0, 6);
	SetPriorityClass (GetCurrentProcess(), priority[index]);
#endif // NL_OS_WINDOWS

	// Init Verbose Modes (at the beginning to be able to display them as soon as possible).
	::VerboseVP				= ClientCfg.VerboseVP;
	::VerboseAnimUser			= ClientCfg.VerboseAnimUser;
	::VerboseAnimSelection	= ClientCfg.VerboseAnimSelection;
#ifdef LOG_ALL_TRAFFIC
	NLMISC::VerboseAllTraffic = ClientCfg.VerboseAllTraffic;
#endif

	// (re-)Initialize contextual cursors.
	if (!ClientCfg.R2EDEnabled)
	{
		initContextualCursor();
	}

	// Prim files
	READ_STRINGVECTOR_DEV(PrimFiles);

	// Reset GlobalWind Setup
	if(Scene)
	{
		Scene->setGlobalWindPower(ClientCfg.GlobalWindPower);
		Scene->setGlobalWindDirection(ClientCfg.GlobalWindDirection);
	}

	if (Driver)
	{
		// Set the monitor color properties
		CMonitorColorProperties monitorColor;
		for(uint i=0; i<3; i++)
		{
			monitorColor.Contrast[i]	= ClientCfg.Contrast;
			monitorColor.Luminosity[i]	= ClientCfg.Luminosity;
			monitorColor.Gamma[i]		= ClientCfg.Gamma;
		}
		if(!Driver->setMonitorColorProperties(monitorColor))
			cfgWarning("reloadCFG: setMonitorColorProperties fails");
	}


	// Show/hide all or parts of the user body.
	if (UserEntity)
	{
		UserEntity->eyesHeight(ClientCfg.EyesHeight);
		UserEntity->updateVisualDisplay();

		// Run speed and camera dist max are set according to R2 char mode
		UserEntity->flushR2CharMode();
	}

	// Initialize the camera distance (after camera dist max)
	View.setCameraDistanceMaxForPlayer();

	// draw in client light?
	if(ClientCfg.Light)
	{
		ClientCfg.DrawBoxes = true;
		ClientCfg.Names = true;
	}

	// Set Day / Time
	if (ClientCfg.Local)
	{
		uint32 tickOffset = (uint32)(ClientCfg.RyzomDay * RYZOM_HOURS_IN_TICKS * 24 + ClientCfg.RyzomTime * RYZOM_HOURS_IN_TICKS);
		RT.resetTickOffset();
		RT.increaseTickOffset( tickOffset );
	}
#endif

	// for reset effect of variable in mainLoop(), set true
	ClientCfg.IsInvalidated= true;

	// Allow warning display only first time.
	DisplayCFGWarning= false;

	// If it is the load time, bkup the ClientCfg into LastClientCfg
	if(firstTimeSetValues)
		LastClientCfg = ClientCfg;

	// no more true.
	firstTimeSetValues= false;

	READ_INT_DEV(NumFrameForProfile);
	READ_STRING_FV(KlientChatPort);
	READ_BOOL_DEV(SimulateServerTick);

	READ_BOOL_DEV(DamageShieldEnabled)

	READ_BOOL_DEV(AllowDebugLua)
	READ_BOOL_DEV(LoadLuaDebugger)
	READ_BOOL_DEV(DisplayLuaDebugInfo)

	READ_BOOL_DEV(LuaDebugInfoGotoButtonEnabled)
	READ_STRING_DEV(LuaDebugInfoGotoButtonTemplate)
	READ_STRING_DEV(LuaDebugInfoGotoButtonCaption)
	READ_STRING_DEV(LuaDebugInfoGotoButtonFunction)


	READ_BOOL_DEV(BeepWhenLaunched)

	READ_STRING_DEV(R2ClientGw)

	READ_FLOAT_FV(FogDistAndDepthLookupBias)

	READ_INT_DEV(R2EDLoadDynamicFeatures)

	READ_BOOL_DEV(CheckR2ScenarioMD5);

	CConfigFile::CVar *pcvHardwareCursors = ClientCfg.ConfigFile.getVarPtr("HardwareCursors");
	if(pcvHardwareCursors)
	{
		ClientCfg.HardwareCursors.clear ();
		int iSz = pcvHardwareCursors->size();
		for (int i = 0; i < iSz; i++)
			ClientCfg.HardwareCursors.insert(toLower(pcvHardwareCursors->asString(i)));
	}
	else
	{
		cfgWarning("Default value used for 'HardwareCursors'");
		// default list of harware cursors
		ClientCfg.HardwareCursors.insert("curs_can_pan.tga");
		ClientCfg.HardwareCursors.insert("curs_can_pan_dup.tga");
		ClientCfg.HardwareCursors.insert("curs_create.tga");
		ClientCfg.HardwareCursors.insert("curs_create_multi.tga");
		ClientCfg.HardwareCursors.insert("curs_create_vertex_invalid.tga");
		ClientCfg.HardwareCursors.insert("curs_default.tga");
		ClientCfg.HardwareCursors.insert("curs_dup.tga");
		ClientCfg.HardwareCursors.insert("curs_L.tga");
		ClientCfg.HardwareCursors.insert("curs_M.tga");
		ClientCfg.HardwareCursors.insert("curs_pan.tga");
		ClientCfg.HardwareCursors.insert("curs_pan_dup.tga");
		ClientCfg.HardwareCursors.insert("curs_pick.tga");
		ClientCfg.HardwareCursors.insert("curs_pick_dup.tga");
		ClientCfg.HardwareCursors.insert("curs_R.tga");
		ClientCfg.HardwareCursors.insert("curs_resize_BL_TR.tga");
		ClientCfg.HardwareCursors.insert("curs_resize_BR_TL.tga");
		ClientCfg.HardwareCursors.insert("curs_resize_LR.tga");
		ClientCfg.HardwareCursors.insert("curs_resize_TB.tga");
		ClientCfg.HardwareCursors.insert("curs_rotate.tga");
		ClientCfg.HardwareCursors.insert("curs_scale.tga");
		ClientCfg.HardwareCursors.insert("curs_stop.tga");
		ClientCfg.HardwareCursors.insert("text_cursor.tga");
		ClientCfg.HardwareCursors.insert("r2_hand_can_pan.tga");
		ClientCfg.HardwareCursors.insert("r2_hand_pan.tga");
		ClientCfg.HardwareCursors.insert("r2ed_tool_can_pick.tga");
		ClientCfg.HardwareCursors.insert("r2ed_tool_can_rotate.tga");
		ClientCfg.HardwareCursors.insert("r2ed_tool_pick.tga");
		ClientCfg.HardwareCursors.insert("r2ed_tool_rotate.tga");
		ClientCfg.HardwareCursors.insert("r2ed_tool_rotating.tga");
	}

	// languages and types of Ring Scenarii
	READ_STRINGVECTOR_FV(ScenarioLanguages);
	READ_STRINGVECTOR_FV(ScenarioTypes);

	// build name
	READ_STRING_FV(BuildName)

	READ_BOOL_DEV(DisplayTPReason)

	//READ_INT_FV(TPQuitButtonX)
	//READ_INT_FV(TPQuitButtonY)
	//READ_INT_FV(TPCancelButtonX)
	//READ_INT_FV(TPCancelButtonY)
	READ_INT_FV(TPButtonW)
	READ_INT_FV(TPButtonH)

	READ_STRING_FV(ScenarioSavePath)


	ClientCfg.R2EDClippedEntityBlendTime = 0.18f;
	READ_FLOAT_FV(R2EDClippedEntityBlendTime)

	// vl: BackgroundDownloader is hardcoded to false and we don't want to run it, even if the cfg wants it
	//READ_BOOL_FV(BackgroundDownloader)

}// load //


//-----------------------------------------------
// serial :
// Serialize CFG.
//-----------------------------------------------
void CClientConfig::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Start the opening of a new node named ClientCFG.
	f.xmlPush("ClientCFG");

		f.xmlPushBegin("Light");
		f.xmlPushEnd();
		f.serial(Light);
		f.xmlPop();

		f.xmlPushBegin("Windowed");
		f.xmlPushEnd();
		f.serial(Windowed);
		f.xmlPop();

		f.xmlPushBegin("Width");
		f.xmlPushEnd();
		f.serial(Width);
		f.xmlPop();

		f.xmlPushBegin("Height");
		f.xmlPushEnd();
		f.serial(Height);
		f.xmlPop();

		f.xmlPushBegin("Depth");
		f.xmlPushEnd();
		f.serial(Depth);
		f.xmlPop();

		f.xmlPushBegin("Contrast");
		f.xmlPushEnd();
		f.serial(Contrast);
		f.xmlPop();

		f.xmlPushBegin("Luminosity");
		f.xmlPushEnd();
		f.serial(Luminosity);
		f.xmlPop();

		f.xmlPushBegin("Gamma");
		f.xmlPushEnd();
		f.serial(Gamma);
		f.xmlPop();


		f.xmlPushBegin("AttackDist");
		f.xmlPushEnd();
		f.serial(AttackDist);
		f.xmlPop();

		// SelectWithRClick
		f.xmlPushBegin("SelectWithRClick");
		f.xmlPushEnd();
		f.serial(SelectWithRClick);
		f.xmlPop();

	// Close the serial for the Client CFG.
	f.xmlPop();
}// serial //


//-----------------------------------------------
// init :
//-----------------------------------------------
void CClientConfig::init(const string &configFileName)
{
	// if the users client config does not exist
	if(!CFile::fileExists(configFileName))
	{
		// create the basic .cfg
		FILE *fp = fopen(configFileName.c_str(), "w");

		if (fp == NULL)
			nlerror("CFG::init: Can't create config file '%s'", configFileName.c_str());
		else
			nlwarning("CFG::init: creating '%s' with default values", configFileName.c_str ());

		// get current locale
		std::string lang = toLower(std::string(setlocale(LC_CTYPE, "")));
		lang = lang.substr(0, 2);

		const std::vector<std::string> &languages = CI18N::getLanguageCodes();

		// search if current locale is defined in language codes
		for(uint i = 0; i < languages.size(); ++i)
		{
			if (lang == languages[i])
			{
				// store the language code in the config file
				fprintf(fp, "LanguageCode         = \"%s\";\n", lang.c_str());
				break;
			}
		}

		fclose(fp);
	}

	// read the exising config file (don't parse it yet!)
	ucstring content;
	NLMISC::CI18N::readTextFile(configFileName, content);
	std::string contentUtf8 = content.toUtf8();

	// while there are "RootConfigFilename" values, remove them
	size_t pos = 0;
	while((pos = contentUtf8.find("RootConfigFilename")) != configFileName.npos)
	{
		size_t endOfLine = contentUtf8.find("\n", pos);
		contentUtf8.erase(pos, (endOfLine - pos) + 1);
	}

	// get current location of the root config file (client_default.cfg)
	std::string defaultConfigLocation;
	if(!getDefaultConfigLocation(defaultConfigLocation))
		nlerror("cannot find client_default.cfg");

	// and store it in the RootConfigFilename value in the very first line
	contentUtf8.insert(0, std::string("RootConfigFilename   = \"") +
		defaultConfigLocation + "\";\n");

	// save the updated config file
	NLMISC::COFile configFile(configFileName, false, true, false);
	configFile.serialBuffer((uint8*)contentUtf8.c_str(), (uint)contentUtf8.size());
	configFile.close();

	// now we can continue loading and parsing the config file


	// if the config file will be modified, it calls automatically the function setValuesOnFileChange()
	ClientCfg.ConfigFile.setCallback (CClientConfig::setValuesOnFileChange);

	// load the config files
	ClientCfg.ConfigFile.load (configFileName);


	// update the ConfigFile variable in the config file
	CConfigFile::CVar *varPtr = ClientCfg.ConfigFile.getVarPtr ("ClientVersion");
	if (varPtr)
	{
		string str = varPtr->asString ();
		if (str != RYZOM_VERSION && ClientCfg.SaveConfig)
		{
			nlinfo ("Update and save the ClientVersion variable in config file %s -> %s", str.c_str(), RYZOM_VERSION);
			varPtr->setAsString (RYZOM_VERSION);
			ClientCfg.ConfigFile.save ();
		}
	}
	else
	{
		nlwarning ("There's no ClientVersion variable in the config file!");
	}

}// init //


//-----------------------------------------------
// init :
//-----------------------------------------------
void CClientConfig::release ()
{
#ifndef RZ_NO_CLIENT
	// Do we have to save the cfg file ?
	if (ClientCfg.SaveConfig)
	{
		// Save values
		try
		{
			CConfigFile::CVar *varPtr = NULL;

			// Driver still alive ?
			if (Driver && Driver->isActive ())
			{
				sint32 x, y;
				uint32 width, height;

				Driver->getWindowPos(x, y);
				Driver->getWindowSize(width, height);

				// Are we in window mode ?
				if (ClientCfg.Windowed /* && !isWindowMaximized() */)
				{
					// Save windows position
					writeInt("PositionX", x);
					writeInt("PositionY", y);

					// Save windows size
					writeInt("Width", std::max((sint)width, 800));
					writeInt("Height", std::max((sint)height, 600));
				}
			}

			// Save if in FPV or TPV.
			writeBool("FPV", ClientCfg.FPV);

			// Save the camera distance
			writeDouble("CameraDistance", ClientCfg.CameraDistance);
		}
		catch (const Exception &e)
		{
			nlwarning ("Error while set config file variables : %s", e.what ());
		}

		// Save it
		ClientCfg.ConfigFile.save ();
	}
#endif
}

bool CClientConfig::readBool (const std::string &varName)
{
	bool bVal = false;
	CConfigFile::CVar *varPtr = ConfigFile.getVarPtr(varName);
	if(varPtr)
		bVal = varPtr->asInt() ? true : false;
	else
		nlwarning("CFG: Default value used for '%s' !!!",varName.c_str());
	return bVal;
}

void CClientConfig::writeBool (const std::string &varName, bool bVal, bool bForce)
{
	CConfigFile::CVar *varPtr = bForce ? ConfigFile.insertVar(varName, CConfigFile::CVar()):ConfigFile.getVarPtr(varName);
	if(varPtr)
		varPtr->forceAsInt(bVal ? 1:0);
	else
		nlwarning("CFG: Default value used for '%s' !!!",varName.c_str());
}

sint32 CClientConfig::readInt (const std::string &varName)
{
	sint32 bVal = 0;
	CConfigFile::CVar *varPtr = ConfigFile.getVarPtr(varName);
	if(varPtr)
		bVal = varPtr->asInt();
	else
		nlwarning("CFG: Default value used for '%s' !!!",varName.c_str());
	return bVal;
}

void CClientConfig::writeInt (const std::string &varName, sint32 bVal, bool bForce)
{
	CConfigFile::CVar *varPtr = bForce ? ConfigFile.insertVar(varName, CConfigFile::CVar()):ConfigFile.getVarPtr(varName);
	if(varPtr)
		varPtr->forceAsInt(bVal);
	else
		nlwarning("CFG: Default value used for '%s' !!!",varName.c_str());
}

double CClientConfig::readDouble (const std::string &varName)
{
	double bVal = 0;
	CConfigFile::CVar *varPtr = ConfigFile.getVarPtr(varName);
	if(varPtr)
		bVal = varPtr->asDouble();
	else
		nlwarning("CFG: Default value used for '%s' !!!",varName.c_str());
	return bVal;
}

void CClientConfig::writeDouble (const std::string &varName, double dVal, bool bForce)
{
	CConfigFile::CVar *varPtr = bForce ? ConfigFile.insertVar(varName, CConfigFile::CVar()):ConfigFile.getVarPtr(varName);
	if(varPtr)
		varPtr->forceAsDouble(dVal);
	else
		nlwarning("CFG: Default value used for '%s' !!!",varName.c_str());
}

string CClientConfig::readString (const std::string &varName)
{
	string sVal;
	CConfigFile::CVar *varPtr = ConfigFile.getVarPtr(varName);
	if(varPtr)
		sVal = varPtr->asString();
	else
		nlwarning("CFG: Default value used for '%s' !!!",varName.c_str());
	return sVal;
}

void CClientConfig::writeString (const std::string &varName, const std::string &strVal, bool bForce)
{
	CConfigFile::CVar *varPtr = bForce ? ConfigFile.insertVar(varName, CConfigFile::CVar()):ConfigFile.getVarPtr(varName);
	if(varPtr)
		varPtr->forceAsString(strVal);
	else
		nlwarning("CFG: Default value used for '%s' !!!",varName.c_str());
}

// ***************************************************************************
bool CClientConfig::readBoolNoWarning(const std::string &varName)
{
	bool bVal = false;
	CConfigFile::CVar *varPtr = ConfigFile.getVarPtr(varName);
	if(varPtr)
		bVal = varPtr->asInt() ? true : false;
	return bVal;
}

sint32 CClientConfig::readIntNoWarning(const std::string &varName)
{
	sint32 bVal = 0;
	CConfigFile::CVar *varPtr = ConfigFile.getVarPtr(varName);
	if(varPtr)
		bVal = varPtr->asInt();
	return bVal;
}

double CClientConfig::readDoubleNoWarning(const std::string &varName)
{
	double bVal = 0;
	CConfigFile::CVar *varPtr = ConfigFile.getVarPtr(varName);
	if(varPtr)
		bVal = varPtr->asDouble();
	return bVal;
}

// ***************************************************************************
float CClientConfig::getActualLandscapeThreshold() const
{
	// The threshold to set in landscape is the inverse of the CFG one
	return 1.0f/LandscapeThreshold;
}

// ***************************************************************************
string	CClientConfig::getHtmlLanguageCode() const
{
	if(LanguageCode=="wk")
		return "en";
	else
		return LanguageCode;
}

// ***************************************************************************
ucstring CClientConfig::buildLoadingString( const ucstring& ucstr ) const
{
	if( LoadingStringCount > 0 )
	{
		uint index = rand()%LoadingStringCount;
		string tipId = "uiLoadingString"+toString(index);
		ucstring randomUCStr = CI18N::get(tipId);
		return randomUCStr;
	}
	else
		return ucstr;
}

// ***************************************************************************
bool CClientConfig::getDefaultConfigLocation(std::string& p_name) const
{
	std::string defaultConfigFileName = "client_default.cfg";
	std::string defaultConfigPath;

	p_name.clear();

#ifdef NL_OS_MAC
	// on mac, client_default.cfg should be searched in .app/Contents/Resources/
	defaultConfigPath = CPath::standardizePath(getAppBundlePath() + "/Contents/Resources/");
#elif defined(RYZOM_ETC_PREFIX)
	// if RYZOM_ETC_PREFIX is defined, client_default.cfg might be over there
	defaultConfigPath = CPath::standardizePath(RYZOM_ETC_PREFIX);
#else
	// some other prefix here :)
#endif // RYZOM_ETC_PREFIX

	// look in the current working directory first
	if (CFile::isExists(defaultConfigFileName))
		p_name = defaultConfigFileName;

	// if not in working directory, check using prefix path
	else if (CFile::isExists(defaultConfigPath + defaultConfigFileName))
		p_name = defaultConfigPath + defaultConfigFileName;

	// if some client_default.cfg was found return true
	if(p_name.size())
		return true;

	return false;
}
