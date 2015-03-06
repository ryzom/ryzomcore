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



#ifndef _PS_WRAPPER_H
#define _PS_WRAPPER_H




#include "nel/misc/rgba.h"
#include "nel/misc/vector.h"
//
#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/texture.h"
//
#include "particle_node.h"

namespace NLQT
{

/// Wrapper to read/write a value of type T
template <class T> class IPSWrapper
{
public:
	NLQT::CWorkspaceNode *OwnerNode; // Owner node of the property. When the property is modified, then the node will be marked as 'modified'
public:
	IPSWrapper() : OwnerNode(NULL)
	{
	}
	// for derivers : get a value
	virtual T get(void) const = 0;
	void setAndUpdateModifiedFlag(const T &value)
	{
		if (OwnerNode)
		{
			OwnerNode->setModified(true);
		}
		set(value);
	}
protected:
	// for derivers : set a value
	virtual void set(const T &) = 0;
};


/// Wrapper to read/write a scheme of type T
template <class T> class IPSSchemeWrapper
{
public:
	NLQT::CWorkspaceNode *OwnerNode; // Owner node of the property. When the property is modified, then the node will be marked as 'modified'
public:
	IPSSchemeWrapper() : OwnerNode(NULL) {}
	typedef NL3D::CPSAttribMaker<T> scheme_type;
	virtual scheme_type *getScheme(void) const = 0;
	void setSchemeAndUpdateModifiedFlag(scheme_type *s)
	{
		if (OwnerNode)
		{
			OwnerNode->setModified(true);
		}
		setScheme(s);
	}
protected:
	virtual void setScheme(scheme_type *s) = 0;
};



/// RGBA wrapper
typedef IPSWrapper<NLMISC::CRGBA> IPSWrapperRGBA;
typedef IPSSchemeWrapper<NLMISC::CRGBA> IPSSchemeWrapperRGBA;

/// float wrapper
typedef IPSWrapper<float> IPSWrapperFloat;
typedef IPSSchemeWrapper<float> IPSSchemeWrapperFloat;

/// uint wrapper
typedef IPSWrapper<uint32> IPSWrapperUInt;
typedef IPSSchemeWrapper<uint32> IPSSchemeWrapperUInt;

/// sint wrapper
typedef IPSWrapper<sint32> IPSWrapperInt;

/// texture wrapper
class IPSWrapperTexture
{
public:
	NLQT::CWorkspaceNode *OwnerNode;
public:
	// ctor
	IPSWrapperTexture() : OwnerNode(NULL) {}
	virtual NL3D::ITexture *get(void) = 0;
	virtual void setAndUpdateModifiedFlag(NL3D::ITexture *tex)
	{
		if (OwnerNode) OwnerNode->setModified(true);
		set(tex);
	}
protected:
	virtual void set(NL3D::ITexture *) = 0;
};

} /* namespace NLQT */

#endif