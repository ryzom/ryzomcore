/** \file client_cfg.h
 * <File description>
 *
 * $Id: client_cfg.h,v 1.4 2007/07/27 15:14:50 cado Exp $
 */



#ifndef CL_CLIENT_CFG_H
#define CL_CLIENT_CFG_H


//////////////
// Includes //
//////////////
// Misc.
#include "nel/misc/types_nl.h"
//#include "nel/misc/vector.h"
#include "nel/misc/config_file.h"
// std.
#include <string>
// Game Share
#include "game_share/gender.h"


///////////
// Using //
///////////
//using NLMISC::CVector;
using std::string;


//---------------------------------------------------
// CClientConfig :
// Struct to manage a config file for the client.
//---------------------------------------------------
struct CClientConfig
{
	// the config file must be always be available
	NLMISC::CConfigFile	ConfigFile;

	bool R2Mode;

	void init (const string &configFileName);
	bool				IsInvalidated;

	// Session id of mainland where to create new chars
	sint				CreateOnMainland;
	/// Select character automatically (dont go to create perso) (-1 if no auto select)
	sint				SelectCharacter;
	/// Host.
	string				FSHost;
	/// Number of simulated clients
	uint				NbConnections;
	/// Time inputs
	//uint				ForceDeltaTime;		// 0 to use real delta time
	/// Pre Data Path.
	//std::vector<string>	PreDataPath;
	/// Data Path.
	std::vector<string>	DataPath;
	/// Data Path no recurse.
	std::vector<string>	DataPathNoRecurse;
	/// Update packed sheet Path.
	//std::vector<string>			UpdatePackedSheetPath;
	/// True if we want the packed sheet to be updated if needed
	//bool			UpdatePackedSheet;
	/// Path for the Id file.
	//string		IdFilePath;
	/// Vector with some entities to spawn at the beginning.
	//std::vector<string> StartCommands;
	/// Prim file to load
	//std::vector<string> PrimFiles;
	/// To force the client to sleep a bit (in ms).
	//sint			Sleep;
	/// Force process priority
	//sint			ProcessPriority;
	/// The Sheet used by the user.
	//std::string	UserSheet;
	/// Distance between 2 attackers.
	//float			AttackDist;
	/// Change the direction once this angle is reached.
	//double		ChangeDirAngle;
	/// Distance Maximum to be able to select an entity.
	//float			SelectionDist;
	/// Distance Maximum to be able to loot/harvest a corpse.
	//float			LootDist;
	bool			VerboseVP;
	bool			VerboseAllTraffic;
	// LIGO //
	//std::string	LigoPrimitiveClass;
	// Default creature spawned when the entity do not exist
	//std::string	DefaultEntity;
	// Restrain the predicted interval
	bool			RestrainPI;
	// Dump Visual Slot IDs.
	//bool			DumpVSIndex;
	// Font size for Help infos.
	// Enable/disable Floating Point Exceptions
	//bool			FPExceptions;
	// Use to launch exceptions when there is something wrong
	bool			Check;
	// Use PACS collision for all (Work In Progress).
	//bool			UsePACSForAll;
	
	//float			ImpactTriggerTime;
	//bool			NeedComputeVS;
	//bool			ForceLanguage;
	std::string	LanguageCode;
	//bool			DebugStringManager;
	//bool			PreCacheShapes;
	uint32			SimulatePacketLossRatio;
	//int			CheckMemoryEveryNFrame;	// -1 no check (default) else number frame to skip before a memory checking
	//bool			LogMemoryAllocation;	// false no log (default) else log each memory allocation in the file "alloc.memlog"
	//int			LogMemoryAllocationSize;	// Size of the block to log (default : 8)
	// Reload config files, teleport list and sheet_id.bin
	bool			AutoReloadFiles;
	// SimulateServerTick
	//bool			SimulateServerTick;
//	bool			RequestQuit;
	
public:
	/// Constructor.
	CClientConfig();

	static void setValues ();				// Set the values of the ClientCfg instance
	static void setValuesOnFileChange ();	// called when cfg modified
	
	/// End process
	void release ();

	bool readBool (const std::string &varName);
	void writeBool (const std::string &varName, bool val);
	sint32 readInt (const std::string &varName);
	void writeInt (const std::string &varName, sint32 val);
	double readDouble (const std::string &varName);
	void writeDouble (const std::string &varName, double val);
	void writeString (const std::string &varName, const std::string &bVal);

	// return 0 / false if not succeed
	bool readBoolNoWarning (const std::string &varName);
	sint32 readIntNoWarning (const std::string &varName);
	double readDoubleNoWarning (const std::string &varName);
	std::string readString (const std::string &varName);

	// Return LanguageCode but if "wk", then return "en"
	//string	getHtmlLanguageCode() const;

};// CClientConfig //


////////////
// GLOBAL //
////////////
extern CClientConfig ClientCfg;
extern const std::string ConfigFileName;

#endif // CL_CLIENT_CFG_H

/* End of client_cfg.h */





















