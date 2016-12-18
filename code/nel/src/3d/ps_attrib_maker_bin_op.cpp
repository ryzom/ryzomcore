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

#include "nel/3d/ps_attrib_maker_bin_op.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

	void MakePrivate(uint8 * dest, const NLMISC::CRGBA *src1, const NLMISC::CRGBA *src2, uint32 stride, uint32 numAttrib, CPSBinOp::BinOp op)
	{
		switch (op)
		{
			case CPSBinOp::modulate:
			{
				// CRGBA OPTIMISATION
				NLMISC::CRGBA::modulateColors((NLMISC::CRGBA *) dest, src1, src2, numAttrib, sizeof(NLMISC::CRGBA), stride);
			}
			break;
			case CPSBinOp::add:
			{
				// CRGBA OPTIMISATION
				NLMISC::CRGBA::addColors((NLMISC::CRGBA *) dest, src1, src2, numAttrib, sizeof(NLMISC::CRGBA), stride);
			}
			break;
			case CPSBinOp::subtract:
			{
				// CRGBA OPTIMISATION
				NLMISC::CRGBA::subtractColors((NLMISC::CRGBA *) dest, src1, src2, numAttrib, sizeof(NLMISC::CRGBA), stride);
			}
			break;
			default: break;
		}
	}

	void Make4Private(uint8 * dest, const NLMISC::CRGBA *src1, const NLMISC::CRGBA *src2, uint32 stride, uint32 numAttrib, CPSBinOp::BinOp op)
	{
		switch (op)
		{
			case CPSBinOp::modulate:
			{
				// CRGBA OPTIMISATION
				NLMISC::CRGBA::modulateColors((NLMISC::CRGBA *) dest, src1, src2, numAttrib, sizeof(NLMISC::CRGBA), stride, 4);
			}
			break;
			case CPSBinOp::add:
			{
				// CRGBA OPTIMISATION
				NLMISC::CRGBA::addColors((NLMISC::CRGBA *) dest, src1, src2, numAttrib, sizeof(NLMISC::CRGBA), stride, 4);
			}
			break;
			case CPSBinOp::subtract:
			{
				// CRGBA OPTIMISATION
				NLMISC::CRGBA::subtractColors((NLMISC::CRGBA *) dest, src1, src2, numAttrib, sizeof(NLMISC::CRGBA), stride, 4);
			}
			break;
			default: break;
		}
	}


	void MakeNPrivate(uint8 * dest, const NLMISC::CRGBA *src1, const NLMISC::CRGBA *src2, uint32 stride, uint32 numAttrib, CPSBinOp::BinOp op, uint nbReplicate)
	{
		switch (op)
		{
			case CPSBinOp::modulate:
			{
				// CRGBA OPTIMISATION
				NLMISC::CRGBA::modulateColors((NLMISC::CRGBA *) dest, src1, src2, numAttrib, sizeof(NLMISC::CRGBA), stride, nbReplicate);
			}
			break;
			case CPSBinOp::add:
			{
				// CRGBA OPTIMISATION
				NLMISC::CRGBA::addColors((NLMISC::CRGBA *) dest, src1, src2, numAttrib, sizeof(NLMISC::CRGBA), stride, nbReplicate);
			}
			break;
			case CPSBinOp::subtract:
			{
				// CRGBA OPTIMISATION
				NLMISC::CRGBA::subtractColors((NLMISC::CRGBA *) dest, src1, src2, numAttrib, sizeof(NLMISC::CRGBA), stride, nbReplicate);
			}
			break;
			default: break;
		}
	}
} // NL3D
