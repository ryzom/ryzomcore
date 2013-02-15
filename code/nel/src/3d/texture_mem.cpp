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

#include "nel/3d/texture_mem.h"
#include "nel/misc/mem_stream.h"

#include <memory>


namespace NL3D
{


/*==================================================================*\
							CTEXTUREMEM
\*==================================================================*/

/*------------------------------------------------------------------*\
							doGenerate()
\*------------------------------------------------------------------*/
void CTextureMem::doGenerate(bool /* async */)
{
	if (_Data)
	{
		if (_IsFile)
		{
			NLMISC::CMemStream m (true);
			m.fill (_Data, _Length);
			load (m);
		}
		else
		{
			resize(_TexWidth, _TexHeight, _TexType);
			::memcpy(&getPixels(0)[0], _Data, _Length);
			buildMipMaps();
		}
	}
	else
	{
		makeDummy();
	}
}


static NLMISC::CRGBA WhitePix(255, 255, 255, 255); // the texture datas ... :)

///===========================================================================
ITexture *CTextureMem::Create1x1WhiteTex()
{
	static NLMISC::CSmartPtr<ITexture> tex  = NULL;
	if (!tex)
	{
		tex = new CTextureMem((uint8 *) &WhitePix,
							   sizeof(WhitePix),
							   false, /* dont delete */
							   false, /* not a file */
							   1, 1);
		static_cast<CTextureMem *>((ITexture *)tex)->setShareName("#WhitePix1x1");
	}
	return (ITexture *) tex;
}

///===========================================================================
void	CTextureMem::setAllowDegradation(bool allow)
{
	_AllowDegradation= allow;
}

///===========================================================================
uint32 CTextureMem::getImageWidth() const
{
	return _TexWidth;
}

///===========================================================================
uint32 CTextureMem::getImageHeight() const
{
	return _TexHeight;
}


} // NL3D
