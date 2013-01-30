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


#include "stdmisc.h"
#include "nel/misc/grid_traversal.h"
#include "nel/misc/vector_2f.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

// ********************************************************************************************************************************
void	CGridTraversal::startTraverse(const NLMISC::CVector2f &start, sint &nextX, sint &nextY)
{
	nextX = (sint) floorf(start.x);
	nextY = (sint) floorf(start.y);
}

// ********************************************************************************************************************************
bool	CGridTraversal::traverse(const NLMISC::CVector2f &start, const NLMISC::CVector2f &dir, sint &x, sint &y)
{
	if (dir.x > 0.f)
	{
		float lambdaX = (x + 1.f - start.x) / dir.x;
		if (dir.y > 0.f)
		{
			float lambdaY = (y + 1 - start.y) / dir.y;
			if (lambdaX < lambdaY)
			{
				if (lambdaX > 1.f) return false;
				++ x;
				return true;
			}
			else
			{
				if (lambdaY > 1.f) return false;
				++ y;
				return true;
			}
		}
		else if (dir.y < 0.f)
		{
			float lambdaY = (y - start.y) / dir.y;
			if (lambdaX < lambdaY)
			{
				if (lambdaX > 1.f) return false;
				++ x;
				return true;
			}
			else
			{
				if (lambdaY > 1.f) return false;
				-- y;
				return true;
			}
		}
		++ x;
		return x <= (sint) floorf(start.x + dir.x);
	}
	else if (dir.x < 0.f)
	{
		float lambdaX = (x - start.x) / dir.x;
		if (dir.y > 0.f)
		{
			float lambdaY = (y + 1.f - start.y) / dir.y;
			if (lambdaX < lambdaY)
			{
				if (lambdaX > 1.f) return false;
				-- x;
				return true;
			}
			else
			{
				if (lambdaY > 1.f) return false;
				++ y;
				return true;
			}
		}
		else if (dir.y < 0.f)
		{
			float lambdaY = (y - start.y) / dir.y;
			if (lambdaX < lambdaY)
			{
				if (lambdaX > 1.f) return false;
				-- x;
				return true;
			}
			else
			{
				if (lambdaY > 1.f) return false;
				-- y;
				return true;
			}
		}
		-- x;
		return x >= (sint) floorf(start.x + dir.x);
	}

	if (dir.y > 0.f)
	{
		++ y;
		return y <= (sint) floorf(start.y + dir.y);

	}
	else if (dir.y < 0.f)
	{
		-- y;
		return y >= (sint) floorf(start.y + dir.y);
	}
	return false;
}

} // NLMISC
