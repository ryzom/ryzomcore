// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

//
// Includes
//

#include <nel/misc/types_nl.h>

#include <list>

#include <nel/misc/event_listener.h>
#include <nel/misc/command.h>
#include <nel/misc/log.h>
#include <nel/misc/displayer.h>
#include <nel/misc/aabbox.h>

#include <nel/3d/u_driver.h>
#include <nel/3d/u_scene.h>
#include <nel/3d/u_skeleton.h>
#include <nel/3d/u_play_list.h>
#include <nel/3d/u_animation_set.h>
#include <nel/3d/u_animation.h>
#include <nel/3d/u_play_list_manager.h>
#include <nel/3d/u_play_list.h>
#include <nel/3d/u_transform.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_text_context.h>

#include "animation.h"
#include "entities.h"
#include "snowballs_client.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NL3D;

namespace SBCLIENT {

//
// Constantes
//

// Amount of time for the transistion between 2 animations
CAnimationTime TransitionTime = 0.25f;

//
// Variables
//

UAnimationSet *AnimationSet = NULL;
UPlayListManager *PlayListManager = NULL;

struct Anim
{
	const char	*Name;
	bool		 Loop;
	uint		 Id;
	UAnimation	*Animation;
};

Anim AnimIdArray[][2] =
{
	{ { "patterfeet.anim", false, 0, NULL }, { "", false, 0, NULL } },
	{ { "marche.anim", true, 0, NULL }, { "", false, 0, NULL } },
	{ { "idle.anim", true, 0, NULL }, { "", false, 0, NULL } },
	{ { "log_on.anim", false, 0, NULL }, { "", false, 0, NULL } },
	{ { "log_off.anim", false, 0, NULL }, { "", false, 0, NULL } },
	{ { "lancelaboule.anim", false, 0, NULL }, { "", false, 0, NULL } },
	{ { "prepaboule.anim", false, 0, NULL }, { "", false, 0, NULL } },
	{ { "prepaboulecycle.anim", true, 0, NULL }, { "", false, 0, NULL } },
	{ { "impact.anim", false, 0, NULL }, { "", false, 0, NULL } },
};


//
// Functions
//

void	computeAnimation (CEntity &entity, EAnim anim)
{
	// Get the current time
	double currentTime = AnimationTime;

//	nlinfo ("%d playing animation", anim);
//	nlinfo ("%d playing animation %s ct%f st%f et%f", anim, AnimIdArray[anim][0].Name, currentTime, AnimIdArray[anim][0].Animation->getBeginTime (), AnimIdArray[anim][0].Animation->getEndTime ());

	// Find the new slot for the full animation (0 or 1)
	uint newSlot = entity.NextEmptySlot;
	uint oldSlot = 1 - entity.NextEmptySlot;
	entity.NextEmptySlot = 1 - entity.NextEmptySlot;

	UPlayList::TWrapMode wrapMode = AnimIdArray[anim][0].Loop ? UPlayList::Repeat : UPlayList::Clamp;
	
	entity.PlayList->setAnimation			(newSlot, AnimIdArray[anim][0].Id);
	entity.PlayList->setTimeOrigin			(newSlot, currentTime);
	entity.PlayList->setWeightSmoothness	(newSlot, 1.0f);
	entity.PlayList->setWrapMode			(newSlot, wrapMode);

	double OldStartTime, OldEndTime;
	double NewStartTime, NewEndTime;

	// Get the starting time of the old animation slot
	entity.PlayList->getStartWeight	(oldSlot, OldStartTime);
	
	// Compute the time delta between start of the old animation and now
	double dt = currentTime - OldStartTime;

	// Compute the new transition value depending of the current time

	if (dt > TransitionTime)
		dt = TransitionTime;

	OldStartTime = currentTime - (TransitionTime - dt);
	OldEndTime = currentTime + dt;
		
	NewStartTime = currentTime;
	NewEndTime = currentTime + dt;

	// Set new weights on the old and the new animation slot

	entity.PlayList->setStartWeight	(oldSlot, 1.0f, OldStartTime);
	entity.PlayList->setEndWeight	(oldSlot, 0.0f, OldEndTime);

	entity.PlayList->setStartWeight	(newSlot, 0.0f, NewStartTime);
	entity.PlayList->setEndWeight	(newSlot, 1.0f, OldEndTime);

	// Keep in mind what is the last animation id we set
	entity.StartAnimationTime = (float)currentTime;
}


void	playAnimation (CEntity &entity, EAnim anim, bool force)
{
	nlassert (anim > -2 && anim < 20);
//	nlinfo ("playAnimation() %d", anim);

	// Get the current time
	CAnimationTime currentTime = AnimationTime;

	// Can't do animation without skeleton
	if (entity.Skeleton.empty())
		return;

	// If the first time we play an animation, creates the animation class
	if (entity.PlayList == NULL)
		createAnimation (entity);

	if (force || entity.AnimQueue.empty())
	{
		computeAnimation (entity, anim);
		
		// clear the animation queue
		//nlinfo ("clearing animation queue");
		while (!entity.AnimQueue.empty())
			entity.AnimQueue.pop ();
	}

//	nlinfo ("pushing animation %d", anim);
//	nlinfo ("pushing animation %s", AnimIdArray[anim][0].Name);
	entity.AnimQueue.push (anim);
}

void	createAnimation (CEntity &entity)
{
	nlassert (!entity.Instance.empty() && !entity.Skeleton.empty() && AnimationSet != NULL);

	entity.PlayList = PlayListManager->createPlayList (AnimationSet);
	entity.PlayList->registerTransform (entity.Instance);
	entity.PlayList->registerTransform (entity.Skeleton);
}

void	deleteAnimation (CEntity &entity)
{
	if (entity.PlayList == NULL)
		return;

	PlayListManager->deletePlayList (entity.PlayList);
	entity.PlayList= NULL;
}


void	initAnimation()
{
	AnimationSet = Driver->createAnimationSet ();
	
	// Add all animations in the animation set
	for (uint i = 0; i < sizeof (AnimIdArray) / sizeof (AnimIdArray[0]); i++)
	{
		if (AnimIdArray[i][0].Name[0] != '\0')
		{
			AnimIdArray[i][0].Id = AnimationSet->addAnimation (AnimIdArray[i][0].Name, AnimIdArray[i][0].Name);
			AnimIdArray[i][0].Animation = AnimationSet->getAnimation (AnimIdArray[i][0].Id);
		}

		if (AnimIdArray[i][1].Name[0] != '\0')
		{
			AnimIdArray[i][1].Id = AnimationSet->addAnimation (AnimIdArray[i][1].Name, AnimIdArray[i][1].Name);
			AnimIdArray[i][1].Animation = AnimationSet->getAnimation (AnimIdArray[i][1].Id);
		}
	}
	AnimationSet->build ();

	PlayListManager = Scene->createPlayListManager ();
}

void	updateAnimation()
{
	// Get the current time
	CAnimationTime currentTime = AnimationTime;

	for (EIT eit = Entities.begin (); eit != Entities.end (); eit++)
	{
		CEntity	&entity = (*eit).second;

		if (entity.AnimQueue.empty ())
		{
//			nlwarning ("empty queue update!!!");
			continue;
		}

		EAnim currentAnim = entity.AnimQueue.front ();
		if (!AnimIdArray[currentAnim][0].Loop && currentTime >= entity.StartAnimationTime + AnimIdArray[currentAnim][0].Animation->getEndTime () - TransitionTime/2)
		{
			// remove the current anim
			entity.AnimQueue.pop ();

			if (entity.AnimQueue.empty ())
			{
//				nlwarning ("empty queue!!!!!!");
				continue;
			}

			EAnim newAnim = entity.AnimQueue.front ();

			computeAnimation (entity, newAnim);
/*
			nlinfo ("playing animation %s ct%f st%f et%f", AnimIdArray[newAnim][0].Name, currentTime, AnimIdArray[newAnim][0].Animation->getBeginTime (), AnimIdArray[newAnim][0].Animation->getEndTime ());
			// setup the new anim
			entity.PlayList->setAnimation (0, AnimIdArray[newAnim][0].Id);
			entity.PlayList->setTimeOrigin (0, currentTime);
			entity.PlayList->setStartWeight (0, 1.0f, currentTime);
			entity.PlayList->setEndWeight (0, 1.0f, currentTime+TransitionTime);
			entity.PlayList->setWeightSmoothness (0, 1.0f);

			if (AnimIdArray[newAnim][0].Loop)
				entity.PlayList->setWrapMode (0, UPlayList::Repeat);
			else
				entity.PlayList->setWrapMode (0, UPlayList::Clamp);

			entity.StartAnimationTime = currentTime;
*/		}
	}

	// compute new animation position depending of the current time
	PlayListManager->animate (AnimationTime);
}

void	releaseAnimation()
{
	Scene->deletePlayListManager (PlayListManager);

// The next line doesn t work (say that AnimationSet is not a valid AnimationSet Ptr) so we comment it.
//	Scene->deleteAnimationSet (AnimationSet);
}

} /* namespace SBCLIENT */

/* end of file */
