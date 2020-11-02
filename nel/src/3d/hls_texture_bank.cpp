// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "nel/3d/hls_texture_bank.h"


using	namespace std;
using	namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// CHLSTextureBank
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CHLSTextureBank::CHLSTextureBank()
{
}
// ***************************************************************************
void			CHLSTextureBank::reset()
{
	contReset(_ColorTextures);
	contReset(_TextureInstanceData);
	contReset(_TextureInstances);
}
// ***************************************************************************
uint32			CHLSTextureBank::addColorTexture(const CHLSColorTexture &tex)
{
	_ColorTextures.push_back(tex);
	return (uint32)_ColorTextures.size()-1;
}
// ***************************************************************************
void			CHLSTextureBank::addTextureInstance(const std::string &name, uint32 colorTextureId, const vector<CHLSColorDelta> &cols)
{
	string	nameLwr= toLowerAscii(name);

	// checks
	nlassert(colorTextureId<_ColorTextures.size());
	CHLSColorTexture	&colText= _ColorTextures[colorTextureId];
	nlassert(cols.size()==colText.getNumMasks());

	// new instance
	CTextureInstance	textInst;
	textInst._ColorTextureId= colorTextureId;
	textInst._DataIndex= (uint32)_TextureInstanceData.size();
	// leave ptrs undefined
	textInst._DataPtr= NULL;
	textInst._ColorTexturePtr= NULL;

	// allocate/fill data
	uint32	nameSize= (uint32)(nameLwr.size()+1);
	uint32	colSize= (uint32)cols.size()*sizeof(CHLSColorDelta);
	_TextureInstanceData.resize(_TextureInstanceData.size() + nameSize + colSize);
	// copy name
	if (nameSize != 0) memcpy(&_TextureInstanceData[textInst._DataIndex], nameLwr.c_str(), nameSize);
	// copy cols
	if (colSize != 0) memcpy(&_TextureInstanceData[textInst._DataIndex+nameSize], &cols[0], colSize);

	// add the instance.
	_TextureInstances.push_back(textInst);
}
// ***************************************************************************
void			CHLSTextureBank::compilePtrs()
{
	uint8	*data= &_TextureInstanceData[0];

	// For all texture instances, compute ptr.
	for(uint i=0;i<_TextureInstances.size();i++)
	{
		CTextureInstance	&text= _TextureInstances[i];
		text._DataPtr= data + text._DataIndex;
		text._ColorTexturePtr= &_ColorTextures[text._ColorTextureId];
	}
}


// ***************************************************************************
void			CHLSTextureBank::compile()
{
	// compile the ptrs.
	compilePtrs();

	// No other ops for now.
}


// ***************************************************************************
void			CHLSTextureBank::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);

	f.serialCont(_ColorTextures);
	f.serialCont(_TextureInstanceData);
	f.serialCont(_TextureInstances);

	// Must compile ptrs.
	if(f.isReading())
	{
		// compile the ptrs only.
		compilePtrs();
	}
}


// ***************************************************************************
void			CHLSTextureBank::fillHandleArray(std::vector<CTextureInstanceHandle> &array)
{
	for(uint i=0;i<_TextureInstances.size();i++)
	{
		CTextureInstanceHandle	h;
		h.Texture= &_TextureInstances[i];
		array.push_back(h);
	}
}


// ***************************************************************************
// ***************************************************************************
// CHLSTextureBank::CTextureInstance
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CHLSTextureBank::CTextureInstance::serial(NLMISC::IStream &f)
{
	f.serialVersion(0);

	f.serial(_DataIndex);
	f.serial(_ColorTextureId);
}


// ***************************************************************************
bool			CHLSTextureBank::CTextureInstance::operator<(const CTextureInstance &t) const
{
	// compare the 2 strings.
	return (strcmp((const char*)_DataPtr, (const char*)t._DataPtr)<0);
}
// ***************************************************************************
bool			CHLSTextureBank::CTextureInstance::operator<=(const CTextureInstance &t) const
{
	// compare the 2 strings.
	return (strcmp((const char*)_DataPtr, (const char*)t._DataPtr)<=0);
}


// ***************************************************************************
bool			CHLSTextureBank::CTextureInstance::sameName(const char *str)
{
	return (strcmp((const char*)_DataPtr, str)==0);
}


// ***************************************************************************
void			CHLSTextureBank::CTextureInstance::buildColorVersion(NLMISC::CBitmap &out)
{
	// get ptr to color deltas.
	uint	nameSize= (uint)strlen((const char*)_DataPtr)+1;
	CHLSColorDelta		*colDeltas= (CHLSColorDelta*)(_DataPtr + nameSize);

	// build the texture.
	_ColorTexturePtr->buildColorVersion(colDeltas, out);
}


} // NL3D
