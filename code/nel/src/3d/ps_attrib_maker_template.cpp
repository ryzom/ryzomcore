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

#include "std3d.h"
#include "nel/3d/ps_attrib_maker_template.h"
#include "nel/misc/system_info.h"


namespace NL3D
{


void computeGradient(const NLMISC::CRGBA *valueTab, uint32 numValues, uint32 nbStages, CPSVector<NLMISC::CRGBA>::V &grad, NLMISC::CRGBA &minValue, NLMISC::CRGBA &maxValue)
{
	nlassert(numValues > 1);
	nlassert(nbStages > 0);

	maxValue = minValue = valueTab[0];
	uint nbValues = (numValues - 1) * nbStages;
	grad.resize(nbValues + 1);


	float step = 1.0f / float(nbStages);
	float alpha;

	NLMISC::CRGBA *dest = &grad[0];
#if defined(NL_OS_WINDOWS) && !defined(NL_NO_ASM)
	float stepX127 = 127.f * step;
	if (NLMISC::CSystemInfo::hasMMX())
	{
		// do interpolation using mmx
		for (uint32 k = 0; k  < (numValues - 1); ++k)
		{
			uint32 col0 = *(uint32 *) &valueTab[k];
			uint64 stepPacked;
			sint16 *step = (sint16 *) &stepPacked;
			step[0] = (sint16) (stepX127 * (valueTab[k + 1].R - valueTab[k].R));
			step[1] = (sint16) (stepX127 * (valueTab[k + 1].G - valueTab[k].G));
			step[2] = (sint16) (stepX127 * (valueTab[k + 1].B - valueTab[k].B));
			step[3] = (sint16) (stepX127 * (valueTab[k + 1].A - valueTab[k].A));
			__asm
			{
					pxor mm0, mm0		 // mm0 = 0
					movd mm1, col0
					punpcklbw mm0, mm1 // store current col in mm0, each component is stored in 7:8 fixed integer
					psrlw     mm0, 1
					movq mm2, stepPacked       // store step for gradient
					mov  edi, dest
					mov  eax, 4
					mov  ecx, nbStages
					sub  edi, eax
				myLoop:
					// unpack current color
					movq        mm1, mm0
					add			edi, eax // advance destination
					psrlw       mm1, 7
					packuswb    mm1, mm1
					movd		[edi], mm1 // store result
					paddsw     mm0, mm2 // increment color
					dec ecx
					jne myLoop
					emms
			}
			dest += nbStages;
		}
	}
	else
#endif
	{
		// copy the tab performing linear interpolation between values given in parameter
		for (uint32 k = 0; k  < (numValues - 1); ++k)
		{
			alpha = 0;

			for(uint32 l = 0; l < nbStages; ++l)
			{
				// use the right version of the template function PSValueBlend
				// to do the job
				*dest++ = PSValueBlend(valueTab[k], valueTab[k + 1], alpha);
				alpha += step;
			}
		}
	}
	*dest = valueTab[numValues - 1];
}

}
