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



#include "stdpch.h"


//////////////
// INCLUDES //
//////////////
// Misc.
#include "nel/misc/debug.h"
#include "nel/misc/displayer.h"
#include "nel/misc/path.h"
#include "nel/misc/i18n.h"
#include "nel/misc/log.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/report.h"
#include "nel/misc/class_registry.h"
#include "nel/misc/system_info.h"
#include "nel/misc/block_memory.h"
#include "nel/misc/system_utils.h"
// 3D Interface.
#include "nel/3d/bloom_effect.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/u_text_context.h"
#include "nel/3d/u_shape_bank.h"
#include "nel/3d/stereo_hmd.h"
// Net.
#include "nel/net/email.h"
// Ligo.
#include "nel/ligo/ligo_config.h"

// Std.
#include <fstream>
#include <sstream>
// Game Share
#include "game_share/ryzom_version.h"
// Client
#include "init.h"
#include "input.h"
#include "client_cfg.h"			// Configuration of the client.
#include "actions_client.h"
#include "color_slot_manager.h"
#include "movie_shooter.h"
#include "continent_manager.h"
#include "interface_v3/animal_position_state.h"
//#include "osd_client.h"
#include "debug_client.h"
#include "ingame_database_manager.h"
#include "client_chat_manager.h"
#include "interface_v3/input_handler_manager.h"
#include "interface_v3/interface_manager.h"
//#include "crtdbg.h"
#include "sound_manager.h"
#include "net_manager.h"
#include "sheet_manager.h"

#include "interface_v3/sbrick_manager.h"
#include "nel/gui/widget_manager.h"
//
#include "gabarit.h"
#include "hair_set.h"
//#include "starting_roles.h"

#include "init_main_loop.h"

#include "resource.h"

#include "time_client.h"
#include "pacs_client.h"
#include "interface_v3/music_player.h"

#include "input.h"
#include "interface_v3/add_on_manager.h"

#include "bg_downloader_access.h"

#include "nel/misc/check_fpu.h"

#include "login_progress_post_thread.h"

#include "browse_faq.h"



// XMLLib
#include <libxml/xmlmemory.h>

#ifdef NL_OS_WINDOWS
#include <windows.h>
extern HINSTANCE HInstance;
extern HWND SlashScreen;
#endif // NL_OS_WINDOWS

#include "app_bundle_utils.h"

#include <new>

///////////
// USING //
///////////
using namespace NLMISC;
using namespace NLNET;
using namespace NL3D;
using namespace NLLIGO;
using namespace std;

// NVIDIA recommanded drivers
#define NVIDIA_RECOMMANDED_DRIVERS UINT64_CONSTANT(0x0006000e000a1820)
#define NVIDIA_RECOMMANDED_DRIVERS_STRING_TEST "nvidia"
#define NVIDIA_RECOMMANDED_DRIVERS_STRING_NTEST "go"
#define NVIDIA_RECOMMANDED_DRIVERS_VENDOR "NVIDIA"
#define NVIDIA_RECOMMANDED_DRIVERS_URL "http://www.nvidia.com/drivers"

// ATI recommanded drivers
#define ATI_RECOMMANDED_DRIVERS UINT64_CONSTANT(0x0006000e000a191e)
#define ATI_RECOMMANDED_DRIVERS_STRING_TEST "radeon"
#define ATI_RECOMMANDED_DRIVERS_VENDOR "ATI Technologies Inc."
#define ATI_RECOMMANDED_DRIVERS_URL "http://www.ati.com/support/driver.html"

// ProgressBar steps in init / connection phase
#define BAR_STEP_INIT_CONNECTION 17

/////////////
// GLOBALS //
/////////////
// Ligo primitive class
CLigoConfig				LigoConfig;

CMsgBoxDisplayer		MsgBoxError;
CClientChatManager		ChatMngr;

bool					LastScreenSaverEnabled = false;


extern void				registerInterfaceElements();
extern CContinentManager ContinentMngr;

// Tips of the day count
#define RZ_NUM_TIPS 17
ucstring				TipsOfTheDay;
uint					TipsOfTheDayIndex;

// includes for following register classes
#include "entities.h"
#include "character_cl.h"
#include "player_cl.h"
#include "user_entity.h"
#include "fx_cl.h"
#include "item_cl.h"

///////////////
// FUNCTIONS //
///////////////

// ***************************************************************************


// XML allocator functions
// Due to Bug #906, we disable the stl xml allocation
/*
static volatile bool XmlAllocUsesSTL = true;

static std::allocator<uint8> xmlStlAlloc;



void XmlFree4NeL (void *ptr)
{
//	if (XmlAllocUsesSTL)
	{
		int size = *(((int *) ptr) - 1);
		xmlStlAlloc.deallocate((uint8 *) ptr - sizeof(int), size + sizeof(int));
	}
// 	else
// 	{
// 		MemoryDeallocate (ptr);
// 	}
}

void *XmlMalloc4NeL (size_t size)
{
//	if (XmlAllocUsesSTL)
	{
		int *newB = (int *) xmlStlAlloc.allocate(size + sizeof(int));
		*newB = (int)size;
		return (void *) (newB + 1);
	}
// 	else
// 	{
// 		return MemoryAllocate(size);
// 	}
}

void *XmlRealloc4NeL (void *ptr, size_t size)
{
//	if (XmlAllocUsesSTL)
	{
		if (ptr == NULL) return XmlMalloc4NeL(size);
		int oldSize = *(((int *) ptr) - 1);
		if (oldSize == (int) size) return ptr;
		void *newB = XmlMalloc4NeL(size);
		memcpy(newB, ptr, std::min(oldSize, (int) size));
		XmlFree4NeL(ptr);
		return newB;
	}
// 	else
// 	{
// 		// Get the block size
// 		return MemoryReallocate (ptr, size);
// 	}
}

char *XmlStrdup4NeL (const char *str)
{
	nlassert (str);
	char *newStr;
//	if (XmlAllocUsesSTL)
	{
		newStr = (char *) XmlMalloc4NeL(strlen (str)+1);
	}
// 	else
// 	{
// 		newStr = (char*)MemoryAllocate(strlen (str)+1);
// 	}
	strcpy (newStr, str);
	return newStr;
}
*/


#ifdef NL_OS_WINDOWS


static ucstring CurrentErrorMessage;

static INT_PTR CALLBACK ExitClientErrorDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM /* lParam */)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			if (CI18N::hasTranslation("TheSagaOfRyzom"))
			{
				SetWindowTextW(hwndDlg, (WCHAR*)CI18N::get ("TheSagaOfRyzom").c_str ());
			}
			SetDlgItemTextW(hwndDlg, IDC_ERROR_MSG_TEXT, (WCHAR*) CurrentErrorMessage.c_str ());
			if (CI18N::hasTranslation("uiRyzomErrorMsgBoxExit"))
			{
				SetDlgItemTextW(hwndDlg, IDOK, (WCHAR*)CI18N::get ("uiRyzomErrorMsgBoxExit").c_str ());
			}
			if (CI18N::hasTranslation("uiRyzomErrorMsgBoxHelp"))
			{
				SetDlgItemTextW(hwndDlg, IDC_RYZOM_ERROR_HELP, (WCHAR*)CI18N::get ("uiRyzomErrorMsgBoxHelp").c_str ());
			}
			RECT rect;
			RECT rectDesktop;
			GetWindowRect (hwndDlg, &rect);
			GetWindowRect (GetDesktopWindow (), &rectDesktop);
			SetWindowPos (hwndDlg, HWND_TOPMOST, (rectDesktop.right-rectDesktop.left-rect.right+rect.left)/2, (rectDesktop.bottom-rectDesktop.top-rect.bottom+rect.top)/2, 0, 0, SWP_NOSIZE);
			HICON exitClientDlgIcon = LoadIcon(HInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
			::SendMessage(hwndDlg, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) exitClientDlgIcon);
		}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hwndDlg, IDOK);
				break;
				case IDC_RYZOM_ERROR_HELP:
				{
					if (Driver)
					{
						HWND wnd = Driver->getDisplay();
						ShowWindow(wnd, SW_MINIMIZE);
					}
					browseFAQ(ClientCfg.ConfigFile);
					EndDialog(hwndDlg, IDOK);
				}
				break;
			}
		break;
		case WM_CLOSE:
			EndDialog(hwndDlg, IDOK);
		break;
	}
	return FALSE;
}

#endif



// Use this function to return an error to the final user and exit the client
void ExitClientError (const char *format, ...)
{
	char *str;
	NLMISC_CONVERT_VARGS (str, format, 256/*NLMISC::MaxCStringSize*/);

	// Driver ?
	if (Driver)
	{
		Driver->release();
	}

#ifdef NL_OS_WINDOWS
	CurrentErrorMessage.fromUtf8(str);
	DialogBox(HInstance, MAKEINTRESOURCE(IDD_ERROR_HELP_MESSAGE_BOX), NULL, ExitClientErrorDialogProc);
	/*
		ucstring ucstr;
		ucstr.fromUtf8 (str);
		MessageBoxW (NULL, (WCHAR*)ucstr.c_str(), (WCHAR*)CI18N::get ("TheSagaOfRyzom").c_str (), MB_OK|MB_ICONERROR);
	*/
#else
	fprintf (stderr, "%s\n", str);
#endif
	// Exit
	extern void quitCrashReport ();
	quitCrashReport ();
	NLMISC::NL3D_BlockMemoryAssertOnPurge = false; // at this point some object may remain allocated
												   // so don't want to fire an assert here
	exit (EXIT_FAILURE);
}

// Use this function to return an information to the final user
void ClientInfo (const ucstring &message)
{
#ifdef NL_OS_WINDOWS
	MessageBoxW (NULL, (WCHAR*)message.c_str(), (WCHAR*)CI18N::get ("TheSagaOfRyzom").c_str (), MB_OK|MB_ICONINFORMATION);
#endif
}

// Use this function to ask a question to the final user
bool ClientQuestion (const ucstring &message)
{
#ifdef NL_OS_WINDOWS
	return MessageBoxW (NULL, (WCHAR*)message.c_str(), (WCHAR*)CI18N::get ("TheSagaOfRyzom").c_str (), MB_YESNO|MB_ICONQUESTION) != IDNO;
#else
	return false;
#endif
}

void selectTipsOfTheDay (uint /* tips */)
{
	/* todo tips of the day uncomment
	tips %= RZ_NUM_TIPS;
	TipsOfTheDayIndex = tips;
	ucstring title = CI18N::get ("uiTipsTitle");
	title += toString (tips+1);
	title += " : ";
	TipsOfTheDay = title+CI18N::get ("uiTips"+toString (tips));*/
	// todo tips of the day remove
	//trap TipsOfTheDay = CI18N::get ("uiMessageOfTheDay");
	TipsOfTheDay = ""; //trap
}

// ***************************************************************************

// For nel memory
void outOfMemory()
{
	nlstopex (("OUT OF MEMORY"));
}

// For multi cpu, active only one CPU for the main thread
uint64		Debug_OldCPUMask= 0;
uint64		Debug_NewCPUMask= 0;
void setCPUMask ()
{
	uint64 cpuMask = IProcess::getCurrentProcess ()->getCPUMask();
	Debug_OldCPUMask= cpuMask;

	// get the processor to allow process
	uint i = 0;
	while ((i<64) && ((cpuMask&(SINT64_CONSTANT(1)<<i)) == 0))
		i++;

	// Set the CPU mask
	if (i<64)
	{
		IProcess::getCurrentProcess ()->setCPUMask(1<<i);
		//IThread::getCurrentThread ()->setCPUMask (1<<i);
	}

	// check
	cpuMask = IProcess::getCurrentProcess ()->getCPUMask();
	Debug_NewCPUMask= cpuMask;
}

void	displayCPUInfo()
{
	nlinfo("CPUInfo: CPUMask before change: %x, after change: %x, CPUID: %x, hasHyperThreading: %s", (uint32)Debug_OldCPUMask, (uint32)Debug_NewCPUMask, CSystemInfo::getCPUID(), (CSystemInfo::hasHyperThreading()?"YES":"NO"));
}

string getVersionString (uint64 version)
{
	return toString ("%u.%u.%u.%u", (unsigned int) (version >> 48), (unsigned int) ((version >> 32) & 0xffff), (unsigned int) ((version >> 16) & 0xffff), (unsigned int) (version & 0xffff));
}


string getSystemInformation()
{
	string s;
	s += "Memory: " + bytesToHumanReadable(CSystemInfo::availablePhysicalMemory()) + "/" + bytesToHumanReadable(CSystemInfo::totalPhysicalMemory()) + "\n";
	s += "Process Virtual Memory: " + bytesToHumanReadable(CSystemInfo::virtualMemory()) + "\n";
	s += "OS: " + CSystemInfo::getOS() + "\n";
	s += "Processor: " + CSystemInfo::getProc() + "\n";
	s += toString("CPUID: %x\n", CSystemInfo::getCPUID());
	s += toString("HT: %s\n", CSystemInfo::hasHyperThreading()?"YES":"NO");
	s += toString("CpuMask: %x\n", IProcess::getCurrentProcess ()->getCPUMask());


	if(Driver)
		s += "NeL3D: " + string(Driver->getVideocardInformation()) + "\n";
	else
		s += "NeL3D: No driver\n";

	// More display info
	string deviceName;
	uint64 driverVersion;
	if (CSystemInfo::getVideoInfo (deviceName, driverVersion))
	{
		s += "3DCard: ";
		s += deviceName;
		s += ", version ";
		s += getVersionString (driverVersion) + "\n";
	}

	if (SoundMngr && SoundMngr->getMixer())
		SoundMngr->getMixer()->writeProfile (s);
	else
		s += "No sound\n";

	return s;
}

static string crashCallback()
{
	string s = getDebugInformation();
	s += getSystemInformation();

	#ifdef NL_OS_WINDOWS
		if (Driver)
		{
			NL3D::UDriver::CMode mode;
			Driver->getCurrentScreenMode(mode);
			if (!mode.Windowed)
			{
				HWND wnd = Driver->getDisplay();
				ShowWindow(wnd, SW_MINIMIZE);
			}
		}
	#endif


	return s;
}

void checkDriverVersion()
{
	string deviceName;
	uint64 driverVersion;
	if (CSystemInfo::getVideoInfo (deviceName, driverVersion))
	{
		static uint64 driversVersion[]=
		{
			NVIDIA_RECOMMANDED_DRIVERS,
			ATI_RECOMMANDED_DRIVERS
		};
		static const char *driversTest[]=
		{
			NVIDIA_RECOMMANDED_DRIVERS_STRING_TEST,
			ATI_RECOMMANDED_DRIVERS_STRING_TEST
		};
		static const char *driversNTest[]=
		{
			NVIDIA_RECOMMANDED_DRIVERS_STRING_NTEST,
			NULL
		};
		static const char *driversURL[]=
		{
			NVIDIA_RECOMMANDED_DRIVERS_URL,
			ATI_RECOMMANDED_DRIVERS_URL
		};
		static const char *driversVendor[]=
		{
			NVIDIA_RECOMMANDED_DRIVERS_VENDOR,
			ATI_RECOMMANDED_DRIVERS_VENDOR,
		};

		uint i;
		for (i=0; i< sizeofarray(driversVersion); i++)
		{
			string lwr = deviceName;
			strlwr(lwr);
			if ((lwr.find (driversTest[i])!=string::npos) && (driversNTest[i]==NULL || lwr.find (driversNTest[i])==string::npos))
			{
				if (driverVersion < driversVersion[i])
				{
					ucstring message = CI18N::get ("uiUpdateDisplayDriversNotUpToDate") + "\n\n";
					// message += CI18N::get ("uiUpdateDisplayDriversVendor") + driversVendor[i] + "\n";
					message += CI18N::get ("uiUpdateDisplayDriversCard") + deviceName + "\n";
					message += CI18N::get ("uiUpdateDisplayDriversCurrent") + getVersionString (driverVersion) + "\n";
					message += CI18N::get ("uiUpdateDisplayDriversRecommanded") + getVersionString (driversVersion[i]) + "\n\n";
					message += CI18N::get ("uiUpdateDisplayDrivers") + "\n";
					if (ClientQuestion (message))
					{
						openURL(driversURL[i]);
						extern void quitCrashReport ();
						quitCrashReport ();
						exit (EXIT_FAILURE);
					}
				}
				break;
			}
		}
		if (i==sizeof (driversVersion)/sizeof(uint))
			nlwarning ("Unknown video card : %s", deviceName.c_str());
	}
	else
		nlwarning ("Can't check video driver version");
}

void checkDriverDepth ()
{
	// Check desktop is in 32 bit else no window mode allowed.
	if (ClientCfg.Windowed)
	{
		nlassert (Driver);
		UDriver::CMode mode;
		Driver->getCurrentScreenMode(mode);
#ifdef NL_OS_WINDOWS
		if (mode.Depth != 32)
#else
		if (mode.Depth != 16 && mode.Depth != 24 && mode.Depth != 32)
#endif
			ExitClientError (CI18N::get ("uiDesktopNotIn32").toUtf8().c_str ());
	}
}

static std::string replaceApplicationDirToken(const std::string &dir)
{

#ifdef NL_OS_MAC
  // if client_default.cfg is not in current directory, and it's not an absolute path, use application default directory
  if (!CFile::isExists("client_default.cfg") && dir.size()>0 && dir[0]!='/')
    {
	return  getAppBundlePath() + "/Contents/Resources/" + dir;
    }
#else
	static const std::string token = "<ApplicationDir>";
	std::string::size_type pos = dir.find(token);
	if (pos != std::string::npos)
		return dir.substr(0, pos) + getAppBundlePath() + dir.substr(pos + token.length());
#endif

//	preDataPath = getAppBundlePath() + "/Contents/Resources/" + preDataPath;

	return dir;
}

void listStereoDisplayDevices(std::vector<NL3D::CStereoDeviceInfo> &devices)
{
	bool cache = VRDeviceCache.empty();
	nldebug("VR [C]: List devices");
	if (cache)
	{
		VRDeviceCache.push_back(std::pair<std::string, std::string>("Auto", "0"));
	}
	IStereoDisplay::listDevices(devices);
	for (std::vector<NL3D::CStereoDeviceInfo>::iterator it(devices.begin()), end(devices.end()); it != end; ++it)
	{
		std::stringstream name;
		name << IStereoDisplay::getLibraryName(it->Library) << " - " << it->Manufacturer << " - " << it->ProductName;
		std::stringstream fullname;
		fullname << std::string("[") << name << "] [" << it->Serial << "]";
		nlinfo("VR [C]: Stereo Display: %s", name.str().c_str());
		if (cache)
		{
			VRDeviceCache.push_back(std::pair<std::string, std::string>(name.str(), it->Serial)); // VR_CONFIG
		}
	}
}

void cacheStereoDisplayDevices() // VR_CONFIG
{
	if (VRDeviceCache.empty())
	{
		std::vector<NL3D::CStereoDeviceInfo> devices;
		listStereoDisplayDevices(devices);
	}
}

void initStereoDisplayDevice()
{
	if (ClientCfg.VREnable)
	{
		// VR_CONFIG
		nldebug("VR [C]: Enabled");
		std::vector<NL3D::CStereoDeviceInfo> devices;
		listStereoDisplayDevices(devices);
		CStereoDeviceInfo *deviceInfo = NULL;
		if (ClientCfg.VRDisplayDevice == std::string("Auto")
			&& devices.begin() != devices.end())
		{
			deviceInfo = &devices[0];
		}
		else
		{
			for (std::vector<NL3D::CStereoDeviceInfo>::iterator it(devices.begin()), end(devices.end()); it != end; ++it)
			{
				std::stringstream name;
				name << IStereoDisplay::getLibraryName(it->Library) << " - " << it->Manufacturer << " - " << it->ProductName;
				if (name.str() == ClientCfg.VRDisplayDevice)
					deviceInfo = &(*it);
				if (ClientCfg.VRDisplayDeviceId == it->Serial)
					break;
			}
		}
		if (deviceInfo)
		{
			nlinfo("VR [C]: Create VR stereo display device");
			StereoDisplay = IStereoDisplay::createDevice(*deviceInfo);
			if (StereoDisplay)
			{
				if (deviceInfo->Class == CStereoDeviceInfo::StereoHMD)
				{
					nlinfo("VR [C]: Stereo display device is a HMD");
					StereoHMD = static_cast<IStereoHMD *>(StereoDisplay);
				}
				if (Driver) // VR_DRIVER
				{
					StereoDisplay->setDriver(Driver);
				}
			}
		}
	}
	else
	{
		nldebug("VR [C]: NOT Enabled");
	}
	IStereoDisplay::releaseUnusedLibraries();
}

void addSearchPaths(IProgressCallback &progress)
{
	// Add search path of UI addon. Allow only a subset of files.
	// Must do it first because take precedence other standard files
	InterfaceAddOnManager.addSearchFiles("uiaddon", "*.xml;*.lua;*.tga", "login_*.xml;out_v2_*.xml", &progress);

	// Add Standard search paths
	{
		H_AUTO(InitRZAddSearchPath2)
		for (uint i = 0; i < ClientCfg.DataPath.size(); i++)
		{
			progress.progress ((float)i/(float)ClientCfg.DataPath.size());
			progress.pushCropedValues ((float)i/(float)ClientCfg.DataPath.size(), (float)(i+1)/(float)ClientCfg.DataPath.size());

			CPath::addSearchPath(replaceApplicationDirToken(ClientCfg.DataPath[i]), true, false, &progress);

			progress.popCropedValues ();
		}

		CPath::loadRemappedFiles("remap_files.csv");
	}
	for (uint i = 0; i < ClientCfg.DataPathNoRecurse.size(); i++)
	{
		progress.progress ((float)i/(float)ClientCfg.DataPathNoRecurse.size());
		progress.pushCropedValues ((float)i/(float)ClientCfg.DataPathNoRecurse.size(), (float)(i+1)/(float)ClientCfg.DataPathNoRecurse.size());

		CPath::addSearchPath(replaceApplicationDirToken(ClientCfg.DataPathNoRecurse[i]), false, false, &progress);

		progress.popCropedValues ();
	}
}


void addPreDataPaths(NLMISC::IProgressCallback &progress)
{
	NLMISC::TTime initPaths = ryzomGetLocalTime ();
	H_AUTO(InitRZAddSearchPaths)
	for (uint  i = 0; i < ClientCfg.PreDataPath.size(); i++)
	{
		progress.progress ((float)i/(float)ClientCfg.PreDataPath.size());
		progress.pushCropedValues ((float)i/(float)ClientCfg.PreDataPath.size(), (float)(i+1)/(float)ClientCfg.PreDataPath.size());

		CPath::addSearchPath(replaceApplicationDirToken(ClientCfg.PreDataPath[i]), true, false, &progress);

		progress.popCropedValues ();
	}
	//nlinfo ("PROFILE: %d seconds for Add search paths Predata", (uint32)(ryzomGetLocalTime ()-initPaths)/1000);
}

static void addPackedSheetUpdatePaths(NLMISC::IProgressCallback &progress)
{
	for(uint i = 0; i < ClientCfg.UpdatePackedSheetPath.size(); i++)
	{
		progress.progress((float)i/(float)ClientCfg.UpdatePackedSheetPath.size());
		progress.pushCropedValues ((float)i/(float)ClientCfg.UpdatePackedSheetPath.size(), (float)(i+1)/(float)ClientCfg.UpdatePackedSheetPath.size());
		CPath::addSearchPath(replaceApplicationDirToken(ClientCfg.UpdatePackedSheetPath[i]), true, false, &progress);
		progress.popCropedValues();
	}
}

//---------------------------------------------------
// prelogInit :
// Initialize the application before login
// if the init fails, call nlerror
//---------------------------------------------------
void prelogInit()
{
	try
	{
		// Assert if no more memory
		//	NLMEMORY::SetOutOfMemoryHook(outOfMemory);

		// Progress bar for init() and connection()
		ProgressBar.reset (BAR_STEP_INIT_CONNECTION);

		set_new_handler(outOfMemory);

		// save screen saver state and disable it
		LastScreenSaverEnabled = CSystemUtils::isScreensaverEnabled();

		if (LastScreenSaverEnabled)
			CSystemUtils::enableScreensaver(false);

		// Random init
		srand ((uint)CTime::getLocalTime());

		// Set FPU exceptions
#ifdef NL_OS_WINDOWS
		_control87 (_EM_INVALID|_EM_DENORMAL/*|_EM_ZERODIVIDE|_EM_OVERFLOW*/|_EM_UNDERFLOW|_EM_INEXACT, _MCW_EM);
#endif // NL_OS_WINDOWS
		
		CTime::CTimerInfo timerInfo;
		NLMISC::CTime::probeTimerInfo(timerInfo);
		if (timerInfo.RequiresSingleCore) // TODO: Also have a FV configuration value to force single core.
			setCPUMask();
		
		FPU_CHECKER_ONCE

		NLMISC::TTime initStart = ryzomGetLocalTime ();

		H_AUTO ( RZ_Client_Init );

		NLMISC_REGISTER_CLASS(CStage);
		NLMISC_REGISTER_CLASS(CStageSet);
		NLMISC_REGISTER_CLASS(CEntityManager);
		NLMISC_REGISTER_CLASS(CCharacterCL);
		NLMISC_REGISTER_CLASS(CPlayerCL);
		NLMISC_REGISTER_CLASS(CUserEntity);
		NLMISC_REGISTER_CLASS(CFxCL);
		NLMISC_REGISTER_CLASS(CItemCL);
		NLMISC_REGISTER_CLASS(CNamedEntityPositionState);
		NLMISC_REGISTER_CLASS(CAnimalPositionState);

	//	_CrtSetDbgFlag( _CRTDBG_CHECK_CRT_DF  );

		// Init XML Lib allocator
		// Due to Bug #906, we disable the stl xml allocation
		// nlverify (xmlMemSetup (XmlFree4NeL, XmlMalloc4NeL, XmlRealloc4NeL, XmlStrdup4NeL) == 0);

		// Init the debug memory
		initDebugMemory();

		// Add a displayer for Debug Infos.
		createDebug();

		// Client.Log displayer
		nlassert( !ErrorLog->getDisplayer("CLIENT.LOG") );
		CFileDisplayer *ClientLogDisplayer = new CFileDisplayer(getLogDirectory() + "client.log", true, "CLIENT.LOG");
		DebugLog->addDisplayer (ClientLogDisplayer);
		InfoLog->addDisplayer (ClientLogDisplayer);
		WarningLog->addDisplayer (ClientLogDisplayer);
		ErrorLog->addDisplayer (ClientLogDisplayer);
		AssertLog->addDisplayer (ClientLogDisplayer);

		setCrashCallback(crashCallback);

		// Display Some Info On CPU
		displayCPUInfo();

		// Display the client version.
#if FINAL_VERSION
		nlinfo("RYZOM VERSION : FV %s ("__DATE__" "__TIME__")", RYZOM_VERSION);
#else
		nlinfo("RYZOM VERSION : DEV %s ("__DATE__" "__TIME__")", RYZOM_VERSION);
#endif

		FPU_CHECKER_ONCE

		// Set default email value for reporting error
		setReportEmailFunction ((void*)sendEmail);
		setDefaultEmailParams ("smtp.nevrax.com", "", "ryzombug@nevrax.com");

		// create the save dir.
		if (!CFile::isExists("save")) CFile::createDirectory("save");

		// create the user dir.
		if (!CFile::isExists("user")) CFile::createDirectory("user");

#if !FINAL_VERSION
		// if we're not in final version then start the file access logger to keep track of the files that we read as we play
		//ICommand::execute("iFileAccessLogStart",*NLMISC::InfoLog);
#endif

		// Load the application configuration.
		ucstring nmsg("Loading config file...");
		ProgressBar.newMessage (nmsg);

		ClientCfg.init(ConfigFileName);

		CLoginProgressPostThread::getInstance().init(ClientCfg.ConfigFile);

		// check "BuildName" in ClientCfg
		//nlassert(!ClientCfg.BuildName.empty()); // TMP comment by nico do not commit

		// Start memory allocation log
// 		if (ClientCfg.LogMemoryAllocation)
// 			NLMEMORY::StartAllocationLog ("alloc.memlog", ClientCfg.LogMemoryAllocationSize);

		// Remap tga files on dds files.
		CPath::remapExtension ("dds", "tga", true);
		CPath::remapExtension ("dds", "png", true);
		CPath::remapExtension ("png", "tga", true);
		FPU_CHECKER_ONCE

		addPreDataPaths(ProgressBar);

		FPU_CHECKER_ONCE
		{
			H_AUTO(InitRZUIStr)

			FPU_CHECKER_ONCE
			// Set the data path for the localisation.
			const ucstring nmsg("Loading I18N...");
			ProgressBar.newMessage ( nmsg );

			FPU_CHECKER_ONCE
			STRING_MANAGER::CLoadProxy loadProxy;
			CI18N::setLoadProxy(&loadProxy);
			CI18N::load(ClientCfg.LanguageCode);
			CI18N::setLoadProxy(NULL);
			FPU_CHECKER_ONCE

			// Yoyo: Append the skills and Bricks to the I18N
			STRING_MANAGER::CStringManagerClient::initI18NSpecialWords(ClientCfg.LanguageCode);
			FPU_CHECKER_ONCE
		}

		FPU_CHECKER_ONCE

		// Check driver version
		checkDriverVersion();

		// Initialize the VR devices (even more important than the most important part of the client)
		nmsg = "Initializing VR devices...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
		initStereoDisplayDevice(); // VR_CONFIG

		// Create the driver (most important part of the client).
		nmsg = "Creating 3d driver...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

		UDriver::TDriver driver = UDriver::OpenGl;

#ifdef NL_OS_WINDOWS
		uint icon = (uint)LoadIcon(HInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
#else
		uint icon = 0;
#endif // NL_OS_WINDOWS

		switch(ClientCfg.Driver3D)
		{
#ifdef NL_OS_WINDOWS

			case CClientConfig::Direct3D:
				driver = UDriver::Direct3d;
			break;
#endif // NL_OS_WINDOWS
			case CClientConfig::DrvAuto:
			case CClientConfig::OpenGL:
				driver = UDriver::OpenGl;
			break;
			case CClientConfig::OpenGLES:
				driver = UDriver::OpenGlEs;
			break;
			default:
			break;
		}

		Driver = UDriver::createDriver(icon, driver);

		if(Driver == NULL)
		{
			ExitClientError (CI18N::get ("Can_t_load_the_display_driver").toUtf8().c_str ());
			// ExitClientError() call exit() so the code after is never called
			return;
		}

		CLoginProgressPostThread::getInstance().step(CLoginStep(LoginStep_VideoModeSetup, "login_step_video_mode_setup"));

		FPU_CHECKER_ONCE

		// Check the driver is not is 16 bits
		checkDriverDepth ();

		// For login phase, MUST be in windowed
		UDriver::CMode mode;
		mode.Width    = 1024;
		mode.Height   = 768;
		mode.Windowed = true;

		// Disable Hardware Vertex Program.
		if(ClientCfg.DisableVtxProgram)
			Driver->disableHardwareVertexProgram();
		// Disable Hardware Vertex AGP.
		if(ClientCfg.DisableVtxAGP)
			Driver->disableHardwareVertexArrayAGP();
		// Disable Hardware Texture Shader.
		if(ClientCfg.DisableTextureShdr)
			Driver->disableHardwareTextureShader();

		// Enable or disable VSync
		if(ClientCfg.WaitVBL)
			Driver->setSwapVBLInterval(1);
		else
			Driver->setSwapVBLInterval(0);
		
		if (StereoDisplay) // VR_CONFIG  // VR_DRIVER
		{
			// override mode TODO
		}

		// Set the mode of the window.
		if (!Driver->setDisplay (mode, false))
		{
			string msg;
			if (mode.Windowed)
			{
				msg = CI18N::get ("can_t_create_a_window_display").toUtf8();
			}
			else
			{
				msg = CI18N::get ("can_t_create_a_fullscreen_display").toUtf8();
			}
			msg += " (%dx%d %d ";
			msg += CI18N::get ("bits").toUtf8 ();
			msg += ")";
			ExitClientError (msg.c_str (), mode.Width, mode.Height, mode.Depth);
			// ExitClientError() call exit() so the code after is never called
			return;
		}

		// initialize system utils class
		CSystemUtils::init();
		CSystemUtils::setWindow(Driver->getDisplay());

		CLoginProgressPostThread::getInstance().step(CLoginStep(LoginStep_VideoModeSetupHighColor, "login_step_video_mode_setup_high_color"));

#ifdef NL_OS_WINDOWS

		CBGDownloaderAccess::getInstance().init();

		if (SlashScreen)
			DestroyWindow (SlashScreen);

#endif // NL_OS_WINDOW

		// Set the title
		Driver->setWindowTitle(CI18N::get("TheSagaOfRyzom"));

#if defined(NL_OS_UNIX) && !defined(NL_OS_MAC)
		// add all existing icons
		vector<string> filenames;
		filenames.push_back("/usr/share/icons/hicolor/128x128/apps/ryzom.png");
		filenames.push_back("/usr/share/icons/hicolor/48x48/apps/ryzom.png");
		filenames.push_back("/usr/share/icons/hicolor/32x32/apps/ryzom.png");
		filenames.push_back("/usr/share/icons/hicolor/24x24/apps/ryzom.png");
		filenames.push_back("/usr/share/icons/hicolor/22x22/apps/ryzom.png");
		filenames.push_back("/usr/share/icons/hicolor/16x16/apps/ryzom.png");
		filenames.push_back("/usr/share/pixmaps/ryzom.png");

		// check if an icon is present in registered paths
		if(CPath::exists("ryzom.png"))
		  filenames.push_back(CPath::lookup("ryzom.png"));

		vector<CBitmap> bitmaps;
		
		for(size_t i = 0; i < filenames.size(); ++i)
		{
			CIFile file;

			if (CFile::fileExists(filenames[i]) && file.open(filenames[i]))
			{
				CBitmap bitmap;
				if (bitmap.load(file))
					bitmaps.push_back(bitmap);
			}
		}

		Driver->setWindowIcon(bitmaps);
#endif

		sint32 posX = 0, posY = 0;

		if (ClientCfg.Windowed)
		{
			// use position saved in config
			posX = ClientCfg.PositionX;
			posY = ClientCfg.PositionY;
		}
		else
		{
			// position is not saved in config so center the window
			if (Driver->getCurrentScreenMode(mode))
			{
				posX = (mode.Width - Driver->getWindowWidth())/2;
				posY = (mode.Height - Driver->getWindowHeight())/2;
			}
		}

		// Set the window position
		Driver->setWindowPos(posX, posY);

		// Show the window
		Driver->showWindow();

		// for background downloader : store this window handle in shared memory for later access
		// (we use SendMessage to communicate with the background downloader)

		// Enough AGP for vertices ?
		if (Driver->getAvailableVertexAGPMemory () == 0)
		{
			/*
			std::string deviceName;
			uint64 driverVersion;
			CSystemInfo::getVideoInfo(deviceName, driverVersion);
			deviceName = NLMISC::toLower(deviceName);
			// for radeon 7200, patch because agp crash with agp with OpenGL -> don't display the message
			if (!(Driver->getNbTextureStages() <= 3 && strstr(deviceName.c_str(), "radeon")))
			{*/
				if (ClientQuestion (CI18N::get ("agp_trouble")))
				{
					openDoc ("client_troubles.html");
					extern void quitCrashReport ();
					quitCrashReport ();
					exit (EXIT_FAILURE);
				}
			//}
		}

		FPU_CHECKER_ONCE

		// Test mouse & keyboard low-level mode, if DisableDirectInput not set.
		// In case of failure, exit the client.
		// In case of success, set it back to normal mode, to provide for the user
		// the ability to manually set the firewall's permissions when the client connects.
		// The low-level mode will actually be set when "launching" (after loading).
		if (!ClientCfg.DisableDirectInput)
		{
			// Test mouse and set back to normal mode
			if (!Driver->enableLowLevelMouse (true, ClientCfg.HardwareCursor))
			{
				ExitClientError (CI18N::get ("can_t_initialise_the_mouse").toUtf8 ().c_str ());
				// ExitClientError() call exit() so the code after is never called
				return;
			}
			Driver->enableLowLevelMouse (false, ClientCfg.HardwareCursor);

			// Test keyboard and set back to normal mode
			// NB : keyboard will be initialized later now
			/*if (!Driver->enableLowLevelKeyboard (true))
			{
				ExitClientError (CI18N::get ("can_t_initialise_the_keyboard").toUtf8 ().c_str ());
				// ExitClientError() call exit() so the code after is never called
				return;
			}
			Driver->enableLowLevelKeyboard (false);
			*/
		}

		// Set the monitor color properties
		CMonitorColorProperties monitorColor;
		for ( uint i=0; i<3; i++)
		{
			monitorColor.Contrast[i] = ClientCfg.Contrast;
			monitorColor.Luminosity[i] = ClientCfg.Luminosity;
			monitorColor.Gamma[i] = ClientCfg.Gamma;
		}
		if (!Driver->setMonitorColorProperties (monitorColor))
		{
			nlwarning("init : setMonitorColorProperties fails");
		}

		// The client require at least 2 textures.
		if(Driver->getNbTextureStages() < 2)
			throw Exception("Application require at least 2 textures stages !!");

		Driver->enableUsedTextureMemorySum(true);

		// Initialize the font manager.
		Driver->setFontManagerMaxMemory(2000000);

		// Init the DXTCCompression.
		Driver->forceDXTCCompression(ClientCfg.ForceDXTC);

		// Set the anisotropic filter
		Driver->setAnisotropicFilter(ClientCfg.AnisotropicFilter);

		// Divide the texture size.
		if (ClientCfg.DivideTextureSizeBy2)
			Driver->forceTextureResize(2);
		else
			Driver->forceTextureResize(1);

		// Create a generic material.
		GenericMat = Driver->createMaterial();
		if(GenericMat.empty())
			nlerror("init: Cannot Create the generic material.");


		// Create a text context. We need to put the full path because we not already add search path
//		resetTextContext ("bremenb.ttf", false);
		resetTextContext ("ryzom.ttf", false);

		
		CInterfaceManager::getInstance();

		// Yoyo: initialize NOW the InputHandler for Event filtering.
		CInputHandlerManager *InputHandlerManager = CInputHandlerManager::getInstance();
		InputHandlerManager->addToServer (&Driver->EventServer);

		std::string filename = CPath::lookup( ClientCfg.XMLInputFile, false );
		if( !filename.empty() )
			InputHandlerManager->readInputConfigFile( filename );

		ProgressBar.setFontFactor(0.85f);

		nmsg = "Loading background...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

		// Choose a tips of the day
		selectTipsOfTheDay (rand());

		// Create the loading texture. We can't do that before because we need to add pre search path first and driver
		//UseEscapeDuringLoading = USE_ESCAPE_DURING_LOADING;
		UseEscapeDuringLoading = false;
		beginLoading (StartBackground); //put here intro Gameforge if wanted

		FPU_CHECKER_ONCE

		// Define the root path that contains all data needed for the application.
		nmsg = "Adding search paths...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

		if(!ClientCfg.TestBrowser)
		{
			NLMISC::TTime initPaths = ryzomGetLocalTime ();
			addSearchPaths(ProgressBar);
			if (ClientCfg.UpdatePackedSheet)
			{
				addPackedSheetUpdatePaths(ProgressBar);
			}
			//nlinfo ("PROFILE: %d seconds for Add search paths Data", (uint32)(ryzomGetLocalTime ()-initPaths)/1000);
		}

		// Register the reflected classes
		registerInterfaceElements();

		// set driver used by bloom (must be called before init)
		CBloomEffect::getInstance().setDriver(Driver);

		// init bloom effect
		CBloomEffect::getInstance().init(driver != UDriver::Direct3d);
		
		if (StereoDisplay) // VR_CONFIG
		{
			// Init stereo display resources
			StereoDisplay->setDriver(Driver); // VR_DRIVER
		}

		nlinfo ("PROFILE: %d seconds for prelogInit", (uint32)(ryzomGetLocalTime ()-initStart)/1000);

		FPU_CHECKER_ONCE
	}
	catch (const Exception &e)
	{
		ExitClientError (e.what());
	}
}


// ***************************************************************************
void	initBotObjectSelection()
{
	// Get the driver shape bank
	UShapeBank	*shapeBank= Driver->getShapeBank();
	if(!shapeBank)
		return;

	// Parse all .creature
	const CSheetManager::TEntitySheetMap	&sheets= SheetMngr.getSheets();
	CSheetManager::TEntitySheetMap::const_iterator		it= sheets.begin();
	for(;it!=sheets.end();it++)
	{
		CEntitySheet	*entitySheet= it->second.EntitySheet;
		if(entitySheet->Type!=CEntitySheet::FAUNA)
			continue;
		const CCharacterSheet *sheet= dynamic_cast<const CCharacterSheet *>(entitySheet);
		if(sheet)
		{
			// If the entity define no Skeleton, it is not skinned
			// thus we must force the shape to bkup the geometry in RAM, for fast selection
			if(sheet->IdSkelFilename==CStaticStringMapper::emptyId())
			{
				// For all equipement (yoyo: Body is theorically the only one bound)
				std::vector<const CCharacterSheet::CEquipment*> equipList;
				sheet->getWholeEquipmentList(equipList);
				for(uint i=0;i<equipList.size();i++)
				{
					string	strShape= equipList[i]->getItem();
					// if some item bound
					if(!strShape.empty())
					{
						// If this is a reference on an item
						string ext = CFile::getExtension(strShape);
						if((ext == "item") || (ext == "sitem"))
						{
							// IS the item a valid one ?
							CSheetId itemId;
							if(itemId.buildSheetId(NLMISC::strlwr(strShape)))
							{
								// Get this item sheet ?
								CItemSheet		*itemSheet= dynamic_cast<CItemSheet *>(SheetMngr.get(itemId));
								if(itemSheet)
								{
									// and so get the actual shape name
									strShape= itemSheet->getShape();
								}
							}
						}

						// If ok (after possible .sitem translation)
						if(!strShape.empty())
						{
							shapeBank->buildSystemGeometryForshape(strShape);
						}
					}
				}
			}
		}
	}
}



// ***************************************************************************
//---------------------------------------------------
// postlogInit :
// Initialize the application after login
// if the init fails, call nlerror
//---------------------------------------------------
void postlogInit()
{
	Driver->clearBuffers(CRGBA::Black);
	Driver->swapBuffers();
	CNiceInputAuto niceInputs;
	ucstring nmsg;

	try
	{
		NLMISC::TTime initStart = ryzomGetLocalTime();
		NLMISC::TTime initLast = initStart;
		NLMISC::TTime initCurrent = initLast;
		{
			H_AUTO(InitRZNetwk)
			// Initialize the Generic Message Header Manager.
			nmsg = "Initializing network...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			std::string msgXMLPath = CPath::lookup("msg.xml");
			GenericMsgHeaderMngr.init(msgXMLPath);
			initializeNetwork();

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Initializing network", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}

		{
			H_AUTO(InitRZChat)

			// init the chat manager
			nmsg = "Initializing chat manager...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			ChatMngr.init( CPath::lookup("chat_static.cdb") );

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Initializing chat manager", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}

		{
			H_AUTO(InitRZLigo)

			// Read the ligo primitive class file
			nmsg = "Initializing primitive classes...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			if (!LigoConfig.readPrimitiveClass (ClientCfg.LigoPrimitiveClass.c_str(), false))
			{
				nlwarning ("Can't load primitive class file %s", ClientCfg.LigoPrimitiveClass.c_str());
			}

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Initializing primitive classes", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}

		// set the primitive context
		CPrimitiveContext::instance().CurrentLigoConfig = &LigoConfig;

		{
			H_AUTO(InitRZShIdI)

			nmsg = "Initializing sheets...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			// Initialize Sheet IDs.
			CSheetId::init (ClientCfg.UpdatePackedSheet);

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
		}

		{
			H_AUTO(InitRZSound)

			// Init the sound manager
			nmsg = "Initializing sound manager...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
			if(ClientCfg.SoundOn)
			{
				// tmp fix : it seems that, at this point, if the bg downloader window has focus and
				// not the Ryzom one, then sound init fails
				/*#ifdef NL_OS_WINDOWS
					HWND hWnd = Driver->getDisplay ();
					nlassert (hWnd);
					ShowWindow(hWnd, SW_RESTORE);
					SetForegroundWindow(hWnd);
				#endif*/
				// bg downloader not used anymore anyways
				SoundMngr = new CSoundManager(&ProgressBar);
				try
				{
					SoundMngr->init(&ProgressBar);
				}
				catch(const Exception &e)
				{
					nlwarning("init : Error when creating 'SoundMngr' : %s", e.what());
					// leak the alocated sound manager...
					SoundMngr = 0;
				}

				// Play Music just after the SoundMngr is inited
				if(SoundMngr)
				{
					// init the SoundMngr with backuped volume
					SoundMngr->setSFXVolume(ClientCfg.SoundSFXVolume);
					SoundMngr->setGameMusicVolume(ClientCfg.SoundGameMusicVolume);

					// no fadein, and not async because don't work well because of loading in the main thread
					// Force use GameMusic volume
					const uint	fadeInTime= 500;
					SoundMngr->playMusic(ClientCfg.SoundOutGameMusic, fadeInTime, false, true, true);
					// Because of blocking loading, force the fadeIn
					TTime	t0= ryzomGetLocalTime();
					TTime	t1;
					while((t1=ryzomGetLocalTime())<t0+fadeInTime)
					{
						//ProgressBar.progress(1.f);
						SoundMngr->updateAudioMixerOnly();
					}
				}
			}

			CPath::memoryCompress(); // Because sound call addSearchPath

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Initializing sound manager", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}

		{
			H_AUTO(InitRZSheetL)

			// load packed sheets
			nmsg = "Loading sheets...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			SheetMngr.setOutputDataPath("../../client/data");
			SheetMngr.load (ProgressBar, ClientCfg.UpdatePackedSheet, ClientCfg.NeedComputeVS, ClientCfg.DumpVSIndex);

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Initializing sheets", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}

		{
			nmsg = "Initializing bricks...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			CSBrickManager::getInstance()->init(); // Must be done after sheet loading
			//STRING_MANAGER::CStringManagerClient::specialWordsMemoryCompress(); // Must be done after brick manager init

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Initializing bricks", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}

		if (!ClientCfg.Light)
		{
			H_AUTO(InitRZCh)

			nmsg = "Initializing Color Slot...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			// Initialize the color slot manager
			initColorSlotManager();

			nmsg = "Initializing Gabarit Set...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			// Initialize the set of gabarit
			GabaritSet.loadGabarits (ProgressBar);

			nmsg = "Initializing Hair Set...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			// Initialize the hair sets
			HairSet.init (ProgressBar);

			nmsg = "Initializing Starting Role Set...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			// Init all BotObjects for fast selection
			nmsg = "Initializing Bot Objects...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
			initBotObjectSelection();

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Initializing Color Slot etc.", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}
		else
		{
			// To have the same number of newMessage in client light
			nmsg = "";
			ProgressBar.newMessage (nmsg);
			ProgressBar.newMessage (nmsg);
			ProgressBar.newMessage (nmsg);
			ProgressBar.newMessage (nmsg);
			ProgressBar.newMessage (nmsg);
		}

		// Initialize MovieShooter
		{
			nmsg = "Initializing Movie Shooter ...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
			if(ClientCfg.MovieShooterMemory>0)
			{
				MovieShooter.init(ClientCfg.MovieShooterMemory);
				MovieShooter.setFrameSkip(ClientCfg.MovieShooterFrameSkip);
			}

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Initializing Movie Shooter", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}

		{
			nmsg = "Initializing primitives...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			// Register the ligo primitives for .primitive sheets
			NLLIGO::Register ();

			// Load PACS primitive
			initPrimitiveBlocks();

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Initializing primitives", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}

		{
			nmsg = "Executing cfg file start commands...";
			ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

			// Call the user commands from the config file if any
			CConfigFile::CVar *var;
			if ((var = ClientCfg.ConfigFile.getVarPtr ("StartCommands")) != NULL)
			{
				for (uint i = 0; i < var->size(); i++)
				{
					ICommand::execute (var->asString(i), *InfoLog);
				}
			}

			initLast = initCurrent;
			initCurrent = ryzomGetLocalTime();
			//nlinfo ("PROFILE: %d seconds (%d total) for Executing cfg file start commands", (uint32)(initCurrent-initLast)/1000, (uint32)(initCurrent-initStart)/1000);
		}

		// Next step will be the connection with the server.
		nmsg = "Connecting...";
		ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );

		nlinfo ("PROFILE: %d seconds for postlogInit", (uint32)(ryzomGetLocalTime ()-initStart)/1000);
	}
	catch (const Exception &e)
	{
		ExitClientError (e.what());
	}
}// init //
