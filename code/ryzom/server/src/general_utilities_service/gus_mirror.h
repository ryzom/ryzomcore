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



#ifndef GUS_MIRROR_H
#define GUS_MIRROR_H

#include "nel/misc/entity_id.h"
#include "game_share/mirrored_data_set.h"

namespace GUS
{

	// The mirror interface allow any GUS module to access mirror data.
	// The list of datasets and property declared in the mirror is
	// configured from the GUS config file (because changing mirror
	// declared property on the fly is not possible).
	// 
	class CGusMirror
	{
	public:
		static CGusMirror *getInstance();

		/// Interface class for module using mirror
		class IMirrorModuleCallback
		{
		public:
			// General callback
			virtual void mirrorIsReady(CGusMirror *gusMirror) {}
			virtual void serviceMirrorUp(CGusMirror *gusMirror, const std::string &serviceName, NLNET::TServiceId serviceId) {}
			virtual void serviceMirrorDown(CGusMirror *gusMirror, const std::string &serviceName, NLNET::TServiceId serviceId) {}
			virtual void mirrorTickUpdate(CGusMirror *gusMirror) {}

			// mirror updates
			virtual void entityAdded(CGusMirror *gusMirror, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex) {}
			virtual void entityRemoved(CGusMirror *gusMirror, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex, const NLMISC::CEntityId *entityId) {}
			virtual void propertyChanged(CGusMirror *gusMirror, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex, TPropertyIndex propIndex) {}

			IMirrorModuleCallback();
			virtual ~IMirrorModuleCallback();
		};
		
		/// A module register it's interface to receive mirror callback
		virtual void registerModuleCallback(IMirrorModuleCallback *mirrorCallback) =0;
		/// Unregister the callbacks
		virtual void unregisterModuleCallback(IMirrorModuleCallback *mirrorCallback) =0;


		virtual CMirroredDataSet *getDataSet(const std::string &dataSetName) =0;

	};

}; // namespace GUS

#endif //GUS_MIRROR_H
