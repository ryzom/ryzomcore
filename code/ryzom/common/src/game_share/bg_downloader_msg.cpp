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
#include "bg_downloader_msg.h"
#include "nel/misc/i18n.h"
#include "nel/misc/string_conversion.h"



namespace BGDownloader
{

const char *DownloaderMutexName = "RyzomBgDownloader";

ucstring getWrittenSize(uint32 nSize)
{
	float fSize = ((float)nSize)/(1024.0f*1024.0f);
	ucstring ucs = NLMISC::toString("%.1f", fSize) + " " + NLMISC::CI18N::get("uiMb");
	return ucs;
}


NL_BEGIN_STRING_CONVERSION_TABLE (TMsgType)
	NL_STRING_CONVERSION_TABLE_ENTRY(BGD_UpdateStatusString)
	NL_STRING_CONVERSION_TABLE_ENTRY(BGD_State)
	NL_STRING_CONVERSION_TABLE_ENTRY(BGD_Mode)
	NL_STRING_CONVERSION_TABLE_ENTRY(BGD_TaskResult)
	NL_STRING_CONVERSION_TABLE_ENTRY(BGD_DescFile)
	NL_STRING_CONVERSION_TABLE_ENTRY(BGD_Error)
	NL_STRING_CONVERSION_TABLE_ENTRY(BGD_Priority)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_GetMode)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_GetState)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_GetTaskResult)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_Stop)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_StartTask)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_SetMode)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_SetVerbose)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_Shutdown)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_GetDescFile)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_Show)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_Hide)
	NL_STRING_CONVERSION_TABLE_ENTRY(CL_SetPriority)
NL_END_STRING_CONVERSION_TABLE(TMsgType, MSGTYPEConversion, UnknownMessageType)


std::string toString(TMsgType msgType)
{
	return MSGTYPEConversion.toString(msgType);
}


} // BGDownloader

