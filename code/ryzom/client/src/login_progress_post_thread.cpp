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
#include "login_progress_post_thread.h"
#include "game_share/login_registry.h"
#include "nel/misc/system_info.h"


using namespace NLMISC;




///////////////////////
// Utility functions //
///////////////////////


//===========================================================================================
static std::string uint64ToHex(uint64 size)
{
	char data[256];
	sprintf(data, "%"NL_I64"X", size);
	return std::string(data);
}

//===========================================================================================
static std::string sizeToHumanStd(uint64 size)
{
	double total = (double) size;
	std::string unit;
	if (total >= 1024.0*1024.0*1024.0)
	{
		total /= 1024.0*1024*1024;
		unit = "GB";
	}
	else if (total >= 1024*1024.0)
	{
		total /= 1024.0*1024;
		unit = "MB";
	}
	else if (total >= 1024.0)
	{
		total = int(total/1024.0);
		unit = "KB";
	}
	else
	{
		unit = "B";
	}
	static char buffer[256];
	if ( total > 100.0)
	{
		sprintf(buffer, "%0.0f", (double)total);
	}
	else if (total > 10.0)
	{
		sprintf(buffer, "%0.1f", (double)total);
	}
	else
	{
		sprintf(buffer, "%0.2f", (double)total);
	}
	std::string ret(buffer);
	ret+=unit;

	return ret;
}

//===========================================================================================
static std::string getVideoInfoDeviceName()
{
	uint64 version;
	std::string ret = "";
	bool ok = CSystemInfo::getVideoInfo(ret, version);
	if (ok)
	{
		return ret.c_str();
	}
	return "";
}

//===========================================================================================
static uint64 getVideoDriverVersion()
{
	static std::string ret;
	uint64 version=0;
	ret = "";
	bool ok = CSystemInfo::getVideoInfo(ret, version);
	if (ok)
	{
		return version;
	}
	return 0;
}

//===========================================================================================
static std::string extractToken(const std::string& res, const std::string& token)
{
	std::string ret("");
	std::string tokenBegin = std::string("<") + token + ">";
	std::string::size_type tokenBeginLen = tokenBegin.size();
	std::string tokenEnd = std::string("</") + token + ">";
	std::string::size_type tokenEndLen = tokenBegin.size();

	std::string::size_type begin = res.find(tokenBegin);
	std::string::size_type end = begin != std::string::npos && res.size() > tokenBeginLen + tokenEndLen ? res.find(tokenEnd, begin+tokenBeginLen) : std::string::npos;
	if (begin != std::string::npos && end != std::string::npos)
	{
		ret = res.substr(begin+tokenBeginLen, end - (begin+tokenBeginLen));
	}
	return ret;

}

/////////////////////////
// login progress task //
/////////////////////////


typedef std::deque<CLoginStep> TStepQueue;

class CLoginProgressTask : public NLMISC::IRunnable
{
public:
	std::string					StartupHost;
	std::string					StartupPage;
	CSynchronized<TStepQueue>	StepQueue;
	bool						StopWanted;
public:
	//============================================================================================
	CLoginProgressTask() : StepQueue("")
	{
		StopWanted = false;
	}

	//============================================================================================
	void clearQueue()
	{
		CSynchronized<TStepQueue>::CAccessor access(&StepQueue);
		access.value().clear();
	}

	//============================================================================================
	virtual void run()
	{
		try
		{
			// init and send hardware info if first time
			std::string productInstallId = CLoginRegistry::getProductInstallId();
			if (productInstallId.empty())
			{
				throw NLMISC::Exception("Couldn't retrieve product install id");
			}
			std::string res = sendMsg(std::string("login") + "&install_id=" + productInstallId);
			std::string userId = extractToken(res, "user_id");
			if (userId.empty())
			{
				res = sendMsg(std::string("init") + "&install_id=" + productInstallId + buildSysInfoString());
				userId = extractToken(res, "user_id");
			}
			if (userId.empty())
			{
				throw NLMISC::Exception("Couldn't retrieve user id");
			}
			// from now, wait each step from the client
			for (;;)
			{
				CLoginStep loginStep;
				bool newStep = false;
				{
					CSynchronized<TStepQueue>::CAccessor access(&StepQueue);
					if (!access.value().empty())
					{
						loginStep = access.value().front();
						access.value().pop_front();
						newStep = true;
					}
					if (newStep)
					{
						if (loginStep.Step == (uint)LoginStep_Stop)
						{
							break;
						}
						uint currentStep = CLoginRegistry::getLoginStep();
						// during install program some messages can be launch multiple times (eg "install_update&percent=17" that indicates that 17 percent of the installation is already done)
						bool isInInstallation = InstallStep_StartDownload <= loginStep.Step && loginStep.Step <=  InstallStep_StopInstall;
						bool hasBeenInstalled = currentStep > InstallStep_StopInstall;
						if ((isInInstallation&&!hasBeenInstalled) || (loginStep.Step > CLoginRegistry::getLoginStep()) )
						{
							std::string ret= sendMsg(loginStep.PostString + "&user_id=" + userId);
							// If we reached this point, server has correctly received the step -> commit current progress in registry
							// Only send msg for player that have never installed the software
							CLoginRegistry::setLoginStep(loginStep.Step);
							// Async Ret enable the player to Know via pulling the return of the sendMsg
							if (loginStep.AsyncRet && loginStep.AsyncSent)
							{
								(*loginStep.AsyncRet) = ret;
								(*loginStep.AsyncSent) = true;
							}

						}
					}
				}
				nlSleep(1000);
				if (StopWanted) break;
			}
		}
		catch (const std::exception &e)
		{
			nlwarning(e.what());
		}
		catch(...)
		{
			clearQueue();
			throw;
		}
		clearQueue();
	}
	//============================================================================================
	std::string sendMsg(const std::string &msg)
	{
		CHttpClient httpClient;
		std::string ret;
		if (!httpClient.connect("http://" + StartupHost))
		{
			throw NLMISC::Exception("Can't connect to http server");
		}
		//std::string postString = "http://" + StartupHost + StartupPage +  "?cmd=log&msg=" + msg;
		//nlwarning("POST STRING = %s", postString.c_str());
		if (!httpClient.sendPost("http://" + StartupHost + StartupPage, "cmd=log&msg=" + msg))
		{
			throw NLMISC::Exception("Post failed");
		}
		if (!httpClient.receive(ret))
		{
			throw NLMISC::Exception("Receive failed");
		}
		httpClient.disconnect();
		return ret;
	}
	//============================================================================================
	std::string buildSysInfoString()
	{
		std::string sis;
		sis =  "&os="				+ CSystemInfo::getOS();
		sis += "&proc="				+ CSystemInfo::getProc();
		sis += "&memory="			+ sizeToHumanStd(CSystemInfo::totalPhysicalMemory());
		sis += "&video_card="		+ getVideoInfoDeviceName();
		sis += "&driver_version="	+ uint64ToHex(getVideoDriverVersion());
		/*
		sis =  "&os=win2k";
		sis += "&proc=p4";
		sis += "&memory=1024";
		sis += "&video_card=ati";
		sis += "&driver_version=1";
		*/
		return sis;
	}
};

//===========================================================================================
CLoginProgressPostThread::CLoginProgressPostThread()
{
	// Construct
	_Thread = NULL;
	_Task = NULL;
}

//===========================================================================================
CLoginProgressPostThread::~CLoginProgressPostThread()
{
	release();
}

//===========================================================================================
void CLoginProgressPostThread::init(const std::string &startupHost, const std::string &startupPage)
{
	nlassert(!_Thread);
	CLoginProgressTask *lpt = new CLoginProgressTask;
	// Its not the real Startup Host this values comes from the InstallStatsUrl value of the config file
	lpt->StartupHost = startupHost;
	lpt->StartupPage = startupPage;
	_Task = lpt;
	_Thread = IThread::create(lpt);
	if (!_Thread)
	{
		release();
		return;
	}
	_Thread->start();
}

//===========================================================================================
void CLoginProgressPostThread::release()
{
	if (_Task)
	{
		safe_cast<CLoginProgressTask *>(_Task)->StopWanted = true;
	}
	if (_Thread && _Thread->isRunning()) _Thread->wait();
	delete _Task;
	delete _Thread;
	_Task = NULL;
	_Thread = NULL;
}

//===========================================================================================
void CLoginProgressPostThread::step(const CLoginStep &ls)
{
	if (!_Task) return;
	if (!_Thread->isRunning()) return;
	CLoginProgressTask *lpt = safe_cast<CLoginProgressTask *>(_Task);
	{
		CSynchronized<TStepQueue>::CAccessor access(&lpt->StepQueue);
		access.value().push_back(ls);
	}
}

//===========================================================================================
void CLoginProgressPostThread::init(NLMISC::CConfigFile &configFile)
{
	std::string installStartupPage;
	std::string installStartupHost;
	static std::string httpStr = "http://";
	// The url where the stats system are has changed from
	// http://r2linux03:80/login2/client_install.php (using InstallStartupPage and StartupPage )
	// http://r2linux03:80/stats/stats.php (using InstallStatsUrl

	if (configFile.getVarPtr("InstallStatsUrl") )
	{

		static std::string httpStr = "http://";
		static std::string::size_type httpStrSize = httpStr.size();
		std::string tmp = configFile.getVarPtr("InstallStatsUrl")->asString(0);
		std::string::size_type it= tmp.find(httpStr);
		if (it != std::string::npos)
		{
			std::string::size_type hostPageSeparator = tmp.find("/", httpStrSize);
			if (hostPageSeparator != std::string::npos)
			{
				installStartupPage = tmp.substr(hostPageSeparator); //keep the leading slash
				installStartupHost = tmp.substr(httpStrSize, hostPageSeparator  - httpStrSize); // dont keep the last slah
			}
		}
	}
	else
	{
		nlwarning("Error the InstallStatsUrl is not in the client_default.cfg.");
	}
	init(installStartupHost, installStartupPage);
}




