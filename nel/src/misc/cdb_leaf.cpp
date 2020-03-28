// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
// Copyright (C) 2016  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

//#define TRACE_READ_DELTA
//#define TRACE_WRITE_DELTA
//#define TRACE_SET_VALUE

#define DELAYED_OBSERVERS

//////////////
// Includes //
//////////////
#include "nel/misc/cdb_leaf.h"
#include "nel/misc/xml_auto_ptr.h"
#include "nel/misc/bit_mem_stream.h"
//#include <iostream.h>

////////////////
// Namespaces //
////////////////
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLMISC{


//-----------------------------------------------
//	init
//-----------------------------------------------
void CCDBNodeLeaf::init(  xmlNodePtr node, IProgressCallback &/* progressCallBack */, bool /* mapBanks */, CCDBBankHandler * /* bankHandler */ )
{
	CXMLAutoPtr type((const char*)xmlGetProp (node, (xmlChar*)"type"));
	nlassert((const char *) type != NULL);

	// IF type is an INT with n bits [1,64].
	if ((type.getDatas()[0] == 'I') || (type.getDatas()[0] == 'U'))
	{
		uint nbBit;
		fromString((const char *) (type.getDatas() + 1), nbBit);

		if(nbBit>=1 && nbBit<=64)
			_Type=(ICDBNode::EPropType)nbBit;
		else
		{
			nlwarning("CCDBNodeLeaf::init : property is an INT and should be between [1,64] but it is %d bit(s).", nbBit);
			_Type = ICDBNode::UNKNOWN;
		}
	}
	else if (type.getDatas()[0] == 'S')
	{
		uint nbBit;
		fromString((const char *) (type.getDatas() + 1), nbBit);

		if(nbBit>=1 && nbBit<=64)
			_Type = (ICDBNode::EPropType)(nbBit+64);
		else
		{
			nlwarning("CCDBNodeLeaf::init : property is an SINT and should be between [1,64] but it is %d bit(s).", nbBit);
			_Type = ICDBNode::UNKNOWN;
		}
	}
	// ELSE
	else
	{
		// IF it is a TEXT.
		if(!strcmp(type, "TEXT"))
			_Type = ICDBNode::TEXT;
		// ELSE type unknown.
		else
		{
			nlwarning("CCDBNodeLeaf::init : type '%s' is unknown.", type.getDatas());
			_Type = ICDBNode::UNKNOWN;
		}
	}

} // init //


//-----------------------------------------------
//	getNode
//
//-----------------------------------------------
ICDBNode * CCDBNodeLeaf::getNode( uint16 /* idx */ )
{
	return this;
} // getNode //


//-----------------------------------------------
//	getNode
//
//-----------------------------------------------
ICDBNode * CCDBNodeLeaf::getNode (const CTextId& id, bool /* bCreate */)
{
	if (_DBSM->localUnmap(_Name) == id.readNext())
	{
		if (id.size() == id.getCurrentIndex())
			return this;
	}
	return NULL;
} // getNode //

//-----------------------------------------------
//	write
//
//-----------------------------------------------
void CCDBNodeLeaf::write( CTextId& id, FILE * f)
{
	fprintf(f,"%" NL_I64 "d\t%s\n",_Property,id.toString().c_str());
} // write //

//-----------------------------------------------
//	readDelta
//-----------------------------------------------
void CCDBNodeLeaf::readDelta(TGameCycle gc, CBitMemStream & f )
{
	// If the property Type is valid.
	if(_Type > UNKNOWN && _Type < Nb_Prop_Type)
	{
		// Read the Property Value according to the Property Type.
		uint64 recvd = 0;
		uint bits;
		if (_Type == TEXT)
			bits = 32;
		else if (_Type <= I64)
			bits = _Type;
		else
			bits = _Type - 64;
		f.serial(recvd, bits);


		// if the DB update is older than last DB update, abort (but after the read!!)
		if(gc<_LastChangeGC)
			return;

		// bkup _oldProperty
		_oldProperty = _Property;

		// setup new one
		_Property = (sint64)recvd;

		// if signed
		if (! ((_Type == TEXT) || (_Type <= I64)))
		{
			// extend bit sign
			sint64 mask = (((sint64)1)<<bits)-(sint64)1;
			if( (_Property >> (bits-1))==1 )
			{
				_Property |= ~mask;
			}
		}
		if ( verboseDatabase )
		{
			nlinfo( "CDB: Read value (%u bits) %" NL_I64 "d", bits, _Property );
		}

		// bkup the date of change
		_LastChangeGC= gc;

		notifyObservers();

	}
	else
		nlwarning("CCDBNodeLeaf::readDelta : Property Type Unknown ('%d') -> not serialized.", (uint)_Type);
}// readDelta //


//-----------------------------------------------
// resetData
//-----------------------------------------------
void CCDBNodeLeaf::resetData(TGameCycle gc, bool forceReset)
{
	if(forceReset)
	{
		_LastChangeGC = 0;
		setValue64(0);
	}
	else if (gc>=_LastChangeGC)	// apply only if happens after the DB change
	{
		_LastChangeGC = gc;
		setValue64(0);
	}

//  Same version but without observer notification:
//	if ((!forceReset) && (gc<_LastChangeGC)) // if !forceReset, apply only if happens after the DB change
//		return;
//
//	if (forceReset)
//		gc = 0;
//
//	_LastChangeGC = gc;
//	_oldProperty = _Property;
//	if (_Property != 0)
//		_Changed = true;
//	_Property = 0;
}

//-----------------------------------------------
//	getProp
//
//-----------------------------------------------
sint64 CCDBNodeLeaf::getProp( CTextId& id )
{
	// assert that there are no lines left in the textid
	nlassert( id.getCurrentIndex() == id.size() );

	// Return the property value.
	return getValue64();
} // getProp //



//-----------------------------------------------
//	setProp
// Set the value of a property (the update flag is set to true)
// \param id is the text id of the property/grp
// \param name is the name of the property
// \param value is the value of the property
// \return bool : 'false' if id is too long.
//-----------------------------------------------
bool CCDBNodeLeaf::setProp( CTextId& id, sint64 value )
{
	// assert that there are no lines left in the textid
	if(id.getCurrentIndex() != id.size())
		return false;

	// Set the property value (and set "_Changed" flag with 'true');
	CCDBNodeLeaf::setValue64(value);

	// Done
	return true;
}// setProp //


//-----------------------------------------------
//	setPropCheckGC
//-----------------------------------------------
bool CCDBNodeLeaf::setPropCheckGC(TGameCycle gc, sint64 value)
{
	// Apply only if happens after the DB change
	if(gc>=_LastChangeGC)
	{
		// new recent date
		_LastChangeGC= gc;

		// Set the property value (and set "_Changed" flag with 'true');
		CCDBNodeLeaf::setValue64(value);

		return true;
	}
	else
		return false;
}

//-----------------------------------------------
//	clear
//
//-----------------------------------------------
void CCDBNodeLeaf::clear()
{

} // clear //



//-----------------------------------------------
//-----------------------------------------------
void CCDBNodeLeaf::setValue64(sint64 prop)
{
	if (_Property != prop)
	{
		if (!_Changed)
		{
			_Changed = true;
		}

		_oldProperty = _Property;
		_Property = prop;
		// notify observer
		notifyObservers();
	}
}

void CCDBNodeLeaf::setValue32(sint32 prop)
{
	sint64 newVal = (sint64)prop;
	setValue64(newVal);
}

void CCDBNodeLeaf::setValue16(sint16 prop)
{
	sint64 newVal = (sint64)prop;
	setValue64(newVal);

}

void CCDBNodeLeaf::setValue8(sint8 prop)
{
	sint64 newVal = (sint64)prop;
	setValue64(newVal);
}

void CCDBNodeLeaf::setValueBool(bool prop)
{
	sint64 newVal = (sint64)prop;
	setValue64(newVal);
}

void CCDBNodeLeaf::setValueRGBA (const CRGBA &color)
{
	sint64 newVal = (uint32)(color.R+(color.G<<8)+(color.B<<16)+(color.A<<24));
	setValue64(newVal);
}


void CCDBNodeLeaf::display(const std::string &prefix)
{
	nlinfo("%sL %s", prefix.c_str(), _DBSM->localUnmap(_Name).c_str());
}

//-----------------------------------------------
//	addObserver
//
//-----------------------------------------------
bool CCDBNodeLeaf::addObserver(IPropertyObserver* observer,CTextId& /* id */)
{
	_Observers.push_back(observer);
	return true;
}

//-----------------------------------------------
//	removeObserver
//
//-----------------------------------------------
bool CCDBNodeLeaf::removeObserver(IPropertyObserver* observer, CTextId& /* id */)
{
	std::vector<IPropertyObserver *>::iterator endIt = std::remove(_Observers.begin(), _Observers.end(), observer);
	if (endIt == _Observers.end()) return false; // no observer has been removed..
	_Observers.erase(endIt, _Observers.end());
	return true;
}

//-----------------------------------------------
void CCDBNodeLeaf::notifyObservers()
{
	std::vector<IPropertyObserver*> obs = _Observers;
	// notify observer
	for (std::vector<IPropertyObserver*>::const_iterator it = obs.begin(); it != obs.end(); it++)
	{
		(*it)->update(this);
	}
	// mark parent branchs
	if (_Parent)
		_Parent->onLeafChanged( _Name );
}






#ifdef TRACE_READ_DELTA
#undef TRACE_READ_DELTA
#endif

#ifdef TRACE_WRITE_DELTA
#undef TRACE_WRITE_DELTA
#endif

#ifdef TRACE_SET_VALUE
#undef TRACE_SET_VALUE
#endif
//#############################################################################################

}

