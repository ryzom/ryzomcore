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



#include "stdafx.h"
#include "cfg_file.h"
#include "client_config.h"
#include "client_configDlg.h"
#include "display_dlg.h"

using namespace std;
using namespace NLMISC;

CConfigFile ConfigFile;

// Translation windows map
std::map<HWND, std::string>		HwndMap;

// ***************************************************************************

string GetString (const char *var)
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr (var);

	// Available ?
	if (variable)
	{
		return variable->asString ();
	}
	else
	{
		theApp.error ("Can't find the variable "+(ucstring)var+" in "CONFIG_FILE_NAME);
		return "";
	}
}

// ***************************************************************************

sint GetInt (const char *var)
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr (var);

	// Available ?
	if (variable)
	{
		return variable->asInt ();
	}
	else
	{
		theApp.error ("Can't find the variable "+(ucstring)var+" in "CONFIG_FILE_NAME);
		return -1;
	}
}

// ***************************************************************************

float GetFloat (const char *var)
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr (var);

	// Available ?
	if (variable)
	{
		return variable->asFloat ();
	}
	else
	{
		theApp.error ("Can't find the variable "+(ucstring)var+" in "CONFIG_FILE_NAME);
		return -1;
	}
}

// ***************************************************************************

bool GetBool (const char *var)
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr (var);

	// Available ?
	if (variable)
	{
		string _bool = toLower (variable->asString ());
		if (_bool == "true")
			return true;
		if (_bool == "false")
			return false;
		theApp.error ("The variable "+(ucstring)var+" is not a boolean "CONFIG_FILE_NAME);
		return false;
	}
	else
	{
		theApp.error ("Can't find the variable "+(ucstring)var+" in "CONFIG_FILE_NAME);
		return false;
	}
}

// ***************************************************************************

void SetString (const char *var, const char *value)
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr (var);

	// Available ?
	if (variable)
	{
		if (variable->Type == CConfigFile::CVar::T_STRING)
		{
			if (variable->asString () != value)
				variable->setAsString (value);
		}
		else
		{
			theApp.error ("The variable "+(ucstring)var+" is not a string "CONFIG_FILE_NAME);
		}		
	}
	else
	{
		theApp.error ("Can't find the variable "+(ucstring)var+" in "CONFIG_FILE_NAME);
	}
}

// ***************************************************************************

void SetBool (const char *var, bool value)
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr (var);

	// Available ?
	if (variable)
	{
		if (variable->Type == CConfigFile::CVar::T_STRING)
		{
			const char *valueToSet = value?"true":"false";
			if (variable->asString () != valueToSet)
				variable->setAsString (valueToSet);
		}
		else
		{
			theApp.error ("The variable "+(ucstring)var+" is not a string "CONFIG_FILE_NAME);
		}		
	}
	else
	{
		theApp.error ("Can't find the variable "+(ucstring)var+" in "CONFIG_FILE_NAME);
	}
}

// ***************************************************************************

void SetInt (const char *var, sint value)
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr (var);

	// Available ?
	if (variable)
	{
		if (variable->Type == CConfigFile::CVar::T_INT)
		{
			if (variable->asInt () != value)
				variable->setAsInt (value);
		}
		else
		{
			theApp.error ("The variable "+(ucstring)var+" is not an integer "CONFIG_FILE_NAME);
		}
	}
	else
	{
		theApp.error ("Can't find the variable "+(ucstring)var+" in "CONFIG_FILE_NAME);
	}
}

// ***************************************************************************

void SetFloat (const char *var, float value)
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr (var);

	// Available ?
	if (variable)
	{
		if (variable->Type == CConfigFile::CVar::T_REAL)
		{
			if (fabs (variable->asFloat () - value) > 0.000001f)
				variable->setAsFloat (value);
		}
		else
		{
			theApp.error ("The variable "+(ucstring)var+" is not a float "CONFIG_FILE_NAME);
		}
	}
	else
	{
		theApp.error ("Can't find the variable "+(ucstring)var+" in "CONFIG_FILE_NAME);
	}
}

// ***************************************************************************

void SetToConfigFile ()
{
	// Get the window pointer
	CClient_configDlg *dlg = (CClient_configDlg*)theApp.m_pMainWnd;
	dlg->UpdateData (TRUE);

	// *** General dialog

	// Get the dialog
	CGeneralDlg &general = dlg->GeneralDlg;
	SetInt ("SaveConfig", (int)general.SaveConfig);
	SetString ("LanguageCode", (general.Language==2)?"de":(general.Language==1)?"fr":"en");
	if (general.Sleep)
		SetInt ("ProcessPriority", -1);
	else
		SetInt ("ProcessPriority", 0);

	// *** Display dialog

	// Get the dialog
	CDisplayDlg &display = dlg->DisplayDlg;
	SetInt ("PositionX", display.PositionX);
	SetInt ("PositionY", display.PositionY);
	SetInt ("FullScreen", !display.Windowed);
	switch(display.DriverChoiceMode)
	{
		case CDisplayDlg::DrvChooseDirect3D: SetString("Driver3D", "Direct3D"); break;
		case CDisplayDlg::DrvChooseOpenGL:	 SetString("Driver3D", "OpenGL"); break;
		case CDisplayDlg::DrvChooseAuto:	 SetString("Driver3D", "Auto"); break;
	}
	if (display.Windowed)
	{
		SetInt ("Width", display.Width);
		SetInt ("Height", display.Height);
		SetInt ("Depth", 32);
	}
	else
	{
		// Mode valid ?		
		if (display.DriverChoiceMode != CDisplayDlg::DrvChooseUnknwown)
		{
			const CVideoMode &videoMode = VideoModes[display.getActualDriver()][display.Mode];
			SetInt ("Width", videoMode.Width);
			SetInt ("Height", videoMode.Height);
			SetInt ("Depth", 32);
			SetInt ("Frequency", videoMode.Frequency);
		}
	}

	// *** Display details dialog

	// Get the dialog
	CDisplayDetailsDlg &displayDetails = dlg->DisplayDetailsDlg;
	SetInt ("HDEntityTexture", displayDetails.TextureQualityInt==2);
	SetInt ("DivideTextureSizeBy2", displayDetails.TextureQualityInt==0);
	SetFloat ("LandscapeThreshold", QualityToLandscapeThreshold[displayDetails.LandscapeQualityInt]);
	SetFloat ("Vision", QualityToZFar[displayDetails.LandscapeQualityInt]);
	SetFloat ("LandscapeTileNear", QualityToLandscapeTileNear[displayDetails.LandscapeQualityInt]);
	SetInt ("SkinNbMaxPoly", QualityToSkinNbMaxPoly[displayDetails.CharacterQualityInt]);
	SetInt ("NbMaxSkeletonNotCLod", QualityToNbMaxSkeletonNotCLod[displayDetails.CharacterQualityInt]);
	SetInt ("FxNbMaxPoly", QualityToFxNbMaxPoly[displayDetails.FXQualityInt]);

	// *** Display details dialog

	// Get the dialog
	CDisplayAdvancedDlg &displayAdvanced = dlg->DisplayAdvancedDlg;
	SetInt ("ForceDXTC", !displayAdvanced.DisableDXTC);
	SetInt ("DisableVtxProgram", (int)displayAdvanced.DisableVertexProgram);
	SetInt ("DisableVtxAGP", (int)displayAdvanced.DisableAGPVertices);
	SetInt ("DisableTextureShdr", (int)displayAdvanced.DisableTextureShaders);

	// *** Sound dialog

	// Get the dialog
	CSoundDlg &sound = dlg->SoundDlg;
	SetInt ("SoundOn", (int)sound.SoundOn);
	SetInt ("UseEax", sound.EAX ? true : false);
	SetInt ("MaxTrack", sound.getAsNumSoundBuffer());
	SetStringDriverSound(sound.FMod ? "FMod" : "Auto");
	SetInt ("SoundForceSoftwareBuffer", sound.ForceSoundSoft? true : false);
}

// ***************************************************************************

uint getFullscreenMode (uint driver, const CVideoMode &mode)
{
	// Look for the valid mode ?
	uint i;
	for (i=0; i<VideoModes[driver].size (); i++)
	{
		// Good one ?
		if (mode == VideoModes[driver][i])
		{
			// This mode
			return i;
		}
	}
	return 0xffffffff;
}

// ***************************************************************************

void GetFromConfigFile ()
{
	// Get the window pointer
	CClient_configDlg *dlg = (CClient_configDlg*)theApp.m_pMainWnd;

	// Get the dialogs
	CGeneralDlg &general = dlg->GeneralDlg;
	general.UpdateData ();

	// *** General dialog
	general.SaveConfig = GetInt ("SaveConfig");
	string lang = GetString ("LanguageCode");
	(lang == "de")?general.Language=2:(lang == "fr")?general.Language=1:general.Language=0;

	int sleep = GetInt ("ProcessPriority");
	if (sleep != -1)
	{
		general.Sleep = 0;
	}
	else
	{
		general.Sleep = -1;
	}
	general.UpdateData (FALSE);

	// *** Display dialog

	CDisplayDlg &display = dlg->DisplayDlg;
	display.UpdateData ();
	display.Windowed = !GetInt ("FullScreen");
	display.Width = GetInt ("Width");
	display.Height = GetInt ("Height");
	display.PositionX = GetInt ("PositionX");
	display.PositionY = GetInt ("PositionY");
	std::string driverMode = GetString("Driver3D");
	if (nlstricmp(driverMode, "Auto") == 0 || nlstricmp(driverMode, "0") == 0) display.DriverChoiceMode = CDisplayDlg::DrvChooseAuto;
	else if (nlstricmp(driverMode, "OpenGL") == 0 || nlstricmp(driverMode, "1") == 0) display.DriverChoiceMode = CDisplayDlg::DrvChooseOpenGL;	
	else if (nlstricmp(driverMode, "Direct3D") == 0 || nlstricmp(driverMode, "2") == 0) display.DriverChoiceMode = CDisplayDlg::DrvChooseDirect3D;
	else display.DriverChoiceMode = CDisplayDlg::DrvChooseAuto;
	display.Mode = -1;
	// The video mode
	CVideoMode mode;
	mode.Width = display.Width;
	mode.Height = display.Height;
	mode.ColorDepth = 32;
	mode.Frequency = GetInt ("Frequency");

	// Look for the valid mode ?
	uint i;
	CDisplayDlg::TDriver actualDriver = display.getActualDriver();
	for (i=0; i<VideoModes[actualDriver].size (); i++)
	{
		// Good one ?
		if (mode == VideoModes[actualDriver][i])
		{
			// This mode
			display.Mode = i;
			break;
		}
	}

	// If not found, look for default resolution
	mode.Width = 1024;
	mode.Height = 768;
	mode.ColorDepth = 32;
	mode.Frequency = 60;
	

	display.updateState ();
	display.UpdateData (FALSE);

	// *** Display details dialog

	CDisplayDetailsDlg &displayDetails = dlg->DisplayDetailsDlg;
	int hdTextureInstalled = GetInt ("HDTextureInstalled");
	displayDetails.UpdateData ();
	displayDetails.MaxTextureQuality= hdTextureInstalled?2:1;
	// NB: if the player changes its client.cfg, mixing HDEntityTexture=1 and DivideTextureSizeBy2=1, it will
	// result to a low quality!
	if(GetInt ("DivideTextureSizeBy2"))
		displayDetails.TextureQualityInt = 0;
	else if(GetInt ("HDEntityTexture") && hdTextureInstalled)
		displayDetails.TextureQualityInt = 2;
	else
		displayDetails.TextureQualityInt = 1;
	displayDetails.LandscapeQualityInt = GetQuality (QualityToLandscapeThreshold, GetFloat ("LandscapeThreshold"));
	displayDetails.FXQualityInt = GetQuality (QualityToFxNbMaxPoly, GetInt ("FxNbMaxPoly"));
	// minimize quality according to NbMaxSkeletonNotCLod and SkinNbMaxPoly
	displayDetails.CharacterQualityInt = GetQuality (QualityToSkinNbMaxPoly, GetInt ("SkinNbMaxPoly"));
	displayDetails.CharacterQualityInt = min(displayDetails.CharacterQualityInt, GetQuality (QualityToNbMaxSkeletonNotCLod, GetInt ("NbMaxSkeletonNotCLod")));
	displayDetails.updateState ();
	displayDetails.UpdateData (FALSE);

	// *** Display advanced dialog

	CDisplayAdvancedDlg &displayAdvanced = dlg->DisplayAdvancedDlg;
	displayAdvanced.UpdateData ();
	displayAdvanced.DisableDXTC = !GetInt ("ForceDXTC");
	displayAdvanced.DisableVertexProgram = GetInt ("DisableVtxProgram") ? TRUE : FALSE;
	displayAdvanced.DisableAGPVertices = GetInt ("DisableVtxAGP") ? TRUE : FALSE;
	displayAdvanced.DisableTextureShaders = GetInt ("DisableTextureShdr") ? TRUE : FALSE;
	displayAdvanced.UpdateData (FALSE);

	// *** Sound dialog

	// Get the dialog
	CSoundDlg &sound = dlg->SoundDlg;
	sound.UpdateData ();
	sound.SoundOn = GetInt ("SoundOn");
	sound.EAX = GetInt ("UseEax") ? TRUE : FALSE;
	sound.FMod = GetStringDriverSound()=="FMod" ? TRUE : FALSE;
	sound.ForceSoundSoft = GetInt ("SoundForceSoftwareBuffer") ? TRUE : FALSE;
	sound.setAsNumSoundBuffer( GetInt ("MaxTrack") );
	sound.updateState ();
	sound.UpdateData (FALSE);
}

// ***************************************************************************

bool LoadConfigFile ()
{
	// Read the file
	try
	{
		ConfigFile.load (CONFIG_FILE_NAME);
	}
	catch (Exception &e)
	{
		theApp.error ("Error reading the file "CONFIG_FILE_NAME" : "+string (e.what ()));
		return false;
	}
	return true;
}

// ***************************************************************************

bool SaveConfigFile ()
{
	// Read the file
	try
	{
		ConfigFile.save ();
	}
	catch (Exception &e)
	{
		theApp.error (CI18N::get ("uiConfigErrorWritingTheFile")+" "CONFIG_FILE_NAME" : "+string (e.what ()));
		return false;
	}
	return true;
}

// ***************************************************************************

void InvalidateConfig ()
{
	theApp.Modified = true;

	// Get the window pointer
	CClient_configDlg *dlg = (CClient_configDlg*)theApp.m_pMainWnd;
	dlg->ApplyCtrl.EnableWindow (TRUE);

	// Set the title
	setWindowText(*dlg, (WCHAR*)(CI18N::get ("uiConfigTitle") + " *").c_str());
}

// ***************************************************************************

void ResetConfigFile ()
{
	// Clear the config file
	ConfigFile.clear ();
	ConfigFile.reparse (/*CONFIG_DEFAULT_FILE_NAME*/);
	CConfigFile::CVar var;
	var.Type = CConfigFile::CVar::T_STRING;
	var.setAsString (CONFIG_DEFAULT_FILE_NAME);
	CConfigFile::CVar *newVar = ConfigFile.insertVar ("RootConfigFilename", var);
}

// ***************************************************************************

void MergeConfigFile (CConfigFile &configFile)
{
	// For each variable in the new config file
	uint count = configFile.getNumVar ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Get the variable
		CConfigFile::CVar *src = configFile.getVar (i);

		// Get the destination variable
		CConfigFile::CVar *dest = ConfigFile.insertVar (src->Name, *src);
		if ((src->Type == CConfigFile::CVar::T_INT) && (dest->Type == CConfigFile::CVar::T_INT) ||
			(src->Type == CConfigFile::CVar::T_REAL) && (dest->Type == CConfigFile::CVar::T_INT) ||
			(src->Type == CConfigFile::CVar::T_INT) && (dest->Type == CConfigFile::CVar::T_REAL) ||
			(src->Type == CConfigFile::CVar::T_REAL) && (dest->Type == CConfigFile::CVar::T_REAL) ||
			(src->Type == CConfigFile::CVar::T_STRING) && (dest->Type == CConfigFile::CVar::T_STRING))
		{
			// Get the method for the merge
			TMergeMethod merge = GetMergeMethod (src->Name.c_str ());
			if (src->Type == CConfigFile::CVar::T_INT)
			{
				if (dest->Root)
				{
					SetInt (src->Name.c_str (), src->asInt());
				}
				else
				{
					if (merge == PreferInferior)
						SetInt (src->Name.c_str (), min (src->asInt(), dest->asInt()));
					else if (merge == PreferSuperior)
						SetInt (src->Name.c_str (), max (src->asInt(), dest->asInt()));
					else if (merge == PreferTrue)
						SetInt (src->Name.c_str (), (int)((src->asInt()!=0)||(dest->asInt()!=0)));
					else if (merge == PreferFalse)
						SetInt (src->Name.c_str (), (int)((src->asInt()!=0)&&(dest->asInt()!=0)));
				}
			}
			else if (src->Type == CConfigFile::CVar::T_REAL)
			{
				if (dest->Root)
				{
					SetFloat (src->Name.c_str (), src->asFloat());
				}
				else
				{
					if (merge == PreferInferior)
						SetFloat (src->Name.c_str (), min (src->asFloat(), dest->asFloat()));
					else if (merge == PreferSuperior)
						SetFloat (src->Name.c_str (), max (src->asFloat(), dest->asFloat()));
					else if (merge == PreferTrue)
						SetFloat (src->Name.c_str (), (float)((src->asFloat()!=0.f)||(dest->asFloat()!=0.f)));
					else if (merge == PreferFalse)
						SetFloat (src->Name.c_str (), (float)((src->asFloat()!=0.f)&&(dest->asFloat()!=0.f)));
				}
			}
			else if (src->Type == CConfigFile::CVar::T_STRING)
			{
				if (dest->Root)
				{
					SetString (src->Name.c_str (), src->asString().c_str ());
				}
				else
				{
					if (merge == PreferTrue)
						SetBool (src->Name.c_str (), (src->asString()!="false")||(dest->asString()!="true"));
					else if (merge == PreferFalse)
						SetBool (src->Name.c_str (), (src->asString()!="false")&&(dest->asString()!="true"));
					else
						SetString (src->Name.c_str (), src->asString().c_str ());
				}
			}
		}
	}
}

// ***************************************************************************

void ResetConfigFileToDefault ()
{
	// For each variable not ROOT in the current config file
	uint count = ConfigFile.getNumVar ();
	uint i;
	for (i=0; i<count; i++)
	{
		// Get the destination variable
		CConfigFile::CVar *dest = ConfigFile.getVar (i);
		
		// Temp: avoid changing this variable (debug: binded to the actual texture set installed)
		if(dest->Name=="HDTextureInstalled")
			continue;

		// Get the variable from default config file
		CConfigFile::CVar *src = ConfigFileDefault.getVarPtr (dest->Name);

		// if default exist and cur not already default
		if ( src && !dest->Root && 
		   ((src->Type == CConfigFile::CVar::T_INT) && (dest->Type == CConfigFile::CVar::T_INT) ||
			(src->Type == CConfigFile::CVar::T_REAL) && (dest->Type == CConfigFile::CVar::T_INT) ||
			(src->Type == CConfigFile::CVar::T_INT) && (dest->Type == CConfigFile::CVar::T_REAL) ||
			(src->Type == CConfigFile::CVar::T_REAL) && (dest->Type == CConfigFile::CVar::T_REAL) ||
			(src->Type == CConfigFile::CVar::T_STRING) && (dest->Type == CConfigFile::CVar::T_STRING)))
		{
			if (src->Type == CConfigFile::CVar::T_INT)
			{
				SetInt (src->Name.c_str (), src->asInt());
			}
			else if (src->Type == CConfigFile::CVar::T_REAL)
			{
				SetFloat (src->Name.c_str (), src->asFloat());
			}
			else if (src->Type == CConfigFile::CVar::T_STRING)
			{
				SetString (src->Name.c_str (), src->asString().c_str());
			}
		}
	}
}

// ***************************************************************************

void backupWindowHandleRec (CWnd *cwnd)
{
	if (!cwnd)
		cwnd = theApp.m_pMainWnd;

	CString text;
	cwnd->GetWindowText (text);
	string stlText = (const char*)text;

	// Add an entry for this window
	if ((stlText.size()>2) && (stlText[0] == 'u') && (stlText[1] == 'i'))
		HwndMap.insert (std::map<HWND, std::string>::value_type (*cwnd, (const char*)text));
	/* else if (!stlText.empty() && GetInt ("TestConfig") )
		HwndMap.insert (std::map<HWND, std::string>::value_type (*cwnd, (const char*)"toto")); */

	// Go for the children
	CWnd* child = cwnd->GetWindow(GW_CHILD);
	while (child)
	{
		backupWindowHandleRec (child);
		child = child->GetWindow(GW_HWNDNEXT);
	}
}

// ***************************************************************************

void localizeWindowsRec (CWnd *cwnd, bool useMap)
{
	if (!cwnd)
		cwnd = theApp.m_pMainWnd;

	if (useMap)
	{
		std::map<HWND, std::string>::iterator ite = HwndMap.find (*cwnd);
		if (ite != HwndMap.end())
		{
			ucstring ucName = CI18N::get (ite->second);			
			setWindowText(ite->first, (WCHAR*)ucName.c_str());
		}
	}
	else
	{
		CString name;
		cwnd->GetWindowText (name);
		ucstring ucName = CI18N::get ((const char*)name);
		setWindowText(*cwnd, (WCHAR*)ucName.c_str());
	}

	// Go for the children
	CWnd* child = cwnd->GetWindow(GW_CHILD);
	while (child)
	{
		localizeWindowsRec (child, useMap);
		child = child->GetWindow(GW_HWNDNEXT);
	}
}


// ***************************************************************************
std::string	GetStringDriverSound()
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr ("DriverSound");
	
	// Available ?
	if (variable)
		return variable->asString ();
	// else force FMod
	else
		return "FMod";
}

// ***************************************************************************
sint		GetIntForceLanguage()
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr ("ForceLanguage");
	
	// Available ?
	if (variable)
		return variable->asInt ();
	// else force "NO"
	else
		return 0;
}

// ***************************************************************************
sint		GetIntTestConfig()
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr ("TestConfig");
	
	// Available ?
	if (variable)
		return variable->asInt ();
	// else force NO
	else
		return 0;
}

// ***************************************************************************
void		SetStringDriverSound(const char *value)
{
	// Get the variable pointer
	CConfigFile::CVar *variable = ConfigFile.getVarPtr ("DriverSound");
	
	// Available ?
	if (variable)
	{
		if (variable->Type == CConfigFile::CVar::T_STRING)
		{
			if (variable->asString () != value)
				variable->setAsString (value);
		}
		else
		{
			theApp.error (ucstring("The variable DriverSound is not a string "CONFIG_FILE_NAME));
		}		
	}
	else
	{
		// no op (FMod should be forced)
	}
}

