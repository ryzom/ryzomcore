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


#include "std_afx.h"

#include "nel/3d/ps_located.h"
#include "nel/3d/particle_system.h"


#include "nel/misc/stream.h"
#include "nel/misc/mem_stream.h"

#include "dup_ps.h"

#include <memory>

using namespace NL3D;

//=======================================================================



/** This can duplicate any serializable type by using a serialization policy (polymorphic, non polymorphic ..)
  * The serialization policy must have a method to serial a pointer on the object (see example below)  
  * NB : of course this is slow (but convenient) way of performing a copy 
  * TODO maybe this could be used elsewhere ?    
  */
template <class TSerializePolicy, typename T>
static T	*DupSerializable(const T *in) throw(NLMISC::EStream)
{
	NLMISC::CMemStream ms;	
	nlassert(!ms.isReading());
	T *nonConstIn = const_cast<T *>(in);
	TSerializePolicy::serial(nonConstIn, ms);
	std::vector<uint8> datas(ms.length());
	std::copy(ms.buffer(), ms.buffer() + ms.length(), datas.begin());		
	ms.resetPtrTable();
	ms.invert();
	ms.fill(&datas[0], (uint32)datas.size());
	nlassert(ms.isReading());	
	T *newObj = NULL;
	TSerializePolicy::serial(newObj, ms);
	return newObj;
}

/** A policy to duplicate a non-polymorphic type
  */
struct CDupObjPolicy
{
	template <typename T>
	static void serial(T *&obj, NLMISC::IStream &dest)  throw(NLMISC::EStream)
	{ 	
		dest.serialPtr(obj);
		/*if (dest.isReading())
		{
			std::auto_ptr<T> newObj(new T);
			newObj->serialPtr(dest);
			delete obj;
			obj = newObj.release();
		}
		else
		{		
			obj->serial(dest);
		}*/
	}	
};

/** A policy to duplicate a polymorphic type
  */
struct CDupPolymorphicObjPolicy
{
	template <typename T>
	static void serial(T *&obj, NLMISC::IStream &dest)  throw(NLMISC::EStream)
	{ 	
		dest.serialPolyPtr(obj);		
	}
};



//=======================================================================
/////////////////////////////////////////
// temp until there is a clone method  //
/////////////////////////////////////////
NL3D::CParticleSystemProcess	*DupPSLocated(const CParticleSystemProcess *in)
{
	if (!in) return NULL;
	try
	{
		// if the located doesn't belon to a system, copy it direclty
		if (in->getOwner() == NULL)
		{
			return DupSerializable<CDupPolymorphicObjPolicy>(in);
		}
		else
		{
			uint index = in->getOwner()->getIndexOf(*in);
			/** Duplicate the system, and detach.
			  * We can't duplicate the object direclty (it may be referencing other objects in the system, so these objects will be copied too...)
			  */
			 std::auto_ptr<CParticleSystem> newPS(DupSerializable<CDupObjPolicy>(in->getOwner()));	
			 // scene pointer is not serialised, but 'detach' may need the scene to be specified
			 newPS->setScene(in->getOwner()->getScene());
			return newPS->detach(index);			
		}
	}
	catch (NLMISC::EStream &e)
	{
		nlwarning (e.what());
		return NULL;
	}
}

//=======================================================================
/////////////////////////////////////////
// temp until there is a clone method  //
/////////////////////////////////////////
NL3D::CPSLocatedBindable	*DupPSLocatedBindable(CPSLocatedBindable *in)
{
	if (!in) return NULL;
	try
	{
		// if no owner, can copy the object directy
		if (in->getOwner() == NULL)
		{
			return DupSerializable<CDupPolymorphicObjPolicy>(in);
		}
		else
		{
			CParticleSystem *srcPS = in->getOwner()->getOwner();
			std::auto_ptr<CParticleSystem> newPS(DupSerializable<CDupObjPolicy>(srcPS));	
			// scene pointer is not serialised, but 'detach' may need the scene to be specified
			newPS->setScene(in->getOwner()->getOwner()->getScene());
			//
			uint index	  = srcPS->getIndexOf(*(in->getOwner()));
			uint subIndex = in->getOwner()->getIndexOf(in);
			//									
			newPS->setScene(in->getOwner()->getScene()); // 'unbind' require the scene to be attached
			CPSLocated *loc = NLMISC::safe_cast<CPSLocated *>(newPS->getProcess(index));
			return loc->unbind(subIndex);
		}
	}
	catch (NLMISC::EStream &e)
	{
		nlwarning (e.what());
		return NULL;
	}
}
