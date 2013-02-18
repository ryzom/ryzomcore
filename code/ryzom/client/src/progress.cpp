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
#include "progress.h"
#include "time_client.h"
#include "global.h"
#include "nel/misc/events.h"
#include "nel/3d/u_texture.h"
#include "game_share/ryzom_version.h"
#include "nel/misc/i18n.h"
#include "continent.h"
#include "weather.h"
#include "weather_manager_client.h"
#include "interface_v3/input_handler_manager.h"
#include "interface_v3/interface_manager.h"
#include "release.h"
#include "net_manager.h"
#include "client_cfg.h"
#include "bg_downloader_access.h"
#include "nel/misc/system_utils.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;


extern void selectTipsOfTheDay (uint tips);
extern NL3D::UMaterial LoadingMaterial;
extern NL3D::UMaterial LoadingMaterialFull;

extern std::vector<UTextureFile*> LogoBitmaps;
extern uint TipsOfTheDayIndex;
extern ucstring			TipsOfTheDay;
extern bool					UseEscapeDuringLoading;

CProgress::CProgress ()
{
	_LastUpdate = ryzomGetLocalTime ();
	_FontFactor = 1.0f;
	pushCropedValues (0, 1);
	ApplyTextCommands = false;
	_TPCancelFlag = false;
}

CProgress::~CProgress ()
{
}

void CProgress::setFontFactor(float temp)
{
	_FontFactor = temp;
}

void CProgress::newMessage (const ucstring& message)
{
	popCropedValues ();
	_CurrentRootStep++;
	pushCropedValues ((float)_CurrentRootStep/(float)_RootStepCount, (float)(_CurrentRootStep+1)/(float)_RootStepCount);

	// New message
	_ProgressMessage = message;

	// Force call to progress
	internalProgress (0);
}

void CProgress::reset (uint rootNodeCount)
{
	_CurrentRootStep = 0;
	_RootStepCount = rootNodeCount;
	popCropedValues ();
	pushCropedValues (0, 1/(float)_RootStepCount);
	ApplyTextCommands = false;

	finish();
}

void CProgress::progress (float value)
{
	// Timer
	sint64 newTime = ryzomGetLocalTime ();
	if (newTime - _LastUpdate > PROGRESS_BAR_UPDATE)
	{
		internalProgress (value);

		_LastUpdate = newTime;
	}
}

float nextLine (uint fontHeight, uint windowHeight, float previousY)
{
	if (windowHeight > 0)
	{
		return previousY-(float)fontHeight/(float)windowHeight;
	}
	return previousY;
}

void drawLoadingBitmap (float progress)
{
	if (!LoadingMaterial.empty() && !LoadingMaterialFull.empty())
	{
		Driver->setMatrixMode2D11();
		LoadingMaterialFull.setAlphaTestThreshold (1.f-progress);

		CQuadUV		quad;
		uint wh =  Driver->getWindowHeight();
		uint ww = Driver->getWindowWidth();
		float x1 = 0;
		float y1 = 0;
		float x2 = 1;
		float y2 = 1;

		if ((ww > 1024) || (wh > 768))
		{
			if ((ww - 1024) > (wh - 768))
				x1 = ((ww - (wh * 1024.f) / 768.f) / 2.f) / ww;
			else
				y1 = ((wh - (ww * 768.f) / 1024.f) / 2.f) / wh;
		}

		if (x1 != 0)
			x2 = 1 - x1;
		if (y1 != 0)
			y2 = 1 - y1;

		quad.V0.set (x1,y1,0);
		quad.V1.set (x2,y1,0);
		quad.V2.set (x2,y2,0);
		quad.V3.set (x1,y2,0);

		quad.Uv0.U= 0;
		quad.Uv0.V= 0.75f;
		quad.Uv1.U= 1;
		quad.Uv1.V= 0.75f;
		quad.Uv2.U= 1;
		quad.Uv2.V= 0;
		quad.Uv3.U= 0;
		quad.Uv3.V= 0;

		Driver->drawQuad(0, 0, 1, 1, CRGBA(0, 0, 0, 255));
		Driver->drawQuad(quad, LoadingMaterial);
		Driver->drawQuad(quad, LoadingMaterialFull);
	}
}

void CProgress::internalProgress (float value)
{
	// Get croped value
	value = getCropedValue (value);

	// can't do anything if no driver
	if (Driver == NULL)
		return;

	if (Driver->AsyncListener.isKeyPushed (KeyDOWN))
		selectTipsOfTheDay (TipsOfTheDayIndex-1);
	if (Driver->AsyncListener.isKeyPushed (KeyUP))
		selectTipsOfTheDay (TipsOfTheDayIndex+1);

	// Font factor
	float fontFactor = 1;
	if (Driver->getWindowHeight() > 0)
		fontFactor = (float)Driver->getWindowHeight() / 600.f;
	fontFactor *= _FontFactor;
	// Set 2d view.
	Driver->setMatrixMode2D11();
	Driver->clearBuffers (CRGBA(0,0,0,0));

	// Display the loading background.
	drawLoadingBitmap (value);

	// temporary values for conversions
	float x, y, width, height;

	for(uint i = 0; i < ClientCfg.Logos.size(); i++)
	{
		std::vector<string> res;
		explode(ClientCfg.Logos[i], std::string(":"), res);
		if(res.size()==9 && i<LogoBitmaps.size() && LogoBitmaps[i]!=NULL)
		{
			fromString(res[1], x);
			fromString(res[2], y);
			fromString(res[3], width);
			fromString(res[4], height);
			Driver->drawBitmap(x/(float)ClientCfg.Width, y/(float)ClientCfg.Height, width/(float)ClientCfg.Width, height/(float)ClientCfg.Height, *LogoBitmaps[i]);
		}
	}

	if (TextContext != NULL)
	{
		// Init the Pen.
		TextContext->setKeep800x600Ratio(false);
		TextContext->setColor(CRGBA(255,255,255));
		TextContext->setFontSize((uint)(12.f * fontFactor));
		TextContext->setHotSpot(UTextContext::TopRight);

#if !FINAL_VERSION
		// Display the Text.
		TextContext->printAt(1, 0.98f, _ProgressMessage);
#else
		if( ClientCfg.LoadingStringCount > 0 )
		{
			TextContext->printAt(1, 0.98f, _ProgressMessage);
		}
#endif // FINAL_VERSION

		// Display the build version.
		TextContext->setFontSize((uint)(12.f * fontFactor));
		TextContext->setHotSpot(UTextContext::TopRight);
		string str;
#if FINAL_VERSION
		str = "FV ";
#else
		str = "DEV ";
#endif
		str += RYZOM_VERSION;
		TextContext->printfAt(1.0f,1.0f, str.c_str());

		// Display the tips of the day.
		TextContext->setFontSize((uint)(16.f * fontFactor));
		TextContext->setHotSpot(UTextContext::MiddleTop);
		ucstring::size_type index = 0;
		ucstring::size_type end = TipsOfTheDay.find((ucchar)'\n');
		if (end == string::npos)
			end = TipsOfTheDay.size();
		float fY = ClientCfg.TipsY;
		if (!TipsOfTheDay.empty())
		{
			while (index < end)
			{
				// Get the line
				ucstring line = TipsOfTheDay.substr (index, end-index);

				// Draw the line
				TextContext->printAt(0.5f, fY, line);
				fY = nextLine (TextContext->getFontSize(), Driver->getWindowHeight(), fY);

				index=end+1;
				end = TipsOfTheDay.find((ucchar)'\n', index);
				if (end == ucstring::npos)
					end = TipsOfTheDay.size();
			}

			// More help
			TextContext->setFontSize((uint)(12.f * fontFactor));
			/* todo tips of the day uncomment
			ucstring ucstr = CI18N::get ("uiTipsEnd");
			TextContext->printAt(0.5f, fY, ucstr); */
			fY = nextLine (TextContext->getFontSize(), Driver->getWindowHeight(), fY);
			fY = nextLine (TextContext->getFontSize(), Driver->getWindowHeight(), fY);
		}



		if (!_TPReason.empty())
		{
			TextContext->setHotSpot(UTextContext::MiddleMiddle);
			TextContext->setFontSize((uint)(14.f * fontFactor));
			TextContext->printAt(0.5f, 0.5f, _TPReason);
			TextContext->setHotSpot(UTextContext::BottomLeft);
			TextContext->setColor(NLMISC::CRGBA::White);
		}



		if (!_TPCancelText.empty())
		{
			if (ClientCfg.Width != 0 && ClientCfg.Height != 0)
			{
				TextContext->setFontSize((uint)(15.f * fontFactor));
				TextContext->setHotSpot(UTextContext::BottomLeft);

				ucstring uc = CI18N::get("uiR2EDTPEscapeToInteruptLoading") + " (" + _TPCancelText + ") - " + CI18N::get("uiDelayedTPCancel");
				UTextContext::CStringInfo info = TextContext->getStringInfo(uc);
				float stringX = 0.5f - info.StringWidth/(ClientCfg.Width*2);
				TextContext->printAt(stringX, 7.f / ClientCfg.Height, uc);
			}
		}


		// Teleport help
		//fY = ClientCfg.TeleportInfoY;
		if (!ApplyTextCommands && LoadingContinent && !LoadingContinent->Indoor)
		{
			TextContext->setFontSize((uint)(13.f * fontFactor));

			// Print some more info
			uint32 day = RT.getRyzomDay();
			str = toString (CI18N::get ("uiTipsTeleport").toUtf8().c_str(),
				CI18N::get (LoadingContinent->LocalizedName).toUtf8().c_str(),
				day,
				(uint)RT.getRyzomTime(),
				CI18N::get ("uiSeason"+toStringEnum(CRyzomTime::getSeasonByDay(day))).toUtf8().c_str(),
				CI18N::get (WeatherManager.getCurrWeatherState().LocalizedName).toUtf8().c_str());
			ucstring ucstr;
			ucstr.fromUtf8 (str);
			TextContext->setHotSpot(UTextContext::MiddleBottom);
			TextContext->setColor(CRGBA(186, 179, 163, 255));
			TextContext->printAt(0.5f, 25/768.f, ucstr);
		}

		// apply text commands
		if( ApplyTextCommands )
		{
			std::vector<CClientConfig::SPrintfCommand> printfCommands = ClientCfg.PrintfCommands;
			if(FreeTrial) printfCommands = ClientCfg.PrintfCommandsFreeTrial;

			if( !printfCommands.empty() )
			{
				TextContext->setHotSpot(UTextContext::MiddleBottom);

				vector<CClientConfig::SPrintfCommand>::iterator itpc;
				for( itpc = printfCommands.begin(); itpc != printfCommands.end(); ++itpc )
				{
					float x = 0.5f;//((*itpc).X / 1024.f);
					float y = ((*itpc).Y / 768.f);
					TextContext->setColor( (*itpc).Color );
					TextContext->setFontSize( (uint)(16.f * fontFactor));

					// build the ucstr(s)
					ucstring ucstr = CI18N::get((*itpc).Text);
					vector<ucstring> vucstr;
					ucstring sep("\n");
					splitUCString(ucstr,sep,vucstr);

					// Letter size
					UTextContext::CStringInfo si = TextContext->getStringInfo(ucstring("|"));
					uint fontHeight = (uint) si.StringHeight + 2; // we add 2 pixels for the gap

					uint i;
					float newy = y;
					for( i=0; i<vucstr.size(); ++i )
					{
						TextContext->printAt(x,newy, vucstr[i]);
						newy = nextLine(fontHeight, Driver->getWindowHeight(), newy);
					}
				}
			}
		}
	}

	// Clamp
	clamp (value, 0.f, 1.f);

	// Set matrix
	Driver->setMatrixMode2D11 ();

	// want to receive the 'mouse down' event to deal with the 'cancel tp button'
	Driver->EventServer.addListener(EventMouseDownId,	this);

	// Update messages
	CInputHandlerManager::getInstance()->pumpEventsNoIM();

	Driver->EventServer.removeListener(EventMouseDownId,	this);

	// Exit ?
	bool activeDriver =  Driver->isActive();
	if ((UseEscapeDuringLoading && Driver->AsyncListener.isKeyPushed (KeyESCAPE)) || !activeDriver)
	{
		// Release the application
		releaseMainLoop(true);
		release();
		// Leave the application
		extern void quitCrashReport ();
		quitCrashReport ();
		exit(EXIT_SUCCESS);
	}

	if(!_TPCancelText.empty() &&  Driver->AsyncListener.isKeyPushed(KeySHIFT) && Driver->AsyncListener.isKeyPushed(KeyESCAPE))
	{
		_TPCancelFlag = true;
	}


	CBGDownloaderAccess::getInstance().update();
	// Display to screen.
	Driver->swapBuffers();

	// \todo GUIGUI : Remove this when possible.
	NetMngr.update();
	IngameDbMngr.flushObserverCalls();
	NLGUI::CDBManager::getInstance()->flushObserverCalls();

	// update system dependent progress bar
	static uint previousValue = 0;
	uint currentValue = (uint)(value*100.0f);

	if (currentValue != previousValue)
	{
		CSystemUtils::updateProgressBar(currentValue, 100);
		previousValue = currentValue;
	}
}


void CProgress::setTPMessages(const ucstring &tpReason,const ucstring &tpCancelText, const std::string &/* iconName */)
{
	_TPReason = tpReason;
	_TPCancelText = tpCancelText;
}


void CProgress::operator ()(const CEvent& /* event */)
{
}

bool CProgress::getTPCancelFlag(bool clearFlag /*=true*/)
{
	bool flag = _TPCancelFlag;
	if (clearFlag)
	{
		_TPCancelFlag = false;
	}
	return flag;
}

void CProgress::release()
{
	setTPMessages(ucstring(), ucstring(), "");
	_TPCancelFlag = false;
}

void CProgress::finish()
{
	// stop system dependent progress bar
	CSystemUtils::updateProgressBar(1, 0);
}

