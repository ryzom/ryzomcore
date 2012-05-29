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


#include <assert.h>
#include <algorithm>
#include "readpic.h"
#include "pic.h"
#include <nel/misc/rgba.h>

using namespace std;

//============================================================
// Image API.
//============================================================


bool	PIC_LoadPic(string path, vector<NLMISC::CBGRA> &tampon, uint &Width, uint &Height)
{
	uint32			id;
	char			*pal, *data;
	unsigned long	w,h,depth;
	uint			i;


	// Loadons l'image.
	id= PIC_Load((char*)path.c_str(), 0);
	if(id==0)
		return false;
	PIC_GetInfos( id,  &pal, &data, &w, &h, &depth);
	Width=w;
	Height=h;

	// On traduit en RGBA.
	tampon.resize(w*h);
	switch(depth)
	{
		case 8:
			for(i=0;i<w*h;i++)
			{
				tampon[i].R= data[i];
				tampon[i].G= data[i];
				tampon[i].B= data[i];
				tampon[i].A= data[i];
			}
			break;
		case 24:
			for(i=0;i<w*h;i++)
			{
				tampon[i].R= data[i*3+ 0];
				tampon[i].G= data[i*3+ 1];
				tampon[i].B= data[i*3+ 2];
				tampon[i].A= 255;
			}
			break;
		case 32:
			for(i=0;i<w*h;i++)
			{
				tampon[i].R= data[i*4+ 0];
				tampon[i].G= data[i*4+ 1];
				tampon[i].B= data[i*4+ 2];
				tampon[i].A= data[i*4+ 3];
			}
			break;
	}

	// On ferme.
	PIC_Destroy(id);

	return true;
}
