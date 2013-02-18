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
#include "nel/misc/time_nl.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/file.h"
#include "nel/3d/u_camera.h"
#include "nel/gui/action_handler.h"
#include "client_cfg.h"
#include "view.h"
//
#include <algorithm>


using namespace NLMISC;
using namespace NL3D;
using namespace NLGUI;

// TODO nico : that stuff was coded in a hurry, but could be good to add it to NL3D with a better packaging later...


//extern UCamera	MainCam;
extern CView View;


// state ofthe recorder
enum TState
{
	Idle,
	Recording,
	PlayBack
};


// camera sample
class CSample
{
public:
	TTime		Date;
	CVector     Pos;
	CVector     Heading;
	//CVMatrix     Matrix;
public:
	void serial(NLMISC::IStream &f) throw(EStream)
	{
		f.serial(Date);
		f.serial(Pos);
		f.serial(Heading);
		//f.serial(Matrix);
	}
};

bool operator<(const CSample &lhs, const CSample &rhs)
{
	return lhs.Date < rhs.Date;
}


typedef std::vector<CSample> TTrack;


// recorder
static TState		 State = Idle;   // current state of the camera recorder
static TTime		 TimeOrigin = 0;  // time origin for record or playback
static TTrack		 Track;



bool isRecordingCamera()
{
	return State == Recording;
}


void updateCameraRecorder()
{
	switch(State)
	{
		case  Idle:
			// no-op
		break;
		case Recording:
		{
			CSample sample;
			sample.Date = CTime::getLocalTime() - TimeOrigin;
			//sample.Matrix = MainCam.getMatrix();
			sample.Pos = View.viewPos();
			sample.Heading = View.view();
			if (Track.empty())
			{
				// push first sample
				Track.push_back(sample);
			}
			else  if (sample.Date > Track.back().Date)
			{
				// push a new sample only if more recent
				Track.push_back(sample);
			}
		}
		break;
		case PlayBack:
		{
			if (Track.empty())
			{
				State = Idle;
				break;
			}
			TTime date = CTime::getLocalTime() - TimeOrigin;
			CSample compVal;
			compVal.Date = date;
			TTrack::const_iterator it = std::lower_bound(Track.begin(), Track.end(), compVal);
			if (it == Track.end())
			{
				//MainCam.setMatrix(Track.back().Matrix);
				View.viewPos(Track.back().Pos);
				View.view(Track.back().Heading);
				State = Idle;
				break;
			}
			if (it == Track.begin())
			{
				//MainCam.setMatrix(Track.back().Matrix);
				View.viewPos(Track.front().Pos);
				View.view(Track.front().Heading);
				break;
			}
			const CSample &s0 = *(it - 1);
			const CSample &s1 = *it;
			float lambda = float(date  - s0.Date) / float(s1.Date  - s0.Date);
			//
			CVector pos;
			CVector heading;
			//
			if (ClientCfg.CameraRecorderBlend)
			{
				pos     = lambda * s1.Pos + (1.f - lambda) * s0.Pos;
				heading = (lambda * s1.Heading + (1.f - lambda) * s0.Heading).normed();
			}
			else
			{
				pos     = s0.Pos;
				heading = s0.Heading;
			}

			View.viewPos(pos);
			View.view(heading);


			/*
			CVector pos = lambda * s1.Matrix.getPos() + (1.f - lambda) * s0.Matrix.getPos();
			CVector I = lambda * s1.Matrix.getI() + (1.f - lambda) * s0.Matrix.getI();
			I.normalize();
			CVector J = lambda * s1.Matrix.getJ() + (1.f - lambda) * s0.Matrix.getJ();
			J = (J - (J * I) * I).normed();
			CVector K = I ^ J;
			CMatrix mat;
			mat.setPos(pos);
			mat.setRot(I, J, K);
			UTransform::TTransformMode oldMode = MainCam.getTransformMode();
			MainCam.setTransformMode(UTransformable::DirectMatrix);
			MainCam.setMatrix(mat);
			MainCam.setTransformMode(oldMode);
			*/
		}
		break;
	}
};

// ***************************************************************************
class CAHToggleCameraRecorder : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (State == Recording)
		{
			State = Idle;
			return;
		}
		State = Recording;
		TimeOrigin = CTime::getLocalTime();
		Track.clear();
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHToggleCameraRecorder, "toggle_camera_recorder");

// ***************************************************************************
class CAHCameraRecorderPlayback : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		if (State == PlayBack)
		{
			State = Idle;
			return;
		}
		State = PlayBack;
		TimeOrigin = CTime::getLocalTime();
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHCameraRecorderPlayback, "camera_recorder_playback");

// ***************************************************************************
class CAHSaveCameraRecord : public IActionHandler
{
	virtual void execute (CCtrlBase * /* pCaller */, const string &/* Params */)
	{
		try
		{
			std::string filename = ClientCfg.CameraRecorderPrefix + ".cr";
			if (!ClientCfg.CameraRecorderPath.empty())
			{
				filename = ClientCfg.CameraRecorderPath + "/" + filename;
			}
			filename = CFile::findNewFile(filename);
			if (!filename.empty())
			{
				COFile f(filename);
				f.serialVersion(0);
				f.serialCont(Track);
			}
			else
			{
				nlwarning("Couldn't compute camera recorder next filename");
			}
		}
		catch(const EStream &e)
		{
			nlwarning(e.what());
		}
	}
};
// ***************************************************************************
REGISTER_ACTION_HANDLER (CAHSaveCameraRecord, "save_camera_record");


// ***************************************************************************
NLMISC_COMMAND(loadCamRec, "Load a camera path record file (.cr)", "<filename>")
{
	if(args.empty())
		return false;
	std::string filename = args[0];
	string::size_type pos = args[0].find_last_of ('.');
	if (pos == string::npos)
	{
		filename += ".cr";
	}
	std::string path;
	std::string searchFilename = filename;
	if (!ClientCfg.CameraRecorderPath.empty())
	{
		searchFilename = ClientCfg.CameraRecorderPath + "/" + searchFilename;
	}
	if (CFile::fileExists(searchFilename))
	{
		path = searchFilename;
	}
	else
	{
		path = CPath::lookup(filename, false, true);
	}
	if (!path.empty())
	{
		try
		{
			CIFile f(path);
			f.serialVersion(0);
			f.serialCont(Track);
			State = Idle;
		}
		catch(const EStream &e)
		{
			nlwarning(e.what());
		}
	}
	return true;
}











