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


#include "nel/3d/i_program.h"

namespace NL3D
{
	const char *IProgram::uniformNames[ NUM_UNIFORMS ] =
	{
		"mvpMatrix",
		"mvMatrix",
		"normalMatrix",
		"texMatrix0",
		"texMatrix1",
		"texMatrix2",
		"texMatrix3",
		"constant0",
		"constant1",
		"constant2",
		"constant3",
		"diffuse",
		"mcolor",
		"sampler0",
		"sampler1",
		"sampler2",
		"sampler3",
		"alphaTreshold",
		"fogStart",
		"fogEnd",
		"fogColor",
		"fogDensity",
		"light0Dir",
		"light1Dir",
		"light2Dir",
		"light3Dir",
		"light4Dir",
		"light5Dir",
		"light6Dir",
		"light7Dir",
		"light0ColDiff",
		"light1ColDiff",
		"light2ColDiff",
		"light3ColDiff",
		"light4ColDiff",
		"light5ColDiff",
		"light6ColDiff",
		"light7ColDiff",
		"light0ColAmb",
		"light1ColAmb",
		"light2ColAmb",
		"light3ColAmb",
		"light4ColAmb",
		"light5ColAmb",
		"light6ColAmb",
		"light7ColAmb",
		"light0ColSpec",
		"light1ColSpec",
		"light2ColSpec",
		"light3ColSpec",
		"light4ColSpec",
		"light5ColSpec",
		"light6ColSpec",
		"light7ColSpec",
		"light0Shininess",
		"light1Shininess",
		"light2Shininess",
		"light3Shininess",
		"light4Shininess",
		"light5Shininess",
		"light6Shininess",
		"light7Shininess",
		"light0Pos",
		"light1Pos",
		"light2Pos",
		"light3Pos",
		"light4Pos",
		"light5Pos",
		"light6Pos",
		"light7Pos",
		"light0ConstAttn",
		"light1ConstAttn",
		"light2ConstAttn",
		"light3ConstAttn",
		"light4ConstAttn",
		"light5ConstAttn",
		"light6ConstAttn",
		"light7ConstAttn",
		"light0LinAttn",
		"light1LinAttn",
		"light2LinAttn",
		"light3LinAttn",
		"light4LinAttn",
		"light5LinAttn",
		"light6LinAttn",
		"light7LinAttn",
		"light0QuadAttn",
		"light1QuadAttn",
		"light2QuadAttn",
		"light3QuadAttn",
		"light4QuadAttn",
		"light5QuadAttn",
		"light6QuadAttn",
		"light7QuadAttn"
	};
}

