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

#ifndef NL_TRANSPORT_CLASS_H
#define NL_TRANSPORT_CLASS_H


//
// Includes
//

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/variable.h"

#include "unified_network.h"
#include "message.h"

#include <vector>
#include <string>

namespace NLNET {

//
// Macros
//

/**
 * Use this macro to register a class that can be transported in the init of your program.
 */

#define TRANSPORT_CLASS_REGISTER(_c) \
	static _c _c##Instance; \
	CTransportClass::registerClass (_c##Instance);

#define NETTC_INFO if (!VerboseNETTC.get()) {} else nlinfo
#define NETTC_DEBUG if (!VerboseNETTC.get()) {} else nldebug
extern NLMISC::CVariable<bool> VerboseNETTC;


//
// Classes
//

/**
 * You have to inherit this class and implement description() and callback() method.
 * For an example of use, take a look at nel/samples/class_transport sample.
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2002
 */
class CTransportClass
{
public:
	virtual ~CTransportClass() {}
	/** Different types that we can use in a Transport class
	 * warning: if you add/change a prop, change also in CTransportClass::init()
	 * warning: PropUKN must be the last value (used to resize a vector)
	 */
	enum TProp {
		PropUInt8, PropUInt16, PropUInt32, PropUInt64,
		PropSInt8, PropSInt16, PropSInt32, PropSInt64,
		PropBool, PropFloat, PropDouble, PropString, PropDataSetRow, PropSheetId, PropUCString, PropUKN };
//		PropBool, PropFloat, PropDouble, PropString, PropDataSetRow, PropEntityId, PropSheetId, PropUKN };


	//
	// Static methods
	//

	/// Init the transport class system (must be called one time, in the IService5::init() for example)
	static void init ();

	/// Release the transport class system (must be called one time, in the IService5::release() for example)
	static void release ();

	/** Call this function to register a new transport class.
	 * \param instance A reference to a GLOBAL space of the instance of this transport class. It will be used when receive this class from network.
	 */
	static void registerClass (CTransportClass &instance);

	/// Display registered transport class (debug purpose)
	static void displayLocalRegisteredClass ();


	//
	// Virtual methods
	//

	/** You have to implement this function with the description of your class. This description
	 * is used to send the class accross the network and read it. It must contains a class name and
	 * a set properties.
	 * Example:
	 *\code
		virtual void description ()
		{
			className ("SharedClass");
			property ("i1", PropUInt32, (uint32)11, i1);
		}
	 *\endcode
	 */
	virtual void description () = 0;

	/** This function will be call when we receive this class from the network. It will use the instance given at the
	 * registration process. By default, it does nothing.
	 */
	virtual void callback (const std::string &/* name */, NLNET::TServiceId /* sid */) { }


	//
	// Other methods
	//

	/// send the transport class to a specified service using the service id
	void send (NLNET::TServiceId sid);

	/// send the transport class to a specified service using the service name
	void send (const std::string &serviceName);

	/** The name of the transport class. Must be unique for each class.
	 */
	void className (const std::string &name);

	/** Return the name of the transport class.
	 *	The result is valid only AFTER calling of REGISTER_TRANSPORT_CLASS.
	 */
	const std::string &className()		{ return Name; }

	/** One property of the class. Look description() for an example of use.
	 * \param name The name of the property
	 * \param type Type of the property
	 * \param defaultValue The value you want to be set when a message comes without this property
	 * \param value Reference to the value where the property will be read or write
	 */
	template <class T> void property (const std::string &name, TProp type, T defaultValue, T &value)
	{
		switch (type)
		{
		case PropUInt8: case PropSInt8: case PropBool: nlassert(sizeof(T) == sizeof (uint8)); break;
		case PropUInt16: case PropSInt16: nlassert(sizeof(T) == sizeof (uint16)); break;
		case PropUInt32: case PropSInt32: case PropDataSetRow: nlassert(sizeof(T) == sizeof (uint32)); break;
		case PropUInt64: case PropSInt64: nlassert(sizeof(T) == sizeof (uint64)); break;
		case PropFloat: nlassert(sizeof(T) == sizeof (float)); break;
		case PropDouble: nlassert(sizeof(T) == sizeof (double)); break;
		case PropString: nlassert(sizeof(T) == sizeof (std::string)); break;
//		case PropEntityId: nlassert(sizeof(T) == sizeof (NLMISC::CEntityId)); break;
		case PropSheetId: nlassert(sizeof(T) == sizeof (NLMISC::CSheetId)); break;
		case PropUCString: nlassert(sizeof(T) == sizeof (ucstring)); break;
		default: nlerror ("property %s have unknown type %d", name.c_str(), type);
		}

		if (Mode == 2)			// write
		{
			// send only if needed
			// todo manage unknown prop
			TempMessage.serial (value);
		}
		else if (Mode == 3)	// register
		{
			// add a new prop to the current class
			nlassert (TempRegisteredClass.Instance != NULL);
			TempRegisteredClass.Instance->Prop.push_back (new CRegisteredProp<T> (name, type, defaultValue, &value));
		}
		else if (Mode == 4)	// display
		{
			std::string val;
			val = "defval: ";
			val += NLMISC::toString (defaultValue);
			val += " val: ";
			val += NLMISC::toString (value);
			NETTC_DEBUG ("NETTC:   prop %s %d: %s", name.c_str(), type, val.c_str());
		}
		else
		{
			nlstop;
		}
	}

	template <class T> void propertyVector (const std::string &name, TProp type, std::vector<T> &value)
	{
		if (Mode == 2)			// write
		{
			// send only if needed
			// todo manage unknown prop
			TempMessage.serialCont (value);
		}
		else if (Mode == 3)	// register
		{
			// add a new prop to the current class
			nlassert (TempRegisteredClass.Instance != NULL);
			TempRegisteredClass.Instance->Prop.push_back (new CRegisteredPropCont<std::vector<T> > (name, type, &value));
		}
		else if (Mode == 4)	// display
		{
			typedef typename std::vector<T>::iterator __iterator;
			std::string val;
			for (__iterator it = value.begin (); it != value.end(); it++)
			{
				val += NLMISC::toString (T(*it));
				val += " ";
			}
			NETTC_DEBUG ("NETTC:   prop %s %d: %d elements ( %s)", name.c_str(), type, value.size(), val.c_str());
		}
		else
		{
			nlstop;
		}
	}

	template <class T> void propertyCont (const std::string &name, TProp type, T &value)
	{
		if (Mode == 2)			// write
		{
			// send only if needed
			// todo manage unknown prop
			TempMessage.serialCont (value);
		}
		else if (Mode == 3)	// register
		{
			// add a new prop to the current class
			nlassert (TempRegisteredClass.Instance != NULL);
			TempRegisteredClass.Instance->Prop.push_back (new CRegisteredPropCont<T> (name, type, &value));
		}
		else if (Mode == 4)	// display
		{
			typedef typename T::iterator __iterator;
			std::string val;
			for (__iterator it = value.begin (); it != value.end(); it++)
			{
				val += NLMISC::toString (*it);
				val += " ";
			}
			NETTC_DEBUG ("NETTC:   prop %s %d: %d elements ( %s)", name.c_str(), type, value.size(), val.c_str());
		}
		else
		{
			nlstop;
		}
	}

	// Read the header (first part of the transport class message, currently only className)
	static void readHeader(CMessage& msgin, std::string& className)
	{
		msgin.serial(className);
	}

	/// Display with nlinfo the content of the class (debug purpose)
	void display ();

protected:

	//
	// Structures
	//

	struct CRegisteredBaseProp
	{
		CRegisteredBaseProp () : Type(PropUKN) { }
		virtual ~CRegisteredBaseProp() {}


		CRegisteredBaseProp (const std::string &name, TProp type) : Name(name), Type(type) { }

		std::string	Name;
		TProp Type;

		virtual void serialDefaultValue (NLMISC::IStream &/* f */) { }

		virtual void serialValue (NLMISC::IStream &/* f */) { }

		virtual void setDefaultValue () { }
	};

	typedef std::vector<std::pair<std::string, std::vector <CRegisteredBaseProp> > > TOtherSideRegisteredClass;

	struct CRegisteredClass
	{
		CTransportClass *Instance;

		CRegisteredClass () { clear (); }

		void clear () { Instance = NULL; }
	};

	typedef std::map<std::string, CRegisteredClass> TRegisteredClass;

	template <class T> struct CRegisteredProp : public CRegisteredBaseProp
	{
		CRegisteredProp () : Value(NULL) { }

		CRegisteredProp (const std::string &name, TProp type, T defaultValue, T *value = NULL) :
			CRegisteredBaseProp (name, type), DefaultValue(defaultValue), Value (value) { }

		T DefaultValue, *Value;

		virtual void serialDefaultValue (NLMISC::IStream &f)
		{
			f.serial (DefaultValue);
		}

		virtual void serialValue (NLMISC::IStream &f)
		{
			nlassert (Value != NULL);
			f.serial (*Value);
		}

		virtual void setDefaultValue ()
		{
			nlassert (Value != NULL);
			*Value = DefaultValue;
		}
	};

	template <class T> struct CRegisteredPropCont : public CRegisteredBaseProp
	{
		CRegisteredPropCont () : Value(NULL) { }

		CRegisteredPropCont (const std::string &name, TProp type, T *value = NULL) :
			CRegisteredBaseProp (name, type), Value (value) { }

		T *Value;

		virtual void serialDefaultValue (NLMISC::IStream &/* f */)
		{
			// nothing
		}

		virtual void serialValue (NLMISC::IStream &f)
		{
			nlassert (Value != NULL);
			f.serialCont (*Value);
		}

		virtual void setDefaultValue ()
		{
			nlassert (Value != NULL);
			Value->clear ();
		}
	};


	//
	// Variables
	//

	// Name of the class
	std::string	Name;

	// States to decode the stream from the network
	std::vector<std::vector<std::pair<sint, TProp> > > States;

	// Contains all propterties for this class
	std::vector<CRegisteredBaseProp *> Prop;


	//
	// Methods
	//

	// Read the TempMessage and call the callback
	bool read (const std::string &name, NLNET::TServiceId sid);

	// Used to create a TempMessage with this class
	NLNET::CMessage &write ();


	//
	// Static Variables
	//

	// Used to serialize unused properties from the TempMessage
	static std::vector<CRegisteredBaseProp *>	DummyProp;

	// Select what the description() must do
	static uint									Mode;	// 0=nothing 1=read 2=write 3=register 4=display

	// Contains all registered transport class
	static TRegisteredClass						LocalRegisteredClass;	// registered class that are in my program

	// The registered class that is currently filled (before put in LocalRegisteredClass)
	static CRegisteredClass						TempRegisteredClass;

	// The message that is currently filled/emptyed
	static NLNET::CMessage						TempMessage;

	static bool									Init;

	//
	// Static methods
	//

	// Called by release() to delete all structures
	static void unregisterClass ();

	// Fill the States merging local and other side class
	static void registerOtherSideClass (NLNET::TServiceId sid, TOtherSideRegisteredClass &osrc);

	// Create a message with local transport classes to send to the other side
	static void createLocalRegisteredClassMessage ();

	// Send the local transport classes to another service using the service id
	static void sendLocalRegisteredClass (NLNET::TServiceId sid)
	{
		nlassert (Init);
		NETTC_DEBUG ("NETTC: sendLocalRegisteredClass to %hu", sid.get());
		createLocalRegisteredClassMessage ();
		NLNET::CUnifiedNetwork::getInstance()->send (sid, TempMessage);
	}

	// Display a specific registered class (debug purpose)
	static void displayLocalRegisteredClass (CRegisteredClass &c);
	static void displayDifferentClass (NLNET::TServiceId sid, const std::string &className, const std::vector<CRegisteredBaseProp> &otherClass, const std::vector<CRegisteredBaseProp *> &myClass);


	//
	// Friends
	//

	friend void cbTCReceiveMessage (NLNET::CMessage &msgin, const std::string &name, NLNET::TServiceId sid);
	friend void cbTCUpService (const std::string &serviceName, NLNET::TServiceId sid, void *arg);
	friend void cbTCReceiveOtherSideClass (NLNET::CMessage &msgin, const std::string &name, NLNET::TServiceId sid);
};



/**
 * Get the name of message (for displaying), or extract the class name if it is a transport class.
 *
 * Preconditions:
 * - msgin is an input message that contains a valid message
 *
 * Postconditions:
 * - msgin.getPos() was modified
 * - msgName contains "msg %s" or "transport class %s" where %s is the name of message, or the name
 *   transport class is the message is a CT_MSG
 */
void getNameOfMessageOrTransportClass( NLNET::CMessage& msgin, std::string& msgName );


//
// Inlines
//

inline void CTransportClass::className (const std::string &name)
{
	if (Mode == 2)		// write
	{
		TempMessage.serial (const_cast<std::string &> (name));
	}
	else if (Mode == 3) // register
	{
		// add a new entry in my registered class
		nlassert (TempRegisteredClass.Instance != NULL);
		TempRegisteredClass.Instance->Name = name;
	}
	else if (Mode == 4) // display
	{
		NETTC_DEBUG ("NETTC: class %s:", name.c_str());
	}
	else
	{
		nlstop;
	}
}


inline void CTransportClass::send (NLNET::TServiceId sid)
{
	nlassert (Init);
	NLNET::CUnifiedNetwork::getInstance()->send (sid, write ());
}


inline void CTransportClass::send (const std::string &serviceName)
{
	nlassert (Init);
	NLNET::CUnifiedNetwork::getInstance()->send (serviceName, write ());
}

inline void CTransportClass::display ()
{
	nlassert (Mode == 0);

	// set the mode to register
	Mode = 4;

	description ();

	// set to mode none
	Mode = 0;
}

inline NLNET::CMessage &CTransportClass::write ()
{
	nlassert (Init);
	nlassert (Mode == 0);

#ifndef FINAL_VERSION
	// Did the programmer forget to register the transport class? Forbid sending then.
	nlassert( LocalRegisteredClass.find( className() ) != LocalRegisteredClass.end() );
#endif

	// set the mode to register
	Mode = 2;

	TempMessage.clear ();
	if (TempMessage.isReading())
		TempMessage.invert();
	TempMessage.setType ("CT_MSG");

	description ();

	// set to mode none
	Mode = 0;

	display ();

	return TempMessage;
}

inline bool CTransportClass::read (const std::string &name, NLNET::TServiceId sid)
{
	nlassert (Init);
	nlassert (Mode == 0);

	// there's no info about how to read this message from this sid, give up
	if (sid.get() >= States.size())
		return false;

	// set flag of all prop

	std::vector<uint8> bitfield;
	bitfield.resize (Prop.size(), 0);

	// init prop from the stream
	uint i;
	for (i = 0; i < States[sid.get()].size(); i++)
	{
		if (States[sid.get()][i].first == -1)
		{
			// skip the value from the stream
			DummyProp[States[sid.get()][i].second]->serialDefaultValue (TempMessage);
		}
		else
		{
			// get the good value
			Prop[States[sid.get()][i].first]->serialValue (TempMessage);
			bitfield[States[sid.get()][i].first] = 1;
		}
	}

	// set default value for unknown prop
	for (i = 0; i < Prop.size(); i++)
	{
		if (bitfield[i] == 0)
		{
			Prop[i]->setDefaultValue ();
		}
	}

	display ();

	// call the user callback
	callback (name, sid);
	return true;
}

} // NLNET

#endif // NL_TRANSPORT_CLASS_H

/* End of transport_class.h */
