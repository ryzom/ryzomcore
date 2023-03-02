/** \file u_decal.h
 * TODO: File description
 *
 * $Id$
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_U_DECAL_H
#define NL_U_DECAL_H

#include "nel/misc/types_nl.h"
#include "nel/3d/viewport.h"
#include "nel/3d/scissor.h"
#include "nel/3d/frustum.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/matrix.h"
#include "nel/misc/rgba.h"
#include "nel/misc/rect.h"
#include "nel/misc/bitmap.h"
#include "nel/misc/event_server.h"
#include "nel/misc/event_listener.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/3d/primitive_profile.h"
#include "nel/3d/u_transform.h"



namespace NL3D
{

class UDecal : public UTransform
{
public:	

	/// Constructors
	UDecal() { _Object = NULL; }
	UDecal(class CDecal *object) { _Object = (ITransformable*)object; };
	/// Attach an object to this proxy
	void			attach(class CDecal *object) { _Object = (ITransformable*)object; }
	/// Detach the object
	void			detach() { _Object = NULL; }
	/// Return true if the proxy is empty() (not attached)
	bool			empty() const {return _Object==NULL;}
	/// For advanced usage, get the internal object ptr
	class CDecal	*getObjectPtr() const {return (CDecal*)_Object;}

};


} //NL3D

#endif
