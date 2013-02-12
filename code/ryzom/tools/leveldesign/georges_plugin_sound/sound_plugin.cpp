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

#include "sound_plugin.h"
#include "sound_document_plugin.h"
#include "resource.h"
//#include "sound/driver/buffer.h"
#include "nel/misc/vector.h"
#include "nel/misc/path.h"
#include "nel/sound/u_source.h"
#include "loading_dialog.h"
#include "nel/ligo/primitive.h"

#include "nel/sound/sound.h"
#include "nel/sound/simple_sound.h"
#include <afxcmn.h>

using namespace std;
using namespace NLSOUND;
using namespace NLMISC;



namespace NLGEORGES
{



// ***************************************************************************
/*
__declspec( dllexport ) IEditPlugin *IGeorgesEditGetInterface (int version, NLGEORGES::IEdit *globalInterface)
{
	// Same version ?
	if (version == NLGEORGES_PLUGIN_INTERFACE_VERSION)
	{
		return new CSoundPlugin(globalInterface);
	}
	else
	{
		MessageBox (NULL, "Plugin version invalid.", "Sound plugin for georges editor", MB_OK|MB_ICONEXCLAMATION);
		return NULL;
	}

}
*/

// ***************************************************************************

CSoundPlugin::CSoundPlugin(IEdit *globalInterface)
:	_Source(NULL), 
	_Sound(NULL), 
	_FreeSound(false),
	_ActiveDoc(NULL),
	_InvalidSound(false)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Initialize without sheet id bin
	NLMISC::CSheetId::initWithoutSheet();

	CVector dir;

	_GlobalInterface = globalInterface;
/*	_SoundDriver = ISoundDriver::createDriver();
	_Listener = _SoundDriver->createListener();
	_Listener->setRolloffFactor(1.0f);
	_Source = _SoundDriver->createSource();
	dir.set(0, 1, 0);
	_Source->setDirection(dir);
	_Buffer = 0;
*/

	CLoadingDialog	loadDlg;

	loadDlg.Create(IDD_DIALOG_LOADING);
	loadDlg.Message = "";
	loadDlg.UpdateData(FALSE);
	loadDlg.ShowWindow(SW_SHOW);

	CConfigFile &cf = globalInterface->getConfigFile();
	// Add the search path
	try
	{
		CConfigFile::CVar *psearchPath = cf.getVarPtr("SearchPath");
		if (psearchPath != 0)
		{
			for (uint i=0; i<psearchPath->size(); ++i)
			{
				string path = psearchPath->asString(i);
				loadDlg.Message = (string("Adding search path \"")+path+"\"").c_str();
				loadDlg.UpdateData(FALSE);
				loadDlg.RedrawWindow();
				NLMISC::CPath::addSearchPath(path, true, false);
			}
		}
	}
	catch(...)
	{
		nlwarning("Error while setting search path");
	}


	loadDlg.Message = "Initializing Audio Mixer";
	loadDlg.UpdateData(FALSE);
	loadDlg.RedrawWindow();
	// init the mixer
	_Mixer = UAudioMixer::createAudioMixer();
	// the the sample path
	_Mixer->setSamplePath(cf.getVar("SamplePath").asString());
	_Mixer->setPackedSheetOption(cf.getVar("PackedSheetPath").asString(), true);
	// and init
	_Mixer->init(32, true, false, NULL, true);

	_Listener = _Mixer->getListener();

	// load the sample banks.
/*	try
	{
		_Mixer->setSamplePath(cf.getVar("sample_path").asString());
		CConfigFile::CVar *psampleBanks = cf.getVarPtr("load_sample_bank");
		if (psampleBanks != 0)
		{
			for (sint i=0; i<psampleBanks->size(); ++i)
			{
				loadDlg.Message = (string("Loading sample bank \"")+psampleBanks->asString(i)+"\"").c_str();
				loadDlg.UpdateData(FALSE);
				loadDlg.RedrawWindow();
				_Mixer->loadSampleBank(false, psampleBanks->asString(i));
			}
		}
	}
	catch(...)
	{
		nlwarning("Error while loading sample banks");
	}
*/
	// load the sound banks.
/*	try
	{
		CConfigFile::CVar *psoundBanks = cf.getVarPtr("load_sound_bank");
		if (psoundBanks != 0)
		{
			for (sint i=0; i<psoundBanks->size(); ++i)
			{
				loadDlg.Message = (string("Loading sound bank \"")+psoundBanks->asString(i)+"\"").c_str();
				loadDlg.UpdateData(FALSE);
				loadDlg.RedrawWindow();
//				_Mixer->loadSoundBank(psoundBanks->asString(i));
			}
		}
	}
	catch(...)
	{
		nlwarning("Error while loading sound banks");
	}
*/
//	loadDlg.CloseWindow();
	loadDlg.DestroyWindow();
}


// ***************************************************************************

CSoundPlugin::~CSoundPlugin ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	stop();

	_Dialog.DestroyWindow();


//	_Dialog.ShowWindow(SW_HIDE);
//	_Dialog.DestroyWindow();

/*	if (_Buffer != 0)
	{
		delete _Buffer;
	}

*/	if (_Sound != 0 && _FreeSound)
	{
		delete _Sound;
	}

	delete _Mixer;
		
/*
	if (_Listener != 0)
	{
		delete _Listener;
	}

	if (_SoundDriver != 0)
	{
		delete _SoundDriver;
	}
*/
}


bool CSoundPlugin::checkSound(CSound *sound, const vector<pair<string, CSound*> > &subsounds, vector<string> &missingFiles)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	vector<pair<string, CSound*> >::const_iterator first(subsounds.begin()), last(subsounds.end());

	for (; first != last; ++first)
	{
		if (first->second == sound)
			return false;

		if (first->second == 0 && !first->first.empty())
			missingFiles.push_back(first->first);
		else if (first->second != 0)
		{
			vector<pair<string, CSound*> > v2;
			first->second->getSubSoundList(v2);

			if (!checkSound(sound, v2, missingFiles))
				return false;
		}
	}
	return true;
}


// ***************************************************************************

void CSoundPlugin::dialogInit(HWND mainFrm)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Initilize the search path for CPath::lookup
	string searchPath;
	_GlobalInterface->getSearchPath(searchPath);
	CPath::addSearchPath(searchPath, true, true);

	_Dialog.init(this, mainFrm);
}

// ***************************************************************************

bool CSoundPlugin::pretranslateMessage(MSG *pMsg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if ((pMsg->message == WM_KEYDOWN) && (pMsg->wParam == VK_SPACE))
	{
		//MessageBox (NULL, "PLAY!", "Sound plugin", MB_OK);

//		if (_Source)
//		{
//			if (!_Source->isPlaying())
//			{
				play();
/*			}
			else
			{
				stop();
				play();
			}
*///		}
		return true;
	}

	return false;
}

bool CSoundPlugin::hasAlpha()
{
	if (_Sound)
		return _Sound->getSoundType() == CSound::SOUND_SIMPLE;
	else
		return false;
}


void CSoundPlugin::updateDisplay()
{
	if (_Sound != 0)
	{
		if (_Sound->getSoundType() == CSound::SOUND_SIMPLE)
		{
			CSimpleSound *ss = static_cast<CSimpleSound*>(_Sound);
			setMinMaxDistances(ss->getMinDistance(), ss->getMaxDistance());
			_Dialog.setAlpha(static_cast<CSimpleSound*>(_Sound)->getAlpha());
		}
		else
		{
			setMinMaxDistances(1, _Sound->getMaxDistance()); // 1m to max dist.
		}
		_Dialog.setAngles(uint32(180 * _Sound->getConeInnerAngle() / Pi), uint32(180 * _Sound->getConeOuterAngle() / Pi)); 
	}

}



void CSoundPlugin::setActiveDocument(IEditDocument *pdoc)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	stop();
	if (_Source != NULL)
	{
//		_Mixer->removeSource(_Source);
		delete _Source;
		_Source = NULL;
	}
	// release old sound.
	if (_Sound != NULL && _FreeSound)
	{
		delete _Sound;
		_Sound = NULL;
	}
	if (_ActiveDoc != pdoc)
	{
		// we have changed document.
		_InvalidSound = false;
	}

	_ActiveDoc = pdoc;
	if (_ActiveDoc != NULL)
	{
		//std::string filename;
		_ActiveDoc->getFilename(_Filename);
		_Filename = NLMISC::CFile::getFilenameWithoutExtension(_Filename);
		_Dialog.setName(_Filename);

		// 1st, try to found the sound in the preloaded sound bank.
		_Sound = _Mixer->getSoundId(CSheetId(_Filename, "sound"));
		if (_Sound == NULL)
		{
			// not found, create a new one.
			_Sound = CSound::createSound(_Filename, _ActiveDoc->getForm()->getRootNode());
			_FreeSound = true;
		}
		else
		{
			// update the existing sound with the george document
			_Sound->importForm(_Filename, _ActiveDoc->getForm()->getRootNode());
			_FreeSound = false;
		}
		if (_Sound)
		{
			vector<pair<string, CSound*> >	subsounds;
			_Sound->getSubSoundList(subsounds);
			vector<string>	missingFiles;
			bool invalid = !checkSound(_Sound, subsounds, missingFiles);

			if (invalid && !_InvalidSound)
			{
				MessageBox(NULL, "This sound contains an infinite recursion !", "Sound Error", MB_ICONERROR);
			}

			// pre-create the sound to force loading any missing sample bank (thus avoiding unwanted message box)
			_Dialog.fillContextArgs(&_Context);
			_Source = _Mixer->createSource(_Sound, false, NULL, NULL, NULL, NULL);
			if (_Source)
				delete _Source;
			_Source = NULL;

			// recheck the sound
			missingFiles.clear();
			subsounds.clear();
			_Sound->getSubSoundList(subsounds);
			invalid = !checkSound(_Sound, subsounds, missingFiles);

			invalid |= !missingFiles.empty();
			if (!missingFiles.empty() && !_InvalidSound)
			{
/*				// try to load missing sample bank
				for (uint i=0; i<missingFiles.size(); ++i)
				{
					if (missingFiles[i].find(" (sample)") != string::npos)
					{
						// try to find the sample bank
						string sample = missingFiles[i].substr(0, missingFiles[i].find(" (sample)")) + ".wav";
						string path = CPath::lookup(sample, false, false, false);
						if (!path.empty())
						{
							// extract samplebank name
							path = NLMISC::CFile::getPath(path);
							vector<string> rep;
							explode(path, "/", rep, true);

							_Mixer->loadSampleBank(false, rep.back());

							goto retrySound;
						}
					}
				}
*/
				string message("The folowing files are missing for this sound :\n");
				vector<string>::iterator first(missingFiles.begin()), last(missingFiles.end());
				for (;first != last;++first)
				{
						message += (*first)+"\n";
				}

				MessageBox(NULL, message.c_str(), "Sound incomplete", MB_ICONWARNING);
			}
			

			// NB : _InvalidSound and invalid are used to avoid repetitive warning message.
			//		After a first warning message, the message masked until the sound become
			//		valid (ie : no recursion in sound dependance and no missing files).
			_InvalidSound = invalid;


			std::string filename;
			_ActiveDoc->getFilename(filename);
			_Dialog.setFilename(filename);
			_Dialog.fillContextArgs(&_Context);
			_Source = _Mixer->createSource(_Sound, false, NULL, NULL, NULL, &_Context);

			if (_Sound->getSoundType() == CSound::SOUND_SIMPLE)
			{
				CSimpleSound *ss = static_cast<CSimpleSound*>(_Sound);
				setMinMaxDistances(ss->getMinDistance(), ss->getMaxDistance());
			}
			else
			{
				setMinMaxDistances(1, _Sound->getMaxDistance()); // 1m to max dist.
			}
			_Dialog.setAngles(uint32(180 * _Sound->getConeInnerAngle() / Pi), uint32(180 * _Sound->getConeOuterAngle() / Pi)); 
			if (!_InvalidSound)
				_Dialog.setDuration(_Sound->getDuration()); 
			else
				_Dialog.setDuration(0); 
		}
	}
}

void CSoundPlugin::updateEnvFlags(const UAudioMixer::TBackgroundFlags &backgroundFlags)
{
	_Mixer->setBackgroundFlags(backgroundFlags);
}


// ***************************************************************************

void CSoundPlugin::onCreateDocument(IEditDocument *document)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Set text in the global dialog
	string dfn;
	document->getDfnFilename(dfn);

	// Bind an interface on the document
	if (dfn.compare("sound.dfn") == 0)
	{
		document->bind(this, new CSoundDocumentPlugin(this, document));
	}

	stop();
}

// ***************************************************************************

void CSoundPlugin::createNew()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	char BASED_CODE szFilter[] = "Sound (*.sound)|*.sound|All Files (*.*)|*.*||";
	CFileDialog fileDlg(FALSE, ".sound", "*.sound", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);

	if (fileDlg.DoModal() == IDOK)
	{
		string filename = (const char*) fileDlg.GetPathName();
		_GlobalInterface->createDocument("sound.dfn", filename.c_str());
	}
}

// ***************************************************************************

void CSoundPlugin::activate(bool activate)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (IsWindow(_Dialog))
	{
		_Dialog.ShowWindow(activate? SW_SHOW : SW_HIDE);
	}

}

// ***************************************************************************

void CSoundPlugin::getPluginName(std::string &name)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	name = "Sound plugin";
}

// ***************************************************************************

void CSoundPlugin::play(std::string &filename)
{
	try
	{
		stop();
		// force to recreate the sound (mainly for contextual sound)
		setActiveDocument(_ActiveDoc);

		if (_Sound != NULL && !_InvalidSound)
		{
			// if it is a context source, recreate the source each time
			if( _Sound->getSoundType() == CSound::SOUND_CONTEXT)
			{
				if (_Source != NULL)
//					_Mixer->removeSource(_Source);
					delete _Source;
				_Dialog.fillContextArgs(&_Context);
				_Source = _Mixer->createSource(_Sound, false, NULL, NULL, NULL, &_Context);
			}
			
			if (_Sound->getSoundType() == CSound::SOUND_BACKGROUND)
			{
				// this is a background sound, we must create a pseudo environnent for baground sound
				NLLIGO::CPrimRegion	region;

				region.Name = "simulation";

				NLLIGO::CPrimPoint	point;
				point.Point = NLMISC::CVector::Null;
//				point.Name = string("simulation-")+_Sound->getName()+"-000";

				region.VPoints.push_back(point);
				string name = string("simulation-")+NLMISC::CFile::getFilenameWithoutExtension(_Sound->getName().toString())+"-000";
				if (region.VPoints.back().checkProperty("name"))
					region.VPoints.back().removePropertyByName("name");

				region.VPoints.back().addPropertyByName("name", new NLLIGO::CPropertyString(name));
				

				// TODO : repair this
//				_Mixer->loadBackgroundSoundFromRegion(region);
				_Mixer->setBackgroundFilterFades(_Dialog.FilterFades);
				_Mixer->playBackgroundSound();
				_Mixer->setBackgroundFilterFades(_Dialog.FilterFades);
				_PlayBackground = true;
			}
			else if (_Source)
			{
				_Source->setPos (CVector(0,0,0));
				_Source->play();
				_Dialog.setPlaying(true);
			}
		}

/*		if (_Buffer != 0)
		{
			_Source->setStaticBuffer(0);
			delete _Buffer;
			_Buffer = 0;
		}

		if (filename.empty())
		{
			return;
		}

		_Buffer = _SoundDriver->createBuffer();

		string path = CPath::lookup(filename, true, true, true);
		_SoundDriver->loadWavFile(_Buffer, path.c_str());

		_Source->setStaticBuffer(_Buffer);
		_Source->play();
*/
	}
	catch (ESoundDriver& e)
	{
		string reason = e.what();
		MessageBox (NULL, reason.c_str(), "Sound plugin", MB_OK);
	}
}

// ***************************************************************************

void CSoundPlugin::stop()
{
	if (_Sound && _Sound->getSoundType() == CSound::SOUND_BACKGROUND)
	{
		_Mixer->stopBackgroundSound();
		_PlayBackground = false;
	}
	else if (_Source)
	{
		if (_Source->isPlaying())
		{
			_Dialog.setPlaying(false);
			_Source->stop();
		}
	}
}

// ***************************************************************************

uint32 CSoundPlugin::getTime()
{
	if ((_Source == 0) /*|| !_Source->isPlaying()*/)
	{
		return 0;
	}
	else
	{
		return _Source->getTime();
	}
}

void CSoundPlugin::update()
{
	_Mixer->update();
}

bool CSoundPlugin::isPlaying()
{
	if (_Sound && _Sound->getSoundType() == NLSOUND::CSound::SOUND_BACKGROUND)
		return _PlayBackground;
	else if (_Source)
		return _Source->isPlaying();
	else
		return false;
}


void CSoundPlugin::reloadSamples()
{
/*	CLoadingDialog	loadDlg;
	loadDlg.Create(IDD_DIALOG_LOADING);
	loadDlg.Message = "Reloading sample banks...";
	loadDlg.UpdateData(FALSE);
	loadDlg.ShowWindow(SW_SHOW);
*/
	_Mixer->reloadSampleBanks(false);

//	loadDlg.CloseWindow();
//	loadDlg.DestroyWindow();
}

void CSoundPlugin::reloadSounds()
{
//	_Mixer->reloadSounds();
}



} // namespace NLGEORGES
