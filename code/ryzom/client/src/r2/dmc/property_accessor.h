// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef DMS_PROPERTYACCESSORT_H
#define DMS_PROPERTYACCESSORT_H

#include "nel/misc/types_nl.h"
#include "game_share/object.h"

#include <string>
#include <list>

namespace R2
{
	class CDynamicMapClient;
	class CObjectFactory;

	class CPropertyAccessor
	{
		public:
		CPropertyAccessor(CDynamicMapClient* client, CObjectFactory* factory)
		{
			_Client = client;
			_Factory = factory; //client->getFactory
		}

		~CPropertyAccessor();

		CObject* getPropertyValue(CObject* component, const std::string& attrName);

		// see if an object has a value in its base (e.g say if it redefines the default value)
		bool hasValueInBase(CObject* component, const std::string& attrName);

		//get The propertyValue as number or 0 if not found
		double getValueAsNumber(CObject* component, const std::string& attrName) const;

		const CObject* getPropertyValue(const CObject* component, const std::string& attrName) const;

		void getPropertyList(CObject* component, std::list<std::string>& propertyList);


		/** Add a local value that prevails over the network value when read from this property accessor
		  * 'localValue' is a value supplied by the caller. When getPropertyValue will be called on 'shadowedValue', then
		  * 'localValue' will be returned instead, thus shadowing the original value.
		  * Any previous value that is shadowing the original value will be deleted when calling 'shadowValue'
		  *
		  * Intended usage is to modify a value locally on the client while it is being edited and before its value is commited
		  * Typically example is the slider in the client ui :
		  * While moving a slider the client wants to have instant feedback of the value on the screen.
		  * Update of screen content is usually done by implementing event handlers that are triggered when a property of an object
		  * changes. Unless the shadowing system is used, a property change can only occur when the corresponding network msg
		  * is received. So each time the slider move, we should send a requestSetNode or similar network message and wait for it to return back to our client (edition
		  * messages are repeated to all editing clients)
		  *
		  * This is not desirable in the case of sliders, because too many messages would be generated while the user move the mouse.
		  * Instead we only want to send a net message when the user release the mouse button
		  *
		  * pattern of use :
		  *
		  * - While a property is being edited, 'shadowValue' should be called. Posssibly notification of property change should be fired by the caller.
		  *   any call to CPropertyAccessor::getPropertyValue will return the shadowed value instead of
		  * - If value is confirmed : 'commitValue' should be called, and the matching requestxxx  method should be called to notify the server that the value has changed.
		  *                           current local value is copied into. Possibly, notification of property change should be fired.
		  * - If value is canceled  : 'rollbackValue' should be called. Possibly, notification of property change should be fired.
		  *
		  *
		  */
		void shadowValue(CObject *shadowedValue, CObject *localValue);
		// Test if a value is shadowed, and returns the shadowing value
		CObject *getShadowingValue(CObject *shadowedValue);
		/** stops value shadowing for the object 'shadowedValue', confirming the changes.
		  * The local value is copied into the shadowed value
		  */
		void commitValue(CObject *shadowedValue);
		/** stops value shadowing for the object 'shadowedValue', leaving the shadowed value unmodified.
		  */
		void rollbackValue(CObject *shadowedValue);

		private:
			CDynamicMapClient* _Client;
			// remove dead references to shadowed value
			void purgeShadowedValues();
		public:
			class CShadowedValue
			{
			public:
				CShadowedValue() : LocalValue(NULL) {}
				CObject::TRefPtr ShadowedValue;
				CObject          *LocalValue;
			};
			// usually there isn't more than a few shadowed value at a time.
		private:
			std::vector<CShadowedValue> _ShadowedValues;
			CObjectFactory* _Factory;

	};

} // namespace DMS
#endif //DMS_PROPERTYACCESSORT_H
