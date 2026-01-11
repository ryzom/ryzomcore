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

#include "sound_document_plugin.h"
#include "resource.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include <mmsystem.h>
#include <math.h>

using namespace std;
using namespace NLMISC;

namespace NLGEORGES
{


// ***************************************************************************

void CSoundDocumentPlugin::dialogInit(HWND documentView)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	updateInfo(true);
}

// ***************************************************************************

bool CSoundDocumentPlugin::pretranslateMessage(MSG *pMsg)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return false;
}

// ***************************************************************************

void CSoundDocumentPlugin::activate(bool activated)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (activated)
	{
		updateInfo(true);

		_Plugin->setActiveDocument(_Document);

	}
	else
	{
		_Plugin->stop();
	}
}

// ***************************************************************************

void CSoundDocumentPlugin::onValueChanged(const char *formName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	updateInfo(true);
}

// ***************************************************************************

void CSoundDocumentPlugin::onNodeChanged()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	updateInfo(false);
}

// ***************************************************************************
//
//
//  FIXME: Should be using CSound.importForm()
//
//
//
void CSoundDocumentPlugin::updateInfo(bool stopSound)
{
	// Filename
//	_Document->getForm()->getRootNode().getValueByName(_Filename, ".SoundType.Filename");

/*	// InternalConeAngle
	_Document->getForm()->getRootNode().getValueByName(_InnerAngle, ".InternalConeAngle");
	if (_InnerAngle > 360)
	{
		_InnerAngle = 360;
	}
	float inner = (float) (Pi * _InnerAngle / 180.0f);  // convert to radians

	// ExternalConeAngle
	_Document->getForm()->getRootNode().getValueByName(_OuterAngle, ".ExternalConeAngle");
	if (_OuterAngle > 360)
	{
		_OuterAngle = 360;
	}
	float outer = (float) (Pi * _OuterAngle / 180.0f);  // convert to radians
*/
/*	// Loop
	_Document->getForm()->getRootNode().getValueByName(_Loop, ".Loop");

	// Gain
	_Document->getForm()->getRootNode().getValueByName(_Gain, ".Gain");
	if (_Gain > 0)
	{
		_Gain = 0;
	}
	if (_Gain < -100)
	{
		_Gain = -100;
	}
	float ampGain = (float) pow(10.0, _Gain / 20.0); // convert dB to linear gain
*/
/*	// External gain
	_Document->getForm()->getRootNode().getValueByName(_ExternalGain, ".ExternalGain");
	if (_ExternalGain > 0)
	{
		_ExternalGain = 0;
	}
	if (_ExternalGain < -100)
	{
		_ExternalGain = -100;
	}
	float ampExtGain = (float) pow(10.0, _ExternalGain / 20.0); // convert dB to linear gain
*/
/*
	// Transpoee
	_Document->getForm()->getRootNode().getValueByName(_Transpose, ".Transpose");
	float pitch =  (float) pow(1.0594630943592952645618252949463, _Transpose); // convert semi-tones to playback speed
*/
	// MinDistance
/*	float mindist;
	_Document->getForm()->getRootNode().getValueByName(mindist, ".SoundType.MinDistance");

	// MaxDistance
	float maxdist;
	_Document->getForm()->getRootNode().getValueByName(maxdist, ".SoundType.MaxDistance");

	// Alpha
	float alpha;
	_Document->getForm()->getRootNode().getValueByName(alpha, ".SoundType.Alpha");
*/
	// Send the values down the drain

	// For the source
//	_Plugin->setLoop(_Loop);
//	_Plugin->setGain(ampGain);
//	_Plugin->setCone(inner, outer, ampExtGain);
//	_Plugin->setPitch(pitch);
//	_Plugin->setMinMaxDistances(mindist, maxdist);
//	_Plugin->setAlpha(alpha);


	// For the dialog
	string filename;
	_Document->getFilename(filename);
	string name = NLMISC::CFile::getFilenameWithoutExtension(filename);

	if( stopSound)
		_Plugin->setActiveDocument(_Document);

	_Plugin->updateDisplay();

//	_Plugin->setName(name);
//	_Plugin->setFilename(_Filename);
//	_Plugin->setAngles(_InnerAngle, _OuterAngle);


}



} // namespace NLGEORGES