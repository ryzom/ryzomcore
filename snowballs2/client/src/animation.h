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

#ifndef ANIMATION_H
#define ANIMATION_H

namespace SBCLIENT {

//
// External definitions
//

class CEntity;

//
// External variables
//

// Animation id used by playAnimation()
enum EAnim { NoAnim = -1, PrepareWalkAnim, WalkAnim, IdleAnim, LogInAnim, LogOffAnim, ThrowSnowball, PrepareSnowBall, PrepareSnowBallCycle, HitAnim };

//
// External functions
//

// This function is automatically called by playAnimation() the first time
void	createAnimation (CEntity &entity);

// Change the animation of an entity. You have to give the animation id
void	playAnimation (CEntity &entity, EAnim anim,  bool force = false);

// This function must be called before releasing the entity to delete the entity animation
void	deleteAnimation (CEntity &entity);

void	initAnimation ();
void	updateAnimation ();
void	releaseAnimation ();

} /* namespace SBCLIENT */

#endif // ANIMATION_H

/* End of animation.h */
