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

#include "nel/misc/algo.h"


using	namespace std;

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

bool		testWildCard(const std::string &strIn, const std::string &wildCard)
{
	return testWildCard(strIn.c_str(), wildCard.c_str());
}


// ***************************************************************************
bool		testWildCard(const char *strIn, const char *wildCard)
{
	// run the 2 string in //el
	while(*wildCard!=0 && *strIn!=0)
	{
		// if same char, continue.
		if(*wildCard==*strIn)
		{
			wildCard++;
			strIn++;
		}
		// if wildCard is ?, continue
		else if(*wildCard=='?')
		{
			wildCard++;
			strIn++;
		}
		// if wildcard is *, recurs check.
		else if(*wildCard=='*')
		{
			wildCard++;
			// if last *, its OK.
			if(*wildCard==0)
				return true;
			// else must check next strings.
			else
			{
				// build the wilcard token. eg from "*pipo?", take "pipo"
				string	token;
				while(*wildCard!='*' && *wildCard!='?' && *wildCard!=0)
				{
					token+= *wildCard;
					wildCard++;
				}
				// if token size is empty, error
				if(token.empty())
					return false;

				// in strIn, search all the occurence of token. For each solution, recurs test.
				string	sCopy= strIn;
				string::size_type pos= sCopy.find(token, 0);
				while(pos!=string::npos)
				{
					// do a testWildCard test on the remaining string/wildCard
					if( testWildCard(strIn+pos+token.size(), wildCard) )
						// if succeed, end
						return true;
					// fails=> test with another occurence of token in the string.
					pos= sCopy.find(token, pos+1);
				}

				// if all failed, fail
				return false;
			}
		}
		// else fail
		else
			return false;
	}

	// If quit here because end Of 2 strs, OK.
	if(*wildCard==0 && *strIn==0)
		return true;
	// if quit here because wildCard=="*" and s="", OK too.
	if(*strIn==0 && wildCard[0]=='*' && wildCard[1]==0)
		return true;

	/*
		Else false:
			It may be wildCard="?aez" and s="" => error
			It may be wildCard="" and s="aer" => error
	*/
	return false;
}


// ***************************************************************************
void		splitString(const std::string &str, const std::string &separator, std::vector<std::string> &retList)
{
	string::size_type pos=0;
	string::size_type newPos=0;
	retList.clear();
	while( (newPos= str.find(separator,pos)) != string::npos)
	{
		// if not empty sub str. (skip repetition of separator )
		if(newPos-pos>0)
			retList.push_back(str.substr(pos, newPos-pos));
		// skip token
		pos= newPos+separator.size();
	}
	// copy the last substr
	if( pos<str.size() )
		retList.push_back(str.substr(pos, str.size()-pos));
}


// ***************************************************************************
void		splitUCString(const ucstring &ucstr, const ucstring &separator, std::vector<ucstring> &retList)
{
	ucstring::size_type pos=0;
	ucstring::size_type newPos=0;
	retList.clear();
	while( (newPos= ucstr.find(separator,pos)) != ucstring::npos)
	{
		// if not empty sub str. (skip repetition of separator )
		if(newPos-pos>0)
			retList.push_back(ucstr.substr(pos, newPos-pos));
		// skip token
		pos= newPos+separator.size();
	}
	// copy the last substr
	if( pos<ucstr.size() )
		retList.push_back(ucstr.substr(pos, ucstr.size()-pos));
}

// ***************************************************************************

void drawFullLine (float x0, float y0, float x1, float y1, std::vector<std::pair<sint, sint> > &result)
{
	result.clear ();
	// x0 must be < x1
	float dx = (float) fabs (x0-x1);
	float dy = (float) fabs (y0-y1);
	if ((dx == 0) && (dy == 0))
		result.push_back (pair<sint, sint> ((sint)floor (x0), (sint)floor (y0)));
	else if (dx > dy)
	{
		if (x0 > x1)
		{
			// Xchg 0 and 1
			float temp = x0;
			x0 = x1;
			x1 = temp;
			temp = y0;
			y0 = y1;
			y1 = temp;
		}

		float deltaX = x1 - x0;
		const float deltaY = (y1-y0)/deltaX;

		// Current integer pixel
		sint currentX = (sint)floor (x0);
		sint currentY = (sint)floor (y0);

		while (deltaX >= 0)
		{
			// Next point
			sint previousY = currentY;

			// Next y0
			if (deltaX > 1)
				y0 += deltaY;
			else
				y0 += deltaX * deltaY;

			deltaX -= 1;

			currentY = (sint)y0;

			// Add point
			if (currentY<=previousY)
			{
				do
				{
					result.push_back (pair<sint, sint> (currentX, previousY));
					previousY--;
				}
				while (currentY<=previousY);
			}
			else
			{
				do
				{
					result.push_back (pair<sint, sint> (currentX, previousY));
					previousY++;
				}
				while (currentY>=previousY);
			}

			// Next X
			currentX++;
		}
	}
	else
	{
		if (y0 > y1)
		{
			// Xchg 0 and 1
			float temp = y0;
			y0 = y1;
			y1 = temp;
			temp = x0;
			x0 = x1;
			x1 = temp;
		}

		float deltaY = y1 - y0;
		const float deltaX = (x1-x0)/deltaY;

		// Current integer pixel
		sint currentY = (sint)floor (y0);
		sint currentX = (sint)floor (x0);

		while (deltaY >= 0)
		{
			// Next point
			sint previousX = currentX;

			// Next x0
			if (deltaY > 1)
				x0 += deltaX;
			else
				x0 += deltaY * deltaX;

			deltaY -= 1;

			currentX = (sint)x0;

			// Add point
			if (currentX<=previousX)
			{
				do
				{
					result.push_back (pair<sint, sint> (previousX, currentY));
					previousX--;
				}
				while (currentX<=previousX);
			}
			else
			{
				do
				{
					result.push_back (pair<sint, sint> (previousX, currentY));
					previousX++;
				}
				while (currentX>=previousX);
			}

			// Next Y
			currentY++;
		}
	}
}

// ***************************************************************************

void drawLine (float x0, float y0, float x1, float y1, vector<pair<sint, sint> > &result)
{
	float	dx = (float)(floor(x1+0.5) - floor(x0+0.5));
	float	dy = (float)(floor(y1+0.5) - floor(y0+0.5));

	float	rdx = x1-x0;
	float	rdy = y1-y0;

	sint	d = (sint)std::max(fabs(dx), fabs(dy));
	float	maxd = (float)(std::max(fabs(rdx), fabs(rdy)));

	rdx /= maxd;
	rdy /= maxd;

	for (; d>=0; --d)
	{
		result.push_back(make_pair<sint,sint>((sint)floor(x0+0.5), (sint)floor(y0+0.5)));

		x0 += rdx;
		y0 += rdy;
	}
}

// ***************************************************************************

} // NLMISC
