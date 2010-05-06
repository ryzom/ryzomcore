/**
 * \file game_time.cpp
 * \brief CGameTime
 * \date 2008-11-26 14:44GMT
 * \author Jan Boon (Kaetemi)
 * CGameTime
 */

/* 
 * Copyright (C) 2008  by authors
 * 
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 * 
 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include <nel/misc/types_nl.h>
#include "game_time.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/value_smoother.h>
#include <nel/misc/config_file.h>
#include <nel/misc/time_nl.h>

// Project includes
#include "snowballs_client.h"
#include "configuration.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace SBCLIENT {

// impl note: at some point animation time can be linked to server as well
// see version of time.cpp in snowballs5 experimental code

static bool _SkipAnimationOnce;
static NLMISC::TTime _TimeMs;
static CValueSmootherTemplate<float> _FpsSmoother;

static void cbFpsSmoothing(CConfigFile::CVar &var)
{
	_FpsSmoother.init((uint)var.asInt());
}

void CGameTime::init()
{
	_SkipAnimationOnce = true; // since we're loading ... ;)
	_TimeMs = NLMISC::CTime::getLocalTime();
	LocalTime = ((TLocalTime)_TimeMs) / 1000.0;
	LocalTimeDelta = 0.0;
	AnimationTime = 0.0;
	AnimationTimeDelta = 0.f;
	FramesPerSecond = 0.f;
	FramesPerSecondSmooth = 0.f;
	CConfiguration::setAndCallback("FpsSmoothing", cbFpsSmoothing);
}

void CGameTime::release()
{
	// could also call init again here just for fun but nothing useful otherwise
	CConfiguration::dropCallback("FpsSmoothing");
	_FpsSmoother.reset();
}

void CGameTime::updateTime()
{
	TTime timems = NLMISC::CTime::getLocalTime();
	TTime deltams = timems - _TimeMs;
	_TimeMs = timems;

	if (!deltams) // time has not moved
	{
		// average of previous fps and this fps should be ok
		FramesPerSecond *= 3;

		LocalTimeDelta = 0.f;
		AnimationTimeDelta = 0.f;
	}
	else
	{
		FramesPerSecond = 1000.0f / (float)deltams;

		TLocalTime localTime = ((TLocalTime)timems) / 1000.0;
		LocalTimeDelta = localTime - LocalTime;
		LocalTime = localTime;

		if (_SkipAnimationOnce)
		{
			AnimationTimeDelta = 0.f;
			_SkipAnimationOnce = false;
		}
		else
		{
			AnimationTimeDelta = (TAnimationTime)LocalTimeDelta;
			AnimationTime += (TGlobalAnimationTime)LocalTimeDelta;
		}
	}

	_FpsSmoother.addValue(FramesPerSecond);
	FramesPerSecondSmooth = _FpsSmoother.getSmoothValue();
}

void CGameTime::skipAnimationOnce()
{
	_SkipAnimationOnce = true;
}

} /* namespace SBCLIENT */

/* end of file */
