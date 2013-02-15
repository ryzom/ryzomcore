/**
 * \file globals.h
 * \date November 2004
 * \author Matt Raykowski
 * \author Henri Kuuste
 */

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

// global includes

#include <nel/misc/types_nl.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <list>
#include <deque>
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <algorithm>
#include <exception>

#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/debug.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>
#include <nel/misc/common.h>
#include <nel/misc/events.h>
#include <nel/misc/fast_mem.h>
#include <nel/misc/config_file.h>
#include <nel/misc/system_info.h>
#include <nel/misc/mem_displayer.h>

#include <nel/3d/u_scene.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
#include <nel/3d/u_texture.h>
#include <nel/3d/u_instance.h>
#include <nel/3d/u_text_context.h>
#include <nel/3d/u_particle_system_instance.h>

static const float GScale = 0.01f;

