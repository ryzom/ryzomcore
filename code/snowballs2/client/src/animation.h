/** \file animation.h
 * Animation interface between the game and NeL
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX SNOWBALLS.
 * NEVRAX SNOWBALLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX SNOWBALLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX SNOWBALLS; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

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
