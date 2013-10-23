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
#include "sound_plugin.h"
#include "resource.h"
#include "nel/misc/path.h"
#include "LoadDialog.h"
#include "nel/sound/u_listener.h"

using namespace NLMISC;
using namespace NLLIGO;
using namespace NLSOUND;
using namespace std;

CFileDisplayer		*SoundPluginLogDisplayer= NULL;

// vl: important to add the next line or AfxGetApp() will returns 0 after AFX_MANAGE_STATE(AfxGetStaticModuleState());
CWinApp theApp;

extern "C"
{
	//void *createSoundPlugin()
	void *createPlugin()
	{
		return new CSoundPlugin();
	}
}

CSoundPlugin::CSoundPlugin()
	: _PluginAccess(0),
	_ListenerPos(0,0,0),
	LoadDlg(0),
	_BackgroundSoundPlayed(false),
	_Mixer(NULL)

{
	NLMISC::createDebug();
	SoundPluginLogDisplayer= new CFileDisplayer("world_editor_sound_plugin.log", true, "WORLD_EDITOR_SOUND_PLUGIN.LOG");
	DebugLog->addDisplayer (SoundPluginLogDisplayer);
	InfoLog->addDisplayer (SoundPluginLogDisplayer);
	WarningLog->addDisplayer (SoundPluginLogDisplayer);
	ErrorLog->addDisplayer (SoundPluginLogDisplayer);
	AssertLog->addDisplayer (SoundPluginLogDisplayer);

	nlinfo("Starting sound plugin...");
	
}


CSoundPlugin::~CSoundPlugin()
{
}

void CSoundPlugin::progress (float progressValue)
{
	char tmp[1024];
	sprintf(tmp, "Initializing mixer : %i%%", int(progressValue * 100));
	LoadDlg->Message = tmp;
	LoadDlg->UpdateData(FALSE);
	LoadDlg->RedrawWindow();
}


void CSoundPlugin::init(IPluginAccess *pluginAccess)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	AfxEnableControlContainer();
	_PluginAccess = pluginAccess;
	_PluginName="Sound Plugin";

	LoadDlg = new CLoadDialog;
	LoadDlg->Create(IDD_DIALOG_LOAD);
	LoadDlg->Message = "Initializing mixer";
	LoadDlg->UpdateData(FALSE);
	LoadDlg->ShowWindow(SW_SHOW);
	LoadDlg->RedrawWindow();

//	std::string path = "r:/code/ryzom/data/sound";
//	NLMISC::CPath::addSearchPath(path, true, false);

	LoadDlg->Message = "Adding search path";
	LoadDlg->UpdateData(FALSE);
	LoadDlg->Invalidate();

	CConfigFile &cf = pluginAccess->getConfigFile();
	// Add the search path
	try
	{
		CConfigFile::CVar *psearchPath = cf.getVarPtr("SearchPath");
		for (uint i=0; i<psearchPath->size(); ++i)
		{
			std::string path = psearchPath->asString(i);
			LoadDlg->Message = (std::string("Adding search path \"")+path+"\"").c_str();
			LoadDlg->UpdateData(FALSE);
			LoadDlg->RedrawWindow();
			NLMISC::CPath::addSearchPath(path, true, false);
		}
	}
	catch(...)
	{
		nlwarning("Error while setting search path");
	}
	delete LoadDlg;
	LoadDlg = 0;

	ReInit();

	_DialogFlag = new CDialogFlags(_Mixer);
	// open the dialog flag window
	_DialogFlag->Create(IDD_DIALOG_FLAGS, CWnd::FromHandle(_PluginAccess->getMainWindow()->m_hWnd));
	_DialogFlag->init(this);

	_DialogFlag->ShowWindow(TRUE);

	_Mixer->getListener()->setOrientation(CVector(0,1,0), CVector(0,0,1));

	_PluginActive=true;
}

void CSoundPlugin::ReInit()
{
	CConfigFile &cf = _PluginAccess->getConfigFile();
	std::string packedSheetPath;
	CConfigFile::CVar *ppackedSheetPath = cf.getVarPtr("PackedSheetPath");
	packedSheetPath = ppackedSheetPath->asString();
	
	LoadDlg = new CLoadDialog;
	LoadDlg->Create(IDD_DIALOG_LOAD);
	LoadDlg->Message = "Initializing Audio Mixer";
	LoadDlg->UpdateData(FALSE);
	LoadDlg->ShowWindow(SW_SHOW);
	LoadDlg->RedrawWindow();

	// If it's a true reinit, must reload primitives
	bool	mustReloadPrimitives= false;
	if (_Mixer != NULL)
	{
		mustReloadPrimitives= true;
		delete _Mixer;
		_Mixer = NULL;
	}
	
	// get maxtrack
	int maxTrack;
	try
	{
		maxTrack = cf.getVar("MaxTrack").asInt();
	}
	catch(...)
	{
		maxTrack = 32;
	}

	// get DriverMod
	bool	fmodDriver = false;
	try
	{
		fmodDriver = toLower(cf.getVar("DriverSound").asString())=="fmod";
	}
	catch(...)
	{
		fmodDriver = false;
	}

	// create the audio mixer.
	_Mixer = UAudioMixer::createAudioMixer();
	_Mixer->setSamplePath(cf.getVar("SamplePath").asString());
	_Mixer->setPackedSheetOption(packedSheetPath, true);
	try
	{
		if(fmodDriver)
		{
			// init FMod, software, no eax
			_Mixer->init(maxTrack, false, false, this, false, UAudioMixer::DriverFMod, true);
		}
		else
		{
			// init DSound, hardware, eax
			_Mixer->init(maxTrack, true, false, this, false, UAudioMixer::DriverAuto, false);
		}
	}
	catch(...)
	{
		MessageBox(NULL, "Error while initializing audio mixer.\n", "ERROR", MB_ICONERROR);
		exit(1);
	}
	
	// Force reinit of all the primitives if reload
	if(mustReloadPrimitives)
	{
		std::vector<NLLIGO::IPrimitive*>	prims;
		_PluginAccess->getAllRootPluginPrimitive(prims);
		for(uint i=0;i<prims.size();i++)
		{
			primitiveChanged(prims[i]);
		}
	}

	// If background sound was played before the reinit, replay now
	if(_BackgroundSoundPlayed)
	{
		_Mixer->playBackgroundSound();
	}


	delete LoadDlg;
	LoadDlg = 0;

	// remove flood of config file reparsing
	DebugLog->addNegativeFilter("Adding config file");
}

/// The current region has changed.
//void CSoundPlugin::primRegionChanged(const std::vector<NLLIGO::CPrimRegion*> &regions)
void CSoundPlugin::primitiveChanged(const NLLIGO::IPrimitive *root)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// load the first region of each kind : audio, effect and sample.
	bool	audio = false;
	bool	effects = false;
	bool	samples = false;

	_Mixer->loadBackgroundAudioFromPrimitives(*root);

/*	std::vector<NLLIGO::CPrimRegion*>::const_iterator first(regions.begin()), last(regions.end());
	for (; first != last; ++first)
	{
		if ((*first)->Name.find("_audio") != std::string::npos && !audio)
		{
			_Mixer->loadBackgroundSoundFromRegion(*(*first));
			audio = true;
		}
		else if ((*first)->Name.find("_effects") != std::string::npos && !effects)
		{
			_Mixer->loadBackgroundEffectsFromRegion(*(*first));
			effects = true;
		}
		else if ((*first)->Name.find("_samples") != std::string::npos && !samples)
		{
			_Mixer->loadBackgroundSamplesFromRegion(*(*first));
			samples = true;
		}
	}
*/
}
/// The region has been modifed.
/*void CSoundPlugin::primRegionModifed()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
}
*/
/// The listener has been moved on the map.
void CSoundPlugin::positionMoved(const NLMISC::CVector &position)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	_ListenerPos = position;

	_Mixer->setListenerPos(_ListenerPos);
}

void CSoundPlugin::lostPositionControl()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// update the dialog box.
	_DialogFlag->lostPositionControl();

}


void CSoundPlugin::play()
{
	// set the user var present in the config file
	CConfigFile &cfg = _PluginAccess->getConfigFile();
	try
	{
		cfg.reparse();
		CConfigFile::CVar v = cfg.getVar("UserVarSim");
		for (uint i=0; i<v.size()/2; ++i)
		{
			string	varName = v.asString(i*2);
			float	value = v.asFloat(i*2+1);
			_Mixer->setUserVar(CStringMapper::map(varName), value);
		}
	}
	catch(...)
	{
	}

	_Mixer->playBackgroundSound();
	_BackgroundSoundPlayed= true;
}
void CSoundPlugin::stop()
{
	_Mixer->stopBackgroundSound();
	_BackgroundSoundPlayed= false;
}
void CSoundPlugin::startMoveEar()
{
	_PluginAccess->startPositionControl(this, _ListenerPos);
}
void CSoundPlugin::stopMoveEar()
{
	_PluginAccess->stopPositionControl(this);
}

void CSoundPlugin::update()
{
	// set the user var present in the config file
	CConfigFile &cfg = _PluginAccess->getConfigFile();
	try
	{
		cfg.reparse();
		CConfigFile::CVar v = cfg.getVar("UserVarSim");
		for (uint i=0; i<v.size()/2; ++i)
		{
			string	varName = v.asString(i*2);
			float	value = v.asFloat(i*2+1);
			_Mixer->setUserVar(CStringMapper::map(varName), value);
		}
	}
	catch(...)
	{
	}

	const NLSOUND::UAudioMixer::TBackgroundFlags	&flags = _Mixer->getBackgroundFlags();;
	_Mixer->update();	
}


std::string CSoundPlugin::getStatusString()
{
	char tmp[1024];
	sprintf(tmp, "Sample mem : %7.3f Mo\nTrks/Mut/Ply/Inst : %2u/%2u/%2u/%2u", 
		_Mixer->getLoadedSampleSize()/(1024.0f*1024.0f), 
		_Mixer->getUsedTracksCount(), 
		_Mixer->getMutedPlayingSourcesCount(),
		_Mixer->getPlayingSourcesCount(),
		_Mixer->getSourcesInstanceCount()
		);

	return std::string(tmp);
}

void CSoundPlugin::getLoadedSampleBankInfo(std::vector<std::pair<std::string, uint> > &result)
{
	_Mixer->getLoadedSampleBankInfo(result);
}

void CSoundPlugin::onIdle()
{
}

bool CSoundPlugin::isActive()
{
	return _PluginActive;
}

string& CSoundPlugin::getName()
{
	return _PluginName;
}

bool CSoundPlugin::activatePlugin()
{
	if(!_PluginActive)
	{
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
		AfxEnableControlContainer();
		
		//_DialogFlag->Create(IDD_DIALOG_FLAGS, CWnd::FromHandle(_PluginAccess->getMainWindow()->m_hWnd));
		_DialogFlag->ShowWindow(TRUE);
		//_DialogFlag->init(this);
		_PluginActive=true;
		return true;
	}
	return false;
}

bool CSoundPlugin::closePlugin()
{
	if (_PluginActive)
	{
		//_DialogFlag->CloseWindow();
		_DialogFlag->ShowWindow(FALSE);
		_PluginActive=false;
		return true;
	}
	return false;
}