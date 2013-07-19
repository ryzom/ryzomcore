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



#ifndef RZ_INTERFACE_ANIM_H
#define RZ_INTERFACE_ANIM_H

#include "nel/gui/interface_property.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/interface_link.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/u_track.h"

namespace NLGUI
{

	/**
	 * class managing an animation track
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2003
	 */
	class CInterfaceTrack
	{

	public:

		CInterfaceTrack();
		virtual ~CInterfaceTrack();

		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

		void update (double currentTime);

		bool isDynamic () { return _Dynamic; }

		void eval(); // Evaluate dynamic keys

	private:

		enum ETrackType
		{
			Track_Linear,
			Track_TCB,
			Track_Bezier
		};

		struct SDynKey
		{
			std::string Time;
			std::string Value;
			std::string InTan;
			std::string OutTan;
			std::string Step;
			std::string Tension;
			std::string Continuity;
			std::string Bias;
			std::string EaseTo;
			std::string EaseFrom;
		};

		bool					_Dynamic;
		std::vector<SDynKey>	_DynKeys;

		ETrackType _Type;
		NL3D::UTrackKeyframer *_TrackKeyFramer;
		std::vector<CInterfaceLink::CTargetInfo> _Targets;
	};

	/**
	 * class managing an animation of the interface
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2003
	 */
	class CInterfaceAnim
	{

	public:

		CInterfaceAnim();
		virtual ~CInterfaceAnim();

		virtual bool parse (xmlNodePtr cur, CInterfaceGroup *parentGroup);

		void update();
		void start();
		void stop();

		bool isFinished() { return _Finished; }
		bool isDisableButtons() { return _DisableButtons; }

	protected:

		CInterfaceGroup *_Parent;

		// Parsed properties
		double _Duration;
		bool _DisableButtons;

		std::string _AHOnFinish;
		std::string _AHOnFinishParams;

		std::string _Id;

		std::vector<CInterfaceTrack*> _Tracks;

		// Current anim
		double _CurrentTime;
		bool _Finished;
		bool _AnimHasToBeStopped;

	};

}

#endif // NL_INTERFACE_ANIM_H

/* End of interface_anim.h */


