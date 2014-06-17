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

#include "stdmisc.h"

#include "nel/misc/system_info.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#	include <WinNT.h>
#	include <tchar.h>
#	include <intrin.h>
#	define nlcpuid(regs, idx) __cpuid(regs, idx)
#else
#	include <sys/types.h>
#	include <sys/stat.h>
#	include <sys/sysctl.h>
#	include <fcntl.h>
#	include <unistd.h>
#	include <cerrno>
#	ifdef NL_CPU_INTEL
#		include <cpuid.h>
#		define nlcpuid(regs, idx) __cpuid(idx, regs[0], regs[1], regs[2], regs[3])
#	endif // NL_CPU_INTEL
#endif // NL_OS_WINDOWS

#include "nel/misc/system_info.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"

using namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC {

#ifdef NL_OS_UNIX
	static string getCpuInfo(const string &colname)
	{
		if (colname.empty())
			return "";

		int fd = open("/proc/cpuinfo", O_RDONLY);
		if (fd == -1)
		{
			nlwarning ("SI: Can't open /proc/cpuinfo: %s", strerror (errno));
			return "";
		}
		else
		{
			char buffer[4096+1];
			uint32 len = read(fd, buffer, sizeof(buffer)-1);
			close(fd);
			buffer[len] = '\0';

			vector<string> splitted;
			explode(string(buffer), string("\n"), splitted, true);

			std::string value;

			for(uint32 i = 0; i < splitted.size(); i++)
			{
				vector<string> sline;
				explode(splitted[i], string(":"), sline, true);
				if(sline.size() == 2 && trim(sline[0]) == colname)
				{
					value = sline[1];
				}
			}

			if (!value.empty())
				return trim(value);
		}
		nlwarning ("SI: Can't find the colname '%s' in /proc/cpuinfo", colname.c_str());
		return "";
	}

	// return the value of the colname in bytes from /proc/meminfo
	static uint64 getSystemMemory (const string &colname)
	{
		if (colname.empty())
			return 0;

		int fd = open("/proc/meminfo", O_RDONLY);
		if (fd == -1)
		{
			nlwarning ("SI: Can't open /proc/meminfo: %s", strerror (errno));
			return 0;
		}
		else
		{
			char buffer[4096+1];
			uint32 len = read(fd, buffer, sizeof(buffer)-1);
			close(fd);
			buffer[len] = '\0';

			vector<string> splitted;
			explode(string(buffer), string("\n"), splitted, true);

			for(uint32 i = 0; i < splitted.size(); i++)
			{
				vector<string> sline;
				explode(splitted[i], string(" "), sline, true);
				if(sline.size() == 3 && sline[0] == colname)
				{
					uint64 val = atoiInt64(sline[1].c_str());
					if(sline[2] == "kB") val *= 1024;
					return val;
				}
			}
		}
		nlwarning ("SI: Can't find the colname '%s' in /proc/meminfo", colname.c_str());
		return 0;
	}
#endif // NL_OS_UNIX

#ifdef NL_OS_MAC
static sint32 getsysctlnum(const string &name)
{
	sint32 value = 0;
	size_t len = sizeof(value);
	if(sysctlbyname(name.c_str(), &value, &len, NULL, 0) != 0)
	{
		nlwarning("SI: Can't get '%s' from sysctl: %s", name.c_str(), strerror (errno));
	}
	return value;
}

static sint64 getsysctlnum64(const string &name)
{
	sint64 value = 0;
	size_t len = sizeof(value);
	if(sysctlbyname(name.c_str(), &value, &len, NULL, 0) != 0)
	{
		nlwarning("SI: Can't get '%s' from sysctl: %s", name.c_str(), strerror (errno));
	}
	return value;
}

static string getsysctlstr(const string &name)
{
	string value("Unknown");
	size_t len;
	char *p;
	if(sysctlbyname(name.c_str(), NULL, &len, NULL, 0) == 0)
	{
		p = (char*)malloc(len);
		if(sysctlbyname(name.c_str(), p, &len, NULL, 0) == 0)
		{
			value = p;
		}
		else
		{
			nlwarning("SI: Can't get '%s' from sysctl: %s", name.c_str(), strerror (errno));
		}
		free(p);
	}
	else
	{
		nlwarning("SI: Can't get '%s' from sysctl: %s", name.c_str(), strerror (errno));
	}
	return value;
}
#endif // NL_OS_MAC

string CSystemInfo::getOS()
{
	string OSString = "Unknown";

#ifdef NL_OS_WINDOWS

	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
	typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

	SYSTEM_INFO si;
	PGNSI pGNSI;
	PGPI pGPI;
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;
	const int BUFSIZE = 80;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	bOsVersionInfoEx = GetVersionExA ((OSVERSIONINFO *) &osvi);

	if(!bOsVersionInfoEx)
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionExA ( (OSVERSIONINFO *) &osvi) )
			return OSString+" Can't GetVersionEx()";
	}

	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.

	pGNSI = (PGNSI) GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetNativeSystemInfo");

	if (NULL != pGNSI)
		pGNSI(&si);
	else
		GetSystemInfo(&si);

	// Test for the Windows NT product family.
	if ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion > 4 )
	{
		OSString = "Microsoft";

		if ( osvi.dwMajorVersion > 6 )
		{
			OSString += " Windows (not released)";
		}
		else if ( osvi.dwMajorVersion == 6 )
		{
			if ( osvi.dwMinorVersion == 2 )
			{
				if( osvi.wProductType == VER_NT_WORKSTATION )
					OSString += " Windows 8";
				else
					OSString += " Windows Server 2012";
			}
			else if ( osvi.dwMinorVersion == 1 )
			{
				if( osvi.wProductType == VER_NT_WORKSTATION )
					OSString += " Windows 7";
				else
					OSString += " Windows Server 2008 R2";
			}
			else if ( osvi.dwMinorVersion == 0 )
			{
				if( osvi.wProductType == VER_NT_WORKSTATION )
					OSString += " Windows Vista";
				else
					OSString += " Windows Server 2008";
			}
			else
			{
				OSString += " Windows (not released)";
			}

			pGPI = (PGPI) GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetProductInfo");

			DWORD dwType;
			pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.wServicePackMajor, osvi.wServicePackMinor, &dwType);

			// Test for the specific product family.
			switch( dwType )
			{
#ifdef PRODUCT_UNLICENSED
			case PRODUCT_UNLICENSED:
				OSString += " Unlicensed";
				break;
#endif
#ifdef PRODUCT_ULTIMATE
			case PRODUCT_ULTIMATE:
				OSString += " Ultimate Edition";
				break;
#endif
#ifdef PRODUCT_HOME_BASIC
			case PRODUCT_HOME_BASIC:
				OSString += " Home Basic Edition";
				break;
#endif
#ifdef PRODUCT_HOME_PREMIUM
			case PRODUCT_HOME_PREMIUM:
				OSString += " Home Premium Edition";
				break;
#endif
#ifdef PRODUCT_ENTERPRISE
			case PRODUCT_ENTERPRISE:
				OSString += " Enterprise Edition";
				break;
#endif
#ifdef PRODUCT_HOME_BASIC_N
			case PRODUCT_HOME_BASIC_N:
				OSString += " Home Basic N Edition";
				break;
#endif
#ifdef PRODUCT_BUSINESS
			case PRODUCT_BUSINESS:
				OSString += " Business Edition";
				break;
#endif
#ifdef PRODUCT_STANDARD_SERVER
			case PRODUCT_STANDARD_SERVER:
				OSString += " Standard Edition";
				break;
#endif
#ifdef PRODUCT_DATACENTER_SERVER
			case PRODUCT_DATACENTER_SERVER:
				OSString += " Datacenter Edition";
				break;
#endif
#ifdef PRODUCT_SMALLBUSINESS_SERVER
			case PRODUCT_SMALLBUSINESS_SERVER:
				OSString += " Small Business Server";
				break;
#endif
#ifdef PRODUCT_ENTERPRISE_SERVER
			case PRODUCT_ENTERPRISE_SERVER:
				OSString += " Enterprise Edition";
				break;
#endif
#ifdef PRODUCT_STARTER
			case PRODUCT_STARTER:
				OSString += " Starter Edition";
				break;
#endif
#ifdef PRODUCT_DATACENTER_SERVER_CORE
			case PRODUCT_DATACENTER_SERVER_CORE:
				OSString += " Datacenter Edition (core installation)";
				break;
#endif
#ifdef PRODUCT_STANDARD_SERVER_CORE
			case PRODUCT_STANDARD_SERVER_CORE:
				OSString += " Standard Edition (core installation)";
				break;
#endif
#ifdef PRODUCT_ENTERPRISE_SERVER_CORE
			case PRODUCT_ENTERPRISE_SERVER_CORE:
				OSString += " Enterprise Edition (core installation)";
				break;
#endif
#ifdef PRODUCT_ENTERPRISE_SERVER_IA64
			case PRODUCT_ENTERPRISE_SERVER_IA64:
				OSString += " Enterprise Edition for Itanium-based Systems";
				break;
#endif
#ifdef PRODUCT_BUSINESS_N
			case PRODUCT_BUSINESS_N:
				OSString += " Business N Edition";
				break;
#endif
#ifdef PRODUCT_WEB_SERVER
			case PRODUCT_WEB_SERVER:
				OSString += " Web Server Edition";
				break;
#endif
#ifdef PRODUCT_CLUSTER_SERVER
			case PRODUCT_CLUSTER_SERVER:
				OSString += " Cluster Server Edition";
				break;
#endif
#ifdef PRODUCT_HOME_SERVER
			case PRODUCT_HOME_SERVER:
				OSString += " Home Server Edition";
				break;
#endif
#ifdef PRODUCT_STORAGE_EXPRESS_SERVER
			case PRODUCT_STORAGE_EXPRESS_SERVER:
				OSString += " Storage Server Express Edition";
				break;
#endif
#ifdef PRODUCT_STORAGE_STANDARD_SERVER
			case PRODUCT_STORAGE_STANDARD_SERVER:
				OSString += " Storage Server Standard Edition";
				break;
#endif
#ifdef PRODUCT_STORAGE_WORKGROUP_SERVER
			case PRODUCT_STORAGE_WORKGROUP_SERVER:
				OSString += " Storage Server Workgroup Edition";
				break;
#endif
#ifdef PRODUCT_STORAGE_ENTERPRISE_SERVER
			case PRODUCT_STORAGE_ENTERPRISE_SERVER:
				OSString += " Storage Server Enterprise Edition";
				break;
#endif
#ifdef PRODUCT_SERVER_FOR_SMALLBUSINESS
			case PRODUCT_SERVER_FOR_SMALLBUSINESS:
				OSString += " Essential Server Solutions Edition";
				break;
#endif
#ifdef PRODUCT_SMALLBUSINESS_SERVER_PREMIUM
			case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				OSString += " Small Business Server Premium Edition";
				break;
#endif
#ifdef PRODUCT_HOME_PREMIUM_N
			case PRODUCT_HOME_PREMIUM_N:
				OSString += " Home Premium N Edition";
				break;
#endif
#ifdef PRODUCT_ENTERPRISE_N
			case PRODUCT_ENTERPRISE_N:
				OSString += " Enterprise N Edition";
				break;
#endif
#ifdef PRODUCT_ULTIMATE_N
			case PRODUCT_ULTIMATE_N:
				OSString += " Ultimate N Edition";
				break;
#endif
#ifdef PRODUCT_WEB_SERVER_CORE
			case PRODUCT_WEB_SERVER_CORE:
				OSString += " Web Server Edition (core installation)";
				break;
#endif
#ifdef PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT
			case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
				OSString += " Essential Business Server Management Server Edition";
				break;
#endif
#ifdef PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY
			case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
				OSString += " Essential Business Server Security Server Edition";
				break;
#endif
#ifdef PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING
			case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
				OSString += " Essential Business Server Messaging Server Edition";
				break;
#endif
#ifdef PRODUCT_SMALLBUSINESS_SERVER_PRIME
			case PRODUCT_SMALLBUSINESS_SERVER_PRIME:
				OSString += " Small Business Server Prime Edition";
				break;
#endif
#ifdef PRODUCT_HOME_PREMIUM_SERVER
			case PRODUCT_HOME_PREMIUM_SERVER:
				OSString += " Home Premium Server Edition";
				break;
#endif
#ifdef PRODUCT_SERVER_FOR_SMALLBUSINESS_V
			case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
				OSString += " Essential Server Solutions without Hyper-V Edition";
				break;
#endif
#ifdef PRODUCT_STANDARD_SERVER_V
			case PRODUCT_STANDARD_SERVER_V:
				OSString += " Standard without Hyper-V Edition";
				break;
#endif
#ifdef PRODUCT_DATACENTER_SERVER_V
			case PRODUCT_DATACENTER_SERVER_V:
				OSString += " Datacenter without Hyper-V Edition";
				break;
#endif
#ifdef PRODUCT_ENTERPRISE_SERVER_V
			case PRODUCT_ENTERPRISE_SERVER_V:
				OSString += " Enterprise without Hyper-V Edition";
				break;
#endif
#ifdef PRODUCT_DATACENTER_SERVER_CORE_V
			case PRODUCT_DATACENTER_SERVER_CORE_V:
				OSString += " Datacenter without Hyper-V Edition (core installation)";
				break;
#endif
#ifdef PRODUCT_STANDARD_SERVER_CORE_V
			case PRODUCT_STANDARD_SERVER_CORE_V:
				OSString += " Standard without Hyper-V Edition (core installation)";
				break;
#endif
#ifdef PRODUCT_ENTERPRISE_SERVER_CORE_V
			case PRODUCT_ENTERPRISE_SERVER_CORE_V:
				OSString += " Enterprise without Hyper-V Edition (core installation)";
				break;
#endif
#ifdef PRODUCT_HYPERV
			case PRODUCT_HYPERV:
				OSString += " Hyper-V Server Edition";
				break;
#endif
			default:
				OSString += toString(" Unknown Edition (0x%04x)", dwType);
			}

			if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
				OSString += " 64-bit";
			else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
				OSString += " 32-bit";
		}
		else if ( osvi.dwMajorVersion == 5 )
		{
			if ( osvi.dwMinorVersion == 2 )
			{
				if( GetSystemMetrics(89 /* SM_SERVERR2 */) )
					OSString += " Windows Server 2003 R2";
#ifdef VER_SUITE_STORAGE_SERVER
				else if ( osvi.wSuiteMask == VER_SUITE_STORAGE_SERVER )
					OSString += " Windows Storage Server 2003";
#endif
#ifdef VER_SUITE_WH_SERVER
				else if ( osvi.wSuiteMask == VER_SUITE_WH_SERVER )
					OSString += " Windows Home Server";
#endif
				else if( osvi.wProductType == VER_NT_WORKSTATION &&
					si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
					OSString += " Windows XP Professional x64 Edition";
				else
					OSString += " Windows Server 2003";

				// Test for the server type.
				if ( osvi.wProductType != VER_NT_WORKSTATION )
				{
					if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
					{
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
							OSString += " Datacenter Edition for Itanium-based Systems";
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							OSString += " Enterprise Edition for Itanium-based Systems";
					}

					else if ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 )
					{
						if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
							OSString += " Datacenter x64 Edition";
						else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							OSString += " Enterprise x64 Edition";
						else
							OSString += " Standard x64 Edition";
					}

					else
					{
						if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
							OSString += " Enterprise Edition";
#ifdef VER_SUITE_DATACENTER
						else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
							OSString += " Datacenter Edition";
#endif
#ifdef VER_SUITE_BLADE
						else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
							OSString += " Web Edition";
#endif
#ifdef VER_SUITE_COMPUTE_SERVER
						else if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
							OSString += " Compute Cluster Edition";
#endif
						else
							OSString += " Standard Edition";
					}
				}
			}
			else if ( osvi.dwMinorVersion == 1 )
			{
				OSString += " Windows XP";
				if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
					OSString += " Home Edition";
				else
					OSString += " Professional";
			}
			else if ( osvi.dwMinorVersion == 0 )
			{
				OSString += " Windows 2000";

				if ( osvi.wProductType == VER_NT_WORKSTATION )
				{
					OSString += " Professional";
				}
				else
				{
					if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
						OSString += " Datacenter Server";
					else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						OSString += " Advanced Server";
					else
						OSString += " Server";
				}
			}
			else
			{
				OSString += " Unknown Windows";
			}
		}
		else if ( osvi.dwMajorVersion <= 4 )
		{
			OSString += " Windows NT";

			// Test for specific product on Windows NT 4.0 SP6 and later.
			if( bOsVersionInfoEx )
			{
				// Test for the workstation type.
				if ( osvi.wProductType == VER_NT_WORKSTATION )
				{
					if( osvi.dwMajorVersion == 4 )
						OSString += " Workstation 4.0";
					else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
						OSString += " Home Edition";
					else
						OSString += " Professional";
				}

				// Test for the server type.
				else if ( osvi.wProductType == VER_NT_SERVER )
				{
					if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
						OSString += " Server 4.0 Enterprise Edition";
					else
						OSString += " Server 4.0";
				}
			}
			else  // Test for specific product on Windows NT 4.0 SP5 and earlier
			{
				HKEY hKey;
				TCHAR szProductType[BUFSIZE];
				DWORD dwBufLen=BUFSIZE;
				LONG lRet;

				lRet = RegOpenKeyExA( HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey );
				if( lRet != ERROR_SUCCESS )
					return OSString + " Can't RegOpenKeyEx";

				lRet = RegQueryValueExA( hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);
				if( (lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE) )
					return OSString + " Can't ReQueryValueEx";

				RegCloseKey( hKey );

				if ( lstrcmpi( _T("WINNT"), szProductType) == 0 )
					OSString += " Workstation";
				if ( lstrcmpi( _T("LANMANNT"), szProductType) == 0 )
					OSString += " Server";
				if ( lstrcmpi( _T("SERVERNT"), szProductType) == 0 )
					OSString += " Advanced Server";
			}
		}

		std::string servicePack;

		if( osvi.dwMajorVersion == 4 && lstrcmpi( osvi.szCSDVersion, _T("Service Pack 6") ) == 0 )
		{
			HKEY hKey;
			LONG lRet;

			// Test for SP6 versus SP6a.
			lRet = RegOpenKeyExA( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009", 0, KEY_QUERY_VALUE, &hKey );
			if( lRet == ERROR_SUCCESS )
				servicePack = "Service Pack 6a";
			else // Windows NT 4.0 prior to SP6a
			{
				servicePack = osvi.szCSDVersion;
			}

			RegCloseKey( hKey );
		}
		else // Windows NT 3.51 and earlier or Windows 2000 and later
		{
			servicePack = osvi.szCSDVersion;
		}

		// Include service pack (if any)
		if (!servicePack.empty()) OSString += " " + servicePack;

		// Include build number
		OSString += toString(" (Build %d)", osvi.dwBuildNumber & 0xFFFF);
	}
	else if ( VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId )
	{
		OSString = "Microsoft";

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			OSString += " Windows 95";
			if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
				OSString += " OSR2";
		}
		else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			OSString += " Windows 98";
			if ( osvi.szCSDVersion[1] == 'A' )
				OSString += " SE";
		}
		else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
		{
			OSString += " Windows Millennium Edition";
		}
		else
			OSString += " Windows 9x";
	}
	else if ( VER_PLATFORM_WIN32s == osvi.dwPlatformId )
	{
		OSString = toString("Microsoft Windows %d.%d + Win32s", osvi.dwMajorVersion, osvi.dwMinorVersion);
	}
	else
	{
		OSString = toString("Microsoft Windows %d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion);
	}

	OSString += toString( " (%d.%d %d)", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber & 0xFFFF);

#elif defined NL_OS_MAC

	OSString = getsysctlstr("kern.version");

#elif defined NL_OS_UNIX

	int fd = open("/proc/version", O_RDONLY);
	if (fd == -1)
	{
		nlwarning ("SI: Can't get OS from /proc/version: %s", strerror (errno));
	}
	else
	{
		char buffer[4096+1];
		int len = read(fd, buffer, sizeof(buffer)-1);
		close(fd);

		// remove the \n and set \0
		buffer[len-1] = '\0';

		OSString = buffer;
	}

#endif	// NL_OS_UNIX

	return OSString;
}

string CSystemInfo::getProc ()
{
	string ProcString = "Unknown";

#ifdef NL_OS_WINDOWS

	LONG result;
	char value[1024];
	DWORD valueSize;
	HKEY hKey;

	result = ::RegOpenKeyExA (HKEY_LOCAL_MACHINE, "Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey);
	if (result == ERROR_SUCCESS)
	{
		// get processor name
		valueSize = 1024;
		result = ::RegQueryValueEx (hKey, _T("ProcessorNameString"), NULL, NULL, (LPBYTE)value, &valueSize);
		if (result == ERROR_SUCCESS)
			ProcString = value;
		else
			ProcString = "UnknownProc";

		ProcString += " / ";

		// get processor identifier
		valueSize = 1024;
		result = ::RegQueryValueEx (hKey, _T("Identifier"), NULL, NULL, (LPBYTE)value, &valueSize);
		if (result == ERROR_SUCCESS)
			ProcString += value;
		else
			ProcString += "UnknownIdentifier";

		ProcString += " / ";

		// get processor vendor
		valueSize = 1024;
		result = ::RegQueryValueEx (hKey, _T("VendorIdentifier"), NULL, NULL, (LPBYTE)value, &valueSize);
		if (result == ERROR_SUCCESS)
			ProcString += value;
		else
			ProcString += "UnknownVendor";

		ProcString += " / ";

		// get processor frequency
		result = ::RegQueryValueEx (hKey, _T("~MHz"), NULL, NULL, (LPBYTE)value, &valueSize);
		if (result == ERROR_SUCCESS)
		{
			uint32 freq = *(int *)value;
			// discard the low value (not enough significant)
			freq /= 10;
			freq *= 10;
			ProcString += toString("%uMHz", freq);
		}
		else
			ProcString += "UnknownFreq";
	}

	// Make sure to close the reg key
	RegCloseKey (hKey);

	// count the number of processor (max 8, in case this code don't work well)
	uint	numProc= 1;
	for(uint i=1;i<8;i++)
	{
		string	tmp= string("Hardware\\Description\\System\\CentralProcessor\\") + toString(i);

		// try to open the key
		result = ::RegOpenKeyExA (HKEY_LOCAL_MACHINE, tmp.c_str(), 0, KEY_QUERY_VALUE, &hKey);
		// Make sure to close the reg key
		RegCloseKey (hKey);

		if(result == ERROR_SUCCESS)
			numProc++;
		else
			break;
	}
	ProcString += " / ";
	ProcString += toString(numProc) + " Processors found";

#elif defined NL_OS_MAC

	ProcString = getsysctlstr("machdep.cpu.brand_string");
	ProcString += " / ";
	ProcString += getsysctlstr("hw.machine");
	ProcString += " Family " + toString(getsysctlnum("machdep.cpu.family"));
	ProcString += " Model " + toString(getsysctlnum("machdep.cpu.model"));
	ProcString += " Stepping " + toString(getsysctlnum("machdep.cpu.stepping"));
	ProcString += " / ";
	ProcString += getsysctlstr("machdep.cpu.vendor");
	ProcString += " / ";
	ProcString += toString(getsysctlnum64("hw.cpufrequency")/1000000)+"MHz";
	ProcString += " / ";
	ProcString += toString(getsysctlnum("hw.ncpu")) + " Processors found";

#elif defined NL_OS_UNIX

	uint processors = 0;
	if (fromString(getCpuInfo("processor"), processors)) ++processors;

	ProcString = getCpuInfo("model name");
	ProcString += " / ?";
	ProcString += " Family " + getCpuInfo("cpu family");
	ProcString += " Model " + getCpuInfo("model");
	ProcString += " Stepping " + getCpuInfo("stepping");
	ProcString += " / ";
	ProcString += getCpuInfo("vendor_id");
	ProcString += " / ";
	ProcString += getCpuInfo("cpu MHz")+"MHz";
	ProcString += " / ";
	ProcString += toString("%u Processors found", processors);

#endif

	// Remove beginning spaces
	ProcString = ProcString.substr (ProcString.find_first_not_of (" "));

	return ProcString;
}

uint64 CSystemInfo::getProcessorFrequency(bool quick)
{
	static uint64 freq = 0;
#ifdef	NL_CPU_INTEL
	static bool freqComputed = false;
	if (freqComputed) return freq;

	if (!quick)
	{
		TTicks bestNumTicks   = 0;
		uint64 bestNumCycles  = 0;
		uint64 numCycles;
		const uint numSamples = 5;
		const uint numLoops   = 50000000;

		volatile uint k; // prevent optimization for the loop
		for(uint l = 0; l < numSamples; ++l)
		{
			TTicks startTick = NLMISC::CTime::getPerformanceTime();
			uint64 startCycle = rdtsc();
			volatile uint dummy = 0;
			for(k = 0; k < numLoops; ++k)
			{
				++ dummy;
			}
			numCycles = rdtsc() - startCycle;
			TTicks numTicks = NLMISC::CTime::getPerformanceTime() - startTick;
			if (numTicks > bestNumTicks)
			{
				bestNumTicks  = numTicks;
				bestNumCycles = numCycles;
			}
		}
		freq = (uint64) ((double) bestNumCycles * 1 / CTime::ticksToSecond(bestNumTicks));
	}
	else
	{
		TTicks timeBefore = NLMISC::CTime::getPerformanceTime();
		uint64 tickBefore = rdtsc();
		nlSleep (100);
		TTicks timeAfter = NLMISC::CTime::getPerformanceTime();
		TTicks tickAfter = rdtsc();

		double timeDelta = CTime::ticksToSecond(timeAfter - timeBefore);
		TTicks tickDelta = tickAfter - tickBefore;

		freq = (uint64) ((double)tickDelta / timeDelta);
	}

	nlinfo ("SI: CSystemInfo: Processor frequency is %.0f MHz", (float)freq/1000000.0);
	freqComputed = true;
#endif // NL_CPU_INTEL
	return freq;
}

static bool DetectMMX()
{
	#ifdef NL_CPU_INTEL
		if (!CSystemInfo::hasCPUID()) return false; // cpuid not supported ...

		sint32 CPUInfo[4];
		nlcpuid(CPUInfo, 1);
		// check for bit 23 = MMX instruction set
		if (CPUInfo[3] & 0x800000) return true;
	#endif // NL_CPU_INTEL

	return false;
}


static bool DetectSSE()
{
	#ifdef NL_CPU_INTEL
		if (!CSystemInfo::hasCPUID()) return false; // cpuid not supported ...

		sint32 CPUInfo[4];
		nlcpuid(CPUInfo, 1);

		if (CPUInfo[3]  & 0x2000000)
		{
			// check OS support for SSE
			try
			{
				#ifdef NL_OS_WINDOWS
				#ifdef NL_NO_ASM
				unsigned int tmp = _mm_getcsr();
				nlunreferenced(tmp);
				#else
				__asm
				{
					xorps xmm0, xmm0  // Streaming SIMD Extension
				}
				#endif // NL_NO_ASM
				#elif NL_OS_UNIX
					__asm__ __volatile__ ("xorps %xmm0, %xmm0;");
				#endif // NL_OS_UNIX
			}
			catch(...)
			{
				return false;
			}

			// printf("sse detected\n");

			return true;
		}
	#endif // NL_CPU_INTEL

	return false;
}

bool CSystemInfo::_HaveMMX = DetectMMX ();
bool CSystemInfo::_HaveSSE = DetectSSE ();

bool CSystemInfo::hasCPUID ()
{
	#ifdef NL_CPU_INTEL
		 uint32 result = 0;
		#ifdef NL_OS_WINDOWS
		#ifdef NL_NO_ASM
			sint32 CPUInfo[4] = {-1};
			nlcpuid(CPUInfo, 0);
			if (CPUInfo[3] != -1) result = 1;
		#else
		 __asm
		 {
			 pushad
			 pushfd
			 //	 If ID bit of EFLAGS can change, then cpuid is available
			 pushfd
			 pop  eax					// Get EFLAG
			 mov  ecx,eax
			 xor  eax,0x200000			// Flip ID bit
			 push eax
			 popfd						// Write EFLAGS
			 pushfd
			 pop  eax					// read back EFLAG
			 xor  eax,ecx
			 je   noCpuid				// no flip -> no CPUID instr.

			 popfd						// restore state
			 popad
			 mov  result, 1
			 jmp  CPUIDPresent

			noCpuid:
			 popfd					    // restore state
			 popad
			 mov result, 0
			CPUIDPresent:
		 }
		#endif // NL_NO_ASM
		#elif NL_OS_UNIX // NL_OS_WINDOWS
			__asm__ __volatile__ (
				/* Save Register */
				"pushl  %%ebp;"
				"pushl  %%ebx;"
				"pushl  %%edx;"

				/* Check if this CPU supports cpuid */
				"pushf;"
				"pushf;"
				"popl   %%eax;"
				"movl   %%eax, %%ebx;"
				"xorl   $(1 << 21), %%eax;"	// CPUID bit
				"pushl  %%eax;"
				"popf;"
				"pushf;"
				"popl   %%eax;"
				"popf;"                  	// Restore flags
				"xorl   %%ebx, %%eax;"
				"jz     NoCPUID;"
				"movl   $1, %0;"
				"jmp    CPUID;"

			"NoCPUID:;"
				"movl   $0, %0;"
              		"CPUID:;"
				"popl   %%edx;"
				"popl   %%ebx;"
				"popl   %%ebp;"

				:"=a"(result)
                	);
		#endif // NL_OS_UNIX
		return result == 1;
	#else
		return false;
	#endif
}


uint32 CSystemInfo::getCPUID()
{
#ifdef NL_CPU_INTEL
	if(hasCPUID())
	{
		uint32 result = 0;
		sint32 CPUInfo[4];
		nlcpuid(CPUInfo, 1);
		return CPUInfo[3];
	}
#endif // NL_CPU_INTEL

	return 0;
}

/*
 *	Note: Not used in NeL probably in Ryzom closed source. Not translated in AT&T asm, I don't understand the aim of this method
 *	      Returns true if the CPU has HT,  even if it is disabled. Maybe shoud count how many (virtual) core there is.
 */
bool CSystemInfo::hasHyperThreading()
{
#ifdef NL_OS_WINDOWS
	if(hasCPUID())
	{
		sint32 CPUInfo[4];

		// get vendor string from cpuid
		char vendor_id[32];
		memset(vendor_id, 0, sizeof(vendor_id));
		nlcpuid(CPUInfo, 0);
		memcpy(vendor_id, &CPUInfo[1], sizeof(sint32));
		memcpy(vendor_id+4, &CPUInfo[3], sizeof(sint32));
		memcpy(vendor_id+8, &CPUInfo[2], sizeof(sint32));

		// get cpuid flags
		nlcpuid(CPUInfo, 1);

		// pentium 4 or later processor?
		if ((((CPUInfo[0] & 0xf00) == 0xf00) || (CPUInfo[0] & 0xf00000)) &&
			strcmp(vendor_id, "GenuineIntel") == 0)
			return (CPUInfo[3] & 0x10000000)!=0; // Intel Processor Hyper-Threading
	}
#endif

	return false;
}

bool CSystemInfo::isNT()
{
#ifdef NL_OS_WINDOWS
	OSVERSIONINFO ver;
	ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&ver);
	return ver.dwPlatformId == VER_PLATFORM_WIN32_NT;
#else
	return false;
#endif
}

string CSystemInfo::availableHDSpace (const string &filename)
{
#ifdef NL_OS_UNIX
	string cmd = "df ";
	if(filename.empty())
		cmd += ".";
	else
		cmd += filename;
	cmd += " >/tmp/nelhdfs";
	sint error = system (cmd.c_str());
	if (error)
		nlwarning("'%s' failed with error code %d", cmd.c_str(), error);

	int fd = open("/tmp/nelhdfs", O_RDONLY);
	if (fd == -1)
	{
		return 0;
	}
	else
	{
		char buffer[4096+1];
		int len = read(fd, buffer, sizeof(buffer)-1);
		close(fd);
		buffer[len] = '\0';

		vector<string> splitted;
		explode(string(buffer), string("\n"), splitted, true);

		if(splitted.size() < 2)
			return "NoInfo";

		vector<string> sline;
		explode(splitted[1], string(" "), sline, true);

		if(sline.size() < 5)
			return splitted[1];

		string space = sline[3] + "000";
		return bytesToHumanReadable(space);
	}
#else
	nlunreferenced(filename);
	return "NoInfo";
#endif
}

uint64 CSystemInfo::availablePhysicalMemory ()
{
#ifdef NL_OS_WINDOWS
	MEMORYSTATUS ms;
	GlobalMemoryStatus (&ms);
	return uint64(ms.dwAvailPhys);
#elif defined NL_OS_MAC
	return uint64(getsysctlnum64("hw.usermem"));
#elif defined NL_OS_UNIX
	return getSystemMemory("MemFree:")+getSystemMemory("Buffers:")+getSystemMemory("Cached:");
#else
	return 0;
#endif
}

uint64 CSystemInfo::totalPhysicalMemory ()
{
#ifdef NL_OS_WINDOWS
	MEMORYSTATUS ms;
	GlobalMemoryStatus (&ms);
	return uint64(ms.dwTotalPhys);
#elif defined NL_OS_MAC
	return uint64(getsysctlnum64("hw.physmem"));
#elif defined NL_OS_UNIX
	return getSystemMemory("MemTotal:");
#else
	return 0;
#endif
}

#ifndef NL_OS_WINDOWS
static inline char *skipWS(const char *p)
{
    while (isspace(*p)) p++;
    return (char *)p;
}

static inline char *skipToken(const char *p)
{
    while (isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    return (char *)p;
}
#endif

uint64 CSystemInfo::getAllocatedSystemMemory ()
{
	uint64 systemMemory = 0;
#ifdef NL_OS_WINDOWS
	// Get system memory information
	HANDLE hHeap[100];
	DWORD heapCount = GetProcessHeaps (100, hHeap);

	uint heap;
	for (heap = 0; heap < heapCount; heap++)
	{
		PROCESS_HEAP_ENTRY entry;
		entry.lpData = NULL;
		while (HeapWalk (hHeap[heap], &entry))
		{
			if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
			{
				systemMemory += entry.cbData + entry.cbOverhead;
			}
		}
	}

#elif defined NL_OS_UNIX

	int fd = open("/proc/self/stat", O_RDONLY);
	if (fd == -1)
	{
		nlwarning ("HA: Can't get OS from /proc/self/stat: %s", strerror (errno));
	}
	else
	{
		char buffer[4096], *p;
		int len = read(fd, buffer, sizeof(buffer)-1);
		close(fd);

		buffer[len] = '\0';

		p = buffer;
		p = strchr(p, ')')+1;			/* skip pid */
		p = skipWS(p);
		p++;

		p = skipToken(p);				/* skip ppid */
		p = skipToken(p);				/* skip pgrp */
		p = skipToken(p);				/* skip session */
		p = skipToken(p);				/* skip tty */
		p = skipToken(p);				/* skip tty pgrp */
		p = skipToken(p);				/* skip flags */
		p = skipToken(p);				/* skip min flt */
		p = skipToken(p);				/* skip cmin flt */
		p = skipToken(p);				/* skip maj flt */
		p = skipToken(p);				/* skip cmaj flt */
		p = skipToken(p);				/* utime */
		p = skipToken(p);				/* stime */
		p = skipToken(p);				/* skip cutime */
		p = skipToken(p);				/* skip cstime */
		p = skipToken(p);				/* priority */
		p = skipToken(p);				/* nice */
		p = skipToken(p);				/* skip timeout */
		p = skipToken(p);				/* skip it_real_val */
		p = skipToken(p);				/* skip start_time */

		systemMemory = strtoul(p, &p, 10);	/* vsize in bytes */
	}

#endif // NL_OS_WINDOWS
	return systemMemory;
}


NLMISC_CATEGORISED_DYNVARIABLE(nel, string, AvailableHDSpace, "Hard drive space left in bytes")
{
	// ace: it's a little bit tricky, if you don't understand how it works, don't touch!
	static string location;
	if (get)
	{
		*pointer = (CSystemInfo::availableHDSpace(location));
		location = "";
	}
	else
	{
		location = *pointer;
	}
}

NLMISC_CATEGORISED_DYNVARIABLE(nel, string, AvailablePhysicalMemory, "Physical memory available on this computer in bytes")
{
	if (get) *pointer = bytesToHumanReadable(CSystemInfo::availablePhysicalMemory ());
}

NLMISC_CATEGORISED_DYNVARIABLE(nel, string, TotalPhysicalMemory, "Total physical memory on this computer in bytes")
{
	if (get) *pointer = bytesToHumanReadable(CSystemInfo::totalPhysicalMemory ());
}

NLMISC_CATEGORISED_DYNVARIABLE(nel, string, ProcessUsedMemory, "Memory used by this process in bytes")
{
	if (get) *pointer = bytesToHumanReadable(CSystemInfo::getAllocatedSystemMemory ());
}

NLMISC_CATEGORISED_DYNVARIABLE(nel, string, OS, "OS used")
{
	if (get) *pointer = CSystemInfo::getOS();
}

#ifdef NL_OS_WINDOWS
struct DISPLAY_DEVICE_EX
{
    DWORD  cb;
    CHAR  DeviceName[32];
    CHAR  DeviceString[128];
    DWORD  StateFlags;
    CHAR  DeviceID[128];
    CHAR  DeviceKey[128];
};
#endif // NL_OS_WINDOWS

bool CSystemInfo::getVideoInfo (std::string &deviceName, uint64 &driverVersion)
{
#ifdef NL_OS_WINDOWS
	/* Get the device name with EnumDisplayDevices (doesn't work under win95).
	 * Look for driver information for this device in the registry
	 *
	 * Follow the recommendations in the news group comp.os.ms-windows.programmer.nt.kernel-mode : "Get Video Driver ... Need Version"
	 */

	HMODULE hm = GetModuleHandle(TEXT("USER32"));
	if (hm)
	{
		BOOL (WINAPI* EnumDisplayDevices)(LPCTSTR lpDevice, DWORD iDevNum, PDISPLAY_DEVICE lpDisplayDevice, DWORD dwFlags) = NULL;
		*(FARPROC*)&EnumDisplayDevices = GetProcAddress(hm, "EnumDisplayDevicesA");
		if (EnumDisplayDevices)
		{
			DISPLAY_DEVICE_EX DisplayDevice;
			uint device = 0;
			DisplayDevice.cb = sizeof (DISPLAY_DEVICE_EX);
			bool found = false;
			while (EnumDisplayDevices(NULL, device, (DISPLAY_DEVICE*)&DisplayDevice, 0))
			{
				// Main board ?
				if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) &&
					(DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) &&
					((DisplayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) == 0) &&
					(DisplayDevice.DeviceKey[0] != 0))
				{
					found = true;

					// The device name
					deviceName = DisplayDevice.DeviceString;

					string keyPath = DisplayDevice.DeviceKey;
					string keyName;

					// Get the window version
					OSVERSIONINFO ver;
					ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
					GetVersionEx(&ver);
					bool atleastNT4 = (ver.dwMajorVersion  > 3) && (ver.dwPlatformId == VER_PLATFORM_WIN32_NT);
					bool winXP = ((ver.dwMajorVersion == 5) && (ver.dwMinorVersion == 1)) || (ver.dwMajorVersion > 5);

					// * Get the registry entry for driver

					// OSversion >= osWin2k
					if (atleastNT4)
					{
						if (winXP)
						{
							string::size_type pos = keyPath.rfind ('\\');
							if (pos != string::npos)
								keyPath = keyPath.substr (0, pos+1);
							keyPath += "Video";
							keyName = "Service";
						}
						else
						{
							string::size_type pos = toLower(keyPath).find ("\\device");
							if (pos != string::npos)
								keyPath = keyPath.substr (0, pos+1);
							keyName = "ImagePath";
						}
					}
					else // Win 9x
					{
						keyPath += "\\default";
						keyName = "drv";
					}

					// Format the key path
					if (toLower(keyPath).find ("\\registry\\machine") == 0)
					{
						keyPath = "HKEY_LOCAL_MACHINE" + keyPath.substr (strlen ("\\registry\\machine"));
					}

					// Get the root key
					static const char *rootKeys[]=
					{
						"HKEY_CLASSES_ROOT\\",
						"HKEY_CURRENT_CONFIG\\",
						"HKEY_CURRENT_USER\\",
						"HKEY_LOCAL_MACHINE\\",
						"HKEY_USERS\\",
						"HKEY_PERFORMANCE_DATA\\",
						"HKEY_DYN_DATA\\"
					};
					static const HKEY rootKeysH[]=
					{
						HKEY_CLASSES_ROOT,
						HKEY_CURRENT_CONFIG,
						HKEY_CURRENT_USER,
						HKEY_LOCAL_MACHINE,
						HKEY_USERS,
						HKEY_PERFORMANCE_DATA,
						HKEY_DYN_DATA,
					};
					uint i;
					HKEY keyRoot = HKEY_LOCAL_MACHINE;
					for (i=0; i<sizeof(rootKeysH)/sizeof(HKEY); i++)
					{
						if (toUpper(keyPath).find (rootKeys[i]) == 0)
						{
							keyPath = keyPath.substr (strlen (rootKeys[i]));
							keyRoot = rootKeysH[i];
							break;
						}
					}

					// * Read the registry
					HKEY baseKey;
					if (RegOpenKeyExA(keyRoot, keyPath.c_str(), 0, KEY_READ, &baseKey) == ERROR_SUCCESS)
					{
						DWORD valueType;
						char value[512];
						DWORD size = 512;
						if (RegQueryValueExA(baseKey, keyName.c_str(), NULL, &valueType, (unsigned char *)value, &size) == ERROR_SUCCESS)
						{
							// Null ?
							if (value[0] != 0)
							{
								bool ok = !winXP;
								if (winXP)
								{
									// In Windows'XP we got service name -> not real driver name, so
									string xpKey = string ("System\\CurrentControlSet\\Services\\")+value;
									if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, xpKey.c_str(), 0, KEY_READ, &baseKey) == ERROR_SUCCESS)
									{
										size = 512;
										if (RegQueryValueExA(baseKey, "ImagePath", NULL, &valueType, (unsigned char *)value, &size) == ERROR_SUCCESS)
										{
											if (value[0] != 0)
											{
												ok = true;
											}
											else
												nlwarning ("CSystemInfo::getVideoInfo : empty value ImagePath in key %s", xpKey.c_str());
										}
										else
											nlwarning ("CSystemInfo::getVideoInfo : can't query ImagePath in key %s", xpKey.c_str());
									}
									else
										nlwarning ("CSystemInfo::getVideoInfo : can't open key %s", xpKey.c_str());
								}

								// Version dll link
								HMODULE hmVersion = LoadLibrary (_T("version"));
								if (hmVersion)
								{
									BOOL (WINAPI* _GetFileVersionInfo)(LPTSTR, DWORD, DWORD, LPVOID) = NULL;
									DWORD (WINAPI* _GetFileVersionInfoSize)(LPTSTR, LPDWORD) = NULL;
									BOOL (WINAPI* _VerQueryValue)(const LPVOID, LPTSTR, LPVOID*, PUINT) = NULL;
									*(FARPROC*)&_GetFileVersionInfo = GetProcAddress(hmVersion, "GetFileVersionInfoA");
									*(FARPROC*)&_GetFileVersionInfoSize = GetProcAddress(hmVersion, "GetFileVersionInfoSizeA");
									*(FARPROC*)&_VerQueryValue = GetProcAddress(hmVersion, "VerQueryValueA");
									if (_VerQueryValue && _GetFileVersionInfoSize && _GetFileVersionInfo)
									{
										// value got the path to the driver
										string driverName = value;
										if (atleastNT4)
										{
											nlverify (GetWindowsDirectoryA(value, 512) != 0);
										}
										else
										{
											nlverify (GetSystemDirectoryA(value, 512) != 0);
										}
										driverName = string (value) + "\\" + driverName;

										DWORD dwHandle;
										DWORD size = _GetFileVersionInfoSize ((char*)driverName.c_str(), &dwHandle);
										if (size)
										{
											vector<uint8> buffer;
											buffer.resize (size);
											if (_GetFileVersionInfo((char*)driverName.c_str(), dwHandle, size, &buffer[0]))
											{
												VS_FIXEDFILEINFO *info;
												UINT len;
												char bslash[] = { '\\', 0x00 };
												if (_VerQueryValue(&buffer[0], bslash, (VOID**)&info, &len))
												{
													driverVersion = (((uint64)info->dwFileVersionMS)<<32)|info->dwFileVersionLS;
													return true;
												}
												else
													nlwarning ("CSystemInfo::getVideoInfo : VerQueryValue fails (%s)", driverName.c_str());
											}
											else
												nlwarning ("CSystemInfo::getVideoInfo : GetFileVersionInfo fails (%s)", driverName.c_str());
										}
										else
											nlwarning ("CSystemInfo::getVideoInfo : GetFileVersionInfoSize == 0 (%s)", driverName.c_str());
									}
									else
										nlwarning ("CSystemInfo::getVideoInfo : No VerQuery, GetFileVersionInfoSize, GetFileVersionInfo functions");
								}
								else
									nlwarning ("CSystemInfo::getVideoInfo : No version dll");
							}
							else
								nlwarning ("CSystemInfo::getVideoInfo : empty value %s in key %s", keyName.c_str(), keyPath.c_str());
						}
						else
							nlwarning ("CSystemInfo::getVideoInfo : can't query value %s in key %s", keyName.c_str(), keyPath.c_str());
					}
					else
						nlwarning ("CSystemInfo::getVideoInfo : can't open key %s", keyPath.c_str());
				}
				device++;
			}
			if (!found)
				nlwarning ("CSystemInfo::getVideoInfo : No primary display device found");
		}
		else
			nlwarning ("CSystemInfo::getVideoInfo : No EnumDisplayDevices function");
	}
	else
		nlwarning ("CSystemInfo::getVideoInfo : No user32 dll");

#endif // NL_OS_WINDOWS

	// Fails
	return false;
}

uint64 CSystemInfo::virtualMemory ()
{
#ifdef NL_OS_WINDOWS
	MEMORYSTATUS ms;
	GlobalMemoryStatus (&ms);
	return uint64(ms.dwTotalVirtual - ms.dwAvailVirtual);
#else
	return 0;
#endif
}

} // NLMISC
