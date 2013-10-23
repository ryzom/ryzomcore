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

#ifndef NL_PACS_CLIENT_H
#define NL_PACS_CLIENT_H

#include "stdnet.h"

#include "inet_address.h"
#include "callback_client.h"
#include "naming_client.h"

#include "nel/pacs/u_move_primitive.h"
#include "nel/pacs/u_collision_desc.h"

#define NLNET_PACS_PROTOCOL_VERSION 1

namespace NLNET
{

TCallbackItem PacsCallbackArray[];

/**
 * Client side of Pacs Service. Allows to use PACS functionnality by the networtk.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CPacsClient
{
	friend void cbPacsAnswer (CMessage &msgin, TSockId from, CCallbackNetBase &netbase);
public:

	/// Constructor
	CPacsClient()
	{
		// No connexion
		_Server=NULL;
	}

	~CPacsClient()
	{
		disconnect ();
	}

	bool connect ();

	void disconnect ()
	{
		if (_Server)
		{
			_Server->disconnect ();
			delete _Server;
		}
	}

	/**
	  * Prepare a new message
	  *
	  * You must call this method before do anything before sending the message.
	  */
	void	initMessage ()
	{
		_Message.clear ();
		_Message.setType ("PACS");
		_Message.serialCheck ((uint32)NLNET_PACS_PROTOCOL_VERSION);
	}

	/**
	  * Send the message
	  *
	  * You must call this method after initMessage and others calls to setup methods.
	  */
	void	sendMessage ()
	{
		// Checks
		nlassert (_Server);

		// Close the message
		bool nlFalse=false;
		_Message.serial (nlFalse);

		// Send the message
		_Server->send (_Message);
	}

	/**
	  * Update method. Should be called evenly.
	  */
	void	update ()
	{
		// Checks
		nlassert (_Server);

		_Server->update ();
	}

	/// \name Global retriever methods

	/**
	  * Make a raytrace test on the service.
	  *
	  * The service will answer this message with a rayTestCallback message.
	  *
	  * \param p0 is the first point of the ray.
	  * \param p1 is the second point of the ray.
	  * \param testId is the id of the test.
	  */
	void rayTest (double p0, double p1, uint32 testId)
	{
		// Append to the current message
		std::string name="RY";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, p0, p1, testId);
	}

	/// \name Move container methods

	/**
	  * Add a primitive in the service. Set the new primitive as current.
	  *
	  * No answer will be send by the service.
	  *
	  * \param id is the ID to attach to the new primitive.
	  */
	void addPrimitive (NLPACS::UMovePrimitive::TUserData id)
	{
		// Append to the current message
		std::string name="AD";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, id);
	}

	/**
	  * Remove a primitive from the service.
	  *
	  * No answer will be send by the service.
	  *
	  * \param id is the ID attach to the primitive to remove.
	  */
	void removePrimitive (NLPACS::UMovePrimitive::TUserData id)
	{
		// Append to the current message
		std::string name="RV";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, id);
	}

	/**
	  * Evaluate the collision on the servive.
	  *
	  * The service will answer this message with a triggerCallback message.
	  *
	  * \param evalId is the id of the evaluation.
	  * \param deltaTime is the delta time used to evaluate the system.
	  */
	void evalCollision (uint32 evalId, double deltaTime)
	{
		// Append to the current message
		std::string name="EV";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, evalId, deltaTime);
	}

	/**
	  * Test a move of a primitive on the service.
	  *
	  * The service will answer this message with a testMoveCallback message.
	  *
	  * \param id is the id of the primitive to test a move.
	  * \param speed is the speed of the primitive during its move.
	  * \param deltaTime is the time interval of the move to test.
	  */
	void testMove (NLPACS::UMovePrimitive::TUserData id, const NLMISC::CVectorD& speed, double deltaTime)
	{
		// Append to the current message
		std::string name="TS";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, id, const_cast<NLMISC::CVectorD&> (speed), deltaTime);
	}

	/// \name Primitives methods

	/**
	  * Set the current primitive on the service. The primitive stay current
	  * for the current message.
	  *
	  * No answer will be send by the service.
	  *
	  * \param id is the id of the current primitive to use.
	  */
	void setCurrentPrimitive (NLPACS::UMovePrimitive::TUserData id)
	{
		// Append to the current message
		std::string name="CU";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, id);
	}

	/**
	  * Set the type of the current primitive on the service.
	  *
	  * No answer will be send by the service.
	  *
	  * \param type is the new type for the primitive on the service.
	  */
	void setPrimitiveType (NLPACS::UMovePrimitive::TType type)
	{
		// Append to the current message
		std::string name="TY";
		uint32 t=(uint32)type;
		bool nlTrue=true;
		_Message.serial (nlTrue, name, t);
	}

	/**
	  * Set the reaction type of the current primitive on the service.
	  *
	  * No answer will be send by the service.
	  *
	  * \param type is the newreaction  type for the primitive on the service.
	  */
	void setReactionType (NLPACS::UMovePrimitive::TReaction type)
	{
		// Append to the current message
		std::string name="RT";
		uint32 t=(uint32)type;
		bool nlTrue=true;
		_Message.serial (nlTrue, name, t);
	}

	/**
	  * Set the trigger type of the current primitive on the service.
	  *
	  * No answer will be send by the service.
	  *
	  * \param type is the new trigger type for the primitive on the service.
	  */
	void setTriggerType (NLPACS::UMovePrimitive::TTrigger type)
	{
		// Append to the current message
		std::string name="TT";
		uint32 t=(uint32)type;
		bool nlTrue=true;
		_Message.serial (nlTrue, name, t);
	}

	/**
	  * Set the collision mask of the current primitive on the service.
	  *
	  * No answer will be send by the service.
	  *
	  * \param mask is the new collision mask for the primitive on the service.
	  */
	void setCollisionMask (NLPACS::UMovePrimitive::TCollisionMask mask)
	{
		// Append to the current message
		std::string name="CT";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, mask);
	}

	/**
	  * Set the occlusion mask of the current primitive on the service.
	  *
	  * No answer will be send by the service.
	  *
	  * \param mask is the new occlusion mask for the primitive on the service.
	  */
	void setOcclusionMask (NLPACS::UMovePrimitive::TCollisionMask mask)
	{
		// Append to the current message
		std::string name="OT";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, mask);
	}

	/**
	  * Set the obstacle flag of the current primitive on the service.
	  *
	  * No answer will be send by the service.
	  *
	  * \param obstacle is the new obstacle flag for the primitive on the service.
	  */
	void setObstacle (bool obstacle)
	{
		// Append to the current message
		std::string name="OB";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, obstacle);
	}

	/**
	  * Set the orientation the current primitive on the service.
	  *
	  * No answer will be send by the service.
	  *
	  * \param orientation is the new orientation for the primitive on the service.
	  */
	void setOrientation (double orientation)
	{
		// Append to the current message
		std::string name="OR";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, orientation);
	}

	/**
	  * Set the attenuation factor of the current primitive on the service.
	  *
	  * No answer will be send by the service.
	  *
	  * \param absorption is the new attenuation factor for the primitive on the service.
	  */
	void setAbsorption (float absorption)
	{
		// Append to the current message
		std::string name="AB";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, absorption);
	}

	/**
	  * Set the size of the current primitive on the service. Only for boxes primitives.
	  *
	  * No answer will be send by the service.
	  *
	  * \param width is the new size on X axis factor for the primitive on the service.
	  * \param depth is the new size on Y axis factor for the primitive on the service.
	  */
	void setSize (float width, float depth)
	{
		// Append to the current message
		std::string name="SZ";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, width, depth);
	}

	/**
	  * Set the height of the current primitive on the service. For boxes and cylinders primitives.
	  *
	  * No answer will be send by the service.
	  *
	  * \param height is the new size on Z axis factor for the primitive on the service.
	  */
	void setHeight (float height)
	{
		// Append to the current message
		std::string name="HE";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, height);
	}

	/**
	  * Set the radius of the current primitive on the service. For cylinders primitives.
	  *
	  * No answer will be send by the service.
	  *
	  * \param height is the new size on Z axis factor for the primitive on the service.
	  */
	void setRadius (float radius)
	{
		// Append to the current message
		std::string name="RD";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, radius);
	}

	/**
	  * Make the current primitive a global move. This move is slow.
	  * Use it only for the first placement and for teleporting.
	  *
	  * No answer will be send by the service.
	  *
	  * \param position is the new position for the primitive on the service.
	  */
	void globalMove (const NLMISC::CVectorD& position)
	{
		// Append to the current message
		std::string name="GM";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, const_cast<NLMISC::CVectorD&> (position));
	}

	/**
	  * Make the current primitive a relative move. This move is fast.
	  * Use it for current move. Make first a relative move of all your
	  * primitives, then put a evalCollision message. Then you can
	  * query position and speed by posting getPositionSpeed message.
	  *
	  * No answer will be send by the service.
	  *
	  * \param position is the new position for the primitive on the service.
	  */
	void relativeMove (const NLMISC::CVectorD& speed)
	{
		// Append to the current message
		std::string name="RM";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, const_cast<NLMISC::CVectorD&> (speed));
	}

	/**
	  * Query the position and the speed of the primitive after an evalCollision message.
	  *
	  * The service will answer with a getPositionSpeedCallback message.
	  *
	  * \param id is the id of the primitive to get the position and the speed.
	  */
	void getPositionSpeed (NLPACS::UMovePrimitive::TUserData id)
	{
		// Append to the current message
		std::string name="PS";
		bool nlTrue=true;
		_Message.serial (nlTrue, name, id);
	}

protected:

	/// \name Callbacks

	/**
	  * This call back is called when a message is coming. Used for synchronisation.
	  */
	virtual void	messageCallback ()
	{}

	/**
	  * This message is send by the service to answer the rayTest request.
	  *
	  * \param testId is the test ID passed to rayTest().
	  * \param testResult is false if the ray is not clipped, else true.
	  */
	virtual void	rayTestCallback (uint32 testId, bool testResult)
	{}

	/**
	  * This message is send by the service to answer the evalCollision request.
	  *
	  * \param evalId is the id of the evaluation passed to evalCollision.
	  * \param triggerInfo is an array of trigger descriptor. Each entry of the array is
	  * a new trigger raised by evalCollision.
	  */
	virtual void	triggerCallback (uint32 evalId, const std::vector<NLPACS::UTriggerInfo>& triggerInfo)
	{}

	/**
	  * This message is send by the service to answer the testMove request.
	  *
	  * \param id is the id of the primitive tested.
	  * \param testResult is true if the primitive can do that move, else false.
	  */
	virtual void	testMoveCallback (NLPACS::UMovePrimitive::TUserData id, bool testResult)
	{}

	/**
	  * This message is send by the service to answer the getPositionSpeed request.
	  *
	  * \param id is the id of the primitive.
	  * \param position is the new position of the primitive.
	  * \param speed is the new speed of the primitive.
	  */
	virtual void	getPositionSpeedCallback (NLPACS::UMovePrimitive::TUserData id, const NLMISC::CVectorD &position, const NLMISC::CVectorD &speed)
	{}

private:
	CCallbackClient		*_Server;
	CMessage			_Message;
};

// Callback to listen to the server
static void cbPacsAnswer (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Get the client pointer
	CPacsClient *client=(CPacsClient*)(uint)from->appId ();

	// Check stream
	msgin.serialCheck ((uint32)NLNET_PACS_PROTOCOL_VERSION);

	// Message callback
	client->messageCallback ();

	bool again;
	msgin.serial (again);

	while (again)
	{
		// Read the message sub string
		std::string subMessage;
		msgin.serial (subMessage);

		// This code can work only if sizeof (uint) == sizeof (void*)
		nlassert (sizeof (uint)==sizeof (void*));

		// Raytrace callback ?
		if (subMessage=="RY")
		{
			// Read test id and test result
			uint32 testId;
			bool testResult;
			msgin.serial (testId, testResult);

			// Call the callback
			client->rayTestCallback (testId, testResult);
		}
		// Trigger callback ?
		else if (subMessage=="TR")
		{
			// Read eval id and trigger info
			uint32 evalId;
			std::vector<NLPACS::UTriggerInfo> triggerInfo;
			msgin.serial (evalId);
			msgin.serialCont (triggerInfo);

			// Call the callback
			client->triggerCallback (evalId, triggerInfo);
		}
		// Test move callback ?
		else if (subMessage=="TM")
		{
			// Read the primitive id and test result
			NLPACS::UMovePrimitive::TUserData id;
			bool testResult;
			msgin.serial (id, testResult);

			// Call the callback
			client->testMoveCallback (id, testResult);
		}
		// Test move callback ?
		else if (subMessage=="PS")
		{
			// Read the primitive id and test result
			NLPACS::UMovePrimitive::TUserData id;
			NLMISC::CVectorD position;
			NLMISC::CVectorD speed;
			msgin.serial (id, position, speed);

			// Call the callback
			client->getPositionSpeedCallback (id, position, speed);
		}
		else
			NLMISC::nlError ("Pacs client: unkown sub message string");

		// Next message ?
		msgin.serial (again);
	}
}

static TCallbackItem PacsCallbackArray[] =
{
	{ "PACS_ASW", cbPacsAnswer }
};

inline 	bool CPacsClient::connect ()
{
	// Create a connexion
	_Server = new CCallbackClient;

	// Look up for PACS service
	CNamingClient::lookupAndConnect ("PS", *_Server);
	if (_Server->connected())
	{
		// Add callback array
		_Server->addCallbackArray (PacsCallbackArray, sizeof (PacsCallbackArray) / sizeof (PacsCallbackArray[0]));

		// This code can work only if sizeof (uint) == sizeof (void*)
		nlassert (sizeof (uint)==sizeof (void*));
		_Server->id ()->setAppId ((uint64)(uint)this);

		// Return ok
		return true;
	}
	else
	{
		return false;
	}
}

} // NLNET


#endif // NL_PACS_CLIENT_H

/* End of pacs_client.h */
