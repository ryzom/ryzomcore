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



#ifndef CL_CLIENT_CFG_H
#define CL_CLIENT_CFG_H


//////////////
// Includes //
//////////////
// Misc.
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/rgba.h"
#include "nel/misc/config_file.h"

#include "nel/sound/sound_anim_manager.h"

// std.
#include <string>

// Game Share
#include "game_share/gender.h"
#include "game_share/character_title.h"


///////////
// Using //
///////////
using NLMISC::CVector;
using NLMISC::CRGBA;
using std::string;


//---------------------------------------------------
// CClientConfig :
// Struct to manage a config file for the client.
//---------------------------------------------------
struct CClientConfig
{
	enum TDriver3D { DrvAuto = 0, OpenGL, Direct3D, OpenGLES };
	enum TDriverSound { SoundDrvAuto = 0, SoundDrvFMod, SoundDrvOpenAL, SoundDrvDirectSound, SoundDrvXAudio2 };
	enum TStageLCTUsage { StageUseNoLCT = 0, StageUseAllLCT, StageUsePosOnlyLCT };

	// the config file must be always be available
	NLMISC::CConfigFile		ConfigFile;

	void init (const string &configFileName);
	bool			IsInvalidated;
	/// Save the cfg file when exit the client ?
	bool			SaveConfig;

	/// Window position in windowed mode
	sint32			PositionX;
	sint32			PositionY;

	/// Window frequency
	uint			Frequency;

	/// Skip introdution to ryzom (before the outgame select character
	bool				SkipIntro;
	/// Select character automatically (dont go to create perso) (-1 if no auto select)
	sint				SelectCharacter;
	/// Selected slot in select char interface
	uint8				SelectedSlot;
	
	/// Textures for interface login
	std::vector<string> TexturesLoginInterface;
	std::vector<string> TexturesLoginInterfaceDXTC;

	/// Textures for interface outgame
	std::vector<string> TexturesOutGameInterface;
	std::vector<string> TexturesOutGameInterfaceDXTC;

	/// Textures for ingame interface and r2 interface
	std::vector<string> TexturesInterface;
	std::vector<string> TexturesInterfaceDXTC;

	/// vector of XML file names that describe the interfaces config for login
	std::vector<string> XMLLoginInterfaceFiles;

	/// vector of XML file names that describe the interfaces config for outgame menus
	std::vector<string> XMLOutGameInterfaceFiles;

	/// vector of XML file names that describe the interfaces config
	std::vector<string> XMLInterfaceFiles;

	/// vector of XML file names that describe R2 editor
	std::vector<string> XMLR2EDInterfaceFiles;

	/// logo that are displayed
	std::vector<string>	Logos;

	/// vector of XML file names that describe input config
	std::string		XMLInputFile;

	/// Application start and just start the outgame web browser
	bool			TestBrowser;
	/// Start URL for testing the browser
	std::string		TestBrowserUrl;
	/// Application start with only the interfaces
	bool			Light;
	/// Is the landscape enabled ?
	bool			LandscapeEnabled;
	/// Is the microlife enabled
	bool			MicroLifeEnabled;
	/// are villages enabled
	bool			VillagesEnabled;
	// is EAM enabled
	bool			EAMEnabled;
	/// is level design mode enabled
	bool			LevelDesignEnabled;
	// Cache result of interface parsing
	bool			CacheUIParsing;
	/// Skip display of EULA (for test purposes)
	bool			SkipEULA;
	/// Direct 3d
	TDriver3D		Driver3D;
	/// Application start in a window or in fullscreen.
	bool			Windowed;
	/// Width for the Application.
	uint16			Width;
	/// Height for the Application.
	uint16			Height;
	/// Bit Per Pixel (only used in Fullscreen mode).
	uint16			Depth;
	/// Monitor Constrast [-1 ~ 1], default 0
	float			Contrast;
	/// Monitor Luminosity [-1 ~ 1], default 0
	float			Luminosity;
	/// Monitor Gamma [-1 ~ 1], default 0
	float			Gamma;

	// VR
	bool			VREnable;
	std::string		VRDisplayDevice;
	std::string		VRDisplayDeviceId;

	/// Client in Local mode or not.
	bool			Local;
	/// Host.
	string			FSHost;
	/// Login.
	bool			DisplayAccountButtons;
	string			CreateAccountURL;
	string			EditAccountURL;
	string			ConditionsTermsURL;
	string			BetaAccountURL;
	string			ForgetPwdURL;
	string			FreeTrialURL;
	string			LoginSupportURL;

	/// User entity Position (in local mode only).
	CVector			Position;
	/// User entity Heading (in local mode only).
	CVector			Heading;
	/// Height for the eyes.
	float			EyesHeight;
	/// Velocity for the Walk.
	float			Walk;
	/// Velocity for the Run.
	float			Run;
	/// Velocity for the Walk in DM ing or editing.
	float			DmWalk;
	/// Velocity for the Run in DM ing or editing.
	float			DmRun;
	/// Velocity for the Fly.
	float			Fly;
	/// Acceleration for the Fly.
	float			FlyAccel;

	/// Allow debug commands
	bool			AllowDebugCommands;

	/// \name Inputs

	/// Use a hardware cursor
	bool			DisableDirectInput;
	bool			DisableDirectInputKeyboard;
	bool			HardwareCursor;
	float			HardwareCursorScale; // scale for hardware cursor bitmap (in ]0, 1])
	float			CursorSpeed;
	uint			CursorAcceleration;
	float			FreeLookSpeed;
	uint			FreeLookAcceleration;
	float			FreeLookSmoothingPeriod;
	bool			FreeLookInverted;
	// true if camera is centered when user casts a spell
	bool			AutomaticCamera;
	bool			DblClickMode;
	bool			AutoEquipTool;

	/// Time inputs
	uint			ForceDeltaTime;		// 0 to use real delta time

	/// Background Color
	CRGBA			BGColor;
	/// Landscape Tile Near.
	float			LandscapeTileNear;
	/** Landscape Threshold. NB: threshold is inverted ULandscape::setThreshold(), to be more intelligible in client.cfg
	 * MUST USE getActualLandscapeThreshold() to get the real value
	 */
	float			LandscapeThreshold;
	/// Vision.
	float			Vision;
	float			Vision_min;
	float			Vision_max;
	/// Number poly max for LOD Balancing. Fx and Skin
	uint			FxNbMaxPoly;
	// Is cloud displayed
	bool			Cloud;
	float			CloudQuality;
	uint			CloudUpdate;
	uint			SkinNbMaxPoly;
	uint			NbMaxSkeletonNotCLod;
	float			CharacterFarClip;
	/// ScreenAspectRatio
	float			ScreenAspectRatio;
	/// Field of View (FoV)
	float			FoV;
	/// Force the DXTC Compression.
	bool			ForceDXTC;
	/// Set the anisotropic filter
	sint			AnisotropicFilter;
	/// Divide texture size by 2
	bool			DivideTextureSizeBy2;
	/// Disable Hardware Vertex Program.
	bool			DisableVtxProgram;
	/// Disable Hardware Vertex AGP.
	bool			DisableVtxAGP;
	/// Disable Hardware Texture Shader.
	bool			DisableTextureShdr;
	/// Enable/Disable MicroVeget.
	bool			MicroVeget;
	/// Density of microvegetation in %
	float			MicroVegetDensity;
	/// Enable/Disable High Def Entity Texture quality. By default its false
	bool			HDEntityTexture;
	/// True if the client has HD Texture installed 512*512)
	bool			HDTextureInstalled;
	/// Enable/Disable Fog
	bool			Fog;
	/// Enable/Disable VSync
	bool			WaitVBL;

	/// Timer mode. 0 : QueryPerformanceCounter, 1 : timeGetTime.
	uint			TimerMode;

	/// Global Wind Setup
	float			GlobalWindPower;
	CVector			GlobalWindDirection;

	// Is bloom effect activated
	bool			Bloom;
	bool			SquareBloom;
	float			DensityBloom;

	/// Movie Shooter
	uint			MovieShooterMemory;
	string			MovieShooterPath;
	string			MovieShooterPrefix;
	float			MovieShooterFramePeriod;
	bool			MovieShooterBlend;
	uint			MovieShooterFrameSkip;

	// Camera recorder
	string			CameraRecorderPath;
	string			CameraRecorderPrefix;
	bool			CameraRecorderBlend;

	/// Screen shot
	uint			ScreenShotWidth;	// If 0 : normal screen shot, else custom screen shot without interface
	uint			ScreenShotHeight;
	bool			ScreenShotFullDetail; // If set to true, then load balancing will be disabled for the duration of the screenshot
	bool			ScreenShotZBuffer; // If set to true, save also the ZBuffer in a file

	/////////////////////////
	// NEW PATCHING SYSTEM //
	bool			PatchWanted;
	std::string		PatchUrl;
	std::string		PatchletUrl;
	std::string		PatchVersion;
	std::string		PatchServer;

	std::string		RingReleaseNotePath;
	std::string		ReleaseNotePath;

	std::string		WebIgMainDomain;
	std::vector<string>	WebIgTrustedDomains;


	///////////////
	// ANIMATION //
	// With a bigger angle, rotation is animated.
	double			AnimatedAngleThreshold;
	uint			BlendFrameNumber;



	/// Shout color.
	CRGBA			ColorShout;
	/// Talk color.
	CRGBA			ColorTalk;

	/// Sound or not?
	bool			SoundOn;

	/// Sound Driver
	TDriverSound	DriverSound;

	/// SoundForceSoftwareBuffer
	bool			SoundForceSoftwareBuffer;

	/// The outgame music file
	string			SoundOutGameMusic;

	/// The Sound SFX Volume (0-1)  (ie all but music)
	float			SoundSFXVolume;
	/// This is volume for "InGame" music. Does not affect the MP3 player volume
	float			SoundGameMusicVolume;

	/// Time in ms of sound to be faded in/out when the user is teleported
	sint32			SoundTPFade;

	/// For Dev only
	bool			EnableBackgroundMusicTimeConstraint;

	/// Directory where to generate the sound packed sheets
	string			SoundPackedSheetPath;
	/// the directory where the sample banks are stored (.wav)
	string			SampleBankDir;
	/// The audio gain for user entity sound
	float			UserEntitySoundLevel;
	/// A flag that indicated if we use EAX
	bool			UseEax;
	/// A flag the indicate we whant ADPCM sample (reduce memory by 1/4, but lower quality)
	bool			UseADPCM;
	/// The max number of track we want to use.
	uint			MaxTrack;

	/// Pre Data Path.
	std::vector<string>			PreDataPath;
	/// Data Path.
	std::vector<string>			DataPath;
	/// Data Path no recurse.
	std::vector<string>			DataPathNoRecurse;
	/// Update packed sheet Path.
	std::vector<string>			UpdatePackedSheetPath;
	/// True if we want the packed sheet to be updated if needed
	bool			UpdatePackedSheet;
	/// TIme before exiting the application (in sec).
	float			EndScreenTimeOut;
	/// Texture file name for the loading Background.
	string			Loading_BG;
	string			LoadingFreeTrial_BG;
	/// Texture file name for the launch Background.
	string			Launch_BG;
	/// Texture file name
	string			TeleportKami_BG;
	string			TeleportKaravan_BG;
	/// Texture file name
	string			Elevator_BG;
	/// Texture file name
	string			ResurectKami_BG;
	string			ResurectKaravan_BG;
	/// Texture file name  for the last Background.
	string			End_BG;
	string			IntroNevrax_BG;
	string			IntroNVidia_BG;
	/// Message screen position
	float			TipsY;
	float			TeleportInfoY;
	/// Name of the scene to play.
	string			SceneName;
	/// Path for the Id file.
	string			IdFilePath;
	// directories where pacs primitive can be found
	std::vector<string>	PacsPrimDir;

	/// Vector with some entities to spawn at the beginning.
	std::vector<string> StartCommands;

	/// Display or not the shadows.
	bool			Shadows;
	/// Shadows are disabled after this distance.
	float			ShadowsClipFar;
	/// Shadows draw with just 1 part after this distance.
	float			ShadowsLodDist;
	/// ShadowMap Z Direction Clamping when player is on landscape/interior
	float			ShadowZDirClampLandscape;
	float			ShadowZDirClampInterior;
	float			ShadowZDirClampSmoothSpeed;
	/// ShadowMap Max Depth when player is on landscape/interior
	float			ShadowMaxDepthLandscape;
	float			ShadowMaxDepthInterior;
	float			ShadowMaxDepthSmoothSpeed;


	/// Prim file to load
	std::vector<string>			PrimFiles;

	/// Display or not the Names.
	bool			Names;
	/// To force the client to sleep a bit (in ms).
	sint			Sleep;
	/// Force process priority
	sint			ProcessPriority;
	// To show/hide the entities path
	bool			ShowPath;
	/// Draw the Boxes used for the selection.
	bool			DrawBoxes;

	/// The Sheet used by the user.
	std::string		UserSheet;
	/// (only use in local mode) User Sex.
	GSGENDER::EGender	Sex;

	/// height added to character primitive
	float			PrimitiveHeightAddition;

	/// Distance between 2 attackers.
	float			AttackDist;


	/// Day at the beginning (in local mode only)
	uint32			RyzomDay;
	/// Time at the beginning (in local mode only)
	float			RyzomTime;


	/// Temp for test : manual setup of the weather function
	bool			ManualWeatherSetup;

	float			ChaseReactionTime;

	/// Time  allowed to adjust the camera from 1st to 3rd person view (in ms).
	double			TimeToAdjustCamera;
	/// Change the direction once this angle is reached.
	double			ChangeDirAngle;
	/// Guild Symbol Size
	float			GuildSymbolSize;
	/// Distance Maximum to be able to select an entity.
	float			SelectionDist;
	/// For fair selection of entity, relative to center of bbox.
	float			SelectionOutBBoxWeight;
	/// Distance Maximum to be able to loot/harvest a corpse.
	float			LootDist;
	/// Space Selection: Max distance for space selection.
	float			SpaceSelectionDist;
	/// Space Selection: max number of entities to cycle through
	uint32			SpaceSelectionMaxCycle;


	////////////
	// TUNING //
	/// Water Offset
	float			WaterOffset;

	/// Water Offset
	float			FyrosWaterOffset;
	float			MatisWaterOffset;
	float			TrykerWaterOffset;
	float			ZoraiWaterOffset;

	/// Water Offset for creature (mektoubs)
	float			WaterOffsetCreature;
	/// Time before removing entities collisions from entities with the user in MS.
	uint32			TimeToRemoveCol;
	/// Time before stoping running to 0.5m to an entity for executing an action
	uint32			MoveToTimeToStopStall;
	/// Third Person View Min Pitch.
	float			TPVMinPitch;
	/// Third Person View Max Pitch.
	float			TPVMaxPitch;
	/// Max Head Targeting distance
	float			MaxHeadTargetDist;
	/// Name of the FX played when dead
	std::string		DeadFXName;
	/// Name of the FX played for each impact.
	std::string		ImpactFXName;
	/// Name of the FX played when Skill Up.
	std::string		SkillUpFXName;
	/// Factor of the walk animation distance after which the entity will start moving.
	double			MinDistFactor;

	/// Scale used to display names.
	float			NameScale;
	/// Distance between the name and the extended name (this value is multiply with the name scale).
	float			NamePos;
	/// Name Font Size (above entities).
	uint32			NameFontSize;
	/// Names will be hidden after this Value.
	float			MaxNameDist;
	/// Before this Value, the name size is constant.
	float			ConstNameSizeDist;
	/// If true names won't move all the time over entities
	bool			StaticNameHeight;
	/// Bars Height / 2
	float			BarsHeight;
	/// Bars Width / 2
	float			BarsWidth;
	/// Fight Area Size
	double			FightAreaSize;
	/// Destination Threshold
	double			DestThreshold;
	/// Radius of the Position Limiter (Useful to avoid some Noise on Positions).
	double			PositionLimiterRadius;
	/// Significant Distance
	double			SignificantDist;
	/// ZBias for InScene bubble
	float			BubbleZBias;
	/// ZBias for InScene Forage Interface
	float			ForageInterfaceZBias;

	/// visual scale
	float			FyrosScale;
	float			MatisScale;
	float			TrykerScale;
	float			ZoraiScale;

	/// Racial Animation
	bool			EnableRacialAnimation;

	/////////////
	// OPTIONS //
	/// Right click select too.
	bool			SelectWithRClick;
	/// Walk/Run at the beginning
	bool			RunAtTheBeginning;
	/// Rotation Velocity
	float			RotKeySpeedMax;
	float			RotKeySpeedMin;
	/// Rotation Acceleration
	float			RotAccel;
	/// Put Back Items after use
	bool			PutBackItems;
	/// Display the name of the entity under the cursor.
	bool			ShowNameUnderCursor;
	/// Display the name of the entity selected.
	bool			ShowNameSelected;
	/// Force display of names under this distance
	float			ShowNameBelowDistanceSqr;
	/// Force the FPV when the user is indoor.
	bool			ForceIndoorFPV;
	/// Follow on Attack
	bool			FollowOnAtk;
	/// Attack on Select
	bool			AtkOnSelect;
	/// Makes entities transparent if they are under cursor
	bool			TransparentUnderCursor;


	/////////////////
	// PREFERENCES //
	/// First Person View or Not.
	bool			FPV;
	/// Distance of the camera from the user.
	float			CameraHeight;
	float			CameraDistance;
	float			CameraDistStep;
	float			CameraDistMin;	// Last distance before FirstPersonView
	float			CameraDistMax; // distance max of the camera for ryzom player and ring tester player
	float			DmCameraDistMax; // distance max of the camera for ring/dm
	float			CameraAccel;
	float			CameraSpeedMin;
	float			CameraSpeedMax;
	float			CameraResetSpeed;

	//////////////
	// VERBOSES //
	bool			VerboseVP;
	bool			VerboseAnimUser;
	bool			VerboseAnimSelection;
	bool			VerboseAllTraffic;


	//////////
	// LIGO //
	std::string		LigoPrimitiveClass;

	///////////
	// DEBUG //
	// Display names of missing animation files
	bool			DisplayMissingAnimFile;
	// Default creature spawned when the entity do not exist
	std::string		DefaultEntity;
	// Restrain the predicted interval
	bool			RestrainPI;
	// Dump Visual Slot IDs.
	bool			DumpVSIndex;
	// Font size for Help infos.
	uint32			HelpFontSize;
	// Color for the Help Font.
	CRGBA			HelpFontColor;
	// Line step for Help infos.
	float			HelpLineStep;
	// Font size for debug infos.
	uint32			DebugFontSize;
	// Color for the Debug Font.
	CRGBA			DebugFontColor;
	// Line step for debug infos.
	float			DebugLineStep;
	//
	CVector			HeadOffset;
	// Enable/disable Floating Point Exceptions
	bool			FPExceptions;
	// Use to launch exceptions when there is something wrong
	bool			Check;
	// Use PACS collision for all (Work In Progress).
	bool			UsePACSForAll;

	bool			DisplayWeapons;
	bool			NeedComputeVS;
	bool			BlendForward;

	bool			ForceLanguage;
	std::string		LanguageCode;
	bool			DebugStringManager;
	bool			PreCacheShapes;
	bool			ResetShapeBankOnRetCharSelect;

	std::string		LastLogin;

	uint32			SimulatePacketLossRatio;

	// Parameters for colors of messages in system info
	// Mode is the display settings :
	// Normal	: just display in the system info window
	// Over		: must be displayed at bottom of the screen and in system info window
	// OverOnly		: must be displayed at bottom of the screen
	// Center	; must be displayed at the center of the screen and in system info window
	// Around	; must be displayed in the around chat window
	// CenterAround	; must be displayed at the center of the screen and in around chat window
	struct SSysInfoParam
	{
		CRGBA Color;
		std::string SysInfoFxName;
		enum TMode { Normal, Over, OverOnly, Center, Around, CenterAround };
		TMode Mode;
		SSysInfoParam()
		{
			Color= CRGBA::Black;
			Mode= Normal;
		}
	};
	std::map<std::string, SSysInfoParam> SystemInfoParams;


	// printf commands to display localized strings in loading screen
	struct SPrintfCommand
	{
		uint X;
		uint Y;
		CRGBA Color;
		uint FontSize;
		std::string Text;
	};
	std::vector<SPrintfCommand> PrintfCommands;

	std::vector<SPrintfCommand> PrintfCommandsFreeTrial;

	// funny loading messages count
	uint16 LoadingStringCount;


	// DEBUG MEMORY
	int				CheckMemoryEveryNFrame;	// -1 no check (default) else number frame to skip before a memory checking
	bool			LogMemoryAllocation;	// false no log (default) else log each memory allocation in the file "alloc.memlog"
	int				LogMemoryAllocationSize;	// Size of the block to log (default : 8)

	// ground FXs
	float           GroundFXMaxDist;   // max dist for ground fxs
	uint			GroundFXMaxNB;     // max number of ground fxs
	uint			GroundFXCacheSize; // max number of cached ground fxs

	//////////
	// TEMP //
	// Array with the name of all offensive impact FXs.
	std::vector<string> OffImpactFX;

	// Pacs prim used for ZC
	std::string		ZCPacsPrim;

	// Reload config files, teleport list and sheet_id.bin
	bool			AutoReloadFiles;

	// Use new version of the blend shapes (if true) new version with min,max in race_stats
	bool			BlendShapePatched;

	// Not secure boolean to disable some commands on client side
	bool			ExtendedCommands;

	// Timed FXs
	sint			MaxNumberOfTimedFXInstances;

	// Selection FXs
	std::string		SelectionFX;
	std::string		MouseOverFX;
	float			SelectionFXSize;

	// Time to update water envmap
	float			WaterEnvMapUpdateTime;

	// number of frames to profile (0 for start/stop scheme)
	uint			NumFrameForProfile;

	// KlientChatPort
	std::string		KlientChatPort;

	// SimulateServerTick
	bool			SimulateServerTick;

	// usage of LCT in _Stages of CCharacterCL
	TStageLCTUsage	StageLCTUsage;

	// TMP : for integration of damage shield
	bool			DamageShieldEnabled;
	//////////
	// R2ED //
	//////////
	/// Activate R2 login behavior
	bool					R2Mode;
	/// Force direct R2 editor jumpstart
	bool					R2EDEnabled;
	bool					R2EDVerboseParseTime;
	bool					R2EDDontReparseUnchangedUIFiles;
	std::vector<string>		R2EDReloadFiles;
	bool					R2EDExtendedDebug;
	bool					R2EDLightPalette;
	uint32					R2EDAutoSaveWait; // wait between 2 autosave in second
	uint32					R2EDAutoSaveSlot; // number of autosave file
	uint32					R2EDLoadDynamicFeatures;
	bool					R2EDMustVerifyRingAccessWhileLoadingAnimation;
	bool					R2EDUseVerboseRingAccess;

	float					R2EDClippedEntityBlendTime;

	//0: direct send packet to DSS <=> for not breaking thing
	//1: cut packet send to DSS    <=> for local or SBS less shard (dev)
	//2: cut packet, simulate SBS  <=> debug
	//3: cut packet, send to SBS   <=> will be the final value


	uint32					R2EDDssNetwork;

	/////////
	// LUA //
	/////////

	/// Allow Lua commands (commands beginning with Lua)
	bool			AllowDebugLua;
	bool			LoadLuaDebugger;

	bool			LuaDebugInfoGotoButtonEnabled;
	std::string		LuaDebugInfoGotoButtonTemplate;
	std::string		LuaDebugInfoGotoButtonCaption;
	std::string		LuaDebugInfoGotoButtonFunction;


	/// Display additional Lua DebugInfo
	bool			DisplayLuaDebugInfo;

	bool			BeepWhenLaunched; // beep when the client is launched

	// R2 Gw
	std::string		R2ClientGw;

	float			FogDistAndDepthLookupBias;

	// name of the files for hardware cursors
	std::set<std::string> HardwareCursors;

	bool			CheckR2ScenarioMD5;

	// vector of languages avoidable in Ring scenari
	std::vector<string> ScenarioLanguages;

	// vector of types avoidable in Ring scenari
	std::vector<string> ScenarioTypes;

	// build name
	std::string			BuildName;

	//
	bool				DisplayTPReason;


	//uint32				TPCancelButtonX;
	//uint32				TPCancelButtonY;
	//uint32				TPQuitButtonX;
	//uint32				TPQuitButtonY;
	uint32				TPButtonW;
	uint32				TPButtonH;

	std::string			ScenarioSavePath;


	// tmp for background downloader integration
	bool				BackgroundDownloader;

public:
	/// Constructor.
	CClientConfig();
	virtual ~CClientConfig() {}

	static void setValues ();				// Set the values of the ClientCfg instance
	static void setValuesOnFileChange ();	// called when cfg modified

	/// Serialize CFG.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	/// End process
	void release ();

	bool readBool (const std::string &varName);
	void writeBool (const std::string &varName, bool val, bool bForce = false);
	sint32 readInt (const std::string &varName);
	void writeInt (const std::string &varName, sint32 val, bool bForce = false);
	double readDouble (const std::string &varName);
	void writeDouble (const std::string &varName, double val, bool bForce = false);
	void writeString (const std::string &varName, const std::string &bVal, bool bForce = false);

	// return 0 / false if not succeed
	bool readBoolNoWarning (const std::string &varName);
	sint32 readIntNoWarning (const std::string &varName);
	double readDoubleNoWarning (const std::string &varName);
	std::string readString (const std::string &varName);


	// NB: threshold is inverted ULandscape::setThreshold(), to be more intelligible in client.cfg
	float	getActualLandscapeThreshold() const;

	// Return LanguageCode but if "wk", then return "en"
	string	getHtmlLanguageCode() const;

	// return a random loading tip or, if there are not, return the string in argument
	ucstring buildLoadingString( const ucstring& ucstr ) const;

	/// get the path to client_default.cfg including the filename itself.
	bool getDefaultConfigLocation(std::string& fileLocation) const;

};// CClientConfig //


////////////
// GLOBAL //
////////////
extern CClientConfig LastClientCfg;
extern CClientConfig ClientCfg;
extern const std::string ConfigFileName;

#endif // CL_CLIENT_CFG_H

/* End of client_cfg.h */


