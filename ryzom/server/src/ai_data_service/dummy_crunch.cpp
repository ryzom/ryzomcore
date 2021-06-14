// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "nel/misc/command.h"

#include "game_share/bmp4image.h"

#include <vector>

using namespace std;
using namespace NLMISC;



#include <stack>

class CNeighbour
{
	uint SurfaceCell;
	uint CellSurfaceIdx;

	uint8 Heading[16][16];
};

class CSurface
{
public:
	CSurface()
	{
		memset(Points,0,sizeof(Points));
	}

	void addPoint(uint x,uint y)
	{
		Points[y]|=(1<<x);
	}

	void displayPoints()
	{
		char txt[]="0123456789abcdef";
		nlinfo("Surface Points: ",txt);
		for (uint j=0;j<16;++j)
		{
			for (uint i=0;i<16;++i)
				txt[i]=(Points[j]&(1<<i))? ' ': '#';
			nlinfo("- %s",txt);
		}

	}

	sint16 Points[16];	// boolean map of surface shape
	uint8 XNeighbours[16][16];
	uint8 YNeighbours[16][16];
	std::vector<CNeighbour> Neighbours;
};

class CCell
{
public:
	sint8 Points[16][16];	// ends up with set of surface ids
	std::vector <CSurface *> Surfaces;
};

void generateSurfaces(char *topLeft,uint lineLen,CCell &cell)
{
	std::stack<uint> stack;
	uint i,j;

	// fill the points grid with -1 where accessible and -2 where inaccessible
	uint goodCount=0, badCount=0;
	for (j=0;j<16;++j)
		for (i=0;i<16;++i)
			if (*(topLeft+i+lineLen*j)==' ')
			{
				cell.Points[j][i]= -1;
				++goodCount;
			}
			else
			{
				cell.Points[j][i]= -2;
				++badCount;
			}

	// flood fill to convert -1s to surface ids
	uint surfaceCount=~0u;
	for (j=0;j<16;++j)
		for (i=0;i<16;++i)
			if (cell.Points[j][i]==-1)
			{
				CSurface *surface=new CSurface;
				cell.Surfaces.push_back(surface); 
				++surfaceCount;
				cell.Points[j][i]=surfaceCount;
				stack.push(16*j+i);
				while (!stack.empty())
				{
					// pop the coordinate off the stack
					uint x=stack.top()&0xf;
					uint y=stack.top()/16;
					stack.pop();

					// add the point to the surface
					surface->addPoint(x,y);

					// look for neighbouring points to add to the same surface
					if (x<15 && cell.Points[y][x+1]==-1) { cell.Points[y][x+1]=surfaceCount; stack.push(16*y+ x+1); }
					if (x>0  && cell.Points[y][x-1]==-1) { cell.Points[y][x-1]=surfaceCount; stack.push(16*y+ x-1); }
					if (y<15 && cell.Points[y+1][x]==-1) { cell.Points[y+1][x]=surfaceCount; stack.push(16*y+16+ x); }
					if (y>0  && cell.Points[y-1][x]==-1) { cell.Points[y-1][x]=surfaceCount; stack.push(16*y-16+ x); }
				}
				surface->displayPoints();
			}
	nlinfo("Generated %i surfaces, %i accessible cell.Points, %i inaccessible cell.Points",surfaceCount+1,goodCount,badCount);
	for (j=0;j<16;++j)
	{
		char txt[17];
		for (i=0;i<16;++i)
			txt[i]=(cell.Points[j][i]>=0 && cell.Points[j][i]<=9)? cell.Points[j][i]+'0': '#';
		txt[16]=0;
		nlinfo("- surfaces: %s", txt);
	}
}

static void generateSurfaceNeighbourhoods(
	CCell *tl, CCell *tm, CCell *tr,
	CCell *ml, CCell *mm, CCell *mr,
	CCell *bl, CCell *bm, CCell *br)
{
	// todo here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// generate horizontal neighbours

	// generate vertical neighbours

	// * add propagation maps to the surfaces
	// for each surface propagate out following neighbour map
}

NLMISC_COMMAND(dummyCrunch,"Run a test of pacs crunch","")
{
	if(args.size()!=0)
		return false;

	char testMap[]=
		"0000000000000000111111111111111100000000000000001111111111111111"
		"0000000000000000111111111111111100000000000000001111111111111111"
		"0000000000000000111111111111111100000000000000001111111111111111"
		"000                          111000                          111"
		"000                                                          111"
		"000                          111000                          111"
		"000   00000000001111111111   111000   00000000001111111111   111"
		"000                      1   111000                      1   111"
		"000                      1   111000                      1   111"
		"000                      1   111000                      1   111"
		"000                      1   111000                      1   111"
		"000                      1   111000                      1   111"
		"000                      1   111000                      1   111"
		"000                      1   111000                      1   111"
		"000   000000000011111    1   111000   000000000011111    1   111"
		"000      000000011111    1   111000      000000011111    1   111"
		"222      222222233333    3   333222      222222233333    3   333"
		"222222222222222233333    3   333222222222222222233333    3   333"
		"222                      3   333222                      3   333"
		"222                      3                               3   333"
		"222 2222222222223333333333   333222 2222222222223333333333   333"
		"22222                        33322222                        333"
		"222                          333222                          333"
		"222                          333222                          333"
		"222222222222222233333333333333332222222222  22223333333333333333"
		"22222222222222223              32222222222  22223333333333333333"
		"22222222222222223 333333333333 32222222222  22223333333333333333"
		"22222222222222223 33        33 32222222222  22223333333333333333"
		"22222222222222223           33 32222222222  22223333333333333333"
		"22222222222222223333        33 32222222222  22223333333333333333"
		"222222222222222233333      333        2222  22223333333333333333"
		"222222222222222233333333  33333322222 2222  22223333333333333333"
		"0000000000000000111111111  1111100000 0000  00001111111111111111"
		"00000000000000001111111111  111100000 0000  00001111111111111111"
		"000000000000000011111111111  11100000 0000  00001111111111111111"
		"000                           11000                          111"
		"000                            1000                          111"
		"000                          1  000                          111"
		"000   00000000001111111111   11  00   00000000001111111111   111"
		"000                      1   111  0                      1   111"
		"000                      1   1110                        1   111"
		"000                      1   11100                       1   111"
		"000                      1   111000                      1   111"
		"000                      1   111000                      1   111"
		"000                      1   111000                      1   111"
		"000                      1   111000                      1   111"
		"000   000000000011111    1   111000   000000000011111    1   111"
		"000      000000011111    1   111000      000000011111    1   111"
		"222      222222233333    3   333222      222222233333    3   333"
		"222222222222222233333    3   333222222222222222233333    3   333"
		"222                      3   333222                      3   333"
		"222                      3   333222                      3   333"
		"222 2222222222223333333333   333222 2222222222223333333333   333"
		"22222                        33322222                        333"
		"222                          333222                          333"
		"222                          333222                          333"
		"2222222222222222333333333333333322222222222222223333333333333333"
		"2222   222222222333      333333322         2222233           333"
		"22222 22222222223333 33 33333333222 22 22 222222333 3 33333 3333"
		"22222 22222222223333 33 33333333222 22 22 222222333 33 333 33333"
		"22222 22222222223333 33 33333333222 22 22 222222333 333 3 333333"
		"22222 22222222223333 33 33333333222 22 22 222222333 3333 3333333"
		"2222   222222222333      333333322         2222233           333"
		"2222222222222222333333333333333322222222222222223333333333333333";


	int i,j;

	// need to create a dummy cell that links into the cell grid in all slots as 
	// default value
//	*** here ***

	// setup a 4x4 cell grid in a 6x6 grid surrounded by NULLs
	typedef CCell *TCellRef;
	TCellRef cells[6][6];
	memset (cells,0,sizeof(cells));
	for (j=0;j<4;++j)
		for (i=0;i<4;++i)
			cells[j+1][i+1]=new CCell;

	// generate surfaces
	for (j=0;j<4;++j)
		for (i=0;i<4;++i)
			generateSurfaces(testMap+i*16+j*64*16,64,*cells[j+1][i+1]);

	// generate surface neighbourhoods
	for (j=0;j<4;++j)
		for (i=0;i<4;++i)
			generateSurfaceNeighbourhoods(
				cells[1+j-1][1+i-1], cells[1+j-1][1+i+0], cells[1+j-1][1+i+1],
				cells[1+j+0][1+i-1], cells[1+j+0][1+i+0], cells[1+j+0][1+i+1],
				cells[1+j+1][1+i-1], cells[1+j+1][1+i+0], cells[1+j+1][1+i+1]
				);

	return true;
}
