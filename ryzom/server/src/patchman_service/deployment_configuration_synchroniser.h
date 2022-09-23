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

#ifndef DEPLOYMENT_CONFIGURATION_SYNCHRONISER_H
#define DEPLOYMENT_CONFIGURATION_SYNCHRONISER_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// game share
//#include "game_share/deployment_configuration.h"

// local
#include "module_admin_itf.h"


//-----------------------------------------------------------------------------
// class CDeploymentConfigurationSynchroniser
//-----------------------------------------------------------------------------

class CDeploymentConfigurationSynchroniser: public PATCHMAN::CDeploymentConfigurationSynchroniserSkel
{
public:
	// specialisation of CDeploymentConfigurationSynchroniserSkel
	void requestSync(NLNET::IModuleProxy *sender);
	void sync(NLNET::IModuleProxy *sender, const NLNET::TBinBuffer &dataBlob);

	// our own virtual callback method for derived clases to implement (optionally)
	virtual void cbDeploymentConfigurationSynchronised(NLNET::IModuleProxy* sender) {}

protected:
	// ctor and initialisation...
	CDeploymentConfigurationSynchroniser();
	void init(NLNET::IModule* parent);

private:
	NLNET::IModule* _Parent;
};

#endif
