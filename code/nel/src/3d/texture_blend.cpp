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

#include "nel/3d/texture_blend.h"
#include "nel/misc/common.h"

namespace NL3D {



CTextureBlend::CTextureBlend() : _BlendFactor(0), _SharingEnabled(true)
{
}


// ************************************************************************
bool	CTextureBlend::supportSharing() const
{
	return _BlendTex[0] && _BlendTex[0]->supportSharing()
			&& _BlendTex[1] && _BlendTex[1]->supportSharing();
}

// ************************************************************************
std::string		CTextureBlend::getShareName() const
{
	nlassert(supportSharing());
	char fmt[1024];
	NLMISC::smprintf(fmt, 1024, "BlendTex0:%s:BlendTex1:%s:blendFactor:%d",
			 _BlendTex[0]->getShareName().c_str(), _BlendTex[1]->getShareName().c_str(),
			 (uint) _BlendFactor
			 );
	return fmt;
}


// ************************************************************************
void CTextureBlend::enableSharing(bool enabled /*= false*/)
{
	_SharingEnabled = enabled;
}


// ************************************************************************
void CTextureBlend::release()
{
	if (_BlendTex[0] && _BlendTex[0]->getReleasable()) _BlendTex[0]->release();
	if (_BlendTex[1] && _BlendTex[1]->getReleasable()) _BlendTex[1]->release();
	ITexture::release();
}

// ************************************************************************
bool CTextureBlend::setBlendFactor(uint16 factor)
{
	nlassert(factor <= 256);
	if (factor != _BlendFactor)
	{
		_BlendFactor = factor;
		touch(); // need to recompute blending
		return true;
	}
	return false;
}


// ************************************************************************
void CTextureBlend::setBlendTexture(uint index, ITexture *tex)
{
	nlassert(index < 2);
	if (tex != _BlendTex[index])
	{
		_BlendTex[index] = tex;
		touch(); // need to recompute blending
	}
}


// ************************************************************************
void CTextureBlend::doGenerate(bool async)
{
	if (!_BlendTex[0] || !_BlendTex[1])
	{
		makeDummy();
		return;
	}
	//NLMISC::TTicks start = NLMISC::CTime::getPerformanceTime();
	_BlendTex[0]->generate();
	_BlendTex[1]->generate();

	this->blend(*_BlendTex[0], *_BlendTex[1], _BlendFactor, true);
	/*NLMISC::TTicks end = NLMISC::CTime::getPerformanceTime();
	nlinfo("blend time = %.2f", (float) (1000 * NLMISC::CTime::ticksToSecond(end - start)));*/
}


// ************************************************************************
void	CTextureBlend::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(0);
	ITexture::serial(f);
	for (uint k = 0; k < 2; ++k)
	{
		ITexture *tex = NULL;
		if (f.isReading())
		{
			f.serialPolyPtr(tex);
			_BlendTex[k] = tex;
			touch();
		}
		else
		{
			tex = _BlendTex[k];
			f.serialPolyPtr(tex);
		}
	}
	f.serial(_SharingEnabled, _BlendFactor);
}

} // NL3D
