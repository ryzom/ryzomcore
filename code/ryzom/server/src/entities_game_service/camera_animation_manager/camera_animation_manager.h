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

#ifndef RY_CAMERAANIMATIONMANAGER_H
#define RY_CAMERAANIMATIONMANAGER_H

#include "nel/ligo/primitive.h"
#include <string>
#include "camera_animation_manager/camera_animation_step_factory.h"
#include "nel/misc/entity_id.h"

/************************************************************************/
/* Class that manages the camera animations. (singleton).
 * It's responsible of :
 * - Parsing camera animations in primitives
 * - Sending a specified animation to the client
 * 
 * \author Fabien Henon
 * \date 2012
 */
/************************************************************************/
class CCameraAnimationManager
{
public:
	/// Gets the current instance of the manager
	static CCameraAnimationManager* getInstance() { return _Instance; }
	/// Creates the instance of the manager and parse the animations
	static void init();
	/// Releases the animations
	static void release();

	/// Function that sends all the instructions of a camera animation to the specified entity
	void sendAnimation(const NLMISC::CEntityId& eid, const std::string& animationName);

private:
	/// Constructor
	CCameraAnimationManager();
	/// Destructor
	~CCameraAnimationManager();

	/// Function that parses the camera animations
	bool parseCameraAnimations(const NLLIGO::IPrimitive* prim, const std::string& filename);

	/// Instance of the manager
	static CCameraAnimationManager* _Instance;

	/// Class that contains information about an animation
	class TCameraAnimInfo
	{
	public:
		TCameraAnimInfo()
		{
			Name = "";
		}
		/// Function called to release the animations
		void release()
		{
			// We delete the camera animation steps
			for (std::vector<ICameraAnimationStep*>::iterator it = Steps.begin(); it != Steps.end(); ++it)
			{
				ICameraAnimationStep* step = *it;
				delete step;
			}
			Steps.clear();

			Name = "";
		}
		/// Function called to send the camera animation instruction specified by the current step to the client
		void sendAnimationStep(const NLMISC::CEntityId& eid, int currentStep);
		/// Function called to send all the camera animation instructions to the client
		void sendAnimationSteps(const NLMISC::CEntityId& eid);

		std::string Name;
		std::vector<ICameraAnimationStep*> Steps;

	private:
		class TCameraAnimTimerEvent: public CTimerEvent
		{
		public:
			TCameraAnimTimerEvent(TCameraAnimInfo* info, int nextStep, const NLMISC::CEntityId& eid): _Eid(eid)
			{
				_Infos = info;
				_Next = nextStep;
			}
			// Callback called when the timer finished
			void timerCallback(CTimer* owner)
			{
				// We tell the camera anim info to send the current step
				_Infos->sendAnimationStep(_Eid, _Next);
			}

		private:
			TCameraAnimInfo* _Infos;
			int _Next;
			const NLMISC::CEntityId& _Eid;
		};
	};

	typedef std::map<std::string, TCameraAnimInfo> TCameraAnimationContainer;

	/// Variable that contains the animations
	TCameraAnimationContainer Animations;
};


#endif /* RY_CAMERAANIMATIONMANAGER_H */
