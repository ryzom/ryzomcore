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

#include <string>

#include "nel/misc/types_nl.h"
#include "nel/3d/driver.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/misc/algo.h"

//#include <cstdio>

using namespace NLMISC;
using namespace std;

namespace NL3D
{

// ***************************************************************************
const uint32 IDriver::InterfaceVersion = 0x6e; // gpu program interface

// ***************************************************************************
IDriver::IDriver() : _SyncTexDrvInfos( "IDriver::_SyncTexDrvInfos" )
{
	_PolygonMode= Filled;
	_StaticMemoryToVRAM=false;
	_ResetCounter=0;
}

// ***************************************************************************
IDriver::~IDriver()
{
	// Must clean up everything before closing driver.
	// Must doing this in release(), so assert here if not done...
	{
		CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
		TTexDrvInfoPtrMap &rTexDrvInfos = access.value();
		nlassert( rTexDrvInfos.size() == 0 );
	}

	nlassert(_TexDrvShares.size()==0);
	nlassert(_MatDrvInfos.size()==0);
	nlassert(_VBDrvInfos.size()==0);
	nlassert(_IBDrvInfos.size()==0);
	nlassert(_GPUPrgDrvInfos.size()==0);
}


// ***************************************************************************
bool		IDriver::release(void)
{
	// Called by derived classes.

	// DO THIS FIRST => to auto kill real textures (by smartptr).
	// First, Because must not kill a pointer owned by a CSmartPtr.
	// Release Textures drv.
	ItTexDrvSharePtrList		ittex;
	while( (ittex = _TexDrvShares.begin()) !=_TexDrvShares.end() )
	{
		// NB: at CTextureDrvShare deletion, this->_TexDrvShares is updated (entry deleted);
		delete *ittex;
	}


	// Release refptr of TextureDrvInfos. Should be all null (because of precedent pass).
	{
		CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
		TTexDrvInfoPtrMap &rTexDrvInfos = access.value();

		// must be empty, because precedent pass should have deleted all.
		nlassert(rTexDrvInfos.empty());
	}

	// Release material drv.
	ItMatDrvInfoPtrList		itmat;
	while( (itmat = _MatDrvInfos.begin()) != _MatDrvInfos.end() )
	{
		// NB: at IShader deletion, this->_MatDrvInfos is updated (entry deleted);
		delete *itmat;
	}

	// Release VBs drv.
	ItVBDrvInfoPtrList		itvb;
	while( (itvb = _VBDrvInfos.begin()) != _VBDrvInfos.end() )
	{
		// NB: at IVBDrvInfo deletion, this->_VBDrvInfos is updated (entry deleted);
		delete *itvb;
	}

	// Release IBs drv.
	ItIBDrvInfoPtrList		itib;
	while( (itib = _IBDrvInfos.begin()) != _IBDrvInfos.end() )
	{
		// NB: at IIBDrvInfo deletion, this->_IBDrvInfos is updated (entry deleted);
		delete *itib;
	}

	// Release GPUPrg drv.
	ItGPUPrgDrvInfoPtrList		itGPUPrg;
	while( (itGPUPrg = _GPUPrgDrvInfos.begin()) != _GPUPrgDrvInfos.end() )
	{
		// NB: at IVertexProgramDrvInfos deletion, this->_GPUPrgDrvInfos is updated (entry deleted);
		delete *itGPUPrg;
	}

	return true;
}


// ***************************************************************************
GfxMode::GfxMode(uint16 w, uint16 h, uint8 d, bool windowed, bool offscreen, uint frequency, sint8 aa, const std::string &displayDevice)
{
	DisplayDevice = displayDevice;
	Windowed = windowed;
	Width = w;
	Height = h;
	Depth = d;
	OffScreen = offscreen;
	Frequency = frequency;
	AntiAlias = aa;
}

// ***************************************************************************
IDriver::TMessageBoxId IDriver::systemMessageBox (const char* message, const char* title, IDriver::TMessageBoxType type, IDriver::TMessageBoxIcon icon)
{
	static const char* icons[iconCount]=
	{
		"",
		"WAIT:\n",
		"QUESTION:\n",
		"HEY!\n",
		"",
		"WARNING!\n",
		"ERROR!\n",
		"INFORMATION:\n",
		"STOP:\n"
	};
	static const char* messages[typeCount]=
	{
		"Press any key...",
		"(O)k or (C)ancel ?",
		"(Y)es or (N)o ?",
		"(A)bort (R)etry (I)gnore ?",
		"(Y)es (N)o (C)ancel ?",
		"(R)etry (C)ancel ?"
	};
	printf ("%s%s\n%s", icons[icon], title, message);
	for(;;)
	{
		printf ("\n%s", messages[type]);
		int c=getchar();
		if (type==okType)
			return okId;
		switch (c)
		{
		case 'O':
		case 'o':
			if ((type==okType)||(type==okCancelType))
				return okId;
			break;
		case 'C':
		case 'c':
			if ((type==yesNoCancelType)||(type==okCancelType)||(type==retryCancelType))
				return cancelId;
			break;
		case 'Y':
		case 'y':
			if ((type==yesNoCancelType)||(type==yesNoType))
				return yesId;
			break;
		case 'N':
		case 'n':
			if ((type==yesNoCancelType)||(type==yesNoType))
				return noId;
			break;
		case 'A':
		case 'a':
			if (type==abortRetryIgnoreType)
				return abortId;
			break;
		case 'R':
		case 'r':
			if (type==abortRetryIgnoreType)
				return retryId;
			break;
		case 'I':
		case 'i':
			if (type==abortRetryIgnoreType)
				return ignoreId;
			break;
		}
	}
	nlassert (0);		// no!
	return okId;
}




// ***************************************************************************
void			IDriver::removeVBDrvInfoPtr(ItVBDrvInfoPtrList  vbDrvInfoIt)
{
	_VBDrvInfos.erase(vbDrvInfoIt);
}
// ***************************************************************************
void			IDriver::removeIBDrvInfoPtr(ItIBDrvInfoPtrList  ibDrvInfoIt)
{
	_IBDrvInfos.erase(ibDrvInfoIt);
}
// ***************************************************************************
void			IDriver::removeTextureDrvInfoPtr(ItTexDrvInfoPtrMap texDrvInfoIt)
{
	CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
	TTexDrvInfoPtrMap &rTexDrvInfos = access.value();

	rTexDrvInfos.erase(texDrvInfoIt);
}
// ***************************************************************************
void			IDriver::removeTextureDrvSharePtr(ItTexDrvSharePtrList texDrvShareIt)
{
	_TexDrvShares.erase(texDrvShareIt);
}
// ***************************************************************************
void			IDriver::removeMatDrvInfoPtr(ItMatDrvInfoPtrList shaderIt)
{
	_MatDrvInfos.erase(shaderIt);
}
// ***************************************************************************
void			IDriver::removeGPUPrgDrvInfoPtr(ItGPUPrgDrvInfoPtrList gpuPrgDrvInfoIt)
{
	_GPUPrgDrvInfos.erase(gpuPrgDrvInfoIt);
}

// ***************************************************************************
bool			IDriver::invalidateShareTexture (ITexture &texture)
{
	// Create the shared Name.
	std::string	name;
	getTextureShareName (texture, name);

	// Look for the driver info for this share name
	CSynchronized<TTexDrvInfoPtrMap>::CAccessor access(&_SyncTexDrvInfos);
	TTexDrvInfoPtrMap &rTexDrvInfos = access.value();
	TTexDrvInfoPtrMap::iterator iteDrvInfo = rTexDrvInfos.find (name);
	if (iteDrvInfo != rTexDrvInfos.end())
	{
		// Now parse all shared info
		TTexDrvSharePtrList::iterator shareIte = _TexDrvShares.begin ();
		while (shareIte != _TexDrvShares.end ())
		{
			// Good one ?
			if ((*shareIte)->DrvTexture == iteDrvInfo->second)
			{
				// Remove this one
				TTexDrvSharePtrList::iterator toRemove = shareIte;
				shareIte++;
				delete (*toRemove);
			}
			else
				shareIte++;
		}

		// Ok
		return true;
	}
	return false;
}
// ***************************************************************************
void			IDriver::getTextureShareName (const ITexture& tex, string &output)
{
	// Create the shared Name.
	output= toLower(tex.getShareName());

	// append format Id of the texture.
	static char	fmt[256];
	smprintf(fmt, 256, "@Fmt:%d", (uint32)tex.getUploadFormat());
	output+= fmt;

	// append mipmap info
	if(tex.mipMapOn())
		output+= "@MMp:On";
	else
		output+= "@MMp:Off";
}

// ***************************************************************************

void			IDriver::setStaticMemoryToVRAM (bool staticMemoryToVRAM)
{
	_StaticMemoryToVRAM=staticMemoryToVRAM;
}

// ***************************************************************************
class CTextureDebugInfo
{
public:
	uint	MemoryCost;
	string	Line;

	bool	operator<(const CTextureDebugInfo &o) const {return Line<o.Line;}
};

class CTextureDebugKey
{
public:
	ITexture::TUploadFormat			UpLoadFormat;
	ITexture::CTextureCategory		*Category;

	bool	operator<(const CTextureDebugKey &o) const
	{
		const	string	&s0= Category?Category->Name:_EmptyString;
		const	string	&s1= o.Category?o.Category->Name:_EmptyString;
		if(s0 == s1)
			return UpLoadFormat<o.UpLoadFormat;
		else
			return s0<s1;
	}

private:
	static std::string				_EmptyString;
};
std::string				CTextureDebugKey::_EmptyString;


// ***************************************************************************
void IDriver::profileTextureUsage(std::vector<std::string> &result)
{
	std::set<ITextureDrvInfos	*>		texSet;
//	uint	i;

	// reserve result, sort by UploadFormat
	map<CTextureDebugKey, vector<CTextureDebugInfo> >	tempInfo;

	// Parse all the DrvShare list
	uint	totalSize= 0;
	ItTexDrvSharePtrList	it= _TexDrvShares.begin();
	for(;it!=_TexDrvShares.end();it++)
	{
		// get TexDrvInfos and owner
		ITextureDrvInfos	*gltext= (ITextureDrvInfos*)(ITextureDrvInfos*)(*it)->DrvTexture;
		ITexture			*text= (*it)->getOwnerTexture();
		nlassert(gltext && text);

		// sort by upload format and texture category
		CTextureDebugKey	infoKey;
		infoKey.UpLoadFormat= text->getUploadFormat();
		nlassert(infoKey.UpLoadFormat<ITexture::UploadFormatCount);
		infoKey.Category= text->getTextureCategory();

		// get the shareName
		string	shareName;
		if(text->supportSharing())
			shareName= toLower(text->getShareName());
		else
			shareName= "Not Shared";

		// only if not already append to the set
		if(texSet.insert(gltext).second)
		{
			uint	memCost= gltext->getTextureMemoryUsed();
			totalSize+= memCost;
			string	typeStr= typeid(*text).name();
			strFindReplace(typeStr, "class NL3D::", string());
			tempInfo[infoKey].push_back(CTextureDebugInfo());
			tempInfo[infoKey].back().Line= toString("Type: %15s. ShareName: %s. Size: %d Ko",
				typeStr.c_str(),
				shareName.c_str(),
				memCost/1024);
			tempInfo[infoKey].back().MemoryCost= memCost;
		}
	}

	// For convenience, sort
	map<CTextureDebugKey, vector<CTextureDebugInfo> >::iterator		itCat;
	for(itCat= tempInfo.begin();itCat!= tempInfo.end();itCat++)
		sort(itCat->second.begin(), itCat->second.end());

	// Store into result, appending Tag for each Mo reached. +10* is for extra lines and security
	result.clear();
	result.reserve(texSet.size() + 10*(tempInfo.size()) + totalSize/(1024*1024));

	// copy and add tags
	for(itCat= tempInfo.begin();itCat!= tempInfo.end();itCat++)
	{
		const CTextureDebugKey		&infoKey= itCat->first;
		vector<CTextureDebugInfo>	&infoVect= itCat->second;

		string		strUploadFormat;
		switch(infoKey.UpLoadFormat)
		{
		case	ITexture::Auto: strUploadFormat= ("Format: Auto"); break;
		case	ITexture::RGBA8888: strUploadFormat= ("Format: RGBA8888"); break;
		case	ITexture::RGBA4444: strUploadFormat= ("Format: RGBA4444"); break;
		case	ITexture::RGBA5551: strUploadFormat= ("Format: RGBA5551"); break;
		case	ITexture::RGB888: strUploadFormat= ("Format: RGB888"); break;
		case	ITexture::RGB565: strUploadFormat= ("Format: RGB565"); break;
		case	ITexture::DXTC1: strUploadFormat= ("Format: DXTC1"); break;
		case	ITexture::DXTC1Alpha: strUploadFormat= ("Format: DXTC1Alpha"); break;
		case	ITexture::DXTC3: strUploadFormat= ("Format: DXTC3"); break;
		case	ITexture::DXTC5: strUploadFormat= ("Format: DXTC5"); break;
		case	ITexture::Luminance: strUploadFormat= ("Format: Luminance"); break;
		case	ITexture::Alpha: strUploadFormat= ("Format: Alpha"); break;
		case	ITexture::AlphaLuminance: strUploadFormat= ("Format: AlphaLuminance"); break;
		case	ITexture::DsDt: strUploadFormat= ("Format: DsDt"); break;
		default: strUploadFormat= toString("Format??: %d", infoKey.UpLoadFormat); break;
		}

		// header info
		result.push_back(toString("**** %s. %s ****", infoKey.Category?infoKey.Category->Name.c_str():"",
			strUploadFormat.c_str()) );

		// display stats for this format
		uint	tagTotal= 0;
		uint	curTotal= 0;
		for(uint j=0;j<infoVect.size();j++)
		{
			result.push_back(infoVect[j].Line);
			tagTotal+= infoVect[j].MemoryCost;
			curTotal+= infoVect[j].MemoryCost;
			if(tagTotal>=1024*1024)
			{
				result.push_back(toString("---- %.1f Mo", float(curTotal)/(1024*1024)));
				tagTotal= 0;
			}
		}
		// append last line?
		if(tagTotal!=0)
			result.push_back(toString("---- %.1f Mo", float(curTotal)/(1024*1024)));
	}

	// append the total
	result.push_back(toString("**** Total ****"));
	result.push_back(toString("Total: %d Ko", totalSize/1024));
}

// ***************************************************************************

}
