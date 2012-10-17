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

/*
 * Limitations: Not threadsafe, not reentrant.
 */

//
// Includes
//

#include "stdnet.h"

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"

#include "nel/net/unified_network.h"

#include "nel/net/transport_class.h"


//
// Namespace
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace NLNET {

//
// Globals
//

NLMISC::CVariable<bool> VerboseNETTC("nel","VerboseNETTC","Enable verbose logging in CTransportClass operations",true,0,true);


//
// Variables
//

uint CTransportClass::Mode = 0;	// 0=nothing 1=read 2=write 3=register

map<string, CTransportClass::CRegisteredClass>	CTransportClass::LocalRegisteredClass;	// registered class that are in my program

CTransportClass::CRegisteredClass	CTransportClass::TempRegisteredClass;

NLNET::CMessage	CTransportClass::TempMessage;

vector<CTransportClass::CRegisteredBaseProp *> CTransportClass::DummyProp;

bool CTransportClass::Init = false;


//
// Functions
//

string typeToString (CTransportClass::TProp type)
{
	string conv[] = {
		"PropUInt8", "PropUInt16", "PropUInt32", "PropUInt64",
		"PropSInt8", "PropSInt16", "PropSInt32", "PropSInt64",
		"PropBool", "PropFloat", "PropDouble", "PropString", "PropDataSetRow", "PropSheetId", "PropUCString", "PropUKN" };
//		"PropBool", "PropFloat", "PropDouble", "PropString", "PropDataSetRow", "PropEntityId", "PropSheetId", "PropUKN" };

	if (type > CTransportClass::PropUKN)
		return "<InvalidType>";
	return conv[type];
}

void CTransportClass::displayDifferentClass (TServiceId sid, const string &className, const vector<CRegisteredBaseProp> &otherClass, const vector<CRegisteredBaseProp *> &myClass)
{
	NETTC_INFO ("NETTC: Service with sid %hu send me the TransportClass '%s' with differents properties:", sid.get(), className.c_str());
	NETTC_INFO ("NETTC:  My local TransportClass is:");
	for (uint i = 0; i < myClass.size(); i++)
	{
		NETTC_INFO ("NETTC:    Property: %d Name: '%s' type: '%s'", i, myClass[i]->Name.c_str(), typeToString(myClass[i]->Type).c_str());
	}

	NETTC_INFO ("NETTC:  The other side TransportClass is:");
	for (uint i = 0; i < otherClass.size(); i++)
	{
		NETTC_INFO ("NETTC:    Property: %d Name: '%s' type: '%s'", i, otherClass[i].Name.c_str(), typeToString(otherClass[i].Type).c_str());
	}
}

void CTransportClass::registerOtherSideClass (TServiceId sid, TOtherSideRegisteredClass &osrc)
{
	for (TOtherSideRegisteredClass::iterator it = osrc.begin(); it != osrc.end (); it++)
	{
		// find the class name in the map

		TRegisteredClass::iterator res = LocalRegisteredClass.find ((*it).first);
		if (res == LocalRegisteredClass.end ())
		{
			// The other service knows a class that we don't
			// there was previously an nlwarning here but that was wrong because it is quite normal for this to happen when one service
			// ueses different transport classes to communicate with several different services, so the message has been changed to an nldebug
			NETTC_DEBUG ("NETTC: the other side class '%s' declared from service %d is not registered in my system, skip it", (*it).first.c_str(),(uint32)sid.get());
			continue;
		}

		if (sid.get() >= (*res).second.Instance->States.size ())
			(*res).second.Instance->States.resize (sid.get()+1);

		(*res).second.Instance->States[sid.get()].clear ();

		for (sint j = 0; j < (sint)(*it).second.size (); j++)
		{
			// check each prop to see the correspondance

			// try to find the prop name in the array
			uint k;
			for (k = 0; k < (*res).second.Instance->Prop.size(); k++)
			{
				if ((*it).second[j].Name == (*res).second.Instance->Prop[k]->Name)
				{
					if ((*it).second[j].Type != (*res).second.Instance->Prop[k]->Type)
					{
						nlwarning ("NETTC: Property '%s' of the class '%s' have not the same type in the 2 sides (%s %s)", (*it).second[j].Name.c_str(), (*it).first.c_str(), typeToString((*it).second[j].Type).c_str(), typeToString((*res).second.Instance->Prop[k]->Type).c_str());
					}
					break;
				}
			}
			if (k == (*res).second.Instance->Prop.size())
			{
				// not found, put -1
				(*res).second.Instance->States[sid.get()].push_back (make_pair (-1, (*it).second[j].Type));
			}
			else
			{
				// same, store the index
				(*res).second.Instance->States[sid.get()].push_back (make_pair (k, PropUKN));
			}
		}

		// check if the version are the same
		if ((*it).second.size () != (*res).second.Instance->Prop.size ())
		{
			// 2 class don't have the same number of prop => different class => display class
			displayDifferentClass (sid, (*it).first.c_str(), (*it).second, (*res).second.Instance->Prop);
		}
		else
		{
			// check if the prop are same
			for (uint i = 0; i < (*res).second.Instance->Prop.size (); i++)
			{
				if ((*res).second.Instance->Prop[i]->Name != (*it).second[i].Name)
				{
					// different name => different class => display class
					displayDifferentClass (sid, (*it).first.c_str(), (*it).second, (*res).second.Instance->Prop);
					break;
				}
				else if ((*res).second.Instance->Prop[i]->Type != (*it).second[i].Type)
				{
					// different type => different class => display class
					displayDifferentClass (sid, (*it).first.c_str(), (*it).second, (*res).second.Instance->Prop);
					break;
				}
			}
		}
	}

	displayLocalRegisteredClass ();
}


void CTransportClass::registerClass (CTransportClass &instance)
{
	nlassert (Init);
	nlassert (Mode == 0);

	// set the mode to register
	Mode = 3;

	// clear the current class
	TempRegisteredClass.clear ();

	// set the instance pointer
	TempRegisteredClass.Instance = &instance;

	// fill name and props
	TempRegisteredClass.Instance->description ();

	// add the new registered class in the array
	LocalRegisteredClass[TempRegisteredClass.Instance->Name] = TempRegisteredClass;

	// set to mode none
	Mode = 0;
}

void CTransportClass::unregisterClass ()
{
	for (TRegisteredClass::iterator it = LocalRegisteredClass.begin(); it != LocalRegisteredClass.end (); it++)
	{
		for (uint j = 0; j < (*it).second.Instance->Prop.size (); j++)
		{
			delete (*it).second.Instance->Prop[j];
		}
		(*it).second.Instance->Prop.clear ();
		(*it).second.Instance = NULL;
	}
	LocalRegisteredClass.clear ();
}

void CTransportClass::displayLocalRegisteredClass (CRegisteredClass &c)
{
	NETTC_DEBUG ("NETTC:  > %s", c.Instance->Name.c_str());
	for (uint j = 0; j < c.Instance->Prop.size (); j++)
	{
		NETTC_DEBUG ("NETTC:    > %s %s", c.Instance->Prop[j]->Name.c_str(), typeToString(c.Instance->Prop[j]->Type).c_str());
	}

	for (uint l = 0; l < c.Instance->States.size (); l++)
	{
		if (c.Instance->States[l].size () != 0)
		{
			NETTC_DEBUG ("NETTC:      > sid: %u", l);
			for (uint k = 0; k < c.Instance->States[l].size (); k++)
			{
				NETTC_DEBUG ("NETTC:      - %d type : %s", c.Instance->States[l][k].first, typeToString(c.Instance->States[l][k].second).c_str());
			}
		}
	}
}

void CTransportClass::displayLocalRegisteredClass ()
{
	NETTC_DEBUG ("NETTC:> LocalRegisteredClass:");
	for (TRegisteredClass::iterator it = LocalRegisteredClass.begin(); it != LocalRegisteredClass.end (); it++)
	{
		displayLocalRegisteredClass ((*it).second);
	}
}

void cbTCReceiveMessage (CMessage &msgin, const string &name, TServiceId sid)
{
	NETTC_DEBUG ("NETTC: cbReceiveMessage");

	CTransportClass::TempMessage.clear();
	CTransportClass::TempMessage.assignFromSubMessage( msgin );

	string className;
	CTransportClass::readHeader(CTransportClass::TempMessage, className);

	CTransportClass::TRegisteredClass::iterator it = CTransportClass::LocalRegisteredClass.find (className);
	if (it == CTransportClass::LocalRegisteredClass.end ())
	{
		nlwarning ("NETTC: Receive unknown transport class '%s' received from %s-%hu", className.c_str(), name.c_str(), sid.get());
		return;
	}

	nlassert ((*it).second.Instance != NULL);

	if (!(*it).second.Instance->read (name, sid))
	{
		nlwarning ("NETTC: Can't read the transportclass '%s' received from %s-%hu (probably not registered on sender service)", className.c_str(), name.c_str(), sid.get());
	}
}

void cbTCReceiveOtherSideClass (CMessage &msgin, const string &/* name */, TServiceId sid)
{
	NETTC_DEBUG ("NETTC: cbReceiveOtherSideClass");

	CTransportClass::TOtherSideRegisteredClass osrc;

	uint32 nbClass;
	msgin.serial (nbClass);

	NETTC_DEBUG ("NETTC: %d class", nbClass);

	for (uint i = 0; i < nbClass; i++)
	{
		string className;
		msgin.serial (className);

		osrc.push_back(make_pair (className, vector<CTransportClass::CRegisteredBaseProp>()));

		uint32 nbProp;
		msgin.serial (nbProp);

		NETTC_DEBUG ("NETTC:   %s (%d prop)", className.c_str(), nbProp);

		for (uint j = 0; j < nbProp; j++)
		{
			CTransportClass::CRegisteredBaseProp prop;
			msgin.serial (prop.Name);
			msgin.serialEnum (prop.Type);
			NETTC_DEBUG ("NETTC:     %s %s", prop.Name.c_str(), typeToString(prop.Type).c_str());
			osrc[osrc.size()-1].second.push_back (prop);
		}
	}

	// we have the good structure
	CTransportClass::registerOtherSideClass (sid, osrc);
}

static TUnifiedCallbackItem CallbackArray[] =
{
	{ "CT_LRC", cbTCReceiveOtherSideClass },
	{ "CT_MSG", cbTCReceiveMessage },
};

void cbTCUpService (const std::string &serviceName, TServiceId sid, void * /* arg */)
{
	NETTC_DEBUG ("NETTC: CTransportClass Service %s %hu is up", serviceName.c_str(), sid.get());
	if (sid.get() >= 256)
		return;
	CTransportClass::sendLocalRegisteredClass (sid);
}

void CTransportClass::init ()
{
	// this isn't an error!
	if (Init) return;

	CUnifiedNetwork::getInstance()->addCallbackArray (CallbackArray, sizeof (CallbackArray) / sizeof (CallbackArray[0]));

	// create an instance of all d'ifferent prop types

	DummyProp.resize (PropUKN);

	nlassert (PropUInt8 < PropUKN); DummyProp[PropUInt8] = new CTransportClass::CRegisteredProp<uint8>;
	nlassert (PropUInt16 < PropUKN); DummyProp[PropUInt16] = new CTransportClass::CRegisteredProp<uint16>;
	nlassert (PropUInt32 < PropUKN); DummyProp[PropUInt32] = new CTransportClass::CRegisteredProp<uint32>;
	nlassert (PropUInt64 < PropUKN); DummyProp[PropUInt64] = new CTransportClass::CRegisteredProp<uint64>;
	nlassert (PropSInt8 < PropUKN); DummyProp[PropSInt8] = new CTransportClass::CRegisteredProp<sint8>;
	nlassert (PropSInt16 < PropUKN); DummyProp[PropSInt16] = new CTransportClass::CRegisteredProp<sint16>;
	nlassert (PropSInt32 < PropUKN); DummyProp[PropSInt32] = new CTransportClass::CRegisteredProp<sint32>;
	nlassert (PropSInt64 < PropUKN); DummyProp[PropSInt64] = new CTransportClass::CRegisteredProp<sint64>;
	nlassert (PropBool < PropUKN); DummyProp[PropBool] = new CTransportClass::CRegisteredProp<bool>;
	nlassert (PropFloat < PropUKN); DummyProp[PropFloat] = new CTransportClass::CRegisteredProp<float>;
	nlassert (PropDouble < PropUKN); DummyProp[PropDouble] = new CTransportClass::CRegisteredProp<double>;
	nlassert (PropString < PropUKN); DummyProp[PropString] = new CTransportClass::CRegisteredProp<string>;
//	nlassert (PropDataSetRow < PropUKN); DummyProp[PropDataSetRow] = new CTransportClass::CRegisteredProp<TDataSetRow>;
//	nlassert (PropEntityId < PropUKN); DummyProp[PropEntityId] = new CTransportClass::CRegisteredProp<CEntityId>;
	nlassert (PropSheetId < PropUKN); DummyProp[PropSheetId] = new CTransportClass::CRegisteredProp<CSheetId>;
	nlassert (PropUCString < PropUKN); DummyProp[PropUCString] = new CTransportClass::CRegisteredProp<ucstring>;

	// we have to know when a service comes, so add callback (put the callback before all other one because we have to send this message first)
	CUnifiedNetwork::getInstance()->setServiceUpCallback("*", cbTCUpService, NULL, false);

	Init = true;
}

void CTransportClass::release ()
{
	unregisterClass ();

	for (uint i = 0; i < DummyProp.size (); i++)
	{
		delete DummyProp[i];
	}
	DummyProp.clear ();
}

void CTransportClass::createLocalRegisteredClassMessage ()
{
	TempMessage.clear ();
	if (TempMessage.isReading())
		TempMessage.invert();
	TempMessage.setType ("CT_LRC");

	uint32 nbClass = (uint32)LocalRegisteredClass.size ();
	TempMessage.serial (nbClass);

	for (TRegisteredClass::iterator it = LocalRegisteredClass.begin(); it != LocalRegisteredClass.end (); it++)
	{
		nlassert ((*it).first == (*it).second.Instance->Name);

		TempMessage.serial ((*it).second.Instance->Name);

		uint32 nbProp = (uint32)(*it).second.Instance->Prop.size ();
		TempMessage.serial (nbProp);

		for (uint j = 0; j < (*it).second.Instance->Prop.size (); j++)
		{
			// send the name and the type of the prop
			TempMessage.serial ((*it).second.Instance->Prop[j]->Name);
			TempMessage.serialEnum ((*it).second.Instance->Prop[j]->Type);
		}
	}
}


/*
 * Get the name of message (for displaying), or extract the class name if it is a transport class.
 *
 * Preconditions:
 * - msgin is an input message that contains a valid message
 *
 * Postconditions:
 * - the pos in msgin was modified
 * - msgName contains "msg %s" or "transport class %s" where %s is the name of message, or the name
 * transport class is the message is a CT_MSG.
 */
void getNameOfMessageOrTransportClass( NLNET::CMessage& msgin, std::string& msgName )
{
	if ( msgin.getName() == "CT_MSG" )
	{
		try
		{
			msgin.seek( msgin.getHeaderSize(), NLMISC::IStream::begin );
			msgin.serial( msgName );
		}
		catch (const EStreamOverflow&)
		{
			msgName = "<Name not found>";
		}
		msgName = "transport class " + msgName;
	}
	else
	{
		msgName = "msg " + msgin.getName();
	}
}

} // NLNET
